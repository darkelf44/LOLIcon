#pragma once

// Constants
#define MAX_FONTS 4

// Icons in the font
#define ICON_PSLOGO   "\x01\x02"
#define ICON_LOLI    "\x11\x12\x13"

#define ICON_ON       "\x0e\x0f\x10"
#define ICON_OFF      "\x0b\x0c\x0d"

#define ICON_UP       "\x07"
#define ICON_DOWN     "\x08"
#define ICON_LEFT     "\x09"
#define ICON_RIGHT    "\x0a"

#define ICON_TRIANGLE "\x03"
#define ICON_CIRCLE   "\x04"
#define ICON_CROSS    "\x05"
#define ICON_SQUARE   "\x06"

#define ICON_BT       "\x14"
#define ICON_POWER    "\x15"
#define ICON_RESTART  "\x16"
#define ICON_SLEEP    "\x17"

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
void display_init(void);
void display_set_framebuffer(const SceDisplayFrameBuf * fb);

// Text drawing functions
int32_t display_draw_text(const char * text, size_t n, uint32_t fgcolor, uint32_t bgcolor);
int32_t display_draw_text_at(const char * text, size_t n, uint16_t x, uint16_t y, uint32_t fgcolor, uint32_t bgcolor);
