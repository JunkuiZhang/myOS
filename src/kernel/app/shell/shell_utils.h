#pragma once
#include <cstdint>

struct ShellSize {
	uint32_t width;
	uint32_t height;
	uint32_t total_pixel_num;
};

struct PaddingPixel {
	uint32_t leading;
	uint32_t trailing;
	uint32_t top;
	uint32_t bottom;
	uint32_t between;
};

struct ShellRowColInfo {
	uint32_t current_row;
	uint32_t current_col;
	uint32_t max_row;
	uint32_t max_col;
};

struct ShellCursor {
	uint32_t at_row;
	uint32_t at_col;
	uint32_t padding_vertical;
	uint32_t padding_left;
	uint32_t thickness;
};
