/*
 * TMAG5273 Distance measurement sensor
 * WHowe <github.com/whowechina>
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hardware/i2c.h"
#include "board_defs.h"

#include "tmag5273.h"

#define TMAG5273_DEF_ADDR 0x35

#define IO_TIMEOUT_US 1000

static struct {
    i2c_inst_t *port;
    uint8_t addr;
} instances[16] = { { i2c0, TMAG5273_DEF_ADDR } };

static int current_instance = 0;

#define INSTANCE_NUM (sizeof(instances) / sizeof(instances[0]))
#define I2C_PORT instances[current_instance].port
#define I2C_ADDR instances[current_instance].addr


static void write_reg(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, data, 2, false,
                             time_us_64() + IO_TIMEOUT_US);
}

/*
static void write_reg16(uint8_t reg, uint16_t value)
{
    uint8_t data[3] = { reg, value >> 8, value & 0xff };
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, data, 3, false,
                             time_us_64() + IO_TIMEOUT_US);
}

static void write_reg_list(const uint16_t *list)
{
    const uint16_t *regs = list + 1;
    for (int i = 0; i < *list; i++) {
        write_reg(regs[i] >> 8, regs[i] & 0xff);
    }
}
*/

static uint8_t read_reg(uint8_t reg)
{
    uint8_t value;
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, &reg, 1, true,
                             time_us_64() + IO_TIMEOUT_US);
    i2c_read_blocking_until(I2C_PORT, I2C_ADDR, &value, 1, false,
                             time_us_64() + IO_TIMEOUT_US);
    return value;
}

static uint16_t read_reg16(uint8_t reg)
{
    uint8_t value[2];
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, &reg, 1, true,
                             time_us_64() + IO_TIMEOUT_US);
    i2c_read_blocking_until(I2C_PORT, I2C_ADDR, value, 2, false,
                             time_us_64() + IO_TIMEOUT_US);
    return (value[0] << 8) | value[1];
}

/*
static void write_many(uint8_t reg, const uint8_t *src, uint8_t len)
{
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, &reg, 1, true,
                             time_us_64() + IO_TIMEOUT_US);
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, src, len, false,
                             time_us_64() + IO_TIMEOUT_US);
}

static void read_many(uint8_t reg, uint8_t *dst, uint8_t len)
{
    i2c_write_blocking_until(I2C_PORT, I2C_ADDR, &reg, 1, true,
                             time_us_64() + IO_TIMEOUT_US);
    i2c_read_blocking_until(I2C_PORT, I2C_ADDR, dst, len, false,
                             time_us_64() + IO_TIMEOUT_US * len);
}
*/

void tmag5273_init(unsigned instance, i2c_inst_t *i2c_port, uint8_t i2c_addr)
{
    if (instance < INSTANCE_NUM) {
        instances[instance].port = i2c_port;
        instances[instance].addr = i2c_addr ? i2c_addr : TMAG5273_DEF_ADDR;
    }
}

void tmag5273_use(unsigned instance)
{
    if (instance < INSTANCE_NUM) {
        current_instance = instance;
    }
}

bool tmag5273_is_present()
{
    return false;
}

bool tmag5273_init_sensor()
{
    tmag5273_write_reg(0x03, 0x01 << 2); // Enable angle calculation
    tmag5273_write_reg(0x02, 0x03 << 4); // X-Y
    tmag5273_write_reg(0x01, 0x02); // Continuous mode
    return true;
}

uint8_t tmag5273_read_reg(uint8_t addr)
{
    return read_reg(addr);
}

void tmag5273_write_reg(uint8_t addr, uint8_t value)
{
    write_reg(addr, value);
}

uint16_t tmag5273_read_angle()
{
    return (read_reg16(0x19) & 0x1fff) >> 4;
}
