/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef MSG_SERV_H
#define MSG_SERV_H

#define DRV_IPC_MSG         (1)//drv ipc use msg queue
#define DRV_IPC_SHM         (2)//drv ipc use share memmory

#define DRV_IPC_CHOOSE      (DRV_IPC_SHM)

#define MSG_ID              (2)
#define SHM_DRV_ID          (4)
#define SHM_APP_ID          (3)
#define MSG_FTOK_FILE       "/tmp_msg"
#define SHM_DRV_FTOK_FILE   "/tmp_drv_shm"
#define SHM_APP_FTOK_FILE   "/tmp_app_shm"

#define APP_SHM_ADDR_LED    128     //led app will use tmp_app_shm start from APP_SHM_ADDR_LED
#define APP_SHM_ADDR_TOF    256     //tof app will use tmp_app_shm start from APP_SHM_ADDR_TOF

#define SHM_LEN             (1024)

#define BALLDI_SUCC         (0)
#define BALLDI_FAIL         (1)

#define UART_TRY_TIME       (2)

//only two client use msg queue now
#define CHN_DRV_TO_PY       (0)
#define CHN_DRV_TO_CTL      (1)

#pragma pack(1)

//msg
typedef struct _t_text_send
{
    long mtype_ret;//msg type return to client, dif client use dif msg type
    int cmd;
    int val;
} t_text_send;

typedef struct _t_text_ret
{
    long nouse;
    int stat;
    int val;
} t_text_ret;

typedef struct _t_msgbuf
{
    long mtype;
    char mtext[sizeof(t_text_send)];
} t_msg_send;

typedef struct _t_msgbuf_ret
{
    long mtype;
    char mtext[sizeof(t_text_ret)];
} t_msg_ret;


//shm

//only two client use drv shm now
#define DRV_BLK_NUM         (2)

//local shm block num, how mang tasks allow to send cmd together
#define LOC_DRV_BLK_NUM     (2)

#define LOC_WAIT_TIME_MAX   50

/**
ping-pong ...
don't need mutex
LOOP(
    client set cmd and val
    -> set recv flg

    server read recv flg ready
    -> run drv func
    -> set stat and val
    -> set ret flg

    client read ret flg ready
    -> client set cmd and val
    -> set recv flg
    ....
)

 **/

typedef struct _t_shmbuf_recv
{
	int flg;
	int cmd;
	int val;
} t_shmbuf_recv;

typedef struct _t_shmbuf_ret
{
	int flg;
	int stat;
	int val;
} t_shmbuf_ret;

typedef struct _t_shm_buf_blk
{
	t_shmbuf_recv buf_recv;
	t_shmbuf_ret  buf_ret;
}t_shm_buf_blk;


#pragma pack()

typedef int (*pf_cmd_operation)(uint8_t cmd, int val_in, uint16_t *val_out);

void reg_func_cmd_operation(pf_cmd_operation f);

char *get_shm_imu_ptr(void);
char *get_shm_led_ptr(void);
char *get_shm_tof_ptr(void);

int local_send_cmd(uint8_t blk_use, int cmd, int val_send, int *val_ret);

int  init_balldi_msg(void);
void loop_deal_msg(void);
void init_sig(void);


#endif
