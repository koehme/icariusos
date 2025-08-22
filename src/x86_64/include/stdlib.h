/**
 * @file stdlib.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "types.h"

usize itoa(s64 value, const int base, const bool upper, char* buf);
usize utoa(u64 value, const int base, const bool upper, char* buf);