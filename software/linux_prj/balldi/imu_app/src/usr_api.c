/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include "usr_api.h"
#include "usr_shm.h"

#define IF_FIRST_TIME_CALC \
    static int flg = 0; \
    if(UNLIKELY( (0 == flg) && (++flg) ))

static int g_i2c_inited = 0;
static int fd_mpu6050 = -1;
static double pi = 3.141592653;

int is_mpu6050_inited(void)
{
	return g_i2c_inited;
}

int i2c_init(void)
{
	fd_mpu6050 ;
	if( -1 == (fd_mpu6050 = open(MPU6050_DEV, O_RDWR)) )
	{
		printf("mpu6050 i2c: %s init failed .\n", MPU6050_DEV);
		return MPU_ERR;
	}

	if( ioctl(fd_mpu6050, I2C_SLAVE_FORCE, MPU6050_SLAVE_ADDR) < 0 )
	{
		printf("mpu6050 i2c set slave address failed .\n");
		return MPU_ERR;
	}

	g_i2c_inited = 1;
	return MPU_SUCC;
}

#if 0
void $$;
#endif

int i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data)
{
	if( UNLIKELY(!is_mpu6050_inited()) )
	{
		i2c_init();
	}
	return i2c_smbus_write_i2c_block_data(fd_mpu6050, reg_addr, length, data);
}

int i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data)
{
	static int ret = 0;
	if( UNLIKELY(!is_mpu6050_inited()) )
	{
		i2c_init();
	}
	if( length != (ret = i2c_smbus_read_i2c_block_data(fd_mpu6050, reg_addr, length, data)) )
	{
		printf("mpu6050 i2c read failed , ret %d\n", ret);
		return MPU_ERR;
	}

	return MPU_SUCC;
}

void delay_ms(unsigned long num_ms)
{
	usleep(num_ms*1000);
#if 0
	struct timeval tv;
	tv.tv_sec = (num_ms * 1000) / 1000000;
	tv.tv_usec = (num_ms * 1000) % 1000000;

	int err = 0;
	do{
		err = select(0, NULL, NULL, NULL, &tv);
	} while(err < 0 && errno == EINTR);
#endif
}

void get_ms(unsigned long *count)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	*count = tp.tv_sec * 1000 + tp.tv_nsec / (1000000);
}

#if 0
void $$;
#endif

int i2c_write_byte(unsigned char cmd, unsigned char val)
{
	if( UNLIKELY(!is_mpu6050_inited()) )
	{
		i2c_init();
	}
	return i2c_smbus_write_byte_data(fd_mpu6050, cmd, val);
}

unsigned char i2c_read_byte(unsigned char cmd)
{
	if( UNLIKELY(!is_mpu6050_inited()) )
	{
		i2c_init();
	}
	return i2c_smbus_read_byte_data(fd_mpu6050, cmd);
}

#if 0
void $$;
#endif

#define GYRO_FSR 	1
#define ACCEL_FSR 	0
#define GA 			9.8			//Gravitational acceleration

//+- x dps
static float gyro_dps_map[4] = {
	250.0,
	500.0,
	1000.0,
	2000.0,
};

//+- x g
static float accel_g_map[4] = {
	2.0,
	4.0,
	8.0,
	16.0,
};

//para about hardware
//you can set static offset of origin data  here
static int16_t gyro_offset_map[3] = {
	-175,
	-174,
	-167,
};

static int16_t accel_offset_map[3] = {
	911,
	419,
	-155,
};


int mpu6050_init()
{
	if( UNLIKELY(!is_mpu6050_inited()) )
	{
		printf("i2c not inited, in %s", __FUNCTION__);
		return MPU_ERR;
	}

	usleep(100000);
	CHECK_RET(i2c_write_byte(0x6B, 0x80));		//rst mpu6050
	usleep(100000);
	CHECK_RET(i2c_write_byte(0x6B, 0x00));		//rst mpu6050

	unsigned char devid = i2c_read_byte(0x75);
	if(MPU6050_SLAVE_ADDR != devid && MPU6050_SLAVE_ADDR_DEFAULT != devid)
	{
		printf("error dev id : %u, exit.\n", devid);
		return MPU_ERR;
	}
	printf("dev id : %u.\n", devid);

	CHECK_RET(i2c_write_byte(0x1B, (GYRO_FSR) << 3));	//gyro fsr , 0:+-250dps 1:+-500dps 2:+-1000dps 3:+-2000dps
	CHECK_RET(i2c_write_byte(0x1C, (ACCEL_FSR) << 3));	//accel fsr , 0:+-2g 1:+-4g 2:+-8g 3:+-16g

	CHECK_RET(i2c_write_byte(0x38, 0x00));		//int en: disable
	CHECK_RET(i2c_write_byte(0x6A, 0x00));		//user ctrl: disable
	CHECK_RET(i2c_write_byte(0x23, 0x00));		//fifo: disable
	CHECK_RET(i2c_write_byte(0x37, 0x80));		//bypass

	CHECK_RET(i2c_write_byte(0x6B, 0x00));		//clksel pll x-axis
	CHECK_RET(i2c_write_byte(0x6C, 0x00));		//gyro acel work

	CHECK_RET(i2c_write_byte(0x19, (unsigned char)(1000 / (USR_SET_RATE) - 1)) );	//sample rate , 4 ~ 1000
	//CHECK_RET(i2c_write_byte(0x1A, 3));			//low pass

	return MPU_SUCC;
}

int mpu6050_origin_data(int16_t accel[], int16_t gyro[])
{
	unsigned char buf[6];
	if(i2c_read(0, REG_ACCEL_BASE, 6, buf))
		return MPU_ERR;

    accel[0] = MAKE_16B(buf[1], buf[0]) + accel_offset_map[0];
    accel[1] = MAKE_16B(buf[3], buf[2]) + accel_offset_map[1];
    accel[2] = MAKE_16B(buf[5], buf[4]) + accel_offset_map[2];

	if(i2c_read(0, REG_GYRO_BASE, 6, buf))
		return MPU_ERR;

    gyro[0] = MAKE_16B(buf[1], buf[0]) + gyro_offset_map[0];
    gyro[1] = MAKE_16B(buf[3], buf[2]) + gyro_offset_map[1];
    gyro[2] = MAKE_16B(buf[5], buf[4]) + gyro_offset_map[2];

	return MPU_SUCC;
}

//common result
static double g_pitch = 0.0;
static double g_roll  = 0.0;
static double g_yaw = 0.0;
//calced by accel
static double g_pitch_a = 0.0;
static double g_roll_a  = 0.0;
//calced by gyro
static double g_pitch_g = 0.0;
static double g_roll_g  = 0.0;
static double g_yaw_g  = 0.0;
//change of
static double g_delta_pitch = 0.0;
static double g_delta_roll  = 0.0;
static double g_delta_yaw  = 0.0;


/**
 * @brief
 *
 * @param [IN]  ori_accel
 * @param [IN]  ori_gyro
 * @param [OUT] accel
 * @param [OUT] gyro
 * @return int
 */
int mpu6050_switch_data(int16_t ori_accel[], int16_t ori_gyro[], float accel[], float gyro[], double *pitch, double *roll, double *yaw)
{
	if(mpu6050_origin_data(ori_accel, ori_gyro))
		return MPU_ERR;

    double gyro_r[3];

	gyro[0]  = ori_gyro[0] * gyro_dps_map[GYRO_FSR] / 0x8000;
	gyro[1]  = ori_gyro[1] * gyro_dps_map[GYRO_FSR] / 0x8000;
	gyro[2]  = ori_gyro[2] * gyro_dps_map[GYRO_FSR] / 0x8000;

	accel[0] = ori_accel[0] * accel_g_map[ACCEL_FSR] * GA / 0x8000;
	accel[1] = ori_accel[1] * accel_g_map[ACCEL_FSR] * GA / 0x8000;
	accel[2] = ori_accel[2] * accel_g_map[ACCEL_FSR] * GA / 0x8000;

    // rad
    gyro_r[0] = (double)gyro[0] * pi / 180;
    gyro_r[1] = (double)gyro[1] * pi / 180;
    gyro_r[2] = (double)gyro[2] * pi / 180;

    //static int cnt0 = 0;
    //if(++cnt0 > 5)
    //{
    //    //printf("gyro:%+8.2f %+8.2f %+8.2f  accel:%+8.2f %+8.2f %+8.2f \n", gyro_r[0], gyro_r[1], gyro_r[2], accel[0], accel[1], accel[2]);
    //    //printf("gyro:%+8.2f %+8.2f %+8.2f  gyro_r:%+8.2f %+8.2f %+8.2f \n", gyro[0], gyro[1], gyro[2], gyro_r[0], gyro_r[1], gyro_r[2]);
    //    cnt0 = 0;
    //}

    //use accel
    //roll = arctan(ay / az)
    //pitch = -arctan(ax / √(ay^ay + az^az))
	g_roll_a  = atan(accel[1]/accel[2]);
	g_pitch_a = -atan( accel[0] / sqrt(accel[1] * accel[1] + accel[2] * accel[2] ) );

    IF_FIRST_TIME_CALC
    {
        //use the static result calced by accel to init
        g_roll  = g_roll_a;
        g_pitch = g_pitch_a;
        g_yaw   = 0; //init yaw
    }

    double sin_p = sin(g_delta_pitch / USR_SET_RATE);
    double sin_r = sin(g_delta_roll  / USR_SET_RATE);
    double cos_p = cos(g_delta_pitch / USR_SET_RATE);
    double cos_r = cos(g_delta_roll  / USR_SET_RATE);

    //use gyro
    // | Δroll   |    | 1     sinp * sinr / cosp     cosr * sinp / cosp    |     |gx |
    // | Δpitch  |  = | 0     cosr                   -sinr                 |  ·  |gy |
    // | Δyaw    |    | 0     sinr / cosp            cosr / cosp           |     |gz |
    g_delta_roll  = gyro_r[0] + gyro_r[1] * sin_p * sin_r / cos_p + gyro_r[2] * cos_r * sin_p / cos_p;
    g_delta_pitch = gyro_r[1] * cos_r + gyro_r[2] * (-sin_r);
    g_delta_yaw   = gyro_r[1] * sin_r / cos_p + gyro_r[2] * cos_r / cos_p;

    //Rx+1 = Rx + ΔR = Rx + dr/dt * Δt
    g_roll_g  = g_roll  + g_delta_roll / USR_SET_RATE;
    g_pitch_g = g_pitch + g_delta_pitch / USR_SET_RATE;
    g_yaw_g   = g_yaw   + g_delta_yaw / USR_SET_RATE;

    //common result by kalman filtering
    //TODO:
    //now use a simple way
    g_roll  += (g_roll_a  - g_roll_g)  * 0.3;
    g_pitch += (g_pitch_a - g_pitch_g) * 0.3;
    g_yaw    = g_yaw_g;

    while(g_yaw > pi)
    {
        g_yaw -= pi;
    }
    while(g_yaw < -pi)
    {
        g_yaw += pi;
    }

    //dbg
    //    printf("p(a g c):%+7.2f %+7.2f %+7.2f  r(a g c):%+7.2f %+7.2f %+7.2f  y:%+7.2f gyro:%+7.2f %+7.2f %+7.2f gyro_r:%+7.2f %+7.2f %+7.2f sp:%+7.2f sr:%+7.2f cp:%+7.2f cr:%+7.2f delta:%+7.2f %+7.2f %+7.2f\n",
    //            g_pitch_a, g_pitch_g, g_pitch,
    //            g_roll_a, g_roll_g, g_roll,
    //            g_yaw,
    //            gyro[0], gyro[1], gyro[2],
    //            gyro_r[0], gyro_r[1], gyro_r[2],
    //            sin_p,sin_r,cos_p,cos_r,
    //            g_delta_roll, g_delta_pitch, g_delta_yaw
    //            );


    *pitch = g_pitch;
    *roll  = g_roll;
    *yaw   = g_yaw;

	return MPU_SUCC;
}


static float g_last_gyro[3] = {0.0, 0.0, 0.0};

/**
 * @brief
 *
 * @param gyro [IN]
 * @param gyro_acc [OUT]
 * @return int
 */
int mpu6050_calc_gyro_acc(float gyro[3], float gyro_acc[3])
{
    gyro_acc[0] = (gyro[0] - g_last_gyro[0]) / USR_SET_RATE;
    gyro_acc[1] = (gyro[1] - g_last_gyro[1]) / USR_SET_RATE;
    gyro_acc[2] = (gyro[2] - g_last_gyro[2]) / USR_SET_RATE;

    g_last_gyro[0] = gyro[0];
    g_last_gyro[1] = gyro[1];
    g_last_gyro[2] = gyro[2];

	return MPU_SUCC;
}

#if 0
void $$;
#endif

//sig 2
void sig_handle(int no)
{
    switch(no)
    {
        case SIGKILL:
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
			close(fd_mpu6050);
			shm_close();
            break;
        default:
            break;
    }

	exit(0);
}

int main()
{
	int ret = 0;

	int16_t ori_accel[3];
	int16_t ori_gyro[3];
	float accel[3];
	float gyro[3];
	float gyro_acc[3];
    float pos[3];
	double pitch = 0;
	double roll  = 0;
	double yaw  = 0;
    unsigned long target_tick = 0;

	CHECK_RET(i2c_init());
	CHECK_RET(mpu6050_init());
	CHECK_RET(shm_init());

	signal(SIGKILL, sig_handle);
	signal(SIGQUIT, sig_handle);
	signal(SIGINT,  sig_handle);
	signal(SIGTERM, sig_handle);

    get_ms(&target_tick);
	for(;;)
	{
        static unsigned long tmp_tick = 0;
		mpu6050_origin_data(ori_accel, ori_gyro);
		mpu6050_switch_data(ori_accel, ori_gyro, accel, gyro, &pitch, &roll, &yaw);
        mpu6050_calc_gyro_acc(gyro, gyro_acc);

        //use degree to convey more information
        pos[0] = (float)(pitch * 180.0 / pi);
        pos[1] = (float)(roll * 180 / pi);
        pos[2] = (float)(yaw * 180 / pi);

		if(ret)
		{
			printf("!%d\n", ret);
			usleep(10000 / USR_SET_RATE);
			continue;
		}
		//printf("ori acc : %06d %06d %06d  ", ori_accel[0], ori_accel[1], ori_accel[2]);
		//printf("ori gyro: %06d %06d %06d  ", ori_gyro[0], ori_gyro[1], ori_gyro[2]);
		//printf("acc : %+5.2f %+5.2f %+5.2f  ", accel[0], accel[1], accel[2]);
		//printf("gyro: %+5.2f %+5.2f %+5.2f  ", gyro[0], gyro[1], gyro[2]);
		//printf("gyro_acc: %+5.2f %+5.2f %+5.2f  ", gyro_acc[0], gyro_acc[1], gyro_acc[2]);
		//printf("pitch: %+5.2f  roll: %+5.2f  yaw: %+5.2f  ", pitch, roll, yaw);
        set_shm_dat(pos, gyro, accel, gyro_acc);

        //make sure period
        while(1)
        {
            get_ms(&tmp_tick);
            if(tmp_tick >= target_tick + 1000 / USR_SET_RATE)
            {
                target_tick += 1000 / USR_SET_RATE;
                break;
            }
            usleep(200);
        }

        //printf("tick: %+15ld  ", tmp_tick);
        //printf("\n");
	}

	return 0;
}
