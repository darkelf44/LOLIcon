// Include common headers
#include <common.h>

// Colors
#define FGCOLOR 0xFFFFFFFF
#define BGCOLOR 0x55000000
#define FGSELECT 0xFFFFFFFF
#define BGSELECT 0xFF00FF00

// Global variables
Menu menu = {0};

// Initialize or reset the menu
void menu_init(void)
{
	// Not visible
	menu.visible = false;
	menu.capture = false;

	// Display main menu
	menu_page_main.selected = 0;
	menu.page = &menu_page_main;
}

// Open the menu
void menu_open(void)
{
	// Make menu visible
	menu.visible = true;
	menu.capture = true;

	// Display main menu
	menu_page_main.selected = 0;
	menu.page = &menu_page_main;
}

// Close the menu
void menu_close(void)
{
	// Make menu invisible
	menu.visible = false;
	menu.capture = false;
}

// Center and draw menu title
static void menu_draw_title(const char * title)
{
	// Variables
	size_t n = strlen(title);
	uint16_t dx = display.font->char_width;
	uint16_t dy = display.font->char_height;

	// Center title
	display.text_x = (display.framebuffer.width - n * dx) / 2;
	display.text_y = 5 * dy;

	// Draw title
	display_draw_text(title, n, FGCOLOR, BGCOLOR);

	// Skip some lines
	display.text_y += 2 * dy;
}

// Draw a simple static menu entry
static void menu_draw_entry(int16_t index, const char * text, int16_t align)
{
	// Variables
	size_t n = strlen(text);
	uint16_t dx = display.font->char_width;
	uint32_t fgcolor, bgcolor;

	// Align text vertically
	if (align > 0)
		display.text_x = (display.framebuffer.width / 2) - align * dx;
	else if (align < 0)
		display.text_x = (display.framebuffer.width / 2) - align * dx + - n * dx;
	else
		display.text_x = (display.framebuffer.width - n * dx) / 2;
		
	// Set colors
	if (index == menu.page->selected)
	{
		fgcolor = global_config.selected_color;
		bgcolor = global_config.selected_background;
	}
	else if (index < 0)
	{
		fgcolor = global_config.disabled_color;
		bgcolor = global_config.disabled_background;
	}
	else
	{
		fgcolor = global_config.menu_color;
		bgcolor = global_config.menu_background;
	}

	// Draw entry
	display_draw_text(text, n, fgcolor, bgcolor);
}

static INLINE void menu_draw_entry_ln(int16_t index, const char * text, int16_t align)
{
	// Draw the entry
	menu_draw_entry(index, text, align);

	// Start new line
	display.text_y += display.font->char_height;
}

// Converts a boolean to a toggle for drawing
static INLINE const char * to_toggle(bool value)
{
	return value ? ICON_ON : ICON_OFF;
}

// Converts an integer to a slider for drawing
static INLINE const char * to_slider(int16_t value, int16_t max)
{
	return "";
}


// Main Menu page

static void menu_main_draw(void)
{
	menu_draw_title(ICON_LOLI "Menu" " [indev]");

	menu_draw_entry_ln(0, "Global settings", 10);
	menu_draw_entry_ln(1, "Control settings", 10);
	menu_draw_entry_ln(2, "Overclock settings", 10);
	menu_draw_entry_ln(3, "Information and Tools", 10);
	menu_draw_entry_ln(4, "Save profile", 10);
	menu_draw_entry_ln(5, "Save profile as default", 10);
	menu_draw_entry_ln(6, "Reset profile", 10);
	menu_draw_entry_ln(7, "Exit game", 10);
	display.text_y += 8;
	menu_draw_entry_ln(8, ICON_SLEEP " Suspend vita", 8);
	menu_draw_entry_ln(9, ICON_RESTART " Restart vita", 8);
	menu_draw_entry_ln(10, ICON_POWER " Shutdown vita", 8);
}

void menu_main_input(uint32_t pressed)
{
	// Contants
	static const int count = 11;

	// Navigate
	if ((pressed & SCE_CTRL_UP) && menu.page->selected > 0)
		-- menu.page->selected;
	if ((pressed & SCE_CTRL_DOWN) && menu.page->selected < count -1)
		++ menu.page->selected;

	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.page->selected = 0;
	// Accept
	if (pressed & SCE_CTRL_CROSS)
	{
		switch (menu.page->selected)
		{
			case 0:
				// Switch to "Global settings" page
				menu_page_global_settings.selected = 0;
				menu_page_global_settings.previous = menu.page;
				menu.page = &menu_page_global_settings;
				break;

			case 1:
				// Switch to "Global settings" page
				menu_page_control_settings.selected = 0;
				menu_page_control_settings.previous = menu.page;
				menu.page = &menu_page_control_settings;
				break;

			case 2:
				// Switch to "Global settings" page
				menu_page_overclock_settings.selected = 0;
				menu_page_overclock_settings.previous = menu.page;
				menu.page = &menu_page_overclock_settings;
				break;

			case 3:
				// Switch to "Global settings" page
				menu_page_information_and_tools.selected = 0;
				menu_page_information_and_tools.previous = menu.page;
				menu.page = &menu_page_information_and_tools;
				break;

			case 8:
				// Suspend Vita
				kscePowerRequestSuspend();
				break;

			case 9:
				// Restart Vita
				kscePowerRequestColdReset();
				break;

			case 10:
				// Shutdown Vita
				kscePowerRequestStandby();
				break;
		}
	}
}

Page menu_page_main = { NULL, 0, &menu_main_draw, (MenuInputFunction) &menu_main_input };

// Global Settings page

static void menu_global_settings_draw(void)
{
	menu_draw_title(ICON_LOLI "Menu" " / Global settings");
	
	menu_draw_entry_ln(0, "Text settings", 8);
	display.text_y += 8;
	menu_draw_entry_ln(1, "Save global settings", 12);
	menu_draw_entry_ln(2, "Reset global settings", 12);
	menu_draw_entry_ln(3, "Reload global settings", 12);
}

static void menu_global_settings_input(uint32_t pressed)
{
	// Contants
	static const int count = 4;

	// Navigate
	if ((pressed & SCE_CTRL_UP) && menu.page->selected > 0)
		-- menu.page->selected;
	if ((pressed & SCE_CTRL_DOWN) && menu.page->selected < count -1)
		++ menu.page->selected;

	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.page = menu.page->previous;
	// Accept
	if (pressed & SCE_CTRL_CROSS)
	{
		switch (menu.page->selected)
		{
			case 1:
				config_save_global();
				break;

			case 2:
				config_reset_global();
				break;

			case 3:
				config_load_global();
				break;
		}
	}
}

Page menu_page_global_settings = { NULL, 0, &menu_global_settings_draw, (MenuInputFunction) &menu_global_settings_input };

// Control Settings page

static void menu_control_settings_draw(void)
{
	// Constants
	static const char * BLUETOOTH_TOUCH_MODE[] = {
		/*0*/ "Disabled",
		/*1*/ "Front",
		/*2*/ "Back",
	};
	static const char * BLUETOOTH_TOUCH_CLICK[] = {
		/*0*/ "Disabled",
		/*1*/ "Touch",
		/*2*/ "Toggle (F/B)",
		/*3*/ "Toggle (D/F/B)",
	};
	static const char * BLUETOOTH_MOTION_MODE[] = {
		/*0*/ "Disabled",
		/*1*/ "Normal",
	};
	
	menu_draw_title(ICON_LOLI "Menu" " / Control settings");
	
	menu_draw_entry(0, "Swap the " ICON_CROSS " and " ICON_CIRCLE " buttons", 20); menu_draw_entry_ln(0, to_toggle(profile_config->enable_button_swap), -20);
	menu_draw_entry(1, "Disable the L3 and R3 buttons", 20); menu_draw_entry_ln(1, to_toggle(profile_config->disable_button_L3_R3), -20);
	menu_draw_entry(-1, "Enable custom layout", 20); menu_draw_entry_ln(-1, to_toggle(profile_config->enable_button_remap), -20);
	menu_draw_entry_ln(-1, "Edit custom layout", 20);
	display.text_y += 8;
	menu_draw_entry(-1, ICON_BT " Touchpad mode", 20); menu_draw_entry_ln(-1, BLUETOOTH_TOUCH_MODE[profile_config->bluetooth_touch_mode], -20);
	menu_draw_entry(-1, ICON_BT " Touchpad click", 20); menu_draw_entry_ln(-1, BLUETOOTH_TOUCH_CLICK[profile_config->bluetooth_touch_click], -20);
	menu_draw_entry(-1, ICON_BT " Motion controls", 20); menu_draw_entry_ln(-1, BLUETOOTH_MOTION_MODE[profile_config->bluetooth_motion_mode], -20);
	display.text_y += 8;
	menu_draw_entry(-1, "Exit and save changes", 11);

}

static void menu_control_settings_input(uint32_t pressed)
{
	// Contants
	static const int count = 2;

	// Navigate
	if ((pressed & SCE_CTRL_UP) && menu.page->selected > 0)
		-- menu.page->selected;
	if ((pressed & SCE_CTRL_DOWN) && menu.page->selected < count -1)
		++ menu.page->selected;

	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.page = menu.page->previous;
	// Accept
	if (pressed & SCE_CTRL_CROSS)
	{
		switch (menu.page->selected)
		{
			case 0:
				profile_config->enable_button_swap = !profile_config->enable_button_swap;
				break;
			case 1:
				profile_config->disable_button_L3_R3 = !profile_config->disable_button_L3_R3;
				break;
		}
	}
}

Page menu_page_control_settings = { NULL, 0, &menu_control_settings_draw, (MenuInputFunction) &menu_control_settings_input };

// Overclock Settings page

static void menu_overclock_settings_draw(void)
{
	menu_draw_title(ICON_LOLI "Menu" " / Overclock settings");
}

static void menu_overclock_settings_input(uint32_t pressed)
{
	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.page = menu.page->previous;
}

Page menu_page_overclock_settings = { NULL, 0, &menu_overclock_settings_draw, (MenuInputFunction) &menu_overclock_settings_input };

// Tools & Information page

static void menu_information_and_tools_draw(void)
{
	// Variables
	char buffer[20];
	
	menu_draw_title(ICON_LOLI "Menu" " / Information and Tools");

	// Draw information screen
	menu_draw_entry(100, "Shell ID:", 12);	menu_draw_entry_ln(100, kuint2str(buffer, shell_pid), -12);
	menu_draw_entry(100, "Focus ID:", 12);	menu_draw_entry_ln(100, kuint2str(buffer, focus_pid), -12);
	menu_draw_entry(100, "Focus title:", 12);	menu_draw_entry_ln(100, focus_name, -12);
}

static void menu_information_and_tools_input(uint32_t pressed)
{
	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.page = menu.page->previous;
		
	// Unlock controls
	if (pressed & SCE_CTRL_R3)
		menu.capture = !menu.capture;
		
	// Control logging
	if (pressed & SCE_CTRL_L1)
		enable_logging = true;
	if (pressed & SCE_CTRL_R1)
		enable_logging = false;
}

Page menu_page_information_and_tools = { NULL, 0, &menu_information_and_tools_draw, (MenuInputFunction) &menu_information_and_tools_input };

// Confirm Dialog

Page menu_page_confirm = { NULL };
Page menu_page_edit_color = { NULL };

