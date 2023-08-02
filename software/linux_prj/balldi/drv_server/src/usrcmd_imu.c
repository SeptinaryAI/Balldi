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
#include "usrcmd_imu.h"

int usr_rd_pos_pitch(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float pitch = 0;

    get_share_float(sizeof(float) * ADDR_IMU_PITCH, &pitch);
	DBG_F("pitch: %+5.1f  ", pitch);

	if(pitch < -90.0 || pitch > 90.0)
	{
		printf("pitch val out of range.\n");
		return RET_OUT_RANGE;
	}

	*val_out = (uint16_t)((int16_t)(pitch * 10));

	return RET_SUCCEED;
}

int usr_rd_pos_roll(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float roll = 0;

    get_share_float(sizeof(float) * ADDR_IMU_ROLL, &roll);
	DBG_F("roll: %+5.1f  ", roll);

	if(roll < -180.0 || roll > 180.0)
	{
		printf("roll val out of range.\n");
		return RET_OUT_RANGE;
	}

	*val_out = (uint16_t)((int16_t)(roll * 10));

	return RET_SUCCEED;
}

int usr_rd_pos_yaw(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float yaw = 0;

    get_share_float(sizeof(float) * ADDR_IMU_YAW, &yaw);
	DBG_F("yaw: %+5.1f  \n", yaw);

	if(yaw < -180.0 || yaw > 180.0)
	{
		printf("yaw val out of range.\n");
		return RET_OUT_RANGE;
	}

	*val_out = (uint16_t)((int16_t)(yaw * 10));

	return RET_SUCCEED;
}

int usr_rd_gyro1(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float gyro = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO1, &gyro);
	printf("gyro1: %.1f\n", gyro);

	*val_out = (uint16_t)((int16_t)(gyro * 10));

	return RET_SUCCEED;
}
int usr_rd_gyro2(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float gyro = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO2, &gyro);
	//printf("gyro2: %.1f\n", gyro);

	*val_out = (uint16_t)((int16_t)(gyro * 10));

	return RET_SUCCEED;
}
int usr_rd_gyro3(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float gyro = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO3, &gyro);
	//printf("gyro3: %.1f\n", gyro);

	*val_out = (uint16_t)((int16_t)(gyro * 10));

	return RET_SUCCEED;
}
int usr_rd_accel1(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float accel = 0;

    get_share_float(sizeof(float) * ADDR_IMU_ACCEL1, &accel);
	//printf("accel1: %.1f\n", accel);

	*val_out = (uint16_t)((int16_t)(accel * 10));

	return RET_SUCCEED;
}
int usr_rd_accel2(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float accel = 0;

    get_share_float(sizeof(float) * ADDR_IMU_ACCEL2, &accel);
	//printf("accel2: %.1f\n", accel);

	*val_out = (uint16_t)((int16_t)(accel * 10));

	return RET_SUCCEED;
}
int usr_rd_accel3(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float accel = 0;

    get_share_float(sizeof(float) * ADDR_IMU_ACCEL3, &accel);
	//printf("accel3: %.1f\n", accel);

	*val_out = (uint16_t)((int16_t)(accel * 10));

	return RET_SUCCEED;
}

int usr_rd_gryo1_acc(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float acc = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO1_ACC, &acc);
	//printf("gyro1 acc: %.1f\n", acc);

	*val_out = (uint16_t)((int16_t)(acc * 10));

	return RET_SUCCEED;
}

int usr_rd_gryo2_acc(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float acc = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO2_ACC, &acc);
	//printf("gyro1 acc: %.1f\n", acc);

	*val_out = (uint16_t)((int16_t)(acc * 10));

	return RET_SUCCEED;
}

int usr_rd_gryo3_acc(uint16_t *val_out)
{
	if(NULL == get_shm_imu_ptr())
	{
		printf("share mem ptr is NULL.\n");
		return RET_NULL_PTR;
	}
	float acc = 0;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO3_ACC, &acc);
	//printf("gyro1 acc: %.1f\n", acc);

	*val_out = (uint16_t)((int16_t)(acc * 10));

	return RET_SUCCEED;
}
