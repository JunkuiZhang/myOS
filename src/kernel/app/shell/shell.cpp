#include "shell.h"

Shell *OS_SHELL;

Shell::Shell(unsigned int *bbase, unsigned int resolution_hor,
			 unsigned int resolution_ver, unsigned int foreground_color,
			 unsigned int background_color) {
	font_info.raw_data = FONT_DATA;
	font_info.bytes_per_word = 24;
	font_info.font_width = 12;
	font_info.font_height = 16;
	size.width = (uint32_t)resolution_hor;
	size.height = (uint32_t)resolution_ver;
	size.total_pixel_num = size.width * size.height;
	padding = PaddingPixel{15, 15, 12, 12, 3};
	buffer_base = bbase;
	row_col_info.max_row =
		(size.height - padding.top - padding.bottom - font_info.font_height) /
		(font_info.font_height + 2 * padding.between);
	row_col_info.max_col = (size.width - padding.leading - padding.trailing -
							font_info.font_width) /
						   font_info.font_width;
	row_col_info.current_row = 0;
	row_col_info.current_col = 0;
	font_foreground_color = foreground_color;
	font_background_color = background_color;

	shell_cursor.at_row = 0;
	shell_cursor.at_col = 0;

	total_string_index = 0;
	total_row_num = 0;
	scroll_indicator = false;
	draw_string_pointer = 0;
	temp_string_index = 0;
	row_pointer_list[0] = 0;
}

Shell::Shell() {}

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
		for (uint32_t i = 0; i < size.total_pixel_num; i++) {
			buffer_base[i] = 0xff002244;
		}
		draw_string_pointer =
			row_pointer_list[total_row_num - row_col_info.max_row];
		row_col_info.current_row = 0;
		row_col_info.current_col = 0;
		/* draw from here */
		for (uint32_t x = draw_string_pointer; x < total_string_index; x++) {
			char current = total_string_data[x];
			drawChar(current, true);
		}
	}
}

void Shell::getCharIndex(uint32_t row, uint32_t col, uint32_t *result) {
	if (row < 0 || row > row_col_info.max_row || col < 0 ||
		col > row_col_info.max_col)
		return;
	*result = row_pointer_list[row] + col;
}

void Shell::charBufferPush(char new_char, char *buffer_pointer,
						   uint32_t *buffer_index) {
	buffer_pointer[(*buffer_index)] = new_char;
	(*buffer_index)++;
}

void Shell::stringPreprocess(const char *str, va_list args) {
	uint32_t string_index;
	char current_char;

	char temp_arg_buffer[20];
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
					charBufferPush(temp_arg_buffer[x], temp_string_data,
								   &temp_string_index);
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
					charBufferPush(temp_arg_buffer[x], temp_string_data,
								   &temp_string_index);
				}
				continue;
			}
			if (current_char == 's') {
				uint32_t indicator = 0;
				char *temp_string;
				temp_string = va_arg(args, char *);
				while (temp_string[indicator] != 0) {
					charBufferPush(temp_string[indicator], temp_string_data,
								   &temp_string_index);
					indicator++;
				}
				continue;
			}
		} else {
			charBufferPush(current_char, temp_string_data, &temp_string_index);
		}
	}
	for (uint32_t x = 0; x < temp_string_index; x++) {
		charBufferPush(temp_string_data[x], total_string_data,
					   &total_string_index);
	}
}

void Shell::print(const char *str, ...) {
	va_list args;
	va_start(args, str);
	stringPreprocess(str, args);
	va_end(args);
	for (uint32_t x = 0; x < temp_string_index; x++) {
		char current_char = temp_string_data[x];
		putchar(current_char);
	}
	temp_string_index = 0;
}

void Shell::println(const char *str, ...) {
	va_list args;
	va_start(args, str);
	stringPreprocess(str, args);
	va_end(args);
	for (uint32_t x = 0; x < temp_string_index; x++) {
		char current_char = temp_string_data[x];
		putchar(current_char);
	}
	temp_string_index = 0;
	putchar('\n');
}

void Shell::plainPrint() {
	for (uint32_t x = 0; x < temp_string_index; x++) {
		char current_char = temp_string_data[x];
		putchar(current_char);
	}
	temp_string_index = 0;
}

void Shell::putchar(char c) {
	charBufferPush(c, total_string_data, &total_string_index);
	drawChar(c, false);
}

void Shell::drawChar(char this_char, bool draw_background) {
	if (this_char == ' ') {
		draw_string_pointer++;
		pushCol();
		return;
	}
	if (this_char == '\r' || this_char == '\n') {
		draw_string_pointer++;
		pushRow();
		return;
	}
	// pixel before current row
	uint32_t pixel_offset = (row_col_info.current_row *
								 (font_info.font_height + 2 * padding.between) +
							 padding.top) *
								size.width +
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
			temp_offset = size.width * bit_row + bit_col;
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

void Shell::deleteChar(uint32_t row, uint32_t col) {
	if (row < 0 || row > row_col_info.max_row)
		return;
	uint32_t total_size = total_string_index - 1;
	uint32_t *char_index = nullptr;
	getCharIndex(row, col, char_index);
	if (char_index == nullptr)
		return;
	row_col_info.current_col = col;
	row_col_info.current_row = row;
	draw_string_pointer = *char_index;
	total_string_index = *char_index;
	for (uint32_t x = *char_index; x < total_size; x++) {
		char next_char = total_string_data[x + 1];
		charBufferPush(next_char, total_string_data, &total_string_index);
		drawChar(next_char, true);
	}
}
