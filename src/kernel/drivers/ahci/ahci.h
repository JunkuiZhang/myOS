#pragma once
#include "../pci.h"
#include <stdint.h>

namespace AHCI {

enum PortType {
	None = 0,
	SATA = 1,
	SEMB = 2,
	PM = 3,
	SATAPI = 4,
};

struct HostBusAdapterPort {
	uint32_t command_list_base;
	uint32_t command_list_base_upper;
	uint32_t fis_base_address;
	uint32_t fis_base_address_upper;
	uint32_t interrupt_status;
	uint32_t interrupt_enable;
	uint32_t command_status;
	uint32_t reserved;
	uint32_t task_file_data;
	uint32_t signature;
	uint32_t sata_status;
	uint32_t sata_control;
	uint32_t sata_error;
	uint32_t sata_active;
	uint32_t command_issue;
	uint32_t sata_notification;
	uint32_t fis_switch_control;
	uint32_t reserved1[11];
	uint32_t vendor[4];
};

struct HostBusAdapterMemory {
	uint32_t host_capability;
	uint32_t global_host_control;
	uint32_t interrupt_status;
	uint32_t ports_impled;
	uint32_t version;
	uint32_t ccc_control;
	uint32_t ccc_ports;
	uint32_t enclosure_managment_location;
	uint32_t enclosure_managment_control;
	uint32_t host_capability_extended;
	uint32_t bios_handoff_control_status;
	uint8_t reserved0[0x74];
	uint8_t vendor[0x60];
	HostBusAdapterPort ports[1];
};

class AHCIDriver {
  private:
	PCI::PCIDeviceHeader *pci_base_address;
	HostBusAdapterMemory *abar;

	void probePorts();

  public:
	AHCIDriver(PCI::PCIDeviceHeader *pci_base_addr);
	~AHCIDriver();
};

} // namespace AHCI
