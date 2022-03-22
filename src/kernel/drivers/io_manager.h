#pragma once
#include "../lib/utils/keyboard_case.h"
#include <cstdint>

class IOHandlerManager {
  private:
	KeyboardPressedInfo keyboard_info;

  public:
	IOHandlerManager();
	~IOHandlerManager();

	void handleKeyboard(uint8_t scancode);
};

extern IOHandlerManager *OS_IO_Manager;
