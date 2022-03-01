#include "acpi.h"

namespace ACPI {

bool stringCompare(char *str1, char *str2, uint64_t length) {
	for (uint64_t x = 0; x < length; x++) {
		if (str1[x] != str2[x])
			return false;
	}
	return true;
}

void *findTable(SDTHeader *sdt_header, char *signature) {
	uint64_t entries = (sdt_header->length - sizeof(SDTHeader)) / 8;
	for (uint64_t x = 0; x < entries; x++) {
		SDTHeader *new_header = (SDTHeader *)(*(
			(uint64_t *)((uint64_t)sdt_header + sizeof(SDTHeader) + 8 * x)));
		if (stringCompare(new_header->signature, signature, 4)) {
			return (void *)new_header;
		}
	}
	return 0;
}

} // namespace ACPI
