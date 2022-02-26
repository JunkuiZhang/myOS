#pragma once
#include <stdint.h>

/* https://wiki.osdev.org/Paging */
struct PageDirectoryEntry {
	/* P, or 'Present'. If the bit is set, the page is actually in physical
	 * memory at the moment. For example, when a page is swapped out, it is not
	 * in physical memory and therefore not 'Present'. If a page is called, but
	 * not present, a page fault will occur, and the OS should handle it. */
	bool present : 1;
	/* If the bit is set, the page is read/write. Otherwise when it is not set,
	 * the page is read-only. */
	bool read_write : 1;
	/* Controls access to the page based on privilege level. If the bit is set,
	 * then the page may be accessed by all; if the bit is not set, however,
	 * only the supervisor can access it. */
	bool user_super : 1;
	/*  If the bit is set, write-through caching is enabled. If not, then
	 * write-back is enabled instead. */
	bool write_through : 1;
	/* If the bit is set, the page will not be cached. Otherwise, it will be. */
	bool cache_disabled : 1;
	/* Discover whether a PDE or PTE was read during virtual address
	 * translation. If it has, then the bit is set, otherwise, it is not. Note
	 * that, this bit will not be cleared by the CPU, so that burden falls on
	 * the OS.  */
	bool accessed : 1;
	bool ignore0 : 1;
	bool larger_pages : 1;
	bool ignore1 : 1;
	/* Reserved for OS to use. */
	uint8_t available : 3;
	uint64_t address : 52;
};

struct PageTable {
	PageDirectoryEntry entries[512];
} __attribute__((aligned(0x1000)));
