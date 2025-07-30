############################
## Memory Layout Overview ##
############################

Formula: Entry Index * (1024*1024*4)
							^
							4 MiB

Example: 897 * 4194304 = 0xE0400000 - 0xE07FFFFF

| Virtual Address Range     | Physical Address Range    | Description                                | Page Directory Entry |
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| 0x00000000 - 0x003FFFFF   | 0x00000000 - 0x003FFFFF   | Identity Mapping for Kernel Initialization | Entry 0              |
| 0x00400000 - 0x007FFFFF   | 0x00400000 - 0x007FFFFF   | Identity Mapping for Kernel Initialization | Entry 1              |
| 0x00800000 - 0x00BFFFFF   | 0x00800000 - 0x00BFFFFF   | Identity Mapping for Kernel Initialization | Entry 2              |
| 0x00C00000 - 0x00FFFFFF   | 0x00C00000 - 0x00FFFFFF   | Identity Mapping for Kernel Initialization | Entry 3              |
| 0x01000000 - 0x013FFFFF   | 0x01000000 - 0x013FFFFF   | Identity Mapping for Kernel Initialization | Entry 4              |
| 0x01400000 - 0x017FFFFF   | 0x01400000 - 0x017FFFFF   | Identity Mapping for Kernel Initialization | Entry 5              |
| 0x01800000 - 0x01BFFFFF   | 0x01800000 - 0x01BFFFFF   | Identity Mapping for Kernel Initialization | Entry 6              |
| 0x01C00000 - 0x01FFFFFF   | 0x01C00000 - 0x01FFFFFF   | Identity Mapping for Kernel Initialization | Entry 7              |
| 0x02000000 - 0x023FFFFF   | 0x02000000 - 0x023FFFFF   | Identity Mapping for Kernel Initialization | Entry 8              |
| 0x02400000 - 0x027FFFFF   | 0x02400000 - 0x027FFFFF   | Identity Mapping for Kernel Initialization | Entry 9              |
| 0x02800000 - 0x02BFFFFF   | 0x02800000 - 0x02BFFFFF   | Identity Mapping for Kernel Initialization | Entry 10             |
| 0x02C00000 - 0x02FFFFFF   | 0x02C00000 - 0x02FFFFFF   | Identity Mapping for Kernel Initialization | Entry 11             |
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| 0x00000000 - 0x003FFFFF   | 0x00000000 - 0x003FFFFF   | _remove_identity_mapping()				 | Entry 1				|
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| `0x00000000 - 0x3FFFFFFF` |        Dynamic            | User Program Code (Max. 1 GiB)             | Entry   0-255        |
| `0x40000000 - 0xBFBFFFFF` |        Dynamic            | User Heap (Grows Upwards, Max. 1,75 GiB)   | Entry 256-766        |
| `0xBFC00000 - 0xBFFFFFFF` |        Dynamic            | User Stack (Grows Downwards, 4 MiB)   	 | Entry 767            |
| `0xC0000000 - 0xC03FFFFF` | `0x00000000 - 0x003FFFFF` | Kernel Memory (| 	  4 MiB)                 | Entry 768            |
| `0xC0400000 - 0xC07FFFFF` | `0x00400000 - 0x007FFFFF` | Kernel Memory (->   4 MiB)                 | Entry 769            |
| `0xC0800000 - 0xC0BFFFFF` | `0x00800000 - 0x00BFFFFF` | Kernel Memory (->   4 MiB)                 | Entry 770            |
| `0xC0C00000 - 0xC0FFFFFF` | `0x00C00000 - 0x00FFFFFF` | Kernel Memory (->   4 MiB)                 | Entry 771            |
| `0xC1000000 - 0xC13FFFFF` | `0x01000000 - 0x013FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 772            |
| `0xC1400000 - 0xC17FFFFF` | `0x01400000 - 0x017FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 773            |
| `0xC1800000 - 0xC1BFFFFF` | `0x01800000 - 0x01BFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 774            |
| `0xC1C00000 - 0xC1FFFFFF` | `0x01C00000 - 0x01FFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 775            |
| `0xC2000000 - 0xC23FFFFF` | `0x02000000 - 0x023FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 776            |
| `0xC2400000 - 0xC27FFFFF` | `0x02400000 - 0x027FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 777            |
| `0xC2800000 - 0xC2BFFFFF` | `0x02800000 - 0x02BFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 778            |
| `0xC2C00000 - 0xC2FFFFFF` | `0x02C00000 - 0x02FFFFFF` | Kernel Stack				                 | Entry 779            |
| `0xE0000000 - 0xE03FFFFF` | `0xFD000000 - 0xFD3FFFFF` | Framebuffer                                | Entry 896            |
| **Unmapped Addresses**    | -                         | Dynamic Allocation by PageFault Handler    | N/A                  |
|---------------------------|---------------------------|--------------------------------------------|----------------------|

################################################
## Stack Memory Layout for Higher-Half Kernel ##
################################################

| **Region**          | **Start Stack Address** | **End Stack Address**   | **Size**        | **Description**                               	|
|----------------------|------------------------|-------------------------|-----------------|-----------------------------------------------	|
| Kernel Stack         | `0xC2C00000`           | `0xC2C07FFF`            | 32 KiB          | TSS for Ring 3 -> Ring 0 and Kernel Tasks         |
| Reserved Stack Space | `0xC2C08000`           | `0xC2FFFFFF`            | 4064 KiB        | Reserved for Stack Expansion          			|

################################################
## Stack Memory Layout TSS                    ##
################################################

+-----------------------------+  0xC2C00000  ← KERNEL_STACK_BOTTOM
|                             |
|     Stack Bottom (ESP End)  |
|                             |
+-----------------------------+  0xC2C07FFF  ← KERNEL_STACK_TOP (TSS.esp0)
+-----------------------------+  0xC2C08000  ← RESERVED_STACK_START
|                             |
|     Reserved Stack Space    |
|                             |
|                             |
+-----------------------------+  0xC2FFFFFF  ← RESERVED_STACK_END