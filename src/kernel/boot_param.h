#pragma once
// #include <uefi.h>
// #include </Users/junkuizhang/CodingProjects/OS/myOS/src/gnu-efi/inc/efi.h>
#include "./memory/efi_memory.h"
#include "drivers/acpi.h"
#include <cstddef>
/**
 * Struct passed to the "kernel" from the exit_bs loader
 */
struct BootParamter {
	unsigned int *framebuffer;
	unsigned int width;
	unsigned int height;
	size_t framebuffer_size;
	void *mem_map;
	uint64_t mem_map_size;
	uint64_t mem_desc_size;
	ACPI::RSDP2 *rsdp;
};
