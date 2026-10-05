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

    struct {
        bool active;
        bool dir;
        uint16_t speed_10x;
    } sim_spin; // software-simulated spin-down

    uint16_t last_speed;
    uint8_t stop_count;
    int8_t accel_count;

    uint8_t units;
} spin_ctx_t;

static spin_ctx_t spin_ctx[SPIN_NUM];

static int spin_get_delta(spin_ctx_t *ctx)
{
    int delta = ctx->angle - ctx->old_angle;
    if (delta > FULL_SCALE / 2) {
        delta -= FULL_SCALE;
    } else if (delta < -FULL_SCALE / 2) {
        delta += FULL_SCALE;
    }

    if (abs(delta) <= 8) {
        delta = 0;
    } else {
        ctx->old_angle = ctx->angle;
    }

    return delta;
}

static void spin_move(spin_ctx_t *ctx, int delta)
{
    ctx->queue += delta;

    int step = FULL_SCALE / musec_cfg->spin.units_per_turn;
    int num_steps = ctx->queue / step;

    ctx->units += num_steps;
    ctx->queue -= step * num_steps;
}

static int spin_suppress(spin_ctx_t *ctx, int delta)
{
    if (musec_cfg->spin.suppress.threshold == 0) {
        ctx->sim_spin.active = false;
        ctx->stop_count = 0;
        return delta;
    }

    if (abs(delta) >= musec_cfg->spin.suppress.threshold) {
        // threshold exceeded
        ctx->sim_spin.active = true;
        ctx->stop_count = 0;
        ctx->accel_count = 0;
        ctx->sim_spin.dir = (delta > 0);
        ctx->sim_spin.speed_10x = abs(delta) * 10;
    }

    if (ctx->sim_spin.active) {
        if ((abs(delta) > 20) && ((delta > 0) != ctx->sim_spin.dir)) {
            // inverted direction detected
            ctx->sim_spin.active = false;
            ctx->stop_count = 0;
            ctx->last_speed = abs(delta);
            return delta;
        }

        if (delta == 0) {
            ctx->stop_count++;
            if (ctx->stop_count >= 4) {
                // physical stop detected
                ctx->sim_spin.active = false;
                ctx->last_speed = abs(delta);
                return delta;
            }
        } else {
            ctx->stop_count = 0;
        }

        ctx->accel_count += (abs(delta) > ctx->last_speed) ? 1 : -1;
        if (ctx->accel_count >= 5) {
            // external force detected (natural irregularities filtered)
            ctx->sim_spin.active = false;
        } else if (ctx->accel_count < 0) {
            ctx->accel_count = 0;
        }
    }

    ctx->last_speed = abs(delta);

    if (ctx->sim_spin.active) {
        // spin-down
        int decay = musec_cfg->spin.suppress.decay;
        ctx->sim_spin.speed_10x =
            (ctx->sim_spin.speed_10x > decay) ? ctx->sim_spin.speed_10x - decay : 0;

        // simulated speed instead of actual delta
        delta = (ctx->sim_spin.dir ? ctx->sim_spin.speed_10x : -ctx->sim_spin.speed_10x) / 10;
    }

    return delta;
}

static void spin_proc(int index)
{
    spin_ctx_t *ctx = &spin_ctx[index];

    int delta = spin_get_delta(ctx);

    int final_delta = spin_suppress(ctx, delta);

    spin_move(ctx, final_delta);
}

void spin_update()
{
    static int index = 0;

    tmag5273_use(index);
    uint16_t raw = tmag5273_read_angle();
    bool invert = musec_cfg->spin.reversed & (1 << index);
    int angle = invert ? FULL_SCALE - raw : raw;

    spin_ctx[index].angle = angle;

    spin_proc(index);

    index = (index + 1) % SPIN_NUM;
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
