#!/usr/bin/python3

import sys
import os
import time
from ctypes import *
from socket import socket, AF_INET, SOCK_STREAM

scripts_path = os.path.dirname(__file__)

def parse_led_clock(act, adj):
    cmd_ret = os.system('ps aux | grep IoT_serv.py | grep -v grep')

    if cmd_ret != 0:
        os.system('nohup python3 ' + scripts_path + '/../IoT_serv.py >/dev/null 2>&1 &')
        time.sleep(0.5)

    s = socket(AF_INET, SOCK_STREAM)
    s.connect(('localhost',8081))
    if '关' in act:
        s.send(b'device_led_clock_set_stat:1')
    elif '开' in act:
        s.send(b'device_led_clock_set_stat:0')
    s.close()



if __name__ == '__main__':
    if len(sys.argv) <= 3:
        print('script request 3 args !!')
        exit()

    args = []
    for i in range(1, len(sys.argv)):
        args.append((sys.argv[i]))

    obj = args[0]
    act = args[1]
    adj = args[2]

    parse_led_clock(act, adj)

    
