#!/usr/bin/python3

from ctypes import *
import time

# UART CMD
CMD_GET_DISTANCE = 0x50

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

    for i in range(1000):
        send_cmd(dll, CMD_GET_DISTANCE, 0)
        time.sleep(0.05)

    dll.close_api()


if __name__ == '__main__':
    main()
