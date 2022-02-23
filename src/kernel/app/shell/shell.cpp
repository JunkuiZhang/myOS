#include "shell.h"

Shell::Shell(unsigned int *bbase, unsigned int resolution_hor,
			 unsigned int resolution_ver) {
	font_info.raw_data = FONT_DATA;
	font_info.bytes_per_word = 24;
	font_info.font_width = 12;
	font_info.font_height = 16;
	shell_horizontal_pixel = (uint32_t)resolution_hor;
	padding = PaddingPixel{15, 15, 12, 12, 3};
	shell_vertical_pixel = (uint32_t)resolution_ver;
	buffer_base = bbase;
	row_col_info.max_row = (shell_vertical_pixel - padding.top -
							padding.bottom - font_info.font_height) /
						   (font_info.font_height + 2 * padding.between);
	row_col_info.max_col = (shell_horizontal_pixel - padding.leading -
							padding.trailing - font_info.font_width) /
						   font_info.font_width;
	row_col_info.current_row = 0;
	row_col_info.current_col = 0;
	font_foreground_color = 0xffffffff;

	for (uint32_t i = 0; i < 256; i++) {
		string_buffer[i] = 0;
	}
	string_buffer_index = 0;
}

Shell::~Shell() {}

unsigned char *Shell::matchChar(char c) {
	return &font_info.raw_data[((uint32_t)c - 0x20) * font_info.bytes_per_word];
}

void Shell::checkBounds() {
	if (row_col_info.current_row > row_col_info.max_row) {
		row_col_info.current_row = 0;
		row_col_info.current_col += 1;
	}
	if (row_col_info.current_col > row_col_info.max_col) {
		row_col_info.current_col = 0;
		row_col_info.current_row += 1;
	}
}

void Shell::stringBufferPush(char new_char) {
	string_buffer[string_buffer_index] = new_char;
	string_buffer_index++;
}

void Shell::stringPreprocess(const char *str, va_list args) {
	uint32_t string_index;
	char current_char;
	char temp_str_buffer[20];
	for (string_index = 0;; string_index++) {
		current_char = str[string_index];
		if (current_char == 0) {
			break;
		}
		if (current_char == '%') {
			string_index++;
			current_char = str[string_index];
			if (current_char == '%') {
				string_index++;
				continue;
			}
			if (current_char == 'd') {
				int number = va_arg(args, int);
				int left = number / 10;
				int bufferable = number % 10;
				uint32_t indicator = 0;
				while (left > 0 || bufferable > 0) {
					temp_str_buffer[indicator] = (char)(bufferable + 0x30);
					indicator += 1;
					bufferable = left % 10;
					left = left / 10;
				}
				for (int x = indicator - 1; x >= 0; x--) {
					stringBufferPush(temp_str_buffer[x]);
				}
				continue;
			}
			if (current_char == 'x') {
				int number = va_arg(args, int);
				int left = number / 16;
				int bufferable = number % 16;
				uint32_t indicator = 0;
				while (left > 0 || bufferable > 0) {
					if (bufferable < 10) {
						temp_str_buffer[indicator] = (char)(bufferable + 0x30);
					} else {
						temp_str_buffer[indicator] =
							(char)(0x41 + bufferable - 10);
					}
					indicator += 1;
					bufferable = left % 16;
					left = left / 16;
				}
				for (int x = indicator - 1; x >= 0; x--) {
					stringBufferPush(temp_str_buffer[x]);
				}
				continue;
			}
			if (current_char == 's') {
				uint32_t temp_string_index = 0;
				char *temp_string;
				temp_string = va_arg(args, char *);
				while (temp_string[temp_string_index] != 0) {
					stringBufferPush(temp_string[temp_string_index]);
					temp_string_index += 1;
				}
				continue;
			}
		} else {
			stringBufferPush(current_char);
		}
	}
}

void Shell::print(const char *str, ...) {
	va_list args;
	va_start(args, str);
	stringPreprocess(str, args);
	va_end(args);
	uint32_t total_num = string_buffer_index;
	for (uint32_t index = 0; index < total_num; index++) {
		char current_char = string_buffer[index];
		string_buffer[index] = 0;
		this->putchar(current_char);
	}
	string_buffer_index = 0;
}

void Shell::putchar(char c) {
	if (c == ' ') {
		row_col_info.current_col += 1;
		checkBounds();
		return;
	}
	if (c == '\r' || c == '\n') {
		row_col_info.current_row += 1;
		row_col_info.current_col = 0;
		return;
	}
	// pixel before current row
	uint32_t pixel_offset = (row_col_info.current_row *
								 (font_info.font_height + 2 * padding.between) +
							 padding.top) *
								shell_horizontal_pixel +
							row_col_info.current_col * font_info.font_width +
							padding.leading;

	auto char_data = matchChar(c);
	for (uint32_t byte_index = 0; byte_index < font_info.bytes_per_word;
		 byte_index++) {
		for (uint32_t bit_index = 0; bit_index < 8; bit_index++) {
			unsigned char bit_mask = 0x80 >> bit_index;
			uint32_t temp_offset;
			uint32_t bit_row =
				((8 * byte_index) + bit_index) / font_info.font_width;
			uint32_t bit_col =
				((8 * byte_index) + bit_index) % font_info.font_width;
			if (char_data[byte_index] & bit_mask) {
				temp_offset = shell_horizontal_pixel * bit_row + bit_col;
				temp_offset += pixel_offset;
				*((unsigned int *)(buffer_base + temp_offset)) =
					font_foreground_color;
			}
		}
	}
	row_col_info.current_col += 1;
	checkBounds();
	return;
}
void Shell::getShellInfo() {
	print("Shell row: %d, col: %d\n", row_col_info.max_row,
		  row_col_info.max_col);
}
