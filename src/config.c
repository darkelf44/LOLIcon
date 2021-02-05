#include <common.h>

// Paths
#define PREFIX "ur0:/lolimenu/"
#define PATH_GLOBAL_CONFIG PREFIX "global.cfg"
#define PATH_PROFILE_CONFIG PREFIX "profile/%s.cfg"

// Globals
GlobalConfig global_config = {0};
ProfileConfig shell_config;
ProfileConfig default_config;
ProfileConfig * profile_config = NULL;


// Profile config cache
static ProfileConfig profile_config_cache[8];

void config_init(void)
{
	// Try loading global config
	if (!config_load_global())
		config_reset_global();

	// Try loading default config
	if (!config_load_profile(&default_config, "default"))
		config_reset_profile(&default_config);

	// Try loading shell_config
	if (!config_load_profile(&shell_config, "main"))
		shell_config = default_config;

	// Set the current config to the default one
	profile_config = &default_config;
}

// Reset global configuration
void config_reset_global()
{
	// Reset colors
	global_config.menu_color          = 0xFFFFFFFF;	// White
	global_config.menu_background     = 0x44000000;	// Black (transparent)
	global_config.disabled_color      = 0xFF888888;	// Grey
	global_config.disabled_background = 0x44000000;	// Black (transparent)
	global_config.selected_color      = 0xFFFFFFFF;	// White
	global_config.selected_background = 0x4400FF00;	// Green (transparent)
}

// Reset profile configuration
void config_reset_profile(ProfileConfig * config)
{
	// Reset controls
	config->enable_button_swap = false;
	config->enable_button_remap = false;

	// Reset bluetooth controls
	config->bluetooth_touch_mode = false;
	config->bluetooth_motion_mode = false;

	// Reset overclock
	config->enable_overclock = false;
	config->overclock_profile = 0;
}

// Load global configuration
bool config_load_global()
{
	// FIXME: Not implemented
	return false;
}

// Save global configuration
bool config_save_global()
{
	// FIXME: Not implemented
	return false;
}

// Load profile configuration
bool config_load_profile(ProfileConfig * config, const char * profile)
{
	// FIXME: Not implemented
	return false;
}

// Save profile configuration
bool config_save_profile(ProfileConfig * config, const char * profile)
{
	// FIXME: Not implemented
	return false;
}
