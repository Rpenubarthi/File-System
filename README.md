# File-System 

## Description  
This project is a custom **FUSE-based file system** that was implemented in C. It allows for users to mount a 1MB disk image as a functional file system. The file system supports basic file operations, directories, and large file storage.

## Features  
- **File Operations**  
  - Create, read, write, rename, and delete files  
  - Support for small files (≤ 4KB)  
  - Extendable to larger files (≥ 40KB)  
- **Directory Management**  
  - Create, list, rename, and delete directories  
  - Support for nested directories  
- **Efficient Storage Allocation**  
  - Implements a block-based system to manage disk space effectively  

## Installation  
Ensure you are using **Ubuntu (or another modern Linux system)** and install the required dependencies:  
```sh
sudo apt-get install libfuse-dev libbsd-dev pkg-config
```

## Compilation & Running
The following commands are required to run the file system:

1. Use the makefile to compile/recompile the code
```sh
make nufs
```

2. Run the file system using this format:
```sh
./nufs [FUSE_OPTIONS] mount_point disk_image
```


