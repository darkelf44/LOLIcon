/* klib.h - stdlib.h replacement for TaiHen kernel modules */
#pragma once

// Includes
#include <stddef.h>
#include <stdbool.h>
#include <psp2/types.h>

// INLINE macro - Forces the function to be inlined
#undef INLINE
#define INLINE __attribute__((always_inline)) __inline__

// Initialize and finalize memory system
void klib_init(void);
void klib_done(void);

// Dynamic memory from the kernel
void * kmalloc(size_t size);
void kfree(void * ptr);

// Lightweight mutex
typedef volatile int KMutex;

// Lightweight mutex functions
INLINE bool kmutex_try_lock(KMutex * mutex)
{
	return 0 == __atomic_exchange_n(mutex, 1, __ATOMIC_ACQUIRE);
}
INLINE void kmutex_unlock(KMutex * mutex)
{
	__atomic_store_n(mutex, 0, __ATOMIC_RELEASE);
}

