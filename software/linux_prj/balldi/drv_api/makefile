CFLAGS = -g

BUILD_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./inc

OUT_TARGET = $(BUILD_DIR)/drv_api.out
SO_TARGET = $(BUILD_DIR)/drv_api.so
LIB_TARGET = $(BUILD_DIR)/libdrvapi.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
O_FILES = $(patsubst %.c, %.o, $(SRC_FILES))



%.o: %.c
	gcc $(CFLAGS) -D _MAKE_SO_ -c -fPIC $< -I$(INC_DIR) -o $@ -lpthread

.PHONY: pre lib out run clean pre after


lib: clean pre $(O_FILES)
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -nostartfiles -shared -fPIC -o $(SO_TARGET) $(O_FILES)
	ar -cr $(LIB_TARGET) $(O_FILES)
	make after

pre:
	cp ../drv_server/inc/msg_serv.h ./inc/msg_serv.h

after:
	cp $(SO_TARGET) ./test/
	cp $(LIB_TARGET) ./test/
	rm ./inc/msg_serv.h

out: pre
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi;\
	gcc $(CFLAGS) -o $(OUT_TARGET) -I$(INC_DIR) $(SRC_FILES)
	rm ./inc/msg_serv.h

run: out
	$(OUT_TARGET)

clean:
	rm ./src/*.o -f
	rm ./build -rf

