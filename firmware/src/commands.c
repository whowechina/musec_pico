#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "config.h"
#include "save.h"
#include "cli.h"

#include "spin.h"

#include "usb_descriptors.h"

#define SENSE_LIMIT_MAX 9
#define SENSE_LIMIT_MIN -9

static void disp_light()
{
    printf("[Light]\n");
    printf("  Level: %d.\n", musec_cfg->light.level);
}

static void disp_spin()
{
    printf("[Spin]\n");
    printf("  Units Per Turn: %d.\n", musec_cfg->spin.units_per_turn);
    for (int i = 0; i < 5; i++) {
        printf("  Spinner %d: %s, %s.\n", i + 1,
               spin_present(i) ? "OK" : "ERROR",
               musec_cfg->spin.reversed[i] ? "Reversed" : "Forward");
    }
}

static void disp_pedal()
{
    printf("[Pedal]\n");
    printf("  Internal: %s.\n", musec_cfg->pedal.internal ? "ON" : "OFF");
    printf("  External: %s, %s.\n",
            musec_cfg->pedal.external ? "ON" : "OFF",
            musec_cfg->pedal.invert_ext ? "Inverted" : "Normal");
}

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [light|spin|pedal]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    if (argc == 0) {
        disp_light();
        disp_spin();
        disp_pedal();
        return;
    }

    const char *choices[] = {"light", "spin", "pedal" };
    switch (cli_match_prefix(choices, count_of(choices), argv[0])) {
        case 0:
            disp_light();
            break;
        case 1:
            disp_spin();
            break;
        case 2:
            disp_pedal();
            break;
        default:
            printf(usage);
            break;
    }
}

static int fps[2];
void fps_count(int core)
{
    static uint32_t last[2] = {0};
    static int counter[2] = {0};

    counter[core]++;

    uint32_t now = time_us_32();
    if (now - last[core] < 1000000) {
        return;
    }
    last[core] = now;
    fps[core] = counter[core];
    counter[core] = 0;
}

static void handle_level(int argc, char *argv[])
{
    const char *usage = "Usage: level <0..255>\n";
    if (argc != 1) {
        printf(usage);
        return;
    }

    int level = cli_extract_non_neg_int(argv[0], 0);
    if ((level < 0) || (level > 255)) {
        printf(usage);
        return;
    }

    musec_cfg->light.level = level;
    config_changed();
    disp_light();
}

static void handle_spin_rate(const char *rate)
{
    const char *usage = "Usage: spin rate <units_per_turn>\n"
                        "  units_per_turn: 20..255\n";
    int units = cli_extract_non_neg_int(rate, 0);
    if ((units < 20) || (units > 255)) {
        printf(usage);
        return;
    }

    musec_cfg->spin.units_per_turn = units;
    config_changed();
    disp_spin();
}

static void handle_spin_invert(int id, const char *dir)
{
    const char *usage = "Usage: spin <id> <forward|reversed>\n"
                        "  id: 1..5\n";

    const char *choices[] = {"forward", "reversed"};
    int match = cli_match_prefix(choices, count_of(choices), dir);
    if (match < 0) {
        printf(usage);
        return;
    }

    musec_cfg->spin.reversed[id] = match;

    config_changed();
    disp_spin();
}

static void handle_spin(int argc, char *argv[])
{
    const char *usage = "Usage: spin rate <units_per_turn>\n"
                        "       spin <id> <normal|reverse>\n"
                        "  units_per_turn: 20..255\n"
                        "  id: 1..5\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    const char *choices[] = { "1", "2", "3", "4", "5", "rate" };
    int match = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (match < 0) {
        printf(usage);
        return;
    }

    if (match == 5) {
        handle_spin_rate(argv[1]);
        return;
    }

    handle_spin_invert(match, argv[1]);
}

static void handle_pedal(int argc, char *argv[])
{
    const char *usage = "Usage: pedal <internal|external> <on|off>\n"
                        "       pedal invert <on|off> (only for external)\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    const char *target_choices[] = {"internal", "external", "invert"};
    const char *onoff_choices[] = {"off", "on"};
    int target = cli_match_prefix(target_choices, count_of(target_choices), argv[0]);
    int onoff = cli_match_prefix(onoff_choices, 2, argv[1]);
    if ((target < 0) || (onoff < 0)) {
        printf(usage);
        return;
    }

    if (target == 0) {
        musec_cfg->pedal.internal = onoff;
    } else if (target == 1) {
        musec_cfg->pedal.external = onoff;
    } else {
        musec_cfg->pedal.invert_ext = onoff;
    }

    config_changed();
    disp_pedal();
}

static void handle_save()
{
    save_request(true);
}

static void handle_factory_reset()
{
    config_factory_reset();
    printf("Factory reset done.\n");
}

void commands_init()
{
    cli_register("display", handle_display, "Display all config.");
    cli_register("level", handle_level, "Set LED brightness level.");
    cli_register("spin", handle_spin, "Set spin rate.");
    cli_register("pedal", handle_pedal, "Set pedal mode.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", handle_factory_reset, "Reset everything to default.");
}
