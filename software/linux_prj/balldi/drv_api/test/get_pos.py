#!/usr/bin/python3

from ctypes import *
import time

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

USRCMD_GET_POS_PITCH    = 0x40
USRCMD_GET_POS_ROLL     = 0x41
USRCMD_GET_POS_YAW      = 0x42
USRCMD_GET_GYRO1        = 0x43
USRCMD_GET_GYRO2        = 0x44
USRCMD_GET_GYRO3        = 0x45
USRCMD_GET_ACCEL1       = 0x46
USRCMD_GET_ACCEL2       = 0x47
USRCMD_GET_ACCEL3       = 0x48

def send_cmd(dll, cmd, val_send):
    func = dll.send_cmd

    func.argtypes = [c_int, c_int, POINTER(c_int)]
    func.restype  = c_int

    val_ret = c_int(0)
    sig_val_ret = 0

    ret = func(cmd, val_send, byref(val_ret))
    if val_ret.value > 0x7FFF:
        sig_val_ret = -(0xFFFF - val_ret.value)
    else:
        sig_val_ret = val_ret.value
    print('cmd: '+ str(cmd) + ' , val_ret: '+ str(val_ret.value) + ' , sig val: ' + str(sig_val_ret) + ' , ret: ' + str(ret))
    return ret

def main():
    dll = CDLL("./drv_api.so")
    dll.init_api()

    cnt = 0

    while True:
        if send_cmd(dll, USRCMD_GET_POS_PITCH, 0) != 0:
            break
        if send_cmd(dll, USRCMD_GET_POS_ROLL, 0) != 0:
            break
        if send_cmd(dll, USRCMD_GET_POS_YAW, 0) != 0:
            break
        if send_cmd(dll, USRCMD_GET_GYRO1, 0) != 0:
            break
        print('cnt: '+str(cnt))
        cnt = cnt + 1
        time.sleep(0.2)

    dll.close_api()


if __name__ == '__main__':
    main()