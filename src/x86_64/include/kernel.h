/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "fb.h"
#include "font.h"
#include "hal.h"
#include "init.h"
#include "kerr.h"
#include "panic.h"
#include "renderer.h"
#include "tty.h"

void kmain(void);