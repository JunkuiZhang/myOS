#include "screen.h"

ScreenManager *OS_SCREEN_MANAGER;

ScreenManager::ScreenManager(unsigned int *framebuffer_base,
							 uint32_t screen_width, uint32_t screen_height,
							 unsigned int color) {
	framebuffer = framebuffer_base;
	width = screen_width;
	height = screen_height;
	background_color = color;
	request_draw = false;
}

ScreenManager::ScreenManager() {}

ScreenManager::~ScreenManager() {}

void ScreenManager::update() {
	// request_draw = true;
}

void ScreenManager::clearScreen() {
	for (uint32_t i = 0; i < width * height; i++) {
		framebuffer[i] = background_color;
	}
}

void ScreenManager::clearScreen(unsigned int target_color) {
	for (uint32_t i = 0; i < width * height; i++) {
		framebuffer[i] = target_color;
	}
}
