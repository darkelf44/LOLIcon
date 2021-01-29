// Include headers
#include <common.h>

// Debug logging
void logf(Severity level, const char * msg, ...)
{
	va_list list;
	va_start(list, msg);
	logv(level, msg, list);
	va_end(list);
}

void logv(Severity level, const char * msg, va_list list)
{
	// Variables
	static char buffer[0x100];
	size_t n;
	
	// Compose message
	snprintf(buffer, sizeof(buffer), "[*%08x] ", (uint32_t) ksceKernelGetProcessId());
	n = strlen(buffer);
	vsnprintf(buffer + n, sizeof(buffer) - n, msg, list);
	n = strlen(buffer + n) + n;
	buffer[n ++] = '\n';
	
	// Write to log file
	{
		SceUID fd = ksceIoOpen("ux0:/lolimenu/log.txt", SCE_O_RDWR|SCE_O_APPEND|SCE_O_CREAT, 0777);
		if (fd > 0)
		{
			ksceIoWrite(fd, buffer, n);
			ksceIoClose(fd);
		}
	}
}
