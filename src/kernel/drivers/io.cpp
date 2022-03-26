#include "io.h"
#include <cstdint>

void outByte(uint16_t port, uint8_t value) {
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inByte(uint16_t port) {
	uint8_t return_value;
	asm volatile("inb %1, %0" : "=a"(return_value) : "Nd"(port));
	return return_value;
}

void writeU32(uint16_t port, uint32_t value) {
	asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t readU32(uint16_t port) {
	uint32_t return_value;
	asm volatile("inl %1, %0" : "=a"(return_value) : "Nd"(port));
	return return_value;
}

void ioWait() { asm volatile("outb %%al, $0x80" : : "a"(0)); }
