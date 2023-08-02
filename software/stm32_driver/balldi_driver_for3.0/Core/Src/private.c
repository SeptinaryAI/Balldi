/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "main.h"
#include "private.h"

extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

//extern IWDG_HandleTypeDef hiwdg;
//extern WWDG_HandleTypeDef hwwdg;

static uint8_t  RxStat = UNFINISHED;
static uint8_t  TxStat = FINISHED;

static uint8_t  RxBuf[RX_BUF_LEN];
static uint8_t  TxBuf[TX_BUF_LEN];

static uint16_t AdcBuf[ADC_BUF_LEN];

static uint16_t AdBat               = 0;
static uint16_t AdChrg              = 0;

static uint16_t IsChrg              = 0;
static uint16_t IsStdby             = 0;

static uint16_t ResisMap            = 4;                              //0 ~ 7 , charge current raise , default level = 4

//static uint16_t ModeCtrl            = 0;                              //ctrl mode

static uint16_t TargetTickServoH        = 97;       // target tick of servo high vol, (1.8 degree/tick), valid tick high range:[25, 125] -> make sure high vol time [0.5ms , 2.5ms] ~ [0 deg , 180 deg]
static uint16_t LastTargetTickServoH    = 0xFFFF;   // last setted target tick of servo high vol, (1.8 degree/tick)
static char     ServoEn                 = 'e';      // enable make pwm for servo
static uint16_t ServoStep               = 15;       // pulse step
static char     ServoKeepLimit          = 'd';      // if ='e' (enable), wiil only maintain torque for a limited period of time

static uint16_t TargetTickServo2H       = 75;       // target tick of servo high vol, (1.8 degree/tick), valid tick high range:[25, 125] -> make sure high vol time [0.5ms , 2.5ms] ~ [0 deg , 180 deg]
static uint16_t LastTargetTickServo2H   = 0xFFFF;   // last setted target tick of servo high vol, (1.8 degree/tick)
static char     Servo2En                = 'e';      // enable make pwm for servo
//static uint16_t Servo2Step              = 1;       // pulse step
static char     Servo2KeepLimit         = 'e';      // if ='e' (enable), wiil only maintain torque for a limited period of time
static uint16_t TmpTickServo2H    = 75;       // will approach -> TargetTickServo2H

static uint16_t LeftIntStep         = 0;        //one pulse in [LeftIntStep] times interruption of TIM4 to dirve step moto
static char     LeftPulseEn         = 'd';      //step moto enable control by software, if 'd' (disable), step moto will not generate step signal

static uint16_t RightIntStep        = 0;        //same as left
static char     RightPulseEn        = 'd';      //same as left

static uint16_t LeftSpeed           = 0;
static uint16_t TmpLeftSpeed        = 0;
static uint16_t RightSpeed          = 0;
static uint16_t TmpRightSpeed       = 0;

static char     LeftDir             = 'f';
static char     RightDir            = 'b';
static char     TmpLeftDir          = 'f';  //TmpLeftDir will approch -> LeftDir, considering speed
static char     TmpRightDir         = 'b';  //same

static char     LeftEn              = 'e';  //hardware enable para
static char     RightEn             = 'e';

static uint16_t LeftCount           = 0;
static uint16_t RightCount          = 0;


#if 0
void $dbg_use$;
#endif


TOOL void set_left_dir(char dir);
TOOL void set_right_dir(char dir);
TOOL void set_left_en(char dir);
TOOL void set_right_en(char dir);

#if 0
void $$$$encode_decode$$$$;
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
    //TODO: make it work normally
    uint8_t ret = RET_SUCCEED;
    uint8_t decodeBuf[4] = {0, 0, 0, 0}; //valid data Num 0 ~ 4 bytes

    ret = decode_data(buf, decodeBuf);

    if(RET_SUCCEED != ret)
    {
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

    //data
    ret = encode_data((uint8_t)((SendData & 0xFF00) >> 8),
                        (uint8_t)((SendData & 0x00FF) >> 0),
                        &buf[bytNo]);

    buf[0]              = ENCODE_START;
    buf[TX_BUF_LEN - 1] = ENCODE_END;

    return ret;
}

/* cmd */
#if 0
void $$$$cmd_fun$$$$;
#endif

DBG static uint8_t cmd_get_compile_time(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    char strTime[10] = "\0";

    sprintf(strTime, "%s", __TIME__);

    strTime[2] = strTime[3];
    strTime[3] = strTime[4];
    strTime[4] = strTime[6];
    strTime[5] = strTime[7];
    strTime[6] = '\0';

    memcpy((void *)TxBuf, strTime, TX_BUF_LEN);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

DBG static uint8_t cmd_get_reg(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    //special cmd , not decode
    uint32_t *reg = 0;
    uint32_t regCon = 0;

    reg = (uint32_t *)(((*bufVal) << 24) | ((*(bufVal + 1)) << 16) | ((*(bufVal + 2)) << 8) | (*(bufVal + 3)));
    regCon = *reg;

    memset((void *)TxBuf, 0x00, TX_BUF_LEN);
    memcpy((void *)TxBuf, &regCon, sizeof(regCon));

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_left_count(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_LEFT_COUNT, LeftCount);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_right_count(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_RIGHT_COUNT, RightCount);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set dir for left and right wheels
*/
static uint8_t cmd_set_dir_lr(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  dirs = 0;

    ret = decode_rx_data(bufVal, &dirs);
    JUDGE_NOSUCC_RET(ret, ret);

    LeftDir  = (dirs & 0x00FF) ? 'f' : 'b';
    RightDir = (dirs & 0xFF00) ? 'f' : 'b';

    encode_tx_data(TxBuf, CMD_SET_DIR_LR, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set dir for left wheels
*/
static uint8_t cmd_set_dir_l(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  dir = 0;

    ret = decode_rx_data(bufVal, &dir);
    JUDGE_NOSUCC_RET(ret, ret);

    LeftDir  = dir ? 'f' : 'b';

    encode_tx_data(TxBuf, CMD_SET_DIR_L, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set dir for right wheels
*/
static uint8_t cmd_set_dir_r(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  dir = 0;

    ret = decode_rx_data(bufVal, &dir);
    JUDGE_NOSUCC_RET(ret, ret);

    RightDir  = dir ? 'f' : 'b';

    encode_tx_data(TxBuf, CMD_SET_DIR_R, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set en for left and right wheels
*/
static uint8_t cmd_set_en_lr(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  ens = 0;

    ret = decode_rx_data(bufVal, &ens);
    JUDGE_NOSUCC_RET(ret, ret);

    LeftEn  = (ens & 0x00FF) ? 'e' : 'd';
    RightEn = (ens & 0xFF00) ? 'e' : 'd';

    encode_tx_data(TxBuf, CMD_SET_EN_LR, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set en for left wheels
*/
static uint8_t cmd_set_en_l(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  en = 0;

    ret = decode_rx_data(bufVal, &en);
    JUDGE_NOSUCC_RET(ret, ret);

    LeftEn = en ? 'e' : 'd';

    encode_tx_data(TxBuf, CMD_SET_EN_L, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

/**
 * set en for right wheels
*/
static uint8_t cmd_set_en_r(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t  en = 0;

    ret = decode_rx_data(bufVal, &en);
    JUDGE_NOSUCC_RET(ret, ret);

    RightEn = en ? 'e' : 'd';

    encode_tx_data(TxBuf, CMD_SET_EN_R, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_speed_lr(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t speed = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&speed);
    JUDGE_NOSUCC_RET(ret, ret);

    speed = ABS(speed);

    if(speed > LR_SPEED_MAX)
    {
        speed = LR_SPEED_MAX;
    }
    else if(speed < LR_SPEED_MIN)
    {
        speed = LR_SPEED_MIN;
    }

    LeftSpeed = speed;
    RightSpeed = speed;

    encode_tx_data(TxBuf, CMD_SET_SPEED_LR, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_speed_l(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t speed = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&speed);
    JUDGE_NOSUCC_RET(ret, ret);

    speed = ABS(speed);

    if(speed > LR_SPEED_MAX)
    {
        speed = LR_SPEED_MAX;
    }
    else if(speed < LR_SPEED_MIN)
    {
        speed = LR_SPEED_MIN;
    }

    LeftSpeed = speed;

    encode_tx_data(TxBuf, CMD_SET_SPEED_L, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_speed_r(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t speed = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&speed);
    JUDGE_NOSUCC_RET(ret, ret);

    speed = ABS(speed);

    if(speed > LR_SPEED_MAX)
    {
        speed = LR_SPEED_MAX;
    }
    else if(speed < LR_SPEED_MIN)
    {
        speed = LR_SPEED_MIN;
    }

    RightSpeed = speed;

    encode_tx_data(TxBuf, CMD_SET_SPEED_R, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_servo_target_tick(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);


    uint8_t  ret = RET_SUCCEED;
    uint16_t tick = 0;

    ret = decode_rx_data(bufVal, &tick);

    JUDGE_NOSUCC_RET(ret, ret);

    if(tick > SERVO_TICK_MAX)
    {
        tick = SERVO_TICK_MAX;
    }
    else if(tick < SERVO_TICK_MIN)
    {
        tick = SERVO_TICK_MIN;
    }

    TargetTickServoH = tick;

    encode_tx_data(TxBuf, CMD_SET_SERVO_TARGET_TICK, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_servo2_target_tick(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);


    uint8_t  ret = RET_SUCCEED;
    uint16_t tick = 0;

    ret = decode_rx_data(bufVal, &tick);

    JUDGE_NOSUCC_RET(ret, ret);

    if(tick > SERVO_TICK_MAX)
    {
        tick = SERVO_TICK_MAX;
    }
    else if(tick < SERVO_TICK_MIN)
    {
        tick = SERVO_TICK_MIN;
    }

    TargetTickServo2H = tick;

    encode_tx_data(TxBuf, CMD_SET_SERVO2_TARGET_TICK, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}


static uint8_t cmd_get_adc_bat(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_BAT_VOLTAGE, AdBat);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_adc_chrg(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_CHRG_VOLTAGE, AdChrg);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_is_chrg(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_IS_CHAR, IsChrg);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_is_stdby(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_IS_STDBY, IsStdby);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

#if 0
static uint8_t cmd_set_mode(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret      = RET_SUCCEED;
    uint16_t modeCtl  = 0;

    ret = decode_rx_data(bufVal, &modeCtl);
    JUDGE_NOSUCC_RET(ret, ret);

    if(modeCtl > MODE_CTRL_SPEED)
    {
        return RET_INVALID_VALUE;
    }

    ModeCtrl = modeCtl;

    encode_tx_data(TxBuf, CMD_SET_MODE, ModeCtrl);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}
#endif

static uint8_t cmd_get_prog_resis(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_PROG_RESIS, ResisMap);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_prog_resis(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret      = RET_SUCCEED;
    uint16_t resisMap = 0;

    ret = decode_rx_data(bufVal, &resisMap);
    JUDGE_NOSUCC_RET(ret, ret);

    ResisMap = (resisMap & 0x7);

    encode_tx_data(TxBuf, CMD_SET_PROG_RESIS, ResisMap);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

//use handle map replace switch-case, it will be useful
static fp_cmd_handle cmd_handle_map[0x100] = {
    NULL,  //0x00
    NULL,  //0x01
    NULL,  //0x02
    NULL,  //0x03
    NULL,  //0x04
    NULL,  //0x05
    NULL,  //0x06
    NULL,  //0x07
    NULL,  //0x08
    NULL,  //0x09
    NULL,  //0x0a
    NULL,  //0x0b
    NULL,  //0x0c
    NULL,  //0x0d
    NULL,  //0x0e
    NULL,  //0x0f
    NULL,  //0x10
    NULL,  //0x11
    NULL,  //0x12
    NULL,  //0x13
    NULL,  //0x14
    NULL,  //0x15
    NULL,  //0x16
    NULL,  //0x17
    NULL,  //0x18
    NULL,  //0x19
    NULL,  //0x1a
    NULL,  //0x1b
    NULL,  //0x1c
    NULL,  //0x1d
    NULL,  //0x1e
    NULL,  //0x1f
    NULL,  //0x20
    NULL,  //0x21
    cmd_get_left_count,  //0x22
    cmd_get_right_count,  //0x23
    NULL,  //0x24
    NULL,  //0x25
    NULL,  //0x26
    NULL,  //0x27
    NULL,  //0x28
    NULL,  //0x29
    NULL,  //0x2a
    NULL,  //0x2b
    NULL,  //0x2c
    NULL,  //0x2d
    NULL,  //0x2e
    NULL,  //0x2f
    cmd_get_adc_bat,  //0x30
    cmd_get_adc_chrg,  //0x31
    cmd_get_prog_resis,  //0x32
    cmd_get_is_chrg,  //0x33
    cmd_get_is_stdby,  //0x34
    NULL,  //0x35
    NULL,  //0x36
    NULL,  //0x37
    NULL,  //0x38
    NULL,  //0x39
    NULL,  //0x3a
    NULL,  //0x3b
    NULL,  //0x3c
    NULL,  //0x3d
    NULL,  //0x3e
    NULL,  //0x3f
    NULL,  //0x40
    NULL,  //0x41
    NULL,  //0x42
    NULL,  //0x43
    NULL,  //0x44
    NULL,  //0x45
    NULL,  //0x46
    NULL,  //0x47
    NULL,  //0x48
    NULL,  //0x49
    NULL,  //0x4a
    NULL,  //0x4b
    NULL,  //0x4c
    NULL,  //0x4d
    NULL,  //0x4e
    NULL,  //0x4f
    NULL,  //0x50
    NULL,  //0x51
    NULL,  //0x52
    NULL,  //0x53
    NULL,  //0x54
    NULL,  //0x55
    NULL,  //0x56
    NULL,  //0x57
    NULL,  //0x58
    NULL,  //0x59
    NULL,  //0x5a
    NULL,  //0x5b
    NULL,  //0x5c
    NULL,  //0x5d
    NULL,  //0x5e
    NULL,  //0x5f
    NULL,  //0x60
    NULL,  //0x61
    NULL,  //0x62
    NULL,  //0x63
    NULL,  //0x64
    NULL,  //0x65
    NULL,  //0x66
    NULL,  //0x67
    NULL,  //0x68
    NULL,  //0x69
    NULL,  //0x6a
    NULL,  //0x6b
    NULL,  //0x6c
    NULL,  //0x6d
    NULL,  //0x6e
    NULL,  //0x6f
    NULL,  //0x70
    NULL,  //0x71
    NULL,  //0x72
    NULL,  //0x73
    NULL,  //0x74
    NULL,  //0x75
    NULL,  //0x76
    NULL,  //0x77
    NULL,  //0x78
    NULL,  //0x79
    NULL,  //0x7a
    NULL,  //0x7b
    NULL,  //0x7c
    NULL,  //0x7d
    NULL,  //0x7e
    NULL,  //0x7f
    NULL,  //0x80
    NULL,  //0x81
    NULL,  //0x82
    NULL,  //0x83
    NULL,  //0x84
    NULL,  //0x85
    NULL,  //0x86
    NULL,  //0x87
    NULL,  //0x88
    NULL,  //0x89
    NULL,  //0x8a
    NULL,  //0x8b
    NULL,  //0x8c
    NULL,  //0x8d
    NULL,  //0x8e
    NULL,  //0x8f
    NULL,  //0x90
    NULL,  //0x91
    NULL,  //0x92
    NULL,  //0x93
    NULL,  //0x94
    NULL,  //0x95
    NULL,  //0x96
    NULL,  //0x97
    NULL,  //0x98
    NULL,  //0x99
    NULL,  //0x9a
    NULL,  //0x9b
    NULL,  //0x9c
    NULL,  //0x9d
    NULL,  //0x9e
    NULL,  //0x9f
    NULL,  //0xa0
    cmd_set_servo_target_tick,  //0xa1
    cmd_set_servo2_target_tick,  //0xa2
    NULL,  //0xa3
    NULL,  //0xa4
    cmd_set_dir_lr,  //0xa5
    cmd_set_dir_l,  //0xa6
    cmd_set_dir_r,  //0xa7
    cmd_set_en_lr,  //0xa8
    cmd_set_en_l,  //0xa9
    cmd_set_en_r,  //0xaa
    cmd_set_speed_lr,  //0xab
    cmd_set_speed_l,  //0xac
    cmd_set_speed_r,  //0xad
    NULL,  //0xae
    NULL,  //0xaf
    NULL,  //0xb0
    NULL,  //0xb1
    NULL,  //0xb2
    NULL,  //0xb3
    NULL,  //0xb4
    NULL,  //0xb5
    NULL,  //0xb6
    NULL,  //0xb7
    NULL,  //0xb8
    NULL,  //0xb9
    NULL,  //0xba
    NULL,  //0xbb
    NULL,  //0xbc
    NULL,  //0xbd
    NULL,  //0xbe
    NULL,  //0xbf
    NULL,  //0xc0
    NULL,  //0xc1
    cmd_set_prog_resis,  //0xc2
    NULL,  //0xc3
    NULL,  //0xc4
    NULL,  //0xc5
    NULL,  //0xc6
    NULL,  //0xc7
    NULL,  //0xc8
    NULL,  //0xc9
    NULL,  //0xca
    NULL,  //0xcb
    NULL,  //0xcc
    NULL,  //0xcd
    NULL,  //0xce
    NULL,  //0xcf
    NULL,  //0xd0
    NULL,  //0xd1
    NULL,  //0xd2
    NULL,  //0xd3
    NULL,  //0xd4
    NULL,  //0xd5
    NULL,  //0xd6
    NULL,  //0xd7
    NULL,  //0xd8
    NULL,  //0xd9
    NULL,  //0xda
    NULL,  //0xdb
    NULL,  //0xdc
    NULL,  //0xdd
    NULL,  //0xde
    NULL,  //0xdf
    NULL,  //0xe0
    NULL,  //0xe1
    NULL,  //0xe2
    NULL,  //0xe3
    NULL,  //0xe4
    NULL,  //0xe5
    NULL,  //0xe6
    NULL,  //0xe7
    NULL,  //0xe8
    NULL,  //0xe9
    NULL,  //0xea
    NULL,  //0xeb
    NULL,  //0xec
    NULL,  //0xed
    NULL,  //0xee
    NULL,  //0xef
    cmd_get_reg,  //0xf0
    cmd_get_compile_time,  //0xf1
    NULL,  //0xf2
    NULL,  //0xf3
    NULL,  //0xf4
    NULL,  //0xf5
    NULL,  //0xf6
    NULL,  //0xf7
    NULL,  //0xf8
    NULL,  //0xf9
    NULL,  //0xfa
    NULL,  //0xfb
    NULL,  //0xfc
    NULL,  //0xfd
    NULL,  //0xfe
    NULL,  //0xff
};

/**
 * decode receive data
 * BufRx - only use RX_BUF_LEN , bytes more than this length will be ignored
*/
static uint8_t usr_decode_rx(uint8_t *bufRx)
{
    JUDGE_NULL_RET(bufRx);

    /* check START */
    if(ENCODE_START != bufRx[0])
    {
        return RET_INVALID_START;
    }

    /* check CMD */
    if(NULL == cmd_handle_map[bufRx[1]])
    {
        return RET_INVALID_CMD;
    }

    return cmd_handle_map[bufRx[1]](bufRx + 2);
}



/*uart2 : use for data transmission with nano pi neo*/

/*
start byte: 0xAA
end   byte: 0xFF

change:
0xAA -> 0xAB 0xAB
0xAB -> 0xAB 0xAC

0xFF -> 0xFE 0xFE
0xFE -> 0xFE 0xFD

H3 to STM32:
7 bytes each package
1       1      0~2     0~2     1
start | cmd | data0 | data1 | end

STM32 to H3:
7 bytes each package
1       1        0~2     0~2     1
start | result | data0 | data1 | end


crc: data[0] xor data[1] xor ... - 1

*/
#if 0
void $$$$uart2_about$$$$;
#endif


USRINIT void usr_init_uart2_dma(void)
{
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart2, (uint8_t *)RxBuf, RX_BUF_LEN);
}

CALLBACK void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(&huart2 == huart)
    {
        RxStat = FINISHED;
    }
}

CALLBACK void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    //__HAL_DMA_DISABLE(huart->hdmatx);
    if(&huart2 == huart)
    {
        TxStat = FINISHED;
    }
}

CALLBACK void usr_uart2_idle_callback(void)
{
    //normal mode
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);
        //HAL_UART_DMAStop(&huart2);
        //HAL_UART_Receive_DMA(&huart2, (uint8_t *)RxBuf, RX_BUF_LEN);

        if(ENCODE_START != RxBuf[0])
        {
            /* disable the channel */
            __HAL_DMA_DISABLE(&hdma_usart2_rx);
            /* reset DMA count */
            __HAL_DMA_GET_COUNTER(&hdma_usart2_rx) = RX_BUF_LEN;
            /* enable the channel */
            __HAL_DMA_ENABLE(&hdma_usart2_rx);
        }
    }
}

void usr_uart2_data_calc()
{
    uint8_t ret = usr_decode_rx(RxBuf);

    switch(ret)
    {
        case RET_SUCCEED:
        {
            break;
        }
        case RET_ERROR:
        {
            memcpy((void *)TxBuf, "norerr", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_INVALID_ENCODE:
        {
            memcpy((void*)TxBuf, "inlenc", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_INVALID_CMD:
        {
            memcpy((void*)TxBuf, "unkcmd", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_INVALID_VALUE:
        {
            memcpy((void*)TxBuf, "inlval", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_INVALID_START:
        {
            memcpy((void*)TxBuf, "inlsta", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_OUT_RANGE:
        {
            memcpy((void*)TxBuf, "outrng", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        case RET_NULL_PTR:
        {
            memcpy((void*)TxBuf, "nulptr", TX_BUF_LEN);
            UART2_SEND(TxBuf);
            break;
        }
        default:
        {
            memcpy((void*)TxBuf, "unknow", TX_BUF_LEN);
            UART2_SEND(TxBuf);
        }
    }
}

LOOP void usr_loop_uart2(void)
{
    if(FINISHED == RxStat)
    {
        RxStat = UNFINISHED;
        usr_uart2_data_calc();
    }
}


#if 0
void $$$$moto_about$$$$;
#endif

static inline uint16_t step_moto_val_approach(uint16_t *p_cnt, uint16_t tmp, uint16_t target)
{
    const uint16_t count_skip = 100;     //about main loop period
    const uint16_t step = 1;

    if(++(*p_cnt) < count_skip)
    {
        return tmp;
    }

    *p_cnt = 0;

    if(target >= tmp + step)
    {
        return tmp + step;
    }
    else if(target <= tmp - step)
    {
        return tmp - step;
    }

    //if(target < tmp + step && target > tmp - step)
    return target;
}

static inline uint16_t calc_score_by_speed_and_dir(uint16_t score, char dir, uint16_t speed)
{
    if('f' == dir)
    {
        return score + speed;
    }
    //else
    return score - speed;
}

static inline char get_dir_by_score(uint16_t score)
{
    if(score > (LR_SPEED_MAX - LR_SPEED_MIN))
    {
        return 'f';
    }
    return 'b';
}

static inline uint16_t get_speed_by_score(uint16_t score)
{
    int16_t rel_speed = score - (LR_SPEED_MAX - LR_SPEED_MIN);

    return (uint16_t)ABS(rel_speed);
}

static void smooth_speed_and_dir(void)
{
    static uint16_t count_l = 0;
    static uint16_t count_r = 0;

    //base score set
    uint16_t score_l = LR_SPEED_MAX - LR_SPEED_MIN;
    uint16_t score_r = LR_SPEED_MAX - LR_SPEED_MIN;
    uint16_t target_score_l = LR_SPEED_MAX - LR_SPEED_MIN;
    uint16_t target_score_r = LR_SPEED_MAX - LR_SPEED_MIN;

    score_l = calc_score_by_speed_and_dir(score_l, TmpLeftDir, TmpLeftSpeed);
    score_r = calc_score_by_speed_and_dir(score_r, TmpRightDir, TmpRightSpeed);
    target_score_l = calc_score_by_speed_and_dir(target_score_l, LeftDir, LeftSpeed);
    target_score_r = calc_score_by_speed_and_dir(target_score_r, RightDir, RightSpeed);

    score_l = step_moto_val_approach(&count_l, score_l, target_score_l);
    score_r = step_moto_val_approach(&count_r, score_r, target_score_r);

    //get result
    TmpLeftDir = get_dir_by_score(score_l);
    TmpRightDir = get_dir_by_score(score_r);
    TmpLeftSpeed = get_speed_by_score(score_l);
    TmpRightSpeed = get_speed_by_score(score_r);
}

static void zero_speed_judge(void)
{
    /* speed == 0 judgement */
    if(LR_SPEED_MIN == LeftSpeed && LR_SPEED_MIN == TmpLeftSpeed)
    {
        LeftPulseEn = 'd';
    }
    else
    {
        LeftPulseEn = 'e';
    }

    if(LR_SPEED_MIN == RightSpeed && LR_SPEED_MIN == TmpRightSpeed)
    {
        RightPulseEn = 'd';
    }
    else
    {
        RightPulseEn = 'e';
    }
}

static inline uint16_t servo_val_approach(uint16_t *p_cnt, uint16_t tmp, uint16_t target)
{
    const uint16_t count_skip = 300;     //about main loop period
    const uint16_t step = 1;

    if(++(*p_cnt) < count_skip)
    {
        return tmp;
    }

    *p_cnt = 0;

    if(target >= tmp + step)
    {
        return tmp + step;
    }
    else if(target <= tmp - step)
    {
        return tmp - step;
    }

    //if(target < tmp + step && target > tmp - step)
    return target;
}

static void smooth_servo2(void)
{
    static uint16_t count = 0;

    //TODO:
    TmpTickServo2H = servo_val_approach(&count, TmpTickServo2H, TargetTickServo2H);
}

LOOP void usr_loop_smooth_movement(void)
{
    smooth_speed_and_dir();
    zero_speed_judge();
    smooth_servo2();
}

LOOP void usr_loop_update_speed(void)
{
    const float inv_min = 1.0 / LR_INT_STEP_MIN;
    const float inv_max = 1.0 / LR_INT_STEP_MAX;
    const float inv_range = inv_min - inv_max;

    float rate_l = (float)(TmpLeftSpeed - LR_SPEED_MIN) / (LR_SPEED_MAX - LR_SPEED_MIN);
    float rate_r = (float)(TmpRightSpeed - LR_SPEED_MIN) / (LR_SPEED_MAX - LR_SPEED_MIN);

    LeftIntStep = (uint16_t)(1.0 / (inv_max + rate_l * inv_range));
    RightIntStep = (uint16_t)(1.0 / (inv_max + rate_r * inv_range));
}

static void update_pwm_servo1(void)
{
    static uint16_t update_cnt    = 0;        // servo update angle need time

    if('e' != ServoKeepLimit)
        return ;

    if(TargetTickServoH != LastTargetTickServoH)
    {//change
        //....set
        ServoEn = 'e';

        LastTargetTickServoH = TargetTickServoH;
        update_cnt = 0;
    }
    else if(update_cnt > SERVO_MOVE_COUNT - 1)
    {//cnt
        update_cnt = SERVO_MOVE_COUNT;
        //don't need servo moto to keep work, save power

        //....stop set
        ServoEn = 'd';
    }
    ++update_cnt;
}

static void update_pwm_servo2(void)
{
    static uint16_t update_cnt    = 0;        // servo update angle need time

    if('e' != Servo2KeepLimit)
        return ;

    if(TmpTickServo2H != LastTargetTickServo2H)
    {//change
        //....set
        Servo2En = 'e';

        LastTargetTickServo2H = TargetTickServo2H;
        update_cnt = 0;
    }
    else if(update_cnt > SERVO_MOVE_COUNT - 1)
    {//cnt
        update_cnt = SERVO_MOVE_COUNT;
        //don't need servo moto to keep work, save power

        //....stop set
        Servo2En = 'd';
    }
    ++update_cnt;
}

LOOP void usr_loop_update_pwm_servo(void)
{
    update_pwm_servo1();
    update_pwm_servo2();
}

LOOP void usr_loop_update_dir(void)
{
    set_left_dir(TmpLeftDir);
    set_right_dir(TmpRightDir);
}

LOOP void usr_loop_update_en(void)
{
    set_left_en(LeftEn);
    set_right_en(RightEn);
}

#if 0
void $$$$moto_control_about$$$$;
#endif


TOOL void set_left_dir(char dir)
{
    if('f' == dir)
    {
        /* forward */
        HAL_GPIO_WritePin(GPIO_LEFT_DIR, PIN_LEFT_DIR, STAT_DIR_LEFT_FORWARD);
    }
    else
    {
        /* back */
        HAL_GPIO_WritePin(GPIO_LEFT_DIR, PIN_LEFT_DIR, STAT_DIR_LEFT_BACK);
    }
}

TOOL void set_right_dir(char dir)
{
    if('f' == dir)
    {
        /* forward */
        HAL_GPIO_WritePin(GPIO_RIGHT_DIR, PIN_RIGHT_DIR, STAT_DIR_RIGHT_FORWARD);
    }
    else
    {
        /* back */
        HAL_GPIO_WritePin(GPIO_RIGHT_DIR, PIN_RIGHT_DIR, STAT_DIR_RIGHT_BACK);
    }
}

TOOL void set_left_en(char en)
{
    static char last_en = 'u';

    if(en == last_en)
        return ;

    if('e' == en)
    {
        /* enable */
        HAL_GPIO_WritePin(GPIO_LEFT_EN, PIN_LEFT_EN, STAT_LEFT_EN);
    }
    else
    {
        /* disable */
        HAL_GPIO_WritePin(GPIO_LEFT_EN, PIN_LEFT_EN, STAT_LEFT_DIS);
    }

    last_en = en;
}

TOOL void set_right_en(char en)
{
    static char last_en = 'u';

    if(en == last_en)
        return ;

    if('e' == en)
    {
        /* enable */
        HAL_GPIO_WritePin(GPIO_RIGHT_EN, PIN_RIGHT_EN, STAT_RIGHT_EN);
    }
    else
    {
        /* disable */
        HAL_GPIO_WritePin(GPIO_RIGHT_EN, PIN_RIGHT_EN, STAT_RIGHT_DIS);
    }

    last_en = en;
}

static inline void usr_irq_make_servo1_pwm(void)
{
    static uint16_t target_servo_io = 0;        // target vol of servo , !0: high or 0:low
    static uint16_t tick_cnt = 0;        // servo high/low vol tick now    0.02ms / tick
    static uint16_t step_cnt = 0;

    /* servo enable */
    if(target_servo_io)
    {
        /* high vol stat */
        if(tick_cnt >= TargetTickServoH)
        {
            target_servo_io = !target_servo_io;   // high -> low
            tick_cnt = 0;                 // clean counter

            //action here
            HAL_GPIO_WritePin(GPIO_SERVO_1, PIN_SERVO_1, STAT_SERVO_1_H);
            return ;
        }
    }
    else
    {
        /* low vol stat */
        if(tick_cnt >= SERVO_PERIOD_TICK - TargetTickServoH)
        {
            target_servo_io = !target_servo_io;   // low -> high
            tick_cnt = 0;                 // clean counter

            if(++step_cnt < ServoStep)
                return ;                        //voltage will change to high  1 time / ServoStep step

            step_cnt = 0;

            //action here
            if('e' == ServoEn)
                HAL_GPIO_WritePin(GPIO_SERVO_1, PIN_SERVO_1, STAT_SERVO_1_L);

            return ;
        }
    }
    ++tick_cnt;
}

static inline void usr_irq_make_servo2_pwm(void)
{
    static uint16_t target_servo2_io = 0;        // target vol of servo , !0: high or 0:low
    static uint16_t tick_cnt = 0;        // servo high/low vol tick now    0.02ms / tick

    /* servo enable */
    if(target_servo2_io)
    {
        /* high vol stat */
        if(tick_cnt >= TmpTickServo2H)
        {
            target_servo2_io = !target_servo2_io;   // high -> low
            tick_cnt = 0;                 // clean counter

            //action here
            HAL_GPIO_WritePin(GPIO_SERVO_2, PIN_SERVO_2, STAT_SERVO_2_H);
            return ;
        }
    }
    else
    {
        /* low vol stat */
        if(tick_cnt >= SERVO_PERIOD_TICK - TmpTickServo2H)
        {
            target_servo2_io = !target_servo2_io;   // low -> high
            tick_cnt = 0;                 // clean counter

            //if(++step_cnt < Servo2Step)
            //    return ;                        //voltage will change  1 time / Servo2Step step

            //step_cnt = 0;

            //action here
            if('e' == Servo2En)
                HAL_GPIO_WritePin(GPIO_SERVO_2, PIN_SERVO_2, STAT_SERVO_2_L);

            return ;
        }
    }
    ++tick_cnt;
}

CALLBACK void usr_tim2_irq_callback(void)
{
    usr_irq_make_servo1_pwm();
    usr_irq_make_servo2_pwm();
}

static inline void usr_irq_make_left_pulse(void)
{
    static uint16_t last_io = 0;
    static uint16_t tick_cnt = 0;

    if(last_io)
    {
        //last tick pulse
        last_io = 0;
        tick_cnt = 0;

        //step counter
        if('e' == LeftEn)
        {
            if('f' == LeftDir)
                ++LeftCount;
            else
                --LeftCount;
        }

        HAL_GPIO_WritePin(GPIO_LEFT_STEP, PIN_LEFT_STEP, GPIO_PIN_RESET);
    }
    else
    {
        if(++tick_cnt < LeftIntStep)
        {
            return ;
        }

        //action here
        if('e' == LeftPulseEn)
            HAL_GPIO_WritePin(GPIO_LEFT_STEP, PIN_LEFT_STEP, GPIO_PIN_SET);

        last_io = 1;
        tick_cnt = 0;
    }
}

static inline void usr_irq_make_right_pulse(void)
{
    static uint16_t last_io = 0;
    static uint16_t tick_cnt = 0;

    if(last_io)
    {
        //last tick pulse
        last_io = 0;
        tick_cnt = 0;

        //step counter
        if('e' == RightEn)
        {
            if('f' == RightDir)
                ++RightCount;
            else
                --RightCount;
        }

        HAL_GPIO_WritePin(GPIO_RIGHT_STEP, PIN_RIGHT_STEP, GPIO_PIN_RESET);
    }
    else
    {
        if(++tick_cnt < RightIntStep)
        {
            return ;
        }

        //action here
        if('e' == RightPulseEn)
            HAL_GPIO_WritePin(GPIO_RIGHT_STEP, PIN_RIGHT_STEP, GPIO_PIN_SET);

        last_io = 1;
        tick_cnt = 0;
    }
}


CALLBACK void usr_tim4_irq_callback(void)
{
    usr_irq_make_left_pulse();
    usr_irq_make_right_pulse();
}


#if 0
void $$$$adc_about$$$$;
#endif

USRINIT void usr_init_adc(void)
{
    HAL_Delay(100);
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)AdcBuf, ADC_BUF_LEN);
}

LOOP void usr_loop_update_adc(void)
{
    uint32_t ad_chrg = 0;
    uint32_t ad_bat  = 0;
    uint8_t  count  = 0;

    /* AdcBuf = {val_bat, val_chrg, val_bat, val_chrg, ....} */
    while(count < ADC_BUF_LEN)
    {
        ad_bat  += (AdcBuf[count++] & 0xFFF);
        ad_chrg += (AdcBuf[count++] & 0xFFF);
    }

    ad_bat  /= (ADC_BUF_LEN / 2);
    ad_chrg /= (ADC_BUF_LEN / 2);

    ad_bat  = ad_bat * REFERENCE_VOLTAGE / DAC_RESOLUTION;   // = vol(V) * 1000
    ad_chrg = ad_chrg * REFERENCE_VOLTAGE / DAC_RESOLUTION;  // = vol(V) * 1000

    AdBat  = (uint16_t)ad_bat;
    AdChrg = (uint16_t)ad_chrg;
}

#if 0
void $$$$charge_about$$$$;
#endif

LOOP void usr_loop_update_bat_status(void)
{
    //vol high -> no chrg   ; vol low -> chrging
    //vol high -> no stdby  ; vol low -> stdby
    IsChrg  = HAL_GPIO_ReadPin(GPIO_CHRG, PIN_CHRG) == GPIO_PIN_SET ? BAT_FLASE : BAT_TRUE;
    IsStdby = HAL_GPIO_ReadPin(GPIO_STDBY, PIN_STDBY) == GPIO_PIN_SET ? BAT_FLASE : BAT_TRUE;

    //update charge current
    //chrg level 0 ~ 7  (0b000 ~ 0b111)
    HAL_GPIO_WritePin(GPIO_RESIS_CTL, PIN_R3, (ResisMap & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_RESIS_CTL, PIN_R2, (ResisMap & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_RESIS_CTL, PIN_R1, (ResisMap & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

#if 0
void $$$$wwdg$$$$;
#endif

//CALLBACK void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
//{
//    HAL_WWDG_Refresh(hwwdg);
//}

#if 0
void $$$$dbg_fun$$$$;
#endif

void dbg_set_dir(char l, char r)
{
    LeftDir  = l;
    RightDir = r;
}

void dbg_set_en(char l, char r)
{
    LeftEn  = l;
    RightEn = r;
}

void dbg_set_servo_en(char s1, char s2)
{
    ServoEn = s1;
    Servo2En = s2;
}

void dbg_set_servo_tick_h(uint16_t servo1, uint16_t servo2)
{
    TargetTickServoH = servo1;
    TargetTickServo2H = servo2;
}


void dbg_set_speed(uint16_t l, uint16_t r)
{
    LeftSpeed = l;
    RightSpeed = r;
}
