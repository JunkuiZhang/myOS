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
	unsigned int font_background_color;

	char total_string_data[1024 * 4];
	uint32_t total_string_index;
	bool scroll_indicator;
	uint32_t row_pointer_list[128];
	uint32_t total_row_num;
	uint32_t draw_string_pointer;

	unsigned char *matchChar(char c);
	void pushCol();
	void pushRow();
	void stringPreprocess(const char *str, va_list args);
	void charBufferPush(char new_char, char *buffer_pointer,
						uint32_t *buffer_index);

	void plainPrint();
	void drawChar(char this_char, bool draw_background);

  public:
	Shell(unsigned int *bbase, unsigned int resolution_hor,
		  unsigned int resolution_ver, unsigned int foreground_color,
		  unsigned int background_colo);
	Shell();
	// void init(unsigned int *bbase, unsigned int resolution_hor,
	// 		  unsigned int resolution_ver, unsigned int foreground_color,
	// 		  unsigned int background_colo);
	~Shell();

	void print(const char *str, ...);
	void putchar(char c);

	void getShellInfo();
};

extern Shell *OS_SHELL;
