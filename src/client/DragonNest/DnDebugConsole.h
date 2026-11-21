#pragma once

#include <stdarg.h>

namespace DnDebugConsole
{
	void Initialize();
	void Finalize();
	bool IsInitialized();

	void LogAEx( const char* pszFile, int nLine, const char* pszFunc, const char* pszCategory, const char* pszFmt, ... );
	void LogWEx( const char* pszFile, int nLine, const char* pszFunc, const wchar_t* pwszCategory, const wchar_t* pwszFmt, ... );
	void LogVEx( const char* pszFile, int nLine, const char* pszFunc, const wchar_t* pwszCategory, const wchar_t* pwszFmt, va_list args );

	void LogA( const char* pszCategory, const char* pszFmt, ... );
	void LogW( const wchar_t* pwszCategory, const wchar_t* pwszFmt, ... );
	void LogV( const wchar_t* pwszCategory, const wchar_t* pwszFmt, va_list args );
}

#if !defined( DN_DEBUGCONSOLE_IMPL )
#define LogA(...) LogAEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LogW(...) LogWEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#endif
