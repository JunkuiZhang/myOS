#include "interrupts.h"
#include "../ints_handler/panic.h"

__attribute__((interrupt)) void
pageFalutHandler(struct interrupt_frame *frame) {
	panic("Page fault detected.\n");
	while (1)
		;
}

__attribute__((interrupt)) void
doubleFalutHandler(struct interrupt_frame *frame) {
	panic("Double fault detected.\n");
	while (1)
		;
}

__attribute__((interrupt)) void
generalProtectionFalutHandler(struct interrupt_frame *frame) {
	panic("General protection fault detected.\n");
	while (1)
		;
}
