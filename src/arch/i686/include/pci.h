/**
 * @file pci.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef enum PCI_CONFIG_PORTS
{
    PCI_CONFIG_ADDR = 0xCF8, // Specifies the configuration address that is required to be accesse
    PCI_CONFIG_DATA = 0xCFC, // Generate the configuration access and will transfer the data to or from the CONFIG_DATA register
    PCI_OFFSET_MASK = 0xFC,  // Masking the first two bits of the offset (these are the two lowest bits) with 0xFC ensures that the generated addresses in the PCI configuration space are always aligned to 4-byte boundaries, which is important for correct access to the registers
} PCI_CONFIG_PORTS;

typedef enum PCI_CONFIG_REG_OFFSET
{
    PCI_VENDOR_ID_REG_OFFSET = 0x00,     // Identifies the particular device. Where valid IDs are allocated by the vendor
    PCI_DEVICE_ID_REG_OFFSET = 0x02,     // Identifies the manufacturer of the device
    PCI_COMMAND_REG_OFFSET = 0x04,       // Provides control over a device's ability to generate and respond to PCI cycles
    PCI_STATUS_REG_OFFSET = 0x06,        // A register used to record status information for PCI bus related events
    PCI_REVISION_ID_REG_OFFSET = 0x08,   // Specifies a revision identifier for a particular device
    PCI_PROG_IF_REG_OFFSET = 0x09,       // A read-only register that specifies a register-level programming interface the device has, if it has any at all
    PCI_SUBCLASS_REG_OFFSET = 0x0A,      // A read-only register that specifies the specific function the device performs
    PCI_CLASS_CODE_REG_OFFSET = 0x0B,    // A read-only register that specifies the type of function the device performs
    PCI_CACHE_LINE_SIZE = 0x0C,          // Specifies the system cache line size in 32-bit units
    PCI_LATENCY_TIMER_REG_OFFSET = 0x0D, // Specifies the latency timer in units of PCI bus clocks
    PCI_HEADER_TYPE_REG_OFFSET = 0x0E,   // 0x0: a general device, 0x1: a PCI-to-PCI bridge, 0x2: a PCI-to-CardBus bridge
    PCI_BIST_REG_OFFSET = 0x0F,          // Represents that status and allows control of a devices BIST (built-in self test)
} PCI_CONFIG_REG_OFFSET;

typedef enum PCI_STATE
{
    PCI_DEV_NOT_FOUND = 0xFFFF,
} PCI_STATE;

uint16_t pci_read16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset);
void pci_write16(const uint32_t bus, const uint32_t device, const uint32_t function, const uint8_t offset, const uint16_t data);
void pci_devices_enumerate(void);

#endif