#pragma once

#include <cstdint>

class ScreenManager {
  private:
	unsigned int *framebuffer;
	uint32_t width;
	uint32_t height;
	unsigned int background_color;

  public:
	bool request_draw;

	ScreenManager(unsigned int *framebuffer_base, uint32_t screen_width,
				  uint32_t screen_height, unsigned int color);
	ScreenManager();
	~ScreenManager();

	void update();
	void clearScreen();
	void clearScreen(unsigned int target_color);
};

extern ScreenManager *OS_SCREEN_MANAGER;
