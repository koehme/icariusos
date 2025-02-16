/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ICARIUS_H
#define ICARIUS_H

/*
====================================
    IDT Gate Type
====================================
*/
#define IDT_GATE_TASK 0x05	 // Task Gate
#define IDT_GATE_16BIT_INT 0x06	 // 16-bit Interrupt Gate
#define IDT_GATE_16BIT_TRAP 0x07 // 16-bit Trap Gate
#define IDT_GATE_32BIT_INT 0x0E	 // 32-bit Interrupt Gate
#define IDT_GATE_32BIT_TRAP 0x0F // 32-bit Trap Gate
/*
====================================
    IDT Descriptor Privilege Level (DPL)
====================================
*/
#define IDT_DPL_KERNEL 0x00 // Ring 0 (Kernel)
#define IDT_DPL_USER 0x60   // Ring 3 (Usermode) -> 01100000 (DPL 3)
/*
====================================
    IDT Present Bit
====================================
*/
#define IDT_PRESENT 0x80							  // Interrupt Descriptor Present
#define IDT_KERNEL_INT_GATE (IDT_GATE_32BIT_INT | IDT_DPL_KERNEL | IDT_PRESENT)	  // 0x8E
#define IDT_USER_INT_GATE (IDT_GATE_32BIT_INT | IDT_DPL_USER | IDT_PRESENT)	  // 0xEE
#define IDT_KERNEL_TRAP_GATE (IDT_GATE_32BIT_TRAP | IDT_DPL_KERNEL | IDT_PRESENT) // 0x8F
#define IDT_USER_TRAP_GATE (IDT_GATE_32BIT_TRAP | IDT_DPL_USER | IDT_PRESENT)	  // 0xEF
/*
====================================
    PIT
====================================
*/
#define PIT_BINARY_MODE 0b00000000     // 16-Bit binary mode (Bit 0)
#define PIT_OPERATING_MODE 0b00000110  // Square wave generator (Bit 1-3: 011)
#define PIT_ACCESS_MODE 0b00110000     // Access mode lo/hi byte (Bit 4-5: 11)
#define PIT_CHANNEL 0b00000000	       // Select channel 0 (Bit 6-7: 00)
#define PIT_DATA_PORT_CHANNEL_0 0x40   // Channel 0 data port (read/write)
#define PIT_MODE_COMMAND_REGISTER 0x43 // Mode/Command register (write only)
/*
====================================
    PS2
====================================
*/
#define PS2_BUFFER_OUTPUT 0x01
#define PS2_BUFFER_INPUT 0x02
#define PS2_DATA_PORT 0x60
#define PS2_STATUS_COMMAND_PORT 0x64
#define PS2_PACKAGE_DISPATCH 32
/*
====================================
    PIC
====================================
*/
#define PIC_1_CTRL 0x20
#define PIC_1_DATA 0x21
#define PIC_2_CTRL 0xa0
#define PIC_2_DATA 0xa1
#define PIC_ACK 0x20
#define ICW_1 0x11     // Initialize PIC and expect ICW4
#define MODE_8086 0x01 // 8086/88 (MCS-80/85) mode
#define IRQ_0 0x20     // Offset for IRQs 0-7 (Master PIC)
#define IRQ_8 0x28     // Offset for IRQs 8-15 (Slave PIC)
/*
====================================
    GDT Entry Access Flag
====================================
*/
#define GDT_ACCESS_NULL 0x00	    // Null-Segment
#define GDT_ACCESS_KERNEL_CODE 0x9A // Code-Segment Kernel (Ring 0), X, R, P
#define GDT_ACCESS_KERNEL_DATA 0x92 // Data-Segment Kernel (Ring 0), R/W, P
#define GDT_ACCESS_USER_CODE 0xFA   // Code-Segment User (Ring 3), X, R, P
#define GDT_ACCESS_USER_DATA 0xF2   // Data-Segment User (Ring 3), R/W, P
#define GDT_ACCESS_TSS 0x89	    // TSS-Segment (32-Bit TSS, Ring 0, P)
/*
====================================
    GDT Segment Offset
====================================
*/
#define GDT_NULL_SEGMENT 0x00	     // Null-Segment Offset
#define GDT_KERNEL_CODE_SEGMENT 0x08 // Offset Kernel-Code-Segment
#define GDT_KERNEL_DATA_SEGMENT 0x10 // Offset Kernel-Data-Segment
#define GDT_USER_CODE_SEGMENT 0x18   // Offset User-Code-Segment
#define GDT_USER_DATA_SEGMENT 0x20   // Offset User-Data-Segment
#define GDT_TSS_SEGMENT 0x28	     // Offset TSS-Segment
/*
====================================
    GDT Flags for i686 (32-bit Protected Mode)
====================================
*/
#define GDT_FLAG_GRANULARITY_4K 0x80								     // Use 4 KiB granularity
#define GDT_FLAG_32BIT_MODE 0x40								     // 32-bit protected mode
#define GDT_FLAG_LIMIT_HIGH_MASK 0x0F								     // Upper 4 bits of segment limit
#define GDT_FLAGS_DEFAULT (GDT_FLAG_GRANULARITY_4K | GDT_FLAG_32BIT_MODE | GDT_FLAG_LIMIT_HIGH_MASK) // 0xCF
/*
====================================
    Memory i686
====================================
*/
#define KERNEL_VIRTUAL_START 0xC0000000
#define MAX_KERNEL_SIZE (16 * 1024 * 1024) // 16 MiB

#define MAX_PHYSICAL_MEMORY 0x100000000
#define PAGE_SIZE (1024 * 4 * 1024)
#define MAX_FRAMES (MAX_PHYSICAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE (MAX_FRAMES / 32)
#define KERNEL_PHYS_BASE 0x00000000
#define KERNEL_PHYS_END 0x02FFFFFF
#define KERNEL_VIRT_BASE 0xC0000000
/*
====================================
    Paging
====================================
*/
#define PAGE_SIZE (1024 * 4 * 1024) // 4 MiB
#define PAGE_PRESENT 0x1	    // Page is present in physical memory
#define PAGE_WRITABLE 0x2	    // Page is writable (read/write access)
#define PAGE_USER 0x4		    // User-mode access allowed (0 = kernel-only)
#define PAGE_PWT 0x8		    // Page Write-Through enabled
#define PAGE_PCD 0x10		    // Page Cache Disable (no caching for this page)
#define PAGE_ACCESSED 0x20	    // Set by the CPU when the page is accessed
#define PAGE_DIRTY 0x40		    // Set by the CPU when the page is written to
#define PAGE_PS 0x80		    // Page Size (1 = 4 MiB page, only in PDE)
#define PAGE_GLOBAL 0x100	    // Global page (remains cached in TLB across context switches)
/*
====================================
    Kernel Heap
====================================
*/
#define KERNEL_HEAP_START 0xC1000000
#define KERNEL_HEAP_MAX 0xC2BFFFFF
#define KERNEL_HEAP_CHUNK_SIZE 4096
/*
====================================
    Kernel Stack
====================================
*/
#define KERNEL_STACK_SIZE 0x8000				       // 32 KiB
#define KERNEL_STACK_BOTTOM (KERNEL_HEAP_MAX + 1)		       // 0xC2C00000
#define KERNEL_STACK_TOP (KERNEL_STACK_BOTTOM + KERNEL_STACK_SIZE - 1) // 0xC2C07FFF
#define RESERVED_STACK_START (KERNEL_STACK_TOP + 1)		       // 0xC2C08000
#define RESERVED_STACK_END 0xC2FFFFFF				       // Full 4 MiB Page Ends
/*
====================================
    PCI Configuration Port
====================================
*/
#define PCI_CONFIG_ADDR 0xCF8 // Configuration address register
#define PCI_CONFIG_DATA 0xCFC // Configuration data register
#define PCI_OFFSET_MASK 0xFC  // Mask to align offset to 4-byte boundary
/*
====================================
    PCI Configuration Register Offset
====================================
*/
#define PCI_VENDOR_ID_REG_OFFSET 0x00	  // Identifies the device vendor
#define PCI_DEVICE_ID_REG_OFFSET 0x02	  // Identifies the specific device
#define PCI_COMMAND_REG_OFFSET 0x04	  // Device control and PCI cycle response
#define PCI_STATUS_REG_OFFSET 0x06	  // Status information for PCI events
#define PCI_REVISION_ID_REG_OFFSET 0x08	  // Device revision identifier
#define PCI_PROG_IF_REG_OFFSET 0x09	  // Programming interface identifier
#define PCI_SUBCLASS_REG_OFFSET 0x0A	  // Specific function of the device
#define PCI_CLASS_CODE_REG_OFFSET 0x0B	  // Device function class
#define PCI_CACHE_LINE_SIZE 0x0C	  // System cache line size
#define PCI_LATENCY_TIMER_REG_OFFSET 0x0D // PCI latency timer_t in bus clock units
#define PCI_HEADER_TYPE_REG_OFFSET 0x0E	  // Header type (general, bridge, etc.)
#define PCI_BIST_REG_OFFSET 0x0F	  // Built-in self-test control and status
#define PCI_DEV_NOT_FOUND 0xFFFF	  // Value indicating device not present
/*
====================================
    VGA Textmode (Deprecated)
====================================
*/
#define VGA_CTRL 0x3D4
#define VGA_DATA 0x3D5
#define VGA_LOW_OFFSET 0x0F
#define VGA_HIGH_OFFSET 0x0E
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW 14
#define VGA_COLOR_WHITE 15
/*
====================================
    VBE Framebuffer
====================================
*/
#define FRAMEBUFFER_VIRT_BASE 0xE0000000
#define FRAMEBUFFER_PHYS_BASE 0xFD000000
#define FRAMEBUFFER_WIDTH 800
#define FRAMEBUFFER_HEIGHT 600
#define FRAMEBUFFER_DEPTH (32 / 8)
#define FRAMEBUFFER_SIZE ((FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT) * FRAMEBUFFER_DEPTH)
#define ASCII 128     // ASCII Encoding 128 Characters
#define FONT_WIDTH 8  // Width of a Font Character in Pixels
#define FONT_HEIGHT 8 // Height of a Font Character in Pixels
/*
====================================
    Mouse Command
====================================
*/
#define MOUSE_ENABLE_AUX 0xA8	      // Enable auxiliary device (mouse)
#define MOUSE_DISABLE 0xA7	      // Disable mouse
#define MOUSE_GET_COMPAQ_STATUS 0x20  // Get Compaq status byte
#define MOUSE_SET_COMPAQ_STATUS 0x60  // Set Compaq status byte
#define MOUSE_SEND_COMMAND 0xD4	      // Send a command to the mouse
#define MOUSE_ENABLE_DATA_REPORT 0xF4 // Enable mouse data reporting
#define MOUSE_SET_DEFAULT 0xF6	      // Set mouse to default settings
#define MOUSE_DETECT_ACK 0xFA	      // Acknowledge received command
/*
====================================
    Mouse Flag
====================================
*/
#define LEFT_BUTTON_MASK 0b00000001	// Left button pressed
#define RIGHT_BUTTON_MASK 0b00000010	// Right button pressed
#define MIDDLE_BUTTON_MASK 0b00000100	// Middle button pressed
#define ALIGNED_PACKET_MASK 0b00001000	// Packet alignment
#define Y_AXIS_OVERFLOW_MASK 0b01000000 // Y-axis overflow
#define X_AXIS_OVERFLOW_MASK 0b10000000 // X-axis overflow
#define SIGN_BIT_MASK 0b100000000	// Sign bit for movement
/*
====================================
    Keyboard
====================================
*/
#define KEYBOARD_CTRL_STATS_MASK_OUT_BUF 0x1  // 00000001
#define KEYBOARD_CTRL_STATS_MASK_IN_BUF 0x2   // 00000010
#define KEYBOARD_CTRL_STATS_MASK_SYSTEM 0x4   // 00000100
#define KEYBOARD_CTRL_STATS_MASK_CMD_DATA 0x8 // 00001000
#define KEYBOARD_CTRL_STATS_MASK_LOCKED 0x10  // 00010000
#define KEYBOARD_CTRL_STATS_MASK_AUX_BUF 0x20 // 00100000
#define KEYBOARD_CTRL_STATS_MASK_TIMEOUT 0x40 // 01000000
#define KEYBOARD_CTRL_STATS_MASK_PARITY 0x80  // 10000000
/*
====================================
    Keyboard Encoder Port
====================================
*/
#define KEYBOARD_ENC_INPUT_BUF 0x60
#define KEYBOARD_ENC_CMD_REG 0x60
/*
====================================
    Keyboard Controller Port
====================================
*/
#define KEYBOARD_CTRL_STATS_REG 0x64
#define KEYBOARD_CTRL_CMD_REG 0x64
/*
====================================
    Keyboard Controller Command
====================================
*/
#define KEYBOARD_CTRL_SELF_TEST 0xAA
#define KEYBOARD_CTRL_ENABLE 0xAE
#define KEYBOARD_CTRL_DISABLE 0xAD
#define KESC 0x01
#define KLEFT_SHIFT 0x2A
#define KRIGHT_SHIFT 0x36
#define KCAPS_LOCK 0x3A
#define KALTGR 0x60
/*
====================================
    ATA Limit
====================================
*/
#define ATA_SECTOR_SIZE 512
/*
====================================
    ATA Drive Type
====================================
*/
#define ATA_DRIVE_MASTER 0xE0
#define ATA_DRIVE_SLAVE 0xF0
#define DRIVE_SELECT_MASTER 0xA0
#define DRIVE_SELECT_SLAVE 0xB0
#define DRIVE_SELECT_INVALID 0x0
/*
====================================
    ATA Commands
====================================
*/
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_IDENTIFY 0xEC
/*
====================================
    ATA Status Flags
====================================
*/
#define ATA_STATUS_ERR (1 << 0) // Indicates an error occurred. Send a new command to clear it
#define ATA_STATUS_NIEN (1 << 1)
#define ATA_STATUS_DRQ (1 << 3) // Set when the drive has PIO data to transfer or is ready to accept PIO data
#define ATA_STATUS_SRV (1 << 4) // Overlapped Mode Service Request
#define ATA_STATUS_DF (1 << 5)	// Drive Fault Error (does not set ERR)
#define ATA_STATUS_RDY (1 << 6) // Bit is clear when drive is spun down, or after an error. Set otherwise
#define ATA_STATUS_BSY (1 << 7) // Indicates the drive is preparing to send/receive data (wait for it to clear)
/*
====================================
    ATA Ports
====================================
*/
#define ATA_DATA_PORT 0x1F0
#define ATA_FEATURES_PORT 0x1F1
#define ATA_PRIMARY_ERROR 0x1F1
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT 0x1F3
#define ATA_LBA_MID_PORT 0x1F4
#define ATA_LBA_HIGH_PORT 0x1F5
#define ATA_CONTROL_PORT 0x1F6
#define ATA_COMMAND_PORT 0x1F7
#define ATA_STATUS_REGISTER 0x1F7
/*
====================================
    CMOS
====================================
*/
#define CMOS_PORT_INDEX 0x70
#define CMOS_PORT_DATA 0x71
/*
====================================
    VNODE
====================================
*/
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define READ 0
#define WRITE 1
#define APPEND 2
#define INVALID 3
/*
====================================
    FIFO
====================================
*/
#define FIFO_BUFFER_SIZE 128
#define FIFO_BUFFER_MASK (FIFO_BUFFER_SIZE - 1)
/*
====================================
    Userland
====================================
*/
#define USER_CODE_START 0x00000000
#define USER_CODE_END 0x3FFFFFFF
#define USER_HEAP_START (USER_CODE_END + 1)
#define USER_HEAP_END 0xBFBFFFFF
#define USER_STACK_START (USER_HEAP_END + 1)
#define USER_STACK_END 0xBFFFFFFF
#define FD_STDIN 0  // Standard Input
#define FD_STDOUT 1 // Standard Output
#define FD_STDERR 2 // Standard Error
#define FD_FILE 3
/*
====================================
    POSIX-Compatible Syscalls
====================================
*/
#define MAX_SYSCALL 256
#define SYS_EXIT 1  // _exit(int status)
#define SYS_WRITE 4 // write(int fd, const void *buf, size_t count)

#endif