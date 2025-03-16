/*
 * Musec Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_MUSEC_PICO

#define RGB_PIN_MAIN 14
#define RGB_PIN_PEDAL 15
#define RGB_ORDER GRB // or RGB

#define BUS_I2C i2c0
#define BUS_I2C_SDA 0
#define BUS_I2C_SCL 1
#define BUS_I2C_FREQ 266*1000

#define SPIN_DEF { 13, 12, 11, 10, 9 }
 
 /* Main * 5, Aux, Start, Int. pedal, Ext. pedal */
#define BUTTON_DEF { 16, 17, 18, 19, 20, 22, 21, 2, 3 }

#else

#endif
