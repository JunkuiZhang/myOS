#ifndef BOOT_PARAM_STRUCT
#define BOOT_PARAM_STRUCT
/**
 * Struct passed to the "kernel" from the exit_bs loader
 */
typedef struct {
	unsigned int *framebuffer;
	unsigned int width;
	unsigned int height;
} boot_param_t;
#endif
