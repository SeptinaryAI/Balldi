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
#include "led_shm.h"
#include "led_app.h"

char *g_shm_app_ptr = NULL;
int g_shm_app_id = -1;

int init_shm(void)
{
	key_t key = ftok(SHM_APP_FTOK_FILE, SHM_APP_ID);
	if( key < 0 )
    {
        perror("ftok");
        return RET_ERROR;
    }
	g_shm_app_id = shmget(key, SHM_LEN, 0666);
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

	printf("led_app: shm init succeed! shm id = %d\n", g_shm_app_id);
	return RET_SUCC;
}

void close_shm(void)
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

size_t get_led_shm_addr(void)
{
    return (size_t)g_shm_app_ptr + APP_SHM_ADDR_LED;
}
