#include "pagetable_manager.h"
#include "memory_utils.h"
#include "pagemap_indexer.h"

PageTableManager::PageTableManager(PageTable *PML4_address) {
	pml4_addr = PML4_address;
}

PageTableManager::~PageTableManager() {}

void PageTableManager::mapMemory(void *virtual_memory, void *physical_memory,
								 PageFrameAllocator *os_allocator) {
	PageMapIndexer pagemap_indexer = PageMapIndexer((uint64_t)virtual_memory);

	PageDirectoryEntry pd_entry;
	pd_entry = pml4_addr->entries[pagemap_indexer.page_directory_pointer_index];
	PageTable *page_directory_pointer;
	if (!pd_entry.getFlag(PageTableFlags::present)) {
		page_directory_pointer = (PageTable *)os_allocator->requestPage();
		memset(page_directory_pointer, 0, 4096);
		pd_entry.setAddress((uint64_t)page_directory_pointer >> 12);
		pd_entry.setFlag(PageTableFlags::present, true);
		pd_entry.setFlag(PageTableFlags::read_write, true);
		pml4_addr->entries[pagemap_indexer.page_directory_pointer_index] =
			pd_entry;
	} else {
		page_directory_pointer =
			(PageTable *)((uint64_t)pd_entry.getAddress() << 12);
	}

	pd_entry =
		page_directory_pointer->entries[pagemap_indexer.page_directory_index];
	PageTable *page_directory;
	if (!pd_entry.getFlag(PageTableFlags::present)) {
		page_directory = (PageTable *)os_allocator->requestPage();
		memset(page_directory, 0, 4096);
		pd_entry.setAddress((uint64_t)page_directory >> 12);
		pd_entry.setFlag(PageTableFlags::present, true);
		pd_entry.setFlag(PageTableFlags::read_write, true);
		page_directory_pointer->entries[pagemap_indexer.page_directory_index] =
			pd_entry;
	} else {
		page_directory = (PageTable *)((uint64_t)pd_entry.getAddress() << 12);
	}

	pd_entry = page_directory->entries[pagemap_indexer.page_table_index];
	PageTable *page_table;
	if (!pd_entry.getFlag(PageTableFlags::present)) {
		page_table = (PageTable *)os_allocator->requestPage();
		memset(page_table, 0, 4096);
		pd_entry.setAddress((uint64_t)page_table >> 12);
		pd_entry.setFlag(PageTableFlags::present, true);
		pd_entry.setFlag(PageTableFlags::read_write, true);
		page_directory->entries[pagemap_indexer.page_table_index] = pd_entry;
	} else {
		page_table = (PageTable *)((uint64_t)pd_entry.getAddress() << 12);
	}

	pd_entry = page_table->entries[pagemap_indexer.page_index];
	pd_entry.setAddress((uint64_t)physical_memory >> 12);
	pd_entry.setFlag(PageTableFlags::present, true);
	pd_entry.setFlag(PageTableFlags::read_write, true);

	page_table->entries[pagemap_indexer.page_index] = pd_entry;
}
