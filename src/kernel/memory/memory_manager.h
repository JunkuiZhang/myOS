#pragma once

#include "efi_memory.h"
#include <stdint.h>

class MemoryManager {
  private:
	uint64_t memory_size_bytes;

  public:
	MemoryManager(EfiMemoryDescriptor *memory_map, uint64_t map_entries,
				  uint64_t desc_size);
	~MemoryManager();

	uint64_t getMemorySize(uint32_t mode);
};
