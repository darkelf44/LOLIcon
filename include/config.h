#pragma once

struct SGlobalConfig
{
	// Colors
	uint32_t menu_color;
	uint32_t menu_background;
	uint32_t disabled_color;
	uint32_t disabled_background;
	uint32_t selected_color;
	uint32_t selected_background;
};
typedef struct SGlobalConfig GlobalConfig;

struct SProfileConfig
{
	// Identification
	char name[16];
	
	// Control settings
	bool enable_button_swap;
	bool enable_button_remap;
	uint8_t button_remap_table[16];
	bool disable_button_L3_R3;
	
	// Bluetooth settings
	uint8_t bluetooth_touch_mode;
	uint8_t bluetooth_touch_click;
	uint8_t bluetooth_motion_mode;

	// Overclock settings
	bool enable_overclock;
	uint8_t overclock_profile;
};
typedef struct SProfileConfig ProfileConfig;

// Globals
extern GlobalConfig  global_config;
extern ProfileConfig shell_config;
extern ProfileConfig default_config;
extern ProfileConfig * profile_config;

// Initilaize configs
void config_init(void);

// Manage global configuration
bool config_load_global();
bool config_save_global();
void config_reset_global();

// Manage local configuration
bool config_load_profile(ProfileConfig * config, const char * profile);
bool config_save_profile(ProfileConfig * config, const char * profile);
void config_reset_profile(ProfileConfig * config);
