// =============================================================================
// IrineCore — common/platform/interfaces/WideStringCompat.h
// Cross-platform 2-byte WCHAR (UTF-16LE) <-> UTF-8 conversion and CRT shims.
// =============================================================================
#pragma once

#include "PlatformTypes.h"
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace Irine {
namespace Platform {

std::string  WideToUtf8(const WCHAR* wstr, int len = -1);
std::wstring Utf8ToWide(const char* str, int len = -1);

} // namespace Platform
} // namespace Irine

#if !defined(_WIN32)

// Win32 MultiByte <-> WideChar compatibility on Linux (UTF-8 <-> UTF-16LE)
int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar);

// Case-insensitive string comparisons
int _stricmp(const char* s1, const char* s2);
int _strnicmp(const char* s1, const char* s2, size_t n);
int _wcsicmp(const WCHAR* s1, const WCHAR* s2);
int _wcsnicmp(const WCHAR* s1, const WCHAR* s2, size_t n);
inline int stricmp(const char* s1, const char* s2) { return _stricmp(s1, s2); }
inline int strnicmp(const char* s1, const char* s2, size_t n) { return _strnicmp(s1, s2, n); }
inline int wcsicmp(const WCHAR* s1, const WCHAR* s2) { return _wcsicmp(s1, s2); }
inline int wcsnicmp(const WCHAR* s1, const WCHAR* s2, size_t n) { return _wcsnicmp(s1, s2, n); }

// Safe memory and string copy helpers
errno_t memcpy_s(void* dest, size_t destSize, const void* src, size_t count);
errno_t memmove_s(void* dest, size_t destSize, const void* src, size_t count);
errno_t wmemcpy_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count);
errno_t strcpy_s(char* dest, size_t destSize, const char* src);
errno_t strncpy_s(char* dest, size_t destSize, const char* src, size_t count);
errno_t strcat_s(char* dest, size_t destSize, const char* src);
errno_t strncat_s(char* dest, size_t destSize, const char* src, size_t count);
errno_t wcscpy_s(WCHAR* dest, size_t destElements, const WCHAR* src);
errno_t wcsncpy_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count);
errno_t wcscat_s(WCHAR* dest, size_t destElements, const WCHAR* src);
errno_t wcsncat_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count);

template <size_t N> inline errno_t strcpy_s(char (&dest)[N], const char* src) { return strcpy_s(dest, N, src); }
template <size_t N> inline errno_t strncpy_s(char (&dest)[N], const char* src, size_t count) { return strncpy_s(dest, N, src, count); }
template <size_t N> inline errno_t strcat_s(char (&dest)[N], const char* src) { return strcat_s(dest, N, src); }
template <size_t N> inline errno_t strncat_s(char (&dest)[N], const char* src, size_t count) { return strncat_s(dest, N, src, count); }
template <size_t N> inline errno_t wcscpy_s(WCHAR (&dest)[N], const WCHAR* src) { return wcscpy_s(dest, N, src); }
template <size_t N> inline errno_t wcsncpy_s(WCHAR (&dest)[N], const WCHAR* src, size_t count) { return wcsncpy_s(dest, N, src, count); }
template <size_t N> inline errno_t wcscat_s(WCHAR (&dest)[N], const WCHAR* src) { return wcscat_s(dest, N, src); }
template <size_t N> inline errno_t wcsncat_s(WCHAR (&dest)[N], const WCHAR* src, size_t count) { return wcsncat_s(dest, N, src, count); }

// Numeric <-> wide/narrow conversions
int     _wtoi(const WCHAR* str);
long    _wtol(const WCHAR* str);
double  _wtof(const WCHAR* str);
int64_t _wtoi64(const WCHAR* str);
int64_t _atoi64(const char* str);
char*   _itoa(int val, char* buf, int radix);
WCHAR*  _itow(int val, WCHAR* buf, int radix);
WCHAR*  _ltow(long val, WCHAR* buf, int radix);
WCHAR*  _ultow(unsigned long val, WCHAR* buf, int radix);
WCHAR*  _i64tow(int64_t val, WCHAR* buf, int radix);
WCHAR*  _ui64tow(uint64_t val, WCHAR* buf, int radix);

// Case conversion
char*  _strupr(char* str);
char*  _strlwr(char* str);
WCHAR* _wcsupr(WCHAR* str);
WCHAR* _wcslwr(WCHAR* str);

// Safe file & time CRT helpers
errno_t fopen_s(FILE** pFile, const char* filename, const char* mode);
errno_t _wfopen_s(FILE** pFile, const WCHAR* filename, const WCHAR* mode);
errno_t localtime_s(struct tm* _Tm, const time_t* _Time);

// Formatted output for 2-byte WCHAR on Linux
int _vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list argptr);
int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...);
int sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...);
int _vsnwprintf(WCHAR* buffer, size_t count, const WCHAR* format, va_list argptr);
int _vsnwprintf_s(WCHAR* buffer, size_t sizeOfBuffer, size_t count, const WCHAR* format, va_list argptr);
int _snwprintf(WCHAR* buffer, size_t count, const WCHAR* format, ...);
int _snwprintf_s(WCHAR* buffer, size_t sizeOfBuffer, size_t count, const WCHAR* format, ...);
int swprintf_s(WCHAR* buffer, size_t sizeOfBuffer, const WCHAR* format, ...);

// Legacy MSVC _CRT_NON_CONFORMING_SWPRINTFS 2-arg swprintf / _swprintf overloads
inline int swprintf(WCHAR* buffer, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf(buffer, 65536, format, args);
    va_end(args);
    return r;
}

inline int _swprintf(WCHAR* buffer, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf(buffer, 65536, format, args);
    va_end(args);
    return r;
}

template <size_t N>
inline int sprintf_s(char (&buffer)[N], const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = vsnprintf(buffer, N, format, args);
    va_end(args);
    return r;
}

template <size_t N>
inline int swprintf_s(WCHAR (&buffer)[N], const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf_s(buffer, N, N - 1, format, args);
    va_end(args);
    return r;
}

template <size_t N>
inline int _vsnprintf_s(char (&buffer)[N], size_t count, const char* format, va_list argptr)
{
    return _vsnprintf_s(buffer, N, count, format, argptr);
}

template <size_t N>
inline int _vsnwprintf_s(WCHAR (&buffer)[N], size_t count, const WCHAR* format, va_list argptr)
{
    return _vsnwprintf_s(buffer, N, count, format, argptr);
}

#ifndef _TRUNCATE
#define _TRUNCATE (static_cast<size_t>(-1))
#endif

#endif // !_WIN32
