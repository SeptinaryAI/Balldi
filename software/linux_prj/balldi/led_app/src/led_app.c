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
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "led_shm.h"
#include "led_app.h"

#define ABS(num) ((num) < 0 ? -(num) : (num))

static int Fd_led_spi = -1;
static t_led_shared_data Shared_dat_default =
{
    {
        {0xFFFF00, 50, LED_MODE_NORMAL},
        {0x00FFFF, 50, LED_MODE_NORMAL},
    }
};

static t_led_shared_data *Shared_ptr = NULL;

/* B R G
        LED1        LED2        LED3
LED     24 bit      24 bit      24 bit
        3*8
SPI     72 bit      72 bit      72 bit
        9*8
*/
unsigned Led_colors[LED_NUM];
unsigned Led_brightness[LED_NUM];
unsigned char Led_spi_arrs[LED_NUM*9];
int LightLevel = 50; //0 ~ 100

static int char2hex(char in, unsigned char *out)
{
    if(in <= '9' && in >= '0'){
        *out = in - '0';
    } else if(in <= 'F' && in >= 'A'){
        *out = in - 'A' + 10;
    } else if(in <= 'f' && in >= 'a'){
        *out = in - 'a' + 10;
    } else {
        return -1;
    }

    return 0;
}

void add_bit(unsigned char *byt, int *byt_cnt, int *bi_cnt, unsigned char bi)
{
    if(*bi_cnt >= 8){
        *bi_cnt = 0;
        ++(*byt_cnt);
        ++byt;
    }
    *byt |= ( (bi ? 1 : 0) << (7 - *bi_cnt) );
    ++(*bi_cnt);
}

#define PUSH_H \
{ \
    add_bit( \
        &spi_colors[led_cnt*9+byte_cnt], \
        &byte_cnt, \
        &bit_cnt, \
        SPI_HIGH \
    ); \
}
#define PUSH_L \
{ \
    add_bit( \
        &spi_colors[led_cnt*9+byte_cnt], \
        &byte_cnt, \
        &bit_cnt, \
        SPI_LOW \
    ); \
}

void color2spi(unsigned color[], unsigned char spi_colors[])
{
    for(int led_cnt = 0; led_cnt < LED_NUM; ++led_cnt){
        int byte_cnt = 0;
        int bit_cnt  = 0;

        for(int i = 0; i < 24; ++i){
            if(color[led_cnt] & (1 << (24-1-i))){
                PUSH_H;
                PUSH_H;
                PUSH_L;
            }
            else{
                PUSH_H;
                PUSH_L;
                PUSH_L;
            }
        }
    }
}

void str2color(char *str, int count, unsigned colors[])
{
    int no = 0;
    int bi = 6;
    int ele = 0;
    unsigned char hex_out;

    for(int cnt = 0; cnt < count; ++cnt)
    {
        if(char2hex(str[cnt], &hex_out))
            continue;

        ele = ele * 0x10 + hex_out;
        bi--;

        if(0 == bi)
        {
            colors[no] = ele;
            no++;
            if(no >= LED_NUM)
            {
                break;
            }
            bi = 6;
            ele = 0;
        }
    }
}


void dbg_print_led_colors(unsigned led_colors[])
{
    printf("BRG map -> RGB map:\n");
    for(int i = 0; i < LED_NUM; ++i)
    {
        printf("led%d 0x%06X -> 0x%06X",
                i + 1,
                led_colors[i] & 0xFFFFFF,
                ( (led_colors[i] & 0xFFFF) << 8) | ((led_colors[i] & 0xFF0000) >> 16) );
        printf("\n");
    }
    printf("\n");
}

void dbg_print_spi_arr(unsigned char spi_arr[])
{
    int cnt = 0;
    printf("spi map:\n");
    for(int i = 0; i < LED_NUM; ++i)
    {
        printf("led%d ", i + 1);
        cnt = 0;
        for(int j = 0; j < 9; ++j)
        {
            for(int k = 7; k >= 0; --k)
            {
                printf("%d", (spi_arr[i*9+j] >> k) & 0x1);
                if(0 == ++cnt % 3) printf(" ");
                if(0 == cnt % (3 * 8)) printf(" ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void dbg_print_color(void)
{
    dbg_print_led_colors(Led_colors);
    dbg_print_spi_arr(Led_spi_arrs);
}

#if 0
void $libapi$;
#endif

int init_led_spi(void)
{
    int ret = 0;
    int mode = 0;
    int speed = RGB_SPI_SPEED;

    if((Fd_led_spi = open(RGB_SPI_DEV, O_RDWR)) < 0)
    {
        printf("open spi dev failed\n");
        return LED_ERR;
    }

    ret = ioctl(Fd_led_spi, SPI_IOC_WR_MODE, &mode);
    if(ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MODE failed ret = %d, errno = %d\n", ret, errno);
        return LED_ERR;
    }

    ret = ioctl(Fd_led_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(ret < 0)
    {
        printf("ioctl SPI_IOC_WR_MAX_SPEED_HZ failed ret = %d, errno = %d\n", ret, errno);
        return LED_ERR;
    }

    return LED_SUCC;
}

int close_led_spi(void)
{
    if(-1 != Fd_led_spi)
    {
        close(Fd_led_spi);
        Fd_led_spi = -1;
        printf("close led spi fd\n");
        return 0;
    }
    printf("led spi fd already closed, nothing need to do\n");
    return 0;
}

//balldi has only two led
//if you want more leds!!!!!  use array param
void set_led_color(unsigned rgb1, unsigned rgb2)
{
    unsigned colors[LED_NUM];
    unsigned char reset_buf[RESET_BUF_LEN];
    unsigned char period_buf[RESET_BUF_LEN * 2 + LED_NUM * 9];
    unsigned char spi_arr[LED_NUM * 9];

    memset(colors, 0x00, sizeof(colors));
    colors[0] = rgb1;
    colors[1] = rgb2;

    memset(spi_arr, 0x00, sizeof(spi_arr));
    color2spi(colors, spi_arr);

    memset(reset_buf, 0xFF, RESET_BUF_LEN);
    memcpy(period_buf, reset_buf, RESET_BUF_LEN);//reset led before
    memcpy(period_buf + RESET_BUF_LEN, spi_arr, LED_NUM * 9);//set color
    memcpy(period_buf + LED_NUM * 9 + RESET_BUF_LEN, reset_buf, RESET_BUF_LEN);//reset led after

    write(Fd_led_spi, period_buf, RESET_BUF_LEN * 2 + LED_NUM * 9);

    //dbg_print_led_colors(colors);
    //dbg_print_spi_arr(spi_arr);
}

#if 0
void $shm$;
#endif



static inline void set_color_and_delay(unsigned rgb1, unsigned rgb2, unsigned us)
{
    if(us)
    {
        set_led_color(rgb1, rgb2);
        usleep(us);
    }
}

static inline unsigned int get_rgbhex_by_brightness(unsigned int hex, unsigned int brightness)
{
    return  ((((hex & 0xFF0000) >> 16) * brightness / LED_BRIGHTNESS_MAX) << 16 )
        |   ((((hex & 0xFF00) >> 8) * brightness / LED_BRIGHTNESS_MAX) <<  8)
        |   ((((hex & 0xFF) >> 0) * brightness / LED_BRIGHTNESS_MAX) <<  0);
}

static void led_loop_color_and_bright_by_pwm(void)
{
    //way 1: make pwm, used in the situation of only two LED
    //more accurate colors than way 2, but pwm will consume more resources of CPU
    const unsigned int rate = LED_LOOP_PERIOD_US / LED_BRIGHTNESS_MAX;

    unsigned int slice1 = 0;
    unsigned int slice2 = 0;
    unsigned int slice3 = 0;

    if(Led_brightness[0] == 0)
        Led_colors[0] = 0;

    if(Led_brightness[1] == 0)
        Led_colors[1] = 0;

    if(Led_brightness[0] > Led_brightness[1])
    {
        slice1 = Led_brightness[1];
        slice2 = Led_brightness[0] - Led_brightness[1];
        slice3 = LED_BRIGHTNESS_MAX - Led_brightness[0];

        set_color_and_delay(Led_colors[0], Led_colors[1], slice1 * rate);
        set_color_and_delay(Led_colors[0], 0, slice2 * rate);
        set_color_and_delay(0, 0, slice3 * rate);
    }
    else //Led_brightness[0] > Led_brightness[1]
    {
        slice1 = Led_brightness[0];
        slice2 = Led_brightness[1] - Led_brightness[0];
        slice3 = LED_BRIGHTNESS_MAX - Led_brightness[1];

        set_color_and_delay(Led_colors[0], Led_colors[1], slice1 * rate);
        set_color_and_delay(0, Led_colors[1], slice2 * rate);
        set_color_and_delay(0, 0, slice3 * rate);
    }
}

static void led_loop_color_and_bright_by_rgbhex(void)
{
    //way 2: make rgb hex val to adjust brightness
    //it was smoothly then way 1, but color will lose accuracy(almost no influence)
    unsigned int color1 = 0;
    unsigned int color2 = 0;

    color1 = get_rgbhex_by_brightness(Led_colors[0], Led_brightness[0]);
    color2 = get_rgbhex_by_brightness(Led_colors[1], Led_brightness[1]);

    set_color_and_delay(color1, color2, LED_LOOP_PERIOD_US);
}

static void led_loop_mode_normal(unsigned int cnt, unsigned int led_index)
{
    //always turn on
    Led_colors[led_index] = Shared_ptr->data[led_index].color;
    Led_brightness[led_index] = Shared_ptr->data[led_index].brightness;
}

static void led_loop_mode_breath(unsigned int cnt, unsigned int led_index)
{
    //brightness 0 ~ set_val ~ 0 ~ set_val ~ 0 ....
    Led_colors[led_index] = Shared_ptr->data[led_index].color;
    if(cnt < (LED_COUNT_PERIOD / 2))    //brightness up
        Led_brightness[led_index] = Shared_ptr->data[led_index].brightness * cnt / (LED_COUNT_PERIOD / 2);
    else //(cnt >= (LED_COUNT_PERIOD / 2))  //brightness down
        Led_brightness[led_index] = Shared_ptr->data[led_index].brightness * (LED_COUNT_PERIOD - 1 - cnt) / (LED_COUNT_PERIOD / 2);
}

static void led_loop_mode_flash(unsigned int cnt, unsigned int led_index)
{
    //turn on ... off ... on ... off ...

    if((cnt / (LED_FLASH_PERIOD_US / LED_LOOP_PERIOD_US)) % 2)
    {
        Led_colors[led_index] = Shared_ptr->data[led_index].color;
        Led_brightness[led_index] = Shared_ptr->data[led_index].brightness;
    }
    else
    {
        Led_colors[led_index] = 0;
        Led_brightness[led_index] = 0;
    }
}

static inline void led_loop(void)
{
    static unsigned int cnt = 0;   //range : 0 ~ (LED_COUNT_PERIOD - 1)

    //deal with mode
    for(int i = 0; i < LED_NUM; ++i)
    {
        switch(Shared_ptr->data[i].mode)
        {
            case LED_MODE_BREATH:
                led_loop_mode_breath(cnt, i);
                break;
            case LED_MODE_FLASH:
                led_loop_mode_flash(cnt, i);
                break;
            case LED_MODE_NORMAL:
            default:
                led_loop_mode_normal(cnt, i);
                break;
        }
    }

    //led_loop_color_and_bright_by_pwm();
    led_loop_color_and_bright_by_rgbhex();

    ++cnt;
    if(cnt >= LED_COUNT_PERIOD)
        cnt = 0;
}

int init_shared_ptr(void)
{
    Shared_ptr = NULL;
    Shared_ptr = (t_led_shared_data *)get_led_shm_addr();

    if(-1 + APP_SHM_ADDR_LED == (size_t)Shared_ptr)
    {
        printf("shm app ptr not ready, init shared ptr failed\n");
        return RET_ERROR;
    }

    return RET_SUCC;
}

int clean_shared_ptr(void)
{
    if(-1 + APP_SHM_ADDR_LED == (size_t)Shared_ptr)
    {
        printf("shm app ptr not ready, don't need clean\n");
        return RET_ERROR;
    }

    memset(Shared_ptr, '\0', sizeof(t_led_shared_data));
    Shared_ptr = NULL;
    printf("shared_ptr memory and ptr cleared\n");

    return RET_SUCC;
}

void sig_handle(int no)
{
    switch(no)
    {
        case SIGKILL:
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            set_led_color(0, 0);
            clean_shared_ptr();
			close_shm();
			close_led_spi();
            break;
        default:
            break;
    }

	exit(0);
}

void init_sig(void)
{
    signal(SIGKILL, sig_handle);
    signal(SIGQUIT, sig_handle);
    signal(SIGINT,  sig_handle);
    signal(SIGTERM, sig_handle);
}


int main(int argc, char **argv)
{
    int ret;
    int fd;
    int speed = RGB_SPI_SPEED;
    unsigned char resetBuf[RESET_BUF_LEN];
    unsigned char periodBuf[RESET_BUF_LEN * 2 + LED_NUM * 9];

    init_sig();
    if(init_led_spi())      goto main_error_ret;
    if(init_shm())          goto main_error_api;
    if(init_shared_ptr())   goto main_error_shm;

    memcpy(Shared_ptr, &Shared_dat_default, sizeof(t_led_shared_data));

    if(argc > 1)
    {
        str2color(argv[1], strlen(argv[1]), Led_colors);
        for(int i = 0; i < LED_NUM; ++i)
        {
            Shared_ptr->data[i].color = Led_colors[i];
        }
    }

    if(argc > 2)
    {
        LightLevel = atoi(argv[2]);
        LightLevel = ABS(LightLevel);
        for(int i = 0; i < LED_NUM; ++i)
        {
            Led_brightness[i] = LightLevel;
            Shared_ptr->data[i].brightness = LightLevel;
        }
        printf("default bright: %d/100\n", LightLevel);
    }

    set_led_color(0, 0);//init no light

    for(;;)
    {
        led_loop();
    }

    clean_shared_ptr();
main_error_shm:
    close_shm();
main_error_api:
    close_led_spi();
main_error_ret:

    return 0;
}


//#ifndef _MAKE_SO_

int main_bak(int argc, char **argv)
{
    int ret;
    int fd;
    int speed = RGB_SPI_SPEED;
    unsigned char resetBuf[RESET_BUF_LEN];
    unsigned char periodBuf[RESET_BUF_LEN * 2 + LED_NUM * 9];

    if(argc < 2)
    {
        printf("usage:\n");
        printf("./a.out [BRG,BRG,BRG] [brightness 0 ~ 100]\n");
        printf("./a.out 00FF11,33FFCC,BBEE55 50\n");
        return 0;
    }

    if(argc == 3)
    {
        LightLevel = atoi(argv[2]);
        LightLevel = ABS(LightLevel);
        printf("bright: %d/100\n", LightLevel);
    }

    str2color(argv[1], strlen(argv[1]), Led_colors);

    init_led_spi();

    set_led_color(0, 0);//no light
    set_led_color(Led_colors[0], Led_colors[1]);//target light

    sleep(1);

    printf("bright test\n");
    for(int i = 0; i < 10000; ++i)
    {
        static int step = 0;
        if(0 != LightLevel)
            set_led_color(Led_colors[0], Led_colors[1]);
        usleep(LightLevel * 100);
        if(100 != LightLevel)
            set_led_color(0, 0);
        usleep((100 - LightLevel) * 100);

        if(step++ > 5)
        {
            LightLevel += 1;
            step = 0;
        }

        if(LightLevel > 35)
        {
            unsigned tmp = Led_colors[0];
            Led_colors[0] = Led_colors[1];
            Led_colors[1] = tmp;
            LightLevel = 0;
        }

    }

    //for(int i = 0; i < 100000; ++i)
    //{
    //    sendbuf_clean();
    //    sendbuf_add_color(Led_colors[0], Led_colors[1], LightLevel * 10);
    //    sendbuf_add_nolight(1000 - LightLevel * 10);
    //    sendbuf_to_spi();
    //}

    close_led_spi();

    return 0;
}

//#endif
