#include "memory_manager.h"

MemoryManager::MemoryManager(EfiMemoryDescriptor *memory_map,
							 uint64_t map_entries, uint64_t desc_size) {
	memory_size_bytes = 0;
	for (uint64_t x = 0; x < map_entries; x++) {
		EfiMemoryDescriptor *desc =
			(EfiMemoryDescriptor *)((uint64_t)memory_map + x * desc_size);
		memory_size_bytes += desc->NumberOfPages * 4096;
	}
}

uint64_t MemoryManager::getMemorySize(uint32_t mode) {
	uint64_t res = memory_size_bytes;
	switch (mode) {
	case 1:
		/* in kilobytes */
		res /= 1024;
		break;

	case 2:
		/* in megabytes */
		res /= (1024 * 1024);
		break;

	case 3:
		/* in gigabytes */
		res /= (1024 * 1024 * 1024);
		break;

	default:
		break;
	}

	return res;
}

MemoryManager::~MemoryManager() {}