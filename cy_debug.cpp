#include"cy_debug.h"
#include<stdarg.h>
#include<stdio.h>
#include<Windows.h>



#ifndef  CY_CHECKKEY

void cy_outputdebugstring(const char*format,...){
	va_list va;
	char stringbuffer[4096];
	DWORD thrid = ::GetCurrentThreadId();
	sprintf(stringbuffer,"thrid[%04u]",thrid);
	va_start(va,format);
	vsprintf(stringbuffer+11,format,va);
	va_end(va);
	

	::OutputDebugStringA(stringbuffer);
}
#endif