/*
 * Controller Config
 * WHowe <github.com/whowechina>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t rgb_hsv; // 0: RGB, 1: HSV
    uint8_t val[3]; // RGB or HSV
} rgb_hsv_t;

typedef struct __attribute__((packed)) {
    struct {
        uint8_t units_per_turn;
        uint8_t reversed[5];
    } spin;
    struct {
        bool internal;
        bool external;
        bool invert_ext;
    } pedal;
    struct {
        rgb_hsv_t colors[12];
        uint8_t level;
        uint8_t reserved[15];
    } light;
} musec_cfg_t;

typedef struct {
    uint16_t fps[2];
    bool key_stuck;
} musec_runtime_t;

extern musec_cfg_t *musec_cfg;
extern musec_runtime_t musec_runtime;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default

#endif
