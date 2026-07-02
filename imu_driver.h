#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H

#include <stdbool.h>

#include "sensor_types.h"

bool imu_init(void);
bool imu_read_sample(struct SensorSample *sample);

#endif
