#include "shell.h"

Shell::Shell(unsigned int *bbase, unsigned int resolution_hor,
			 unsigned int resolution_ver, unsigned int foreground_color,
			 unsigned int background_color) {
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
							   (font_info.font_height + 2 * padding.between) -
						   10;
	row_col_info.max_col = (shell_horizontal_pixel - padding.leading -
							padding.trailing - font_info.font_width) /
						   font_info.font_width;
	row_col_info.current_row = 0;
	row_col_info.current_col = 0;
	font_foreground_color = foreground_color;
	font_background_color = background_color;

	total_string_index = 0;
	total_row_num = 0;
	scroll_indicator = false;
	draw_string_pointer = 0;
}

Shell::~Shell() {}

unsigned char *Shell::matchChar(char c) {
	return &font_info.raw_data[((uint32_t)c - 0x20) * font_info.bytes_per_word];
}

void Shell::pushCol() {
	row_col_info.current_col += 1;
	if (row_col_info.current_col > row_col_info.max_col) {
		pushRow();
	}
}

void Shell::pushRow() {
	row_col_info.current_col = 0;
	row_col_info.current_row++;
	total_row_num++;
	row_pointer_list[total_row_num] = draw_string_pointer;
	if (row_col_info.current_row > row_col_info.max_row) {
		row_col_info.current_row--;
		// clear screen
		for (uint32_t i = 0; i < shell_vertical_pixel * shell_horizontal_pixel;
			 i++) {
			buffer_base[i] = 0xff002244;
		}
		draw_string_pointer =
			row_pointer_list[total_row_num - row_col_info.max_row];
		row_col_info.current_row = 0;
		row_col_info.current_col = 0;
		plainPrint();
	}
}

void Shell::charBufferPush(char new_char, char *buffer_pointer,
						   uint32_t *buffer_index) {
	buffer_pointer[(*buffer_index)] = new_char;
	(*buffer_index)++;
}

void Shell::stringPreprocess(const char *str, va_list args) {
	uint32_t string_index;
	char current_char;

	char temp_str_buffer[256];
	char temp_arg_buffer[20];
	uint32_t temp_str_index = 0;
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
				bool have_printed = false;
				while (left > 0 || bufferable > 0 || !have_printed) {
					temp_arg_buffer[indicator] = (char)(bufferable + 0x30);
					indicator += 1;
					bufferable = left % 10;
					left = left / 10;
					have_printed = true;
				}
				for (int x = indicator - 1; x >= 0; x--) {
					charBufferPush(temp_arg_buffer[x], temp_str_buffer,
								   &temp_str_index);
				}
				continue;
			}
			if (current_char == 'x') {
				int number = va_arg(args, int);
				int left = number / 16;
				int bufferable = number % 16;
				uint32_t indicator = 0;
				bool have_printed = false;
				while (left > 0 || bufferable > 0 || !have_printed) {
					if (bufferable < 10) {
						temp_arg_buffer[indicator] = (char)(bufferable + 0x30);
					} else {
						temp_arg_buffer[indicator] =
							(char)(0x41 + bufferable - 10);
					}
					indicator += 1;
					bufferable = left % 16;
					left = left / 16;
					have_printed = true;
				}
				for (int x = indicator - 1; x >= 0; x--) {
					charBufferPush(temp_arg_buffer[x], temp_str_buffer,
								   &temp_str_index);
				}
				continue;
			}
			if (current_char == 's') {
				uint32_t indicator = 0;
				char *temp_string;
				temp_string = va_arg(args, char *);
				while (temp_string[indicator] != 0) {
					charBufferPush(temp_string[indicator], temp_str_buffer,
								   &temp_str_index);
					indicator++;
				}
				continue;
			}
		} else {
			charBufferPush(current_char, temp_str_buffer, &temp_str_index);
		}
	}
	for (uint32_t x = 0; x < temp_str_index; x++) {
		charBufferPush(temp_str_buffer[x], total_string_data,
					   &total_string_index);
	}
}

void Shell::print(const char *str, ...) {
	va_list args;
	va_start(args, str);
	stringPreprocess(str, args);
	va_end(args);
	plainPrint();
}

void Shell::plainPrint() {
	while (1) {
		char current_char = total_string_data[draw_string_pointer];
		if (draw_string_pointer >= total_string_index) {
			break;
		}
		putchar(current_char);
	}
}

void Shell::putchar(char c) {
	if (c == ' ') {
		draw_string_pointer++;
		pushCol();
		return;
	}
	if (c == '\r' || c == '\n') {
		draw_string_pointer++;
		pushRow();
		return;
	}
	drawChar(c, false);
	return;
}

void Shell::drawChar(char this_char, bool draw_background) {
	// pixel before current row
	uint32_t pixel_offset = (row_col_info.current_row *
								 (font_info.font_height + 2 * padding.between) +
							 padding.top) *
								shell_horizontal_pixel +
							row_col_info.current_col * font_info.font_width +
							padding.leading;

	auto char_data = matchChar(this_char);
	for (uint32_t byte_index = 0; byte_index < font_info.bytes_per_word;
		 byte_index++) {
		for (uint32_t bit_index = 0; bit_index < 8; bit_index++) {
			unsigned char bit_mask = 0x80 >> bit_index;
			uint32_t temp_offset;
			uint32_t bit_row =
				((8 * byte_index) + bit_index) / font_info.font_width;
			uint32_t bit_col =
				((8 * byte_index) + bit_index) % font_info.font_width;
			temp_offset = shell_horizontal_pixel * bit_row + bit_col;
			temp_offset += pixel_offset;
			if (char_data[byte_index] & bit_mask) {
				*((unsigned int *)(buffer_base + temp_offset)) =
					font_foreground_color;
			} else if (draw_background) {
				*((unsigned int *)(buffer_base + temp_offset)) =
					font_background_color;
			}
		}
	}
	draw_string_pointer++;
	pushCol();
	return;
}

void Shell::getShellInfo() {
	print("Shell row: %d, col: %d\n", row_col_info.max_row,
		  row_col_info.max_col);
}
