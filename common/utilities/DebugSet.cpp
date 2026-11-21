#include "stdafx.h"
#define DN_DEBUGSET_IMPL
#include "DebugSet.h"
#include "LogFormat.h"
#include <atltime.h>
#include <strsafe.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

namespace CDebugSet
{
	#define MAX_STRLENGTH	(2048)

	char m_szFullDir[MAX_PATH] = {0};
	bool m_bLogFileFinalOpen = false;
	bool m_bLogWrite(false);
	CTime m_LogTime;
	std::string m_strLogFile;
	std::string m_strLogName;
	std::string m_strSTARTTIME;

	static void WriteFormattedToFile( const char* szFileName, const char* pszSeverity, const SLogSourceLoc& loc, const char* szBuffer )
	{
		char szFullPathFileName[MAX_PATH] = {0};
		StringCchCopyA( szFullPathFileName, _countof(szFullPathFileName), m_szFullDir );
		StringCchCatA( szFullPathFileName, _countof(szFullPathFileName), szFileName );

		char szLine[MAX_STRLENGTH + 512] = {0};
		DNFormatLogLineA( szLine, _countof(szLine), pszSeverity, loc, szBuffer );

		FILE *fp = NULL;
		if ( fopen_s( &fp, szFullPathFileName, "at" ) == 0 && fp )
		{
			fputs( szLine, fp );
			fclose( fp );
		}
	}

	void ToFileEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFileName, const char* szFormat, ... )
	{
		va_list argList;
		char szBuffer[MAX_STRLENGTH] = "";

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		SLogSourceLoc loc = { pszFile, nLine, pszFunc };
		WriteFormattedToFile( szFileName, "INFO", loc, szBuffer );
	}

	void ToFile( const char *szFileName, const char *szFormat, ... )
	{
		va_list argList;
		char szBuffer[MAX_STRLENGTH] = "";

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		SLogSourceLoc loc = { "unknown", 0, "unknown" };
		WriteFormattedToFile( szFileName, "INFO", loc, szBuffer );
	}

	std::string GetCurTime ()
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		char szTemp[64] = {0};
		_snprintf_s( szTemp, _countof(szTemp), _TRUNCATE, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
		return szTemp;
	}

	void ToFileWithTimeEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFileName, const char* szFormat, ... )
	{
		va_list argList;
		char szBuffer[MAX_STRLENGTH] = "";

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		SLogSourceLoc loc = { pszFile, nLine, pszFunc };
		WriteFormattedToFile( szFileName, "INFO", loc, szBuffer );
	}

	void ToFileWithTime( const char* szFileName, const char* szFormat, ... )
	{
		va_list argList;
		char szBuffer[MAX_STRLENGTH] = "";

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		SLogSourceLoc loc = { "unknown", 0, "unknown" };
		WriteFormattedToFile( szFileName, "INFO", loc, szBuffer );
	}

	void ToLogFileEx( const char* pszFile, int nLine, const char* pszFunc, const char* szFormat, ... )
	{
#if defined(_DEBUG)
		ASSERT(!m_strLogFile.empty()&&"log file were not set.");

		va_list argList;
		char szBuffer[MAX_STRLENGTH] = {0};

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		FILE *fp = NULL;
		if ( fopen_s( &fp, m_strLogFile.c_str(), "at" ) == 0 && fp )
		{
			if ( !m_bLogWrite )
			{
				fprintf( fp, "--------------------------------------------------------------\n" );
				fprintf( fp, "\tDragon Nest Log Report\n" );
				fprintf( fp, "--------------------------------------------------------------\n\n" );
				fprintf( fp, "Execution time : %s\n\n", m_strSTARTTIME.c_str() );
				m_bLogWrite = true;
			}

			SLogSourceLoc loc = { pszFile, nLine, pszFunc };
			char szLine[MAX_STRLENGTH + 512] = {0};
			DNFormatLogLineA( szLine, _countof(szLine), "DEBUG", loc, szBuffer );
			fputs( szLine, fp );
			fclose( fp );
		}
#endif
	}

	void ToLogFile( const char *szFormat, ... )
	{
#if defined(_DEBUG)
		ASSERT(!m_strLogFile.empty()&&"log file were not set.");

		va_list argList;
		char szBuffer[MAX_STRLENGTH] = {0};

		va_start( argList, szFormat );
		_vsnprintf_s( szBuffer, _countof(szBuffer), _TRUNCATE, szFormat, argList );
		va_end( argList );

		FILE *fp = NULL;
		if ( fopen_s( &fp, m_strLogFile.c_str(), "at" ) == 0 && fp )
		{
			if ( !m_bLogWrite )
			{
				fprintf( fp, "--------------------------------------------------------------\n" );
				fprintf( fp, "\tDragon Nest Log Report\n" );
				fprintf( fp, "--------------------------------------------------------------\n\n" );
				fprintf( fp, "Execution time : %s\n\n", m_strSTARTTIME.c_str() );
				m_bLogWrite = true;
			}

			SLogSourceLoc loc = { "unknown", 0, "unknown" };
			char szLine[MAX_STRLENGTH + 512] = {0};
			DNFormatLogLineA( szLine, _countof(szLine), "DEBUG", loc, szBuffer );
			fputs( szLine, fp );
			fclose( fp );
		}
#endif
	}

	void ClearFile ( const char *szFileName )
	{
		char szFullPathFileName[256] = {0};
		StringCchCopy( szFullPathFileName, 256, m_szFullDir );
		StringCchCat( szFullPathFileName, 256, szFileName );
		DeleteFile ( szFullPathFileName );
	}

	const char* GetPath ()	
	{ 
		return m_szFullDir;
	}

	bool Initialize ( const char *szAppPath, bool bLogFileFinalOpen )
	{
		Finalize();

		TCHAR szFullPath[MAX_PATH] = {0};
		StringCchCopy( szFullPath, MAX_PATH, szAppPath );
		CreateDirectory ( szFullPath, NULL );

		StringCchCopy( m_szFullDir, MAX_PATH, szAppPath );
		CreateDirectory ( m_szFullDir, NULL );

		m_bLogFileFinalOpen = bLogFileFinalOpen;
		m_strSTARTTIME = GetCurTime();

		CTime cCurTime = CTime::GetCurrentTime();
		char strFileName[MAX_STRLENGTH] = {0};
		//sprintf_s( strFileName, MAX_STRLENGTH,  "report_%d%d%d%d%d%d.txt", cCurTime.GetYear(), cCurTime.GetMonth(), cCurTime.GetDay(),
		//																cCurTime.GetHour(), cCurTime.GetMinute(), cCurTime.GetSecond() );
		sprintf_s( strFileName, MAX_STRLENGTH,  "report_dragonnest.txt" );

		m_strLogName = strFileName;
		m_strLogFile = m_szFullDir;
		m_strLogFile += "\\" + m_strLogName;

		m_bLogWrite = false;
		return true;
	}

	bool Finalize()
	{
		//if ( m_bLogWrite && m_bLogFileFinalOpen )
		//{
		//	char strPROCESS[MAX_STRLENGTH] = {0};
		//	sprintf_s( strPROCESS, MAX_STRLENGTH, "notepad %s", m_strLogName.c_str() );

		//	STARTUPINFO si;
		//	memset ( &si, 0, sizeof ( STARTUPINFO ) );
		//	si.cb = sizeof ( STARTUPINFO );
		//	si.dwFlags = 0;

		//	PROCESS_INFORMATION pi;
		//	pi.hProcess = NULL;
		//	pi.hThread = NULL;
		//	CreateProcess ( NULL, strPROCESS, NULL, NULL, FALSE, 0, NULL, m_szFullDir, &si, &pi );
		//	CloseHandle( pi.hProcess );
		//	CloseHandle( pi.hThread );
		//}

		return true;
	}
};