#pragma once

// Includes
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <psp2/types.h>

// Constants
#define KFOPEN_MAX 8
#define KBUFFERSIZE 0x100

// IO buffer constants
#define KIO_NBF 0
#define KIO_LBF 1
#define KIO_FBF 2

// Seek constants
#define KSEEK_SET 0
#define KSEEK_CUR 1
#define KSEEK_END 2

// INLINE macro - Forces the function to be inlined
#undef INLINE
#define INLINE __attribute__((always_inline)) __inline__

// IO file structure
struct _KFile
{
	// File info
	SceUID fd;
	int    flags;
	SceOff offset;
	
	// Buffer info
	char * read;
	size_t read_size;
	char * write;
	size_t write_size;
	char * buffer;
	size_t buffer_size;
};
typedef struct _KFile KFile;

// Open and close file steams
KFile * kfopen(const char * filename, const char * mode);
KFile * kfdopen(SceUID fileno, const char * mode);
bool kfclose(KFile * file);
void kfcloseall();

// Stream functions
SceUID kfileno(KFile * file);
bool kfeof(KFile * file);
bool kferror(KFile * file);
void kfclear(KFile * file);
bool kfflush(KFile * file);

// Block functions
size_t kfread(KFile * file, void * buffer, size_t size);
size_t kfwrite(KFile * file, const void * buffer, size_t size);

// Character functions
int kfgetc(KFile * file);
int kfputc(KFile * file, char ch);
char * kfgets(KFile * file, char * str, size_t n);
size_t kfputs(KFile * file, const char * str);

// Inline character functions
INLINE int kgetc(KFile * file)
	{return file->read_size > 0 ? (-- file->read_size, * file->read ++) : kfgetc(file);}
INLINE int kputc(KFile * file, char ch)
	{return file->write_size > 0 ? (-- file->write_size, * file->write ++ = ch) : kfputc(file, ch);}

// Stream position functions
SceOff kftell(KFile * file);
bool kfseek(KFile * file, SceOff offset, int mode);

// Stream buffer functions
bool ksetbuf(KFile * file, char * buffer, int mode, size_t size);

// Print functions
size_t kfprintf(KFile * file, const char * format, ...);
size_t ksprintf(char * buffer, size_t n, const char * format, ...);
size_t kfprintfv(KFile * file, const char * format, va_list list);
size_t ksprintfv(char * buffer, size_t n, const char * format, va_list list);
