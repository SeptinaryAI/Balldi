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
#include "usrcmd_move_ctl_v2.h"
#include "usrcmd_imu.h"

#define     PI          (3.141592653)

//fly wheel para
#define     F_P1        (15.5)  //dif pitch
#define     F_P2        (0.0)  //dif gyro acc
#define     F_P3        (0.0)  //

//swing lr whell para
#define     S_P         (-1)
#define     S_I         ()
#define     S_D         (0.3)

//forward para
#define     VEL_P       (3)             //forward vel use

//move para
#define     RO_P        (1.5)           //rotary motion gyro use
#define     M_P         (0.3)           //gyro use
#define     M_I         (0.1)           //yaw use
#define     M_D         (1)             //gyro accel use

int ctl_left(int rate);
int ctl_right(int rate);

static pthread_mutex_t MutexMoveCtl = PTHREAD_MUTEX_INITIALIZER;

static uint8_t SendBuf[7] = {0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};

static int   TargetRate = 0;//pwm rate target value, (left and right
static float TargetAngVel = 0;//gyro target value, (left or right
static float TargetYaw = 0;//yaw target value, PID: para 'I' use
static float TargetForwardVel = 2;//go forward vel

static float LastForwardVel = 0;//go forward vel
static int LastLRate = 0x7FFFFFFF;//go forward vel
static int LastRRate = 0x7FFFFFFF;//go forward vel
static int LastTargetRate = 0x7FFFFFFF;//go forward vel
static int LastFlyRate = 0x7FFFFFFF;//fly wheel vel

static float TargetPitchUp = -5.0;
static float TargetPitchDown = -15.0;

static int EnableLr = 1;
static int EnableFly = 0;//fly wheel not used
static int EnableSwingCtl = 0;//use left and right wheels to control pitch


static inline int limit_lr_rate(int rate)
{
    if(rate > MAX_WHEEL_MOTO_RATE) return MAX_WHEEL_MOTO_RATE;
    if(rate < MIN_WHEEL_MOTO_RATE) return MIN_WHEEL_MOTO_RATE;

    return rate;
}

static inline int limit_lr_rate_adj(int rate, int target)
{
    if(target > 0 && rate > target) rate = target - DLY_WHEEL_MOTO_RATE_ADJ;
    if(target < 0 && rate < target) rate = target + DLY_WHEEL_MOTO_RATE_ADJ;

    if(rate > MAX_WHEEL_MOTO_RATE_ADJ) return MAX_WHEEL_MOTO_RATE_ADJ;
    if(rate < MIN_WHEEL_MOTO_RATE_ADJ) return MIN_WHEEL_MOTO_RATE_ADJ;

    return rate;
}

static inline int limit_fly_rate(int rate)
{
    if(rate > MAX_FLY_MOTO_RATE) return MAX_FLY_MOTO_RATE;
    if(rate < MIN_FLY_MOTO_RATE) return MIN_FLY_MOTO_RATE;

    return rate;
}

static inline int rate_low_pass(int val, int last_val)
{
    if(val < last_val - RATE_LOW_PASS_STEP) val = last_val - RATE_LOW_PASS_STEP;
    else if(val > last_val + RATE_LOW_PASS_STEP) val = last_val + RATE_LOW_PASS_STEP;

    return val;
}

static inline int rate_to_fix_integer(int val)
{
    if(val == 0)
        return 0;

    return val > 0 ? WHEEL_RATE_FIX_INTEGER : -WHEEL_RATE_FIX_INTEGER;
}

#if 0
void $$;
#endif


static void control_swing(float pitch, float gyro2, float gyro2_acc, int target_rate, int *rate)
{
    float target_pitch = pitch;
    if(pitch > TargetPitchUp)
        target_pitch = TargetPitchUp;
    else if(pitch < TargetPitchDown)
        target_pitch = TargetPitchDown;

    int adj = (target_pitch - pitch) * S_P + (0 - gyro2) * S_D;
    adj = limit_lr_rate_adj(adj, target_rate);

    printf("pitch:%+5.2f  gyro2:%+5.2f  gyro2_acc:%+5.2f  target_rate:%d  rate_adj:%d\n",
            pitch,        gyro2,        gyro2_acc,        target_rate,    adj);

    *rate = adj;
}

/* fly > 0 -> pitch up */
static int control_fly(float pitch, float gyro2, float gyro2_acc)
{
    int ret = 0;
    int fly_rate = 0;
    int cmd_out = 0;

    float target_pitch = pitch;
    if(pitch > TargetPitchUp)
        target_pitch = TargetPitchUp;
    else if(pitch < TargetPitchDown)
        target_pitch = TargetPitchDown;

    fly_rate = (target_pitch - pitch) * F_P1 + (0 - gyro2_acc) * F_P2;//pid control fly wheel accel
    fly_rate = limit_fly_rate(fly_rate);

    printf("pitch:%+5.2f  gyro2:%+5.2f  gyro2_acc:%+5.2f  fly_rate:%d\n",
            pitch,        gyro2,        gyro2_acc,        fly_rate);

    ret |= local_send_cmd(1, (int)CMD_SET_FLY_PWM, fly_rate, &cmd_out);

    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("set fly pwm error: %d, cmd out %d\n", ret, cmd_out);
        return ret;
    }

    LastFlyRate = fly_rate;

    return RET_SUCCEED;
}

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
    local_send_cmd(1, (int)CMD_SET_FLY_PWM, 0, &cmd_out);\
\
    return RET_THREAD_OVER;\
};\

#if 0
void $$;
#endif

#if 0

/**
 * @brief
 *
 * @param rate      0: stop     >0: keep go straight    <0: keep go back straight
 * @return int return status
 */
static int control_move_old(void)
{
    static int stop_steps = 0;
    int ret = 0;
    int cmd_out = 0;

    int target_rate = TargetRate;
    float target_angvel = TargetAngVel;
    float target_forwardvel = TargetForwardVel;
    float target_yaw = TargetYaw;

    uint8_t target_angvel_eq_0 = IS_FLOAT_ZERO(target_angvel);

    int l_rate = 0;
    int r_rate = 0;

    float gyro3 = 0;
    float gyro3_acc = 0;
    float pitch = 0;
    float gyro2 = 0;
    float gyro2_acc = 0;
    int il_speed = 0;
    int ir_speed = 0;
    int16_t l_speed = 0;
    int16_t r_speed = 0;
    float forward_vel= 0;
    //float yaw = 0;

    //wheel vel
    ret |= local_send_cmd(1, (int)CMD_GET_LEFT_SPEED, 0, &il_speed);
    ret |= local_send_cmd(1, (int)CMD_GET_RIGHT_SPEED, 0, &ir_speed);
    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("get wheel speed error: %d\n", ret);
        return RET_THREAD_OVER;
    }
    l_speed = (int16_t)(il_speed & 0xFFFF);
    r_speed = (int16_t)(ir_speed & 0xFFFF);

    //judge stop move
    //set control 0, and move stop
    if(target_rate == 0 && target_angvel_eq_0)//stop send move cmd
    {
        if(++stop_steps > MAX_STOP_STEP     //max step after stop cmd
        || (l_speed == 0 && r_speed == 0))  //the movement really stoped
        {
            stop_steps = 0;
            LastForwardVel = 0;
            LastLRate = 0;
            LastRRate = 0;
            LastFlyRate = 0;

            local_send_cmd(1, (int)CMD_SET_LEFT_PWM, l_rate, &cmd_out);
            local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, r_rate, &cmd_out);
            local_send_cmd(1, (int)CMD_SET_FLY_PWM, 0, &cmd_out);

            return RET_THREAD_OVER;
        }
    }

    //move continue
    if(target_rate < 0)
    {
        target_forwardvel = -target_forwardvel; //direction: go back
    }

    //target_rate low pass
    target_rate = rate_low_pass(target_rate, LastTargetRate);

    get_share_float(sizeof(float) * ADDR_IMU_GYRO3, &gyro3);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO3_ACC, &gyro3_acc);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO2, &gyro2);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO2_ACC, &gyro2_acc);
    get_share_float(sizeof(float) * ADDR_IMU_PITCH, &pitch);
    //get_share_float(sizeof(float) * ADDR_IMU_YAW, &yaw);

    EMERGENCY_STOP(pitch);

    forward_vel = (float)( (l_speed - r_speed) / 2 + r_speed );

    //calc
    double val_p = 1.0 * (target_angvel - gyro3);
    //double val_i = 1.0 * (target_yaw - yaw);
    double val_d = 1.0 * (0 - gyro3_acc);

    //double adj_val = val_p * M_P + val_i * M_I + val_d * M_D;
    double adj_val = 0;
    if(!target_angvel_eq_0)
    {//rotary motion  or turn left/right
        adj_val = val_p * RO_P + val_d * M_D;
    }
    else
    {//go straight
        adj_val = val_p * M_P + val_d * M_D;
    }

    int half_adj_val = (int)adj_val / 2;
    //printf("targetrate:%d  half:%d\n", target_rate, half_adj_val);

    /* swing control */
    int target_rate_adj = 0;
    if(EnableSwingCtl)
    {
        control_swing(pitch, gyro2, gyro2_acc, target_rate, &target_rate_adj);
    }

    /* forward move control */
    l_rate = target_rate + target_rate_adj + half_adj_val;
    r_rate = target_rate + target_rate_adj - half_adj_val;

    //forward vel adj
    if(target_angvel_eq_0)
    {
        l_rate += (target_forwardvel - forward_vel) * VEL_P;
        r_rate += (target_forwardvel - forward_vel) * VEL_P;
    }//rotary motion need not forward speed

    l_rate = limit_lr_rate(l_rate);
    r_rate = limit_lr_rate(r_rate);

#if 0
    printf("angvel/tar: %+8.2f -> %+8.2f  yaw/tar: %+5.2f -> %+5.2f  forwardvel: %+8.2f  angacc: %+8.2f  val p:%+8.2f i:%+8.2f d:%+8.2f  adj_val:%+8.2f l:%d r:%d\n",
                        //gyro3,target_angvel,yaw,target_yaw,forward_vel,gyro3_acc,val_p,val_i,val_d,adj_val,l_rate,r_rate);
                        gyro3,target_angvel,0.0,target_yaw,forward_vel,gyro3_acc,val_p,0.0,val_d,adj_val,l_rate,r_rate);
#endif

    if(EnableLr)
    {
        ret |= local_send_cmd(1, (int)CMD_SET_LEFT_PWM, l_rate, &cmd_out);
        ret |= local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, r_rate, &cmd_out);
    }
    else
    {
        ret |= local_send_cmd(1, (int)CMD_SET_LEFT_PWM, 0, &cmd_out);
        ret |= local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, 0, &cmd_out);
    }

    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("move ctl set lr pwm error: %d, cmd out %d\n", ret, cmd_out);
        return RET_THREAD_OVER;
    }

    if(EnableFly)
    {
        ret |= control_fly(pitch, gyro2, gyro2_acc);
    }
    else
    {
        ret |= local_send_cmd(1, (int)CMD_SET_FLY_PWM, 0, &cmd_out);
    }

    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("move ctl set fly pwm error: %d, cmd out %d\n", ret, cmd_out);
        return RET_THREAD_OVER;
    }

    LastForwardVel = forward_vel;
    LastLRate = l_rate;
    LastRRate = r_rate;
    LastTargetRate = target_rate;

	return RET_SUCCEED;
}

#endif

/**
 * @brief speed will be set to fixed value
 *
 * @param rate      0: stop     >0: keep go straight    <0: keep go back straight
 * @return int return status
 */
static int control_move(void)
{
    static int stop_steps = 0;
    int ret = 0;
    int cmd_out = 0;

    int target_rate = TargetRate;

    //in this control , target_rate will be set 0 or a fixed integer
    if(target_rate != 0)
    {
        target_rate = rate_to_fix_integer(target_rate);
    }

    float target_angvel = TargetAngVel;
    //float target_forwardvel = TargetForwardVel;
    float target_yaw = TargetYaw;

    uint8_t target_angvel_eq_0 = IS_FLOAT_ZERO(target_angvel);

    int l_rate = 0;
    int r_rate = 0;

    float gyro3 = 0;
    float gyro3_acc = 0;
    float pitch = 0;
    float gyro2 = 0;
    float gyro2_acc = 0;
    int il_speed = 0;
    int ir_speed = 0;
    //int16_t l_speed = 0;
    //int16_t r_speed = 0;
    //float forward_vel= 0;
    //float yaw = 0;

    //wheel vel
    //ret |= local_send_cmd(1, (int)CMD_GET_LEFT_SPEED, 0, &il_speed);
    //ret |= local_send_cmd(1, (int)CMD_GET_RIGHT_SPEED, 0, &ir_speed);
    //if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    //{
    //    printf("get wheel speed error: %d\n", ret);
    //    return RET_THREAD_OVER;
    //}
    //l_speed = (int16_t)(il_speed & 0xFFFF);
    //r_speed = (int16_t)(ir_speed & 0xFFFF);

    //judge stop move
    //set control 0, and move stop
    if(target_rate == 0 && target_angvel_eq_0)//stop send move cmd
    {
        if(++stop_steps > MAX_STOP_STEP)     //max step after stop cmd
        {
            stop_steps = 0;
            //LastForwardVel = 0;
            LastLRate = 0;
            LastRRate = 0;
            LastFlyRate = 0;

            local_send_cmd(1, (int)CMD_SET_LEFT_PWM, l_rate, &cmd_out);
            local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, r_rate, &cmd_out);
            local_send_cmd(1, (int)CMD_SET_FLY_PWM, 0, &cmd_out);

            return RET_THREAD_OVER;
        }
    }

    //move continue
    //if(target_rate < 0)
    //{
    //    target_forwardvel = -target_forwardvel; //direction: go back
    //}

    //target_rate low pass
    target_rate = rate_low_pass(target_rate, LastTargetRate);

    get_share_float(sizeof(float) * ADDR_IMU_GYRO3, &gyro3);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO3_ACC, &gyro3_acc);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO2, &gyro2);
    get_share_float(sizeof(float) * ADDR_IMU_GYRO2_ACC, &gyro2_acc);
    get_share_float(sizeof(float) * ADDR_IMU_PITCH, &pitch);
    //get_share_float(sizeof(float) * ADDR_IMU_YAW, &yaw);

    EMERGENCY_STOP(pitch);

    //forward_vel = (float)( (l_speed - r_speed) / 2 + r_speed );

    //calc
    double val_p = 1.0 * (target_angvel - gyro3);
    //double val_i = 1.0 * (target_yaw - yaw);
    double val_d = 1.0 * (0 - gyro3_acc);

    //double adj_val = val_p * M_P + val_i * M_I + val_d * M_D;
    double adj_val = 0;
    if(!target_angvel_eq_0)
    {//rotary motion  or turn left/right
        adj_val = val_p * RO_P + val_d * M_D;
    }
    else
    {//go straight
        adj_val = val_p * M_P + val_d * M_D;
    }

    int half_adj_val = (int)adj_val / 2;
    //printf("targetrate:%d  half:%d\n", target_rate, half_adj_val);

    /* swing control */
    int target_rate_adj = 0;
    if(EnableSwingCtl)
    {
        control_swing(pitch, gyro2, gyro2_acc, target_rate, &target_rate_adj);
    }

    /* forward move control */
    l_rate = target_rate + target_rate_adj + half_adj_val;
    r_rate = target_rate + target_rate_adj - half_adj_val;

    //forward vel adj
    //if(target_angvel_eq_0)
    //{
    //    l_rate += (target_forwardvel - forward_vel) * VEL_P;
    //    r_rate += (target_forwardvel - forward_vel) * VEL_P;
    //}//rotary motion need not forward speed

    //change limit
    l_rate = rate_low_pass(l_rate, LastLRate);
    r_rate = rate_low_pass(r_rate, LastRRate);

    //max limit
    l_rate = limit_lr_rate(l_rate);
    r_rate = limit_lr_rate(r_rate);

#if 0
    printf("angvel/tar: %+8.2f -> %+8.2f  yaw/tar: %+5.2f -> %+5.2f  forwardvel: %+8.2f  angacc: %+8.2f  val p:%+8.2f i:%+8.2f d:%+8.2f  adj_val:%+8.2f l:%d r:%d\n",
                        //gyro3,target_angvel,yaw,target_yaw,forward_vel,gyro3_acc,val_p,val_i,val_d,adj_val,l_rate,r_rate);
                        gyro3,target_angvel,0.0,target_yaw,forward_vel,gyro3_acc,val_p,0.0,val_d,adj_val,l_rate,r_rate);
#endif

    if(EnableLr)
    {
        ret |= local_send_cmd(1, (int)CMD_SET_LEFT_PWM, l_rate, &cmd_out);
        ret |= local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, r_rate, &cmd_out);
    }
    else
    {
        ret |= local_send_cmd(1, (int)CMD_SET_LEFT_PWM, 0, &cmd_out);
        ret |= local_send_cmd(1, (int)CMD_SET_RIGHT_PWM, 0, &cmd_out);
    }

    if(ret != RET_SUCCEED && ret != RET_SUCCEED_REPLY)
    {
        printf("move ctl set lr pwm error: %d, cmd out %d\n", ret, cmd_out);
        return RET_THREAD_OVER;
    }

    LastLRate = l_rate;
    LastRRate = r_rate;
    LastTargetRate = target_rate;

	return RET_SUCCEED;
}

static void *thread_control_move(void *args)
{
    int ret = 0;

    LastForwardVel = 0;
    LastLRate = 0;
    LastRRate = 0;
    LastTargetRate = 0;

    for(;;)
    {
        ret = control_move();
        if(RET_THREAD_OVER == ret)
        {
            //printf("pthread exit>>>>>\n");
            pthread_mutex_unlock(&MutexMoveCtl);
            pthread_exit("movement finished");
        }
        usleep(PERIOD_MOVE_CONTROL); //control period 50ms
    }
}

static void run_control_move(void)
{
    int lock_stat = pthread_mutex_trylock(&MutexMoveCtl);
    if(0 != lock_stat)
    {
        //I need only an instance of control_move
        //printf("pthread pass>>>>>\n");
        return;
    }
    //printf("pthread enter>>>>>\n");

    pthread_t p;
    pthread_create(&p, NULL, thread_control_move, NULL); //TODO: if you create pthread too frequently, ret 11
}

/**
 * @brief
 *
 * @param rate      0: stop     >0: keep go straight    <0: keep go back straight
 * @return int return status
 */
int usr_ctl_go_straight_v2(int rate)
{
    if(rate < MIN_WHEEL_MOTO_RATE || rate > MAX_WHEEL_MOTO_RATE)
    {
        return RET_OUT_RANGE;
    }

    TargetRate = rate;

    run_control_move();

    return RET_SUCCEED;
}

int usr_ctl_turn_lr_v2(int turn)
{
    if(turn < MIN_TURN_LR_VAL || turn > MAX_TURN_LR_VAL)
    {
        return RET_OUT_RANGE;
    }

    TargetAngVel = (float)turn;

    run_control_move();

    return RET_SUCCEED;
}

int usr_ctl_set_move_lr_enable_v2(int b)
{
    EnableLr = b;
}

int usr_ctl_set_move_fly_enable_v2(int b)
{
    EnableFly = b;
}

int usr_ctl_set_swing_ctl_enable(int b)
{
    EnableSwingCtl = b;
}
