/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bootstrap.h"
#include "fb.h"
#include "font.h"
#include "kres.h"
#include "panic.h"
#include "pfa.h"
#include "renderer.h"
#include "stdio.h"
#include "tty.h"
#include "types.h"

noreturn void kmain(void);