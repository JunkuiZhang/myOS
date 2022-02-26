#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>

typedef struct {
	unsigned int *frame_base;
	unsigned int width;
	unsigned int height;
} GOPInfo;

EFI_FILE *loadKernel(EFI_FILE *directory, CHAR16 *path, EFI_HANDLE image_handle,
					 EFI_SYSTEM_TABLE *system_table) {
	EFI_FILE *ret;
	EFI_STATUS status;
	Print(L"Trying to load kernel...\n");
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
	status =
		uefi_call_wrapper(BS->HandleProtocol, 3, image_handle,
						  &gEfiLoadedImageProtocolGuid, (void **)&loaded_image);
	if (EFI_ERROR(status)) {
		Print(L"Unable to load image!\n");
	}
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;
	status = uefi_call_wrapper(
		BS->HandleProtocol, 3, loaded_image->DeviceHandle,
		&gEfiSimpleFileSystemProtocolGuid, (void **)&file_system);
	if (EFI_ERROR(status)) {
		Print(L"Unable to load file!\n");
	}
	if (directory == NULL) {
		status = uefi_call_wrapper(file_system->OpenVolume, 2, file_system,
								   &directory);
		if (EFI_ERROR(status)) {
			Print(L"Unable to open volume!\n");
		}
	}
	status = uefi_call_wrapper(directory->Open, 4, directory, &ret, path,
							   EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(status)) {
		Print(L"Unable to open file! ERROR: %d\n", status & 0xffff);
		return NULL;
	} else {
		return ret;
	}
}

int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i])
			return -1;
		else if (a[i] > b[i])
			return 1;
	}
	return 0;
}

GOPInfo gop_info;
GOPInfo *initializeGOP() {
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status =
		uefi_call_wrapper(BS->LocateProtocol, 3,
						  &gEfiGraphicsOutputProtocolGuid, NULL, (void **)&gop);
	if (EFI_ERROR(status)) {
		Print(L"Unable to locate gop! ERROR: %d\n", status & 0xffff);
		return NULL;
	} else {
		Print(L"GOP located!\n");
	}
	gop_info.frame_base = (unsigned int *)gop->Mode->FrameBufferBase;
	gop_info.width = gop->Mode->Info->HorizontalResolution;
	gop_info.height = gop->Mode->Info->VerticalResolution;
	Print(L"width: %d, height: %d\n", gop->Mode->Info->HorizontalResolution,
		  gop->Mode->Info->VerticalResolution);
	return &gop_info;
}

typedef struct {
	unsigned int *framebuffer;
	unsigned int width;
	unsigned int height;
	EFI_MEMORY_DESCRIPTOR *mem_map;
	UINTN mem_map_size;
	UINTN mem_desc_size;
} BootParamater;

EFI_STATUS
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
	BootParamater boot_param;
	EFI_STATUS status;
	// Elf64_Addr kernel_entry;

	InitializeLib(image_handle, system_table);
	Print(L"Hello from UEFI!\n");

	EFI_FILE *kernel =
		loadKernel(NULL, L"\\kernel.elf", image_handle, system_table);
	if (kernel == NULL) {
		Print(L"Unable to load kernel!\n");
	} else {
		Print(L"Kernel have been loaded!\n");
	}

	Elf64_Ehdr kernel_file_header;
	{
		UINTN file_info_size;
		EFI_FILE_INFO *file_info;
		uefi_call_wrapper(kernel->GetInfo, 4, kernel, &gEfiFileInfoGuid,
						  &file_info_size, NULL);
		uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, file_info_size,
						  (void **)&file_info);
		// system_table->BootServices->AllocatePool(EfiLoaderData,
		// file_info_size, (void **)&file_info);
		status =
			uefi_call_wrapper(kernel->GetInfo, 4, kernel, &gEfiFileInfoGuid,
							  &file_info_size, (void **)&file_info);
		// kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_info_size,
		// 				(void **)&file_info);
		if (EFI_ERROR(status)) {
			Print(L"==> Unable to get file info. ERROR: %d", status & 0xffff);
		}
		UINTN size = sizeof(kernel_file_header);
		Print(L"Reading file...\n");
		status = uefi_call_wrapper(kernel->Read, 3, kernel, &size,
								   &kernel_file_header);
		if (EFI_ERROR(status)) {
			Print(L"==> Unable to read file header. ERROR: %d",
				  status & 0xffff);
		}
		// kernel->Read(kernel, &size, &kernel_file_header);
	}

	if (memcmp(&kernel_file_header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		kernel_file_header.e_ident[EI_CLASS] != ELFCLASS64 ||
		kernel_file_header.e_ident[EI_DATA] != ELFDATA2LSB ||
		kernel_file_header.e_type != ET_EXEC ||
		kernel_file_header.e_machine != EM_X86_64 ||
		kernel_file_header.e_version != EV_CURRENT) {
		Print(L"Kernel foramt invalid.\n");
	} else {
		Print(L"Kernel header read.\n");
	}

	Elf64_Phdr *kernel_phdr;
	{
		uefi_call_wrapper(kernel->SetPosition, 2, kernel,
						  kernel_file_header.e_phoff);
		// kernel->SetPosition(kernel, kernel_file_header.e_phoff);
		UINTN size =
			kernel_file_header.e_phnum * kernel_file_header.e_phentsize;
		uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, size,
						  (void **)&kernel_phdr);
		// system_table->BootServices->AllocatePool(EfiLoaderData, size,
		// 										 (void **)&kernel_phdr);
		uefi_call_wrapper(kernel->Read, 3, kernel, &size, kernel_phdr);
		// kernel->Read(kernel, &size, kernel_phdr);
	}

	for (Elf64_Phdr *phdr = kernel_phdr;
		 (char *)phdr <
		 (char *)kernel_phdr +
			 kernel_file_header.e_phnum * kernel_file_header.e_phentsize;
		 phdr = (Elf64_Phdr *)((char *)phdr + kernel_file_header.e_phentsize)) {
		switch (phdr->p_type) {
		case PT_LOAD: {
			int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			Elf64_Addr segment = phdr->p_paddr;
			uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress,
							  EfiLoaderData, pages, &segment);
			// system_table->BootServices->AllocatePages(
			// 	AllocateAddress, EfiLoaderData, pages, &segment);
			uefi_call_wrapper(kernel->SetPosition, 2, kernel, phdr->p_offset);
			// kernel->SetPosition(kernel, phdr->p_offset);
			UINTN size = phdr->p_filesz;
			uefi_call_wrapper(kernel->Read, 3, kernel, &size, (void *)segment);
			// kernel->Read(kernel, &size, (void *)segment);
			break;
		}
		}
	}

	Print(L"Kernel have been loaded.\n");
	// kernel_entry = kernel_file_header.e_entry;

	GOPInfo *temp_info = initializeGOP();

	boot_param.framebuffer = (unsigned int *)(temp_info->frame_base);
	boot_param.width = temp_info->width;
	boot_param.height = temp_info->height;

	void (*kernel_entry)(BootParamater *) = ((__attribute__((
		sysv_abi)) void (*)(BootParamater *))kernel_file_header.e_entry);

	while (1) {
		/* get memory map */
		Print(L"Reading memory map.\n");
		EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
		UINTN map_size = 0;
		UINTN map_key = 0;
		UINTN desc_size = 0;
		{
			status = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size,
									   memory_map, &map_key, &desc_size, NULL);
			if (status != EFI_BUFFER_TOO_SMALL || !map_size) {
				Print(L"Unable to init memory map. ERROR: %d\n",
					  status & 0xffff);
			}
			Print(L"Memory map size: %d\n", map_size);
			map_size += 4 * desc_size;
			status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData,
									   map_size, (void **)&memory_map);
			if (!memory_map) {
				Print(L"Unable to allocate memory map. ERROR: %d\n",
					  status & 0xffff);
			}
			status = uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size,
									   memory_map, &map_key, &desc_size, NULL);
			if (EFI_ERROR(status)) {
				Print(L"Unable to get memory map. ERROR: %d\n",
					  status & 0xffff);
			}
			Print(L"Memory map read. map key: %d\n", map_key);
		}

		boot_param.mem_map = memory_map;
		boot_param.mem_map_size = map_size;
		boot_param.mem_desc_size = desc_size;
		EFI_MEMORY_DESCRIPTOR *temp = (EFI_MEMORY_DESCRIPTOR *)memory_map;
		Print(L"Memory type: %d\n", temp[0].Type);

		status =
			uefi_call_wrapper(BS->ExitBootServices, 2, image_handle, map_key);
		if (EFI_ERROR(status)) {
			Print(L"Exit BS failed! ERROR: %d\n", status & 0xffff);
		}

		break;
	}

	kernel_entry(&boot_param);

	while (1)
		;

	return EFI_SUCCESS;
}