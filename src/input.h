#pragma once

// Constants
#define MAX_INPUT_PORTS 4

// Global
void input_handle(int8_t port, uint32_t buttons);
void input_filter(int8_t port, uint32_t * buttons);
