#pragma once
#include "../../lib/font/font_data.h"
#include "shell_utils.h"
#include <stdarg.h>

class Shell {
  private:
	FontStructure font_info;
	ShellSize size;
	PaddingPixel padding;
	unsigned int *buffer_base;
	ShellRowColInfo row_col_info;
	unsigned int font_foreground_color;
	unsigned int screen_background_color;

	char total_string_data[1024 * 4];
	uint32_t total_string_index;
	char temp_string_data[256];
	uint32_t temp_string_index;
	bool scroll_indicator;
	uint32_t row_pointer_list[128];
	uint32_t total_row_num;

	ShellCursor shell_cursor;

	unsigned char *matchChar(char c);
	void pushCol();
	void pushRow();
	void stringPreprocess(const char *str, va_list args);
	void charBufferPush(char new_char, char *buffer_pointer,
						uint32_t *buffer_index);
	void drawChar(char this_char, bool draw_background);
	void updateBufferAndDraw();

  public:
	Shell(unsigned int *bbase, unsigned int resolution_hor,
		  unsigned int resolution_ver, unsigned int foreground_color,
		  unsigned int background_colo);
	Shell();
	~Shell();

	void drawCursor();
	void print(const char *str, ...);
	void println(const char *str, ...);
	void putchar(char c);
	void deleteChar();

	void getShellInfo();
	void clearBuffer();
};

extern Shell *OS_SHELL;
