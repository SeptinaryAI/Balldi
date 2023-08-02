#!/usr/bin/bash

pwd=$(cd $(dirname $0); pwd)

echo "make ledapp...."
cd $pwd/../led_app
make

echo "make mpu6050...."
cd $pwd/../imu_app
make

echo "make tof_app...."
cd $pwd/../tof_app
make

echo "make drv_server...."
cd $pwd/../drv_server
make

echo "make drv_api...."
cd $pwd/../drv_api
make
#cp $pwd/../drv_api/build/libdrvapi.a $pwd/../web_serv_gin/drv/

echo "make mnn_yolo_lite"
cd $pwd/../NN/yolo_lite/mnn/
./make.sh

echo "make cam_api...."
cd $pwd/../cam_v4l2_api
make
#cp $pwd/../cam_v4l2_api/build/libcamv4l2api.a $pwd/../web_serv_gin/cam/

echo "make web_serv_gin...."
cd $pwd/../web_serv_gin
make

cd ${pwd}
