#pragma once
#include "windows.h"
inline BOOL PathFileExistsA(LPCSTR p) noexcept { return (p && _access(p, 0) == 0) ? TRUE : FALSE; }
inline BOOL PathFileExistsW(LPCWSTR p) noexcept { return (p && _waccess(p, 0) == 0) ? TRUE : FALSE; }
#if defined(UNICODE) || defined(_UNICODE)
#define PathFileExists PathFileExistsW
#else
#define PathFileExists PathFileExistsA
#endif
