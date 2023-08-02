/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BALLDI_V3_H
#define BALLDI_V3_H

/* CUBE DEFINE */
#define TIM2_AUTORELOAD_PERIOD  20      // counter:(19 + 1)  -> 0.02ms , TIM2 irq period
#define SERVO_PERIOD_TICK       1000    // servo pwm period : 1000 * 0.02ms == 20ms
//#define TIM4_PERIOD         AutoreloadLR    //not const

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
#define DBG

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

/* UART CMD */
#define CMD_GET_SERVO_PWM       0x13

#define CMD_GET_LEFT_COUNT      0x22
#define CMD_GET_RIGHT_COUNT     0x23

#define CMD_GET_BAT_VOLTAGE     0x30
#define CMD_GET_CHRG_VOLTAGE    0x31
#define CMD_GET_PROG_RESIS      0x32
#define CMD_GET_IS_CHAR         0x33
#define CMD_GET_IS_STDBY        0x34

#define CMD_GET_POS_PITCH       0x40
#define CMD_GET_POS_ROLL        0x41
#define CMD_GET_POS_YAW         0x42
#define CMD_GET_GYRO1           0x43
#define CMD_GET_GYRO2           0x44
#define CMD_GET_GYRO3           0x45
#define CMD_GET_ACCEL1          0x46
#define CMD_GET_ACCEL2          0x47
#define CMD_GET_ACCEL3          0x48
#define CMD_GET_GRYO1_ACC       0x49
#define CMD_GET_GRYO2_ACC       0x4A
#define CMD_GET_GRYO3_ACC       0x4B

#define CMD_GET_DISTANCE        0x50

#define CMD_SET_PROG_RESIS      0xC2


#define CMD_SET_SERVO_TARGET_TICK   0xA1
#define CMD_SET_SERVO2_TARGET_TICK  0xA2

#define CMD_SET_DIR_LR          0xA5
#define CMD_SET_DIR_L           0xA6
#define CMD_SET_DIR_R           0xA7
#define CMD_SET_EN_LR           0xA8
#define CMD_SET_EN_L            0xA9
#define CMD_SET_EN_R            0xAA
#define CMD_SET_SPEED_LR        0xAB
#define CMD_SET_SPEED_L         0xAC
#define CMD_SET_SPEED_R         0xAD


#define CMD_SET_MODE            0xC0

#define USRCMD_SET_GO_STRAIGHT  0xD0
#define USRCMD_SET_TURN_LR      0xD1

#define USRCMD_SET_LINEOUT_ONOFF    0xD8
#define USRCMD_SET_DRIVER_BOARD_RST 0xD9

#define USRCMD_SET_LED1_COLOR       0XE0
#define USRCMD_SET_LED1_BRIGHTNESS  0XE1
#define USRCMD_SET_LED1_MODE        0XE2
#define USRCMD_SET_LED2_COLOR       0XE3
#define USRCMD_SET_LED2_BRIGHTNESS  0XE4
#define USRCMD_SET_LED2_MODE        0XE5

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

/* mode define */
#define MODE_CTRL_PWM           0
#define MODE_CTRL_SPEED         1


/* servo avaliable range [25, 125] */
#define SERVO_TICK_MAX          125
#define SERVO_TICK_MIN          25

#define SERVO_MOVE_COUNT        5000    //servo need x times main() loop to change it's angle

#define LR_SPEED_MAX            100
#define LR_SPEED_MIN            0

/* adc use */
#define ADC_BUF_LEN             40      //ch5, ch6 ,ch5 ,ch6 ,....

/* dac use */
#define REFERENCE_VOLTAGE       3300    //3.3V
#define DAC_RESOLUTION          4096    //12bit DAC

#define BAT_SAMPLE_RESIS_UP         10      //sample up resis KΩ
#define BAT_SAMPLE_RESIS_DOWN       15      //sample down resis KΩ

/* charge use */
#define BAT_TRUE                1
#define BAT_FLASE               0



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




#define DIR_FIX_SET_LR(v)   ((v))
#define DIR_FIX_SET_DIR_L(v)   ((v))
#define DIR_FIX_SET_DIR_R(v)   (!(v))
#define DIR_FIX_SET_SERVO(v)  (100 - (v))
#define DIR_FIX_SET_SERVO2(v)  (100 - (v))

typedef int (*fp_ctl_func)(int val);
typedef int (*fp_rd_func)(uint16_t *val_out);

#define FEED_DOG \
    HAL_IWDG_Refresh(&hiwdg);

#define UART2_SEND(pBuf) \
{ \
    while(UNFINISHED == TxStat); \
    TxStat = UNFINISHED; \
    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)pBuf, TX_BUF_LEN); \
}

int cmd_operation_v3(uint8_t cmd, int val_in, uint16_t *val_out);

#endif
