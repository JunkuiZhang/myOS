#pragma once
#include "acpi.h"
#include <stdint.h>

namespace PCI {

struct PCIDeviceHeader {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t program_interface;
	uint8_t subclass;
	uint8_t mclass;
	uint8_t cacheline_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
};

void enumeratePCI(ACPI::MCFGHeader *mcfg);

} // namespace PCI
