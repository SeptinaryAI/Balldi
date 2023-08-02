#!/usr/bin/python3

import sys
import os
from ctypes import *

scripts_path = os.path.dirname(__file__)

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


def parse_led(act, adj):
    color = 0x000000

    if '开' in act: #open
        if '红' in adj: #red    FF0000
            color = 0x00FF00
        elif '绿' in adj: #green  00FF00
            color = 0xFF0000
        elif '蓝' in adj: #blue   0000FF
            color = 0x0000FF
        else:
            color = 0xFFFFFF

    if '设' in act or '调' in act:
        if '红' in adj: #red    FF0000
            color = 0x00FF00
        elif '绿' in adj: #green  00FF00
            color = 0xFF0000
        elif '蓝' in adj: #blue   0000FF
            color = 0x0000FF
        else:
            color = 0xFFFFFF

    if '关' in act: #close
        color = 0x000000

    if '1' in adj or '一' in adj:
        send_cmd(dll, USRCMD_SET_LED1_BRIGHTNESS, 5000)
        send_cmd(dll, USRCMD_SET_LED1_MODE, LED_MODE_NORMAL)
        send_cmd(dll, USRCMD_SET_LED1_COLOR, color)
    elif '2' in adj or '二' in adj:
        send_cmd(dll, USRCMD_SET_LED2_BRIGHTNESS, 5000)
        send_cmd(dll, USRCMD_SET_LED2_MODE, LED_MODE_NORMAL)
        send_cmd(dll, USRCMD_SET_LED2_COLOR, color)
    else:
        send_cmd(dll, USRCMD_SET_LED1_BRIGHTNESS, 5000)
        send_cmd(dll, USRCMD_SET_LED1_MODE, LED_MODE_NORMAL)
        send_cmd(dll, USRCMD_SET_LED1_COLOR, color)
        send_cmd(dll, USRCMD_SET_LED2_BRIGHTNESS, 5000)
        send_cmd(dll, USRCMD_SET_LED2_MODE, LED_MODE_NORMAL)
        send_cmd(dll, USRCMD_SET_LED2_COLOR, color)


if __name__ == '__main__':
    if len(sys.argv) <= 3:
        print('script request 3 args !!')
        exit()

    args = []
    for i in range(1, len(sys.argv)):
        args.append((sys.argv[i]))

    dll = CDLL(scripts_path + '/../../drv_api/build/drv_api.so')
    dll.init_api()

    obj = args[0]
    act = args[1]
    adj = args[2]

    parse_led(act, adj)

    
