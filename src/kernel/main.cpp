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

extern uint64_t KernelStart;
extern uint64_t KernelEnd;

IDTR Idtr;

void setIDTGate(void *handler, uint8_t entry_offset, uint8_t type_attr,
				uint8_t selector) {
	auto *Interrupt =
		(IDTDescEntry *)(Idtr.offset + entry_offset * sizeof(IDTDescEntry));
	Interrupt->setOffset((uint64_t)handler);
	Interrupt->type_attr = type_attr;
	Interrupt->selector = selector;
}

extern "C" void _start(BootParamter *boot_param) {
	/* START: data setup */
	const unsigned int BackgroundColor = 0xff002244;
	auto FramebufferBase = (uint64_t)boot_param->framebuffer;
	auto SystemScreenManager =
		ScreenManager((unsigned int *)FramebufferBase, boot_param->width,
					  boot_param->height, BackgroundColor);
	SystemScreenManager.clearScreen(BackgroundColor);
	OS_SCREEN_MANAGER = &SystemScreenManager;
	auto SystemShell =
		Shell((unsigned int *)(FramebufferBase), boot_param->width,
			  boot_param->height, 0xffffffff, 0xff002255);
	OS_SHELL = &SystemShell;
	/* START: data setup */

	/* START: set gdt */
	GDTDescriptor GdtDesc;
	GdtDesc.size = sizeof(GDT) - 1;
	GdtDesc.offset = (uint64_t)&default_gdt;
	loadGDT(&GdtDesc);
	/* END: set gdt */

	/* START: memory setup */
	auto SystemPageFrameManager =
		PageFrameAllocator((EfiMemoryDescriptor *)boot_param->mem_map,
						   boot_param->mem_map_size, boot_param->mem_desc_size);
	OS_PAGEFRAME_ALLOCATOR = &SystemPageFrameManager;
	auto KernelSize = (uint64_t)&KernelEnd - (uint64_t)&KernelStart;
	auto KernelPages = KernelSize / 4096 + 1;

	SystemPageFrameManager.lockPages(&KernelStart, KernelPages);

	auto *Pml4 = (PageTable *)SystemPageFrameManager.requestPage();
	memset(Pml4, 0, 4096);

	auto SystemPageTableManager = PageTableManager(Pml4);
	OS_PAGETABLE_MANAGER = &SystemPageTableManager;
	for (uint64_t x = 0; x < SystemPageFrameManager.memory_size; x += 0x1000) {
		SystemPageTableManager.mapMemory((void *)x, (void *)x);
	}

	uint64_t FramebufferSize = boot_param->framebuffer_size + 0x1000;
	SystemPageFrameManager.lockPages((void *)FramebufferBase,
									 FramebufferSize / 4096 + 1);
	for (uint64_t x = FramebufferBase; x < FramebufferBase + FramebufferSize;
		 x += 4096) {
		SystemPageTableManager.mapMemory((void *)x, (void *)x);
	}

	asm("mov %0, %%cr3" : : "r"(Pml4));
	/* END: memory setup */

	/* START: heap setup */
	initializeHeap((void *)0x0000100000000000, 0x10);
	/* END: heap setup */

	/* START: interrupts setup */
	Idtr.limit = 0x0fff;
	Idtr.offset = (uint64_t)SystemPageFrameManager.requestPage();

	setIDTGate((void *)pageFalutHandler, 0xe, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)doubleFalutHandler, 0x8, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)generalProtectionFalutHandler, 0xd, IDT_TA_InterruptGate,
			   0x08);
	setIDTGate((void *)keyboardIntHandler, 0x21, IDT_TA_InterruptGate, 0x08);
	setIDTGate((void *)pitIntHandler, 0x20, IDT_TA_InterruptGate, 0x08);

	asm("lidt %0" : : "m"(Idtr));
	remapPIC();
	/* END: interrupts setup */

	/* START: ACPI setup */
	auto *Xsdt = (ACPI::SDTHeader *)(boot_param->rsdp->XSDT_address);
	auto *Mcfg = (ACPI::MCFGHeader *)ACPI::findTable(Xsdt, "MCFG");
	PCI::enumeratePCI(Mcfg);
	/* END: ACPI setup */

	outByte(PIC1_DATA, 0b11111100);
	outByte(PIC2_DATA, 0b11111111);
	asm("sti"); // enable mask interrupts, "cli" to cancel

	/* START: setup io manager */
	IOHandlerManager IoHandlerManager = IOHandlerManager();
	OS_IO_Manager = &IoHandlerManager;
	/* END: setup io manager */

	/* START: PIT setup */
	PIT::setFrequency(600);
	/* END: PIT setup */

	SystemShell.println("New page map now!");
	SystemShell.println("Hello from kernel!");
	SystemShell.getShellInfo();
	SystemShell.println("Frame size: %d, raw num: %d",
						boot_param->framebuffer_size,
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
	SystemShell.println("Heap: %x", (uint64_t)malloc(0x8000));
	void *Temp = malloc(0x8000);
	SystemShell.println("Heap: %x", (uint64_t)malloc(0x8000));
	SystemShell.println("Heap: %x", (uint64_t)malloc(0x100));
	free(Temp);
	SystemShell.println("Heap: %x", (uint64_t)malloc(0x100));
	for (uint64_t x = 0; x < 20; x++) {
		SystemShell.println("NUM: %d", x);
		PIT::sleep_sec(1);
	}

	/* spin */
	while (true) {
		asm("hlt");
	}
}