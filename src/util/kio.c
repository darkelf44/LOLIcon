// Include file header
#include <util/kio.h>

// Local includes
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

// Buffer for fprintf
char format_buffer[KBUFFERSIZE];

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

bool kmkdirs(const char * filename, SceMode mode)
{
	// Variables
	size_t i, n;
	char * buffer;

	// Get string size, allocate buffer
	for (n = 0; filename[n]; ++ n) {}
	buffer = kmalloc(n);
	if (!buffer)
		return false;

	// Create parent directories
	for (i = 0; i < n; ++ i)
	{
		// Directory separator, create directory
		if ((filename[i] == '/') && (i > 0) && (filename[i - 1] != '/'))
		{
			buffer[i] = 0;
			ksceIoMkdir(buffer, mode);
		}

		// Copy character
		buffer[i] = filename[i];
	}
	// create last directory
	if (filename[i - 1] != '/')
	{
		ksceIoMkdir(buffer, mode);
	}

	// Free buffer
	kfree(buffer);

	// Return the result
	return false;
}


// Format type values
#define FORMAT_TYPE_DEFAULT 0
#define FORMAT_TYPE_HH 1
#define FORMAT_TYPE_H  2
#define FORMAT_TYPE_L  3
#define FORMAT_TYPE_LL 4
#define FORMAT_TYPE_J  5
#define FORMAT_TYPE_Z  6
#define FORMAT_TYPE_T  7

// Format flag values
#define FORMAT_FLAG_BLANK 1
#define FORMAT_FLAG_SIGN  2
#define FORMAT_FLAG_LEFT  4
#define FORMAT_FLAG_ALTER 8
#define FORMAT_FLAG_ZERO  0x10
#define FORMAT_FLAG_EXT_WIDTH     0x100
#define FORMAT_FLAG_EXT_PRECISION 0x200

size_t kfprintf(KFile * file, const char * format, ...)
{
	// Variables
	size_t result;

	// Call implementation
	va_list list;
	va_start(list, format);
	result = kfprintfv(file, format, list);
	va_end(list);

	// Return result
	return result;
}

size_t ksprintf(char * buffer, size_t n, const char * format, ...)
{
	// Variables
	size_t result;

	// Call implementation
	va_list list;
	va_start(list, format);
	result = ksprintfv(buffer, n, format, list);
	va_end(list);

	// Return result
	return result;
}

size_t kfprintfv(KFile * file, const char * format, va_list list)
{
	// Format string
	size_t result = ksprintfv(format_buffer, sizeof(format_buffer), format, list);
	// write to file
	kfputs(file, format_buffer);
	// return the result
	return result;
}

size_t ksprintfv(char * buffer, size_t n, const char * format, va_list list)
{
	// Variables
	size_t i = 0;
	size_t size = (n > 0) ? (n - 1) : 0;
	char * next = buffer;

	// Process format string
	for (; *format; ++ format)
	{
		if (*format == '%')
		{
			// Variables
			uint16_t type = 0;
			uint16_t flags = 0;
			int width = 0;
			int precision = -1;

			// Next character
			++ format;

			// Process format flags
			while (*format == ' ' || *format == '+' || *format == '-' ||*format == '#' ||*format == '0')
			{
				switch (*format)
				{
					case ' ':
						flags |= FORMAT_FLAG_BLANK;
						break;
					case '+':
						flags |= FORMAT_FLAG_SIGN;
						break;
					case '-':
						flags |= FORMAT_FLAG_LEFT;
						break;
					case '#':
						flags |= FORMAT_FLAG_ALTER;
						break;
					case '0':
						flags |= FORMAT_FLAG_ZERO;
						break;
				}
				++ format;
			}
			if (flags & FORMAT_FLAG_LEFT)
				flags &= ~ FORMAT_FLAG_ZERO;
			if (flags & FORMAT_FLAG_SIGN)
				flags &= ~ FORMAT_FLAG_BLANK;

			// Process format width
			if (*format == '*')
			{
				flags |= FORMAT_FLAG_EXT_WIDTH;
				++ format;
			}
			else if (*format >= '1' && *format <= '9')
			{
				width = 0;
				for (; *format >= '0' && *format <= '9'; ++ format)
					width = width * 10 + *format - '0';
			}

			// Process format precision
			if (*format == '.')
			{
				++ format;
				if (*format == '*')
				{
					flags |= FORMAT_FLAG_EXT_PRECISION;
					++ format;
				}
				else
				{
					precision = 0;
					for (; *format >= '0' && *format <= '9'; ++ format)
						precision = precision * 10 + *format - '0';
				}
			}

			// Process format type
			switch (*format)
			{
				case 'h':
					if (format[1] == 'h')
					{
						type = FORMAT_TYPE_HH;
						format += 2;
					}
					else
					{
						type = FORMAT_TYPE_H;
						++ format;
					}
					break;

				case 'l':
					if (format[1] == 'l')
					{
						type = FORMAT_TYPE_LL;
						format += 2;
					}
					else
					{
						type = FORMAT_TYPE_L;
						++ format;
					}
					break;

				case 'L':
					type = FORMAT_TYPE_LL;
					++ format;
					break;

				case 'j':
					type = FORMAT_TYPE_J;
					++ format;
					break;

				case 'z':
					type = FORMAT_TYPE_Z;
					++ format;
					break;

				case 't':
					type = FORMAT_TYPE_T;
					++ format;
					break;
			}

			// Check format string end
			if (!*format)
				break;

			// Format
			switch (*format)
			{
				// Single '%'
				case '%':
					if (i ++ < size)
						*next ++ = '%';
					break;

				// Get the number of characters written
				case 'n':
					switch (type)
					{
						case FORMAT_TYPE_HH:
							* va_arg(list, unsigned char *) = next - buffer;
							break;
						case FORMAT_TYPE_H:
							* va_arg(list, unsigned short *) = next - buffer;
							break;
						case FORMAT_TYPE_L:
							* va_arg(list, unsigned long *) = next - buffer;
							break;
						case FORMAT_TYPE_LL:
							* va_arg(list, unsigned long long *) = next - buffer;
							break;
						case FORMAT_TYPE_J:
							* va_arg(list, uintmax_t *) = next - buffer;
							break;
						case FORMAT_TYPE_Z:
							* va_arg(list, size_t *) = next - buffer;
							break;
						case FORMAT_TYPE_T:
							* va_arg(list, ptrdiff_t *) = next - buffer;
							break;
						default:
							* va_arg(list, unsigned int *) = next - buffer;
							break;
					}
					break;

				// Character
				case 'C':
					type = FORMAT_TYPE_L;
				case 'c':
					// Wide characters are not supported
					{
						// Variables
						char ch;

						// Read parameters in order
						if (flags & FORMAT_FLAG_EXT_WIDTH)
							width = va_arg(list, int);
						if (flags & FORMAT_FLAG_EXT_PRECISION)
							precision = va_arg(list, int);

						// Read value
						ch = (char) va_arg(list, int);

						// Negative widths are aligned left
						if (width < 0)
						{
							width = -width;
							flags |= FORMAT_FLAG_LEFT;
						}

						// update width
						if (width > 0)
							-- width;

						// Padding before
						if (width && !(flags & FORMAT_FLAG_LEFT))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = ' ';

						// Write character
						if (i ++ < size)
							*next ++ = ch;

						// Padding after
						for (; width > 0; -- width)
							if (i ++ < size)
								*next ++ = ' ';
					}
					break;

				// String
				case 'S':
					type = FORMAT_TYPE_L;
				case 's':
					// Wide strings are not supported
					{
						// Variables
						const char * str;

						// Read parameters in order
						if (flags & FORMAT_FLAG_EXT_WIDTH)
							width = va_arg(list, int);
						if (flags & FORMAT_FLAG_EXT_PRECISION)
							precision = va_arg(list, int);
						str = va_arg(list, char *);

						// Negative widths are aligned left
						if (width < 0)
						{
							width = -width;
							flags |= FORMAT_FLAG_LEFT;
						}

						// Find length and padding
						if (precision < 0)
							for (precision = 0; str[precision]; ++ precision) {}
						width = (precision < width) ? width - precision : 0;

						// Padding before
						if (width && !(flags & FORMAT_FLAG_LEFT))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = ' ';

						// Copy string
						for (; precision > 0; -- precision)
							if (i ++ < size)
								*next ++ = *str ++;

						// Padding after
						for (; width > 0; -- width)
							if (i ++ < size)
								*next ++ = ' ';
					}
					break;

				// Signed decimal
				case 'd':
				case 'i':
				// Unsigned decimal
				case 'u':
					{
						// Variables
						size_t n;
						uintmax_t value;
						char sign = 0;
						char buffer[3 * sizeof(uintmax_t) + 1];

						// Read parameters in order
						if (flags & FORMAT_FLAG_EXT_WIDTH)
							width = va_arg(list, int);
						if (flags & FORMAT_FLAG_EXT_PRECISION)
							precision = va_arg(list, int);

						// Negative widths are aligned left
						if (width < 0)
						{
							width = -width;
							flags |= FORMAT_FLAG_LEFT;
						}

						// Read value
						switch (type)
						{
							case FORMAT_TYPE_HH:
								if (*format == 'u')
									value = (unsigned char) va_arg(list, int);
								else
									value = (signed char) va_arg(list, int);
								break;
							case FORMAT_TYPE_H:
								if (*format == 'u')
									value = (unsigned short) va_arg(list, int);
								else
									value = (short) va_arg(list, int);
								break;
							case FORMAT_TYPE_L:
								if (*format == 'u')
									value = va_arg(list, unsigned long);
								else
									value = va_arg(list, long);
								break;
							case FORMAT_TYPE_LL:
								if (*format == 'u')
									value = va_arg(list, unsigned long long);
								else
									value = va_arg(list, long long);
								break;
							case FORMAT_TYPE_J:
								if (*format == 'u')
									value = va_arg(list, uintmax_t);
								else
									value = va_arg(list, uintmax_t);
								break;
							case FORMAT_TYPE_Z:
								if (*format == 'u')
									value = va_arg(list, size_t);
								else
									value = (intptr_t) va_arg(list, size_t);	// Assuming sizeof(size_t) == sizeof(void *)
								break;
							case FORMAT_TYPE_T:
								if (*format == 'u')
									value = (uintptr_t) va_arg(list, ptrdiff_t);	// Assuming sizeof(ptrdiff_t) == sizeof(void *)
								else
									value = va_arg(list, ptrdiff_t);
								break;
							default:
								if (*format == 'u')
									value = va_arg(list, unsigned int);
								else
									value = va_arg(list, int);
						}

						// Calculate sign
						if ((*format == 'u') || (0 < (intmax_t) value))
						{
							if (flags & FORMAT_FLAG_BLANK)
								sign = ' ';
							if (flags & FORMAT_FLAG_SIGN)
								sign = '+';
						}
						else
						{
							sign = '-';
							value = - (intmax_t) value;
						}

						// Convert number
						for (n = 0; value; value /= 10)
							buffer[n ++] = '0' + value % 10;

						// Calculate padding
						width = (n < width) ? (width - n) : 0;
						if (width && sign)
							width -= 1;

						// Padding before
						if (width && !(flags & (FORMAT_FLAG_LEFT | FORMAT_FLAG_ZERO)))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = ' ';

						// Add Sign
						if (sign && (i ++ < size))
							*next ++ = sign;

						// Numeric padding
						if (width && (flags & FORMAT_FLAG_ZERO))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = '0';

						// Minimum digits
						if (n == 0 && (i ++ < size))
							*next ++ = '0';

						// Copy digits in reverse
						while (n -- > 0)
							if (i ++ < size)
								*next ++ = buffer[n];

						// Padding after
						for (; width > 0; -- width)
							if (i ++ < size)
								*next ++ = ' ';
					}
					break;


				// Octal
				case 'o':
				// Hexadecimal
				case 'x':
				case 'X':
					{
						// Variables
						size_t n;
						uintmax_t value;
						char sign = 0;
						char buffer[3 * sizeof(uintmax_t) + 1];

						// Read parameters in order
						if (flags & FORMAT_FLAG_EXT_WIDTH)
							width = va_arg(list, int);
						if (flags & FORMAT_FLAG_EXT_PRECISION)
							precision = va_arg(list, int);

						// Negative widths are aligned left
						if (width < 0)
						{
							width = -width;
							flags |= FORMAT_FLAG_LEFT;
						}

						// Read value
						switch (type)
						{
							case FORMAT_TYPE_HH:
								value = (unsigned char) va_arg(list, int);
								break;
							case FORMAT_TYPE_H:
								value = (unsigned short) va_arg(list, int);
								break;
							case FORMAT_TYPE_L:
								value = va_arg(list, unsigned long);
								break;
							case FORMAT_TYPE_LL:
								value = va_arg(list, unsigned long long);
								break;
							case FORMAT_TYPE_J:
								value = va_arg(list, uintmax_t);
								break;
							case FORMAT_TYPE_Z:
								value = va_arg(list, size_t);
								break;
							case FORMAT_TYPE_T:
								value = (uintptr_t) va_arg(list, ptrdiff_t);
								break;
							default:
								value = va_arg(list, unsigned int);
						}

						// Calculate sign
						if (flags & FORMAT_FLAG_BLANK)
							sign = ' ';
						if (flags & FORMAT_FLAG_SIGN)
							sign = '+';

						// Convert value
						if (*format == 'o')
						{
							for (n = 0; value; value >>= 3)
								buffer[n ++] = '0' + (value & 7);
						}
						else
						{
							const char * digits = (*format == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";
							for (n = 0; value; value >>= 4)
								buffer[n ++] = digits[value & 0xF];
						}

						// Calculate padding
						width = (n < width) ? (width - n) : 0;

						// Padding before
						if (width && !(flags & (FORMAT_FLAG_LEFT | FORMAT_FLAG_ZERO)))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = ' ';

						// Add Sign
						if (sign && (i ++ < size))
							*next ++ = sign;

						// Numeric padding
						if (width && (flags & FORMAT_FLAG_ZERO))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = '0';

						// Minimum digits
						if (n == 0 && (i ++ < size))
							*next ++ = '0';

						// Copy digits in reverse
						while (n -- > 0)
							if (i ++ < size)
								*next ++ = buffer[n];

						// Padding after
						for (; width > 0; -- width)
							if (i ++ < size)
								*next ++ = ' ';
					}
					break;

				// Address (hexadecimal)
				case 'p':
					{
						// Variables
						size_t n = 2 * sizeof(void *);
						uintptr_t value;

						// Read parameters in order
						if (flags & FORMAT_FLAG_EXT_WIDTH)
							width = va_arg(list, int);
						if (flags & FORMAT_FLAG_EXT_PRECISION)
							precision = va_arg(list, int);

						// Negative widths are aligned left
						if (width < 0)
						{
							width = -width;
							flags |= FORMAT_FLAG_LEFT;
						}

						// Read value
						value = (uintptr_t) va_arg(list, void *);

						// Calculate padding
						width = (n < width) ? (width - n) : 0;

						// Padding before
						if (width && !(flags & FORMAT_FLAG_LEFT))
							for (; width > 0; -- width)
								if (i ++ < size)
									*next ++ = ' ';

						// Print value
						while (n -- > 0)
							if (i ++ < size)
								*next ++ = "0123456789ABCDEF"[(value >> (n * 4)) & 0xF];

						// Padding after
						for (; width > 0; -- width)
							if (i ++ < size)
								*next ++ = ' ';
					}
					break;

				// Invalid type
				default:
					if (i ++ < size)
						*next ++ = *format;
			}
		}
		else
		{
			// Copy character
			if (i ++ < size)
				*next ++ = *format;
		}
	}

	// Terminate string
	if (n > 0)
		*next = 0;

	// Return the size of the string
	return i;
}
