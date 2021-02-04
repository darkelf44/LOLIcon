// Include file header
#include <util/klib.h>

// Kernel includes
#include <psp2kern/kernel/sysmem.h> 

// Constants
#define HEAP_NAME "kheap"
#define HEAP_SIZE (16 * 1024)

// Global memory pool
static SceUID heap = 0;

void klib_init(void)
{
	// Create a new heap
	SceKernelHeapCreateOpt opt = {0};
	opt.size = sizeof(opt);
	opt.uselock = 1;
	heap = ksceKernelCreateHeap(HEAP_NAME, HEAP_SIZE, &opt);
}

void klib_done(void)
{
	// Delete the heap
	ksceKernelDeleteHeap(heap);
	heap = NULL;
}

void * kmalloc(size_t size)
{
	// Allocate memory
	return heap ? ksceKernelAllocHeapMemory(heap, size) : NULL;
}

void kfree(void * ptr)
{
	// Free memory
	heap ? ksceKernelFreeHeapMemory(heap, ptr) : 0;
}
