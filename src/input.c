// Include headers
#include <common.h>

// Global variables
static uint32_t prev_buttons[MAX_INPUT_PORTS] = {0};

void input_handle(int8_t port, uint32_t buttons)
{
	// Variables
	uint32_t pressed, released;
	
	if (port >= MAX_INPUT_PORTS)
		return;
	
	// Buttons that were just pressed
	pressed = buttons & ~ prev_buttons[port]; 
	// Buttons that were just released
	released = prev_buttons[port] & ~ buttons;
	// Update previous buttons
	prev_buttons[port] = buttons;

	// Events on button press or release
	if (pressed | released)
	{
		// SELECT + UP - Opens and closes the menu
		if ((pressed & SCE_CTRL_UP) && (buttons & SCE_CTRL_SELECT))
		{
			// Open or close the menu
			if (menu.visible)
				menu_close();
			else
				menu_open();
				
			// stop handing inputs
			return;
		}

		// Call menu input handler
		if (menu.capture)
			menu.input_func(pressed, released, buttons);
	}
}

void input_filter(int8_t port, uint32_t * buttons)
{
	// Remove captured button presses
	if (menu.capture)
		* buttons = 0;
}
