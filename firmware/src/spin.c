/*
 * The Spinning of Buttons
 * WHowe <github.com/whowechina>
 * 
 */

#include "spin.h"

#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "config.h"
#include "board_defs.h"

#include "tmag5273.h"

static const uint8_t spin_enablers[] = SPIN_DEF;

#define SPIN_NUM (count_of(spin_enablers))

void spin_init()
{
    gpio_init(BUS_I2C_SDA);
    gpio_set_function(BUS_I2C_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(BUS_I2C_SDA);
    gpio_init(BUS_I2C_SCL);
    gpio_set_function(BUS_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(BUS_I2C_SCL);
    i2c_init(BUS_I2C, BUS_I2C_FREQ);

    for (int i = 0; i < SPIN_NUM; i++) {
        uint8_t ce = spin_enablers[i];
        gpio_init(ce);
        gpio_set_dir(ce, GPIO_OUT);
        gpio_put(ce, 1);
        sleep_us(1000);
        tmag5273_init(i, BUS_I2C);
        tmag5273_init_sensor();
    }
}

uint8_t spin_num()
{
    return SPIN_NUM;
}

static uint16_t spin_reading[SPIN_NUM];

void spin_update()
{
    for (int i = 0; i < SPIN_NUM; i++) {
        tmag5273_use(i);
        spin_reading[i] = tmag5273_read_angle();
    }
}

uint16_t spin_read(uint8_t index)
{
    if (index >= SPIN_NUM) {
        return 0;
    }
    return spin_reading[index];
}
