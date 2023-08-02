/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BALLDI_V2_H
#define BALLDI_V2_H

#define UART_WAIT_US        2000

/* CUBE DEFINE */
#define TIM1_PERIOD         2000     //1999 + 1

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

/* FUNCTION TYPE */
#define TOOL

#define CALLBACK
#define USRINIT
#define LOOP

/* UART USE */
#define RX_BUF_LEN  7
#define TX_BUF_LEN  7
#define DATA_LEN    (RX_BUF_LEN - 3)

#define FINISHED    1
#define UNFINISHED  0

/* UART ENCODE */
/*
start byte: 0xAA
end   byte: 0xFF

change:
0xAA -> 0xAB 0xAB
0xAB -> 0xAB 0xAC

0xFF -> 0xFE 0xFE
0xFE -> 0xFE 0xFD
*/
#define ENCODE_START            0xAA
#define ENCODE_END              0xFF
#define ENCODE_SW_AB            0xAB
#define ENCODE_SW_AC            0xAC
#define ENCODE_SW_FE            0xFE
#define ENCODE_SW_FD            0xFD

/* set speed direction fix */
#define DIR_FIX_SET_LEFT(v)   (-(v))
#define DIR_FIX_SET_RIGHT(v)  (-(v))
#define DIR_FIX_SET_SERVO(v)  (100 - (v))
#define DIR_FIX_SET_FLY(v)    (-(v))

/* get speed direction fix */
#define DIR_FIX_GET_LEFT(v)   (-(v))
#define DIR_FIX_GET_RIGHT(v)  (-(v))

/* UART GET CMD */
#define CMD_GET_LEFT_PWM        0x10
#define CMD_GET_RIGHT_PWM       0x11
#define CMD_GET_FLY_PWM         0x12
#define CMD_GET_SERVO_PWM       0x13

#define CMD_GET_LEFT_SPEED      0x20
#define CMD_GET_RIGHT_SPEED     0x21

#define CMD_GET_BAT_VOLTAGE     0x30
#define CMD_GET_CHRG_VOLTAGE    0x31
#define CMD_GET_PROG_RESIS      0x32
#define CMD_GET_IS_CHAR         0x33
#define CMD_GET_IS_STDBY        0x34

/* USR DEFINE GET CMD */
#define USRCMD_GET_POS_PITCH    0x40
#define USRCMD_GET_POS_ROLL     0x41
#define USRCMD_GET_POS_YAW      0x42
#define USRCMD_GET_GYRO1        0x43
#define USRCMD_GET_GYRO2        0x44
#define USRCMD_GET_GYRO3        0x45
#define USRCMD_GET_ACCEL1       0x46
#define USRCMD_GET_ACCEL2       0x47
#define USRCMD_GET_ACCEL3       0x48

//get angular acceleration
#define USRCMD_GET_GYROACC1     0x49
#define USRCMD_GET_GYROACC2     0x4A
#define USRCMD_GET_GYROACC3     0x4B

/* UART SET CMD */
#define CMD_SET_LEFT_PWM        0xA0
#define CMD_SET_RIGHT_PWM       0xA1
#define CMD_SET_FLY_PWM         0xA2
#define CMD_SET_SERVO_PWM       0xA3


#define CMD_SET_PROG_RESIS      0xC2

/* USR DEFINE ADVANCE CONTROL CMD */

/* will use IMU MPU6050 to control */
#define USRCMD_SET_GO_STRAIGHT  0xD0
#define USRCMD_SET_TURN_LR      0xD1
#define USRCMD_SET_MOVE_LR_ENABLE   0xD2
#define USRCMD_SET_MOVE_FLY_ENABLE  0xD3

/* fine grained control of fly wheel*/
//#define USRCMD_SET_FLY_PWM_F    0xD5

/* maybe a balance car? */
#define USRCMD_BALANCE          0xD7
#define USRCMD_SET_BALANCE_P    0xD8
#define USRCMD_SET_BALANCE_D    0xD9

/* CMD status return */
#define RUN_CMD_SUCCEED         0x0000
#define RUN_CMD_ERROR           0x1111
#define RUN_CMD_INVALID_ENCODE  0x2222
#define RUN_CMD_INVALID_CMD     0x3333
#define RUN_CMD_INVALID_VALUE   0x4444
#define RUN_CMD_INVALID_START   0x5555
#define RUN_CMD_OUT_RANGE       0x6666
#define RUN_CMD_NULL_PTR        0x7777

/* dbg use */
#define CMD_GET_REG             0xF0
#define CMD_GET_COMPILE_TIME    0xF1


#define SERVO_PWM_MAX           100     // = duty * 1000
#define SERVO_PWM_MIN           50      // = duty * 1000
/* neg pwm means direction is back*/
#define LEFT_PWM_MAX            1000    // = duty * 1000
#define LEFT_PWM_MIN            -1000   // = duty * 1000
#define RIGHT_PWM_MAX           1000    // = duty * 1000
#define RIGHT_PWM_MIN           -1000   // = duty * 1000
#define FLY_PWM_MAX             1000    // = duty * 1000
#define FLY_PWM_MIN             -1000   // = duty * 1000

/* adc use */
#define ADC_BUF_LEN             40      //ch4, ch5 ,ch4 ,ch5 ,....


#define BAT_SAMPLE_RESIS_UP         10      //sample resis 10K
#define BAT_SAMPLE_RESIS_DOWN       15      //sample resis 15K

typedef int (*fp_ctl_func)(int val);
typedef int (*fp_rd_func)(uint16_t *val_out);

#define JUDGE_NULL(ptr) \
{ \
    if(NULL == (ptr)) \
    { \
        return ; \
    } \
}

#define JUDGE_NULL_RET(ptr) \
{ \
    if(NULL == (ptr)) \
    { \
        return RET_NULL_PTR; \
    } \
}

#define JUDGE_NOSUCC_RET(ret, errcod) \
{ \
    if(RET_SUCCEED != (ret)) \
    { \
        return (errcod); \
    } \
}

#define ABS(a)              ((a) > 0 ? (a) : -(a))




int cmd_operation_v2(uint8_t cmd, int val_in, uint16_t *val_out);



#endif
