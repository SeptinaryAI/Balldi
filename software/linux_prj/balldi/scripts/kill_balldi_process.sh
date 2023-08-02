#!/usr/bin/bash

kill1=$(ps aux | grep drv_server.out | grep -v grep | tr -s ' ' | cut -d ' ' -f 2)
kill2=$(ps aux | grep web_serv_gin   | grep -v grep | tr -s ' ' | cut -d ' ' -f 2)
kill3=$(ps aux | grep mpu6050.out    | grep -v grep | tr -s ' ' | cut -d ' ' -f 2)
kill4=$(ps aux | grep ledapp         | grep -v grep | tr -s ' ' | cut -d ' ' -f 2)
kill4=$(ps aux | grep tofapp         | grep -v grep | tr -s ' ' | cut -d ' ' -f 2)

if [ -n "$kill1" ];then
    echo 'find drv_server.out'
    echo 'kill drv_server.out ...'
    kill -15 ${kill1}
fi

if [ -n "$kill2" ];then
    echo 'find web_serv_gin'
    echo 'kill web_serv_gin ...'
    kill -15 ${kill2}
fi

if [ -n "$kill3" ];then
    echo 'find mpu6050.out'
    echo 'kill mpu6050.out ...'
    kill -15 ${kill3}
fi

if [ -n "$kill4" ];then
    echo 'find ledapp'
    echo 'kill ledapp ...'
    kill -15 ${kill4}
fi

if [ -n "$kill5" ];then
    echo 'find tofapp'
    echo 'kill tofapp...'
    kill -15 ${kill3}
fi
