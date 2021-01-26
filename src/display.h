#pragma once

#include <common.h>

// Constants
#define MAX_FONTS 4

struct S_Font
{
	uint16_t     char_count;
	uint16_t     char_width;
	uint16_t     char_height;
	const void * char_table;
};
typedef struct S_Font Font;

struct S_Display
{
	// Framebuffer settings
	SceDisplayFrameBuf framebuffer;
	
	// Text settings
	uint16_t text_x;
	uint16_t text_y;
	
	// Font settings
	Font * font;
	Font   font_list[MAX_FONTS];
};
typedef struct S_Display Display;

// Global variables
extern Display display;

// Setup functions
bool display_init(void);
bool display_set_framebuffer(const SceDisplayFrameBuf * fb);

// Text drawing functions
int32_t display_draw_text(const char * text, size_t n, uint32_t fgcolor, uint32_t bgcolor);
int32_t display_draw_text_at(const char * text, size_t n, uint16_t x, uint16_t y, uint32_t fgcolor, uint32_t bgcolor);
