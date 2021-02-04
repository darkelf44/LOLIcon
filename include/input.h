#pragma once

// Constants
#define FOCUS_NAME_MAX 16

// Focus variables
extern char focus_name[FOCUS_NAME_MAX];
extern SceUID focus_pid;
extern bool focus_is_shell;
extern bool focus_is_pspemu;

// Input handlers
void input_handle(int8_t port, SceCtrlData * ctrl);
void input_filter(int8_t port, SceCtrlData * ctrl);

// Focus handlers
void focus_changed(SceUID pid);
