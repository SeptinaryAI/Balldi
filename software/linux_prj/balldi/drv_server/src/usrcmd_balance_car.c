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
#include <math.h>
#include <pthread.h>
#include <errno.h>
#include "msg_serv.h"
#include "balldi_v2.h"
#include "data.h"
#include "usrcmd_balance_car.h"
#include "usrcmd_imu.h"

#define B_LR_P (4.0)
#define B_LR_I ()
#define B_LR_D (0.5)

#define B_F_P (10)
#define B_F_I ()
#define B_F_D (10)

static pthread_mutex_t MutexBalance= PTHREAD_MUTEX_INITIALIZER;
static int RunBlance = 0;

static float TargetPitchUp = -1.0;
static float TargetPitchDown = -9.0;

static float BalanceP = B_LR_P;
static float BalanceD = B_LR_D;


static inline int limit_lr_rate(int rate)
{
    if(rate > BALANCE_MAX_WHEEL_MOTO_RATE) return BALANCE_MAX_WHEEL_MOTO_RATE;
    if(rate < BALANCE_MIN_WHEEL_MOTO_RATE) return BALANCE_MIN_WHEEL_MOTO_RATE;

    return rate;
}

static inline int limit_fly_rate(int rate)
{
    if(rate > BALANCE_MAX_FLY_MOTO_RATE) return BALANCE_MAX_FLY_MOTO_RATE;
    if(rate < BALANCE_MIN_FLY_MOTO_RATE) return BALANCE_MIN_FLY_MOTO_RATE;

    return rate;
}


//just for fun
int balance()
{
    int ret = RET_SUCCEED;
    int cmd_out = 0;

    float pitch = 0;
    float gyro = 0;
    float gyro_acc = 0;

    int fly_rate = 0;
    int l_rate = 0;
    int r_rate = 0;

    if(!RunBlance)
    {
        int cmd_out = 0;
        local_send_cmd(2, (int)CMD_SET_LEFT_PWM, 0, &cmd_out);
        local_send_cmd(2, (int)CMD_SET_RIGHT_PWM, 0, &cmd_out);
        local_send_cmd(2, (int)CMD_SET_FLY_PWM, 0, &cmd_out);

        return RET_THREAD_OVER;
    }

    float target_pitch = pitch;
    if(pitch > TargetPitchUp)
        target_pitch = TargetPitchUp;
    else if(pitch < TargetPitchDown)
        target_pitch = TargetPitchDown;

    get_share_float(sizeof(float) * ADDR_IMU_GYRO2_ACC, &gyro_acc);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO2, &gyro);
    get_share_float(sizeof(float) * ADDR_IMU_PITCH, &pitch);

    l_rate = (target_pitch - pitch) * BalanceP + (0 - gyro) * BalanceD;
    l_rate = limit_lr_rate(l_rate);
    r_rate = l_rate;
    //fly_rate = (target_pitch - pitch) * B_F_P + (0 - gyro) * B_F_D;
    //fly_rate = limit_fly_rate(fly_rate);

    ret |= local_send_cmd(2, (int)CMD_SET_LEFT_PWM, l_rate, &cmd_out);
    ret |= local_send_cmd(2, (int)CMD_SET_RIGHT_PWM, r_rate, &cmd_out);
    //ret |= local_send_cmd(2, (int)CMD_SET_FLY_PWM, fly_rate, &cmd_out);

    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("balance set lr or fly pwm error: %d, cmd out %u\n", ret, cmd_out);
        return ret;
    }

    printf("pitch:%+5.2f  gyro2:%+5.2f  gyro1_acc:%+5.2f  fly_rate:%d  l:%d  r:%d\n",
            pitch,        gyro,        gyro_acc,        fly_rate,    l_rate, r_rate);

    return RET_SUCCEED;
}

static void *thread_balance(void *args)
{
    int ret = 0;

    for(;;)
    {
        ret = balance();
        if(RET_THREAD_OVER == ret)
        {
            printf("pthread exit>>>>>\n");
            pthread_mutex_unlock(&MutexBalance);
            return NULL;
        }
        usleep(PERIOD_BALANCE); //control period 50ms
    }
}

static void run_balance(void)
{
    int lock_stat = pthread_mutex_trylock(&MutexBalance);
    if(0 != lock_stat)
    {
        //I need only an instance of control_move
        printf("pthread pass>>>>>\n");
        return;
    }
    printf("pthread enter>>>>>\n");

    pthread_t p;
    pthread_create(&p, NULL, thread_balance, NULL);
}


int usr_ctl_balance_v2(int in)
{
    RunBlance = in;
    run_balance();

    return RET_SUCCEED;
}

//*100 times
int usr_ctl_set_balance_p_v2(int p)
{
    BalanceP = (float)p / 100;
}

//*100 times
int usr_ctl_set_balance_d_v2(int d)
{
    BalanceD = (float)d / 100;
}
