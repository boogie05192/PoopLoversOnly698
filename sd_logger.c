#include "sd_logger.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#define SD_SPI_INSTANCE spi1
#define SD_CS_PIN 13
#define SD_MOSI_PIN 11
#define SD_MISO_PIN 12
#define SD_SCK_PIN 10
#define SD_BLOCK_SIZE 512

static uint32_t g_next_block = 0;
static bool g_sd_ready = false;

static uint8_t sd_spi_txrx(uint8_t tx) {
    uint8_t rx = 0;
    spi_write_read_blocking(SD_SPI_INSTANCE, &tx, &rx, 1);
    return rx;
}

static void sd_select(bool selected) {
    gpio_put(SD_CS_PIN, selected ? 0 : 1);
}

static void sd_deselect(void) {
    sd_select(false);
    sd_spi_txrx(0xFF);
}

static void sd_wait_ready(void) {
    while (sd_spi_txrx(0xFF) != 0xFF) {
    }
}

static uint8_t sd_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    sd_wait_ready();
    sd_select(true);

    uint8_t frame[6] = {
        (uint8_t)(0x40 | cmd),
        (uint8_t)(arg >> 24),
        (uint8_t)(arg >> 16),
        (uint8_t)(arg >> 8),
        (uint8_t)arg,
        crc
    };

    for (int i = 0; i < 6; ++i) {
        sd_spi_txrx(frame[i]);
    }

    for (int i = 0; i < 10; ++i) {
        uint8_t response = sd_spi_txrx(0xFF);
        if ((response & 0x80) == 0) {
            sd_deselect();
            return response;
        }
    }

    sd_deselect();
    return 0xFF;
}

static bool sd_write_block(uint32_t block, const uint8_t *data) {
    if (!g_sd_ready) {
        return false;
    }

    if (sd_command(24, block, 0xFF) != 0x01) {
        return false;
    }

    sd_spi_txrx(0xFF);
    sd_spi_txrx(0xFE);

    for (int i = 0; i < SD_BLOCK_SIZE; ++i) {
        sd_spi_txrx(data[i]);
    }

    sd_spi_txrx(0xFF);
    sd_spi_txrx(0xFF);

    uint8_t response = sd_spi_txrx(0xFF);
    if ((response & 0x1F) != 0x05) {
        return false;
    }

    for (int i = 0; i < 10000; ++i) {
        if (sd_spi_txrx(0xFF) != 0x00) {
            return true;
        }
    }

    return false;
}

static bool sd_read_block(uint32_t block, uint8_t *buffer) {
    if (!g_sd_ready) {
        return false;
    }

    if (sd_command(17, block, 0xFF) != 0x01) {
        return false;
    }

    uint8_t token = 0xFF;
    for (int i = 0; i < 40000; ++i) {
        token = sd_spi_txrx(0xFF);
        if (token == 0xFE) {
            break;
        }
    }

    if (token != 0xFE) {
        return false;
    }

    for (int i = 0; i < SD_BLOCK_SIZE; ++i) {
        buffer[i] = sd_spi_txrx(0xFF);
    }

    sd_spi_txrx(0xFF);
    sd_spi_txrx(0xFF);
    return true;
}

bool sd_logger_init(void) {
    spi_init(SD_SPI_INSTANCE, 400 * 1000);
    spi_set_format(SD_SPI_INSTANCE, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1);

    for (int i = 0; i < 10; ++i) {
        sd_spi_txrx(0xFF);
    }

    sd_select(true);

    if (sd_command(0, 0, 0x95) != 0x01) {
        printf("SD init: CMD0 failed.\n");
        return false;
    }

    if (sd_command(8, 0x000001AA, 0x87) != 0x01) {
        printf("SD init: CMD8 failed.\n");
        return false;
    }

    for (int i = 0; i < 100; ++i) {
        if (sd_command(55, 0, 0xFF) == 0x01 && sd_command(41, 0x40000000, 0xFF) == 0x00) {
            break;
        }
        sleep_ms(10);
    }

    if (sd_command(58, 0, 0xFF) != 0x01) {
        printf("SD init: CMD58 failed.\n");
        return false;
    }

    g_next_block = 0;
    g_sd_ready = true;
    printf("SD logger ready. Raw blocks will be written to the card.\n");
    return true;
}

bool sd_logger_write_sample(const struct SensorSample *sample) {
    if (!g_sd_ready) {
        return false;
    }

    char block[SD_BLOCK_SIZE] = {0};
    int written = snprintf(block, sizeof(block),
                           "%lu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                           sample->timestamp_ms,
                           sample->ax,
                           sample->ay,
                           sample->az,
                           sample->gx,
                           sample->gy,
                           sample->gz);

    if (written < 0) {
        return false;
    }

    bool ok = sd_write_block(g_next_block, (const uint8_t *)block);
    if (ok) {
        ++g_next_block;
    }
    return ok;
}

bool sd_logger_read_block(uint32_t block, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }

    uint8_t raw[SD_BLOCK_SIZE] = {0};
    if (!sd_read_block(block, raw)) {
        return false;
    }

    size_t bytes_to_copy = SD_BLOCK_SIZE < buffer_size ? SD_BLOCK_SIZE : buffer_size - 1;
    memcpy(buffer, raw, bytes_to_copy);
    buffer[bytes_to_copy] = '\0';
    return true;
}
