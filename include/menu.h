#pragma once

// Function type for drawing the menu
typedef void (* MenuDrawFunction) (void);
// Function type for handling the input
typedef void (* MenuInputFunction) (uint32_t pressed, uint32_t up, uint32_t down, uint32_t held);

struct SPage
{
	// Selcted line
	int32_t selected;
	// Previous page
	struct SPage * previous;
	// Draw function
	MenuDrawFunction draw_func;
	// Input function
	MenuInputFunction input_func;
};
typedef struct SPage Page;

struct SMenu
{
	// If the menu is visible
	bool visible;
	// Capture the input (usually be the same as visible)
	bool capture;
	// Current page
	Page * page;
};
typedef struct SMenu Menu;

// Global variables
extern Menu menu;

// Menu functions
void menu_init(void);
void menu_open(void);
void menu_close(void);

// Menu pages
extern Page menu_page_main;
extern Page menu_page_global_settings;
extern Page menu_page_control_settings;
extern Page menu_page_overclock_settings;
extern Page menu_page_information_and_tools;

// Window pages
extern Page menu_page_confirm;
extern Page menu_page_edit_color;

// Draw functions
