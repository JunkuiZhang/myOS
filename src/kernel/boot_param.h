#pragma once
#include <uefi.h>
/**
 * Struct passed to the "kernel" from the exit_bs loader
 */
typedef struct {
	unsigned int *framebuffer;
	unsigned int width;
	unsigned int height;
	efi_memory_descriptor_t *mem_map;
	uintn_t mem_map_size;
	uintn_t mem_desc_size;
} boot_param_t;
