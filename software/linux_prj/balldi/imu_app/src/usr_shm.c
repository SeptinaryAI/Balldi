/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "usr_api.h"
#include "usr_shm.h"

char *g_shm_app_ptr = NULL;
int g_shm_app_id = -1;


int shm_init(void)
{
	//int sys_ret = system("mkdir "SHM_APP_FTOK_FILE" -p");
	//printf("make dir: "SHM_APP_FTOK_FILE", ret %d\n", sys_ret);

	key_t key = ftok(SHM_APP_FTOK_FILE, SHM_APP_ID);
	if( key < 0 )
    {
        perror("ftok");
        return MPU_ERR;
    }
	g_shm_app_id = shmget(key, SHM_LEN, 0666);
	if( g_shm_app_id < 0 )
    {
        perror("shmget");
        return MPU_ERR;
    }

	g_shm_app_ptr = shmat(g_shm_app_id, 0, 0);
    if( g_shm_app_ptr < 0 )
    {
        perror("shmat");
        g_shm_app_id = -1;
        return MPU_ERR;
    }
	printf("mpu6050: shm init succeed! shm id = %d\n", g_shm_app_id);
	return MPU_SUCC;
}

void shm_close(void)
{
    g_shm_app_id = -1;
    int ret = shmdt(g_shm_app_ptr);
    if(ret)
    {
        printf("close shared addr failed, return %d\n", ret);
        return ;
    }
    printf("shared addr closed\n");
    //shared memory will deleted by drv_server, so don't deal with it here
}

static inline void set_share_content(unsigned char *buf, unsigned int addr, unsigned int size)
{
	memcpy(g_shm_app_ptr + addr, buf, size);
}

#if 0
void set_shm_pos(float pitch, float roll, float yaw)
{
	if(UNLIKELY(NULL == g_shm_app_ptr))
	{
		printf("share mem ptr is NULL.\n");
		return ;
	}
	static int unit_size = sizeof(float);

	set_share_content((uint8_t *)&pitch , 0             , unit_size);
	set_share_content((uint8_t *)&roll  , unit_size     , unit_size);
	set_share_content((uint8_t *)&yaw   , unit_size * 2 , unit_size);
}
#endif

void set_shm_dat(float pos[3], float gyro[3], float accel[3], float gyro_acc[3])
{
	const int unit_size = sizeof(float);

	if(UNLIKELY(NULL == g_shm_app_ptr))
	{
		printf("share mem ptr is NULL.\n");
		return ;
	}

    set_share_content((uint8_t *)pos        , 0             , unit_size * 3);
    set_share_content((uint8_t *)gyro       , unit_size * 3 , unit_size * 3);
    set_share_content((uint8_t *)accel      , unit_size * 6 , unit_size * 3);
    set_share_content((uint8_t *)gyro_acc   , unit_size * 9 , unit_size * 3);
}