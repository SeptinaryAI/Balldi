CFLAGS = -g

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/drv_server.out

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
O_FILES = $(patsubst %.c, %.o, $(SRC_FILES))

out: clean
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES) -lm -lpthread

run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf
