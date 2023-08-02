/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef USRCMD_GPIO_H
#define USRCMD_GPIO_H

/* RETURN ERROR TYPE */
#define RET_SUCCEED         0
#define RET_ERROR           1
#define RET_INVALID_ENCODE  2
#define RET_INVALID_CMD     3
#define RET_INVALID_VALUE   4
#define RET_INVALID_START   5
#define RET_OUT_RANGE       6
#define RET_NULL_PTR        7
#define RET_ERROR_LEN       8
#define RET_SUCCEED_REPLY   10
#define RET_THREAD_OVER     11

#define SYS_GPIO                "/sys/class/gpio"
#define SYS_GPIO_LINEOUT_SW     "/sys/class/gpio/gpio203"
#define SYS_GPIO_STM32_RST      "/sys/class/gpio/gpio201"
#define CMD_EXPORT_GPIO_LINEOUT_SW  "echo 203 > /sys/class/gpio/export"
#define CMD_EXPORT_GPIO_STM32_RST   "echo 201 > /sys/class/gpio/export"

#define LINEOUT_ON              "0"
#define LINEOUT_OFF             "1"

#define STM32_RST_RST           "0"
#define STM32_RST_WORK          "1"

void init_gpio(void);

int usr_set_lineout_onoff(int val);
int usr_set_driver_board_rst(int rst);

#endif

