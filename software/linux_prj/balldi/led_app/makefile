CFLAGS = -g

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/ledapp

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
O_FILES = $(patsubst %.c, %.o, $(SRC_FILES))

%.o: %.c
	gcc $(CFLAGS) -D _MAKE_SO_ -c -fPIC $< -I$(INC_DIR) -o $@

.PHONY: pre out run clean

out:
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES)


run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf

