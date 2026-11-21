// stdafx.cpp : source file that includes just the standard includes
// DnLauncher.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" FILE* __cdecl __iob_func(void)
{
	return reinterpret_cast<FILE*>(reinterpret_cast<char*>(__acrt_iob_func(2)) - 0x40);
}
#endif
