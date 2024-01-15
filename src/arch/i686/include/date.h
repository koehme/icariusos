/**
 * @file date.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef DATE_H
#define DATE_H

#include <stdint.h>

const static char *months[12] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
};

const static char *days[7] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
};

typedef struct Date
{
    uint16_t weekday;
    uint16_t day;
    uint16_t month;
    uint16_t year;
} Date;

#endif