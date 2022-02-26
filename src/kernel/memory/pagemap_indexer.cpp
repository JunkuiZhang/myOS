#include "pagemap_indexer.h"

PageMapIndexer::PageMapIndexer(uint64_t virtual_address) {
	virtual_address >>= 12;
	page_index = virtual_address & 0x1ff;
	virtual_address >>= 9;
	page_table_index = virtual_address & 0x1ff;
	virtual_address >>= 9;
	page_directory_index = virtual_address & 0x1ff;
	virtual_address >>= 9;
	page_directory_pointer_index = virtual_address & 0x1ff;
}

PageMapIndexer::~PageMapIndexer() {}
