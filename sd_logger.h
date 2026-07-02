#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <stdbool.h>
#include <stddef.h>

#include "sensor_types.h"

bool sd_logger_init(void);
bool sd_logger_write_sample(const struct SensorSample *sample);
bool sd_logger_read_block(uint32_t block, char *buffer, size_t buffer_size);

#endif
