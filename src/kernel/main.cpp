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
#include "memory/memory_utils.h"
#include "memory/page_bitmap.h"
#include "memory/pageframe_alloc.h"
#include "memory/pagemap_indexer.h"
#include "memory/pagetable_manager.h"
#include "scheduling/pit/pit.h"

extern uint64_t __kernel_start;
extern uint64_t __kernel_end;

IDTR idtr;

void setIDTGate(void *handler, uint8_t entry_offset, uint8_t type_attr,
				uint8_t selector) {
	IDTDescEntry *interrupt =
		(IDTDescEntry *)(idtr.offset + entry_offset * sizeof(IDTDescEntry));
	interrupt->setOffset((uint64_t)handler);
	interrupt->type_attr = type_attr;
	interrupt->selector = selector;
}

extern "C" void _start(BootParamter *boot_param) {
	/* START: data setup */
	const unsigned int background_color = 0xff002244;
	uint64_t framebuffer_base = (uint64_t)boot_param->framebuffer;
	auto screen_manager =
		ScreenManager((unsigned int *)framebuffer_base, boot_param->width,
					  boot_param->height, background_color);
	screen_manager.clearScreen(background_color);
	OS_SCREEN_MANAGER = &screen_manager;
	auto shell = Shell((unsigned int *)(framebuffer_base), boot_param->width,
					   boot_param->height, 0xffffffff, 0xff002255);
	OS_SHELL = &shell;
	/* START: data setup */

	/* START: set gdt */
	GDTDescriptor gdt_desc;
	gdt_desc.size = sizeof(GDT) - 1;
	gdt_desc.offset = (uint64_t)&default_gdt;
	loadGDT(&gdt_desc);
	/* END: set gdt */

	/* START: memory setup */
	auto page_frame_manager =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_param->mem_map,
						   boot_param->mem_map_size, boot_param->mem_desc_size);
	OS_PAGEFRAME_ALLOCATOR = &page_frame_manager;
	auto kernel_size = (uint64_t)&__kernel_end - (uint64_t)&__kernel_start;
	auto kernel_pages = kernel_size / 4096 + 1;

	page_frame_manager.lockPages(&__kernel_start, kernel_pages);

	PageTable *pml4 = (PageTable *)page_frame_manager.requestPage();
	memset(pml4, 0, 4096);

	auto page_table_manager = PageTableManager(pml4);
	OS_PAGETABLE_MANAGER = &page_table_manager;
	for (uint64_t x = 0; x < page_frame_manager.memory_size; x += 0x1000) {
		page_table_manager.mapMemory((void *)x, (void *)x);
	}

	uint64_t framebuffer_size = boot_param->framebuffer_size + 0x1000;
	page_frame_manager.lockPages((void *)framebuffer_base,
								 framebuffer_size / 4096 + 1);
	for (uint64_t x = framebuffer_base; x < framebuffer_base + framebuffer_size;
		 x += 4096) {
		page_table_manager.mapMemory((void *)x, (void *)x);
	}

	asm("mov %0, %%cr3" : : "r"(pml4));
	/* END: memory setup */

	/* START: heap setup */
	/* END: heap setup */

	/* START: interrupts setup */
	idtr.limit = 0x0fff;
	idtr.offset = (uint64_t)page_frame_manager.requestPage();

	setIDTGate((void *)pageFalutHandler, 0xe, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)doubleFalutHandler, 0x8, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)generalProtectionFalutHandler, 0xd, IDT_TA_InterruptGate,
			   0x08);
	setIDTGate((void *)keyboardIntHandler, 0x21, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)pitIntHandler, 0x20, IDT_TA_InterruptGate, 0x08);

	asm("lidt %0" : : "m"(idtr));
	remapPIC();
	/* END: interrupts setup */

	outByte(PIC1_DATA, 0b11111100);
	outByte(PIC2_DATA, 0b11111111);
	asm("sti"); // enable mask interrupts, "cli" to cancel

	/* START: setup io manager */
	IOHandlerManager io_handler_manager = IOHandlerManager();
	OS_IO_Manager = &io_handler_manager;
	/* END: setup io manager */

	/* START: PIT setup */
	PIT::setDivisor(2000);
	/* END: PIT setup */

	/* START: ACPI setup */
	ACPI::SDTHeader *xsdt = (ACPI::SDTHeader *)(boot_param->rsdp->XSDT_address);
	shell.println("ADDRESS: %d", (uint64_t)xsdt);
	ACPI::MCFGHeader *mcfg =
		(ACPI::MCFGHeader *)ACPI::findTable(xsdt, (char *)"MCFG");
	PCI::enumeratePCI(mcfg);
	// shell.println("MCFG: %d", (uint64_t)mcfg);
	/* END: ACPI setup */

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
	for (uint64_t x = 0; x < 20; x++) {
		shell.println("NUM: %d", x);
		PIT::sleep_sec(10);
	}

	/* spin */
	while (1) {
		// if (screen_manager.request_draw) {
		// 	screen_manager.request_draw = false;
		// 	screen_manager.clearScreen();
		// 	shell.display();
		// }
		;
	}
}