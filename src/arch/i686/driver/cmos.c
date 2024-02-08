/**
 * @file cmos.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stddef.h>

#include "io.h"
#include "cmos.h"
#include "idt.h"

CMOS cmos = {
    .values = {},
};

int32_t bcd_to_decimal(const int32_t bcd)
{
    const int32_t upper_nibble = (bcd & 0xF0) >> 4;
    const int32_t lower_nibble = bcd & 0x0F;
    const int32_t decimal = (upper_nibble * 10) + lower_nibble;
    return decimal;
};

void cmos_dump(CMOS *self)
{
    uint16_t cmos_register;

    for (cmos_register = 0; cmos_register < 128; cmos_register++)
    {
        asm_outb(CMOS_PORT_INDEX, cmos_register);
        const uint8_t value = asm_inb(CMOS_PORT_DATA);

        for (size_t i = 0; i < 100; i++)
        {
            asm_do_nop();
        };
        self->values[cmos_register] = value;
    };
    return;
};

Date cmos_date(CMOS *self)
{
    cmos_dump(self);
    const int32_t bcd_weekday = self->values[6];
    const int32_t bcd_day = self->values[7];
    const int32_t bcd_month = self->values[8];
    const int32_t bcd_century = self->values[32];
    const int32_t bcd_year = self->values[9];

    const int32_t weekday = bcd_to_decimal(bcd_weekday);
    const int32_t day = bcd_to_decimal(bcd_day);
    const int32_t month = bcd_to_decimal(bcd_month);
    const int32_t year = bcd_century * 10 + bcd_to_decimal(bcd_year);

    Date date = {
        .weekday = weekday,
        .day = day,
        .month = month,
        .year = year,
    };
    return date;
};