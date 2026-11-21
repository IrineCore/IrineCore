#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

struct SLogSourceLoc
{
	const char* pszFile;
	int         nLine;
	const char* pszFunc;
};

inline const char* DNLogBasename( const char* pszPath )
{
	if( !pszPath || !pszPath[0] )
		return "unknown";

	const char* pSlash1 = strrchr( pszPath, '\\' );
	const char* pSlash2 = strrchr( pszPath, '/' );
	const char* pLast = ( pSlash1 > pSlash2 ) ? pSlash1 : pSlash2;
	return pLast ? ( pLast + 1 ) : pszPath;
}

// Formats: "<YYYY-MM-DD HH:MM:SS.mmm> [<severity>] [<file>:<line>] [<function>] : <message>\n"
// Strips trailing CR/LF from message body before appending a single '\n'.
inline int DNFormatLogLineW( wchar_t* pOut, size_t cchOut,
							 const wchar_t* pwszSeverity,
							 const SLogSourceLoc& loc,
							 const wchar_t* pwszMsg )
{
	if( !pOut || cchOut == 0 )
		return 0;

	SYSTEMTIME st;
	GetLocalTime( &st );

	const char* pszFile = DNLogBasename( loc.pszFile );
	const char* pszFunc = ( loc.pszFunc && loc.pszFunc[0] ) ? loc.pszFunc : "unknown";
	const wchar_t* pSev = ( pwszSeverity && pwszSeverity[0] ) ? pwszSeverity : L"INFO";
	const wchar_t* pMsg = pwszMsg ? pwszMsg : L"";

	size_t nMsgLen = wcslen( pMsg );
	while( nMsgLen > 0 && ( pMsg[nMsgLen - 1] == L'\n' || pMsg[nMsgLen - 1] == L'\r' ) )
		--nMsgLen;

	int nWritten = _snwprintf_s(
		pOut, cchOut, _TRUNCATE,
		L"%04u-%02u-%02u %02u:%02u:%02u.%03u [%s] [%S:%d] [%S] : %.*s\n",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		pSev, pszFile, loc.nLine, pszFunc,
		static_cast<int>( nMsgLen ), pMsg );
	return ( nWritten < 0 ) ? static_cast<int>( cchOut - 1 ) : nWritten;
}

inline int DNFormatLogLineA( char* pOut, size_t cchOut,
							 const char* pszSeverity,
							 const SLogSourceLoc& loc,
							 const char* pszMsg )
{
	if( !pOut || cchOut == 0 )
		return 0;

	SYSTEMTIME st;
	GetLocalTime( &st );

	const char* pszFile = DNLogBasename( loc.pszFile );
	const char* pszFunc = ( loc.pszFunc && loc.pszFunc[0] ) ? loc.pszFunc : "unknown";
	const char* pSev = ( pszSeverity && pszSeverity[0] ) ? pszSeverity : "INFO";
	const char* pMsg = pszMsg ? pszMsg : "";

	size_t nMsgLen = strlen( pMsg );
	while( nMsgLen > 0 && ( pMsg[nMsgLen - 1] == '\n' || pMsg[nMsgLen - 1] == '\r' ) )
		--nMsgLen;

	int nWritten = _snprintf_s(
		pOut, cchOut, _TRUNCATE,
		"%04u-%02u-%02u %02u:%02u:%02u.%03u [%s] [%s:%d] [%s] : %.*s\n",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		pSev, pszFile, loc.nLine, pszFunc,
		static_cast<int>( nMsgLen ), pMsg );
	return ( nWritten < 0 ) ? static_cast<int>( cchOut - 1 ) : nWritten;
}
