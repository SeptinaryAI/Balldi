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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "balldi_uart.h"
#include "balldi_v2.h"
#include "balldi_v3.h"
#include "usrcmd_gpio.h"
#include "msg_serv.h"

char *g_shm_app_ptr = NULL;
char *g_shm_drv_ptr = NULL;
static int g_msg_id = -1;
static int g_shm_app_id = -1;
static int g_shm_drv_id = -1;

static float pitch = 0;
static float roll = 0;
static float yaw = 0;

static int no_cmd_operation(uint8_t cmd, int val_in, uint16_t *val_out)
{
    printf("you need regist function:func_cmd_operation !\n");
    return -1;
}

pf_cmd_operation func_cmd_operation = no_cmd_operation;    //operation function really use

void reg_func_cmd_operation(pf_cmd_operation f)
{
    if(NULL == f)
    {
        printf("can not regist NULL function:cmd_operation !\n");
        return ;
    }
    func_cmd_operation = f;
}


#if 0
void $$;
#endif

char *get_shm_imu_ptr(void)
{
    return g_shm_app_ptr;
}

char *get_shm_led_ptr(void)
{
    return g_shm_app_ptr + APP_SHM_ADDR_LED;
}

char *get_shm_tof_ptr(void)
{
    return g_shm_app_ptr + APP_SHM_ADDR_TOF;
}


int init_balldi_msg(void)
{
    if(g_msg_id >= 0)
    {
        printf("msg already inited!\n");
        return 0;
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

    if((g_msg_id = msgget(key, IPC_CREAT | 0666)) < 0)
    {
        printf("msgget failed!\n");
        return RET_ERROR;
    }

    printf("%s init succeed! msg id = %d\n", __FUNCTION__, g_msg_id);
    return RET_SUCCEED;
}

void close_balldi_msg(void)
{
    if(g_msg_id < 0)
        return ;

    if((msgctl(g_msg_id, IPC_RMID, NULL)) < 0)
    {
        perror("msgctl destroy msg queue failed!\n");
        return ;
    }
    printf("remove msg queue succeed.\n");
}


void loop_deal_msg(void)
{
    ssize_t     ret = 0;
    t_msg_send  tmsg_send;
    t_msg_ret   tmsg_ret;
    t_text_ret  *pttext_ret  = (t_text_ret *)tmsg_ret.mtext;
    t_text_send *pttext_send = (t_text_send *)tmsg_send.mtext;

    uint16_t    val_ret = 0;
    int         try_cnt = 0;

    if(g_msg_id < 0)
    {
        printf("mo msg queue here!\n");
        return;
    }

    for(;;)
    {
		memset(&tmsg_send, '\0', sizeof(t_msg_send));
        ret = msgrcv(g_msg_id, (void *)&tmsg_send, sizeof(t_text_send), (long)0, 0);

        if(ret < 0)
        {
            perror("msgrcv failed!\n");
			printf("msgrcv failed! ret = %d\n", ret);
			break;
        }
        try_cnt = 0;

		printf("mtype: %ld\n", tmsg_send.mtype);
		printf("text: \n");
		for(int i = 0; i < 12; ++i)
			printf("0x%x ",tmsg_send.mtext[i]);
		printf("\n");
		printf("cmd: 0x%x\n", pttext_send->cmd);
		printf("mtype_ret: 0x%lx\n", pttext_send->mtype_ret);
		printf("val: 0x%x\n", pttext_send->val);

		if(pttext_send->mtype_ret != 1 && pttext_send->mtype_ret != 2)
		{
			printf("recv invalid type = %ld. break\n", pttext_send->mtype_ret);
			break;
		}

        ret = func_cmd_operation(pttext_send->cmd, pttext_send->val, &val_ret);

        tmsg_ret.mtype  = pttext_send->mtype_ret;
        pttext_ret->stat = ret;
		printf("loop_deal_msg ret stat %d\n", pttext_ret->stat);
		if(RET_SUCCEED == ret)
        	pttext_ret->val  = (int)val_ret;
		else
        	pttext_ret->val  = -1;

		printf("\n");
		printf("\n");

        ret = msgsnd(g_msg_id, (void *)&tmsg_ret, sizeof(t_text_ret), 0);
		if(ret < 0)
		{
            perror("msgsnd failed!\n");
			printf("msgsnd failed! ret = %d\n", ret);
			break;
		}
    }
}

#if 0
void $$;
#endif

static t_shm_buf_blk LocCmdBlks[LOC_DRV_BLK_NUM] = {{0}};

/**
 * @brief drv_serv local thread use send cmd
 *
 * @param blk_use   block to use, between [1, LOC_DRV_BLK_NUM], *warning: for safe, A local thread can only use one blk num
 * @param cmd       Balldi Uart CMD or User CMD
 * @param val_send  value to send
 * @param val_ret   value recved
 */
int local_send_cmd(uint8_t blk_use, int cmd, int val_send, int *val_ret)
{
    uint32_t cnt = 0;
    if(blk_use > LOC_DRV_BLK_NUM)
    {
        printf("invalid local block num: %u\n", blk_use);
        return RET_OUT_RANGE;
    }
    t_shmbuf_recv *recv = &(LocCmdBlks[blk_use - 1].buf_recv);
    t_shmbuf_ret  *ret = &(LocCmdBlks[blk_use - 1].buf_ret);
    recv->cmd = cmd;
    recv->val = val_send;
    ret->flg = 0;
    recv->flg = 1;
    while(!ret->flg)
	{
		if(++cnt > LOC_WAIT_TIME_MAX)
		{
			printf("local thread ret data get failed! wait time: %d\n", cnt);
			return RET_ERROR;
		}
		usleep(1000);
	}

    *val_ret = ret->val;

    return ret->stat;
}

#if 0
void $$;
#endif

int init_balldi_shm(void)
{
	int sys_ret = system("mkdir "SHM_DRV_FTOK_FILE" -p");
	printf("make dir: "SHM_DRV_FTOK_FILE", ret %d\n", sys_ret);

	key_t key = ftok(SHM_DRV_FTOK_FILE, SHM_DRV_ID);
	if( key < 0 )
    {
        perror("ftok");
        return RET_ERROR;
    }
	g_shm_drv_id = shmget(key, SHM_LEN, IPC_CREAT | 0666);
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

void close_balldi_shm(void)
{
	if(g_shm_drv_id < 0)
        return ;

    if(shmdt(g_shm_drv_ptr) < 0)
    {
        perror("unlink drv shm failed!\n");
        return ;
    }

    if(shmctl(g_shm_drv_id, IPC_RMID, 0) < 0)
    {
        perror("remove drv shm failed!\n");
        return ;
    }

    printf("remove shm succeed.\n");
}

void loop_deal_shm(void)
{
	t_shm_buf_blk *p_blk = NULL;
	int cmd = 0;
	int val = 0;
	int stat_ret = 0;
	uint16_t val_ret = 0;

	if(NULL == g_shm_drv_ptr)
	{
		printf("shm ptr is NULL\n");
		return ;
	}

	memset(g_shm_drv_ptr, '\0', sizeof(t_shm_buf_blk) * DRV_BLK_NUM);

	for(;;)
	{
        //deal with client cmd
		for(int blk_cnt = 0; blk_cnt < DRV_BLK_NUM; ++blk_cnt)
		{
			p_blk = (t_shm_buf_blk *)g_shm_drv_ptr + blk_cnt;
            t_shmbuf_ret *ret = &(p_blk->buf_ret);
            t_shmbuf_recv *recv = &(p_blk->buf_recv);

			if(recv->flg)
			{
				recv->flg = 0;
				cmd = recv->cmd;
				val = recv->val;

				stat_ret = func_cmd_operation(cmd, val, &val_ret);

				if(RET_SUCCEED == stat_ret)
                    ret->val = (int)val_ret;
				else
					ret->val = -1;

				ret->stat = stat_ret;

				ret->flg = 1;
		        usleep(UART_WAIT_US);
			}
		}

        //deal with local cmd
		for(int blk_cnt = 0; blk_cnt < LOC_DRV_BLK_NUM; ++blk_cnt)
		{
            p_blk = LocCmdBlks + blk_cnt;
            t_shmbuf_ret *ret = &(p_blk->buf_ret);
            t_shmbuf_recv *recv = &(p_blk->buf_recv);

			if(recv->flg)
			{
				recv->flg = 0;
				cmd = recv->cmd;
				val = recv->val;

				stat_ret = func_cmd_operation(cmd, val, &val_ret);

				if(RET_SUCCEED == stat_ret)
                    ret->val = (int)val_ret;
				else
					ret->val = -1;

				ret->stat = stat_ret;

				ret->flg = 1;
		        usleep(UART_WAIT_US);
			}
        }
	}
}

#if 0
void $$;
#endif

int init_mpu6050_shm(void)
{
	int sys_ret = system("mkdir "SHM_APP_FTOK_FILE" -p");
	printf("make dir: "SHM_APP_FTOK_FILE", ret %d\n", sys_ret);

	key_t key = ftok(SHM_APP_FTOK_FILE, SHM_APP_ID);
	if( key < 0 )
    {
        perror("ftok");
        return RET_ERROR;
    }
	g_shm_app_id = shmget(key, SHM_LEN, IPC_CREAT | 0666);
	if( g_shm_app_id < 0 )
    {
        perror("shmget");
        return RET_ERROR;
    }

	g_shm_app_ptr = shmat(g_shm_app_id, 0, 0);
    if( g_shm_app_ptr < 0 )
    {
        perror("shmat");
        g_shm_app_id = -1;
        return RET_ERROR;
    }
    printf("drv serv: mpu6050 shm init succeed! shm id = %d\n", g_shm_app_id);
	return RET_SUCCEED;
}

void close_mpu6050_shm(void)
{
	if(g_shm_app_id < 0)
        return ;

    if(shmdt(g_shm_app_ptr) < 0)
    {
        perror("unlink mpu6050 shm failed!\n");
        return ;
    }

    if(shmctl(g_shm_app_id, IPC_RMID, 0) < 0)
    {
        perror("remove mpu6050 shm failed!\n");
        return ;
    }

    printf("delete shm succeed.\n");
}

#if 0
void $$;
#endif

//sig 2
void sig_handle(int no)
{
    switch(no)
    {
        case SIGKILL:
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            close_uart_fd();
#if (DRV_IPC_CHOOSE == DRV_IPC_SHM)
			close_balldi_shm();
#elif (DRV_IPC_CHOOSE == DRV_IPC_MSG)
            close_balldi_msg();
#endif
			close_mpu6050_shm();
            break;
        default:
            break;
    }

    exit(0);
}


void init_sig(void)
{
    signal(SIGKILL, sig_handle);
    signal(SIGQUIT, sig_handle);
    signal(SIGINT,  sig_handle);
    signal(SIGTERM, sig_handle);
}


#if 0
void $$;
#endif

int main()
{
#if (DRV_IPC_CHOOSE == DRV_IPC_SHM)
    //reg_func_cmd_operation(cmd_operation_v2);
    reg_func_cmd_operation(cmd_operation_v3);
	init_balldi_shm();
	init_uart();
	init_sig();
	init_mpu6050_shm();
    init_gpio();
	loop_deal_shm();
    close_uart_fd();
	close_balldi_shm();
	close_mpu6050_shm();
#elif (DRV_IPC_CHOOSE == DRV_IPC_MSG)
    //reg_func_cmd_operation(cmd_operation_v2);
    reg_func_cmd_operation(cmd_operation_v3);
	init_balldi_msg();
	init_uart();
	init_sig();
	init_mpu6050_shm();
    init_gpio();
	loop_deal_msg();
    close_uart_fd();
	close_balldi_msg();
	close_mpu6050_shm();
#endif
}

