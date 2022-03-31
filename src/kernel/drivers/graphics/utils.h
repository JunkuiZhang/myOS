#pragma once
#include <cstdint>

namespace Graphics {

using PHYSICAL_ADDRESS = uint64_t;

namespace Intel {

enum RegisterIndex {
	PipeAConf = 0x70008,
	PipeBConf = 0x71008,
	GMBusData = 0x510C,
	GMBusStatus = 0x5108,
	GMBusCommand = 0x5104,
	GMBusClock = 0x5100,
	DisplayPlaneAControl = 0x70180,
	DisplayPlaneALinearOffset = 0x70184,
	DisplayPlaneAStride = 0x70188,
	DisplayPlaneASurface = 0x7019C,
	DPLLDivisorA0 = 0x6040,
	DPLLDivisorA1 = 0x6044,
	DPLLControlA = 0x6014,
	DPLLControlB = 0x6018,
	DPLLMultiplierA = 0x601C,
	HTotalA = 0x60000,
	HBlankA = 0x60004,
	HSyncA = 0x60008,
	VTotalA = 0x6000C,
	VBlankA = 0x60010,
	VSyncA = 0x60014,
	PipeASource = 0x6001C,
	AnalogDisplayPort = 0x61100,
	VGADisplayPlaneControl = 0x71400,
	/* my stuff */
	NDE_RSTWRN_OPT = 0x46408,
	FUSE_STATUS = 0x42000,
	PWR_WELL_CTL = 0x45400, /* Display power control */
	CDCLK_CTL = 0x46000,	/* Change CD clock frequency */
	DPLL_CTRL1 = 0x6c058,	/* Control the DPLL mode, rate, and SSC */
	LCPLL1_CTL =
		0x46010, /* Enable DPLL0 for driving the display core clock (CDCLK), the
				   core display 2X clock (CD2XCLK), and the DDI ports */
	DBUF_CTL = 0x45008, /* DBUF Control */
	CUR_CTL = 0x70080,	/* Cursor Control */
	CUR_BASE = 0x70084, /* Cursor Base Address */
	CUR_POS = 0x70088,
	VGA_CONTROL = 0x41000,
};

}

} // namespace Graphics
