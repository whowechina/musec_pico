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
const uint16_t FULL_SCALE = 360 * 16;

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

    if (musec_cfg->spin.units_per_turn == 0) {
        musec_cfg->spin.units_per_turn = 80;
        config_changed();
    }
}

uint8_t spin_num()
{
    return SPIN_NUM;
}

bool spin_present(uint8_t index)
{
    if (index >= SPIN_NUM) {
        return false;
    }
    return tmag5273_is_present(index);
}

static uint16_t spin_reading[SPIN_NUM];

void spin_update()
{
    for (int i = 0; i < SPIN_NUM; i++) {
        tmag5273_use(i);
        spin_reading[i] = tmag5273_read_angle();
        if (musec_cfg->spin.reversed[i]) {
            spin_reading[i] = FULL_SCALE - spin_reading[i];
        }
    }
}

uint16_t spin_read(uint8_t index)
{
    if (index >= SPIN_NUM) {
        return 0;
    }
    return spin_reading[index];
}

uint16_t spin_units(uint8_t index)
{
    static uint16_t last[SPIN_NUM] = {0};
    static int counter[SPIN_NUM] = {0};

    int delta = spin_reading[index] - last[index];
    if (delta > FULL_SCALE / 2) {
        delta -= FULL_SCALE;
    } else if (delta < -FULL_SCALE / 2) {
        delta += FULL_SCALE;
    }

    int resolution = FULL_SCALE / musec_cfg->spin.units_per_turn;
    if ((delta <= -resolution) || (delta >= resolution)) {
        last[index] = spin_reading[index];
        counter[index] += delta;
    }

    return counter[index] * musec_cfg->spin.units_per_turn / FULL_SCALE;
}
