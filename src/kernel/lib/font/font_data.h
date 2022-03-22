#pragma once
#include <cstdint>

struct FontStructure {
	unsigned char *raw_data;
	uint32_t bytes_per_word;
	uint32_t font_width;
	uint32_t font_height;
};

extern unsigned char FONT_DATA[];
