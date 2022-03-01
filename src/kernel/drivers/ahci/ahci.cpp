#include "ahci.h"
#include "../../app/shell/shell.h"
#include "../../memory/pagetable_manager.h"

namespace AHCI {

#define HBA_PORT_DEVICE_PRESENT 0x3
#define HBA_PORT_INTERFACE_POWER_MANAGEMENT_ACTIVE 0x1
#define SATA_SIGNATURE_ATAPI 0xeb140101
#define SATA_SIGNATURE_ATA 0x00000101
#define SATA_SIGNATURE_SEMB 0xc33c0101
#define SATA_SIGNATURE_PM 0x96690101

PortType checkPortType(HostBusAdapterPort *port) {
	uint32_t sata_status = port->sata_status;
	uint8_t interface_power_management = (sata_status >> 8) & 0b111;
	uint8_t device_detection = sata_status & 0b111;

	if (device_detection != HBA_PORT_DEVICE_PRESENT)
		return PortType::None;
	if (interface_power_management !=
		HBA_PORT_INTERFACE_POWER_MANAGEMENT_ACTIVE)
		return PortType::None;
	switch (port->signature) {
	case SATA_SIGNATURE_ATAPI:
		return PortType::SATAPI;
	case SATA_SIGNATURE_ATA:
		return PortType::SATA;
	case SATA_SIGNATURE_PM:
		return PortType::PM;
	case SATA_SIGNATURE_SEMB:
		return PortType::SEMB;
	default:
		return PortType::None;
	}
}

void AHCIDriver::probePorts() {
	uint32_t ports_impled = abar->ports_impled;
	for (int x = 0; x < 32; x++) {
		if (ports_impled & (1 << x)) {
			PortType port_type = checkPortType(&abar->ports[x]);
			if (port_type == PortType::SATA) {
				OS_SHELL->println("SATA");
			} else if (port_type == PortType::SATAPI) {
				OS_SHELL->println("SATAPI");
			}
		}
	}
}

AHCIDriver::AHCIDriver(PCI::PCIDeviceHeader *pci_base_addr) {
	this->pci_base_address = pci_base_addr;
	OS_SHELL->println("AHCI driver init.");

	abar =
		(HostBusAdapterMemory *)(((PCI::PCIHeader0 *)pci_base_address)->bar5);
	OS_PAGETABLE_MANAGER->mapMemory(abar, abar);
	probePorts();
}

AHCIDriver::~AHCIDriver() {}

} // namespace AHCI
