#pragma once
#include <cstdint>

namespace PCI {

#define PCI_COMMAND 0x04

uint16_t read16(void *addr, uint32_t reg);
uint32_t read32(void *addr, uint32_t reg);
void write16(void *addr, uint32_t reg, uint16_t value);
void write32(void *addr, uint32_t reg, uint32_t value);

void enableBusMastering(void *address);
void enableInterruptLine(void *address);

} // namespace PCI
