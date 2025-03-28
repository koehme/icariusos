/**
 * @file date.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef DATE_H
#define DATE_H

#include <stdint.h>

const static char* months[12] = {
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December",
};

const static char* days[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

typedef struct date {
	uint16_t weekday;
	uint16_t day;
	uint16_t month;
	uint16_t year;
} date_t;

typedef struct {
	int32_t hour;
	int32_t minute;
	int32_t second;
} time_t;

#endif