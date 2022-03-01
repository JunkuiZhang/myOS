#include "page_bitmap.h"

PageBitmap::PageBitmap(size_t m_size, uint8_t *m_buffer) {
	size = m_size;
	buffer = m_buffer;
	for (uint64_t x = 0; x < size; x++) {
		*(uint8_t *)(buffer + x) = 0;
	}
}

PageBitmap::PageBitmap() {
	size = 0;
	buffer = NULL;
}

void PageBitmap::init(size_t m_size, uint8_t *m_buffer) {
	size = m_size;
	buffer = m_buffer;
	for (uint64_t x = 0; x < size; x++) {
		*(uint8_t *)(buffer + x) = 0;
	}
}

bool PageBitmap::operator[](uint64_t index) { get(index); }

bool PageBitmap::get(uint64_t index) {
	if (index > size * 8)
		return false;
	uint64_t byte_index = index / 8;
	uint8_t bit_index = index % 8;
	uint8_t bit_index_mask = 0b10000000 >> bit_index;
	if (buffer[byte_index] & bit_index_mask) {
		return true;
	}
	return false;
}

bool PageBitmap::setBitmapValue(uint64_t index, bool value) {
	if (index > size * 8)
		return false;
	uint64_t byte_index = index / 8;
	uint8_t bit_index = index % 8;
	uint8_t bit_index_mask = 0b10000000 >> bit_index;
	if (value) {
		buffer[byte_index] = buffer[byte_index] | bit_index_mask;
	} else {
		buffer[byte_index] = buffer[byte_index] & (~bit_index_mask);
	}
	return true;
}

size_t PageBitmap::getSize() { return size; }

PageBitmap::~PageBitmap() {}
