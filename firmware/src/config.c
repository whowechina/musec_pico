/*
 * Controller Config and Runtime Data
 * WHowe <github.com/whowechina>
 * 
 * Config is a global data structure that stores all the configuration
 * Runtime is something to share between files.
 */

#include "config.h"
#include "save.h"

musec_cfg_t *musec_cfg;

static musec_cfg_t default_cfg = {
    .spin = {
        .fast_i2c = false,
        .units_per_turn = 80,
    },
    .light = {
        .level = 128,
    },
    .pedal = {
        .internal = true,
        .external = true,
    },
};

musec_runtime_t musec_runtime;

static void config_loaded()
{
}

void config_changed()
{
    save_request(false);
}

void config_factory_reset()
{
    *musec_cfg = default_cfg;
    save_request(true);
}

void config_init()
{
    musec_cfg = (musec_cfg_t *)save_alloc(sizeof(*musec_cfg), &default_cfg, config_loaded);
}
