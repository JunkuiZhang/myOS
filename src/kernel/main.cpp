#include "app/shell/shell.h"
#include "boot_param.h"

boot_param_t *boot_para;

extern "C" void _start(boot_param_t *m_boot_para) {
	boot_para = m_boot_para;
	auto shell =
		Shell(boot_para->framebuffer, boot_para->width, boot_para->height);
	int i;
	for (i = 0; i < boot_para->height * boot_para->width; i++) {
		boot_para->framebuffer[i] = 0xff002255;
	}

	shell.print("TEST\n");
	shell.print("Hello from kernel!\n");
	// shell.print("Hello again!\n");
	shell.print("A pointer can be a single char, as well as the beginning, end "
				"or middle of an array...Think of chars as structs. You "
				"sometimes allocate a single struct on the heap. That, too, "
				"creates a pointer without an array.\n");
	shell.print("Print: %d\n", 70000);
	shell.print("Print: %d\n", 123);
	shell.print("Print: %x\n", 123);
	shell.print("Print: %s\n", "TEST");
	shell.getShellInfo();

	/* spin */
	while (1) {
		;
	}
}