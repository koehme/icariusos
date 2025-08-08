/**
 * @file pci.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "errno.h"
#include "io.h"
#include "kernel.h"
#include "pci.h"
#include "stdio.h"

/* PUBLIC API */
uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset);
void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data);
void pci_enumerate_bus(void);

/* INTERNAL API */
static inline uint32_t _build_config_addr(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset);
static void _populate_dev(pci_dev_t* dev, const uint32_t bus, const uint32_t device, const uint32_t function);
static void _dump_dev(const pci_dev_t* dev);

static inline uint32_t _build_config_addr(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset)
{
	return (0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & PCI_OFFSET_MASK));
};

uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset)
{
	const uint32_t config_address = _build_config_addr(bus, device, function, offset);
	outl(PCI_CONFIG_ADDR, config_address);
	return (inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;
};

void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data)
{
	const uint32_t config_address = _build_config_addr(bus, device, function, offset);
	outl(PCI_CONFIG_ADDR, config_address);
	outl(PCI_CONFIG_DATA, data);
	return;
};

static void _populate_dev(pci_dev_t* dev, const uint32_t bus, const uint32_t device, const uint32_t function)
{
	dev->bus = bus;
	dev->device = device;
	dev->function = function;
	dev->vendor_id = pci_read16(bus, device, function, PCI_VENDOR_ID_REG_OFFSET);

	if (dev->vendor_id == PCI_DEV_NOT_FOUND) {
		return;
	};
	dev->device_id = pci_read16(bus, device, function, PCI_DEVICE_ID_REG_OFFSET);
	dev->command = pci_read16(bus, device, function, PCI_COMMAND_REG_OFFSET);
	dev->status = pci_read16(bus, device, function, PCI_STATUS_REG_OFFSET);
	dev->revision_id = pci_read16(bus, device, function, PCI_REVISION_ID_REG_OFFSET) & 0xFF;
	dev->prog_if = pci_read16(bus, device, function, PCI_PROG_IF_REG_OFFSET) & 0xFF;
	dev->subclass = pci_read16(bus, device, function, PCI_SUBCLASS_REG_OFFSET) & 0xFF;
	dev->class_code = pci_read16(bus, device, function, PCI_CLASS_CODE_REG_OFFSET) & 0xFF;
	dev->cache_line_size = pci_read16(bus, device, function, PCI_CACHE_LINE_SIZE) & 0xFF;
	dev->latency_timer = pci_read16(bus, device, function, PCI_LATENCY_TIMER_REG_OFFSET) & 0xFF;
	dev->header_type = pci_read16(bus, device, function, PCI_HEADER_TYPE_REG_OFFSET) & 0xFF;
	dev->bist = pci_read16(bus, device, function, PCI_BIST_REG_OFFSET) & 0xFF;
	return;
};

static void _dump_dev(const pci_dev_t* dev)
{
	kprintf("\n=============================================\n");
	kprintf(" Bus: %d\tDevice: %d\tFunction: %d\n", dev->bus, dev->device, dev->function);
	kprintf("---------------------------------------------\n");
	kprintf(" Vendor ID       : 0x%x\n", dev->vendor_id);
	kprintf(" Device ID       : 0x%x\n", dev->device_id);
	kprintf(" Class Code      : 0x%x\n", dev->class_code);
	kprintf(" Command         : 0x%x\n", dev->command);
	kprintf(" Status          : 0x%x\n", dev->status);
	kprintf(" Revision ID     : 0x%x\n", dev->revision_id);
	kprintf(" Prog IF         : 0x%x\n", dev->prog_if);
	kprintf(" Subclass        : 0x%x\n", dev->subclass);
	kprintf(" Cache Line Size : 0x%x\n", dev->cache_line_size);
	kprintf(" Latency Timer   : 0x%x\n", dev->latency_timer);
	kprintf(" Header Type     : 0x%x\n", dev->header_type);
	kprintf(" BIST            : 0x%x\n", dev->bist);
	kprintf("=============================================\n");
	return;
};

void pci_enumerate_bus(void)
{
	for (size_t bus = 0; bus < 256; ++bus) {
		for (size_t device = 0; device < 32; ++device) {
			for (size_t function = 0; function < 8; ++function) {
				pci_dev_t dev;
				_populate_dev(&dev, bus, device, function);

				if (dev.vendor_id != PCI_DEV_NOT_FOUND) {
					_dump_dev(&dev);
				};
			};
		};
	};
	return;
};