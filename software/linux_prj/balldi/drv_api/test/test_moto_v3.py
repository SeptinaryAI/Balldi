#!/usr/bin/python3

from ctypes import *
import time

# UART CMD
CMD_SET_PERIOD_LR     =  0xA4
CMD_SET_DIR_LR        =  0xA5
CMD_SET_DIR_L         =  0xA6
CMD_SET_DIR_R         =  0xA7
CMD_SET_EN_LR         =  0xA8
CMD_SET_EN_L          =  0xA9
CMD_SET_EN_R          =  0xAA

CMD_GET_LEFT_COUNT    =  0x22
CMD_GET_RIGHT_COUNT   =  0x23

def send_cmd(dll, cmd, val_send):
    func = dll.send_cmd

    func.argtypes = [c_int, c_int, POINTER(c_int)]
    func.restype  = c_int

    val_ret = c_int(0)

    ret = func(cmd, val_send, byref(val_ret))
    print('cmd: '+ str(cmd) + ' , val_ret: '+ str(val_ret.value) + ' , ret: ' + str(ret))
    return val_ret.value

def main():
    dll = CDLL("./drv_api.so")
    dll.init_api()

    send_cmd(dll, CMD_GET_LEFT_COUNT, 0)
    send_cmd(dll, CMD_GET_RIGHT_COUNT, 0)

    send_cmd(dll, CMD_SET_PERIOD_LR, 90)
    send_cmd(dll, CMD_SET_DIR_L, 0x0000)
    send_cmd(dll, CMD_SET_DIR_R, 0x0000)
    send_cmd(dll, CMD_SET_DIR_LR, 0x0000)
    send_cmd(dll, CMD_SET_EN_L, 0x0000)
    send_cmd(dll, CMD_SET_EN_R, 0x0000)

    time.sleep(10)
    send_cmd(dll, CMD_SET_EN_L, 0x0001)
    send_cmd(dll, CMD_SET_EN_R, 0x0001)
    send_cmd(dll, CMD_SET_PERIOD_LR, 20)
    time.sleep(10)

    send_cmd(dll, CMD_SET_PERIOD_LR, 100)
    time.sleep(10)
    send_cmd(dll, CMD_SET_EN_L, 0x0000)
    send_cmd(dll, CMD_SET_EN_R, 0x0000)
    send_cmd(dll, CMD_GET_LEFT_COUNT, 0)
    send_cmd(dll, CMD_GET_RIGHT_COUNT, 0)

    #send_cmd(dll, CMD_SET_EN_LR, 0x0000)

    time.sleep(1)


    dll.close_api()


if __name__ == '__main__':
    main()
