/*
 * Musec Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_MUSEC_PICO

#define RGB_PIN 14
#define RGB_ORDER GRB // or RGB

#define BUS_I2C i2c0
#define BUS_I2C_SDA 0
#define BUS_I2C_SCL 1
#define BUS_I2C_FREQ 400*1000

#define SPIN_DEF { 13, 12, 11, 10, 9 }
#define BUTTON_DEF { 16, 17, 18, 19, 20, 21, 22, 2 }

#else

#endif
