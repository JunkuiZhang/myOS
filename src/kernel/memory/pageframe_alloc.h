#pragma once

#include "efi_memory.h"
#include "page_bitmap.h"
#include <stdint.h>

class PageFrameAllocator {
  private:
	PageBitmap page_bitmap;
	uint64_t free_memory_size; /* in bytes */

  public:
	PageFrameAllocator(EfiMemoryDescriptor *memory_map, size_t map_size,
					   size_t desc_size);
	~PageFrameAllocator();
};
