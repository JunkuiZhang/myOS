#pragma once
#include "acpi.h"
#include <cstdint>

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

struct PCIHeader0 {
	PCIDeviceHeader header;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t card_bus_cis_pointer;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t expansion_rom_base_address;
	uint8_t capability_pointer;
	uint8_t reserved0;
	uint16_t reserved1;
	uint32_t reserved2;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t minimum_grant;
	uint8_t maximum_latency;
};

void enumeratePCI(ACPI::MCFGHeader *mcfg);

extern const char *DEVICE_CLASSES[];
const char *getVendorName(uint16_t vendor_id);
const char *getDeviceName(uint16_t vendor_id, uint16_t device_id);
const char *getSubClassName(uint8_t class_code, uint8_t subclass_code);
const char *getProgramInterfaceName(uint8_t class_code, uint8_t subclass_code,
									uint8_t prog_interface);

} // namespace PCI
