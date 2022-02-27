#include "panic.h"
#include "../app/shell/shell.h"
#include "../video/screen.h"

void panic(char *panic_message) {
	OS_SCREEN_MANAGER->clearScreen(0xff551111);
	OS_SHELL->print("Kernel panic! ERROR: %s", panic_message);
}
