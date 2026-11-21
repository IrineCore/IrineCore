// stdafx.cpp : source file that includes just the standard includes
// DragonNest.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "LogFormat.h"

#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
bool g_bInvalidDisconnect = false;

void _WriteLogEx( const char* pszFile, int nLine, const char* pszFunc, int nLogType, const char *szStr, ... )
{
#ifdef _FILELOG_PACKET
	if( nLogType != 1 ) return;
#elif _FILELOG_SPOS
	if( nLogType != 0 ) return;
#endif
	va_list list;
	char szTemp[8192] = { 0, };
	va_start( list, szStr );
	_vsnprintf_s( szTemp, _countof(szTemp), _TRUNCATE, szStr, list );
	va_end( list );

	std::wstring wStr;
	ToWideString( szTemp, wStr );
	::g_Log.At( pszFile, nLine, pszFunc ).Log( LogType::_FILELOG, L"%s", wStr.c_str() );
}

void _WriteLog( int nLogType, const char *szStr, ... )
{
#ifdef _FILELOG_PACKET
	if( nLogType != 1 ) return;
#elif _FILELOG_SPOS
	if( nLogType != 0 ) return;
#endif
	va_list list;
	char szTemp[8192] = { 0, };
	va_start( list, szStr );
	_vsnprintf_s( szTemp, _countof(szTemp), _TRUNCATE, szStr, list );
	va_end( list );

	std::wstring wStr;
	ToWideString( szTemp, wStr );
	::g_Log.At( __FILE__, __LINE__, __FUNCTION__ ).Log( LogType::_FILELOG, L"%s", wStr.c_str() );
}

void _WriteLogA( const char *szStr, ... )
{
	va_list list;
	char szTemp[8192] = { 0, };

	va_start( list, szStr );
	_vsnprintf_s( szTemp, _countof(szTemp), _TRUNCATE, szStr, list );
	va_end( list );

	std::wstring wStr;
	ToWideString( szTemp, wStr );
	::g_Log.At( __FILE__, __LINE__, __FUNCTION__ ).Log( LogType::_FILELOG, L"%s", wStr.c_str() );
}

void _WriteDisplayLogEx( const char* pszFile, int nLine, const char* pszFunc, int Logtype, const char *szStr, ... )
{
	va_list list;
	char szString[8192] = { 0, };
	va_start( list, szStr );
	_vsnprintf_s( szString, _countof(szString), _TRUNCATE, szStr, list );
	va_end( list );

	LogType::eLogType eType = LogType::_FILELOG;
	const char* pszSev = "INFO";
	switch( Logtype )
	{
	case 1: eType = LogType::_NORMAL;      pszSev = "INFO";  break;
	case 2: eType = LogType::_DANGERPOINT; pszSev = "WARN";  break;
	case 3: eType = LogType::_ERROR;       pszSev = "ERROR"; break;
	case 4: eType = LogType::_FILELOG;     pszSev = "DEBUG"; break;
	default: break;
	}

	std::wstring wStr;
	ToWideString( szString, wStr );
	::g_Log.At( pszFile, nLine, pszFunc ).Log( eType, L"%s", wStr.c_str() );

	SLogSourceLoc loc = { pszFile, nLine, pszFunc };
	char szFormatted[8704] = { 0, };
	DNFormatLogLineA( szFormatted, _countof(szFormatted), pszSev, loc, szString );
	OutputDebug( "%s", szFormatted );
}

void _WriteDisplayLog( int Logtype, const char *szStr, ... )
{
	va_list list;
	char szString[8192] = { 0, };
	va_start( list, szStr );
	_vsnprintf_s( szString, _countof(szString), _TRUNCATE, szStr, list );
	va_end( list );

	_WriteDisplayLogEx( __FILE__, __LINE__, __FUNCTION__, Logtype, "%s", szString );
}
#else
void _WriteLogEx( const char* pszFile, int nLine, const char* pszFunc, int nLogType, const char *szStr, ... ){}
void _WriteLog( int nLogType, const char *szStr, ... ){}
void _WriteDisplayLogEx( const char* pszFile, int nLine, const char* pszFunc, int Logtype, const char *szStr, ... ){}
void _WriteDisplayLog( int Logtype, const char *szStr, ... ){}
void _WriteLogA( const char *szStr, ... ) {}
#endif
