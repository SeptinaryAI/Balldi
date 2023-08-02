/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include "cam_v4l2.h"
#include "mnnyoloapi.h" //mnn yolov5_lite api header

static pthread_mutex_t MutexOnOff = PTHREAD_MUTEX_INITIALIZER;
static int FdUserNum = 0;
static int InitOk = 0;

static int Fd_video = -1;
static struct v4l2_capability Cap = {};
static t_usrbuf *Usrbuf = NULL;

int Outfile = -1;


#define CHECK_DO(cond, do) \
{ \
    int _ret = (cond); \
    if(0 != _ret) \
    { \
        printf(#cond" failed !, ret %d\n", _ret); \
        do; \
    } \
}

void user_sub_1(void)
{
    FdUserNum -= 1;
    printf("left user num: %d\n", FdUserNum);
}

void user_add_1(void)
{
    FdUserNum += 1;
    printf("now user num: %d\n", FdUserNum);
}

int init_video(void)
{
    Fd_video = open(DEV_VIDEO, O_RDWR);
    if(Fd_video < 0)
    {
        perror("open video0 error");
        return RET_ERR;
    }
    printf("open video fd\n");

    int res = ioctl(Fd_video, VIDIOC_QUERYCAP, &Cap);
    if(-1 == res)
    {
        perror("ioctl Cap");
        return RET_ERR;
    }

    if(Cap.capabilities&V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("capture device!\n");
    }
    else
    {
        printf("not a capture device!\n");
        return RET_ERR;
    }

    struct v4l2_fmtdesc fmt = {};

#ifdef _STREAM_H264_
    char target_fmt[4] = {'H', '2', '6', '4'};
#else
    char target_fmt[4] = {'M', 'J', 'P', 'G'};
#endif

    int is_support_targetfmt = 0;

    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while((res = ioctl(Fd_video, VIDIOC_ENUM_FMT, &fmt)) == 0){
        char c_fmt[4] = {'\0'};
        c_fmt[0] = fmt.pixelformat & 0xff;
        c_fmt[1] = (fmt.pixelformat >> 8 ) & 0xff;
        c_fmt[2] = (fmt.pixelformat >> 16 ) & 0xff;
        c_fmt[3] = (fmt.pixelformat >> 24 ) & 0xff;
        printf("pixformat=%c%c%c%c,description=%s\n", c_fmt[0], c_fmt[1], c_fmt[2], c_fmt[3], fmt.description);

        if( c_fmt[0] == target_fmt[0] &&
            c_fmt[1] == target_fmt[1] &&
            c_fmt[2] == target_fmt[2] &&
            c_fmt[3] == target_fmt[3] )
        {
            is_support_targetfmt = 1;
        }

        fmt.index++;
    }

#ifdef _STREAM_H264_
    if(!is_support_targetfmt)
    {
        printf("camera don't support H264!\n");
        return RET_ERR;
    }
    else
    {
        printf("camera support H264!\n");
    }
#else
    if(!is_support_targetfmt)
    {
        printf("camera don't support MJPG!\n");
        return RET_ERR;
    }
    else
    {
        printf("camera support MJPG!\n");
    }
#endif

    return 0;
}
int close_video(void)
{
    //I'm the last one use fd, so close it
    close(Fd_video);
    Fd_video = -1;

    printf("close video fd\n");

    return 0;
}

int init_fmt(void)
{
    if(FdUserNum > 1)//someone using fd ,must init it
        return 0;

    //set format
    struct v4l2_format format = {};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 352;
    format.fmt.pix.height = 288;

#ifdef _STREAM_H264_
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
#else
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
#endif

    format.fmt.pix.field = V4L2_FIELD_NONE;
    //format.fmt.pix.field = V4L2_FIELD_INTERLACED;

    int res = ioctl(Fd_video, VIDIOC_S_FMT, &format);
    if(res < 0){
        perror("ioctl VIDIOC_S_FMT");
        return RET_ERR;
    }
#ifdef _STREAM_H264_
    printf("H264 fmt set\n");
#else
    printf("JPEG fmt set\n");
#endif
    return 0;
}

int init_fps(void)
{
    if(FdUserNum > 1)//someone using fd ,must init it
        return 0;

    struct v4l2_streamparm parm = {};
    uint32_t target_fps = 30;

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(Fd_video, VIDIOC_G_PARM, &parm);
    printf("get fps: %u\n", parm.parm.output.timeperframe.denominator);

    memset(&parm, '\0', sizeof(struct v4l2_streamparm));

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.output.timeperframe.denominator = target_fps;
    parm.parm.output.timeperframe.numerator = 1;
    ioctl(Fd_video, VIDIOC_S_PARM, &parm);
    printf("set fps: %u\n", target_fps);
}

int init_stream(void)
{
    if(FdUserNum > 1)//someone using fd ,must init it
        return 0;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int res = ioctl(Fd_video, VIDIOC_STREAMON, &type);
    if(res < 0)
    {
        perror("ioctl VIDIOC_STREAMON");
        return RET_ERR;
    }
    printf("stream on\n");
    return 0;
}
int close_stream(void)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int res = ioctl(Fd_video, VIDIOC_STREAMOFF, &type);
    if(res < 0)
    {
        perror("ioctl VIDIOC_STREAMOFF");
        return RET_ERR;
    }
    printf("stream off\n");
    return 0;
}

int init_mmap(void)
{
    Usrbuf = (t_usrbuf *)malloc(sizeof(t_usrbuf) * USR_BUF_SIZE);
    if(NULL == Usrbuf)
    {
        printf("Usrbuf malloc failed\n");
        return RET_ERR;
    }

    struct v4l2_requestbuffers req;

    //set v4l2 memory mmap mode
    req.count = USR_BUF_SIZE;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    int res = ioctl(Fd_video, VIDIOC_REQBUFS, &req);
    if(res < 0)
    {
        perror("ioctl VIDIOC_REQBUFS");
        goto init_mmap_ret_err;
    }

    //mmap kernel v4l2 to userspace
    for(int i = 0; i < USR_BUF_SIZE; ++i)
    {
        struct v4l2_buffer buf = {0};

        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        res = ioctl(Fd_video, VIDIOC_QUERYBUF, &buf);
        if(res < 0)
        {
            perror("ioctl VIDIOC_QUERYBUF");
            goto init_mmap_ret_err;
        }

        Usrbuf[i].start = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, Fd_video, buf.m.offset);
        Usrbuf[i].len = buf.length;

        if(MAP_FAILED == Usrbuf[i].start)
        {
            printf("mmap to userspace failed, buf blk index: %u\n", i);
            goto init_mmap_ret_err;
        }

        //push data first time
        res = ioctl(Fd_video, VIDIOC_QBUF, &buf);
        if(res < 0)
        {
            perror("ioctl VIDIOC_QBUF");
            goto init_mmap_ret_err;
        }
    }

    printf("mmap v4l2\n");
    return 0;

init_mmap_ret_err:

    free(Usrbuf);
    return RET_ERR;
}
int close_mmap()
{
    for(int i = 0; i < USR_BUF_SIZE; ++i)
    {
        if(NULL == Usrbuf)
            break;

        if(NULL == Usrbuf[i].start)
            continue;

        if(munmap(Usrbuf[i].start, Usrbuf[i].len) < 0)
        {
            printf("unmmap failed, buf blk index: %u\n", i);
        }
    }
    free(Usrbuf);
    printf("unmap v4l2\n");
    return 0;
}

void set_use_reasoning(int b)
{
    set_reason_run_flg(b);
}

static int get_use_reasoning(void)
{
    return get_reason_run_flg();
}

int get_frame(unsigned char **pp, int *len)
{
    struct v4l2_buffer buf;
    int res = 0;
    static int frame_cnt = 0;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    //pop queue, get a frame
    res = ioctl(Fd_video, VIDIOC_DQBUF, &buf);
    if(res < 0)
    {
        perror("ioctl VIDIOC_DQBUF");
        return RET_ERR;
    }

    //printf("index:%d\n", buf.index);

    void *start = Usrbuf[buf.index].start;
    int uselen = buf.bytesused;

    //do something here
    //write(Outfile, start, uselen);
    *pp = (unsigned char *)start;
    *len = uselen;
    //printf("%p 0x%X\n", *pp, *len);

    //after get data, push queue
    res = ioctl(Fd_video, VIDIOC_QBUF, &buf);
    if(res < 0)
    {
        perror("ioctl VIDIOC_QBUF");
        return RET_ERR;
    }

    //get 1 frame from $(FRAME_JUMP_STEP) frame
    if(++frame_cnt < FRAME_JUMP_STEP)
    {
        return CAM_STAT_GIVEUP;
    }

    frame_cnt = 0;

    //share frame for yolo lite use
    if(get_use_reasoning())
    {
        set_share_frame((unsigned char *)start, uselen);
    }

    return CAM_STAT_SUCC;
}

int init_v4l2_cam(void)
{
    pthread_mutex_lock(&MutexOnOff);
    if(FdUserNum > 0)
    {
        // someone opened fd, need not init again
        user_add_1();
        pthread_mutex_unlock(&MutexOnOff);
        return 0;
    }

    int ret = 0;
    if(ret = init_video())
    {
        printf("init_video failed !, ret %d\n", ret);
        goto ret_video_fail;
    }
    if(ret = init_fmt())
    {
        printf("init_fmt failed !, ret %d\n", ret);
        goto ret_fmt_fail;
    }
    if(ret = init_fps())
    {
        printf("init_fps failed !, ret %d\n", ret);
        goto ret_fps_fail;
    }
    if(ret = init_mmap())
    {
        printf("init_mmap failed !, ret %d\n", ret);
        goto ret_mmap_fail;
    }
    if(ret = init_stream())
    {
        printf("init_stream failed !, ret %d\n", ret);
        goto ret_stream_fail;
    }

    user_add_1();
    pthread_mutex_unlock(&MutexOnOff);
    return 0;

ret_stream_fail:
    close_stream();

ret_mmap_fail:
    close_mmap();

ret_fps_fail:
ret_fmt_fail:
ret_video_fail:
    close_video();

    pthread_mutex_unlock(&MutexOnOff);
    return RET_ERR;
}
int close_v4l2_cam(void)
{
    pthread_mutex_lock(&MutexOnOff);

    if(FdUserNum > 1)
    {
        user_sub_1();
        pthread_mutex_unlock(&MutexOnOff);
        printf("others using fd, so don't close it");
        return 0;
    }

    CHECK_DO(close_stream(), NULL);
    CHECK_DO(close_mmap()  , NULL);
    CHECK_DO(close_video() , NULL);

    user_sub_1();
    pthread_mutex_unlock(&MutexOnOff);
    return 0;
}

#ifndef _MAKE_SO_

static int dbg_get_frame()
{
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    //pop queue, get a frame
    int res = ioctl(Fd_video, VIDIOC_DQBUF, &buf);
    if(res < 0)
    {
        perror("ioctl VIDIOC_DQBUF");
        return RET_ERR;
    }

    //printf("index:%d\n", buf.index);

    void *start = Usrbuf[buf.index].start;
    int uselen = buf.bytesused;

    //do something here
    //write(Outfile, start, uselen);

    set_share_frame((unsigned char *)start, uselen);

    //after get data, push queue
    res = ioctl(Fd_video, VIDIOC_QBUF, &buf);
    if(res < 0)
    {
        perror("ioctl VIDIOC_QBUF");
        return RET_ERR;
    }

    return 0;
}

int main(void)
{
    CHECK_DO(init_v4l2_cam(), exit(1));

    Outfile = open("./output.h264", O_RDWR | O_CREAT);
    for(int i = 0; i < 15 * 1000; ++i)
    {
        CHECK_DO(dbg_get_frame(), break);
    }
    close(Outfile);

    close_v4l2_cam();

    return 0;
}

#endif
