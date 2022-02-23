#include "screen.h"

ScreenManager::ScreenManager(unsigned int *framebuffer_base,
							 uint32_t screen_width, uint32_t screen_height) {
	framebuffer = framebuffer_base;
	width = screen_width;
	height = screen_height;
}

ScreenManager::~ScreenManager() {}

void ScreenManager::clearScreen(unsigned int target_color) {
	for (uint32_t i = 0; i < width * height; i++) {
		framebuffer[i] = target_color;
	}
}
