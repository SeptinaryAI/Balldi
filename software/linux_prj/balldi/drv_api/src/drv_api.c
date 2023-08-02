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
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "msg_serv.h"
#include "drv_api.h"

static int g_shm_drv_id = -1;
static int g_msg_id = -1;

char *g_shm_drv_ptr = NULL;

int init_msg_api(void)
{
    if(g_msg_id != -1)
    {
        printf("msg already inited! will close it before init.\n");
        close_api();
    }

	int sys_ret = system("mkdir "MSG_FTOK_FILE" -p");
	printf("make dir: "MSG_FTOK_FILE", ret %d\n", sys_ret);
    key_t key = ftok(MSG_FTOK_FILE, MSG_ID);
    if(key < 0)
    {
        perror("ftok");
        return RET_ERROR;
    }
	printf("ftok get key = %d\n", key);

    if((g_msg_id = msgget(key, 0666)) < 0)
    {
        printf("msgget failed!\n");
        return RET_ERROR;
    }

    printf("%s succeed! msg id = %d\n", __FUNCTION__, g_msg_id);

    return RET_SUCCEED;
}

void close_msg_api(void)
{
    //msgctl(g_msg_id, IPC_RMID, NULL);
    g_msg_id = -1;

}

int msg_send_cmd(int cmd, int val_send, int *val_ret)
{
    t_msg_send   tmsg_send;
    t_msg_ret    tmsg_ret;
    t_text_send  *pttext_send = (t_text_send *)tmsg_send.mtext;
    t_text_ret   *pttext_ret  = (t_text_ret *)tmsg_ret.mtext;

    int ret = 0;
    int wait_cnt = 0;

    if(g_msg_id < 0)
    {
        printf("found not msg queue!\n");
        return RET_ERROR;
    }

    pttext_send->mtype_ret = (long)CHN_DRV_TO_CTL;
    pttext_send->cmd = cmd;
    pttext_send->val = val_send;
    tmsg_send.mtype  = (long)CHN_DRV_TO_CTL;

    ret = msgsnd(g_msg_id, (void *)&tmsg_send, sizeof(t_text_send), 0);

	if(-1 == ret)
	{
        printf("msgsnd failed!\n");
        *val_ret = -1;
        return RET_ERROR;
	}

    ret = msgrcv(g_msg_id, (void *)&tmsg_ret, sizeof(t_text_ret), CHN_DRV_TO_CTL, 0);

    if(-1 == ret)
    {
        printf("msgrcv failed! wait time: %d\n", wait_cnt);
        *val_ret = -1;
        return RET_ERROR;
    }

    *val_ret = pttext_ret->val;

	//printf("send_cmd ret %d\n", pttext_ret->stat);

    return pttext_ret->stat;
}

#if 0
void $$;
#endif

int init_shm_api(void)
{
	//int sys_ret = system("mkdir "SHM_DRV_FTOK_FILE" -p");
	//printf("make dir: "SHM_DRV_FTOK_FILE", ret %d\n", sys_ret);

	key_t key = ftok(SHM_DRV_FTOK_FILE, SHM_DRV_ID);
	if( key < 0 )
    {
        perror("ftok");
        return RET_ERROR;
    }
	g_shm_drv_id = shmget(key, SHM_LEN, 0666);
	if( g_shm_drv_id < 0 )
    {
        perror("shmget");
        return RET_ERROR;
    }

	g_shm_drv_ptr = shmat(g_shm_drv_id, 0, 0);
    if( g_shm_drv_ptr < 0 )
    {
        perror("shmat");
        g_shm_drv_id = -1;
        return RET_ERROR;
    }
    printf("drv serv: drv shm init succeed! shm id = %d\n", g_shm_drv_id);
	return RET_SUCCEED;
}

int close_shm_api(void)
{
	g_shm_drv_id = -1;
    int ret = shmdt(g_shm_drv_ptr);
    if(ret)
    {
        printf("close drv shared addr failed, return %d\n", ret);
        return RET_ERROR;
    }
    g_shm_drv_ptr = NULL;
    printf("drv shared addr closed\n");
    //shared memory will deleted by drv_server, so don't deal with it here
    return RET_SUCCEED;
}

int shm_send_cmd(int cmd, int val_send, int *val_ret)
{
	t_shm_buf_blk *p_blk = NULL;
	int cnt = 0;

	if(NULL == g_shm_drv_ptr)
	{
		printf("shm ptr is NULL\n");
		return RET_NULL_PTR;
	}

	p_blk = (t_shm_buf_blk *)g_shm_drv_ptr + DRV_BLK_NO;
	memset(p_blk, '\0', sizeof(t_shm_buf_blk));

	p_blk->buf_recv.cmd = cmd;
	p_blk->buf_recv.val = val_send;

	p_blk->buf_ret.flg = 0;
	p_blk->buf_recv.flg = 1;//set send dat ready, drv_serv will deal with it

	while(!p_blk->buf_ret.flg)
	{
		if(++cnt > WAIT_TIME_MAX)
		{
			printf("shm ret data get failed! wait time: %d\n", cnt);
			return RET_ERROR;
		}
		usleep(1000);
	}

	//now return dat ready
	*val_ret = p_blk->buf_ret.val;
	return p_blk->buf_ret.stat;
}

#if 0
void $$;
#endif

int init_api(void)
{
#if (DRV_IPC_CHOOSE == DRV_IPC_SHM)
	return init_shm_api();
#elif (DRV_IPC_CHOOSE == DRV_IPC_MSG)
	return init_msg_api();
#else
	printf("drv api use invalid IPC mode.\n");
	return -1;
#endif
}

void close_api(void)
{
#if (DRV_IPC_CHOOSE == DRV_IPC_SHM)
	close_shm_api();
#elif (DRV_IPC_CHOOSE == DRV_IPC_MSG)
	close_msg_api();
#else
	printf("drv api use invalid IPC mode.\n");
	return ;
#endif
}

int send_cmd(int cmd, int val_send, int *val_ret)
{
#if (DRV_IPC_CHOOSE == DRV_IPC_SHM)
	return shm_send_cmd(cmd, val_send, val_ret);
#elif (DRV_IPC_CHOOSE == DRV_IPC_MSG)
	return msg_send_cmd(cmd, val_send, val_ret);
#else
	printf("drv api use invalid IPC mode.\n");
	return -1;
#endif
}


#ifndef _MAKE_SO_

//for debug , you don't need it if build drv_api lib
int main(void)
{
    init_api();

    int val = 0;
	unsigned long cnt = 0;
    int ret = 0;

	for(;;)
	{
    	ret = send_cmd(USRCMD_GET_POS_PITCH, 0, &val);
		++cnt;
		if(cnt > 2000000000)
		{
			cnt = 2000000000;
    		printf("ret:%d, val:%d. cnt > 2000000000\n", ret, val);
		}
		else
    		printf("ret:%d, val:%d. cnt x %lu\n", ret, val, cnt);

		if(ret)
			break;

		usleep(200000);
	}

    close_api();

    return 0;
}

#endif
