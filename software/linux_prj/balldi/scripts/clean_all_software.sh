#!/bin/bash

pwd=$(cd $(dirname $0); pwd)

echo "clean ledapp...."
cd $pwd/../led_app
make clean

echo "clean mpu6050...."
cd $pwd/../imu_app
make clean

echo "clean drv_server...."
cd $pwd/../drv_server
make clean

echo "clean drv_api...."
cd $pwd/../drv_api
make clean

echo "clean cam_api...."
cd $pwd/../cam_v4l2_api
make clean

echo "clean tof_app...."
cd $pwd/../tof_app
make clean

echo "clean mnn_yolo_lite...."
cd $pwd/../NN/yolo_lite/mnn/
./clean.sh

echo "clean web_serv_gin...."
cd $pwd/../web_serv_gin
make clean

