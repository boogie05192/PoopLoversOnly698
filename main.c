#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "imu_driver.h"
#include "sd_logger.h"
#include "sensor_types.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define TEST_MODE 1

static void print_sample(const struct SensorSample *sample) {
    printf("sample: t=%lu ax=%0.3f ay=%0.3f az=%0.3f gx=%0.3f gy=%0.3f gz=%0.3f\n",
           sample->timestamp_ms,
           sample->ax,
           sample->ay,
           sample->az,
           sample->gx,
           sample->gy,
           sample->gz);
}

int main(void) {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("Rocket logger starting...\n");
    printf("Test: LED blink without IMU/SD init\n");

    while (true) {
        gpio_put(LED_PIN, 1);
        printf("LED on\n");
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        printf("LED off\n");
        sleep_ms(500);
    }

    return 0;
}
