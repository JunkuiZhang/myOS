#pragma once
#include "../pci.h"
#include <cstdint>

class QemuGraphicsController {
  public:
	QemuGraphicsController(PCI::PCIDeviceHeader *pci_base_addr);
	~QemuGraphicsController() = default;

	void start();

  private:
	PCI::PCIDeviceHeader *pci_base_address;
	void *mmio_addr;
	uint32_t *gtt_addr;
	void *aperture_bar;

	void initDisplay();
	void changeCDFrequency();
	void setupPlanes();
};
