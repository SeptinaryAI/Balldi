CFLAGS = -g

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/camv4l2api.out
SO_TARGET = $(BUILD_DIR)/libcamv4l2api.so
LIB_TARGET = $(BUILD_DIR)/libcamv4l2api.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
O_FILES = $(patsubst %.c, %.o, $(SRC_FILES))

#D_FMT = _STREAM_H264_
D_FMT = _STREAM_MJPG_

%.o: %.c
	gcc $(CFLAGS) -D _MAKE_SO_ -D $(D_FMT) -c -fPIC $< -I$(INC_DIR) -o $@ -lpthread

.PHONY: lib out run clean

lib: clean $(O_FILES)
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -nostartfiles -shared -fPIC -o $(SO_TARGET) $(O_FILES) -lpthread
	ar -cr $(LIB_TARGET) $(O_FILES)

out:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES) -lpthread -ldl -lstdc++


run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf

