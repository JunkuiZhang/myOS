#include "./lib/utils/mem_size_print.h"
#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"
#include "memory/memory_utils.h"
#include "memory/page_bitmap.h"
#include "memory/pageframe_alloc.h"
#include "memory/pagemap_indexer.h"
#include "memory/pagetable_manager.h"

boot_param_t *boot_para;
extern uint64_t __kernel_start;
extern uint64_t __kernel_end;

extern "C" void _start(boot_param_t *m_boot_para) {
	boot_para = m_boot_para;
	const unsigned int background_color = 0xff002244;

	auto OS_PAGEFRAME_MANAGER =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_para->mem_map,
						   boot_para->mem_map_size, boot_para->mem_desc_size);

	auto kernel_size = (uint64_t)&__kernel_end - (uint64_t)&__kernel_start;
	auto kernel_pages = kernel_size / 4096 + 1;
	OS_PAGEFRAME_MANAGER.lockPages(&__kernel_start, kernel_pages);

	PageTable *pml4 = (PageTable *)OS_PAGEFRAME_MANAGER.requestPage();
	memset(pml4, 0, 4096);

	PageTableManager OS_PAGETABLE_MANAGER = PageTableManager(pml4);
	for (uint64_t x = 0; x < OS_PAGEFRAME_MANAGER.memory_size; x += 0x1000) {
		OS_PAGETABLE_MANAGER.mapMemory((void *)x, (void *)x,
									   &OS_PAGEFRAME_MANAGER);
	}

	uint64_t framebuffer_base = (uint64_t)boot_para->framebuffer;
	uint64_t framebuffer_size = boot_para->framebuffer_size + 0x1000;
	for (uint64_t x = framebuffer_base; x < framebuffer_base + framebuffer_size;
		 x += 4096) {
		OS_PAGETABLE_MANAGER.mapMemory((void *)x, (void *)x,
									   &OS_PAGEFRAME_MANAGER);
	}

	asm("mov %0, %%cr3" : : "r"(pml4));

	auto OS_SCREEN_MANAGER = ScreenManager((unsigned int *)framebuffer_base,
										   boot_para->width, boot_para->height);
	OS_SCREEN_MANAGER.clearScreen(background_color);

	// for (uint64_t x = 0; x < boot_para->width * boot_para->height; x++) {
	// 	((unsigned int *)(framebuffer_base))[x] = 0xff0000ff;
	// }

	auto shell = Shell((unsigned int *)framebuffer_base, boot_para->width,
					   boot_para->height, 0xffffffff, 0xff002255);
	shell.print("New page map now!\n");

	shell.print("Kernel size: %d with %d pages.\n", kernel_size, kernel_pages);

	shell.print("Hello from kernel!\n");
	shell.getShellInfo();
	shell.print("Frame size: %d, raw num: %d\n", framebuffer_size,
				boot_para->width * boot_para->height);

	shell.print("Used mem: %d KB\n",
				OS_PAGEFRAME_MANAGER.getUsedMemorySize() / 1024);
	auto free_mem =
		memorySizeFormatter(OS_PAGEFRAME_MANAGER.getFreeMemorySize());
	shell.print("Free mem: %d MB %d KB\n", free_mem.mega_bytes,
				free_mem.kilo_bytes);
	auto resv_mem =
		memorySizeFormatter(OS_PAGEFRAME_MANAGER.getReservedMemorySize());
	shell.print("Reserved mem: %d MB %d KB\n", resv_mem.mega_bytes,
				resv_mem.kilo_bytes);

	for (uint64_t x = 0; x < 10; x++) {
		shell.print("ROW: %d\n", x);
	}

	/* spin */
	while (1)
		;
}