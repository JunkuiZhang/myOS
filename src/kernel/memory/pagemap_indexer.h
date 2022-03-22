#pragma once
#include <cstdint>

class PageMapIndexer {
  private:
	/* data */
  public:
	uint64_t page_directory_pointer_index;
	uint64_t page_directory_index;
	uint64_t page_table_index;
	uint64_t page_index;

	PageMapIndexer(uint64_t virtual_address);
	~PageMapIndexer();
};
