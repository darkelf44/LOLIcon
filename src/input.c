// Include headers
#include <common.h>

// Inputs timestamps within this range are assumed to be old values
#define TIMESTAMP_SHADOW 100000

// Controller repeat delays
#define REPEAT_DELAY_FIRST 400000
#define REPEAT_DELAY_AFTER 200000

// Focus variables
volatile SceUID shell_pid = 0;
volatile SceUID focus_pid = 0;
static KMutex focus_mutex = 0;

char focus_name[FOCUS_NAME_MAX];
bool focus_is_shell = false;
bool focus_is_pspemu = false;

// Input variables
static KMutex   input_mutex = 0;
static uint32_t prev_buttons = 0;
static uint64_t prev_timestamp = 0;
static uint64_t repeat_timestamp = 0;

void input_handle(int8_t port, SceCtrlData * ctrl)
{
	// Only handle the first port (Handling port 1 doubles all events, because its a copy of port 0)
	if (port != 0)
		return;
	// Only handle focused application
	if (focus_pid && focus_pid != ksceKernelGetProcessId())
		return;

	// Inputs are processed on one thread at a time
	if (kmutex_try_lock(&input_mutex))
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
			kmutex_unlock(&input_mutex);
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
			repeat_timestamp = ctrl->timeStamp + REPEAT_DELAY_FIRST;
		}
		else if (repeat_timestamp < ctrl->timeStamp)
		{
			// Button presses
			pressed = ctrl->buttons;
			// Reset repeat time
			repeat_timestamp = ctrl->timeStamp + REPEAT_DELAY_AFTER;
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
		kmutex_unlock(&input_mutex);
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

void focus_reset()
{
	// Remove focused app
	focus_pid = 0;
	// Focus on shell
	if (shell_pid)
		focus_update(shell_pid);
}

void focus_update(SceUID pid)
{
	// No need to change focus
	if (pid == focus_pid)
		return;
		
	// Only change back to shell after focus is reset
	if (focus_pid && pid == shell_pid)
		return;

	// Change focus
	if (kmutex_try_lock(&focus_mutex))
	{
		// Variables
		char title[FOCUS_NAME_MAX];

		// Get process title
		if (ksceKernelGetProcessTitleId(pid, title, FOCUS_NAME_MAX) == 0)
		{
			// Set shell pid on first render
			if (kstreq("main", title))
				shell_pid = pid;

			// Actually change the focus
			focus_pid = pid;
			kstrcpy(focus_name, title, FOCUS_NAME_MAX);
		}
		
		// Unlock mutex
		kmutex_unlock(&focus_mutex);
	}

	// TODO: Load config

	// Apply config
}
