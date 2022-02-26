#pragma once
#include "page_table.h"
#include "pageframe_alloc.h"

class PageTableManager {
  private:
	/* data */
  public:
	PageTable *pml4_addr;

	PageTableManager(PageTable *PML4_address); /* page map level 4 */
	~PageTableManager();

	void mapMemory(void *virtual_memory, void *physical_memory,
				   PageFrameAllocator *os_allocator);
};
