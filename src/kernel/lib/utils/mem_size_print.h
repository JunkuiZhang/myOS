#pragma once
#include "file_size.h"

FileSize memorySizeFormatter(uint64_t size_bytes) {
	FileSize res;
	res.kilo_bytes = size_bytes / 1024;
	res.bytes = size_bytes % 1024;
	res.mega_bytes = res.kilo_bytes / 1024;
	res.kilo_bytes = res.kilo_bytes % 1024;
	return res;
}
