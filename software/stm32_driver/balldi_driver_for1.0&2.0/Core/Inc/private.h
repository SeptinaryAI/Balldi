#ifndef PRIVATE_H
#define PRIVATE_H

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

/* CUBE DEFINE */
#define TIM1_PERIOD         2000     //1999 + 1
#define TIM2_PERIOD         65536    //65535 + 1
#define TIM3_PERIOD         65536    //65535 + 1

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

#define CMD_SET_PROG_RESIS      0xC2

#define CMD_SET_LEFT_PWM        0xA0
#define CMD_SET_RIGHT_PWM       0xA1
#define CMD_SET_FLY_PWM         0xA2
#define CMD_SET_SERVO_PWM       0xA3

#define CMD_WHEEL_ESTOP         0xB0
#define CMD_FLY_ESTOP           0xB2

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
#define GPIO_WHEEL_STBY         GPIOC
#define GPIO_FLY_STBY           GPIOC
#define PIN_WHEEL_STBY          GPIO_PIN_4
#define PIN_FLY_STBY            GPIO_PIN_7

#define GPIO_LEFT_IN1           GPIOC
#define GPIO_LEFT_IN2           GPIOC
#define GPIO_RIGHT_IN1          GPIOC
#define GPIO_RIGHT_IN2          GPIOC
#define GPIO_FLY_IN1            GPIOC
#define GPIO_FLY_IN2            GPIOC

#define PIN_LEFT_IN1            GPIO_PIN_0
#define PIN_LEFT_IN2            GPIO_PIN_1
#define PIN_RIGHT_IN1           GPIO_PIN_2
#define PIN_RIGHT_IN2           GPIO_PIN_3
#define PIN_FLY_IN1             GPIO_PIN_5
#define PIN_FLY_IN2             GPIO_PIN_6

/* encoder define */
#define ENCODER_SAMPLE_STEP     100     //wheel left/right speed update in main() while, execute once for 'ENCODER_SAMPLE_STEP' cycles

/* servo avaliable range [25, 125] */
#define SERVO_PWM_MAX           100     // = duty * 1000
#define SERVO_PWM_MIN           50      // = duty * 1000

#define SERVO_MOVE_COUNT        1200    //servo need x times update to change it's angle

/* neg pwm means direction is back*/
#define LEFT_PWM_LIMIT          100     //LIMIT / 100 max
#define RIGHT_PWM_LIMIT         100     //LIMIT / 100 max
#define FLY_PWM_LIMIT           100     //LIMIT / 100 max

#define LEFT_PWM_MAX            1000    // = duty * 1000
#define LEFT_PWM_MIN            -1000   // = duty * 1000
#define RIGHT_PWM_MAX           1000    // = duty * 1000
#define RIGHT_PWM_MIN           -1000   // = duty * 1000
#define FLY_PWM_MAX             1000    // = duty * 1000
#define FLY_PWM_MIN             -1000   // = duty * 1000

/* adc use */
#define ADC_BUF_LEN             40      //ch4, ch5 ,ch4 ,ch5 ,....

/* dac use */
#define REFERENCE_VOLTAGE       3300    //3.3V
#define DAC_RESOLUTION          4096    //12bit DAC

/* charge use */
#define BAT_TRUE                1
#define BAT_FLASE               0

#define GPIO_CHRG               GPIOB
#define GPIO_STDBY              GPIOB
#define GPIO_RESIS_CTL          GPIOB

#define PIN_CHRG                GPIO_PIN_4
#define PIN_STDBY               GPIO_PIN_3
#define PIN_R1                  GPIO_PIN_15
#define PIN_R2                  GPIO_PIN_14
#define PIN_R3                  GPIO_PIN_13
#define PIN_R4                  GPIO_PIN_12


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

/* pwm > 0 means  direction is forward */
#define GET_DIR(pwm)        ((pwm) >= 0 ? 'f' : 'b')

#define FEED_DOG \
    HAL_IWDG_Refresh(&hiwdg);

#define UART2_SEND(pBuf) \
{ \
    while(UNFINISHED == TxStat); \
    TxStat = UNFINISHED; \
    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)pBuf, TX_BUF_LEN); \
}


#define SET_PWM_LEFT(pwm) \
{ \
    PwmLeft = (pwm); \
    PeriodLeft  = TIM1_PERIOD * (pwm) / 1000 * LEFT_PWM_LIMIT / 100; \
}

#define SET_PWM_RIGHT(pwm) \
{ \
    PwmRight = (pwm); \
    PeriodRight = TIM1_PERIOD * (pwm) / 1000 * RIGHT_PWM_LIMIT / 100; \
}

#define SET_PWM_FLY(pwm) \
{ \
    PwmFly = (pwm); \
    PeriodFly   = TIM1_PERIOD * (pwm) / 1000 * FLY_PWM_LIMIT / 100; \
}

#define SET_PWM_SERVO(pwm) \
{ \
    PwmServo = (pwm); \
    PeriodServo = TIM1_PERIOD * (pwm) / 1000; \
}

typedef uint8_t (*fp_cmd_handle)(uint8_t *bufVal);

USRINIT void usr_init_pwm(void);
USRINIT void usr_init_pwm_val(void);
LOOP void usr_loop_update_pwm_left(void);
LOOP void usr_loop_update_pwm_right(void);
LOOP void usr_loop_update_pwm_fly(void);
LOOP void usr_loop_update_pwm_servo(void);
LOOP void usr_loop_update_dir(void);
LOOP void usr_loop_update_estop(void);

USRINIT void usr_init_encoder(void);
LOOP void usr_loop_update_speed(void);
CALLBACK void usr_tim2_irq_callback(void);
CALLBACK void usr_tim3_irq_callback(void);

USRINIT void usr_init_uart2_dma(void);
LOOP void usr_loop_uart2(void);
CALLBACK void usr_uart2_idle_callback(void);

USRINIT void usr_init_adc(void);
LOOP void usr_loop_update_adc(void);

LOOP void usr_loop_update_bat_status(void);

void dbg_set_pwm(uint16_t l, uint16_t r, uint16_t f, uint16_t s);
#endif
