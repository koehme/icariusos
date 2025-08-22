/**
 * @file stdlib.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#include "types.h"

usize itoa(s64 value, const s32 base, const b8 upper, ch* buf);
usize utoa(u64 value, const s32 base, const b8 upper, ch* buf);