#pragma once

#include "efi_memory.h"
#include "page_bitmap.h"
#include <stdint.h>

class PageFrameAllocator {
  private:
	PageBitmap page_bitmap;
	uint64_t free_memory_size;	   /* in bytes */
	uint64_t used_memory_size;	   /* in bytes */
	uint64_t reserved_memory_size; /* in bytes */
	uint64_t bitmap_index_cache;

	void reservePage(void *address);
	void unreservePage(void *address);
	void reservePages(void *address, uint64_t page_count);
	void unreservePages(void *address, uint64_t page_count);

  public:
	uint64_t memory_size;

	PageFrameAllocator(EfiMemoryDescriptor *memory_map, size_t map_size,
					   size_t desc_size);
	~PageFrameAllocator();

	void lockPage(void *address);
	void freePage(void *address);
	void lockPages(void *address, uint64_t page_count);
	void freePages(void *address, uint64_t page_count);

	uint64_t getFreeMemorySize();
	uint64_t getUsedMemorySize();
	uint64_t getReservedMemorySize();

	void *requestPage();
};
