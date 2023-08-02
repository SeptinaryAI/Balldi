/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef LED_API_H
#define LED_API_H

#define LED_SUCC 0
#define LED_ERR  1

#define RGB_SPI_DEV "/dev/spidev0.0"
#define RGB_SPI_SPEED 2900000 //ws2812b timing
#define SPI_HIGH 0
#define SPI_LOW  1
#define LED_NUM  2

#define RESET_BUF_LEN   70

#define LED_BRIGHTNESS_MAX  10000
#define LED_COUNT_PERIOD    400
#define LED_LOOP_PERIOD_US  10000
#define LED_FLASH_PERIOD_US 200000

#define LED_MODE_NORMAL     0
#define LED_MODE_BREATH     1
#define LED_MODE_FLASH      2

#define RET_ERROR 1
#define RET_SUCC  0

typedef struct
{
    unsigned int color;
    unsigned int brightness;
    unsigned int mode;
} t_led_set_data;

typedef struct
{
    t_led_set_data data[LED_NUM];
} t_led_shared_data;


#define CHECK_RET(dosomething) \
{ \
	int ret = dosomething; \
	if(ret != RET_SUCC) \
	{ \
		printf(#dosomething" failed. ret %d\n", ret); \
		return RET_ERROR; \
	} \
}

#endif
