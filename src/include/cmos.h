/**
 * @file cmos.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef CMOS_H
#define CMOS_H

#include <stdint.h>

typedef enum CMOSPorts
{
    CMOS_PORT_INDEX = 0x70,
    CMOS_PORT_DATA = 0x71
} CMOSPorts;

typedef struct Date
{
    uint16_t weekday;
    uint16_t day;
    uint16_t month;
    uint16_t year;
} Date;

typedef struct CMOS
{
    uint16_t values[128];
} CMOS;

Date cmos_date(CMOS *self);

#endif