#include "pageframe_alloc.h"

PageFrameAllocator::PageFrameAllocator(EfiMemoryDescriptor *memory_map,
									   size_t map_size, size_t desc_size) {
	uint64_t map_entries = map_size / desc_size;
	void *largest_free_memory_segment = NULL;
	size_t largest_free_memory_segment_size = 0;
	uint64_t memory_map_size_bytes = 0;
	used_memory_size = 0;
	free_memory_size = 0;
	reserved_memory_size = 0;

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
	memory_size = memory_map_size_bytes;
	uint64_t bitmap_size = memory_map_size_bytes / 4096 / 8 + 1;

	/* init bitmap */
	page_bitmap.init(bitmap_size, (uint8_t *)largest_free_memory_segment);
	/* lock the bitmap pages */
	lockPages(&page_bitmap, bitmap_size / 4096 + 1);
	/* reserve the unusable && reserved memory */
	for (uint64_t x = 0; x < map_entries; x++) {
		EfiMemoryDescriptor *desc =
			(EfiMemoryDescriptor *)((uint64_t)memory_map + x * desc_size);
		if (desc->Type != 7) {
			reservePages((void *)desc->PhysicalStart, desc->NumberOfPages);
		}
	}
}

void PageFrameAllocator::freePage(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if (!page_bitmap[index])
		return;
	page_bitmap.setBitmapValue(index, false);
	free_memory_size += 4096;
	used_memory_size -= 4096;
}

void PageFrameAllocator::lockPage(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index])
		return;
	page_bitmap.setBitmapValue(index, true);
	free_memory_size -= 4096;
	used_memory_size += 4096;
}

void PageFrameAllocator::unreservePage(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if (!page_bitmap[index])
		return;
	page_bitmap.setBitmapValue(index, false);
	free_memory_size += 4096;
	reserved_memory_size -= 4096;
}

void PageFrameAllocator::reservePage(void *address) {
	uint64_t index = (uint64_t)address / 4096;
	if (page_bitmap[index])
		return;
	page_bitmap.setBitmapValue(index, true);
	free_memory_size -= 4096;
	reserved_memory_size += 4096;
}

void PageFrameAllocator::freePages(void *address, uint64_t page_count) {
	for (uint64_t x = 0; x < page_count; x++) {
		freePage((void *)((uint64_t)address + x * 4096));
	}
}

void PageFrameAllocator::lockPages(void *address, uint64_t page_count) {
	for (uint64_t x = 0; x < page_count; x++) {
		lockPage((void *)((uint64_t)address + x * 4096));
	}
}

void PageFrameAllocator::unreservePages(void *address, uint64_t page_count) {
	for (uint64_t x = 0; x < page_count; x++) {
		unreservePage((void *)((uint64_t)address + x * 4096));
	}
}

void PageFrameAllocator::reservePages(void *address, uint64_t page_count) {
	for (uint64_t x = 0; x < page_count; x++) {
		reservePage((void *)((uint64_t)address + x * 4096));
	}
}

uint64_t PageFrameAllocator::getFreeMemorySize() { return free_memory_size; }

uint64_t PageFrameAllocator::getUsedMemorySize() { return used_memory_size; }

uint64_t PageFrameAllocator::getReservedMemorySize() {
	return reserved_memory_size;
}

void *PageFrameAllocator::requestPage() {
	for (uint64_t x = 0; x < page_bitmap.getSize(); x++) {
		if (page_bitmap[x])
			continue;
		uint64_t addres = x * 4096;
		lockPage((void *)addres);
		return (void *)addres;
	}
	// TODO: page swap
	return NULL;
}

PageFrameAllocator::~PageFrameAllocator() {}
