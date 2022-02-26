#pragma once
// #include <uefi.h>
// #include </Users/junkuizhang/CodingProjects/OS/myOS/src/gnu-efi/inc/efi.h>
#include "./memory/efi_memory.h"
/**
 * Struct passed to the "kernel" from the exit_bs loader
 */
typedef struct {
	unsigned int *framebuffer;
	unsigned int width;
	unsigned int height;
	void *mem_map;
	uint64_t mem_map_size;
	uint64_t mem_desc_size;
} boot_param_t;
