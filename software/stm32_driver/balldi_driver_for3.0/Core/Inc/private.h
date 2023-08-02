/*
Program of a spherical robot called Balldi, it is a funny IoT device.
Its reference prototype is SAMSUNG company's Ballie.

Copyright (C) <2023> <SeptinaryAI https://github.com/SeptinaryAI>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef PRIVATE_H
#define PRIVATE_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

/* CUBE DEFINE */
#define TIM2_AUTORELOAD_PERIOD  20      // counter:(19 + 1)  -> 0.02ms , TIM2 irq period
#define SERVO_PERIOD_TICK       1000    // servo pwm period : 1000 * 0.02ms == 20ms
#define TIM4_PERIOD             10      // 0.01ms

/* RETURN ERROR TYPE */
#define RET_SUCCEED         0
#define RET_ERROR           1
#define RET_INVALID_ENCODE  2
#define RET_INVALID_CMD     3
#define RET_INVALID_VALUE   4
#define RET_INVALID_START   5
#define RET_OUT_RANGE       6
#define RET_NULL_PTR        7

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

#define CMD_SET_PROG_RESIS      0xC2


#define CMD_SET_SERVO_TARGET_TICK   0xA1
#define CMD_SET_SERVO2_TARGET_TICK  0xA2

#define CMD_SET_PERIOD_LR       0xA4
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

/* moto define */
#define GPIO_LEFT_EN            (GPIOB)
#define GPIO_RIGHT_EN           (GPIOB)
#define GPIO_LEFT_DIR           (GPIOB)
#define GPIO_RIGHT_DIR          (GPIOB)
#define GPIO_LEFT_STEP          (GPIOB)
#define GPIO_RIGHT_STEP         (GPIOB)
#define GPIO_SERVO_1            (GPIOB)
#define GPIO_SERVO_2            (GPIOB)

#define PIN_LEFT_DIR            (GPIO_PIN_15)
#define PIN_RIGHT_DIR           (GPIO_PIN_14)
#define PIN_LEFT_EN             (GPIO_PIN_13)
#define PIN_RIGHT_EN            (GPIO_PIN_12)
#define PIN_LEFT_STEP           (GPIO_PIN_9)
#define PIN_RIGHT_STEP          (GPIO_PIN_8)
#define PIN_SERVO_1             (GPIO_PIN_6)
#define PIN_SERVO_2             (GPIO_PIN_7)

#define STAT_DIR_LEFT_FORWARD   (GPIO_PIN_SET)
#define STAT_DIR_RIGHT_FORWARD  (GPIO_PIN_SET)
#define STAT_DIR_LEFT_BACK      (GPIO_PIN_RESET)
#define STAT_DIR_RIGHT_BACK     (GPIO_PIN_RESET)
#define STAT_LEFT_EN            (GPIO_PIN_RESET)
#define STAT_RIGHT_EN           (GPIO_PIN_RESET)
#define STAT_LEFT_DIS           (GPIO_PIN_SET)
#define STAT_RIGHT_DIS          (GPIO_PIN_SET)

#define STAT_SERVO_1_L          (GPIO_PIN_SET)
#define STAT_SERVO_1_H          (GPIO_PIN_RESET)
#define STAT_SERVO_2_L          (GPIO_PIN_SET)
#define STAT_SERVO_2_H          (GPIO_PIN_RESET)

/* servo avaliable range [25, 125] */
#define SERVO_TICK_MAX          125
#define SERVO_TICK_MIN          25

#define SERVO_MOVE_COUNT        5000    //servo need x times main() loop to change it's angle

#define LR_PERIOD_MAX           180     //
#define LR_PERIOD_MIN           8      //

#define LR_SPEED_MAX            100
#define LR_SPEED_MIN            0

#define LR_INT_STEP_MAX         900     //test to find a best value
#define LR_INT_STEP_MIN         6       //test to find a best value

/* adc use */
#define ADC_BUF_LEN             40      //ch5, ch6 ,ch5 ,ch6 ,....

/* dac use */
#define REFERENCE_VOLTAGE       3300    //3.3V
#define DAC_RESOLUTION          4096    //12bit DAC

/* charge use */
#define BAT_TRUE                1
#define BAT_FLASE               0

#define GPIO_CHRG               (GPIOB)
#define GPIO_STDBY              (GPIOB)
#define GPIO_RESIS_CTL          (GPIOA)

#define PIN_CHRG                (GPIO_PIN_3)
#define PIN_STDBY               (GPIO_PIN_4)
#define PIN_R1                  (GPIO_PIN_11)
#define PIN_R2                  (GPIO_PIN_12)
#define PIN_R3                  (GPIO_PIN_13)


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

/* speed > 0 means  direction is forward */
#define GET_DIR(speed)        ((speed) >= 0 ? 'f' : 'b')

#define FEED_DOG \
    HAL_IWDG_Refresh(&hiwdg);

#define UART2_SEND(pBuf) \
{ \
    while(UNFINISHED == TxStat); \
    TxStat = UNFINISHED; \
    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)pBuf, TX_BUF_LEN); \
}



typedef uint8_t (*fp_cmd_handle)(uint8_t *bufVal);

LOOP void usr_loop_smooth_movement(void);
LOOP void usr_loop_update_speed(void);
LOOP void usr_loop_update_pwm_servo(void);
LOOP void usr_loop_update_dir(void);
LOOP void usr_loop_update_en(void);

USRINIT void usr_init_encoder(void);
LOOP void usr_loop_update_speed(void);
CALLBACK void usr_tim2_irq_callback(void);
CALLBACK void usr_tim4_irq_callback(void);

USRINIT void usr_init_uart2_dma(void);
LOOP void usr_loop_uart2(void);
CALLBACK void usr_uart2_idle_callback(void);

USRINIT void usr_init_adc(void);
LOOP void usr_loop_update_adc(void);

LOOP void usr_loop_update_bat_status(void);

void dbg_set_dir(char l, char r);
void dbg_set_en(char l, char r);
void dbg_set_servo_en(char s1, char s2);
void dbg_set_speed(uint16_t l, uint16_t r);
void dbg_set_servo_tick_h(uint16_t servo1, uint16_t servo2);
#endif
