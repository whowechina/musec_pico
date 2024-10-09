/*
 * The Spinning of Buttons
 * WHowe <github.com/whowechina>
 */

#ifndef SPIN_H
#define SPIN_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/flash.h"

void spin_init();
uint8_t spin_num();
void spin_update();
uint16_t spin_read(uint8_t index);
uint16_t spin_units(uint8_t index);
bool spin_present(uint8_t index);

#endif
