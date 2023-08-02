#!/usr/bin/bash

set -e

PWD=$(cd `dirname $0`; pwd)
PRJ_DIR=${PWD}/..
echo 'project dir: '${PRJ_DIR}

echo 'kill all process about balldi ...'
${PWD}/kill_balldi_process.sh

# balldi moto driver service
echo 'start drv_server ...'
nohup ${PRJ_DIR}/drv_server/build/drv_server.out >/dev/null 2>&1 &

sleep 3

echo 'start mpu6050 driver...'
nohup ${PRJ_DIR}/imu_app/build/mpu6050.out >/dev/null 2>&1 &

echo 'start tof...'
nohup ${PRJ_DIR}/tof_app/build/tofapp >/dev/null 2>&1 &

echo 'start led...'
nohup ${PRJ_DIR}/led_app/build/ledapp >/dev/null 2>&1 &

# balldi Gin web service
echo 'start Gin web service...'
cd ${PRJ_DIR}/web_serv_gin/
nohup ./web_serv_gin >/dev/null 2>&1 &
cd ${PWD}

exit 0

