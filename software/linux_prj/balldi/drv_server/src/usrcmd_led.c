/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "dbg.h"
#include "msg_serv.h"
#include "usrcmd_led.h"

int ctl_led1_color(int color)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[0].color = (unsigned int)(color & 0xFFFFFF);

    return RET_SUCCEED;
}

int ctl_led2_color(int color)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[1].color = (unsigned int)(color & 0xFFFFFF);

    return RET_SUCCEED;
}

int ctl_led1_brightness(int level)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[0].brightness = (unsigned int)(level);

    return RET_SUCCEED;
}

int ctl_led2_brightness(int level)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[1].brightness = (unsigned int)(level);

    return RET_SUCCEED;
}

int ctl_led1_mode(int mode)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[0].mode = (unsigned int)(mode);

    return RET_SUCCEED;
}

int ctl_led2_mode(int mode)
{
	if(NULL + APP_SHM_ADDR_LED == get_shm_led_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}

    t_led_shared_data *p = (t_led_shared_data *)get_shm_led_ptr();

    p->data[1].mode = (unsigned int)(mode);

    return RET_SUCCEED;
}
