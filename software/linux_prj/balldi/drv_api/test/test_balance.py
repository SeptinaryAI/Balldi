#!/usr/bin/python3

from ctypes import *
import time
import signal

# UART CMD
CMD_GET_LEFT_PWM        = 0x10
CMD_GET_RIGHT_PWM       = 0x11
CMD_GET_FLY_PWM         = 0x12
CMD_GET_SERVO_PWM       = 0x13
CMD_GET_LEFT_SPEED      = 0x20
CMD_GET_RIGHT_SPEED     = 0x21
CMD_GET_FLY_SPEED       = 0x22
CMD_GET_BAT_VOLTAGE     = 0x30
CMD_GET_CHRG_VOLTAGE    = 0x31
CMD_GET_PROG_RESIS      = 0x32
CMD_GET_IS_CHAR         = 0x33
CMD_GET_IS_STDBY        = 0x34
CMD_SET_LEFT_PWM        = 0xA0
CMD_SET_RIGHT_PWM       = 0xA1
CMD_SET_FLY_PWM         = 0xA2
CMD_SET_SERVO_PWM       = 0xA3
CMD_WHEEL_ESTOP         = 0xB0
CMD_FLY_ESTOP           = 0xB2
CMD_SET_PROG_RESIS      = 0xC2

USRCMD_SET_GO_STRAIGHT  = 0xD0
USRCMD_SET_TURN_LR      = 0xD1

USRCMD_BALANCE          = 0xD7

def send_cmd(dll, cmd, val_send):
    func = dll.send_cmd

    func.argtypes = [c_int, c_int, POINTER(c_int)]
    func.restype  = c_int

    val_ret = c_int(0)

    ret = func(cmd, val_send, byref(val_ret))
    print('cmd: '+ str(cmd) + ' , val_ret: '+ str(val_ret.value) + ' , ret: ' + str(ret))
    return val_ret.value

def signal_kill(signum, frame):
    dll = CDLL("./drv_api.so")
    dll.init_api()
    send_cmd(dll, USRCMD_SET_GO_STRAIGHT, 0)
    send_cmd(dll, USRCMD_SET_TURN_LR, 0)
    send_cmd(dll, CMD_GET_LEFT_PWM, 0)
    send_cmd(dll, CMD_GET_RIGHT_PWM, 0)
    send_cmd(dll, CMD_GET_FLY_PWM, 0)
    send_cmd(dll, USRCMD_BALANCE, 0)
    exit()

def main():
    dll = CDLL("./drv_api.so")
    dll.init_api()
    signal.signal(signal.SIGINT, signal_kill)

    send_cmd(dll, USRCMD_BALANCE, 0)
    time.sleep(5)
    send_cmd(dll, USRCMD_BALANCE, 1)

    for i in range(100000):
        print('wait')
        time.sleep(5)

    dll.close_api()


if __name__ == '__main__':
    main()
