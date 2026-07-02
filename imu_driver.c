#include "imu_driver.h"

#include <stdio.h>

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define IMU_I2C_INSTANCE i2c0
#define IMU_I2C_ADDR 0x6A
#define IMU_I2C_SDA_PIN 0
#define IMU_I2C_SCL_PIN 1

#define LSM6DSL_CTRL1_XL 0x10
#define LSM6DSL_CTRL2_G 0x11
#define LSM6DSL_CTRL3_C 0x12
#define LSM6DSL_CTRL8_XL 0x17
#define LSM6DSL_OUTX_L_XL 0x28
#define LSM6DSL_OUTX_H_XL 0x29
#define LSM6DSL_OUTY_L_XL 0x2A
#define LSM6DSL_OUTY_H_XL 0x2B
#define LSM6DSL_OUTZ_L_XL 0x2C
#define LSM6DSL_OUTZ_H_XL 0x2D
#define LSM6DSL_OUTX_L_G 0x22
#define LSM6DSL_OUTX_H_G 0x23
#define LSM6DSL_OUTY_L_G 0x24
#define LSM6DSL_OUTY_H_G 0x25
#define LSM6DSL_OUTZ_L_G 0x26
#define LSM6DSL_OUTZ_H_G 0x27

static bool imu_write_reg(uint8_t reg_address, uint8_t data) {
    uint8_t payload[2] = { reg_address, data };
    return i2c_write_blocking(IMU_I2C_INSTANCE, IMU_I2C_ADDR, payload, 2, false) == 2;
}

static bool imu_read_reg(uint8_t reg_address, uint8_t *value) {
    if (i2c_write_blocking(IMU_I2C_INSTANCE, IMU_I2C_ADDR, &reg_address, 1, true) != 1) {
        return false;
    }
    int read_count = i2c_read_blocking(IMU_I2C_INSTANCE, IMU_I2C_ADDR, value, 1, false);
    return read_count == 1;
}

static int16_t imu_read_16(uint8_t low_reg, uint8_t high_reg) {
    uint8_t lo = 0;
    uint8_t hi = 0;
    if (!imu_read_reg(low_reg, &lo) || !imu_read_reg(high_reg, &hi)) {
        return 0;
    }
    return (int16_t)((hi << 8) | lo);
}

static void imu_scan_bus(void) {
    printf("I2C scan:");
    for (int addr = 0; addr < 128; ++addr) {
        uint8_t dummy = 0;
        int result = i2c_read_blocking(IMU_I2C_INSTANCE, addr, &dummy, 0, false);
        if (result >= 0) {
            printf(" 0x%02X", addr);
        }
    }
    printf("\n");
}

bool imu_init(void) {
    i2c_init(IMU_I2C_INSTANCE, 100 * 1000);
    gpio_set_function(IMU_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(IMU_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(IMU_I2C_SDA_PIN);
    gpio_pull_up(IMU_I2C_SCL_PIN);

    imu_scan_bus();

    if (!imu_write_reg(LSM6DSL_CTRL1_XL, 0b10011111)) {
        printf("IMU init failed: CTRL1_XL write failed.\n");
        return false;
    }
    if (!imu_write_reg(LSM6DSL_CTRL8_XL, 0b11001000)) {
        printf("IMU init failed: CTRL8_XL write failed.\n");
        return false;
    }
    if (!imu_write_reg(LSM6DSL_CTRL3_C, 0b01000100)) {
        printf("IMU init failed: CTRL3_C write failed.\n");
        return false;
    }
    if (!imu_write_reg(LSM6DSL_CTRL2_G, 0b10011100)) {
        printf("IMU init failed: CTRL2_G write failed.\n");
        return false;
    }

    printf("IMU I2C initialized on SDA=GP0 SCL=GP1\n");
    return true;
}

bool imu_read_sample(struct SensorSample *sample) {
    sample->timestamp_ms = to_ms_since_boot(get_absolute_time());
    sample->ax = imu_read_16(LSM6DSL_OUTX_L_XL, LSM6DSL_OUTX_H_XL) / 16384.0f;
    sample->ay = imu_read_16(LSM6DSL_OUTY_L_XL, LSM6DSL_OUTY_H_XL) / 16384.0f;
    sample->az = imu_read_16(LSM6DSL_OUTZ_L_XL, LSM6DSL_OUTZ_H_XL) / 16384.0f;
    sample->gx = imu_read_16(LSM6DSL_OUTX_L_G, LSM6DSL_OUTX_H_G) / 131.0f;
    sample->gy = imu_read_16(LSM6DSL_OUTY_L_G, LSM6DSL_OUTY_H_G) / 131.0f;
    sample->gz = imu_read_16(LSM6DSL_OUTZ_L_G, LSM6DSL_OUTZ_H_G) / 131.0f;
    return true;
}
