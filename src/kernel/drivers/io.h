#pragma once
#include <cstdint>

void outByte(uint16_t port, uint8_t value);
uint8_t inByte(uint16_t port);
void writeU32(uint16_t port, uint32_t value);
uint32_t readU32(uint16_t port);
void ioWait();
