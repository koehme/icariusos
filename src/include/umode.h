/**
 * @file umode.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef UMODE_H
#define UMODE_H

#include "pfa.h"

// User Program Code
#define USER_CODE_START 0x00000000
#define USER_CODE_END 0x3FFFFFFF
// User Heap
#define USER_HEAP_START (USER_CODE_END + 1)
#define USER_HEAP_END 0xBFBFFFFF
// User Stack
#define USER_STACK_START (USER_HEAP_END + 1)
#define USER_STACK_END 0xBFFFFFFF

#endif