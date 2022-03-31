#include "intel.h"
#include "../../app/shell/shell.h"
#include "../../scheduling/pit/pit.h"
#include "cursor.h"
#include "utils.h"
#include <cstdint>

namespace Graphics {

IntelGraphicsDriver::IntelGraphicsDriver(PCI::PCIDeviceHeader *pci_base_addr) {
	this->pci_base_address = pci_base_addr;
	auto header0 = (PCI::PCIHeader0 *)pci_base_address;
	mmio_addr = header0->bar0 & 0xfffffffc;
	framebuffer_addr = header0->bar2 & 0xfffffffc;
	OS_SHELL->println("Intel graphics driver init... @%x", pci_base_addr);
	// enableBusMastering((void *)pci_base_addr);
	// {
	// 	setGmbusDefaultRate();
	// 	setGmbusPinPair(GMBusPinPair::DedicatedAnalog);
	// }
}

uint32_t IntelGraphicsDriver::readFromRegister(Intel::RegisterIndex index) {
	auto *reg = (volatile uint32_t *)(mmio_addr + index);
	uint32_t value = *reg;
	return value;
}

void IntelGraphicsDriver::writeToRegister(Intel::RegisterIndex index,
										  uint32_t value) {
	auto *reg = (volatile uint32_t *)(mmio_addr + index);
	*reg = value;
}

void IntelGraphicsDriver::setGmbusPinPair(GMBusPinPair pin_pair) {
	writeToRegister(Intel::GMBusClock,
					(readFromRegister(Intel::GMBusClock) & (~0b111)) |
						(pin_pair & 0b111));
}

void IntelGraphicsDriver::setGmbusDefaultRate() {
	writeToRegister(Intel::GMBusClock,
					readFromRegister(Intel::GMBusClock) & ~(0b111 << 8));
}

// IntelGraphicsDriver::QemuGraphicsController(
// 	gtt_addr = (uint32_t *)((uint8_t *)header->bar0 + 2 * 1024 * 1024);
// 	aperture_bar = (void *)header->bar2;
// }

void IntelGraphicsDriver::start() {
	initDisplay();
	setupPlanes();
	disableVGA();
}

void IntelGraphicsDriver::initDisplay() {
	/* https://www.x.org/docs/intel/KBL/intel-gfx-prm-osrc-kbl-vol12-display.pdf
	 * page 112 */
	uint32_t result;
	uint32_t mask;
	// 1. Enable PCH reset handshake
	// a. Set NDE_RSTWRN_OPT RST PCH Handshake En to 1b
	writeToRegister(Intel::NDE_RSTWRN_OPT, 1 << 4);
	// 2. Enable Power Well 1 (PG1) and Misc IO Power
	// a. Poll for FUSE_STATUS Fuse PG0 Distribution Status = 1b
	result = readFromRegister(Intel::FUSE_STATUS);
	PIT::sleep(100);
	if (result & (1 << 27)) {
		OS_SHELL->println("FUSE_STATUS check!");
	} else {
		OS_SHELL->println("FUSE_STATUS fail!");
	}
	// b. Set PWR_WELL_CTL Power Well 1 Request and Misc IO Power Request to 1b
	mask = (1 << 29) | (1 << 1);
	writeToRegister(Intel::PWR_WELL_CTL, mask);
	// c. Poll for PWR_WELL_CTL Power Well 1 State and Misc IO Power State = 1b
	result = readFromRegister(Intel::PWR_WELL_CTL);
	PIT::sleep(100);
	if (result & mask) {
		OS_SHELL->println("PWR_WELL_CTL check!");
	} else {
		OS_SHELL->println("PWR_WELL_CTL fail!");
	}
	// Poll for FUSE_STATUS Fuse PG1 Distribution Status = 1b
	result = readFromRegister(Intel::FUSE_STATUS);
	PIT::sleep(100);
	if (result & (1 << 26)) {
		OS_SHELL->println("FUSE_STATUS check!");
	} else {
		OS_SHELL->println("FUSE_STATUS fail!");
	}
	// 3. Enable CDCLK PLL
	// a. Set CDCLK_CTL CD Frequency Select to the minimum.
	writeToRegister(Intel::CDCLK_CTL, 1 << 27);
	// b. Configure DPLL0 link rate to the rate required for the eDP panel, or
	// 810 MHz (DP 1.62 GHz) if there is no eDP panel.
	writeToRegister(Intel::DPLL_CTRL1, 0x10);
	// c. Set LCPLL1_CTL PLL enable to 1b.
	writeToRegister(Intel::LCPLL1_CTL, 1 << 31);
	// d. Poll for LCPLL1_CTL PLL lock = 1b.
	result = readFromRegister(Intel::LCPLL1_CTL);
	PIT::sleep(100);
	if (result & (1 << 30)) {
		OS_SHELL->println("LCPLL check!");
	} else {
		OS_SHELL->println("LCPLL fail!");
	}
	// 4. Change CD Clock Frequency
	// 5. Enable DBUF
	// a. Set DBUF_CTL DBUF Power Request to 1b.
	writeToRegister(Intel::DBUF_CTL, 1 << 31);
	// b. Poll for DBUF_CTL DBUF Power State = 1b.
	result = readFromRegister(Intel::DBUF_CTL);
	PIT::sleep(100);
	if (result & (1 << 31)) {
		OS_SHELL->println("DBUF check!");
	} else {
		OS_SHELL->println("DBUF fail!");
	}
}

void IntelGraphicsDriver::changeCDFrequency() {
	// 1 Disable all display engine functions using the full mode set disable
	// sequence on all pipes, ports, and planes.
	// TODO: to impl
	// 2 Inform power controller of upcoming frequency change.
	// a. Ensure any previous GT Driver Mailbox transaction is complete.
	// TODO: to impl
	// b. Write GT Driver Mailbox Data0 (GTTMMADDR offset 0x138128) = 0x3.
	// gfxWrite32(gtt_addr, 0x138128, 0x3);
	// c. Write GT Driver Mailbox Data1 (GTTMMADDR offset 0x13812C) = 0x0.
	// gfxWrite32(gtt_addr, 0x13812C, 0x0);
	// d. Write GT Driver Mailbox Interface (GTTMMADDR offset 0x138124) =
	// 0x80000007.
	// gfxWrite32(gtt_addr, 0x138124, 0x80000007);
	// e. Poll GT Driver Mailbox Interface for Run/Busy indication cleared (bit
	// 31 = 0).
	PIT::sleep(100);
	// 3 Change CDCLK_CTL register CD Frequency Select and CD Frequency Decimal
	// to the desired frequency
}

void IntelGraphicsDriver::setupPlanes() {
	/* Setup planes: Primary, cursor */
	// setup primary plane

	// setup cursor plane
	writeToRegister(Intel::CUR_CTL, 0b100111);
	auto cursor_gfxbase =
		(uint32_t)((uint64_t)cursor_data - (uint64_t)framebuffer_addr);
	writeToRegister(Intel::CUR_BASE, cursor_gfxbase);
	writeToRegister(Intel::CUR_POS, (50 << 16) | 50);
}

void IntelGraphicsDriver::disableVGA() {
	outByte(0x3c5, inByte(0x3c5) | (1 << 5));
	PIT::sleep(100);
	writeToRegister(Intel::VGA_CONTROL, 1 << 31);
}

} // namespace Graphics
