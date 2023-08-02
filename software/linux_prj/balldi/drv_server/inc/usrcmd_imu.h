/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef USRCMD_IMU_H
#define USRCMD_IMU_H

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

#define ADDR_IMU_PITCH      0
#define ADDR_IMU_ROLL       1
#define ADDR_IMU_YAW        2
#define ADDR_IMU_GYRO1      3
#define ADDR_IMU_GYRO2      4
#define ADDR_IMU_GYRO3      5
#define ADDR_IMU_ACCEL1     6
#define ADDR_IMU_ACCEL2     7
#define ADDR_IMU_ACCEL3     8
#define ADDR_IMU_GYRO1_ACC  9
#define ADDR_IMU_GYRO2_ACC  10
#define ADDR_IMU_GYRO3_ACC  11

static inline void get_share_float(unsigned int addr, float *get_float)
{
	*get_float = *(float *)(get_shm_imu_ptr() + addr);
}

/* usr cmd */
int usr_rd_pos_pitch(uint16_t *val_out);
int usr_rd_pos_roll(uint16_t *val_out);
int usr_rd_pos_yaw(uint16_t *val_out);
int usr_rd_gyro1(uint16_t *val_out);
int usr_rd_gyro2(uint16_t *val_out);
int usr_rd_gyro3(uint16_t *val_out);
int usr_rd_accel1(uint16_t *val_out);
int usr_rd_accel2(uint16_t *val_out);
int usr_rd_accel3(uint16_t *val_out);
int usr_rd_gryo1_acc(uint16_t *val_out);
int usr_rd_gryo2_acc(uint16_t *val_out);
int usr_rd_gryo3_acc(uint16_t *val_out);


#endif
