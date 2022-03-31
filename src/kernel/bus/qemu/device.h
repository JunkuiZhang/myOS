#pragma once
#include <cstdint>

namespace QEMU {

class Device {
  public:
	Device(void *addr);
	~Device() = default;

  private:
	void *device_addr;

	void enableInterrupt();
	void setStatusBit(uint8_t status_bit);
	void setupQueues();
	void configWrite16(uint32_t offset, uint16_t value);
};

} // namespace QEMU
