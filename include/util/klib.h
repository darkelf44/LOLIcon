/* klib.h - stdlib.h replacement for TaiHen kernel modules */
#pragma once

// Includes
#include <psp2/types.h>

// Initialize and finalize memory system
void klib_init(void);
void klib_done(void);

// Dynamic memory from the kernel
void * kmalloc(size_t size);
void kfree(void * ptr);

