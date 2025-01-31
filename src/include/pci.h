/**
 * @file pci.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#include "icarius.h"

typedef struct pci_dev {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
	uint32_t bus;
	uint32_t device;
	uint32_t function;
} pci_dev_t;


uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset);
void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data);
void pci_enumerate_bus(void);

#endif