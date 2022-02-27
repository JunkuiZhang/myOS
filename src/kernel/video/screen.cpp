#include "screen.h"

ScreenManager *OS_SCREEN_MANAGER;

ScreenManager::ScreenManager(unsigned int *framebuffer_base,
							 uint32_t screen_width, uint32_t screen_height) {
	framebuffer = framebuffer_base;
	width = screen_width;
	height = screen_height;
}

ScreenManager::ScreenManager() {}

ScreenManager::~ScreenManager() {}

void ScreenManager::clearScreen(unsigned int target_color) {
	for (uint32_t i = 0; i < width * height; i++) {
		framebuffer[i] = target_color;
	}
}
