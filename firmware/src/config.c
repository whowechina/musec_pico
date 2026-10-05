/*
 * Controller Config and Runtime Data
 * WHowe <github.com/whowechina>
 * 
 * Config is a global data structure that stores all the configuration
 * Runtime is something to share between files.
 */

#include "config.h"
#include "savedata.h"

musec_cfg_t *musec_cfg;

static musec_cfg_t default_cfg = {
    .spin = {
        .fast_i2c = false,
        .units_per_turn = 80,
        .suppress = {
            .threshold = 0,
            .decay = 25,
        }
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
    if (musec_cfg->spin.units_per_turn == 0) {
        musec_cfg->spin.units_per_turn = default_cfg.spin.units_per_turn;
        config_changed();
    }

    if (musec_cfg->spin.suppress.decay == 0) {
        musec_cfg->spin.suppress = default_cfg.spin.suppress;
        config_changed();
    }
}

void config_changed()
{
    savedata_save(false);
}

void config_factory_reset()
{
    *musec_cfg = default_cfg;
    savedata_save(true);
}

void config_init()
{
    musec_cfg = (musec_cfg_t *)savedata_alloc(sizeof(*musec_cfg), &default_cfg, config_loaded);
}
