#ifndef OS_PLATFORM_FILE
#define OS_PLATFORM_FILE

#ifdef WIN32
	#if defined(_DEBUG)
#include "crtdbg.h"
#define OS_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define OS_NEW_ALIGN new(__FILE__, __LINE__)
	#else
#define OS_NEW new
#define OS_NEW_ALIGN new
	#endif
#else
#define OS_NEW new
#define OS_NEW_ALIGN new
#endif

#endif