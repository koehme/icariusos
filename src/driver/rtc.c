/**
 * @file rtc.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "rtc.h"
#include "cmos.h"
#include "stdlib.h"
#include "vfs.h"
#include <stdbool.h>

/* PUBLIC API */
void rtc_load_timezone(void);
time_t rtc_now(void);

/* INTERNAL API */
static inline bool _is_last_sunday(int day, int weekday);
static bool _is_summer_de(date_t* date);
static int _resolve_timezone_offset(const char* name);

static int timezone_offset = 0;

typedef struct timezone_entry {
	const char* name;
	int offset;
} timezone_entry_t;

static timezone_entry_t timezones[] = {
    {"UTC", +0},
    {"Europe/Berlin", +2},
    {"America/New_York", -4},
};

static inline bool _is_last_sunday(int day, int weekday) { return (day + (7 - weekday)) > 31; }

static bool _is_summer_de(date_t* date)
{
	if (date->month < 3 || date->month > 10)
		return false;

	if (date->month > 3 && date->month < 10)
		return true;

	const bool is_last = _is_last_sunday(date->day, date->weekday);

	if (date->month == 3)
		return is_last && cmos_time(&cmos).hour >= 2;

	if (date->month == 10)
		return !(is_last && cmos_time(&cmos).hour >= 3);
	return false;
};

static int _resolve_timezone_offset(const char* name)
{
	if (strcmp(name, "Europe/Berlin") == 0) {
		date_t date = cmos_date(&cmos);
		return _is_summer_de(&date) ? 2 : 1;
	};

	for (size_t i = 0; i < sizeof(timezones) / sizeof(timezone_entry_t); i++) {
		if (strcmp(name, timezones[i].name) == 0) {
			return timezones[i].offset;
		};
	};
	return 0;
};

void rtc_load_timezone(void)
{
	int offset = 0;
	const int32_t fd = vfs_fopen("A:/ETC/TIMEZONE", "r");

	if (!fd) {
		kprintf("[RTC] Defaulting to UTC +0\n");
		return;
	};
	char buf[32] = {0};
	const int32_t n_bytes = vfs_fread(buf, 1, sizeof(buf) - 1, fd);
	vfs_fclose(fd);

	if (n_bytes <= 0) {
		kprintf("[RTC] Defaulting to UTC +0\n");
		return;
	};
	for (int i = 0; i < n_bytes; i++) {
		if (buf[i] == '\n' || buf[i] == '\r' || buf[i] == ' ') {
			buf[i] = '\0';
			break;
		};
	};
	timezone_offset = _resolve_timezone_offset(buf);

	if (timezone_offset >= 0)
		kprintf("[RTC] Loaded Timezone: '%s' -> UTC +%d\n", buf, timezone_offset);
	else
		kprintf("[RTC] Loaded Timezone: '%s' -> UTC %d\n", buf, timezone_offset);
	return;
};

time_t rtc_now(void)
{
	time_t time = cmos_time(&cmos);
	time.hour += timezone_offset;
	return time;
};