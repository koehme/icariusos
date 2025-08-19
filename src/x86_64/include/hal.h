/**
 * @file hal.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "kres.h"

#include <stdbool.h>
#include <stddef.h>

extern kresult_t asm_sse_setup(void);
extern void asm_hlt(void);
extern void asm_busy_wait(const size_t loops);