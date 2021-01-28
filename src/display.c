// include header
#include <common.h>

// Monospace font
extern const int16_t monospace_char_count;
extern const int16_t monospace_char_width;
extern const int16_t monospace_char_height;
extern const int16_t monospace_char_table[];

// Global variables
Display display = {0};

// Initialize display
void display_init(void)
{
	// Add "monospace" font
	display.font_list[0].char_count  = monospace_char_count;
	display.font_list[0].char_width  = monospace_char_width;
	display.font_list[0].char_height = monospace_char_height;
	display.font_list[0].char_table  = monospace_char_table;
	
	// Set display font
	display.font = &display.font_list[0];

	// Reset text position
	display.text_x = 0;
	display.text_y = 0;
}

// Set the framebuffer before rendering
void display_set_framebuffer(const SceDisplayFrameBuf * fb)
{
	// Copy framebuffer data
	display.framebuffer = *fb;
}

int32_t display_draw_text(const char * text, size_t n, uint32_t fgcolor, uint32_t bgcolor)
{
	// Constants
	const Font font = * display.font;
	uint16_t linesize = display.framebuffer.pitch;
	uint32_t * framebuffer = display.framebuffer.base;
	const uint32_t fgalpha = fgcolor >> 24;
	const uint32_t bgalpha = bgcolor >> 24;

	// Variables
	static uint32_t linebuffer[16];	// Buffer used to blit a line of pixels for the character
	size_t k = 0;

	// Use -1 for null terminated strings
	if (n == (size_t) -1)
		for (n = 0; text[n]; ++ n) /* skip */;

	// Check pixel format
	if (display.framebuffer.pixelformat != SCE_DISPLAY_PIXELFORMAT_A8B8G8R8)
		return -1;
	// Check Y position
	if (display.text_y + font.char_height >= display.framebuffer.height)
		return 0;

	// Remove alpha from colors
	fgcolor &= 0xFFFFFF;
	bgcolor &= 0xFFFFFF;

	// Print text
	for (; (k < n) && (display.text_x + font.char_width < display.framebuffer.width); (++ k, display.text_x += font.char_width))
	{
		// Variables
		int16_t i, j;

		// Get character
		uint16_t ch = (uint16_t) text[k];
		if (ch >= font.char_count)
			ch = 0;

		// Display character
		for (j = 0; j < font.char_height; ++ j)
		{
			// Variables
			int16_t data;
			
			// Prepare a single line of a character for copying
			if (font.char_width > 8)
				data = ((const uint16_t *) font.char_table)[ch * font.char_height + j];
			else
				data = ((const uint8_t *) font.char_table)[ch * font.char_height + j];

			for (i = 0; i < font.char_width; ++ i)
				if (data & (1 << i))
					linebuffer[i] = fgcolor;
				else
					linebuffer[i] = bgcolor;

			// Copy the line into the framebuffer
			ksceKernelMemcpyKernelToUser((uintptr_t)(framebuffer + (display.text_x + i) + (display.text_y + j) * linesize), linebuffer, font.char_width * sizeof(uint32_t));
		}
	}

	// Return the number of characters written
	return k;
}

int32_t display_draw_text_at(const char * text, size_t n, uint16_t x, uint16_t y, uint32_t fgcolor, uint32_t bgcolor)
{
	// Set coordinates
	display.text_x = x;
	display.text_y = y;
	// Draw text
	return display_draw_text(text, n, fgcolor, bgcolor);
}
