#include "./video/screen.h"
#include "app/shell/shell.h"
#include "boot_param.h"

boot_param_t *boot_para;

extern "C" void _start(boot_param_t *m_boot_para) {
	boot_para = m_boot_para;
	const unsigned int background_color = 0xff002244;
	auto screen_manager = ScreenManager(boot_para->framebuffer,
										boot_para->width, boot_para->height);
	screen_manager.clearScreen(background_color);
	auto shell = Shell(boot_para->framebuffer, boot_para->width,
					   boot_para->height, 0xffffffff, 0xff002255);

	// shell.print("TEST\n");
	// shell.print("Hello from kernel!\n");
	// shell.print("A pointer can be a single char, as well as the beginning,
	// end " 			"or middle of an array... Think of chars as structs.You
	// " 			"sometimes allocate a single struct on the heap. That, too, " 			"creates
	// a pointer without an array.\n"); shell.print("Print: %d\n", 70000);
	// shell.print("Print: %d\n", 123);
	// shell.print("Print: %x\n", 123);
	// shell.print("Print: %s\n", "TEST");

	// row test
	for (int x = 0; x < 30; x++) {
		shell.print("Row: %d\n", x);
	}
	shell.getShellInfo();

	/* spin */
	while (1) {
		;
	}
}