# System Memory Manager

My system memory manager heap is implemented as an array, not a linked list, for speed and CPU cache-friendliness.

This initial structure maintains a table or descriptor for the heap. In this table, each byte represents a real memory block in the data pool of the other structure. My heap data pool starts at memory address 0x01000000, which corresponds to 16777216 in decimal. The first block in the data pool also starts at 0x01000000, and regardless of the desired allocation size, it will be aligned to 4096 bytes.

# Example

Here, 50 bytes are requested and aligned to 4096, considered as a single allocation block.

```c
kmalloc(50);
```

In the second call, 5000 bytes are requested, which will consume 2 blocks (4096 + 4096 = 8192 bytes) for allocation.

```c
kmalloc(5000);
```

My HeapDescriptor works with status flags to determine when a block is no longer free, whether it utilizes another block in the allocation, and marks the end of an allocation.

For now, my heap data pool supports 104857600 bytes (10241024100). When divided by the block size of 4096 bytes, I get 25600 bytes or 25 kilobytes required for the HeapDescriptor.

HeapDescriptor operates in real mode memory 0x00007E00 to 0x0007FFFF (480.5 KiB) since I'm already in Protected Mode. The starting address of the heap data pool is 0x01000000 (16777216 in decimal) + (104857600 in decimal) = End address 0x73FFFFF.

# Paging

I have successfully implemented paging in my 32-bit kernel. The PageDirectory consists of 1024 entries, and each entry points to a PageTable. Each PageTable manages 1024 entries, which in turn represent 4096 bytes - one page in the world of memory.

Index 0 of the PageDirectory starts at the physical address 0x0, while index 2 is at 0x400000 (0x400000 == decimal 1024*4096 = 4194304). This creates a linear assignment of the physical addresses to the PageTable entries.

In summary, the system consists of a PageDirectory with 1024 entries, with each entry pointing to a PageTable. Each PageTable has 1024 entries, each representing 4096 bytes. This results in a total memory of 4 GB or 4,294,967,296 bytes.

# Example

With the function 

```c
void page_init_directory(PageDirectory *self, const uint8_t flags);
```

I have initialized a linear PageDirectory with physical addresses. The linking of virtual addresses to physical addresses has been successfully implemented. For example, I can now map for example the virtual address 0x2000 to any physical equivalent.

```c
// Assume ptr_phy_addr = 0x1802000
char *ptr_phy_addr = kcalloc(4096);
virt_address_map(ptr_kpage_dir->directory, (void *)0x2000, (uint32_t)ptr_phy_addr | PAGE_ACCESSED | PAGE_PRESENT | PAGE_READ_WRITE);

asm_do_sti();
kprintf("Enable Interrupts...\n");

char *ptr_virt = (char *)0x2000;

// Change the virtual address to see reflected changes because we map 0x2000 => to a physical addr 0x1802000
ptr_virt[0] = 'A';
ptr_virt[1] = 'W';
ptr_virt[2] = 'E';
ptr_virt[3] = 'S';
ptr_virt[4] = 'O';
ptr_virt[5] = 'M';
ptr_virt[6] = 'E';
ptr_virt[7] = '!';

kprintf("Initializing Virtual Memory Paging...\n");
kprintf("Testing Paging...\nVirtual Address 0x2000 now points to Physical Address 0x1802000...\n");
kprintf("Virtual 0x2000 [");
kprintf(ptr_virt);
kprintf("] == ");
kprintf("Physical 0x1802000 [");
kprintf(ptr_phy_addr);
kprintf("]\n");