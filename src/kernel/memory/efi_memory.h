#pragma once
#include <stdint.h>

struct EfiMemoryDescriptor {
	uint32_t Type;			// Field size is 32 bits followed by 32 bit pad
	uint32_t Pad;			// For alignment
	uint64_t PhysicalStart; // Field size is 64 bits
	uint64_t *VirtualStart; // Field size is 64 bits
	uint64_t NumberOfPages; // Field size is 64 bits
	uint64_t Attribute;		// Field size is 64 bits
};

extern const char *EFI_MEMORY_TYPE_STRINGS[];
