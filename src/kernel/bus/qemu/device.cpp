#include "device.h"
#include "../pci/api.h"

namespace QEMU {

Device::Device(void *addr) {
	device_addr = addr;
	PCI::enableBusMastering(addr);
	PCI::enableInterruptLine(addr);
	enableInterrupt();
}

void Device::enableInterrupt() {}

void Device::setStatusBit(uint8_t status_bit) {}

void Device::setupQueues() {}

void Device::configWrite16(uint32_t offset, uint16_t value) {}

} // namespace QEMU
