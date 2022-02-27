#pragma once

struct interrupt_frame;
__attribute__((interrupt)) void pageFalutHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void
doubleFalutHandler(struct interrupt_frame *frame);
__attribute__((interrupt)) void
generalProtectionFalutHandler(struct interrupt_frame *frame);
