# Buddy Allocator Memory Management

## Introduction
This C language implementation provides a buddy system for memory management, designed to efficiently allocate and free memory blocks of varying sizes. The buddy system uses a binary tree structure to manage free and occupied memory segments, enabling rapid memory operations.

## System Components
- **buddy.h**: Header file for the buddy memory management system.
- **buddy.c**: Implementation file containing the buddy system's functionality.

## Key Features
- **Alignment Support**: The `align` parameter in the `buddy_new` function ensures that the memory blocks are aligned power of two, which is crucial for many systems where specific memory alignment is required for performance optimization.
- **Size Shift Storage**: Instead of storing the actual sizes of memory blocks, the system stores the shift value (power of two). This method significantly reduces the space needed to store size information and simplifies the computation during allocation and deallocation.

## Core Functions
- `buddy_new()`: Initializes a new buddy system.
- `buddy_alloc()`: Allocates a memory block of a specified size.
- `buddy_free()`: Frees a previously allocated memory block.
- `buddy_dump()`: Displays the current status of the memory blocks, useful for debugging.
