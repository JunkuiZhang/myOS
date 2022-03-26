#include "qemu.h"
#include "../../app/shell/shell.h"
#include "../../scheduling/pit/pit.h"
#include "cursor.h"
#include <cstdint>

#define NDE_RSTWRN_OPT 0x46408
#define FUSE_STATUS 0x42000
#define PWR_WELL_CTL 0x45400 /* Display power control */
#define CDCLK_CTL 0x46000	 /* Change CD clock frequency */
#define DPLL_CTRL1 0x6c058	 /* Control the DPLL mode, rate, and SSC */
#define LCPLL1_CTL                                                             \
	0x46010 /* Enable DPLL0 for driving the display core clock (CDCLK), the    \
core display 2X clock (CD2XCLK), and the DDI ports */
#define DBUF_CTL 0x45008 /* DBUF Control */
#define CUR_CTL 0x70080	 /* Cursor Control */
#define CUR_BASE 0x70084 /* Cursor Base Address */
#define CUR_POS 0x70088

void gfxWrite32(void *addr, uint32_t reg, uint32_t value) {
	*(volatile uint32_t *)((uint8_t *)addr + reg) = value;
}

uint32_t gfxRead32(void *addr, uint32_t reg) {
	return *(volatile uint32_t *)((uint8_t *)addr + reg);
}

QemuGraphicsController::QemuGraphicsController(
	PCI::PCIDeviceHeader *pci_base_addr) {
	this->pci_base_address = pci_base_addr;
	auto header = (PCI::PCIHeader0 *)pci_base_address;
	OS_SHELL->println("PCI graphics driver init...");
	OS_SHELL->println(
		"Vendor ID: %x, device ID: %x, bar0: %x, bar2: %x, bar4: %x",
		header->header.vendor_id, header->header.device_id, header->bar0,
		header->bar2, header->bar4);
	mmio_addr = (void *)header->bar0;
	gtt_addr = (uint32_t *)((uint8_t *)header->bar0 + 2 * 1024 * 1024);
	aperture_bar = (void *)header->bar2;
}

void QemuGraphicsController::start() {
	initDisplay();
	setupPlanes();
}

void QemuGraphicsController::initDisplay() {
	/* https://www.x.org/docs/intel/KBL/intel-gfx-prm-osrc-kbl-vol12-display.pdf
	 * page 112 */
	uint32_t result;
	uint32_t mask;
	// 1. Enable PCH reset handshake
	// a. Set NDE_RSTWRN_OPT RST PCH Handshake En to 1b
	gfxWrite32(mmio_addr, NDE_RSTWRN_OPT, 1 << 4);
	// 2. Enable Power Well 1 (PG1) and Misc IO Power
	// a. Poll for FUSE_STATUS Fuse PG0 Distribution Status = 1b
	result = gfxRead32(mmio_addr, FUSE_STATUS);
	PIT::sleep(100);
	if (result & (1 << 27)) {
		OS_SHELL->println("FUSE_STATUS check!");
	} else {
		OS_SHELL->println("FUSE_STATUS fail!");
	}
	// b. Set PWR_WELL_CTL Power Well 1 Request and Misc IO Power Request to 1b
	mask = (1 << 29) | (1 << 1);
	gfxWrite32(mmio_addr, PWR_WELL_CTL, mask);
	// c. Poll for PWR_WELL_CTL Power Well 1 State and Misc IO Power State = 1b
	result = gfxRead32(mmio_addr, PWR_WELL_CTL);
	PIT::sleep(100);
	if (result & mask) {
		OS_SHELL->println("PWR_WELL_CTL check!");
	} else {
		OS_SHELL->println("PWR_WELL_CTL fail!");
	}
	// Poll for FUSE_STATUS Fuse PG1 Distribution Status = 1b
	result = gfxRead32(mmio_addr, FUSE_STATUS);
	PIT::sleep(100);
	if (result & (1 << 26)) {
		OS_SHELL->println("FUSE_STATUS check!");
	} else {
		OS_SHELL->println("FUSE_STATUS fail!");
	}
	// 3. Enable CDCLK PLL
	// a. Set CDCLK_CTL CD Frequency Select to the minimum.
	gfxWrite32(mmio_addr, CDCLK_CTL, 1 << 27);
	// b. Configure DPLL0 link rate to the rate required for the eDP panel, or
	// 810 MHz (DP 1.62 GHz) if there is no eDP panel.
	gfxWrite32(mmio_addr, DPLL_CTRL1, 0x10);
	// c. Set LCPLL1_CTL PLL enable to 1b.
	gfxWrite32(mmio_addr, LCPLL1_CTL, 1 << 31);
	// d. Poll for LCPLL1_CTL PLL lock = 1b.
	result = gfxRead32(mmio_addr, LCPLL1_CTL);
	PIT::sleep(100);
	if (result & (1 << 30)) {
		OS_SHELL->println("LCPLL check!");
	} else {
		OS_SHELL->println("LCPLL fail!");
	}
	// 4. Change CD Clock Frequency
	// 5. Enable DBUF
	// a. Set DBUF_CTL DBUF Power Request to 1b.
	gfxWrite32(mmio_addr, DBUF_CTL, 1 << 31);
	// b. Poll for DBUF_CTL DBUF Power State = 1b.
	result = gfxRead32(mmio_addr, DBUF_CTL);
	PIT::sleep(100);
	if (result & (1 << 31)) {
		OS_SHELL->println("DBUF check!");
	} else {
		OS_SHELL->println("DBUF fail!");
	}
}

void QemuGraphicsController::changeCDFrequency() {
	// 1 Disable all display engine functions using the full mode set disable
	// sequence on all pipes, ports, and planes.
	// TODO: to impl
	// 2 Inform power controller of upcoming frequency change.
	// a. Ensure any previous GT Driver Mailbox transaction is complete.
	// TODO: to impl
	// b. Write GT Driver Mailbox Data0 (GTTMMADDR offset 0x138128) = 0x3.
	gfxWrite32(gtt_addr, 0x138128, 0x3);
	// c. Write GT Driver Mailbox Data1 (GTTMMADDR offset 0x13812C) = 0x0.
	gfxWrite32(gtt_addr, 0x13812C, 0x0);
	// d. Write GT Driver Mailbox Interface (GTTMMADDR offset 0x138124) =
	// 0x80000007.
	gfxWrite32(gtt_addr, 0x138124, 0x80000007);
	// e. Poll GT Driver Mailbox Interface for Run/Busy indication cleared (bit
	// 31 = 0).
	PIT::sleep(100);
	// 3 Change CDCLK_CTL register CD Frequency Select and CD Frequency Decimal
	// to the desired frequency
}

void QemuGraphicsController::setupPlanes() {
	/* Setup planes: Primary, cursor */
	// setup cursor plane
	gfxWrite32(mmio_addr, CUR_CTL, 0b100111);
	auto cursor_gfxbase =
		(uint32_t)((uint64_t)cursor_data - (uint64_t)aperture_bar);
	gfxWrite32(mmio_addr, CUR_BASE, cursor_gfxbase);
	gfxWrite32(mmio_addr, CUR_POS, (50 << 16) | 50);
}
