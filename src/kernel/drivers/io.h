#pragma once
#include <cstdint>

void outByte(uint16_t port, uint8_t value);
uint8_t inByte(uint16_t port);
void ioWait();
