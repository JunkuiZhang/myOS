#include "pci.h"
#include "../memory/pagetable_manager.h"

namespace PCI {

void enumerateFunction(uint64_t device_addr, uint64_t function) {
	uint64_t offset = function << 12;
	uint64_t func_addr = device_addr + offset;
	OS_PAGETABLE_MANAGER->mapMemory((void *)func_addr, (void *)func_addr);
	PCIDeviceHeader *device_header = (PCIDeviceHeader *)func_addr;
	if (device_header->device_id == 0)
		return;
	if (device_header->device_id == 0xffff)
		return;
}

void enumerateDevice(uint64_t bus_addr, uint64_t device) {
	uint64_t offset = device << 15;
	uint64_t device_addr = bus_addr + offset;
	OS_PAGETABLE_MANAGER->mapMemory((void *)device_addr, (void *)device_addr);
	PCIDeviceHeader *device_header = (PCIDeviceHeader *)device_addr;
	if (device_header->device_id == 0)
		return;
	if (device_header->device_id == 0xffff)
		return;

	for (uint64_t func = 0; func < 8; func++) {
		enumerateFunction(device_addr, func);
	}
}

void enumerateBus(uint64_t base_addr, uint64_t bus) {
	uint64_t offset = bus << 20;
	uint64_t bus_addr = base_addr + offset;
	OS_PAGETABLE_MANAGER->mapMemory((void *)bus_addr, (void *)bus_addr);
	PCIDeviceHeader *device_header = (PCIDeviceHeader *)bus_addr;
	if (device_header->device_id == 0)
		return;
	if (device_header->device_id == 0xffff)
		return;

	for (uint64_t device = 0; device < 32; device++) {
		enumerateDevice(bus_addr, device);
	}
}

void enumeratePCI(ACPI::MCFGHeader *mcfg) {
	int entries = (mcfg->header.length - sizeof(ACPI::MCFGHeader)) /
				  sizeof(ACPI::DeviceConfig);
	for (int x = 0; x < entries; x++) {
		ACPI::DeviceConfig *new_config =
			(ACPI::DeviceConfig *)((uint64_t)mcfg + sizeof(ACPI::MCFGHeader) +
								   x * sizeof(ACPI::DeviceConfig));
		for (uint64_t bus = new_config->start_bus; bus < new_config->end_bus;
			 bus++) {
			enumerateBus(new_config->base_addr, bus);
		}
	}
}

} // namespace PCI
