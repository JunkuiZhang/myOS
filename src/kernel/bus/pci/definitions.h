#pragma once

namespace PCI {

enum RegisterOffset {
	VENDOR_ID = 0x00,			 // word
	DEVICE_ID = 0x02,			 // word
	COMMAND = 0x04,				 // word
	STATUS = 0x06,				 // word
	REVISION_ID = 0x08,			 // byte
	PROG_IF = 0x09,				 // byte
	SUBCLASS = 0x0a,			 // byte
	CLASS = 0x0b,				 // byte
	CACHE_LINE_SIZE = 0x0c,		 // byte
	LATENCY_TIMER = 0x0d,		 // byte
	HEADER_TYPE = 0x0e,			 // byte
	BIST = 0x0f,				 // byte
	BAR0 = 0x10,				 // u32
	BAR1 = 0x14,				 // u32
	BAR2 = 0x18,				 // u32
	SECONDARY_BUS = 0x19,		 // byte
	BAR3 = 0x1C,				 // u32
	BAR4 = 0x20,				 // u32
	BAR5 = 0x24,				 // u32
	SUBSYSTEM_VENDOR_ID = 0x2C,	 // u16
	SUBSYSTEM_ID = 0x2E,		 // u16
	CAPABILITIES_POINTER = 0x34, // u8
	INTERRUPT_LINE = 0x3C,		 // byte
	INTERRUPT_PIN = 0x3D,		 // byte
};

} // namespace PCI
