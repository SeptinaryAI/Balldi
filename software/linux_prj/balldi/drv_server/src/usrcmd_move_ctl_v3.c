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
#include "dbg.h"
#include "msg_serv.h"
#include "balldi_v3.h"
#include "data.h"
#include "usrcmd_move_ctl_v3.h"
#include "usrcmd_imu.h"


static pthread_mutex_t MutexMoveCtl = PTHREAD_MUTEX_INITIALIZER;

static int   TargetRate = 0;        //rate target value, (left and right
static float TargetAngVel = 0;      //gyro target value, (left or right

//stop if pitch too big to avoid flipping
#define EMERGENCY_STOP(pitch) \
while(1){\
    int cmd_out;\
\
    if(ABS(pitch) < EMERGENCY_STOP_THR)\
        break;\
\
    TargetRate = 0;\
    TargetAngVel = 0;\
\
    printf("emergency stop!\n");\
\
    local_send_cmd(1, (int)CMD_SET_LEFT_PWM, 0, &cmd_out);\
    local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, 0, &cmd_out);\
\
    return RET_THREAD_OVER;\
};\

#if 0
void $$;
#endif


//map about [rate][angvel]
static uint16_t DirMaps[3][3] =
{
    {0x0100, 0x0000, 0x0001}, //rate < 0 :   angle vel <0 , =0 , >0
    {0x0100, 0xFFFF, 0x0001}, //rate = 0 :   angle vel <0 , =0 , >0
    {0x0100, 0x0101, 0x0001}, //rate > 0 :   angle vel <0 , =0 , >0
};

static uint16_t calc_dir_by_rate_and_angvel(int rate, int angvel)
{
    int index1, index2;

    if(rate > 0)
        index1 = 2;
    else
        index1 = rate ? 0 : 1;

    if(angvel > 0)
        index2 = 2;
    else
        index2 = angvel ? 0 : 1;

    return DirMaps[index1][index2];
}

/**
 * @brief speed will be set to fixed value
 */
static void control_move(void)
{
    static int ctl_step_cnt = 0;
    static int last_dir = 0x0000;
    static int last_speed = 0xFFFF;
    int ret = 0;
    int cmd_out = 0;

    int target_rate = TargetRate;
    int target_angvel = TargetAngVel;
    int abs_rate = ABS(target_rate);
    int target_speed = abs_rate;

    int dir = calc_dir_by_rate_and_angvel(target_rate, target_angvel);

    DBG_F("dir:%#x\n", dir);
    //printf("target rate: %d, \n", target_rate);
    //printf("target dir: %d, ", dir);

    if(0 != target_angvel)  //turn left/right will use angvel to set speed
    {
        //turn left/right
        int abs_angvel = ABS(target_angvel);
        target_speed = abs_angvel * 100 / 300; //rate
    }

    if(last_dir != dir)
    {
        if(0xFFFF == dir)
        {
            last_dir = dir;
            DBG_F("dir no change\n");
        }
        else
        {
            ret = local_send_cmd(1, (int)CMD_SET_DIR_LR, dir, &cmd_out);
            if(RET_SUCCEED == ret || RET_SUCCEED_REPLY == ret)
            {
                last_dir = dir;
                DBG_F("set dir:%#x\n", dir);
            }
        }
    }

    if(last_speed != target_speed)
    {
        ret = local_send_cmd(1, (int)CMD_SET_SPEED_LR, target_speed, &cmd_out);
        if(RET_SUCCEED == ret || RET_SUCCEED_REPLY == ret)
        {
            last_speed = target_speed;
            DBG_F("set speed:%#x\n", target_speed);
        }
    }

    ++ctl_step_cnt;

    return ;
}

static void *thread_control_move(void *args)
{
    int ret = 0;

    DBG_F("thread in>>>>>\n");

    for(;;)
    {
        control_move();
        usleep(PERIOD_MOVE_CONTROL); //control period 50ms
    }
}

static void run_control_move(void)
{
    int lock_stat = pthread_mutex_trylock(&MutexMoveCtl);
    if(0 != lock_stat)
    {
        //I need only an instance of control_move
        DBG_F("pthread pass>>>>>\n");
        return;
    }
    DBG_F("pthread create>>>>>\n");

    pthread_t p;
    int ret = pthread_create(&p, NULL, thread_control_move, NULL);
    if(0 != ret)
    {
        pthread_mutex_unlock(&MutexMoveCtl);
        printf("pthread create failed, ret = %d, unlock mutex.\n", ret);
    }
}

/**
 * @brief
 *
 * @param rate      0: stop     >0: keep go straight    <0: keep go back straight
 * @return int return status
 */
int usr_ctl_go_straight_v3(int rate)
{
    DBG_F("_____ GO STRAIGHT _____\n");
    if(rate < MIN_WHEEL_MOTO_RATE)
        rate = MIN_WHEEL_MOTO_RATE;

    if(rate > MAX_WHEEL_MOTO_RATE)
        rate = MAX_WHEEL_MOTO_RATE;

    TargetRate = rate;

    run_control_move();

    return RET_SUCCEED;
}

int usr_ctl_turn_lr_v3(int turn)
{
    DBG_F("_____ TURN _____\n");
    if(turn < MIN_TURN_LR_VAL)
        turn = MIN_TURN_LR_VAL;

    if(turn > MAX_TURN_LR_VAL)
        turn = MAX_TURN_LR_VAL;

    TargetAngVel = (float)turn;

    run_control_move();

    return RET_SUCCEED;
}

