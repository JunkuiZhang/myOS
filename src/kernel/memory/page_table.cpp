#pragma once
#include "page_table.h"

void PageDirectoryEntry::setFlag(PageTableFlags flag, bool value) {
	uint64_t flag_mask = (uint64_t)1 << flag;
	if (value) {
		raw_data |= flag_mask;
	} else {
		raw_data &= (~flag_mask);
	}
}

bool PageDirectoryEntry::getFlag(PageTableFlags flag) {
	uint64_t flag_mask = (uint64_t)1 << flag;
	return raw_data & flag_mask > 0 ? true : false;
}

void PageDirectoryEntry::setAddress(uint64_t addr) {
	addr &= 0x000000ffffffffff;
	raw_data &= 0xfff0000000000fff;
	raw_data |= (addr << 12);
}

uint64_t PageDirectoryEntry::getAddress() {
	return (raw_data & 0x000ffffffffff000) >> 12;
}
