# fs

* The ATA driver: Was successfully implemented and the data read from the ATA disk was stored in the ata_disk.buffer. This buffer contains the content retrieved from the ATA disk, which I could now access and use if needed. I have also implemented a way to display the buffer when the read operation is complete.

# vfs overview

The Virtual File System (VFS) is a crucial abstraction layer in icariusOS that allows seamless integration and utilization of various file systems. This documentation provides an overview of the VFS implementation and serves as a resource for developers looking to contribute to the file system support in the kernel.

The VFS (Virtual File System) handles key object types, such as:

- **Superblock:** Describes a file system.
- **V-node:** Describes a file.
- **Directory:** Describes a file system directory.

# vfs features

* File System Abstraction
    The VFS enables to work with different file systems without the need to rewrite the core of the operating system each time.
* Dynamic Driver Loading
    The VFS implementation allows file system drivers to be dynamically loaded at runtime, enhancing flexibility and extensibility.
* Generic Interface: 
    The generic interface of the VFS defines fundamental operations such as opening, closing, reading, and writing files, facilitating the integration of new file systems.

# usage

To utilize the VFS follow the instructions outlined in the developer documentation. Here are some basic steps:

* Resolve: Kernel polls each fs which will be loaded and ask if the disk can manage it. The disk itself binds itself to the fs implementation, if it can be handled.
For example if we attach a disk with a FAT16 header the fat16 fs should handle this. it iterates through all fs and check if a specific implementation can handle it.

# data flow: User Program <====> Kernel <====> File Systems

```bash
User program 
             <====> 
                    Kernel
                            <====> 
                                    FAT16
                                    NTFS
                                    FAT32

```

# data flow: fopen process

```bash
User program        <====>    Kernel    <===============>  
                              ^            Path Parser 
                              |
                             PathRootNode  <=====
                          drive 
                            = 'A'
                          PathNode *path 
                            = "hello.txt"
```

# file open (fopen) communication

```bash
    User program consumes the file descriptor from the kernel
        for example: VNodeDescriptor: 1
         |
         v
      Kernel returns its own vnode descriptor to the userland after FAT16 Filesystems fopen() call
         ^
         |
         v
    PathRootNode
         ^
         |
         v
      DISK 'A'
         ^
         |
   Access filesystem which is bound to the disk
         v
    FAT16 fopen() <====> FAT16    Calls FAT16 fopen function
         ^
         |
Returns an internal private file descriptor
```

# fread

```bash
    User program calls 

    VNodeDescriptor 1

        fread(buffer, 10, 1, file_descriptor);

   User program             <====>          Kernel 

    User program pass the buffer and VNode Descriptor to the kernel

        for example: FileDescriptor: 1
         |
         v
      Kernel looks into FileDescriptor it knows the filesystem through the filedescriptor
         ^
         |
   Access filesystem which is bound to the VNode Descriptor
         v
    FAT16 fread() <====> FAT16    Calls FAT16 fread function
         ^
         |
    read into the buffer which is passed from the userland
```

# todos
                  
* Add File System Drivers: Implement a file system driver that adheres to the generic interface of the VFS.
* Load Drivers: Dynamically load the file system driver at runtime using the provided functions.
* Invoke File System Operations: Utilize the generic interface of the VFS to invoke file system operations such as opening, reading, and writing files.

For a detailed understanding and contributions, please refer to the pparser.c, stream.c or ata.c file in the official icariusOS repository.