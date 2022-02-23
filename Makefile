BIOS_CODE := build/bios/OVMF_CODE.fd
BIOS_VARS := build/bios/OVMF_VARS.fd

SRC = $(shell find src/kernel -name *.cpp)
OBJ = $(patsubst src/kernel/%.cpp, build/kernel/%.o, $(SRC))

run: esp_directory build/myos_img/kernel.elf
	qemu-system-x86_64 -net none \
	-drive if=pflash,format=raw,file=$(BIOS_CODE),readonly=on \
	-drive if=pflash,format=raw,file=$(BIOS_VARS),readonly=on \
	-drive format=raw,file=fat:rw:build/myos_img

esp_directory: boot_loader
	cp src/posix-uefi/bootloader.efi build/myos_img/EFI/BOOT/BootX64.efi

boot_loader: 
	cd src/posix-uefi && make

build/kernel/main.o: src/kernel/main.cpp
	clang -target x86_64-elf -fno-stack-protector -fno-stack-check -fno-exceptions -D__x86_64__ -I. -c $< -o $@

build/kernel/%.o: src/kernel/%.cpp
	@mkdir -p $(@D)
	clang -target x86_64-elf -fno-stack-protector -fno-stack-check -fno-exceptions -D__x86_64__ -I. -c $(patsubst build/kernel/%.o, src/kernel/%.cpp, $@) -o $@

build/myos_img/kernel.elf: build/kernel/main.o $(OBJ)
	ld.lld -nostdlib -z max-page-size=0x1000 -Ttext=0x01000000 -o $@ $^
