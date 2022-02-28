#include "interrupts.h"
#include "../app/shell/shell.h"
#include "../drivers/io.h"
#include "../drivers/io_manager.h"
#include "../drivers/keyboard.h"
#include "../ints_handler/panic.h"
#include "../scheduling/pit/pit.h"
#include <stdint.h>

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

__attribute__((interrupt)) void
keyboardIntHandler(struct interrupt_frame *frame) {
	uint8_t scancode = inByte(0x60);
	OS_IO_Manager->handleKeyboard(scancode);
	endPICMaster();
}

__attribute__((interrupt)) void pitIntHandler(struct interrupt_frame *frame) {
	PIT::tick();
	endPICMaster();
}

void endPICMaster() { outByte(PIC1_COMMAND, PIC_EOI); }

void endPICSlave() {
	outByte(PIC2_COMMAND, PIC_EOI);
	outByte(PIC1_COMMAND, PIC_EOI);
}

void remapPIC() {
	uint8_t a1, a2;
	a1 = inByte(PIC1_DATA);
	ioWait();
	a2 = inByte(PIC2_DATA);
	ioWait();

	/* init master chip */
	outByte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	ioWait();
	/* init slave chip */
	outByte(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	ioWait();

	outByte(PIC1_DATA, 0x20);
	ioWait();
	outByte(PIC2_DATA, 0x28);
	ioWait();

	outByte(PIC1_DATA, 4);
	ioWait();
	outByte(PIC2_DATA, 2);
	ioWait();

	outByte(PIC1_DATA, ICW4_8086);
	ioWait();
	outByte(PIC2_DATA, ICW4_8086);
	ioWait();

	outByte(PIC1_DATA, a1);
	ioWait();
	outByte(PIC2_DATA, a2);
}
