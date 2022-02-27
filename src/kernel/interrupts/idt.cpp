#include "idt.h"

void IDTDescEntry::setOffset(uint64_t offset) {
	offset0 = (uint16_t)(offset & 0xffff);
	offset1 = (uint16_t)((offset & 0xffff0000) >> 16);
	offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t IDTDescEntry::getOffset() {
	uint64_t offset = 0;
	offset |= (uint64_t)offset0;
	offset |= (uint64_t)offset1 << 16;
	offset |= (uint64_t)offset2 << 32;
	return offset;
}
