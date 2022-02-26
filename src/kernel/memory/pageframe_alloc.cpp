#include "pageframe_alloc.h"

PageFrameAllocator::PageFrameAllocator(EfiMemoryDescriptor *memory_map,
									   size_t map_size, size_t desc_size) {
	uint64_t map_entries = map_size / desc_size;
	void *largest_free_memory_segment = NULL;
	size_t largest_free_memory_segment_size = 0;
	uint64_t memory_map_size_bytes = 0;

	for (uint64_t x = 0; x < map_entries; x++) {
		EfiMemoryDescriptor *desc =
			(EfiMemoryDescriptor *)((uint64_t)memory_map + x * desc_size);
		uint64_t this_segment_size = desc->NumberOfPages * 4096;
		memory_map_size_bytes += this_segment_size;
		if (desc->Type == 7) {
			// type = EFI conventional mem
			if (this_segment_size > largest_free_memory_segment_size) {
				largest_free_memory_segment = (void *)desc->PhysicalStart;
				largest_free_memory_segment_size = this_segment_size;
			}
		}
	}

	free_memory_size = memory_map_size_bytes;
	uint64_t bitmap_size = memory_map_size_bytes / 4096 / 8 + 1;

	/* init bitmap */
	page_bitmap =
		PageBitmap(bitmap_size, (uint8_t *)largest_free_memory_segment);
}

PageFrameAllocator::~PageFrameAllocator() {}
