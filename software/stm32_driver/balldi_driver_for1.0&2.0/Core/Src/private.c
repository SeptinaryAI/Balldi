#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include "main.h"
#include "private.h"

extern ADC_HandleTypeDef hadc1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
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

static uint16_t PwmServo            = 0;    // = pwm(%) * 1000
static uint16_t PeriodServo         = 0;    // = pwm * TIM1_PERIOD
static uint16_t PeriodServoLast     = 0x7FFF;   // servo pwm will not set if no change
static uint16_t ServoUpdateCount    = 0;    // servo update angle need time

static uint16_t PwmLeft             = 0;    // = pwm(%) * 1000
static uint16_t PeriodLeft          = 0;    // = pwm * TIM1_PERIOD

static uint16_t PwmRight            = 0;    // = pwm(%) * 1000
static uint16_t PeriodRight         = 0;    // = pwm * TIM1_PERIOD

static uint16_t PwmFly              = 0;    // = pwm(%) * 1000
static uint16_t PeriodFly           = 0;    // = pwm * TIM1_PERIOD

/* ESTOP == 0 : moto run   , ESTOP != 0 : moto emergency stop */
static uint8_t  EstopWheel          = 0;
static uint8_t  EstopFly            = 0;

static uint32_t EncLeftLst          = 0;                              //data from encoder last time
static uint32_t EncRightLst         = 0;      
static uint32_t EncTimeLeft         = 0;                              //encoder INT times , number of turns
static uint32_t EncTimeLeftLst      = 0;
static uint32_t EncTimeRight        = 0;                              //encoder INT times , number of turns
static uint32_t EncTimeRightLst     = 0;
/* para about encoder can work normally in case of overflow, cool! */

static uint16_t SpeedLeft           = 0;                              //calc by data from encoder
static uint16_t SpeedRight          = 0;      

static char     LeftDir             = 'f';
static char     RightDir            = 'f';
static char     FlyDir              = 'f';

static uint8_t  SetFlgServo         = 0;
static uint8_t  SetFlgLeft          = 0;
static uint8_t  SetFlgRight         = 0;
static uint8_t  SetFlgFly           = 0;


#if 0
void $dbg_use$;
#endif

uint16_t get_PeriodServo()
{
    return PeriodServo;
}
uint16_t get_PeriodRight()
{
    return PeriodRight;
}
TOOL void set_left_dir(char dir);
TOOL void set_right_dir(char dir);
TOOL void set_fly_dir(char dir);

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

DBG static uint8_t cmd_get_left_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_LEFT_PWM, PwmLeft);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

DBG static uint8_t cmd_get_right_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_RIGHT_PWM, PwmRight);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

DBG static uint8_t cmd_get_fly_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_FLY_PWM, PwmFly);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

DBG static uint8_t cmd_get_servo_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_SERVO_PWM, PwmServo);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_servo_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);


    uint8_t  ret = RET_SUCCEED;
    uint16_t pwm = 0;

    ret = decode_rx_data(bufVal, &pwm);

    JUDGE_NOSUCC_RET(ret, ret);

    if(pwm > SERVO_PWM_MAX) 
    {
        pwm = SERVO_PWM_MAX;
    }
    else if(pwm < SERVO_PWM_MIN)
    {
        pwm = SERVO_PWM_MIN;
    }

    SET_PWM_SERVO(pwm);
    SetFlgServo = 1;

    encode_tx_data(TxBuf, CMD_SET_SERVO_PWM, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_left_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t pwm = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&pwm);
    JUDGE_NOSUCC_RET(ret, ret);

    if(pwm > LEFT_PWM_MAX || pwm < LEFT_PWM_MIN)
    {
        return RET_INVALID_VALUE;
    }

    LeftDir     = GET_DIR(pwm);

    PwmLeft     = ABS(pwm);

    if(PwmLeft && (!EstopWheel))
    {
        EstopWheel = 1;
    }

    SET_PWM_LEFT(PwmLeft);
    SetFlgLeft = 1;

    encode_tx_data(TxBuf, CMD_SET_LEFT_PWM, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_right_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t pwm = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&pwm);
    JUDGE_NOSUCC_RET(ret, ret);

    if(pwm > RIGHT_PWM_MAX || pwm < RIGHT_PWM_MIN)
    {
        return RET_INVALID_VALUE;
    }

    RightDir    = GET_DIR(pwm);

    PwmRight    = ABS(pwm);

    if(PwmRight && (!EstopWheel))
    {
        EstopWheel = 1;
    }

    SET_PWM_RIGHT(PwmRight);
    SetFlgRight = 1;

    encode_tx_data(TxBuf, CMD_SET_RIGHT_PWM, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_set_fly_pwm(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    int16_t pwm = 0;

    ret = decode_rx_data(bufVal, (uint16_t *)&pwm);
    JUDGE_NOSUCC_RET(ret, ret);

    if(pwm > FLY_PWM_MAX || pwm < FLY_PWM_MIN)
    {
        return RET_INVALID_VALUE;
    }

    FlyDir      = GET_DIR(pwm);

    PwmFly      = ABS(pwm);

    if(PwmFly && (!EstopFly))
    {
        EstopFly = 1;
    }

    SET_PWM_FLY(PwmFly);
    SetFlgFly = 1;

    encode_tx_data(TxBuf, CMD_SET_FLY_PWM, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_wheel_estop(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t stat = 0;

    ret = decode_rx_data(bufVal, &stat);
    JUDGE_NOSUCC_RET(ret, ret);

    //0 : estop  1 : continue run
    EstopWheel = stat;

    if(!EstopWheel)
    {
        SET_PWM_LEFT(0);
        SET_PWM_RIGHT(0);
    }

    encode_tx_data(TxBuf, CMD_WHEEL_ESTOP, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_fly_estop(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    uint8_t  ret = RET_SUCCEED;
    uint16_t stat = 0;

    ret = decode_rx_data(bufVal, &stat);
    JUDGE_NOSUCC_RET(ret, ret);

    //0 : estop  1 : continue run
    EstopFly = stat;

    if(!EstopFly)
    {
        SET_PWM_FLY(0);
    }

    encode_tx_data(TxBuf, CMD_WHEEL_ESTOP, RUN_CMD_SUCCEED);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_left_speed(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_LEFT_SPEED, SpeedLeft);

    UART2_SEND(TxBuf);

    return RET_SUCCEED;
}

static uint8_t cmd_get_right_speed(uint8_t *bufVal)
{
    JUDGE_NULL_RET(bufVal);

    encode_tx_data(TxBuf, CMD_GET_RIGHT_SPEED, SpeedRight);

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
    cmd_get_left_pwm,  //0x10
    cmd_get_right_pwm,  //0x11
    cmd_get_fly_pwm,  //0x12
    cmd_get_servo_pwm,  //0x13
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
    cmd_get_left_speed,  //0x20
    cmd_get_right_speed,  //0x21
    NULL,  //0x22
    NULL,  //0x23
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
    cmd_set_left_pwm,  //0xa0
    cmd_set_right_pwm,  //0xa1
    cmd_set_fly_pwm,  //0xa2
    cmd_set_servo_pwm,  //0xa3
    NULL,  //0xa4
    NULL,  //0xa5
    NULL,  //0xa6
    NULL,  //0xa7
    NULL,  //0xa8
    NULL,  //0xa9
    NULL,  //0xaa
    NULL,  //0xab
    NULL,  //0xac
    NULL,  //0xad
    NULL,  //0xae
    NULL,  //0xaf
    cmd_wheel_estop,  //0xb0
    NULL,  //0xb1
    cmd_fly_estop,  //0xb2
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
void $$$$pwm_about$$$$;
#endif

LOOP void usr_loop_update_pwm_left(void)
{
    if(!SetFlgLeft)
        return;

    //up mode 
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, PeriodLeft);
}

LOOP void usr_loop_update_pwm_right(void)
{
    if(!SetFlgRight)
        return;

    //up mode 
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, PeriodRight);
}

LOOP void usr_loop_update_pwm_fly(void)
{
    if(!SetFlgFly)
        return;

    //up mode 
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, PeriodFly);
}

LOOP void usr_loop_update_pwm_servo(void)
{
    if(!SetFlgServo)
        return;

    if(PeriodServo != PeriodServoLast)
    {//change
        //up mode 
        //through voltage switch , output reverse logic . 
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, TIM1_PERIOD - PeriodServo);
        PeriodServoLast = PeriodServo;
        ServoUpdateCount = 0;
    }
    else if(ServoUpdateCount++ > SERVO_MOVE_COUNT)
    {//cnt
        ServoUpdateCount = SERVO_MOVE_COUNT + 1;
        //don't need servo moto to keep work, save power
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, TIM1_PERIOD - 0);
    }
}

LOOP void usr_loop_update_dir(void)
{
    set_fly_dir(FlyDir);
    set_left_dir(LeftDir);
    set_right_dir(RightDir);
}

LOOP void usr_loop_update_estop(void)
{
    if(0 == EstopWheel)
    {
        HAL_GPIO_WritePin(GPIO_WHEEL_STBY, PIN_WHEEL_STBY, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIO_WHEEL_STBY, PIN_WHEEL_STBY, GPIO_PIN_SET);
    }

    if(0 == EstopFly)
    {
        HAL_GPIO_WritePin(GPIO_FLY_STBY, PIN_FLY_STBY, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIO_FLY_STBY, PIN_FLY_STBY, GPIO_PIN_SET);
    }
}

#if 0
void $encoder_about$;
#endif


static inline void loop_update_speed(void)
{
    int32_t cntLeft  = __HAL_TIM_GET_COUNTER(&htim2);
    int32_t cntRight = __HAL_TIM_GET_COUNTER(&htim3);

    SpeedLeft  = (EncTimeLeft  - EncTimeLeftLst) * TIM2_PERIOD + (cntLeft - EncLeftLst);
    SpeedRight = (EncTimeRight - EncTimeRightLst) * TIM3_PERIOD + (cntRight - EncRightLst);

    EncLeftLst = cntLeft;
    EncRightLst = cntRight;
}

LOOP void usr_loop_update_speed(void)
{
    loop_update_speed();
}

CALLBACK void usr_tim2_irq_callback(void)
{
    (__HAL_TIM_GET_COUNTER(&htim2) < (TIM2_PERIOD / 2)) ?
                        ++EncTimeLeftLst : --EncTimeLeftLst;
}

CALLBACK void usr_tim3_irq_callback(void)
{
    (__HAL_TIM_GET_COUNTER(&htim3) < (TIM3_PERIOD / 2)) ?
                        ++EncTimeRightLst : --EncTimeRightLst;
}

USRINIT void usr_init_encoder(void)
{
    HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(&htim2, TIM2_PERIOD / 2);

    HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(&htim3, TIM3_PERIOD / 2);
}

#if 0
void $$$$moto_control_about$$$$;
#endif

USRINIT void usr_init_pwm(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

USRINIT void usr_init_pwm_val(void)
{
    //just for reference, it don't work
    SET_PWM_LEFT(0);
    SET_PWM_RIGHT(0);
    SET_PWM_FLY(0);
    SET_PWM_SERVO(80); //mid position
}

TOOL void set_left_dir(char dir)
{
    if('f' == dir)
    {
        /* forward */
        HAL_GPIO_WritePin(GPIO_LEFT_IN1, PIN_LEFT_IN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_LEFT_IN2, PIN_LEFT_IN2, GPIO_PIN_SET);
    }
    else
    {
        /* back */
        HAL_GPIO_WritePin(GPIO_LEFT_IN1, PIN_LEFT_IN1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIO_LEFT_IN2, PIN_LEFT_IN2, GPIO_PIN_RESET);

    }
}

TOOL void set_right_dir(char dir)
{
    if('f' == dir)
    {
        /* forward */
        HAL_GPIO_WritePin(GPIO_RIGHT_IN1, PIN_RIGHT_IN1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIO_RIGHT_IN2, PIN_RIGHT_IN2, GPIO_PIN_RESET);
    }
    else
    {
        /* back */
        HAL_GPIO_WritePin(GPIO_RIGHT_IN1, PIN_RIGHT_IN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_RIGHT_IN2, PIN_RIGHT_IN2, GPIO_PIN_SET);
    }
}

TOOL void set_fly_dir(char dir)
{
    if('f' == dir)
    {
        /* forward */
        HAL_GPIO_WritePin(GPIO_FLY_IN1, PIN_FLY_IN1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIO_FLY_IN2, PIN_FLY_IN2, GPIO_PIN_RESET);
    }
    else
    {
        /* back */
        HAL_GPIO_WritePin(GPIO_FLY_IN1, PIN_FLY_IN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIO_FLY_IN2, PIN_FLY_IN2, GPIO_PIN_SET);
    }
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
    uint32_t adChrg = 0;
    uint32_t adBat  = 0;
    uint8_t  count  = 0;

    /* AdcBuf = {val_bat, val_chrg, val_bat, val_chrg, ....} */
    while(count < ADC_BUF_LEN)
    {
        adBat  += (AdcBuf[count++] & 0xFFF);
        adChrg += (AdcBuf[count++] & 0xFFF);
    }

    adBat  /= (ADC_BUF_LEN / 2);
    adChrg /= (ADC_BUF_LEN / 2);

    adBat  = adBat * REFERENCE_VOLTAGE / DAC_RESOLUTION;   // = vol(V) * 1000
    adChrg = adChrg * REFERENCE_VOLTAGE / DAC_RESOLUTION;  // = vol(V) * 1000

    AdBat  = (uint16_t)adBat;
    AdChrg = (uint16_t)adChrg;
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

void dbg_set_pwm(uint16_t l, uint16_t r, uint16_t f, uint16_t s)
{
    SET_PWM_LEFT(l);
    SET_PWM_RIGHT(r);
    SET_PWM_FLY(f);
    SET_PWM_SERVO(s); //mid position
}
