#include "heap.h"
#include "pageframe_alloc.h"
#include "pagetable_manager.h"

void *heap_start;
void *heap_end;
HeapSegmentHeader *last_header;

void initializeHeap(void *heap_addr, size_t page_count) {
	void *pos = heap_addr;
	for (size_t x = 0; x < page_count; x++) {
		OS_PAGETABLE_MANAGER->mapMemory(pos,
										OS_PAGEFRAME_ALLOCATOR->requestPage());
		pos = (void *)((size_t)pos + 0x1000);
	}
	size_t heap_length = page_count * 0x1000;
	heap_start = heap_addr;
	heap_end = (void *)((size_t)heap_start + heap_length);
	HeapSegmentHeader *start_segment = (HeapSegmentHeader *)heap_addr;
	start_segment->length = heap_length - sizeof(HeapSegmentHeader);
	start_segment->next = NULL;
	start_segment->last = NULL;
	start_segment->free = true;
	last_header = start_segment;
}

void *malloc(size_t size) {
	if (size % 0x10) {
		size -= (size % 0x10);
		size += 0x10;
	}
	if (size == 0)
		return NULL;

	HeapSegmentHeader *current_segment = (HeapSegmentHeader *)heap_start;
	while (1) {
		if (current_segment->free) {
			if (current_segment->length > size) {
				current_segment->split(size);
				current_segment->free = false;
				return (void *)((uint64_t)current_segment +
								sizeof(HeapSegmentHeader));
			}
			if (current_segment->length == size) {
				current_segment->free = false;
				return (void *)((uint64_t)current_segment +
								sizeof(HeapSegmentHeader));
			}
		}
		if (current_segment->next == NULL)
			break;
		current_segment = current_segment->next;
	}
	expandHeap(size);
	return malloc(size);
}

void free(void *addr) {
	HeapSegmentHeader *segment = (HeapSegmentHeader *)addr - 1;
	segment->free = true;
	segment->combineForward();
	segment->combineBackward();
}

void expandHeap(size_t length) {
	if (length % 0x1000) {
		length -= (length % 0x1000);
		length += 0x1000;
	}
	size_t page_count = length / 0x1000;
	HeapSegmentHeader *new_segment = (HeapSegmentHeader *)heap_end;

	for (size_t x = 0; x < page_count; x++) {
		OS_PAGETABLE_MANAGER->mapMemory(heap_end,
										OS_PAGEFRAME_ALLOCATOR->requestPage());
		heap_end = (void *)((size_t)heap_end + 0x1000);
	}

	new_segment->free = true;
	new_segment->last = last_header;
	last_header->next = new_segment;
	last_header = new_segment;
	new_segment->next = NULL;
	new_segment->length = length - sizeof(HeapSegmentHeader);
	new_segment->combineBackward();
}

HeapSegmentHeader *HeapSegmentHeader::split(size_t split_length) {
	if (split_length < 0x10)
		return NULL;
	int64_t split_segment_length =
		length - split_length - sizeof(HeapSegmentHeader);
	if (split_segment_length < 0)
		return NULL;
	HeapSegmentHeader *new_split_header =
		(HeapSegmentHeader *)((size_t)this + split_length +
							  sizeof(HeapSegmentHeader));
	next->last = new_split_header;
	new_split_header->next = next;
	next = new_split_header;
	new_split_header->last = this;
	new_split_header->length = split_segment_length;
	new_split_header->free = free;
	length = split_length;

	if (last_header == this) {
		last_header = new_split_header;
	}
	return new_split_header;
}

void HeapSegmentHeader::combineForward() {
	if (next == NULL)
		return;
	if (!next->free)
		return;
	if (next == last_header)
		last_header = this;
	if (next->next != NULL)
		next->next->last = this;
	length = length + next->length + sizeof(HeapSegmentHeader);
	next = next->next;
}

void HeapSegmentHeader::combineBackward() {
	if ((last != NULL) && (last->free))
		last->combineForward();
}
