#include <common.h>

//  Initialize module
void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp)
{
	// Initialize subsystems
	klib_init();
	config_init();
	display_init();
	menu_init();

	// Install hooks
	hooks_install();

	// Return
	return SCE_KERNEL_START_SUCCESS;
}

// Finalize module
int module_stop(SceSize argc, const void *args)
{
	// Finalize subsystems
	klib_done();

	// Uninstall hooks
	hooks_uninstall();

	// Return
	return SCE_KERNEL_STOP_SUCCESS;
}
