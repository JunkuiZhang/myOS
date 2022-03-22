#pragma once

#include <cstddef>
#include <cstdint>

class PageBitmap {
  private:
	size_t size;

  public:
	uint8_t *buffer;

	PageBitmap(size_t m_size, uint8_t *m_buffer);
	PageBitmap();
	~PageBitmap();

	void init(size_t m_size, uint8_t *m_buffer);
	bool operator[](uint64_t index);
	bool get(uint64_t index);
	bool setBitmapValue(uint64_t index, bool value);
	size_t getSize();
};
