/**
 * @file pci.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "pci.h"
#include "io.h"
#include <stddef.h>
#include <stdbool.h>

uint16_t pci_read_word(const uint8_t bus, const uint8_t device, const uint8_t function, const uint8_t offset)
{
    const uint32_t local_bus = (uint32_t)bus;
    const uint32_t local_device = (uint32_t)device;
    const uint32_t local_function = (uint32_t)function;
    // Build the register address to allow precise configuration control for PCI devices
    const uint32_t config_address = (uint32_t)((uint32_t)0x80000000) | (local_bus << 16) | (local_device << 11) | (local_function << 8) | (offset & PCI_OFFSET_MASK);
    // Write the specific config address to the pci controller
    asm_outl(PCI_CONFIG_ADDR, config_address);
    // The PCI config register consists of 256 bytes.
    // These 256 bytes are divided into 4 bytes each. That makes 64 registers in total.
    // Read the 32-bit data from the PCI config data register, then shift and mask to extract the desired 16-bit word by the offset
    // ------------------------------------------------------------
    // |      Byte 3     |      Byte 2     |      Byte 1     |      Byte 0    |
    // | 31 ... 24       | 23 ... 16       | 15 ... 8        | 7 ... 0        |
    // ------------------------------------------------------------
    // offset = 0x00 no shifting required. offset = 0x02 shifting is required because the missing 2 bytes are in the higher parts of the 32 bit dword.
    const uint16_t register_data = ((asm_inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return register_data;
};

void pci_devices_enumerate(void)
{
    for (size_t bus = 0; bus < 256; bus++)
    {
        for (size_t device = 0; device < 32; device++)
        {
            for (size_t function = 0; function < 8; function++)
            {
            };
        };
    };
    return 0;
};