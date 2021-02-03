#pragma once

// Function type for drawing the menu
typedef void (* MenuDrawFunction) (void);
// Function type for handling the input
typedef void (* MenuInputFunction) (uint32_t pressed, uint32_t up, uint32_t down, uint32_t held);

struct S_Menu
{
	// If the menu is visible
	bool visible;
	// Capture the input (usually be the same as visible)
	bool capture;
	// Index of the selected item
	int16_t selected;
	
	// Current draw and input functions
	MenuDrawFunction draw_func;
	MenuInputFunction input_func;
};
typedef struct S_Menu Menu;

// Global variables
extern Menu menu;

// Menu functions
void menu_init(void);
void menu_open(void);
void menu_close(void);

// Draw functions
void menu_draw_title(const char * title, uint16_t width);
void menu_draw_entry(int16_t index, const char * text);

// Functions for main menu
void menu_main_draw(void);
void menu_main_input(uint32_t pressed);

// Functions for "Global settings" menu
void menu_global_settings_draw(void);
void menu_global_settings_input(uint32_t pressed);

// Functions for "Control settings" menu
void menu_control_settings_draw(void);
void menu_control_settings_input(uint32_t pressed);

// Functions for "Overclock settings" menu
void menu_overclock_settings_draw(void);
void menu_overclock_settings_input(uint32_t pressed);

// Functions for "Tools" menu
void menu_tools_draw(void);
void menu_tools_input(uint32_t pressed);
