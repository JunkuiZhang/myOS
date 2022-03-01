#include "ahci.h"
#include "../../app/shell/shell.h"
#include "../../memory/heap.h"
#include "../../memory/memory_utils.h"
#include "../../memory/pagetable_manager.h"

namespace AHCI {

#define HBA_PORT_DEVICE_PRESENT 0x3
#define HBA_PORT_INTERFACE_POWER_MANAGEMENT_ACTIVE 0x1
#define SATA_SIGNATURE_ATAPI 0xeb140101
#define SATA_SIGNATURE_ATA 0x00000101
#define SATA_SIGNATURE_SEMB 0xc33c0101
#define SATA_SIGNATURE_PM 0x96690101

#define HBA_PORT_COMMAND_CR 0x8000
#define HBA_PORT_COMMAND_FRE 0x0010
#define HBA_PORT_COMMAND_ST 0x0001
#define HBA_PORT_COMMAND_FR 0x4000

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
			if (port_type == PortType::SATA || port_type == PortType::SATAPI) {
				ports[port_count] = new Port();
				ports[port_count]->port_type = port_type;
				ports[port_count]->hba_port = &abar->ports[x];
				ports[port_count]->port_number = port_count;
				port_count++;
			}
		}
	}
}

Port::Port() {}
Port::~Port() {}

void Port::configure() {
	endCommand();

	void *new_base = OS_PAGEFRAME_ALLOCATOR->requestPage();
	hba_port->command_list_base = (uint32_t)(uint64_t)new_base;
	hba_port->command_list_base_upper = (uint32_t)((uint64_t)new_base >> 32);
	memset((void *)(hba_port->command_list_base), 0, 1024);

	void *fis_base = OS_PAGEFRAME_ALLOCATOR->requestPage();
	hba_port->fis_base_address = (uint32_t)(uint64_t)fis_base;
	hba_port->fis_base_address_upper = (uint32_t)((uint64_t)fis_base >> 32);
	memset(fis_base, 0, 256);

	HBACommandHeader *command_header =
		(HBACommandHeader *)((uint64_t)hba_port->command_list_base +
							 ((uint64_t)hba_port->command_list_base_upper
							  << 32));
	for (int x = 0; x < 32; x++) {
		command_header[x].prdt_length = 8;
		void *command_table_address = OS_PAGEFRAME_ALLOCATOR->requestPage();
		uint64_t address = (uint64_t)command_table_address + (x << 8);
		command_header[x].command_table_base_addr = (uint32_t)(uint64_t)address;
		command_header[x].command_table_base_addr_upper =
			(uint32_t)((uint64_t)address >> 32);
		memset(command_table_address, 0, 256);
	}

	startCommand();
}

void Port::startCommand() {
	while (hba_port->command_status & HBA_PORT_COMMAND_CR)
		;
	hba_port->command_status |= HBA_PORT_COMMAND_FRE;
	hba_port->command_status |= HBA_PORT_COMMAND_ST;
}

void Port::endCommand() {
	hba_port->command_status &= ~HBA_PORT_COMMAND_ST;
	hba_port->command_status &= ~HBA_PORT_COMMAND_FRE;
	while (1) {
		if (hba_port->command_status & HBA_PORT_COMMAND_FR)
			continue;
		if (hba_port->command_status & HBA_PORT_COMMAND_CR)
			continue;
		break;
	}
}

AHCIDriver::AHCIDriver(PCI::PCIDeviceHeader *pci_base_addr) {
	this->pci_base_address = pci_base_addr;
	OS_SHELL->println("AHCI driver init.");

	abar =
		(HostBusAdapterMemory *)(((PCI::PCIHeader0 *)pci_base_address)->bar5);
	OS_PAGETABLE_MANAGER->mapMemory(abar, abar);
	probePorts();

	for (int x = 0; x < port_count; x++) {
		Port *port = ports[x];
		port->configure();
	}
}

AHCIDriver::~AHCIDriver() {}

} // namespace AHCI
