BIOS_CODE := build/bios/OVMF_CODE.fd
BIOS_VARS := build/bios/OVMF_VARS.fd
# macos
EFI_DIRS := /usr/local/include/efi
PREFIX := x86_64-elf-

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

test0:
	$(PREFIX)gcc -I $(EFI_DIRS) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c src/gnu-efi/bootloader/main.c -o build/gnu-efi/main.o
test1: test0
	$(PREFIX)ld -shared -Bsymbolic -L $(EFI_DIRS) -T /usr/local/lib/elf_x86_64_efi.lds src/gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o /usr/local/lib/libefi.a /usr/local/lib/libgnuefi.a build/gnu-efi/main.o -o build/gnu-efi/main.so

test2: test1
	$(PREFIX)objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 build/gnu-efi/main.so build/gnu-efi/main.efi

test3: test2
	# cp build/efi-clang/boot_loader.efi build/myos_img/EFI/BOOT/BootX64.efi
	qemu-system-x86_64 -net none \
	-drive if=pflash,format=raw,file=$(BIOS_CODE),readonly=on \
	-drive if=pflash,format=raw,file=$(BIOS_VARS),readonly=on \
	-drive format=raw,file=fat:rw:build/myos_img
