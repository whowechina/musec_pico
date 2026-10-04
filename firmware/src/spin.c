/*
 * The Spinning of Buttons
 * WHowe <github.com/whowechina>
 * 
 */

#include "spin.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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

    if (musec_cfg->spin.fast_i2c) {
        i2c_init(BUS_I2C, BUS_I2C_FAST_FREQ);
    } else {
        i2c_init(BUS_I2C, BUS_I2C_FREQ);
    }

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

typedef struct {
    uint16_t angle;
    uint16_t old_angle;
    int queue;
    uint8_t units;
} spin_ctx_t;

static spin_ctx_t spin_ctx[SPIN_NUM];

void spin_update()
{
    static int index = 0;
    index = (index + 1) % SPIN_NUM;

    spin_ctx_t *ctx = &spin_ctx[index];

    tmag5273_use(index);
    uint16_t raw = tmag5273_read_angle();

    ctx->angle = musec_cfg->spin.reversed & (1 << index) ? FULL_SCALE - raw : raw;

    int delta = ctx->angle - ctx->old_angle;
    if (delta > FULL_SCALE / 2) {
        delta -= FULL_SCALE;
    } else if (delta < -FULL_SCALE / 2) {
        delta += FULL_SCALE;
    }

    if (abs(delta) <= 8) {
        return;
    }

    ctx->old_angle = ctx->angle;

    ctx->queue += delta;

    int step = FULL_SCALE / musec_cfg->spin.units_per_turn;
    int num_steps = ctx->queue / step;

    ctx->units += num_steps;
    ctx->queue -= step * num_steps;
}

uint16_t spin_read(uint8_t index)
{
    if (index >= SPIN_NUM) {
        return 0;
    }
    return spin_ctx[index].angle;
}

uint16_t spin_units(uint8_t index)
{
    if (index >= SPIN_NUM) {
        return 0;
    }
    return spin_ctx[index].units;
}
