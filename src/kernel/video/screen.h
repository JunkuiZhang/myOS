#pragma once

#include <stdint.h>

class ScreenManager {
  private:
	unsigned int *framebuffer;
	uint32_t width;
	uint32_t height;

  public:
	ScreenManager(unsigned int *framebuffer_base, uint32_t screen_width,
				  uint32_t screen_height);
	~ScreenManager();

	void clearScreen(unsigned int target_color);
};
