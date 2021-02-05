// Include headers
#include <common.h>

// Inputs timestamps within this range are assumed to be old values
#define TIMESTAMP_SHADOW 100000

// Controller repeat delays
#define REPEAT_DELAY_FIRST 400000
#define REPEAT_DELAY_AFTER 200000

// Focus variables
char focus_name[FOCUS_NAME_MAX];
SceUID focus_pid = 0;
bool focus_is_shell = false;
bool focus_is_pspemu = false;

// Input variables
static KMutex   mutex = 0;
static uint32_t repeat = 0;
static uint32_t prev_buttons = 0;
static uint32_t prev_timestamp = 0;

void input_handle(int8_t port, SceCtrlData * ctrl)
{
	// Only handle the first port (Handling port 1 doubles all events, because its a copy of port 0)
	if (port != 0)
		return;

	// Inputs are processed on one thread at a time
	if (kmutex_try_lock(&mutex))
	{
		// Variables
		uint32_t pressed = 0;
		uint32_t down = 0;
		uint32_t up = 0;
		uint32_t held = ctrl->buttons;
		
		// Do not process old inputs
		if (prev_timestamp - ctrl->timeStamp < TIMESTAMP_SHADOW)
		{
			// Release mutex and return
			kmutex_unlock(&mutex);
			return;
		}

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
		prev_timestamp = ctrl->timeStamp;

		// SELECT + UP - Opens and closes the menu
		if ((pressed & SCE_CTRL_UP) && (held & SCE_CTRL_SELECT))
		{
			// Open or close the menu
			if (menu.visible)
				menu_close();
			else
				menu_open();
				
			// Clear pressed button
			pressed &= ~SCE_CTRL_UP;
		}

		// Handle events
		if (pressed)
		{
			// Call menu input handler
			if (menu.visible)
				menu.page->input_func(pressed, up, down, held);
		}
		
		// Release mutex
		kmutex_unlock(&mutex);
	}
}

void input_filter(int8_t port, SceCtrlData * ctrl)
{
	// Remove captured button presses
	if (menu.capture)
		ctrl->buttons = 0;
	
	// Full button remapping
	if (profile_config->enable_button_remap)
	{
		// Remap all buttons
	}
	else
	{
		// Swap cross and circle
		if (profile_config->enable_button_swap)
		{
			uint32_t state = ctrl->buttons & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE);
			if (state == SCE_CTRL_CROSS || state == SCE_CTRL_CIRCLE)
				ctrl->buttons ^= (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE);
		}

		// Filter out L3 and R3 button presses
		if (profile_config->disable_button_L3_R3)
			ctrl->buttons &= ~ (SCE_CTRL_L3 | SCE_CTRL_R3);
	}
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

