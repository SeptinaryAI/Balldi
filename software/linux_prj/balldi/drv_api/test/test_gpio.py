#!/usr/bin/python3

from ctypes import *
import time

# UART CMD
CMD_SET_LINEOUT_ONOFF     =  0xD8
CMD_SET_DRIVER_BOARD_RST  =  0xD9

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

    send_cmd(dll, CMD_SET_DRIVER_BOARD_RST, 1)
    time.sleep(2)


    # play wav to test audio lineout
    send_cmd(dll, CMD_SET_LINEOUT_ONOFF, 1)
    time.sleep(5)
    send_cmd(dll, CMD_SET_LINEOUT_ONOFF, 0)
    time.sleep(5)
    send_cmd(dll, CMD_SET_LINEOUT_ONOFF, 1)

    dll.close_api()


if __name__ == '__main__':
    main()
