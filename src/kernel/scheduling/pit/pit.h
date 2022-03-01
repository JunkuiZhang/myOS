#pragma once
#include <stdint.h>

/* Programable Interval Timer */
namespace PIT {

// extern volatile uint64_t milliseconds_since_boot;
extern volatile uint64_t ms_since_boot;
const uint64_t base_frequency = 1193182;

void sleep_sec(double second);
void sleep(uint64_t milliseconds);
void setDivisor(uint16_t divisor);
uint64_t getFrequency();
void setFrequency(uint64_t frequency);
volatile void tick();

} // namespace PIT
