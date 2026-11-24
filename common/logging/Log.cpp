#include "stdafx.h"
#define DN_LOG_IMPL
#include "Log.h"
#undef g_Log
#undef g_SPLog
#undef g_TriggerLog
#undef g_QuestLog
#include "TimeSet.h"

#if defined( _GAMESERVER )
#include "DNUserSession.h"
#include "DNLogConnection.h"
#if defined( PRE_TRIGGER_UNITTEST_LOG )
CLog g_TriggerUnitTestLog;
#endif // #if defined( PRE_TRIGGER_UNITTEST_LOG )
#if defined( PRE_QUESTSCRIPT_LOG )
CLog g_QuestLog;
#endif // #if defined( PRE_QUESTSCRIPT_LOG )
#if defined( PRE_TRIGGER_LOG )
CLog g_TriggerLog;
#endif // #if defined( PRE_TRIGGER_LOG )
#elif defined( _MASTERSERVER )
extern TMasterConfig g_Config;
#include "DNLogConnection.h"
#include "DNUser.h"
#elif defined( _VILLAGESERVER )
#include "DNLogConnection.h"
#include "DNUserSession.h"
#elif defined( _LOGINSERVER )
#include "DNUserConnection.h"
#include "DNLogConnection.h"
#elif defined( _DBSERVER )
#include "DNLogConnection.h"
#elif defined( _CASHSERVER )
#include "DNLogConnection.h"
#include "DNUserRepository.h"
#endif // #if defined( _GAMESERVER )

CLog g_SPLog;
CLog g_Log;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

static const SLogSourceLoc s_UnknownLogLoc = { "unknown", 0, "unknown" };

// ---------------------------------------------------------------------------
// CLogSite forwarding implementations
// ---------------------------------------------------------------------------
void CLogSite::Log( LogType::eLogType eType, const char* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogVA( m_Loc, eType, 0, 0, 0, 0, fmt, args );
	va_end( args );
}

void CLogSite::Log( LogType::eLogType eType, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogVW( m_Loc, eType, 0, 0, 0, 0, fmt, args );
	va_end( args );
}

void CLogSite::Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogVA( m_Loc, eType, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, fmt, args );
	va_end( args );
}

void CLogSite::Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogVW( m_Loc, eType, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, fmt, args );
	va_end( args );
}

#if defined( _GAMESERVER ) || defined( _VILLAGESERVER )
void CLogSite::Log( LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogSessionV( m_Loc, eType, pSession, fmt, args );
	va_end( args );
}
#elif defined( _MASTERSERVER )
void CLogSite::Log( LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogSessionV( m_Loc, eType, pUser, fmt, args );
	va_end( args );
}
#elif defined( _LOGINSERVER )
void CLogSite::Log( LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogSessionV( m_Loc, eType, pUser, fmt, args );
	va_end( args );
}
#elif defined( _CASHSERVER )
void CLogSite::Log( LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );
	m_Log.LogSessionV( m_Loc, eType, pUser, fmt, args );
	va_end( args );
}
#endif

// ---------------------------------------------------------------------------
// CLog implementation
// ---------------------------------------------------------------------------
CLog::CLog(void) : m_pFile(NULL), m_nOldDay(0), m_nOldHour(0), m_cType(LOGTYPE_CRT)
{
	memset(&m_wszFileName, 0, sizeof(m_wszFileName));
	memset(&m_wszDirName, 0, sizeof(m_wszDirName));
	m_nCounter = 0;
	m_bActive = false;
	m_nServerID = 0;
}

CLog::~CLog(void)
{
	if (m_pFile)
		fclose(m_pFile);
}

void CLog::CalcCounter()
{
	if (m_cType == LOGTYPE_FILE) {
		std::vector<std::string> szVecFileList;
		FindFileListInDirectory(".\\log", "*.log", szVecFileList);
		int nCount = 0;
		const char *pszTemp, *pszTemp2;
		for (DWORD i = 0; i < szVecFileList.size(); i++) {
			pszTemp = _GetSubStrByCount(2, (char*)szVecFileList[i].c_str(), '_');
			if (!pszTemp) continue;
			pszTemp2 = _GetSubStrByCount(0, pszTemp, '.');
			if (!pszTemp2) continue;
			if (atoi(pszTemp2) > nCount) nCount = atoi(pszTemp2);
		}
		m_nCounter = nCount + 1;
	}
}

void CLog::Init(BYTE nType)
{
	_wcscpy(m_wszFileName, _countof(m_wszFileName), L"log", (int)wcslen(L"log"));
	_wcscpy(m_wszDirName, _countof(m_wszDirName), L"log", (int)wcslen(L"log"));

	CreateDirectoryW(m_wszDirName, NULL);

	m_cType = nType;
	CalcCounter();
}

void CLog::Init(const WCHAR *pwszFileName, BYTE nType)
{
	_wcscpy(m_wszFileName, _countof(m_wszFileName), pwszFileName, (int)wcslen(pwszFileName));
	_wcscpy(m_wszDirName, _countof(m_wszDirName), L"log", (int)wcslen(L"log"));
	CreateDirectoryW(m_wszDirName, NULL);

	m_cType = nType;
	CalcCounter();
}

void CLog::Init(const WCHAR *pwszDirName, const WCHAR *pwszFileName, BYTE nType)
{
	_wcscpy(m_wszFileName, _countof(m_wszFileName), pwszFileName, (int)wcslen(pwszFileName));
	_wcscpy(m_wszDirName, _countof(m_wszDirName), pwszDirName, (int)wcslen(pwszDirName));

	CreateDirectoryW(m_wszDirName, NULL);

	m_cType = nType;
	CalcCounter();
}

void CLog::LogTypeLog(LogType::eLogType eType, WCHAR* buf)
{
	switch (m_cType)
	{
	case LOGTYPE_CRT:
	{
		Display(buf);
	}
	break;

	case LOGTYPE_CRT_FILE_HOUR:
	case LOGTYPE_CRT_FILE_DAY:
	{
		Display(buf);
		Filelog(buf);
	}
	break;

	case LOGTYPE_FILEDB_DAY:
		Filelog(buf);
		break;

	default:
	{
#if defined( PRE_FILELOG )
		Filelog(buf);
#endif // #if defined( PRE_FILELOG )

#if defined(_FINAL_BUILD)
		switch (eType)
		{
		case LogType::_FILELOG:
		case LogType::_FILEDBLOG:
		case LogType::_DBSYSTEM_ERROR:
			Filelog(buf);
			break;
		}
#endif	// #if defined(_FINAL_BUILD)
	}
	}
}

void CLog::Display(wchar_t* buf)
{
	wprintf(L"%s", buf);
	fflush(stdout);
}

bool CLog::Filelog(wchar_t* buf)
{
	char temp_file_name[512] = { 0, };
	char szDirName[256] = { 0, };
	char szFileName[256] = { 0, };
	WideCharToMultiByte(CP_ACP, 0, m_wszDirName, -1, szDirName, _countof(szDirName), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_wszFileName, -1, szFileName, _countof(szFileName), NULL, NULL);
	SYSTEMTIME st;
	GetLocalTime(&st);

	int nDayKey = st.wYear * 10000 + st.wMonth * 100 + st.wDay;

#if defined (_DBSERVER) || defined (_CASHSERVER)
	m_LogLock.Lock();
#endif

	switch (m_cType)
	{
	case LOGTYPE_FILE:
	{
		_snprintf_s(temp_file_name, _countof(temp_file_name), _TRUNCATE, "%s/%s_%04u-%02u-%02u_%d.log",
			szDirName, szFileName, st.wYear, st.wMonth, st.wDay, m_nCounter);

		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}

		if (fopen_s(&m_pFile, temp_file_name, "a") != 0 || m_pFile == NULL)
		{
#if defined (_DBSERVER) || defined (_CASHSERVER)
			m_LogLock.UnLock();
#endif
			return false;
		}
	}
	break;

	case LOGTYPE_CRT_FILE_HOUR:
	case LOGTYPE_FILE_HOUR:
#if defined( PRE_TRIGGER_LOG )
	case LOGTYPE_CRT_FILE_TRIGGER_TEST:
#endif // #if defined( PRE_TRIGGER_LOG )
	{
		_snprintf_s(temp_file_name, _countof(temp_file_name), _TRUNCATE, "%s/%s_%04u-%02u-%02u-%02u.log",
			szDirName, szFileName, st.wYear, st.wMonth, st.wDay, st.wHour);

		if (m_nOldDay != nDayKey || m_nOldHour != (int)st.wHour || m_pFile == NULL)
		{
			m_nOldHour = (int)st.wHour;
			m_nOldDay = nDayKey;
			if (m_pFile)
			{
				fclose(m_pFile);
				m_pFile = NULL;
			}

			if (fopen_s(&m_pFile, temp_file_name, "a") != 0 || m_pFile == NULL)
			{
#if defined (_DBSERVER) || defined (_CASHSERVER)
				m_LogLock.UnLock();
#endif
				return false;
			}
		}
	}
	break;

	case LOGTYPE_CRT_FILE_DAY:
	case LOGTYPE_FILE_DAY:
	case LOGTYPE_FILEDB_DAY:
	{
		_snprintf_s(temp_file_name, _countof(temp_file_name), _TRUNCATE, "%s/%s_%04u-%02u-%02u.log",
			szDirName, szFileName, st.wYear, st.wMonth, st.wDay);

		if (m_nOldDay != nDayKey || m_pFile == NULL)
		{
			m_nOldDay = nDayKey;
			if (m_pFile)
			{
				fclose(m_pFile);
				m_pFile = NULL;
			}

			if (fopen_s(&m_pFile, temp_file_name, "a") != 0 || m_pFile == NULL)
			{
#if defined (_DBSERVER) || defined (_CASHSERVER)
				m_LogLock.UnLock();
#endif
				return false;
			}
		}
	}
	break;
	}

	if (m_pFile)
	{
		fputws(buf, m_pFile);
		fflush(m_pFile);
	}

#if defined (_DBSERVER) || defined (_CASHSERVER)
	m_LogLock.UnLock();
#endif
	return true;
}

void CLog::LogVA( const SLogSourceLoc& loc, LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, va_list args )
{
#if defined( SKIP_LOG )
#if defined( PRE_TRIGGER_LOG )
	if (m_cType != LOGTYPE_CRT_FILE_TRIGGER_TEST)
#endif
		return;
#endif

	char szRaw[4096] = { 0, };
	if (fmt)
		_vsnprintf_s(szRaw, _countof(szRaw), _TRUNCATE, fmt, args);

	wchar_t wszRaw[4096] = { 0, };
	MultiByteToWideChar(CP_ACP, 0, szRaw, -1, wszRaw, _countof(wszRaw));

	wchar_t wszFormatted[8192] = { 0, };
	DNFormatLogLineW(wszFormatted, _countof(wszFormatted), DNGetLogSeverityW(eType), loc, wszRaw);

	{
#if defined( PRE_DELETE_LOGLOCK )
#else
		ScopeLock<CSyncLock> lock(m_LogLock);
#endif

#if !defined( _FINAL_BUILD )
		const wchar_t* pSev = DNGetLogSeverityW(eType);
		if (wcscmp(pSev, L"ERROR") == 0)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		else if (wcscmp(pSev, L"WARN") == 0)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif

		LogTypeLog(eType, wszFormatted);

#if !defined( _FINAL_BUILD )
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}

	if (m_cType != LOGTYPE_CRT)
		DBLog(eType, m_nServerID, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, wszFormatted);
}

void CLog::LogVW( const SLogSourceLoc& loc, LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, va_list args )
{
#if defined( SKIP_LOG )
#if defined( PRE_TRIGGER_LOG )
	if (m_cType != LOGTYPE_CRT_FILE_TRIGGER_TEST)
#endif
		return;
#endif

	wchar_t wszRaw[8192] = { 0, };
	if (fmt)
		_vsnwprintf_s(wszRaw, _countof(wszRaw), _TRUNCATE, fmt, args);

	wchar_t wszFormatted[8192] = { 0, };
	DNFormatLogLineW(wszFormatted, _countof(wszFormatted), DNGetLogSeverityW(eType), loc, wszRaw);

	{
#if defined( PRE_DELETE_LOGLOCK )
#else
		ScopeLock<CSyncLock> lock(m_LogLock);
#endif

#if !defined( _FINAL_BUILD )
		const wchar_t* pSev = DNGetLogSeverityW(eType);
		if (wcscmp(pSev, L"ERROR") == 0)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		else if (wcscmp(pSev, L"WARN") == 0)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif

		LogTypeLog(eType, wszFormatted);

#if !defined( _FINAL_BUILD )
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}

	if (m_cType != LOGTYPE_CRT)
		DBLog(eType, m_nServerID, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, wszFormatted);
}

void CLog::Log(LogType::eLogType eType, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogVA(s_UnknownLogLoc, eType, 0, 0, 0, 0, fmt, args);
	va_end(args);
}

void CLog::Log(LogType::eLogType eType, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogVW(s_UnknownLogLoc, eType, 0, 0, 0, 0, fmt, args);
	va_end(args);
}

void CLog::Log(LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogVA(s_UnknownLogLoc, eType, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, fmt, args);
	va_end(args);
}

void CLog::Log(LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogVW(s_UnknownLogLoc, eType, iWorldID, uiAccountDBID, biCharDBID, uiSessionID, fmt, args);
	va_end(args);
}

#if defined( _GAMESERVER ) || defined( _VILLAGESERVER )

void CLog::LogSessionV(const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, va_list args)
{
	if (pSession)
		LogVW(loc, eType, pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), pSession->GetSessionID(), fmt, args);
	else
		LogVW(loc, eType, 0, 0, 0, 0, fmt, args);
}

void CLog::Log(LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogSessionV(s_UnknownLogLoc, eType, pSession, fmt, args);
	va_end(args);
}

#elif defined( _MASTERSERVER )

void CLog::LogSessionV(const SLogSourceLoc& loc, LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, va_list args)
{
	if (pUser)
		LogVW(loc, eType, g_Config.nWorldSetID, pUser->GetAccountDBID(), pUser->GetCharacterDBID(), pUser->GetSessionID(), fmt, args);
	else
		LogVW(loc, eType, 0, 0, 0, 0, fmt, args);
}

void CLog::Log(LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogSessionV(s_UnknownLogLoc, eType, pUser, fmt, args);
	va_end(args);
}

#elif defined( _LOGINSERVER )

void CLog::LogSessionV(const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, va_list args)
{
	if (pUser)
		LogVW(loc, eType, 0, pUser->GetAccountDBID(), 0, pUser->GetSessionID(), fmt, args);
	else
		LogVW(loc, eType, 0, 0, 0, 0, fmt, args);
}

void CLog::Log(LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogSessionV(s_UnknownLogLoc, eType, pUser, fmt, args);
	va_end(args);
}

#elif defined( _CASHSERVER )

void CLog::LogSessionV(const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, va_list args)
{
	if (pUser)
		LogVW(loc, eType, pUser->GetWorldID(), pUser->GetAccountDBID(), pUser->GetCharacterDBID(), 0, fmt, args);
	else
		LogVW(loc, eType, 0, 0, 0, 0, fmt, args);
}

void CLog::Log(LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	LogSessionV(s_UnknownLogLoc, eType, pUser, fmt, args);
	va_end(args);
}
#endif // #if defined( _GAMESERVER )

bool CLog::DBLog(LogType::eLogType eType, int nServerID, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, wchar_t* buf)
{
#if defined( _LOGSERVER ) || !defined( _SERVER )
#else
	if (eType == LogType::_FILELOG)
		return true;

	TLogFile TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.unLogType = static_cast<USHORT>(eType);
	TxPacket.unWorldSetID = static_cast<USHORT>(iWorldID);
	TxPacket.uiAccountDBID = uiAccountDBID;
	TxPacket.biCharDBID = biCharDBID;
	TxPacket.uiSessionID = uiSessionID;
	TxPacket.unServerType = static_cast<USHORT>(_SERVER_TYPEID);
	size_t nLen = wcslen(buf);
	if (nLen >= _countof(TxPacket.wszBuf))
		nLen = _countof(TxPacket.wszBuf) - 1;
	TxPacket.unLen = static_cast<USHORT>(nLen);
	TxPacket.nServerID = nServerID;

	wcsncpy_s(TxPacket.wszBuf, _countof(TxPacket.wszBuf), buf, nLen);

	if (g_pLogConnection)
	{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if (g_pLogConnection->GetSocketContextPtr() == NULL)
			return true;

		CScopeInterlocked Scope(&g_pLogConnection->GetSocketContextPtr()->m_lActiveCount);
		if (Scope.bIsDelete())
			return true;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

		int iLen = sizeof(TxPacket) - sizeof(TxPacket.wszBuf) + (sizeof(WCHAR)*(TxPacket.unLen + 1));
		g_pLogConnection->AddSendData(LOG_FILE, 0, reinterpret_cast<char*>(&TxPacket), iLen);
	}
#endif

	return true;
}

#if defined( _GAMESERVER )
#if defined( PRE_QUESTSCRIPT_LOG )

CScopeScriptLog::CScopeScriptLog(int iMapIndex, const char* pszFuncName)
	:m_iMapIndex(iMapIndex), m_strFuncName(pszFuncName)
{
	QueryPerformanceFrequency(&m_liFrequency);
	QueryPerformanceCounter(&m_liStartTime);
}

CScopeScriptLog::~CScopeScriptLog()
{
	QueryPerformanceCounter(&m_liCurTime);
	double dElapsed = (1000 * (m_liCurTime.QuadPart - m_liStartTime.QuadPart) / static_cast<double>(m_liFrequency.QuadPart));

	if (dElapsed > 0.f)
	{
		char szLog[256];
		_snprintf_s(szLog, _countof(szLog), _TRUNCATE, "[%.3f ms] MapIndex=%d FuncName:%s", dElapsed, m_iMapIndex, m_strFuncName.c_str());
		g_QuestLog.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, szLog);
	}
}

#endif // #if defined( PRE_QUESTSCRIPT_LOG )
#endif // #if defined( _GAMESERVER )

#if !defined( _FINAL_BUILD )

#include <iostream>

CPerformanceLog::CPerformanceLog(const char* pszName, int iOverTick/*=0*/)
	:m_strName(pszName), m_iOverTick(iOverTick)
{
	QueryPerformanceFrequency(&m_liFrequency);
	QueryPerformanceCounter(&m_liStartTime);
}

CPerformanceLog::~CPerformanceLog()
{
	QueryPerformanceCounter(&m_liCurTime);
	double dElapsed = (1000 * (m_liCurTime.QuadPart - m_liStartTime.QuadPart) / static_cast<double>(m_liFrequency.QuadPart));

	if (dElapsed >= m_iOverTick)
	{
		char szLog[256];
		_snprintf_s(szLog, _countof(szLog), _TRUNCATE, "[%.3f ms] Name:%s\n", dElapsed, m_strName.c_str());
		std::cout << szLog;
	}
}

#endif // #if !defined( _FINAL_BUILD )

#if defined( PRE_ADD_QUERYTIMELOG )

CQueryTimeLog::CQueryTimeLog(const char* pszQuery)
	:m_strQuery(pszQuery), m_dwStartTime(timeGetTime())
{
	if (g_SPLog.GetActive())
		g_SPLog.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, L"QueryTimeLog:%S ThreadID:%d", m_strQuery.c_str(), ::GetCurrentThreadId());
}

CQueryTimeLog::~CQueryTimeLog()
{
	DWORD dwElapsed = timeGetTime() - m_dwStartTime;
	if (dwElapsed > QUERYOVERTIME)
	{
		g_Log.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, L"[Query Over Time:%S] ThreadID:%d time=%d ms", m_strQuery.c_str(), ::GetCurrentThreadId(), dwElapsed);
	}
}

#endif // #if defined( PRE_ADD_QUERYTIMELOG )

#if defined( PRE_ADD_QUERYTIMELOG )
CTimeParamSet::CTimeParamSet(CQueryTimeLog* pQuery, __time64_t& pTime64, TIMESTAMP_STRUCT& dbtime, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID)
{
	CTimeSet TimeChecker(dbtime.year, dbtime.month, dbtime.day, dbtime.hour, dbtime.minute, dbtime.second);

	// NULL date
	if (!dbtime.year && !dbtime.month && !dbtime.day && !dbtime.hour && !dbtime.minute && !dbtime.second)
	{
		pTime64 = TimeChecker.GetTimeT64_LC();
		m_bValid = true;
		return;
	}

	if (!TimeChecker.CheckIntegrity())
	{
		m_bValid = false;
		g_Log.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_ERROR, iWorldID, uiAccountDBID, biCharDBID, 0,
			L"[%S] Invalid Time %d:%d:%d:%d:%d:%d", pQuery ? pQuery->GetQueryString() : "Unknown",
			dbtime.year, dbtime.month, dbtime.day, dbtime.hour, dbtime.minute, dbtime.second);
		return;
	}

	pTime64 = TimeChecker.GetTimeT64_LC();
	m_bValid = true;
}
#endif
