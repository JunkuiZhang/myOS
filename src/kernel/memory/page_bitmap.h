#pragma once

#include <stddef.h>
#include <stdint.h>

class PageBitmap {
  private:
	size_t size;
	uint8_t *buffer;

  public:
	PageBitmap(size_t m_size, uint8_t *m_buffer);
	PageBitmap();
	~PageBitmap();

	void init(size_t m_size, uint8_t *m_buffer);
	bool operator[](uint64_t index);
	bool setBitmapValue(uint64_t index, bool value);
	size_t getSize();
};
