/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "balldi_uart.h"
#include "balldi_v2.h"
#include "msg_serv.h"
#include "data.h"
#include "usrcmd_imu.h"
#include "usrcmd_move_ctl_v2.h"
#include "usrcmd_balance_car.h"


static pthread_mutex_t MutexCmd = PTHREAD_MUTEX_INITIALIZER;

static int PwmServo = 0xFFFF;
static int PwmLeft  = 0xFFFF;
static int PwmRight = 0xFFFF;
static int PwmFly   = 0xFFFF;

static int PwmServoTrueSet = 0xFFFF;
static int PwmLeftTrueSet  = 0xFFFF;
static int PwmRightTrueSet = 0xFFFF;
static int PwmFlyTrueSet   = 0xFFFF;

static uint8_t SendBuf[7] = {0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};


#if 0
void $$;
#endif

TOOL static uint8_t decode_data(const uint8_t *tmpBuf, uint8_t *decodeBuf)
{
    JUDGE_NULL_RET(tmpBuf);
    JUDGE_NULL_RET(decodeBuf);

    uint8_t bytNo = 0;

    while(1)
    {
        if(bytNo >= DATA_LEN + 1)
        {
            return RET_OUT_RANGE;
        }

        if(DATA_LEN == bytNo)
        {
            if(ENCODE_END != tmpBuf[bytNo])
            {
                return RET_INVALID_ENCODE;
            }
        }

        if(ENCODE_START == tmpBuf[bytNo])
        {
            return RET_INVALID_ENCODE;
        }

        if(ENCODE_END == tmpBuf[bytNo])
        {
            return RET_SUCCEED;
        }

        if(ENCODE_SW_AB == tmpBuf[bytNo])
        {
            if(bytNo >= DATA_LEN - 1)
            {
                return RET_OUT_RANGE;
            }

            if( (ENCODE_SW_AB != tmpBuf[bytNo + 1])
                && (ENCODE_SW_AC != tmpBuf[bytNo + 1]) )
            {
                return RET_INVALID_ENCODE;
            }

            *(decodeBuf++) = ENCODE_SW_AB == tmpBuf[bytNo + 1] ? 0xAA : 0xAB;
            bytNo += 2;
            continue;
        }
        else if(ENCODE_SW_FE == tmpBuf[bytNo])
        {
            if( bytNo >= DATA_LEN - 1)
            {
                return RET_OUT_RANGE;
            }
            if( (ENCODE_SW_FE != tmpBuf[bytNo + 1])
                && (ENCODE_SW_FD != tmpBuf[bytNo + 1]) )
            {
                return RET_INVALID_ENCODE;
            }

            *(decodeBuf++) = ENCODE_SW_FE == tmpBuf[bytNo + 1] ? 0xFF : 0xFE;
            bytNo += 2;
            continue;
        }

        *(decodeBuf++) = tmpBuf[bytNo++];
    }
}

TOOL static uint8_t decode_rx_data(const uint8_t *buf, uint16_t *getData)
{
    uint8_t ret = RET_SUCCEED;
    uint8_t decodeBuf[4] = {0, 0, 0, 0}; //valid data Num 0 ~ 4 bytes

    ret = decode_data(buf, decodeBuf);

    if(RET_SUCCEED != ret)
    {
        printf("decode_data failed! ret %d . in %s:%d\n", ret, __FUNCTION__, __LINE__);
        return ret;
    }

    *getData =
          (uint16_t)(decodeBuf[3] << 24)
        | (uint16_t)(decodeBuf[2] << 16)
        | (uint16_t)(decodeBuf[1] << 8)
        | (uint16_t)(decodeBuf[0]) ;

    return RET_SUCCEED;
}

TOOL static uint8_t encode_data(const uint8_t encBytH, const uint8_t encBytL, uint8_t *encodeBuf)
{
    JUDGE_NULL_RET(encodeBuf);

    uint8_t bytNo = 0;

    if(ENCODE_START == encBytL)
    {
        encodeBuf[bytNo++] = 0xAB;
        encodeBuf[bytNo++] = 0xAB;
    }
    else if(ENCODE_SW_AB == encBytL)
    {
        encodeBuf[bytNo++] = 0xAB;
        encodeBuf[bytNo++] = 0xAC;
    }
    else if(ENCODE_END == encBytL)
    {
        encodeBuf[bytNo++] = 0xFE;
        encodeBuf[bytNo++] = 0xFE;
    }
    else if(ENCODE_SW_FE == encBytL)
    {
        encodeBuf[bytNo++] = 0xFE;
        encodeBuf[bytNo++] = 0xFD;
    }
    else
    {
        encodeBuf[bytNo++] = encBytL;
    }

    if(ENCODE_START == encBytH)
    {
        encodeBuf[bytNo++] = 0xAB;
        encodeBuf[bytNo++] = 0xAB;
    }
    else if(ENCODE_SW_AB == encBytH)
    {
        encodeBuf[bytNo++] = 0xAB;
        encodeBuf[bytNo++] = 0xAC;
    }
    else if(ENCODE_END == encBytH)
    {
        encodeBuf[bytNo++] = 0xFE;
        encodeBuf[bytNo++] = 0xFE;
    }
    else if(ENCODE_SW_FE == encBytH)
    {
        encodeBuf[bytNo++] = 0xFE;
        encodeBuf[bytNo++] = 0xFD;
    }
    else
    {
        encodeBuf[bytNo++] = encBytH;
    }

    while(bytNo < DATA_LEN)
    {
        //full with 00
        encodeBuf[bytNo++] = 0x00;
    }

    return RET_SUCCEED;
}

TOOL static uint8_t encode_tx_data(uint8_t *buf, const uint8_t cmd, const uint16_t SendData)
{
    JUDGE_NULL_RET(buf);

    uint8_t ret = RET_SUCCEED;
    uint8_t bytNo = 1;

    buf[bytNo++] = cmd;

    ret = encode_data((uint8_t)((SendData & 0xFF00) >> 8),
                        (uint8_t)((SendData & 0x00FF) >> 0),
                        &buf[bytNo]);

    buf[0]              = ENCODE_START;
    buf[TX_BUF_LEN - 1] = ENCODE_END;

    return ret;
}

#if 0
void $$;
#endif


static int uart_send(uint8_t buf[], int len ,uint16_t send_val)
{
    int iRet = RET_SUCCEED;
	uint8_t send_cmd = buf[1];
    uint8_t get_cmd = 0;
    uint8_t rdBuf[RX_BUF_LEN];
    memset(rdBuf, '\0', RX_BUF_LEN);

    ioctl(get_fd_balldi(), TCFLSH, 2);

    int ret = write(get_fd_balldi(), buf, len);
    if(ret < 0)
    {
        printf("write error! %s:%d\n", __FILE__, __LINE__);
        iRet = RET_ERROR;
        goto RET;
    }

    if(ret < TX_BUF_LEN)
    {
        printf("write bytes = %d ,count error! %s:%d\n", ret, __FILE__, __LINE__);
        iRet = RET_ERROR_LEN;
        goto RET;
    }

    ret = read(get_fd_balldi(), rdBuf, RX_BUF_LEN);

    if(ret < 0)
    {
        printf("read error! %s:%d\n", __FILE__, __LINE__);
        iRet = RET_ERROR;
        goto RET;
    }

    if(ret < RX_BUF_LEN)
    {
        printf("read bytes = %d ,count error! %s:%d\n", ret, __FILE__, __LINE__);
        iRet = RET_ERROR_LEN;
        goto RET;
    }

	get_cmd = rdBuf[1];
	if(send_cmd != get_cmd)
	{
		iRet = RET_INVALID_CMD;
		goto RET;
	}

	save_data(get_cmd, send_val);


RET:
    if(iRet != 0 )
        printf("send:\n%02X %02X %02X %02X %02X %02X %02X\n",
                                            buf[0],
                                            buf[1],
                                            buf[2],
                                            buf[3],
                                            buf[4],
                                            buf[5],
                                            buf[6]);
    //ret <= 0 ?
    //      printf("\n")
    //    : printf("recv:\n%02X %02X %02X %02X %02X %02X %02X\n*********\n\n",
    //                                        rdBuf[0],
    //                                        rdBuf[1],
    //                                        rdBuf[2],
    //                                        rdBuf[3],
    //                                        rdBuf[4],
    //                                        rdBuf[5],
    //                                        rdBuf[6]);
    return iRet;
}

static int uart_get(uint8_t buf[], int len, uint16_t *getVal)
{
    int iRet = RET_SUCCEED;
    uint8_t send_cmd = buf[1];
    uint8_t get_cmd = 0;
    uint8_t rdBuf[RX_BUF_LEN];
    memset(rdBuf, '\0', RX_BUF_LEN);

    ioctl(get_fd_balldi(), TCFLSH, 2);

    int ret = write(get_fd_balldi(), buf, len);
    if(ret < 0)
    {
        printf("write error! %s:%d\n", __FILE__, __LINE__);
        iRet = RET_ERROR;
        goto RET2;
    }

    if(ret < TX_BUF_LEN)
    {
        printf("write bytes = %d ,count error! %s:%d\n", ret, __FILE__, __LINE__);
        iRet = RET_ERROR_LEN;
        goto RET2;
    }

    ret = read(get_fd_balldi(), rdBuf, RX_BUF_LEN);

    if(ret < 0)
    {
        printf("read error! %s:%d\n", __FILE__, __LINE__);
        iRet = RET_ERROR;
        goto RET2;
    }

    if(ret < RX_BUF_LEN)
    {
        printf("read bytes = %d ,count error! %s:%d\n", ret, __FILE__, __LINE__);
        iRet = RET_ERROR_LEN;
        goto RET2;
    }

	get_cmd = rdBuf[1];
	if(send_cmd != get_cmd)
	{
		iRet = RET_INVALID_CMD;
		goto RET2;
	}

    iRet = (int)decode_rx_data(rdBuf + 2, getVal);

    if(RET_SUCCEED == iRet)
	{
		save_data(get_cmd, *getVal);
	}


RET2:
    if(iRet != 0 )
        printf("send:\n%02X %02X %02X %02X %02X %02X %02X\n",
                                            buf[0],
                                            buf[1],
                                            buf[2],
                                            buf[3],
                                            buf[4],
                                            buf[5],
                                            buf[6]);
    //ret <= 0 ?
    //      printf("\n")
    //    : printf("recv:\n%02X %02X %02X %02X %02X %02X %02X\n*********get val : %d\n\n",
    //                                        rdBuf[0],
    //                                        rdBuf[1],
    //                                        rdBuf[2],
    //                                        rdBuf[3],
    //                                        rdBuf[4],
    //                                        rdBuf[5],
    //                                        rdBuf[6],
    //                                        *getVal);
    return iRet;
}

#if 0
void $BALLDI_STM32_CMD$;
#endif

static int ctl_servo_ori(int pwm)
{
    int ret = 0;

    pwm = DIR_FIX_SET_SERVO(pwm);

    if(pwm == PwmServoTrueSet)
    {
        return RET_SUCCEED_REPLY;
    }

    PwmServo = pwm;

    //printf("servo pwm: %d\n", pwm);

    encode_tx_data(SendBuf, CMD_SET_SERVO_PWM, (uint16_t)PwmServo);

    if((ret = uart_send(SendBuf, TX_BUF_LEN, (uint16_t)PwmServo)) == RET_SUCCEED)
    {
        PwmServoTrueSet = PwmServo;
    }

    return ret;
}

/* 0 ~ 100 */
static int ctl_servo(int rate)
{
    int min = SERVO_PWM_MIN;
    int max = SERVO_PWM_MAX;
    int val = 0;
    int ret = 0;

    rate = DIR_FIX_SET_SERVO(rate);

    if(rate < 0 || rate > 100)
    {
        return RET_OUT_RANGE;
    }

    val = rate * (max - min) / 100 + min;

    if(val == PwmServoTrueSet)
    {
        return RET_SUCCEED_REPLY;
    }

    PwmServo = val;

    //printf("servo pwm: %d\n", val);

    encode_tx_data(SendBuf, CMD_SET_SERVO_PWM, (uint16_t)PwmServo);

    if((ret = uart_send(SendBuf, TX_BUF_LEN, (uint16_t)PwmServo)) == RET_SUCCEED)
    {
        PwmServoTrueSet = PwmServo;
    }

    return ret;
}

/* -100 ~ 100 */
static int ctl_left(int rate)
{
    int min = LEFT_PWM_MIN;
    int max = LEFT_PWM_MAX;
    int val = 0;
    int ret = 0;

    rate = DIR_FIX_SET_LEFT(rate);

    if(rate < -100 || rate > 100)
    {
        return RET_OUT_RANGE;
    }

    if(rate < 5 && rate > -5)
    {
        rate = 0;
    }

    val = (rate + 100) * (max -  min) / 200 + min;

    if(val == PwmLeftTrueSet)
    {
        return RET_SUCCEED_REPLY;
    }

    PwmLeft = val;

    //printf("left pwm: %d\n", PwmLeft);

    encode_tx_data(SendBuf, CMD_SET_LEFT_PWM, (uint16_t)PwmLeft);

    if((ret = uart_send(SendBuf, TX_BUF_LEN, (uint16_t)PwmLeft)) == RET_SUCCEED)
    {
        PwmLeftTrueSet = PwmLeft;
    }

    return ret;
}

/* -100 ~ 100 */
static int ctl_right(int rate)
{
    int min = RIGHT_PWM_MIN;
    int max = RIGHT_PWM_MAX;
    int val = 0;
    int ret = 0;

    rate = DIR_FIX_SET_RIGHT(rate);

    if(rate < -100 || rate > 100)
    {
        return RET_OUT_RANGE;
    }

    if(rate < 5 && rate > -5)
    {
        rate = 0;
    }

    val = (rate + 100) * (max -  min) / 200 + min;

    if(val == PwmRightTrueSet)
    {
        return RET_SUCCEED_REPLY;
    }

    PwmRight = val;

    //printf("right pwm: %d\n", PwmRight);

    encode_tx_data(SendBuf, CMD_SET_RIGHT_PWM, (uint16_t)PwmRight);

    if((ret = uart_send(SendBuf, TX_BUF_LEN, (uint16_t)PwmRight)) == RET_SUCCEED)
    {
        PwmRightTrueSet = PwmRight;
    }

    return ret;
}

/* -100 ~ 100 */
static int ctl_fly(int rate)
{
    int min = FLY_PWM_MIN;
    int max = FLY_PWM_MAX;
    int val = 0;
    int ret = 0;

    rate = DIR_FIX_SET_FLY(rate);

    if(rate < -100 || rate > 100)
    {
        return RET_OUT_RANGE;
    }

    val = (rate + 100) * (max - min) / 200 + min;

    if(val == PwmFlyTrueSet)
    {
        return RET_SUCCEED_REPLY;
    }

    PwmFly = val;

    //if(PwmFly < 100 && PwmFly > -100)
    //{
    //    PwmFly = 0;
    //}

    //printf("fly pwm: %d\n", PwmFly);

    encode_tx_data(SendBuf, CMD_SET_FLY_PWM, (uint16_t)PwmFly);

    if((ret = uart_send(SendBuf, TX_BUF_LEN, (uint16_t)PwmFly)) == RET_SUCCEED)
    {
        PwmFlyTrueSet = PwmFly;
    }

    return ret;
}

static int ctl_prog_resis(int rate)
{
    if(rate < 0 || rate > 15)
    {
        return RET_OUT_RANGE;
    }

    encode_tx_data(SendBuf, CMD_SET_PROG_RESIS, (uint16_t)rate);

    return uart_send(SendBuf, TX_BUF_LEN, (uint16_t)rate);
}

static int rd_left_speed(uint16_t *val_out)
{
    uint16_t val = 0;
    int ret = RET_SUCCEED;

    encode_tx_data(SendBuf, CMD_GET_LEFT_SPEED, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = (uint16_t)DIR_FIX_GET_LEFT((int16_t)val);

    //printf("left speed: %d\n", *val_out);

    return ret;
}

static int rd_right_speed(uint16_t *val_out)
{
    uint16_t val = 0;
    int ret = RET_SUCCEED;

    encode_tx_data(SendBuf, CMD_GET_RIGHT_SPEED, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = (uint16_t)DIR_FIX_GET_RIGHT((int16_t)val);

    //printf("right speed: %d\n", *val_out);

    return ret;
}

static int rd_bat_vol(uint16_t *val_out)
{
    uint16_t val = 0;
    int ret = RET_SUCCEED;

    encode_tx_data(SendBuf, CMD_GET_BAT_VOLTAGE, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = val + val * BAT_SAMPLE_RESIS_UP / BAT_SAMPLE_RESIS_DOWN;

    //printf("BAT Voltage : %fV\n", *val_out / 1000.0);

    return ret;
}

static int rd_chrg_proc_vol(uint16_t *val_out)
{
    int ret = RET_SUCCEED;
    uint16_t val = 0;

    encode_tx_data(SendBuf, CMD_GET_CHRG_VOLTAGE, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = val;

    //printf("PROC Voltage : %fV\n", val / 1000.0);

    return ret;
}

static int rd_chrg_resis(uint16_t *val_out)
{
    int ret = RET_SUCCEED;
    uint16_t val = 0;

    encode_tx_data(SendBuf, CMD_GET_PROG_RESIS, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = val;

    //printf("charge power level : %d\n", val);

    return ret;
}

static int rd_is_chrg(uint16_t *val_out)
{
    int ret = RET_SUCCEED;
    uint16_t val = 0;

    encode_tx_data(SendBuf, CMD_GET_IS_CHAR, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = val;

    //printf("charging ? %s\n", val ? "Yes" : "No");

    return ret;
}

static int rd_is_stdby(uint16_t *val_out)
{
    int ret = RET_SUCCEED;
    uint16_t val = 0;

    encode_tx_data(SendBuf, CMD_GET_IS_STDBY, (uint16_t)0);
    ret = uart_get(SendBuf, TX_BUF_LEN, &val);

    if(RET_SUCCEED != ret)
    {
        printf("uart_get data failed!\n");
        return ret;
    }

    *val_out = val;

    //printf("charge full ? %s\n", val ? "Yes" : "No");

    return ret;
}

#if 0
void $USR_CMD_IMU_ABOUT$;
#include "usrcmd_imu.h"
#endif


#if 0
void $USR_CMD_CONTROL_ABOUT$;
#include "usrcmd_move_ctl.h"
#endif


/* cmd code reference : stm32 driver code */
static fp_rd_func rd_func_list[0x50] = {
    NULL,   //0x00
    NULL,   //0x01
    NULL,   //0x02
    NULL,   //0x03
    NULL,   //0x04
    NULL,   //0x05
    NULL,   //0x06
    NULL,   //0x07
    NULL,   //0x08
    NULL,   //0x09
    NULL,   //0x0a
    NULL,   //0x0b
    NULL,   //0x0c
    NULL,   //0x0d
    NULL,   //0x0e
    NULL,   //0x0f
    NULL,   //0x10
    NULL,   //0x11
    NULL,   //0x12
    NULL,   //0x13
    NULL,   //0x14
    NULL,   //0x15
    NULL,   //0x16
    NULL,   //0x17
    NULL,   //0x18
    NULL,   //0x19
    NULL,   //0x1a
    NULL,   //0x1b
    NULL,   //0x1c
    NULL,   //0x1d
    NULL,   //0x1e
    NULL,   //0x1f
    rd_left_speed,   //0x20
    rd_right_speed,   //0x21
    NULL,   //0x22
    NULL,   //0x23
    NULL,   //0x24
    NULL,   //0x25
    NULL,   //0x26
    NULL,   //0x27
    NULL,   //0x28
    NULL,   //0x29
    NULL,   //0x2a
    NULL,   //0x2b
    NULL,   //0x2c
    NULL,   //0x2d
    NULL,   //0x2e
    NULL,   //0x2f
    rd_bat_vol,   //0x30
    rd_chrg_proc_vol,   //0x31
    rd_chrg_resis,   //0x32
    rd_is_chrg,   //0x33
    rd_is_stdby,   //0x34
    NULL,   //0x35
    NULL,   //0x36
    NULL,   //0x37
    NULL,   //0x38
    NULL,   //0x39
    NULL,   //0x3a
    NULL,   //0x3b
    NULL,   //0x3c
    NULL,   //0x3d
    NULL,   //0x3e
    NULL,   //0x3f
    usr_rd_pos_pitch,   //0x40
    usr_rd_pos_roll,   //0x41
    usr_rd_pos_yaw,   //0x42
    usr_rd_gyro1,   //0x43
    usr_rd_gyro2,   //0x44
    usr_rd_gyro3,   //0x45
    usr_rd_accel1,   //0x46
    usr_rd_accel2,   //0x47
    usr_rd_accel3,   //0x48
    usr_rd_gryo1_acc,   //0x49
    usr_rd_gryo2_acc,   //0x4A
    usr_rd_gryo3_acc,   //0x4B
};

static fp_ctl_func ctl_func_list[0x40] = {
    /* start from 0xA0 */
    ctl_left,   //0xa0
    ctl_right,   //0xa1
    ctl_fly,   //0xa2
    ctl_servo,   //0xa3
    NULL,   //0xa4
    NULL,   //0xa5
    NULL,   //0xa6
    NULL,   //0xa7
    NULL,   //0xa8
    NULL,   //0xa9
    NULL,   //0xaa
    NULL,   //0xab
    NULL,   //0xac
    NULL,   //0xad
    NULL,   //0xae
    NULL,   //0xaf
    NULL,   //0xb0
    NULL,   //0xb1
    NULL,   //0xb2
    NULL,   //0xb3
    NULL,   //0xb4
    NULL,   //0xb5
    NULL,   //0xb6
    NULL,   //0xb7
    NULL,   //0xb8
    NULL,   //0xb9
    NULL,   //0xba
    NULL,   //0xbb
    NULL,   //0xbc
    NULL,   //0xbd
    NULL,   //0xbe
    NULL,   //0xbf
    NULL,   //0xc0
    NULL,   //0xc1
    ctl_prog_resis,   //0xc2
    NULL,   //0xc3
    NULL,   //0xc4
    NULL,   //0xc5
    NULL,   //0xc6
    NULL,   //0xc7
    NULL,   //0xc8
    NULL,   //0xc9
    NULL,   //0xca
    NULL,   //0xcb
    NULL,   //0xcc
    NULL,   //0xcd
    NULL,   //0xce
    NULL,   //0xcf
    usr_ctl_go_straight_v2,   //0xd0
    usr_ctl_turn_lr_v2,   //0xd1
    usr_ctl_set_move_lr_enable_v2,   //0xd2
    usr_ctl_set_move_fly_enable_v2,   //0xd3
    NULL,   //0xd4
    NULL,   //0xd5
    NULL,   //0xd6
    usr_ctl_balance_v2,   //0xd7
    usr_ctl_set_balance_p_v2,   //0xd8
    usr_ctl_set_balance_d_v2,   //0xd9
    NULL,   //0xda
    NULL,   //0xdb
    NULL,   //0xdc
    NULL,   //0xdd
    NULL,   //0xde
    NULL,   //0xdf
};


int cmd_operation_v2(uint8_t cmd, int val_in, uint16_t *val_out)
{
    static unsigned long long cnt = 0;
	int func_ret = 0;

    if(cmd > 0xe0)
	{
		printf("cmd:0x%2X is out of range.\n", cmd);
        return RET_OUT_RANGE;
	}

    if(cmd >= 0xa0)
    {
        if(NULL == ctl_func_list[cmd - 0xa0])
		{
			printf("cmd:0x%2X is invalid.\n", cmd);
            return RET_INVALID_CMD;
		}

        return ctl_func_list[cmd - 0xa0](val_in);
    }

    if(NULL == rd_func_list[cmd])
	{
		printf("cmd:0x%2X hook function is NULL.\n", cmd);
        return RET_INVALID_CMD;
	}

    pthread_mutex_lock(&MutexCmd);
    func_ret = rd_func_list[cmd](val_out);
    cnt++;
    pthread_mutex_unlock(&MutexCmd);

	if(func_ret)
	{
		printf("cmd:0x%2X func ret err:%d, val_out= %u\n", cmd, func_ret, *val_out);
	}

	return func_ret;
}

