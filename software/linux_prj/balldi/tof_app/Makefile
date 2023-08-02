CC=g++
CFLAGS = -g -Wall

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/tofapp
#SO_TARGET = $(BUILD_DIR)/libtofapi.so
#LIB_TARGET = $(BUILD_DIR)/libtofapi.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
O_FILES = $(patsubst %.cpp, %.o, $(SRC_FILES))

%.o: %.cpp
	gcc $(CFLAGS) -c -fPIC $< -I$(INC_DIR) -o $@ -lpthread

.PHONY: lib out clean

out:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	$(CC) $(CFLAGS) -D _MAKE_OUT_ -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES)

run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf
