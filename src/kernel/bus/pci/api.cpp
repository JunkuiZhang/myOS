#include "api.h"
#include "definitions.h"

namespace PCI {

void write32(void *addr, uint32_t reg, uint32_t value) {
	*(volatile uint32_t *)((uint8_t *)addr + reg) = value;
}

uint32_t read32(void *addr, uint32_t reg) {
	return *(volatile uint32_t *)((uint8_t *)addr + reg);
}

void write16(void *addr, uint32_t reg, uint16_t value) {
	*(volatile uint16_t *)((uint8_t *)addr + reg) = value;
}

uint16_t read16(void *addr, uint32_t reg) {
	return *(volatile uint16_t *)((uint8_t *)addr + reg);
}

void enableBusMastering(void *address) {
	auto value = read16(address, PCI_COMMAND);
	value |= (1 << 2);
	value |= (1 << 0);
	write16(address, PCI_COMMAND, value);
}

void enableInterruptLine(void *address) {
	write16(address, PCI::RegisterOffset::COMMAND,
			read16(address, PCI::RegisterOffset::COMMAND) & ~(1 << 10));
}

} // namespace PCI
