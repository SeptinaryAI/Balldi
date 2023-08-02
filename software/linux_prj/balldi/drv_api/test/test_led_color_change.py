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

color_list = [
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
]

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

    send_cmd(dll, USRCMD_SET_LED1_BRIGHTNESS, 5000)
    send_cmd(dll, USRCMD_SET_LED1_MODE, LED_MODE_BREATH)
    send_cmd(dll, USRCMD_SET_LED2_BRIGHTNESS, 5000)
    send_cmd(dll, USRCMD_SET_LED2_MODE, LED_MODE_BREATH)

    send_cmd(dll, USRCMD_SET_LED1_COLOR, 0)
    send_cmd(dll, USRCMD_SET_LED2_COLOR, 0)

    time.sleep(1)

    b = 0
    r = 0
    g = 0
    target_b = 255
    target_r = 0
    target_g = 0

    for i in range(1,256):
        B = b + (target_b - b) * i / 255
        R = r + (target_r - r) * i / 255
        G = g + (target_g - g) * i / 255
        BRG = (int(B) << 16) + (int(R) << 8) + int(G)
        send_cmd(dll, USRCMD_SET_LED1_COLOR, BRG)
        send_cmd(dll, USRCMD_SET_LED2_COLOR, BRG)
        time.sleep(0.01)

    b = 255
    r = 0
    g = 0
    target_b = 0
    target_r = 255
    target_g = 0

    for i in range(1,256):
        B = b + (target_b - b) * i / 255
        R = r + (target_r - r) * i / 255
        G = g + (target_g - g) * i / 255
        BRG = (int(B) << 16) + (int(R) << 8) + int(G)
        send_cmd(dll, USRCMD_SET_LED1_COLOR, BRG)
        send_cmd(dll, USRCMD_SET_LED2_COLOR, BRG)
        time.sleep(0.01)

    b = 0
    r = 255
    g = 0
    target_b = 255
    target_r = 0
    target_g = 255

    for i in range(1,256):
        B = b + (target_b - b) * i / 255
        R = r + (target_r - r) * i / 255
        G = g + (target_g - g) * i / 255
        BRG = (int(B) << 16) + (int(R) << 8) + int(G)
        send_cmd(dll, USRCMD_SET_LED1_COLOR, BRG)
        send_cmd(dll, USRCMD_SET_LED2_COLOR, BRG)
        time.sleep(0.01)

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
