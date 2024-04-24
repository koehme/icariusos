/**
 * @file pci.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stddef.h>
#include <stdbool.h>

#include "status.h"
#include "kernel.h"
#include "pci.h"
#include "io.h"

// Reads 16-bit from a PCI configuration register
uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset)
{
    // Build the register address to allow precise configuration control for PCI devices
    // The address format is:
    // Bit 31: Enable bit (1 for access to PCI configuration space)
    // Bits 30-24: Reserved (always set to 0)
    // Bits 23-16: Bus number
    // Bits 15-11: Device number
    // Bits 10-8: Function number
    // Bits 7-2: Register offset
    // Bit 1-0: Reserved (always set to 0)
    const uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & PCI_OFFSET_MASK));
    // Write the specific config address to the pci controller
    outl(PCI_CONFIG_ADDR, config_address);
    // The PCI config register consists of 256 bytes.
    // These 256 bytes are divided into 4 bytes each. That makes 64 registers in total.
    // Read the 32-bit data from the PCI config data register, then shift and mask to extract the desired 16-bit word by the offset
    // ------------------------------------------------------------
    // |      Byte 3     |      Byte 2     |      Byte 1     |      Byte 0    |
    // | 31 ... 24       | 23 ... 16       | 15 ... 8        | 7 ... 0        |
    // ------------------------------------------------------------
    // offset = 0x00 no shifting required. offset = 0x02 shifting is required because the missing 2 bytes are in the higher parts of the 32 bit dword.
    const uint16_t register_data = ((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return register_data;
};

// Write 16-bit data to a PCI configuration register
void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data)
{
    // Build the register address to allow precise configuration control for PCI devices
    // The address format is:
    // Bit 31: Enable bit (1 for access to PCI configuration space)
    // Bits 30-24: Reserved (always set to 0)
    // Bits 23-16: Bus number
    // Bits 15-11: Device number
    // Bits 10-8: Function number
    // Bits 7-2: Register offset
    // Bit 1-0: Reserved (always set to 0)
    const uint32_t config_address = (uint32_t)((uint32_t)0x80000000 | (bus << 16) | (device << 11) | (function << 8) | (offset & PCI_OFFSET_MASK));
    // Write the specific configuration address to the PCI controller, to allow direct access to the configuration space of PCI device
    outl(PCI_CONFIG_ADDR, config_address);
    // Write the 16-bit data to the specified PCI configuration register, to configures the desired parameter of the PCI device
    outl(PCI_CONFIG_DATA, data);
    return;
};

void pci_devices_enumerate(void)
{
    for (size_t bus = 0; bus < 256; bus++)
    {
        for (size_t device = 0; device < 32; device++)
        {
            for (size_t function = 0; function < 8; function++)
            {
                const uint16_t vendor_id = pci_read16(bus, device, function, PCI_VENDOR_ID_REG_OFFSET);

                if (vendor_id == PCI_DEV_NOT_FOUND)
                {
                    continue;
                };
                const uint16_t device_id = pci_read16(bus, device, function, PCI_DEVICE_ID_REG_OFFSET);

                const uint16_t command = pci_read16(bus, device, function, PCI_COMMAND_REG_OFFSET);
                const uint16_t status = pci_read16(bus, device, function, PCI_STATUS_REG_OFFSET);

                const uint8_t revision_id = pci_read16(bus, device, function, PCI_REVISION_ID_REG_OFFSET);
                const uint8_t prog_if = pci_read16(bus, device, function, PCI_PROG_IF_REG_OFFSET);
                const uint8_t subclass = pci_read16(bus, device, function, PCI_SUBCLASS_REG_OFFSET);
                const uint8_t class_code = pci_read16(bus, device, function, PCI_CLASS_CODE_REG_OFFSET);

                const uint8_t cache_line_size = pci_read16(bus, device, function, PCI_CACHE_LINE_SIZE);
                const uint8_t latency_timer = pci_read16(bus, device, function, PCI_LATENCY_TIMER_REG_OFFSET);
                const uint8_t header_type = pci_read16(bus, device, function, PCI_HEADER_TYPE_REG_OFFSET);
                const uint8_t bist = pci_read16(bus, device, function, PCI_BIST_REG_OFFSET);

                printf("\nBus: %d, Device: %d, Function: %d \n Vendor ID: 0x%x, Device ID: 0x%x, Class Code: 0x%x\n Command: 0x%x, Status: 0x%x, Revision ID: 0x%x\n Prog IF: 0x%x, Subclass: 0x%x, Cache Line Size: 0x%x\n Latency Timer: 0x%x, Header Type: 0x%x, BIST: 0x%x\n",
                       bus, device, function,
                       vendor_id, device_id, class_code,
                       command, status, revision_id,
                       prog_if, subclass, cache_line_size,
                       latency_timer, header_type, bist);
            };
        };
    };
    return;
};