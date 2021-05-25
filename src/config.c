#include <common.h>

// Paths FIXME: Use these instead of the burnt in values
#define GLOBAL_CONFIG_DIR   "ur0:data/lolimenu"
#define GLOBAL_CONFIG_FILE  "ur0:data/lolimenu/global.cfg"
#define PROFILE_CONFIG_DIR  "ur0:data/lolimenu/profile"
#define PROFILE_CONFIG_FILE "ur0:data/lolimenu/profile/%s.cfg"

// Globals
GlobalConfig global_config = {0};
ProfileConfig shell_config;
ProfileConfig default_config;
ProfileConfig * profile_config = NULL;

// Static globals
static KMutex config_mutex = 0;
static char   config_string_buffer[0x100];

// String conversions
static uint32_t str_to_color(const char * text, uint32_t def);
static const char * color_to_str(uint32_t value);

// INI types
typedef bool (* ini_callback) (int type, const char * key, const char * value);

// INI constants
#define INI_TYPE_ENTRY 0
#define INI_TYPE_GROUP 1

// INI functions
static bool ini_read_file(const char * filename, ini_callback func);
static bool ini_global_config_callback(int type, const char * key, const char * value);
static bool ini_profile_config_callback(int type, const char * key, const char * value);

// Initialize configuration
void config_init(void)
{
	// Try loading global config
//	if (!config_load_global())
		config_reset_global();

	// Try loading default config
//	if (!config_load_profile(&default_config, "default"))
		config_reset_profile(&default_config);

	// Try loading shell_config
//	if (!config_load_profile(&shell_config, "main"))
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
	global_config.selected_background = 0x8800FF00;	// Green (transparent)
}

// Reset profile configuration
void config_reset_profile(ProfileConfig * config)
{
	// Reset controls
	config->enable_button_swap = false;
	config->enable_button_remap = false;
	config->disable_button_L3_R3 = false;

	// Reset bluetooth controls
	config->bluetooth_touch_mode = 0;
	config->bluetooth_touch_click = 0;
	config->bluetooth_motion_mode = 0;

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
	// Variables
	KFile * file;
	
	// Aquire mutex
	if (!kmutex_try_lock(&config_mutex))
		return false;
	
	// Create directory
	kmkdirs(GLOBAL_CONFIG_DIR, 0666);

	// Open file
	file = kfopen(GLOBAL_CONFIG_FILE, "wb");
	if (file)
	{
		// Text section
		kfputs(file, "[text]\n");
		kfprintf(file, "menu_color = %s\n", color_to_str(global_config.menu_color));
		kfprintf(file, "menu_background = %s\n", color_to_str(global_config.menu_background));
		kfprintf(file, "disabled_color = %s\n", color_to_str(global_config.disabled_color));
		kfprintf(file, "disabled_background = %s\n", color_to_str(global_config.disabled_background));
		kfprintf(file, "selected_color = %s\n", color_to_str(global_config.selected_color));
		kfprintf(file, "selected_background = %s\n", color_to_str(global_config.selected_background));
		kfputs(file, "\n");

		// Close file
		kfclose(file);
	}

	// Release mutex and return
	kmutex_unlock(&config_mutex);
	return !! file;
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
	// Variables
	KFile * file;
	
	// Aquire mutex
	if (!kmutex_try_lock(&config_mutex))
		return false;

	// Create directory
	kmkdirs(PROFILE_CONFIG_DIR, 0666);
	
	// Open file
	file = NULL; // kfopen(PATH_GLOBAL_CONFIG, "wb");
	if (!file)
	{
		// Close file
		kfclose(file);
	}

	// Release mutex and return
	kmutex_unlock(&config_mutex);
	return false;
}

// Reads an INI file, calling the callback function on each group and key-value pair
bool ini_read_file(const char * filename, ini_callback func)
{
	return false;
}

// Callback function for reading the global configuration
bool ini_global_config_callback(int type, const char * key, const char * value)
{
	return false;
}

// Callback function for reading profile configurations
bool ini_profile_config_callback(int type, const char * key, const char * value)
{
	return false;
}

// Convert digits to integers
static INLINE uint8_t digit_to_int(char digit)
	{return (digit >= 'a') ? (digit - 'a' + 10) : (digit >= 'A') ? (digit - 'A' + 10) : (digit - '0');}
// Convert integers to digits
static INLINE char int_to_digit(uint8_t value)
	{return ("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ")[value];}

// Converts HTML colors (with the "!" prefix instead of "#") to integers
static uint32_t str_to_color(const char * text, uint32_t def)
{
	// Variables
	size_t n;
	uint32_t color = 0xFF;

	// Check prefix
	if (* text ++ != '!')
		return def;

	// Get length, and check for hex digits
	for (n = 0; kishexch(text[n]); ++ n) {}
	if (text[n])
		return def;

	// Convert string
	switch (n)
	{
		// Short HTML colors (!RGB and !ARGB)
		case 4:
			color = digit_to_int(text[0]) * 0x11;
			++ text;
		case 3:
			color = (color << 8) | digit_to_int(text[0]) * 0x11;
			color = (color << 8) | digit_to_int(text[1]) * 0x11;
			color = (color << 8) | digit_to_int(text[2]) * 0x11;
			break;

		// Long HTML colors (!RRGGBB and !AARRGGBB)
		case 8:
			color = digit_to_int(text[0]);
			color = (color << 4) | digit_to_int(text[1]);
			text += 2;
		case 6:
			color = (color << 4) | digit_to_int(text[0]);
			color = (color << 4) | digit_to_int(text[1]);
			color = (color << 4) | digit_to_int(text[2]);
			color = (color << 4) | digit_to_int(text[3]);
			color = (color << 4) | digit_to_int(text[4]);
			color = (color << 4) | digit_to_int(text[5]);
			break;

		// Invalid format
		default:
			return def;
	}

	// Return result
	return color;
}

// Converts integers to HTML colors (with the "!" prefix instead of "#")
static const char * color_to_str(uint32_t value)
{
	// Variables
	static char buffer[10];
	char * result = buffer;

	// Add prefix
	*result ++ = '!';

	// Add alpha (when needed)
	if ((value & 0xFF000000) != 0xFF000000)
	{
		*result ++ = int_to_digit(value >> 28);
		*result ++ = int_to_digit((value >> 24) & 0xF);
	}

	// Convert number
	*result ++ = int_to_digit((value >> 20) & 0xF);
	*result ++ = int_to_digit((value >> 16) & 0xF);
	*result ++ = int_to_digit((value >> 12) & 0xF);
	*result ++ = int_to_digit((value >> 8) & 0xF);
	*result ++ = int_to_digit((value >> 4) & 0xF);
	*result ++ = int_to_digit(value & 0xF);
	*result ++ = 0;

	// Return result
	return buffer;
}

