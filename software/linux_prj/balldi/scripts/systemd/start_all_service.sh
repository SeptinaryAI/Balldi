#!/bin/bash

FILE_DRVSERV=balldi-drvserv.service
FILE_IMU=balldi-imu.service
FILE_TOF=balldi-tof.service
FILE_LED=balldi-led.service
FILE_WEB=balldi-web.service

systemctl stop $FILE_DRVSERV
systemctl stop $FILE_IMU
systemctl stop $FILE_TOF
systemctl stop $FILE_LED
systemctl stop $FILE_WEB

systemctl start $FILE_DRVSERV
systemctl start $FILE_IMU
systemctl start $FILE_TOF
systemctl start $FILE_LED
systemctl start $FILE_WEB
