#pragma once

namespace CDebugSet
{
	// Output debug information to log files with unified timestamp/severity/source format.
	void ToLogFileEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFormat, ... );
	void ToFileEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFileName, const char* szFormat, ... );
	void ToFileWithTimeEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFileName, const char* szFormat, ... );

	void ToLogFile( const char* szFormat, ... );
	void ToFile( const char* szFileName, const char* szFormat, ... );
	void ToFileWithTime( const char* szFileName, const char* szFormat, ... );
	void ClearFile( const char* szFileName );

	const char* GetPath();
	std::string GetCurTime();

	// Absolute directory path must be configured before writing debug logs.
	bool Initialize( const char* pszFullDir, bool bLogFileFinalOpen = false );
	bool Finalize();
};

#if !defined( DN_DEBUGSET_IMPL )
#define ToLogFile(...)      ToLogFileEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define ToFile(...)         ToFileEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define ToFileWithTime(...) ToFileWithTimeEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#endif
