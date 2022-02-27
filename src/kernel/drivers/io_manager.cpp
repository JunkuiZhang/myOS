#include "io_manager.h"
#include "../app/shell/shell.h"
#include "../video/screen.h"
#include "keyboard.h"

IOHandlerManager *OS_IO_Manager;

IOHandlerManager::IOHandlerManager() { keyboard_info.upper_case = false; }

void IOHandlerManager::handleKeyboard(uint8_t scancode) {
	switch (scancode) {
	case left_shift:
		// if (keyboard_info.upper_case = false)
		keyboard_info.upper_case = true;
		return;
	case left_shift + 0x80: /* left shift is released */
							// if (keyboard_info.upper_case = true)
		keyboard_info.upper_case = false;
		return;
	case right_shift:
		// if (keyboard_info.upper_case = false)
		keyboard_info.upper_case = true;
		return;
	case right_shift + 0x80: /* left shift is released */
							 // if (keyboard_info.upper_case = true)
		keyboard_info.upper_case = false;
		return;
	case enter:
		OS_SHELL->putchar('\n');
		OS_SCREEN_MANAGER->request_draw = true;
		return;
	case space_bar:
		OS_SHELL->putchar(' ');
		OS_SCREEN_MANAGER->request_draw = true;
		return;
	case back_space:
		OS_SHELL->deleteChar();
		OS_SCREEN_MANAGER->request_draw = true;
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
	OS_SCREEN_MANAGER->request_draw = true;
}

IOHandlerManager::~IOHandlerManager() {}
