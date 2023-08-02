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
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "dbg.h"
#include "usrcmd_gpio.h"

static pthread_mutex_t MutexDrvBdRst = PTHREAD_MUTEX_INITIALIZER;
static int FlgStm32Wrong = 0;


int usr_set_lineout_onoff(int val)
{
    int fd = open(SYS_GPIO_LINEOUT_SW"/value", O_WRONLY);
    if(fd < 0)
    {
        printf("open GPIO ot lineout switch value failed.\n");
        return RET_ERROR;
    }

    if(val)
        write(fd, LINEOUT_ON, sizeof(LINEOUT_ON));
    else
        write(fd, LINEOUT_OFF, sizeof(LINEOUT_OFF));

    close(fd);

    return RET_SUCCEED;
}

static void set_flg_driver_board_wrong(void)
{
    FlgStm32Wrong = 1;
}

static int rst_driver_board(void)
{
    int fd = open(SYS_GPIO_STM32_RST"/value", O_WRONLY);
    if(fd < 0)
    {
        printf("open GPIO ot stm32 driver board NRST value failed.\n");
        return RET_ERROR;
    }

    write(fd, STM32_RST_RST, sizeof(STM32_RST_RST));
    usleep(10000);   //stm32 NRST pull down at least 1ms
    write(fd, STM32_RST_WORK, sizeof(STM32_RST_WORK));

    close(fd);

    return RET_SUCCEED;
}

static void* thread_driver_board_rst(void *args)
{
    const unsigned int delay = 200 * 1000;  // 100ms = 200 * 1000us

    for(;;)
    {
        if(FlgStm32Wrong)
        {
            rst_driver_board();
            usleep(delay);      //wait xx us after reset, usr cmd will be recovered
            FlgStm32Wrong = 0;  //clear flag
        }
        else
        {
            usleep(delay);
        }
    }
}

int usr_set_driver_board_rst(int rst)
{
    if(!rst)
        return RET_INVALID_VALUE;

    set_flg_driver_board_wrong();

    int lock_stat = pthread_mutex_trylock(&MutexDrvBdRst);
    if(0 != lock_stat)
    {
        //I need only an instance of driver board rst
        DBG_F("pthread pass>>>>>\n");
        return RET_SUCCEED;
    }
    DBG_F("pthread create>>>>>\n");

    pthread_t p;
    int ret = pthread_create(&p, NULL, thread_driver_board_rst, NULL);
    if(0 != ret)
    {
        pthread_mutex_unlock(&MutexDrvBdRst);
        printf("pthread create failed, ret = %d, unlock mutex.\n", ret);
    }

    return RET_SUCCEED;
}

void init_gpio(void)
{
    //export gpios I need
    if(access(SYS_GPIO_LINEOUT_SW, F_OK))
    {
        system(CMD_EXPORT_GPIO_LINEOUT_SW);
        usleep(1000);
    }
    if(access(SYS_GPIO_STM32_RST, F_OK))
    {
        system(CMD_EXPORT_GPIO_STM32_RST);
        usleep(1000);
    }

    //init gpio direction (only use ouput now
    int fd = open(SYS_GPIO_LINEOUT_SW"/direction", O_WRONLY);
    if(fd < 0)
    {
        printf("open GPIO ot lineout switch direction failed.\n");
        return ;
    }
    write(fd, "out", sizeof("out"));
    close(fd);

    fd = open(SYS_GPIO_STM32_RST"/direction", O_WRONLY);
    if(fd < 0)
    {
        printf("open GPIO ot stm32 driver board NRST direction failed.\n");
        return ;
    }
    write(fd, "out", sizeof("out"));
    close(fd);

    //init gpio val
    usr_set_lineout_onoff(1);
    rst_driver_board();
}

