GIN_DIR = .
GIN_DRV_DIR = ./drv
GIN_CAM_DIR = ./cam

DRV_API_DIR = ../drv_api
CAM_API_DIR = ../cam_v4l2_api
YOLO_API_DIR = ../NN/yolo_lite/mnn


all: clean
	set -e

	echo 1 > /proc/sys/vm/overcommit_memory

	make -C $(DRV_API_DIR)
	rm $(GIN_DRV_DIR)/libdrvapi.a -f
	rm $(GIN_CAM_DIR)/libcamv4l2api.a -f
	cp $(DRV_API_DIR)/build/libdrvapi.a $(GIN_DRV_DIR)/libdrvapi.a
	cp $(CAM_API_DIR)/build/libcamv4l2api.a $(GIN_CAM_DIR)/libcamv4l2api.a
	cp $(CAM_API_DIR)/inc/* $(GIN_CAM_DIR)/
	go build

	echo 0 > /proc/sys/vm/overcommit_memory

clean:
	rm $(GIN_DRV_DIR)/libdrvapi.a -f
	rm $(GIN_CAM_DIR)/libcamv4l2api.a -f
	rm $(GIN_CAM_DIR)/*.h -f
	rm $(GIN_DIR)/web_serv_gin -f
