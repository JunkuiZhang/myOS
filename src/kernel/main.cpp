#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"
#include "memory/memory_manager.h"
#include "memory/page_bitmap.h"

boot_param_t *boot_para;
uint8_t test_bitmap[2];

extern "C" void _start(boot_param_t *m_boot_para) {
	boot_para = m_boot_para;

	const unsigned int background_color = 0xff002244;
	auto screen_manager = ScreenManager(boot_para->framebuffer,
										boot_para->width, boot_para->height);
	screen_manager.clearScreen(background_color);

	auto shell = Shell(boot_para->framebuffer, boot_para->width,
					   boot_para->height, 0xffffffff, 0xff002255);

	uint64_t memory_entries =
		boot_para->mem_map_size / boot_para->mem_desc_size;
	auto memory_manager =
		MemoryManager((EfiMemoryDescriptor *)boot_para->mem_map, memory_entries,
					  boot_para->mem_desc_size);

	shell.print("TEST\n");
	shell.print("Hello from kernel!\n");
	shell.print("A pointer can be a single char, as well as the beginning, end "
				"or middle of an array... Think of chars as structs.You "
				"sometimes allocate a single struct on the heap. That, too, "
				"creates a pointer without an array.\n");
	shell.print("Print: %d\n", 70000);
	shell.print("Print: %x\n", 123);
	shell.getShellInfo();

	shell.print("Total memory: %d MB\n", memory_manager.getMemorySize(2));

	PageBitmap page_bitmap = PageBitmap(10, test_bitmap);
	page_bitmap.setBitmapValue(0, true);
	page_bitmap.setBitmapValue(3, true);
	page_bitmap.setBitmapValue(7, true);

	for (size_t x = 0; x < 10; x++) {
		shell.print("Bitmap[%d]: %s\n", x, page_bitmap[x] ? "TRUE" : "FALSE");
	}

	/* spin */
	while (1)
		;
}