/**
 * @file cmos.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stddef.h>

#include "cmos.h"
#include "icarius.h"
#include "idt.h"
#include "io.h"

/* PUBLIC API */
date_t cmos_date(cmos_t* self);
int32_t cmos_bcd_to_decimal(const int32_t bcd);

/* INTERNAL API */
static void _dump_cmos(cmos_t* self);
static int32_t _bcd_to_decimal(const int32_t bcd);

cmos_t cmos = {
    .values = {},
};

static int32_t _bcd_to_decimal(const int32_t bcd)
{
	const int32_t upper_nibble = (bcd & 0xF0) >> 4;
	const int32_t lower_nibble = bcd & 0x0F;
	const int32_t decimal = (upper_nibble * 10) + lower_nibble;
	return decimal;
};

static void _dump_cmos(cmos_t* self)
{
	for (uint16_t i = 0; i < 128; i++) {
		outb(CMOS_PORT_INDEX, i);
		const uint8_t value = inb(CMOS_PORT_DATA);

		for (size_t i = 0; i < 100; i++) {
			asm_do_nop();
		};
		self->values[i] = value;
	};
	return;
};

time_t cmos_time(cmos_t* self)
{
	_dump_cmos(self);
	const int32_t bcd_hour = self->values[4];
	const int32_t bcd_minute = self->values[2];
	const int32_t bcd_second = self->values[0];

	const int32_t hour = _bcd_to_decimal(bcd_hour);
	const int32_t minute = _bcd_to_decimal(bcd_minute);
	const int32_t second = _bcd_to_decimal(bcd_second);

	time_t time = {
	    .hour = hour,
	    .minute = minute,
	    .second = second,
	};
	return time;
};

date_t cmos_date(cmos_t* self)
{
	_dump_cmos(self);
	const int32_t bcd_weekday = self->values[6];
	const int32_t bcd_day = self->values[7];
	const int32_t bcd_month = self->values[8];
	const int32_t bcd_century = self->values[32];
	const int32_t bcd_year = self->values[9];

	const int32_t weekday = _bcd_to_decimal(bcd_weekday);
	const int32_t day = _bcd_to_decimal(bcd_day);
	const int32_t month = _bcd_to_decimal(bcd_month);
	const int32_t year = bcd_century * 10 + _bcd_to_decimal(bcd_year);

	const date_t date = {
	    .weekday = weekday,
	    .day = day,
	    .month = month,
	    .year = year,
	};
	return date;
};