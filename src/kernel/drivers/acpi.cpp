#include "acpi.h"

namespace ACPI {

void *findTable(ACPI::SDTHeader *sdt_header, char *signature) {
	uint32_t entries = (sdt_header->length - sizeof(ACPI::SDTHeader)) / 8;
	for (int x = 0; x < entries; x++) {
		ACPI::SDTHeader *new_header =
			(ACPI::SDTHeader *)((uint64_t)sdt_header + sizeof(ACPI::SDTHeader) +
								8 * x);
		for (int y = 0; y < 4; y++) {
			if (new_header->signature[y] != signature[y]) {
				break;
			}
			if (y == 3)
				return new_header;
		}
	}
	return 0;
}

} // namespace ACPI
