#!/bin/bash

set -e

PWD=$(cd `dirname $0`; pwd)
BALLDI_DIR=$(cd $PWD/../..; pwd)
SERVICE_DIR=/lib/systemd/system
SERVICE_LN_DIR=/etc/systemd/system

FILE_DRVSERV=balldi-drvserv.service
FILE_IMU=balldi-imu.service
FILE_TOF=balldi-tof.service
FILE_LED=balldi-led.service
FILE_WEB=balldi-web.service

rm -f $SERVICE_DIR/$FILE_DRVSERV
rm -f $SERVICE_DIR/$FILE_IMU
rm -f $SERVICE_DIR/$FILE_TOF
rm -f $SERVICE_DIR/$FILE_LED
rm -f $SERVICE_DIR/$FILE_WEB
rm -f $SERVICE_LN_DIR/$FILE_DRVSERV
rm -f $SERVICE_LN_DIR/$FILE_IMU
rm -f $SERVICE_LN_DIR/$FILE_TOF
rm -f $SERVICE_LN_DIR/$FILE_LED
rm -f $SERVICE_LN_DIR/$FILE_WEB

cp $PWD/${FILE_DRVSERV}_   $SERVICE_DIR/$FILE_DRVSERV
cp $PWD/${FILE_IMU}_       $SERVICE_DIR/$FILE_IMU
cp $PWD/${FILE_TOF}_       $SERVICE_DIR/$FILE_TOF
cp $PWD/${FILE_LED}_       $SERVICE_DIR/$FILE_LED
cp $PWD/${FILE_WEB}_       $SERVICE_DIR/$FILE_WEB

perl -pi -e "s|\{\{BALLDI_DIR\}\}|${BALLDI_DIR//\//\\/}|gi" $SERVICE_DIR/$FILE_DRVSERV
perl -pi -e "s|\{\{BALLDI_DIR\}\}|${BALLDI_DIR//\//\\/}|gi" $SERVICE_DIR/$FILE_IMU
perl -pi -e "s|\{\{BALLDI_DIR\}\}|${BALLDI_DIR//\//\\/}|gi" $SERVICE_DIR/$FILE_TOF
perl -pi -e "s|\{\{BALLDI_DIR\}\}|${BALLDI_DIR//\//\\/}|gi" $SERVICE_DIR/$FILE_LED
perl -pi -e "s|\{\{BALLDI_DIR\}\}|${BALLDI_DIR//\//\\/}|gi" $SERVICE_DIR/$FILE_WEB


systemctl daemon-reload

systemctl enable $FILE_DRVSERV
systemctl enable $FILE_IMU
systemctl enable $FILE_TOF
systemctl enable $FILE_LED
systemctl enable $FILE_WEB

systemctl start $FILE_DRVSERV
systemctl start $FILE_IMU
systemctl start $FILE_TOF
systemctl start $FILE_LED
systemctl start $FILE_WEB


