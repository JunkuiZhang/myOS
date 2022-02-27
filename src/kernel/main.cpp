#include "./lib/utils/mem_size_print.h"
#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"
#include "drivers/io.h"
#include "drivers/io_manager.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/interrupts.h"
#include "memory/memory_utils.h"
#include "memory/page_bitmap.h"
#include "memory/pageframe_alloc.h"
#include "memory/pagemap_indexer.h"
#include "memory/pagetable_manager.h"

extern uint64_t __kernel_start;
extern uint64_t __kernel_end;

IDTR idtr;

extern "C" void _start(BootParamter *boot_param) {
	/* START: set gdt */
	GDTDescriptor gdt_desc;
	gdt_desc.size = sizeof(GDT) - 1;
	gdt_desc.offset = (uint64_t)&default_gdt;
	loadGDT(&gdt_desc);
	/* END: set gdt */

	const unsigned int background_color = 0xff002244;

	auto page_frame_manager =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_param->mem_map,
						   boot_param->mem_map_size, boot_param->mem_desc_size);

	auto kernel_size = (uint64_t)&__kernel_end - (uint64_t)&__kernel_start;
	auto kernel_pages = kernel_size / 4096 + 1;
	page_frame_manager.lockPages(&__kernel_start, kernel_pages);

	PageTable *pml4 = (PageTable *)page_frame_manager.requestPage();
	memset(pml4, 0, 4096);

	auto page_table_manager = PageTableManager(pml4);
	for (uint64_t x = 0; x < page_frame_manager.memory_size; x += 0x1000) {
		page_table_manager.mapMemory((void *)x, (void *)x, &page_frame_manager);
	}

	uint64_t framebuffer_base = (uint64_t)boot_param->framebuffer;
	uint64_t framebuffer_size = boot_param->framebuffer_size + 0x1000;
	for (uint64_t x = framebuffer_base; x < framebuffer_base + framebuffer_size;
		 x += 4096) {
		page_table_manager.mapMemory((void *)x, (void *)x, &page_frame_manager);
	}
	page_frame_manager.lockPages((void *)framebuffer_base,
								 framebuffer_size / 4096 + 1);

	asm("mov %0, %%cr3" : : "r"(pml4));

	auto screen_manager =
		ScreenManager((unsigned int *)framebuffer_base, boot_param->width,
					  boot_param->height, background_color);
	screen_manager.clearScreen(background_color);
	OS_SCREEN_MANAGER = &screen_manager;

	auto shell = Shell((unsigned int *)framebuffer_base, boot_param->width,
					   boot_param->height, 0xffffffff, 0xff002255);
	OS_SHELL = &shell;

	/* START: set idt */
	idtr.limit = 0x0fff;
	idtr.offset = (uint64_t)page_frame_manager.requestPage();

	IDTDescEntry *int_pagefault =
		(IDTDescEntry *)(idtr.offset + 0xe * sizeof(IDTDescEntry));
	int_pagefault->setOffset((uint64_t)pageFalutHandler);
	int_pagefault->type_attr = IDT_TA_InterruptGate;
	int_pagefault->selector = 0x08;

	IDTDescEntry *int_doublefault =
		(IDTDescEntry *)(idtr.offset + 0x8 * sizeof(IDTDescEntry));
	int_doublefault->setOffset((uint64_t)doubleFalutHandler);
	int_doublefault->type_attr = IDT_TA_InterruptGate;
	int_doublefault->selector = 0x08;

	IDTDescEntry *int_general_protection_fault =
		(IDTDescEntry *)(idtr.offset + 0xd * sizeof(IDTDescEntry));
	int_general_protection_fault->setOffset(
		(uint64_t)generalProtectionFalutHandler);
	int_general_protection_fault->type_attr = IDT_TA_InterruptGate;
	int_general_protection_fault->selector = 0x08;

	IDTDescEntry *int_keyboard =
		(IDTDescEntry *)(idtr.offset + 0x21 * sizeof(IDTDescEntry));
	int_keyboard->setOffset((uint64_t)keyboardIntHandler);
	int_keyboard->type_attr = IDT_TA_InterruptGate;
	int_keyboard->selector = 0x08;

	asm("lidt %0" : : "m"(idtr));
	/* END: set idt */

	remapPIC();
	outByte(PIC1_DATA, 0b11111101);
	outByte(PIC2_DATA, 0b11111111);
	asm("sti"); // enable mask interrupts, "cli" to cancel

	/* START: setup io manager */
	IOHandlerManager io_handler_manager = IOHandlerManager();
	OS_IO_Manager = &io_handler_manager;
	/* END: setup io manager */

	shell.println("New page map now!");
	shell.println("Hello from kernel!");
	shell.getShellInfo();
	shell.println("Frame size: %d, raw num: %d", boot_param->framebuffer_size,
				  boot_param->width * boot_param->height);
	// shell.deleteChar(0, 3);
	// shell.print("Used mem: %d KB\n",
	// 			page_frame_manager.getUsedMemorySize() / 1024);
	// auto free_mem =
	// memorySizeFormatter(page_frame_manager.getFreeMemorySize());
	// shell.print("Free mem: %d MB %d KB\n", free_mem.mega_bytes,
	// 			free_mem.kilo_bytes);
	// auto resv_mem =
	// 	memorySizeFormatter(page_frame_manager.getReservedMemorySize());
	// shell.print("Reserved mem: %d MB %d KB\n", resv_mem.mega_bytes,
	// 			resv_mem.kilo_bytes);

	/* spin */
	shell.display();
	while (1) {
		if (screen_manager.request_draw) {
			screen_manager.request_draw = false;
			screen_manager.clearScreen();
			shell.display();
		}
	}
}