#pragma once
#include "../pci.h"
#include "utils.h"
#include <cstdint>

namespace Graphics {

class IntelGraphicsDriver {
  public:
	IntelGraphicsDriver(PCI::PCIDeviceHeader *pci_base_addr);
	~IntelGraphicsDriver() = default;

	void start();

  private:
	enum GMBusPinPair : uint8_t {
		None = 0,
		DedicatedControl = 1,
		DedicatedAnalog = 0b10,
		IntegratedDigital = 0b11,
		sDVO = 0b101,
		Dconnector = 0b111,
	};

	PCI::PCIDeviceHeader *pci_base_address;
	PHYSICAL_ADDRESS mmio_addr;
	PHYSICAL_ADDRESS framebuffer_addr;

	uint32_t readFromRegister(Intel::RegisterIndex index);
	void writeToRegister(Intel::RegisterIndex index, uint32_t value);
	void setGmbusDefaultRate();
	void setGmbusPinPair(GMBusPinPair pin_pair);

	void initDisplay();
	void disableVGA();
	void changeCDFrequency();
	void setupPlanes();
};

} // namespace Graphics
