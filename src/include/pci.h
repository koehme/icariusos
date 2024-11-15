/**
 * @file pci.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef struct pci_dev_t {
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

// PCI configuration ports
#define PCI_CONFIG_ADDR 0xCF8 // Configuration address register
#define PCI_CONFIG_DATA 0xCFC // Configuration data register
#define PCI_OFFSET_MASK 0xFC  // Mask to align offset to 4-byte boundary
// PCI configuration register offsets
#define PCI_VENDOR_ID_REG_OFFSET 0x00	  // Identifies the device vendor
#define PCI_DEVICE_ID_REG_OFFSET 0x02	  // Identifies the specific device
#define PCI_COMMAND_REG_OFFSET 0x04	  // Device control and PCI cycle response
#define PCI_STATUS_REG_OFFSET 0x06	  // Status information for PCI events
#define PCI_REVISION_ID_REG_OFFSET 0x08	  // Device revision identifier
#define PCI_PROG_IF_REG_OFFSET 0x09	  // Programming interface identifier
#define PCI_SUBCLASS_REG_OFFSET 0x0A	  // Specific function of the device
#define PCI_CLASS_CODE_REG_OFFSET 0x0B	  // Device function class
#define PCI_CACHE_LINE_SIZE 0x0C	  // System cache line size
#define PCI_LATENCY_TIMER_REG_OFFSET 0x0D // PCI latency timer in bus clock units
#define PCI_HEADER_TYPE_REG_OFFSET 0x0E	  // Header type (general, bridge, etc.)
#define PCI_BIST_REG_OFFSET 0x0F	  // Built-in self-test control and status
#define PCI_DEV_NOT_FOUND 0xFFFF	  // Value indicating device not present

uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset);
void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data);
void pci_enumerate_bus(void);

#endif