#pragma once

/* https://wiki.osdev.org/Interrupts */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1
#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

struct interrupt_frame;
__attribute__((interrupt)) void pageFalutHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void
doubleFalutHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void
generalProtectionFalutHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void
keyboardIntHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void pitIntHandler(struct interrupt_frame *frame);

void remapPIC();
void endPICMaster();
void endPICSlave();
