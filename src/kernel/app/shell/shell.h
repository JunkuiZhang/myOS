#pragma once

#include "../../lib/font/font_data.h"
#include <stdarg.h>
#include <stdint.h>

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

class Shell {
  private:
	FontStructure font_info;
	uint32_t shell_horizontal_pixel;
	PaddingPixel padding;
	uint32_t shell_vertical_pixel;
	unsigned int *buffer_base;
	ShellRowColInfo row_col_info;
	unsigned int font_foreground_color;

	char string_buffer[256];
	uint32_t string_buffer_index;

	unsigned char *matchChar(char c);
	void checkBounds();
	void stringPreprocess(const char *str, va_list args);
	void stringBufferPush(char new_char);

  public:
	Shell(unsigned int *bbase, unsigned int resolution_hor,
		  unsigned int resolution_ver);
	~Shell();

	void print(const char *str, ...);
	void setPosition(uint32_t row, uint32_t col);
	void putchar(char c);
};
