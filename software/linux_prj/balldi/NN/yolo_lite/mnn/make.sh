#!/bin/sh

./clean.sh

cp ./model_zoo /home/pi/ -r
cp ../../../cam_v4l2_api/inc/mnnyoloapi.h ./include/

mkdir build
cd build
cmake ..
make
cp libmnnyoloapi.so /usr/lib/
