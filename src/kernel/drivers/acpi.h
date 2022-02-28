#pragma once
#include <stdint.h>

namespace ACPI {

struct RSDP2 {
	char signature[8];
	uint8_t check_sum;
	char oem_id[6];
	uint8_t revision;
	uint32_t RSDT_address;
	uint32_t length;
	uint64_t XSDT_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__((packed));

struct SDTHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t check_sum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

// struct XSDT {
// 	struct SDTHeader ph;
// 	uint64_t other[(ph.Length - sizeof(ph)) / 8];
// };

struct MCFGHeader {
	SDTHeader header;
	uint64_t reserved;
} __attribute__((packed));

struct DeviceConfig {
	uint64_t base_addr;
	uint16_t pci_seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t reserved;
} __attribute__((packed));

void *findTable(SDTHeader *sdt_header, char *signature);

} // namespace ACPI
