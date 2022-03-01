#include "pit.h"
#include "../../drivers/io.h"

namespace PIT {

volatile uint64_t ms_since_boot = 0;
// volatile uint64_t milliseconds_since_boot = 0;
uint16_t divisor = 65535;

void setDivisor(uint16_t m_divisor) {
	if (m_divisor < 100)
		m_divisor = 100;
	divisor = m_divisor;
	outByte(0x40, (uint8_t)(m_divisor & 0x00ff));
	ioWait();
	outByte(0x40, (uint8_t)(m_divisor >> 8));
}

uint64_t getFrequency() { return base_frequency / divisor; }

void setFrequency(uint64_t frequency) {
	setDivisor(base_frequency / frequency);
}

volatile void tick() { ms_since_boot += (1000 * divisor / base_frequency); }

void sleep_sec(double second) { sleep((uint64_t)(second * 1000)); }

void sleep(uint64_t milliseconds) {
	uint64_t end_time = ms_since_boot + milliseconds;
	while (ms_since_boot < end_time) {
		asm("hlt");
	}
}

} // namespace PIT
