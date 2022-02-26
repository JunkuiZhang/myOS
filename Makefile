BIOS_CODE := build/bios/OVMF_CODE.fd
BIOS_VARS := build/bios/OVMF_VARS.fd
# macos
EFI_DIRS := /usr/local/include/efi
PREFIX := x86_64-elf-

GCC_FLAGS = \
	-I $(EFI_DIRS) -fpic -ffreestanding -fno-stack-protector \
	-fno-stack-check -fshort-wchar -mno-red-zone \
	-maccumulate-outgoing-args

GLD_FLAGS = \
	-shared -Bsymbolic \
	-L $(EFI_DIRS) -L/usr/local/lib \
	-T/usr/local/lib/elf_x86_64_efi.lds

GLD_LIBS = \
	/usr/local/lib/crt0-efi-x86_64.o -lgnuefi -lefi

OBJ_FLAGS = \
	-j .text -j .sdata -j .data -j .dynamic -j .dynsym  \
	-j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
	--target efi-app-x86_64 --subsystem=10

SRC = $(shell find src/kernel -name *.cpp)
OBJ = $(patsubst src/kernel/%.cpp, build/kernel/%.o, $(SRC))

run: build/myos_img/EFI/BOOT/BootX64.efi build/myos_img/kernel.elf
# run: build/myos_img/EFI/BOOT/BootX64.efi esp_directory
	qemu-system-x86_64 -m 128M -net none \
	-drive if=pflash,format=raw,file=$(BIOS_CODE),readonly=on \
	-drive if=pflash,format=raw,file=$(BIOS_VARS),readonly=on \
	-drive format=raw,file=fat:rw:build/myos_img

build/bootloader/main.o: src/bootloader/main.c
	$(PREFIX)gcc $(GCC_FLAGS) -c $< -o $@

build/bootloader/main.so: build/bootloader/main.o
	$(PREFIX)ld $(GLD_FLAGS) -o $@ $< $(GLD_LIBS)

build/bootloader/main.efi: build/bootloader/main.so
	$(PREFIX)objcopy $(OBJ_FLAGS) $< $@

build/myos_img/EFI/BOOT/BootX64.efi: build/bootloader/main.efi
	cp $< $@

# build/kernel/main.o: src/kernel/main.cpp
# 	$(PREFIX)gcc -I $(EFI_DIRS) -ffreestanding -fshort-wchar -fno-exceptions -c $< -o $@

# build/kernel/%.o: src/kernel/%.cpp
# 	@mkdir -p $(@D)
# 	$(PREFIX)gcc -I $(EFI_DIRS) -ffreestanding -fshort-wchar -fno-exceptions -c $(patsubst build/kernel/%.o, src/kernel/%.cpp, $@) -o $@

# build/myos_img/kernel.elf: build/kernel/main.o $(OBJ)
# 	$(PREFIX)ld -T src/kernel/kernel.ld -static -Bsymbolic -nostdlib -o $@ $^

# esp_directory: boot_loader
# 	cp src/posix-uefi/bootloader.efi build/myos_img/EFI/BOOT/BootX64.efi

# boot_loader: 
# 	cd src/posix-uefi && make

build/kernel/main.o: src/kernel/main.cpp
	clang -target x86_64-elf -fno-stack-protector -fno-stack-check -fno-exceptions -D__x86_64__ -I. -c $< -o $@

build/kernel/%.o: src/kernel/%.cpp
	@mkdir -p $(@D)
	clang -target x86_64-elf -fno-stack-protector -fno-stack-check -fno-exceptions -D__x86_64__ -I. -c $(patsubst build/kernel/%.o, src/kernel/%.cpp, $@) -o $@

build/myos_img/kernel.elf: build/kernel/main.o $(OBJ)
	ld.lld -nostdlib -z max-page-size=0x1000 -T src/kernel/kernel.ld -o $@ $^

# run: esp_directory build/myos_img/kernel.elf
# 	qemu-system-x86_64 -net none \
# 	-drive if=pflash,format=raw,file=$(BIOS_CODE),readonly=on \
# 	-drive if=pflash,format=raw,file=$(BIOS_VARS),readonly=on \
# 	-drive format=raw,file=fat:rw:build/myos_img
