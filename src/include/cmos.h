/**
 * @file cmos.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef CMOS_H
#define CMOS_H

#include "rtc.h"
#include <stdint.h>

typedef struct cmos {
	uint16_t values[128];
} cmos_t;

extern cmos_t cmos;

date_t cmos_date(cmos_t* self);
time_t cmos_time(cmos_t* self);
int rtc_load_utc_offset(void);

#endif