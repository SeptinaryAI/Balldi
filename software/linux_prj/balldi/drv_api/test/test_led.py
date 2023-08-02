#!/usr/bin/python3

from ctypes import *
import time

# UART CMD
USRCMD_SET_LED1_COLOR       = 0XE0
USRCMD_SET_LED1_BRIGHTNESS  = 0XE1
USRCMD_SET_LED1_MODE        = 0XE2
USRCMD_SET_LED2_COLOR       = 0XE3
USRCMD_SET_LED2_BRIGHTNESS  = 0XE4
USRCMD_SET_LED2_MODE        = 0XE5

LED_MODE_NORMAL             = 0
LED_MODE_BREATH             = 1
LED_MODE_FLASH              = 2

def send_cmd(dll, cmd, val_send):
    func = dll.send_cmd

    func.argtypes = [c_int, c_int, POINTER(c_int)]
    func.restype  = c_int

    val_ret = c_int(0)

    ret = func(cmd, val_send, byref(val_ret))
    #print('cmd: '+ str(cmd) + ' , val_ret: '+ str(val_ret.value) + ' , ret: ' + str(ret))
    return val_ret.value

def main():
    dll = CDLL("./drv_api.so")
    dll.init_api()

    send_cmd(dll, USRCMD_SET_LED1_BRIGHTNESS, 10000)
    send_cmd(dll, USRCMD_SET_LED1_MODE, LED_MODE_BREATH)
    send_cmd(dll, USRCMD_SET_LED2_BRIGHTNESS, 10000)
    send_cmd(dll, USRCMD_SET_LED2_MODE, LED_MODE_BREATH)


    color1 = 0xEE1111
    color2 = 0xEE1111

    send_cmd(dll, USRCMD_SET_LED1_COLOR, color1)
    send_cmd(dll, USRCMD_SET_LED2_COLOR, color2)
    time.sleep(10)


    send_cmd(dll, USRCMD_SET_LED1_COLOR, 0x000000)
    send_cmd(dll, USRCMD_SET_LED1_BRIGHTNESS, 0)
    send_cmd(dll, USRCMD_SET_LED1_MODE, LED_MODE_NORMAL)
    send_cmd(dll, USRCMD_SET_LED2_COLOR, 0x000000)
    send_cmd(dll, USRCMD_SET_LED2_BRIGHTNESS, 0)
    send_cmd(dll, USRCMD_SET_LED2_MODE, LED_MODE_NORMAL)

    dll.close_api()


if __name__ == '__main__':
    main()
