/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef USR_API_H
#define USR_API_H

#define MPU_SUCC 0
#define MPU_ERR  1

#define MPU6050_DEV 				"/dev/i2c-0"
#define MPU6050_SLAVE_ADDR  		0x69
#define MPU6050_SLAVE_ADDR_DEFAULT  0x68

#define USR_SET_RATE				100

#define REG_ACCEL_BASE      (0x3B)
#define REG_GYRO_BASE       (0x43)

#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

#define MAKE_16B(u8_l, u8_h) ( (int16_t)( ((uint16_t)(u8_h) << 8) | (uint16_t)(u8_l) ) )

#define CHECK_RET(dosomething) \
{ \
	int ret = dosomething; \
	if(ret != MPU_SUCC) \
	{ \
		printf(#dosomething" failed. ret %d\n", ret); \
		return MPU_ERR; \
	} \
}

#endif
