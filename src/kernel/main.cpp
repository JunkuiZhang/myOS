#include "./lib/utils/mem_size_print.h"
#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"
#include "gdt/gdt.h"
#include "memory/memory_utils.h"
#include "memory/page_bitmap.h"
#include "memory/pageframe_alloc.h"
#include "memory/pagemap_indexer.h"
#include "memory/pagetable_manager.h"

extern uint64_t __kernel_start;
extern uint64_t __kernel_end;

extern "C" void _start(BootParamter *boot_param) {
	GDTDescriptor gdt_desc;
	gdt_desc.size = sizeof(GDT) - 1;
	gdt_desc.offset = (uint64_t)&default_gdt;
	loadGDT(&gdt_desc);

	const unsigned int background_color = 0xff002244;

	auto os_page_frame_manager =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_param->mem_map,
						   boot_param->mem_map_size, boot_param->mem_desc_size);

	auto kernel_size = (uint64_t)&__kernel_end - (uint64_t)&__kernel_start;
	auto kernel_pages = kernel_size / 4096 + 1;
	os_page_frame_manager.lockPages(&__kernel_start, kernel_pages);

	PageTable *pml4 = (PageTable *)os_page_frame_manager.requestPage();
	memset(pml4, 0, 4096);

	auto os_page_table_manager = PageTableManager(pml4);
	for (uint64_t x = 0; x < os_page_frame_manager.memory_size; x += 0x1000) {
		os_page_table_manager.mapMemory((void *)x, (void *)x,
										&os_page_frame_manager);
	}

	uint64_t framebuffer_base = (uint64_t)boot_param->framebuffer;
	uint64_t framebuffer_size = boot_param->framebuffer_size + 0x1000;
	for (uint64_t x = framebuffer_base; x < framebuffer_base + framebuffer_size;
		 x += 4096) {
		os_page_table_manager.mapMemory((void *)x, (void *)x,
										&os_page_frame_manager);
	}
	os_page_frame_manager.lockPages((void *)framebuffer_base,
									framebuffer_size / 4096 + 1);

	asm("mov %0, %%cr3" : : "r"(pml4));

	auto os_screen_manager =
		ScreenManager((unsigned int *)framebuffer_base, boot_param->width,
					  boot_param->height);
	os_screen_manager.clearScreen(background_color);

	auto shell = Shell((unsigned int *)framebuffer_base, boot_param->width,
					   boot_param->height, 0xffffffff, 0xff002255);
	shell.print("New page map now!\n");
	shell.print("Hello from kernel!\n");
	shell.getShellInfo();
	shell.print("Frame size: %d, raw num: %d\n", boot_param->framebuffer_size,
				boot_param->width * boot_param->height);

	shell.print("Used mem: %d KB\n",
				os_page_frame_manager.getUsedMemorySize() / 1024);
	auto free_mem =
		memorySizeFormatter(os_page_frame_manager.getFreeMemorySize());
	shell.print("Free mem: %d MB %d KB\n", free_mem.mega_bytes,
				free_mem.kilo_bytes);
	auto resv_mem =
		memorySizeFormatter(os_page_frame_manager.getReservedMemorySize());
	shell.print("Reserved mem: %d MB %d KB\n", resv_mem.mega_bytes,
				resv_mem.kilo_bytes);

	for (uint64_t x = 0; x < 10; x++) {
		shell.print("ROW: %d\n", x);
	}

	/* spin */
	while (1)
		;
}