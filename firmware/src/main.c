/*
 * Controller Main
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include "board_defs.h"

#include "save.h"
#include "config.h"
#include "cli.h"
#include "commands.h"

#include "hid.h"

#include "light.h"
#include "button.h"

#include "tmag5273.h"

static void run_lights()
{
}

static mutex_t core1_io_lock;
static void core1_loop()
{
    while (1) {
        if (mutex_try_enter(&core1_io_lock, NULL)) {
            run_lights();
            light_update();
            mutex_exit(&core1_io_lock);
        }
        cli_fps_count(1);
        sleep_us(700);
    }
}

static void read_spin()
{
    uint16_t angle = tmag5273_read_angle();
    printf("%5d:", angle);

    angle /= 10;
    for (int i = 0; i < 36; i++) {
        char c = (i == angle) ? '*' : ' ';
        printf("%c", c);
    }
    printf("\n");
}

static void core0_loop()
{
    while(1) {
        tud_task();

        cli_run();

        save_loop();
        cli_fps_count(0);

        button_update();

        hid_update();

        read_spin();

        sleep_us(900);
    }
}

/* if certain key pressed when booting, enter update mode */
static void update_check()
{
    const uint8_t pins[] = BUTTON_DEF;
    int pressed = 0;
    for (int i = 0; i < count_of(pins); i++) {
        uint8_t gpio = pins[i];
        gpio_init(gpio);
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
        sleep_ms(1);
        if (!gpio_get(gpio)) {
            pressed++;
        }
    }

    if (pressed >= 4) {
        sleep_ms(100);
        reset_usb_boot(0, 2);
        return;
    }
}

void init()
{
    sleep_ms(50);
    board_init();

    update_check();

    tusb_init();
    stdio_init_all();

    config_init();
    mutex_init(&core1_io_lock);
    save_init(0xca44caac, &core1_io_lock);

    light_init();
    button_init();

    i2c_init(BUS_I2C, BUS_I2C_FREQ);
    gpio_init(BUS_I2C_SDA);
    gpio_init(BUS_I2C_SCL);
    gpio_set_function(BUS_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(BUS_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(BUS_I2C_SDA);
    gpio_pull_up(BUS_I2C_SCL);

    tmag5273_init(0, BUS_I2C, 0);
    tmag5273_use(0);
    tmag5273_init_sensor();

    cli_init("musec_pico>", "\n   << Musec Pico Controller >>\n"
                            " https://github.com/whowechina\n\n");
    
    commands_init();
}

int main(void)
{
    init();
    multicore_launch_core1(core1_loop);
    core0_loop();
    return 0;
}

struct __attribute__((packed)) {
    uint16_t buttons;
    uint8_t  HAT;
    uint32_t axis;
} hid_joy_out = {0};

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen)
{
    printf("Get from USB %d-%d\n", report_id, report_type);
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize)
{
    hid_proc(buffer, bufsize);
}
