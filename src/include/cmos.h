/**
 * @file cmos.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef CMOS_H
#define CMOS_H

#include "date.h"
#include <stdint.h>

typedef struct cmos {
	uint16_t values[128];
} cmos_t;

date_t cmos_date(cmos_t* self);

#endif