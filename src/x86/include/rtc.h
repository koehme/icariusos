/**
 * @file rtc.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef RTC_H
#define RTC_H

#include <stdint.h>

#define TIMEZONE_FILE "A:/ETC/TIMEZONE"

static const char* months[12] = {
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December",
};

static const char* days[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

typedef struct date {
	uint16_t weekday;
	uint16_t day;
	uint16_t month;
	uint16_t year;
} date_t;

typedef struct time {
	int32_t hour;
	int32_t minute;
	int32_t second;
} time_t;

void rtc_load_timezone(void);
time_t rtc_now(void);

#endif