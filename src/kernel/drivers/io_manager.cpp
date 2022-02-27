#include "io_manager.h"
#include "../app/shell/shell.h"
#include "keyboard.h"

IOHandlerManager *OS_IO_Manager;

IOHandlerManager::IOHandlerManager() { keyboard_info.upper_case = false; }

void IOHandlerManager::handleKeyboard(uint8_t scancode) {
	switch (scancode) {
	case left_shift:
		keyboard_info.upper_case = true;
		return;
	case left_shift + 0x80: /* left shift is released */
		keyboard_info.upper_case = false;
		return;
	case right_shift:
		keyboard_info.upper_case = true;
		return;
	case right_shift + 0x80: /* left shift is released */
		keyboard_info.upper_case = false;
		return;
	case enter:
		OS_SHELL->putchar('\n');
		return;
	case space_bar:
		OS_SHELL->putchar(' ');
		return;
	case back_space:
		OS_SHELL->print(" [Back space] ");
		return;
	}

	if (scancode > 58)
		return;
	char target_char = ASCII_TABLE[scancode];
	if (!target_char)
		return;
	if (keyboard_info.upper_case) {
		target_char -= 32;
	}
	OS_SHELL->putchar(target_char);
}

IOHandlerManager::~IOHandlerManager() {}
