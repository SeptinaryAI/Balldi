CFLAGS = -O2
LIBS = -li2c -lm

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/mpu6050.out

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

out: clean
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -static -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES) $(LIBS)

run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf
