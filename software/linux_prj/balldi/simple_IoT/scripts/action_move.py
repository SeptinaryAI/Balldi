#!/usr/bin/python3

import time
import sys
import os
from ctypes import *

scripts_path = os.path.dirname(__file__)

USRCMD_SET_GO_STRAIGHT  = 0xD0
USRCMD_SET_TURN_LR      = 0xD1
CMD_SET_SERVO_TARGET_TICK   = 0xA1
CMD_SET_SERVO2_TARGET_TICK  = 0xA2


def send_cmd(dll, cmd, val_send):
    func = dll.send_cmd

    func.argtypes = [c_int, c_int, POINTER(c_int)]
    func.restype  = c_int

    val_ret = c_int(0)

    ret = func(cmd, val_send, byref(val_ret))
    #print('cmd: '+ str(cmd) + ' , val_ret: '+ str(val_ret.value) + ' , ret: ' + str(ret))
    return val_ret.value


def parse_move(obj, act, adj):
    if '看' in act:
        if '上' in adj:
            send_cmd(dll, CMD_SET_SERVO2_TARGET_TICK, 20)
        elif '下' in adj: #open
            send_cmd(dll, CMD_SET_SERVO2_TARGET_TICK, 80)
        elif '前' in adj: #open
            send_cmd(dll, CMD_SET_SERVO2_TARGET_TICK, 50)
        elif '左' in adj:
            send_cmd(dll, USRCMD_SET_TURN_LR, -50)
            time.sleep(0.2)
            send_cmd(dll, USRCMD_SET_TURN_LR, 0)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)
        elif '右' in adj:
            send_cmd(dll, USRCMD_SET_TURN_LR, 50)
            time.sleep(0.2)
            send_cmd(dll, USRCMD_SET_TURN_LR, 0)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)

    elif '进' in act :
        if '前' in adj:
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 50)
            time.sleep(0.5)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)

    elif '退' in act :
        if '后' in adj:
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, -50)
            time.sleep(0.5)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)

    elif '移动' in act or '运动' in act:
        if '前' in adj: 
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 50)
            time.sleep(0.5)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)
        elif '后' in adj:
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, -50)
            time.sleep(0.5)
            send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)

    elif '转' in act:
        if '左' in adj:
            send_cmd(dll, USRCMD_SET_TURN_LR, -50)
            time.sleep(0.2)
            send_cmd(dll, USRCMD_SET_TURN_LR, 0)
        elif '右' in adj:
            send_cmd(dll, USRCMD_SET_TURN_LR, 50)
            time.sleep(0.2)
            send_cmd(dll, USRCMD_SET_TURN_LR, 0)

    elif '收起' in act:
        if '尾' in obj:
            send_cmd(dll, CMD_SET_SERVO_TARGET_TICK, 80)

    elif '弹出' in act:
        if '尾' in obj:
            send_cmd(dll, CMD_SET_SERVO_TARGET_TICK, 28)


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

    parse_move(obj, act, adj)

    
