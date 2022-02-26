#include "efi_memory.h"

const char *EFI_MEMORY_TYPE_STRINGS[] = {
	"EFI_Reserved_Memory_Type",
	"EFI_Loader_Code",
	"EFI_Loader_Data",
	"EFI_Bootservices_Code",
	"EFI_Bootservices_Data",
	"EFI_Runtimeservices_Code",
	"EFI_Runtimeservices_Data",
	"EFI_Conventional_Memory",
	"EFI_Unusable_Memory",
	"EFI_ACPI_Reclaim_Memory",
	"EFI_ACPI_Memory_NVS",
	"EFI_Memory_MappedIO",
	"EFI_Memory_MappedIO_Port_Space",
	"EFI_Pal_Code",
};
