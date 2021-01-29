// Include common headers
#include <common.h>

// Colors
#define FGCOLOR 0xFFFFFFFF
#define BGCOLOR 0x66FF0000
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
	menu.selected = 0;
	menu.draw_func = menu_main_draw;
	menu.input_func = menu_main_input;
}

// Open the menu
void menu_open(void)
{
	// Make menu visible
	menu.visible = true;
	menu.capture = true;

	// Display main menu
	menu.selected = 0;
	menu.draw_func = menu_main_draw;
	menu.input_func = menu_main_input;
}

// Close the menu
void menu_close(void)
{
	// Make menu visible
	menu.visible = false;
	menu.capture = false;
}

// Center and draw menu title
void menu_draw_title(const char * title, uint16_t width)
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
	
	// Center the next lines on width
	display.text_x = (display.framebuffer.width - width * dx) / 2;
	display.text_y += 2 * dy;
}

// Draw a simple static menu entry
void menu_draw_entry(int16_t index, const char * text)
{
	// Variables
	size_t n = strlen(text);
	uint16_t x = display.text_x;
	
	// Draw entry
	display_draw_text(text, n, (index == menu.selected) ? FGSELECT : FGCOLOR, (index == menu.selected) ? BGSELECT : BGCOLOR);
	
	// Update position
	display.text_x = x;
	display.text_y += display.font->char_height;
}

void menu_main_draw(void)
{
	menu_draw_title(ICON_LOLI "Menu" " [indev]", 24);
	
	menu_draw_entry(0, "Global settings");
	menu_draw_entry(1, "Control settings");
	menu_draw_entry(2, "Overclock settings");
	menu_draw_entry(3, "Tools");
	menu_draw_entry(4, "Save profile");
	menu_draw_entry(5, "Save profile as default");
	menu_draw_entry(6, "Reset profile");
	menu_draw_entry(7, "Exit game");
	display.text_y += 8;
	menu_draw_entry(8, ICON_SLEEP " Suspend vita");
	menu_draw_entry(9, ICON_RESTART " Restart vita");
	menu_draw_entry(10, ICON_POWER " Shutdown vita");
}

void menu_main_input(uint32_t pressed)
{
	// Contants
	static const int count = 11;
	
	// Navigate
	if (pressed & SCE_CTRL_UP)
		menu.selected = (menu.selected - 1) % count;
	if (pressed & SCE_CTRL_DOWN)
		menu.selected = (menu.selected + 1) % count;
		
	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
		menu.selected = 0;
	// Accept
	if (pressed & SCE_CTRL_CROSS)
	{
		switch (menu.selected)
		{
			case 0:
				// Switch to "Global settings" page
				menu.draw_func = menu_global_settings_draw;
				menu.input_func = menu_global_settings_input;
				menu.selected = 0;
				break;
		}
	}
}

void menu_global_settings_draw(void)
{
	menu_draw_title(ICON_LOLI "Menu" " - Global settings", 26);
}

void menu_global_settings_input(uint32_t pressed)
{
	// Cancel
	if (pressed & SCE_CTRL_CIRCLE)
	{
		menu.draw_func = menu_main_draw;
		menu.input_func = menu_main_input;
		menu.selected = 0;
	}
}


