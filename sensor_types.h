#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

struct SensorSample {
    uint32_t timestamp_ms;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
};

#endif
