#include "pit.h"
#include "../../drivers/io.h"

namespace PIT {

volatile double seconds_since_boot = 0;
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

volatile void tick() {
	seconds_since_boot += 1 / (double)getFrequency();
	// milliseconds_since_boot += (1000 * divisor / base_frequency);
}

void sleep_sec(double second) {
	double end_time = seconds_since_boot + second;
	while (seconds_since_boot < end_time) {
		asm("hlt");
	}
}

void sleep(uint64_t milliseconds) {
	// uint64_t start_time = milliseconds_since_boot;
	// while (milliseconds_since_boot < start_time + milliseconds) {
	// 	asm("hlt");
	// }
	sleep_sec((double)milliseconds / 1000);
}

} // namespace PIT
