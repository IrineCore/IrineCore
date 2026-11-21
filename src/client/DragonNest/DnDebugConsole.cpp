#include "StdAfx.h"
#define DN_DEBUGCONSOLE_IMPL
#include "DnDebugConsole.h"
#include "LogFormat.h"
#include <stdio.h>

namespace DnDebugConsole
{
	namespace
	{
		CRITICAL_SECTION g_csConsole;
		bool g_bLockInit = false;
		bool g_bInitialized = false;
		bool g_bAllocated = false;
		FILE* g_pStdout = NULL;
		FILE* g_pStderr = NULL;
		FILE* g_pStdin = NULL;

		const wchar_t* MapCategoryToSeverity( const wchar_t* pwszCategory, bool& bKeepCategoryTag )
		{
			bKeepCategoryTag = false;
			if( !pwszCategory || !pwszCategory[0] )
				return L"INFO";

			if( _wcsicmp( pwszCategory, L"ERR" ) == 0 || _wcsicmp( pwszCategory, L"ERROR" ) == 0 )
				return L"ERROR";
			if( _wcsicmp( pwszCategory, L"WARN" ) == 0 || _wcsicmp( pwszCategory, L"WARNING" ) == 0 )
				return L"WARN";
			if( _wcsicmp( pwszCategory, L"DEBUG" ) == 0 )
				return L"DEBUG";
			if( _wcsicmp( pwszCategory, L"INFO" ) == 0 || _wcsicmp( pwszCategory, L"LOG" ) == 0 )
				return L"INFO";

			bKeepCategoryTag = true;
			return L"INFO";
		}

		WORD GetCategoryColor( const wchar_t* pwszCategory )
		{
			if( !pwszCategory )
				return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

			if( _wcsicmp( pwszCategory, L"ERR" ) == 0 || _wcsicmp( pwszCategory, L"ERROR" ) == 0 )
				return FOREGROUND_RED | FOREGROUND_INTENSITY;
			if( _wcsicmp( pwszCategory, L"WARN" ) == 0 || _wcsicmp( pwszCategory, L"NET" ) == 0 )
				return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			if( _wcsicmp( pwszCategory, L"INSTANCE" ) == 0 || _wcsicmp( pwszCategory, L"STAGE" ) == 0 )
				return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			if( _wcsicmp( pwszCategory, L"BOT" ) == 0 || _wcsicmp( pwszCategory, L"PARTY" ) == 0 )
				return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			if( _wcsicmp( pwszCategory, L"LOOT" ) == 0 || _wcsicmp( pwszCategory, L"DICE" ) == 0 || _wcsicmp( pwszCategory, L"EXP" ) == 0 )
				return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

			return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}

		void WriteLineLocked( const SLogSourceLoc& loc, const wchar_t* pwszCategory, const wchar_t* pwszMessage )
		{
			bool bKeepCat = false;
			const wchar_t* pSev = MapCategoryToSeverity( pwszCategory, bKeepCat );
			const wchar_t* pMsg = pwszMessage ? pwszMessage : L"";

			wchar_t wszBody[4096] = { 0, };
			if( bKeepCat && pwszCategory && pwszCategory[0] )
				_snwprintf_s( wszBody, _countof(wszBody), _TRUNCATE, L"[%s] %s", pwszCategory, pMsg );
			else
				wcsncpy_s( wszBody, _countof(wszBody), pMsg, _TRUNCATE );

			wchar_t wszLine[4608] = { 0, };
			DNFormatLogLineW( wszLine, _countof(wszLine), pSev, loc, wszBody );

			const wchar_t* pSplit = wcsstr( wszLine, L" : " );
			size_t nPrefixLen = pSplit ? static_cast<size_t>( ( pSplit - wszLine ) + 3 ) : 0;

			HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
			DWORD dwMode = 0;
			if( hOut && hOut != INVALID_HANDLE_VALUE && GetConsoleMode( hOut, &dwMode ) )
			{
				CONSOLE_SCREEN_BUFFER_INFO csbi;
				WORD wOrigAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				if( GetConsoleScreenBufferInfo( hOut, &csbi ) )
					wOrigAttr = csbi.wAttributes;

				DWORD dwWritten = 0;
				if( nPrefixLen > 0 )
				{
					SetConsoleTextAttribute( hOut, GetCategoryColor( pwszCategory ) );
					WriteConsoleW( hOut, wszLine, (DWORD)nPrefixLen, &dwWritten, NULL );
					SetConsoleTextAttribute( hOut, wOrigAttr );
					WriteConsoleW( hOut, wszLine + nPrefixLen, (DWORD)wcslen( wszLine + nPrefixLen ), &dwWritten, NULL );
				}
				else
				{
					WriteConsoleW( hOut, wszLine, (DWORD)wcslen( wszLine ), &dwWritten, NULL );
				}
				return;
			}

			wprintf( L"%s", wszLine );
			fflush( stdout );
		}
	}

	void Initialize()
	{
		if( g_bInitialized )
			return;

		if( !g_bLockInit )
		{
			InitializeCriticalSection( &g_csConsole );
			g_bLockInit = true;
		}

		if( GetConsoleWindow() == NULL )
		{
			if( AllocConsole() )
				g_bAllocated = true;
			else
				AttachConsole( ATTACH_PARENT_PROCESS );
		}

		SetConsoleTitleW( L"DragonNest Debug Console" );
		SetConsoleOutputCP( CP_UTF8 );

		freopen_s( &g_pStdout, "CONOUT$", "w", stdout );
		freopen_s( &g_pStderr, "CONOUT$", "w", stderr );
		freopen_s( &g_pStdin, "CONIN$", "r", stdin );

		HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
		if( hOut && hOut != INVALID_HANDLE_VALUE )
		{
			COORD bufSize = { 160, 5000 };
			SetConsoleScreenBufferSize( hOut, bufSize );
		}

		// Disable QuickEdit so clicking inside the console never pauses the game thread.
		HANDLE hIn = GetStdHandle( STD_INPUT_HANDLE );
		DWORD dwInMode = 0;
		if( hIn && hIn != INVALID_HANDLE_VALUE && GetConsoleMode( hIn, &dwInMode ) )
		{
			dwInMode |= ENABLE_EXTENDED_FLAGS;
			dwInMode &= ~ENABLE_QUICK_EDIT_MODE;
			SetConsoleMode( hIn, dwInMode );
		}

		// Prevent accidental process termination from clicking the console [X] button.
		HWND hConWnd = GetConsoleWindow();
		if( hConWnd )
		{
			HMENU hSysMenu = GetSystemMenu( hConWnd, FALSE );
			if( hSysMenu )
				EnableMenuItem( hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED );
		}

		g_bInitialized = true;
		LogWEx( __FILE__, __LINE__, __FUNCTION__, L"SYS", L"DragonNest Debug Console initialized (scrollback=5000, QuickEdit=OFF)." );
	}

	void Finalize()
	{
		if( !g_bInitialized )
			return;

		LogWEx( __FILE__, __LINE__, __FUNCTION__, L"SYS", L"DragonNest Debug Console shutting down." );
		g_bInitialized = false;

		if( g_pStdout ) { fclose( g_pStdout ); g_pStdout = NULL; }
		if( g_pStderr ) { fclose( g_pStderr ); g_pStderr = NULL; }
		if( g_pStdin ) { fclose( g_pStdin ); g_pStdin = NULL; }

		if( g_bAllocated )
		{
			FreeConsole();
			g_bAllocated = false;
		}

		if( g_bLockInit )
		{
			DeleteCriticalSection( &g_csConsole );
			g_bLockInit = false;
		}
	}

	bool IsInitialized()
	{
		return g_bInitialized;
	}

	void LogVEx( const char* pszFile, int nLine, const char* pszFunc, const wchar_t* pwszCategory, const wchar_t* pwszFmt, va_list args )
	{
		if( !g_bInitialized || !pwszFmt )
			return;

		wchar_t wszBuffer[4096] = { 0, };
		_vsnwprintf_s( wszBuffer, _countof(wszBuffer), _TRUNCATE, pwszFmt, args );

		SLogSourceLoc loc = { pszFile, nLine, pszFunc };
		EnterCriticalSection( &g_csConsole );
		WriteLineLocked( loc, pwszCategory, wszBuffer );
		LeaveCriticalSection( &g_csConsole );
	}

	void LogWEx( const char* pszFile, int nLine, const char* pszFunc, const wchar_t* pwszCategory, const wchar_t* pwszFmt, ... )
	{
		if( !g_bInitialized || !pwszFmt )
			return;

		va_list args;
		va_start( args, pwszFmt );
		LogVEx( pszFile, nLine, pszFunc, pwszCategory, pwszFmt, args );
		va_end( args );
	}

	void LogAEx( const char* pszFile, int nLine, const char* pszFunc, const char* pszCategory, const char* pszFmt, ... )
	{
		if( !g_bInitialized || !pszFmt )
			return;

		char szBuffer[4096] = { 0, };
		va_list args;
		va_start( args, pszFmt );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, pszFmt, args );
		va_end( args );

		wchar_t wszCategory[64] = L"INFO";
		if( pszCategory && pszCategory[0] )
			MultiByteToWideChar( CP_ACP, 0, pszCategory, -1, wszCategory, _countof(wszCategory) );

		wchar_t wszMessage[4096] = { 0, };
		MultiByteToWideChar( CP_ACP, 0, szBuffer, -1, wszMessage, _countof(wszMessage) );

		SLogSourceLoc loc = { pszFile, nLine, pszFunc };
		EnterCriticalSection( &g_csConsole );
		WriteLineLocked( loc, wszCategory, wszMessage );
		LeaveCriticalSection( &g_csConsole );
	}

	void LogV( const wchar_t* pwszCategory, const wchar_t* pwszFmt, va_list args )
	{
		LogVEx( "unknown", 0, "unknown", pwszCategory, pwszFmt, args );
	}

	void LogW( const wchar_t* pwszCategory, const wchar_t* pwszFmt, ... )
	{
		if( !g_bInitialized || !pwszFmt )
			return;

		va_list args;
		va_start( args, pwszFmt );
		LogVEx( "unknown", 0, "unknown", pwszCategory, pwszFmt, args );
		va_end( args );
	}

	void LogA( const char* pszCategory, const char* pszFmt, ... )
	{
		if( !g_bInitialized || !pszFmt )
			return;

		char szBuffer[4096] = { 0, };
		va_list args;
		va_start( args, pszFmt );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, pszFmt, args );
		va_end( args );

		wchar_t wszCategory[64] = L"INFO";
		if( pszCategory && pszCategory[0] )
			MultiByteToWideChar( CP_ACP, 0, pszCategory, -1, wszCategory, _countof(wszCategory) );

		wchar_t wszMessage[4096] = { 0, };
		MultiByteToWideChar( CP_ACP, 0, szBuffer, -1, wszMessage, _countof(wszMessage) );

		SLogSourceLoc loc = { "unknown", 0, "unknown" };
		EnterCriticalSection( &g_csConsole );
		WriteLineLocked( loc, wszCategory, wszMessage );
		LeaveCriticalSection( &g_csConsole );
	}
}
