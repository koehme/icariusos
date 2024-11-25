/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "page.h"

/*
Page directory index (higher bits, e.g., bits 22-31).
Page table index (middle bits, e.g., bits 12-21).
Page offset (lower bits, e.g., bits 0-11).
*/

void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	/*
    Page directory index (higher bits, e.g., bits 22-31).
    Page table index (middle bits, e.g., bits 12-21).
    Page offset (lower bits, e.g., bits 0-11).
    */
	return;
};