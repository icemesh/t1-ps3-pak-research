/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>

void MsgErr(const char* fmt, ...)
{
	va_list va;
	char buf[0x400];
	va_start(va, fmt);
	vsprintf_s(buf, 0x400, fmt, va);
	va_end(va);
	puts("\x1B[0;31mERROR: ========================================================================");
	printf("ERROR: %s", buf);
	puts("ERROR: ========================================================================\x1B[m");
}

void MsgWarn(const char* fmt, ...)
{
	va_list va;
	char buf[0x400];
	va_start(va, fmt);
	vsprintf_s(buf, 0x400, fmt, va);
	va_end(va);
	puts("\x1B[0;33mWARN:  ========================================================================");
	printf("WARN:  %s", buf);
	puts("WARN:  ========================================================================\x1B[m");
}
