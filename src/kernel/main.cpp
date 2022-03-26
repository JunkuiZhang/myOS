#include "./lib/utils/mem_size_print.h"
#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"
#include "drivers/acpi.h"
#include "drivers/io.h"
#include "drivers/io_manager.h"
#include "drivers/pci.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/interrupts.h"
#include "memory/heap.h"
#include "memory/memory_utils.h"
#include "memory/page_bitmap.h"
#include "memory/pageframe_alloc.h"
#include "memory/pagemap_indexer.h"
#include "memory/pagetable_manager.h"
#include "scheduling/pit/pit.h"

extern uint64_t kernel_start;
extern uint64_t kernel_end;

IDTR idtr;

void setIDTGate(void *handler, uint8_t entry_offset, uint8_t type_attr,
				uint8_t selector) {
	auto *interrupt =
		(IDTDescEntry *)(idtr.offset + entry_offset * sizeof(IDTDescEntry));
	interrupt->setOffset((uint64_t)handler);
	interrupt->type_attr = type_attr;
	interrupt->selector = selector;
}

extern "C" void _start(BootParamter *boot_param) {
	/* START: data setup */
	const unsigned int background_color = 0xff002244;
	auto framebuffer_base = (uint64_t)boot_param->framebuffer;
	auto system_screen_manager =
		ScreenManager((unsigned int *)framebuffer_base, boot_param->width,
					  boot_param->height, background_color);
	system_screen_manager.clearScreen(background_color);
	OS_SCREEN_MANAGER = &system_screen_manager;
	auto system_shell =
		Shell((unsigned int *)(framebuffer_base), boot_param->width,
			  boot_param->height, 0xffffffff, 0xff002255);
	OS_SHELL = &system_shell;
	/* START: data setup */

	/* START: set gdt */
	GDTDescriptor gdt_desc;
	gdt_desc.size = sizeof(GDT) - 1;
	gdt_desc.offset = (uint64_t)&default_gdt;
	loadGDT(&gdt_desc);
	/* END: set gdt */

	/* START: memory setup */
	auto system_page_frame_manager =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_param->mem_map,
						   boot_param->mem_map_size, boot_param->mem_desc_size);
	OS_PAGEFRAME_ALLOCATOR = &system_page_frame_manager;
	auto kernel_size = (uint64_t)&kernel_end - (uint64_t)&kernel_start;
	auto kernel_pages = kernel_size / 4096 + 1;

	system_page_frame_manager.lockPages(&kernel_start, kernel_pages);

	auto *pml4 = (PageTable *)system_page_frame_manager.requestPage();
	memset(pml4, 0, 4096);

	auto system_page_table_manager = PageTableManager(pml4);
	OS_PAGETABLE_MANAGER = &system_page_table_manager;
	for (uint64_t x = 0; x < system_page_frame_manager.memory_size;
		 x += 0x1000) {
		system_page_table_manager.mapMemory((void *)x, (void *)x);
	}

	uint64_t framebuffer_size = boot_param->framebuffer_size + 0x1000;
	system_page_frame_manager.lockPages((void *)framebuffer_base,
										framebuffer_size / 4096 + 1);
	for (uint64_t x = framebuffer_base; x < framebuffer_base + framebuffer_size;
		 x += 4096) {
		system_page_table_manager.mapMemory((void *)x, (void *)x);
	}

	asm("mov %0, %%cr3" : : "r"(pml4));
	/* END: memory setup */

	/* START: heap setup */
	initializeHeap((void *)0x0000100000000000, 0x10);
	/* END: heap setup */

	/* START: interrupts setup */
	idtr.limit = 0x0fff;
	idtr.offset = (uint64_t)system_page_frame_manager.requestPage();

	setIDTGate((void *)pageFalutHandler, 0xe, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)doubleFalutHandler, 0x8, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)generalProtectionFalutHandler, 0xd, IDT_TA_InterruptGate,
			   0x08);
	setIDTGate((void *)keyboardIntHandler, 0x21, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)pitIntHandler, 0x20, IDT_TA_InterruptGate, 0x08);

	asm("lidt %0" : : "m"(idtr));
	remapPIC();
	/* END: interrupts setup */

	/* START: ACPI setup */
	auto *xsdt = (ACPI::SDTHeader *)(boot_param->rsdp->XSDT_address);
	auto *mcfg = (ACPI::MCFGHeader *)ACPI::findTable(xsdt, "MCFG");
	PCI::enumeratePCI(mcfg);
	/* END: ACPI setup */

	outByte(PIC1_DATA, 0b11111100);
	outByte(PIC2_DATA, 0b11111111);
	asm("sti"); // enable mask interrupts, "cli" to cancel

	/* START: setup io manager */
	IOHandlerManager io_handler_manager = IOHandlerManager();
	OS_IO_MANAGER = &io_handler_manager;
	/* END: setup io manager */

	/* START: PIT setup */
	PIT::setFrequency(600);
	/* END: PIT setup */

	system_shell.println("New page map now!");
	system_shell.println("Hello from kernel!");
	system_shell.getShellInfo();
	system_shell.println("FB base: %x", framebuffer_base);

	for (uint64_t x = 0; x < 20; x++) {
		system_shell.println("NUM: %d", x);
		PIT::sleep_sec(1);
	}

	/* spin */
	while (true) {
		asm("hlt");
	}
}