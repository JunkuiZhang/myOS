#pragma once
#include <stddef.h>
#include <stdint.h>

struct HeapSegmentHeader {
	size_t length;
	HeapSegmentHeader *next;
	HeapSegmentHeader *last;
	bool free;
	void combineForward();
	void combineBackward();
	HeapSegmentHeader *split(size_t split_length);
};

void initializeHeap(void *heap_addr, size_t page_count);
void *malloc(size_t size);
void free(void *addr);
void expandHeap(size_t length);

inline void *operator new(size_t size) { return malloc(size); }
inline void *operator new[](size_t size) { return malloc(size); }
inline void operator delete(void *ptr) { return free(ptr); }
