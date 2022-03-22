#pragma once
#include <cstdint>

enum PageTableFlags {
	/* P, or 'Present'. If the bit is set, the page is actually in physical
	 * memory at the moment. For example, when a page is swapped out, it is not
	 * in physical memory and therefore not 'Present'. If a page is called, but
	 * not present, a page fault will occur, and the OS should handle it. */
	present = 0,
	/* If the bit is set, the page is read/write. Otherwise when it is not set,
	 * the page is read-only. */
	read_write = 1,
	/* Controls access to the page based on privilege level. If the bit is set,
	 * then the page may be accessed by all; if the bit is not set, however,
	 * only the supervisor can access it. */
	user_super = 2,
	/*  If the bit is set, write-through caching is enabled. If not, then
	 * write-back is enabled instead. */
	write_through = 3,
	/* If the bit is set, the page will not be cached. Otherwise, it will be. */
	cache_disabled = 4,
	/* Discover whether a PDE or PTE was read during virtual address
	 * translation. If it has, then the bit is set, otherwise, it is not. Note
	 * that, this bit will not be cleared by the CPU, so that burden falls on
	 * the OS.  */
	accessed = 5,
	larger_pages = 7,
	/* Reserved for OS to use. */
	custom0 = 9,
	custom1 = 10,
	custom2 = 11,
	/* not excutable, only available on some system */
	NX = 63,
};

/* https://wiki.osdev.org/Paging */
struct PageDirectoryEntry {
	uint64_t raw_data;

	void setFlag(PageTableFlags flag, bool value);
	bool getFlag(PageTableFlags flag);
	void setAddress(uint64_t addr);
	uint64_t getAddress();
};

struct PageTable {
	PageDirectoryEntry entries[512];
} __attribute__((aligned(0x1000)));
