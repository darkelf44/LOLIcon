// Include headers
#include <common.h>

// Controller repeat delays
#define REPEAT_DELAY_FIRST 6000000
#define REPEAT_DELAY_AFTER 4000000

// Focus variables
char focus_name[FOCUS_NAME_MAX];
SceUID focus_pid = 0;
bool focus_is_shell = false;
bool focus_is_pspemu = false;


// Input variables
static uint32_t repeat = 0;
static uint32_t prev_buttons = 0;

void input_handle(int8_t port, SceCtrlData * ctrl)
{
	// Variables
	uint32_t pressed = 0;
	uint32_t down = 0;
	uint32_t up = 0;
	uint32_t held = ctrl->buttons;

	// Only handle the first port (Handling port 1 doubles all events, because its a copy of port 0)
	if (port != 0)
		return;

	// Process button events
	if (ctrl->buttons != prev_buttons)
	{
		// Buttons that were just released
		up = prev_buttons & ~ ctrl->buttons;
		// Buttons that were just pressed
		down = ctrl->buttons & ~ prev_buttons;
		// Buttons presses
		pressed = down;
		// Reset repeat time
		repeat = ctrl->timeStamp + REPEAT_DELAY_FIRST;
	}
	else if (repeat < ctrl->timeStamp)
	{
		// Button presses
		pressed = ctrl->buttons;
		// Reset repeat time
		repeat = ctrl->timeStamp + REPEAT_DELAY_AFTER;
	}

	// Update previous buttons
	prev_buttons = ctrl->buttons;

	// Handle events
	if (pressed)
	{
		// SELECT + UP - Opens and closes the menu
		if ((pressed & SCE_CTRL_UP) && (held & SCE_CTRL_SELECT))
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
			menu.input_func(pressed, up, down, held);
	}
}

void input_filter(int8_t port, SceCtrlData * ctrl)
{
	// Remove captured button presses
	if (menu.capture)
		ctrl->buttons = 0;
}

void focus_changed(SceUID pid)
{
	// Get focused process name
	focus_pid = pid;
	if (ksceKernelGetProcessTitleId(pid, focus_name, FOCUS_NAME_MAX) == 0)
	{
		focus_is_shell = kstreq("main", focus_name);
		focus_is_pspemu = kstreq("PSPEMUCFW", focus_name);
	}
	
	// TODO: reload config
	
	// TODO: apply overclocks
}

