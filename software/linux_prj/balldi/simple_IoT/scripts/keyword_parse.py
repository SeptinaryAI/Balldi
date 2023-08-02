#!/usr/bin/python3
#A simple IoT devices control implementation
#Chinese only
import sys
import os
import time

scripts_path = os.path.dirname(__file__)

action_list = [
    '开', #open
    '关', #close
    '看', #watch
    '设置', #set
    '调', #set
    '移动', #move
    '进', #go forward
    '退', #go back
    '转', #turn round
    '收起', #retract
    '弹出', #eject   .... maybe not very appropriate
]

adj_list = [
    '左', #left
    '右', #right
    '上', #up
    '下', #down
    '前', #forward
    '后', #back
    '红', #red    FF0000
    '绿', #green  00FF00
    '蓝', #blue   0000FF
    '一', #1
    '二', #2
    '1', #1
    '2', #2
]

object_list = [
    '灯',   #light
    'LED',  #led
    '闹钟', #clock
    '时钟', #clock
    '尾巴', #tail
]

conn_list = [
    '然后', #then
]

script_map_act = {
    '开':'', #open
    '关':'', #close
    '看':'action_move.py', #watch
    '设置':'', #set
    '调':'', #set
    '移动':'action_move.py', #move
    '进':'action_move.py', #go forward
    '退':'action_move.py', #go back
    '转':'action_move.py', #turn round
    '收起':'action_move.py', #retract
    '弹出':'action_move.py', #eject
}

script_map_obj = {
    '灯':'action_led.py',
    'LED':'action_led.py',
    '闹钟':'action_led_clock.py',
    '时钟':'action_led_clock.py',
    '尾巴':'action_move.py', #tail
}


def send_action_by_obj(obj, obj_in, act_in, adj_in):
    if len(act_in) == 0 and len(adj_in) == 0:
        print('What do you want to do ?')
        return

    cmd = ''

    str_obj = ' []'
    str_act = ' []'
    str_adj = ' []'
    if len(obj_in) != 0:
        str_obj = ' ' + ','.join(obj_in)
    if len(act_in) != 0:
        str_act = ' ' + ','.join(act_in)
    if len(adj_in) != 0:
        str_adj = ' ' + ','.join(adj_in)

    cmd = 'python3 ' + scripts_path + '/' + script_map_obj[obj] + str_obj + str_act + str_adj

    os.system(cmd)

def send_action_by_act(act, obj_in, act_in, adj_in):
    if len(script_map_act[act]) == 0:
        print('Not found script for this action .')
        return

    if len(act_in) == 0 and len(adj_in) == 0:
        print('What do you want to do ?')
        return

    str_obj = ' []'
    str_act = ' []'
    str_adj = ' []'
    if len(obj_in) != 0:
        str_obj = ' ' + ','.join(obj_in)
    if len(act_in) != 0:
        str_act = ' ' + ','.join(act_in)
    if len(adj_in) != 0:
        str_adj = ' ' + ','.join(adj_in)

    cmd = 'python3 ' + scripts_path + '/' + script_map_act[act] + str_obj + str_act + str_adj

    os.system(cmd)

def operation_parse(str):
    act_in = []
    adj_in = []
    obj_in = []

    for wd in action_list:
        if wd in str:
            act_in.append(wd)

    for wd in adj_list:
        if wd in str:
            adj_in.append(wd)

    for wd in object_list:
        if wd in str:
            obj_in.append(wd)

    print('----')
    print(obj_in)
    print(act_in)
    print(adj_in)
    print('----')
    if len(obj_in) == 0 and len(act_in) == 1:
        send_action_by_act(act_in[0], obj_in, act_in, adj_in)
        return

    if len(obj_in) == 1:
        send_action_by_obj(obj_in[0], obj_in, act_in, adj_in)
        return
        

def slice_str_by_conn(str):
    slices = args[0].split()
    for conn in conn_list:
        temp_slices = []
        for sli in slices:
            temp_slices += sli.split(conn)
        slices = temp_slices
    return slices


if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print('script request args !!')
        exit()

    args = []
    for i in range(1, len(sys.argv)):
        args.append((sys.argv[i]))

    slices = slice_str_by_conn(args[0])
    print(slices)

    for sli in slices:
        operation_parse(sli)
        time.sleep(1)


