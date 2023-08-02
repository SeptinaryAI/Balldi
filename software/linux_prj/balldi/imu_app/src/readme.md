You should record your MPU6050 static offset, modify the code in **usr_api.c** to calibrate MPU6050.

You can find code like this:
```C
//you can set static offset of origin data  here
static int16_t gyro_offset_map[3] = {
        -175,
        -174,
        -167,
};

static int16_t accel_offset_map[3] = {
        911,
        419,
        -155,
};
```


