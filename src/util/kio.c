// Include file header
#include <util/kio.h>
#include <util/klib.h>

// Kernel includes
#include <psp2kern/io/fcntl.h>

// Public open mode flags
#define OPENMODE_READ  1
#define OPENMODE_WRITE 2
#define OPENMODE_TEXT  4
#define OPENMODE_MASK  7

// Private open mode flags
#define OPENMODE_CREATE 0x10000
#define OPENMODE_APPEND 0x20000
#define OPENMODE_DELETE 0x40000

// Public buffer flags
#define BUFFER_STATIC   0x08
#define BUFFERMODE_NONE 0
#define BUFFERMODE_LINE 0x10
#define BUFFERMODE_FULL 0x20

// Other flags
#define FLAG_EOF   0x100
#define FLAG_ERROR 0x200

// Array of file streams
KFile file_array[KFOPEN_MAX] = {0};

// Convert open mode string to stream flags
int mode2flags(const char * mode)
{
	// Variables
	char openmode[3];
	int flags = 0;

	// Normalize mode string
	openmode[0] = *mode ? *mode++ : 0;
	openmode[1] = *mode ? *mode++ : 0;
	openmode[2] = *mode ? *mode++ : 0;
	if (openmode[1] == '+')
	{
		openmode[1] = openmode[2];
		openmode[2] = '+';
	}

	// Validate mode string
	switch (openmode[0])
	{
		case 'r': flags |= OPENMODE_READ; break;
		case 'w': flags |= OPENMODE_WRITE | OPENMODE_CREATE | OPENMODE_DELETE; break;
		case 'a': flags |= OPENMODE_WRITE | OPENMODE_CREATE | OPENMODE_APPEND; break;
		default:
			return -1;
	}
	switch (openmode[1])
	{
		case 0: break;
		case 'b': break;
		case 't': flags |= OPENMODE_TEXT; break;
		default:
			return -1;
	}
	switch (openmode[2])
	{
		case 0: break;
		case '+': flags |= OPENMODE_READ | OPENMODE_WRITE; break;
		default:
			return -1;
	}

	// Return result
	return flags;
}

// Discard buffer after reading
static void rflush(KFile * file)
{
	if (file->read != file->buffer)
	{
		// Discard buffer contents
		file->offset += (file->read - file->buffer);
		file->read = file->buffer;
	}
	file->read_size = 0;
}

// Flush buffer after writing
static bool wflush(KFile * file)
{
	// Flush the stream after writing
	if (file->write != file->buffer)
	{
		// Flush buffer to disk
		char * next = file->buffer;
		while (next < file->write)
		{
			size_t n = ksceIoWrite(file->fd, next, file->write - next);
			if (n > 0)
			{
				file->offset += n;
				next += n;
			}
			else
			{
				file->flags |= FLAG_ERROR;
				return false;
			}
		}
		file->write = file->buffer;
	}
	file->write_size = 0;
	return true;
}


KFile * kfopen(const char * filename, const char * mode)
{
	// Variables
	KFile * file;
	int flags = mode2flags(mode);
	int m = 0;
	size_t i = 0;
	SceUID fd;
	char * buffer;

	// Validate flags
	if (flags < 0)
		return NULL;

	// Find an available stream
	for (i = 0; i < KFOPEN_MAX && file_array[i].flags; ++ i)
		{/* SKIP */}
	if (i < KFOPEN_MAX)
		file = file_array + i;
	else
		return NULL;

	// Set open mode
	switch (flags & (OPENMODE_READ | OPENMODE_WRITE))
	{
		case OPENMODE_READ | OPENMODE_WRITE: m = SCE_O_RDWR; break;
		case OPENMODE_READ: m = SCE_O_RDONLY; break;
		case OPENMODE_WRITE: m = SCE_O_WRONLY; break;
	}
	if (flags & OPENMODE_APPEND)
		m |= SCE_O_APPEND ;
	if (flags & OPENMODE_CREATE)
		m |= SCE_O_CREAT;
	if (flags & OPENMODE_DELETE)
		m |= SCE_O_TRUNC;

	// Open file
	fd = ksceIoOpen(filename, m, 0777);
	if (fd < 0)
		return NULL;

	// Allocate buffer
	buffer = kmalloc(KBUFFERSIZE);
	if (buffer)
		flags |= (flags & OPENMODE_TEXT) ? BUFFERMODE_LINE : BUFFERMODE_FULL;

	// Create result
	file->fd = fd;
	file->flags = flags & 0xFFFF;
	file->offset = 0;
	file->read = buffer;
	file->read_size = 0;
	file->write = buffer;
	file->write_size = 0;
	file->buffer = buffer;
	file->buffer_size = buffer ? KBUFFERSIZE : 0;

	// Return result
	return file;
}


KFile * kfdopen(SceUID fileno, const char * mode)
{
	// Variables
	KFile * file;
	int flags = mode2flags(mode);
	size_t i = 0;
	char * buffer;

	// Validate parameters
	if (fileno < 0 || flags < 0)
		return NULL;

	// Find an available stream
	for (i = 0; i < KFOPEN_MAX && file_array[i].flags; ++ i)
		{/* SKIP */}
	if (i < KFOPEN_MAX)
		file = file_array + i;
	else
		return NULL;

	// Allocate buffer
	buffer = kmalloc(KBUFFERSIZE);
	if (buffer)
		flags |= (flags & OPENMODE_TEXT) ? BUFFERMODE_LINE : BUFFERMODE_FULL;

	// Create result
	file->fd = fileno;
	file->flags = flags & 0xFFFF;
	file->offset = 0;
	file->read = buffer;
	file->read_size = 0;
	file->write = buffer;
	file->write_size = 0;
	file->buffer = buffer;
	file->buffer_size = buffer ? KBUFFERSIZE : 0;

	// Return result
	return file;
}

bool kfclose(KFile * file)
{
	// Flush file
	wflush(file);

	// Free buffer
	if (file->buffer && !(file->flags & BUFFER_STATIC))
		kfree(file->buffer);

	// Reset flags
	file->flags = 0;

	// Close file
	file->fd = ksceIoClose(file->fd);

	// return result
	return file->fd == 0;
}

void kfcloseall()
{
	// Variables
	size_t i;

	// Close all files
	for (i = 0; i < KFOPEN_MAX; ++ i)
		if (file_array[i].flags)
			kfclose(file_array + i);
}


SceUID kfileno(KFile * file)
{
	// Get file descriptor
	return file->fd;
}

bool kfeof(KFile * file)
{
	// Get EOF indicator
	return file->flags & FLAG_EOF;
}

bool kferror(KFile * file)
{
	// Get error indicator
	return file->flags & FLAG_ERROR;
}

void kfclear(KFile * file)
{
	// Clear EOF and error indicator
	file->flags &= ~(FLAG_EOF | FLAG_ERROR);
}

bool kfflush(KFile * file)
{
	
	// Flush all
	if (!file)
	{
		size_t i = 0;
		for (; i < KFOPEN_MAX; ++ i)
			if (file_array[i].flags & OPENMODE_WRITE)
				wflush(file_array + i);
	}

	// Discard the buffer after reading
	rflush(file);

	// Flush the stream after writing
	return wflush(file);
}

size_t kfread(KFile * file, void * buffer, size_t size)
{
	// FIXME: Not implemented
	return 0;
}

size_t kfwrite(KFile * file, const void * buffer, size_t size)
{
	// FIXME: Not implemented
	return 0;
}


int kfgetc(KFile * file)
{
	// Check buffer
	if (file->read_size > 0)
	{
		-- file->read_size;
		return * file->read ++;
	}

	// Check EOF, buffer and openmode
	if (file->flags & FLAG_EOF)
		return -1;
	if (!file->buffer || !(file->flags & OPENMODE_READ))
	{
		file->flags |= FLAG_ERROR;
		return -1;
	}

	// Flush buffer (when needed)
	wflush(file);

	// Fill buffer (read at least one character)
	file->offset += file->read - file->buffer;
	file->read = file->buffer;
	file->read_size = ksceIoRead(file->fd, file->read, file->buffer_size);

	// Get character
	if (file->read_size > 0)
	{
		-- file->read_size;
		return * file->read ++;
	}
	else
	{
		file->flags |= FLAG_EOF;
		return -1;
	}
}

int kfputc(KFile * file, char ch)
{
	// Check buffer
	if (file->write_size > 0)
	{
		-- file->write_size;
		* file->write ++ = ch;
		return ch;
	}

	// Check buffer and openmode
	if (!file->buffer || !(file->flags & OPENMODE_WRITE))
	{
		file->flags |= FLAG_ERROR;
		return -1;
	}

	// Discard buffer (when needed)
	rflush(file);

	// Flush buffer
	if (file->write != file->buffer)
	{
		char * next = file->buffer;
		while (next < file->write)
		{
			size_t n = ksceIoWrite(file->fd, next, file->write - next);
			if (n > 0)
			{
				file->offset += n;
				next += n;
			}
			else
			{
				file->flags |= FLAG_ERROR;
				return -1;
			}
		}
		file->write = file->buffer;
		file->write_size = file->buffer_size;
	}

	// Put character
	-- file->write_size;
	* file->write ++ = ch;
	return ch;
}

char * kfgets(KFile * file, char * str, size_t n)
{
	char * next = str;
	if (n > 1)
	{
		int ch = kgetc(file);
		for (; (ch > 0) && (ch != '\n') && (-- n > 0); ch = kgetc(file))
			{* next ++ = ch;}
		* next = 0;
	}
	return next == str ? NULL : str;
}

size_t kfputs(KFile * file, const char * str)
{
	const char * next = str;
	while (* next)
		kputc(file, * next ++);
	return next - str;
}

SceOff kftell(KFile * file)
{
	// Calculate stream position
	if (file->read != file->buffer)
		return file->offset + (file->read - file->buffer);
	else
		return file->offset + (file->write - file->buffer);
}

bool kfseek(KFile * file, SceOff offset, int mode)
{
	// Variables
	SceOff pos;
	int sce_mode;
	
	// Translate mode
	switch (mode)
	{
		case KSEEK_SET: sce_mode = SCE_SEEK_SET; break;
		case KSEEK_CUR: sce_mode = SCE_SEEK_CUR; break;
		case KSEEK_END: sce_mode = SCE_SEEK_END; break;
		default:
			return false;
	}
	
	// Flush file (when needed)
	wflush(file);
	
	// Seek the stream
	pos = file->offset = ksceIoLseek(file->fd, offset, sce_mode);
	if (pos == (SceOff) -1)
	{
		return false;
	}
	else
	{
		file->offset = pos;
		file->flags &= ~(FLAG_EOF | FLAG_ERROR);
		file->read = file->buffer;
		file->read_size = 0;
		file->write = file->buffer;
		file->write_size = 0;
		return true;
	}
}

bool ksetbuf(KFile * file, char * buffer, int mode, size_t size)
{
	// FIXME: Not implemented
	return false;
}

size_t kfprintf(KFile * file, const char * format, ...)
{
	// FIXME: Not implemented
	return 0;
}

size_t ksprintf(char * buffer, size_t n, const char * format, ...)
{
	// FIXME: Not implemented
	return 0;
}

size_t kfprintfv(KFile * file, const char * format, va_list list)
{
	// FIXME: Not implemented
	return 0;
}

size_t ksprintfv(char * buffer, size_t n, const char * format, va_list list)
{
	// FIXME: Not implemented
	return 0;
}
