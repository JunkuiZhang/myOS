#include "memory_utils.h"

void memset(void *start, uint8_t value, uint64_t num) {
	for (uint64_t x = 0; x < num; x++) {
		*(uint8_t *)((uint64_t)start + x) = value;
	}
}
