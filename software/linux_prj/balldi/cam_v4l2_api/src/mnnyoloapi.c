/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>
#include "cam_v4l2.h"
#include "mnnyoloapi.h" //mnn yolov5_lite api header

static pthread_mutex_t MutexShrFrm = PTHREAD_MUTEX_INITIALIZER; //reason control
static pthread_mutex_t MutexGetFrm = PTHREAD_MUTEX_INITIALIZER; //get frame control

static int thread_reason_run_flg = 0;

//thread use swp data
static unsigned char FrameShrSwp[FRAME_MAX_LEN];
static int FrameLenSwp = 0;

//global save
static unsigned long Fid = 0;       //for repeat judgement
static unsigned char FrameShr[FRAME_MAX_LEN];
static int FrameLen = 0;
static t_result Rst[RST_MAX_LEN];
static int RstLen = 0;   //last reasoning result count

//hook from mnnyoloapi
fp_init_mnn_yolo func_init_mnn_yolo = NULL;
fp_close_mnn_yolo func_close_mnn_yolo = NULL;
fp_get_result_by_jpg_raw func_get_result_by_jpg_raw = NULL;


static inline void hold_get_frame_mtx(void)
{
    pthread_mutex_lock(&MutexGetFrm);
}
static void release_get_frame_mtx(void)
{
    //TODO: create a process , delay xx ms, then unlock MutexGetFrm, wait for Gin dealing with Frame
    pthread_mutex_unlock(&MutexGetFrm);
}

void set_reason_run_flg(int val)
{
    thread_reason_run_flg = val;
}
int get_reason_run_flg(void)
{
    return thread_reason_run_flg;
}

static void *thread_reasoning(void *args)
{
    const unsigned int wait_time = 50000; //us
    for(;;)
    {
        //printf("run_flg: %d\n", thread_reason_run_flg);
        if(!thread_reason_run_flg)
        {
            usleep(wait_time);
            continue;
        }

        if(NULL != func_get_result_by_jpg_raw)
        {
            RstLen = 0;

            func_get_result_by_jpg_raw(Rst, &RstLen, FrameShrSwp, FrameLenSwp);
        }
        else
        {
            printf("mnn yolo api reg failed.\n");
            usleep(wait_time);
            continue;
        }

        //save swp data to global
        hold_get_frame_mtx();

        memcpy(FrameShr, FrameShrSwp, FrameLenSwp);
        FrameLen = FrameLenSwp;
        t_result *tmp = Rst;
        //int i = 0;
        //for(; i < RstLen; ++i, ++tmp)
        //{
        //    printf(".c: x1:%d , ", tmp->x1);
        //    printf("y1:%d , ", tmp->y1);
        //    printf("x2:%d , ", tmp->x2);
        //    printf("y2:%d , ", tmp->y2);
        //    printf("label:%d , ", tmp->label);
        //    printf("id:%d , ", tmp->id);
        //    printf("score:%f \n", tmp->score);
        //}
        ++Fid;

        release_get_frame_mtx();
        usleep(wait_time);
    }
}

void set_share_frame(unsigned char *addr, int len)
{
    int lock_stat = pthread_mutex_trylock(&MutexShrFrm);

    if(0 == lock_stat)
    {
        //first in function do
        if(NULL == func_get_result_by_jpg_raw)
        {
            void *handle = dlopen("libmnnyoloapi.so", RTLD_LAZY);
            func_init_mnn_yolo = dlsym(handle, "init_mnn_yolo");
            func_get_result_by_jpg_raw = dlsym(handle, "get_result_by_jpg_raw");
            func_close_mnn_yolo = dlsym(handle, "close_mnn_yolo");

            if(NULL != func_init_mnn_yolo)
            {
                func_init_mnn_yolo();
                printf("init mnn yolo env\n");
            }
        }

        //I need only an instance
        pthread_t p;
        int ret = pthread_create(&p, NULL, thread_reasoning, NULL);
        if(0 != ret)
        {
            pthread_mutex_unlock(&MutexShrFrm);
            printf("pthread create failed, ret = %d. Unlock mutex.\n", ret);
            return;
        }
        printf("pthread create succeed\n");
        return;
    }

    if(len > FRAME_MAX_LEN)
    {
        printf("too big frame !! len=%d\n", len);
        return;
    }

    //save swp
    int get_frm_lock_stat = pthread_mutex_trylock(&MutexGetFrm);
    if(0 != get_frm_lock_stat)
        return ;

    FrameLenSwp = len;
    memcpy(FrameShrSwp, addr, len);

    release_get_frame_mtx();
}


//Golang cgo api
int get_frame_reason(unsigned char **pp, int *len, t_rslts *rslts, int *rst_len)
{
    static unsigned long fid_last = 0;
    hold_get_frame_mtx();

    //no frame result
    if(0 == FrameLen)
    {
        release_get_frame_mtx();
        return RS_STAT_GIVEUP;
    }

    //Frame don't changed
    if(fid_last == Fid)
    {
        release_get_frame_mtx();
        return RS_STAT_GIVEUP;
    }

    //send addr may cause Gin get error data(data can be changed during processing), but it will not crash, try it....
    *pp = FrameShr;
    *len = FrameLen;
    *rst_len = RstLen;
    memcpy((*rslts).results, Rst, sizeof(t_result) * RstLen);

    //printf("new fid: %ld\n", Fid);

    fid_last = Fid;

    //release_get_frame_mtx();

    return RS_STAT_SUCC;
}

//get share frame finished, release lock
void get_frame_reason_finish(void)
{
    release_get_frame_mtx();
}

int cgo_get_x1(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].x1;
}
int cgo_get_y1(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].y1;
}
int cgo_get_x2(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].x2;
}
int cgo_get_y2(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].y2;
}
int cgo_get_label(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].label;
}
int cgo_get_id(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].id;
}
float cgo_get_score(t_rslts *rslts, unsigned int index)
{
    return rslts->results[index].score;
}
