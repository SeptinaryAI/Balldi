/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "VL53L1X.h"
#include "tof_app.h"
#include "tof_shm.h"

#define TOF_SUCC    0
#define TOF_ERR     1

VL53L1X Distance_Sensor;

// use by drv_server

int init_tof(void)
{
    bool ret = Distance_Sensor.begin();

    if(true != ret)
    {
        std::cout << "tof init failed." << endl;
        return TOF_ERR;
    }
    return TOF_SUCC;
}

int close_tof(void)
{
    Distance_Sensor.softReset();
    return TOF_SUCC;
}

int tof_get_distance(void)
{
    // don't care about init status, just read
    Distance_Sensor.startMeasurement();

    while(false == Distance_Sensor.newDataReady())
        usleep(5);

    return Distance_Sensor.getDistance();
}

int clean_shared_ptr(void)
{
    if(-1 + APP_SHM_ADDR_TOF == (size_t)get_tof_shm_addr())
    {
        std::cout << "shm app ptr not ready, don't need clean" << endl;
        return RET_ERROR;
    }

    memset((void *)get_tof_shm_addr(), '\0', TOF_SHARED_DATE_LEN);
    std::cout << "shared_ptr memory and ptr cleared\n" << endl;

    return RET_SUCC;
}

void sig_handle(int no)
{
    switch(no)
    {
        case SIGKILL:
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            clean_shared_ptr();
			close_shm();
			close_tof();
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

int main()
{
    init_sig();
    if(init_tof())  goto main_error_ret;
    if(init_shm())  goto main_error_tof;

    for(;;)
    {
        uint32_t dist = (uint32_t)tof_get_distance();
        *((uint32_t *)get_tof_shm_addr()) = dist;
        usleep(TOF_LOOP_PERIOD_US);
    }

    clean_shared_ptr();
    close_shm();
main_error_tof:
    close_tof();
main_error_ret:
    return 0;
}

