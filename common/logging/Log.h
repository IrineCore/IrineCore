#pragma once

#include "LogFormat.h"
#include "CriticalSection.h"
#include <sqltypes.h>

#define LOGTYPE_CRT				1
#define LOGTYPE_FILE_HOUR		2
#define LOGTYPE_FILE_DAY		3
#define LOGTYPE_FILE			4	// Real-time file log
#define LOGTYPE_CRT_FILE_DAY	5	// Console + daily file
#define LOGTYPE_CRT_FILE_HOUR	6	// Console + hourly file
#define LOGTYPE_FILEDB_DAY		7	// DB + daily file
#if defined( PRE_TRIGGER_LOG )
#define LOGTYPE_CRT_FILE_TRIGGER_TEST	50	// Trigger log test
#endif // #if defined( PRE_TRIGGER_LOG )

enum Color
{
	_RED = 0,
	_BLUE,
	_GREEN,
	_ERROR = _RED,
	_WARNING = _BLUE,
	_SUCCESS = _GREEN,
};

struct LogType
{
	enum eLogType
	{
		_NORMAL = 0,
		_ERROR,
		_DANGERPOINT,
		_FILELOG,
		_HACK,
		_HACKSHIELD,
		_ITEMDUPLICATE,
		_WEAPONTYPE,
		_SESSIONCRASH,
		_ROOMCRASH,
		_UNREACHABLE,
		_LOADINGDISCONNECT,
		_ROOMFINALIZECRASH,
		_GAMECONNECTLOG,
		_GAMESERVERDELAY,
		_ENTERGAMECHECK,
		_DROPITEMPENALTY,
		_TREASUREPENALTY,
		_CLEARGRADEPENALTY,
		_PRELOADED_DYNAMICLOAD,
		_DELOBJECT_NAMEKEYFINDERR,
		_OCTREELOG,
		_INVALIDACTOR,
		_ACCEPTORERROR,
		_QUESTHACK,
		_ZOMBIEUSER,
		_EVENTLIST,
		_FARM,
		_GUILDWAR,
		_MOVEPACKET_SEQ,
		_SKILL_ACTION,
		_FILEDBLOG,
		_LADDER,
		_PVPROOM,
		_PING,
		_SENDTHREAD,
		_LASTVILLAGEERROR,
		_DBCONNECTIONLOG,
		_ZEROPOPULATION,
		_RESTRICTIP,
		_SOCIALCONNECT,
		_ASIASOFTLOG,
		_SHUTDOWN,
		_DBSYSTEM_ERROR,
		_COMEBACK,
		_BESTFRIEND,
		_BEGINNETGUILD,
		_REMAINEDACTOR,
		_PARTYERROR,
		_GAMEQUITREWARD,
		_PROFESSORK_SKILL,
		_DOORSLOG,
		_STEAMUSERCOUNT,
	};
};

inline const wchar_t* DNGetLogSeverityW( LogType::eLogType eType )
{
	switch( eType )
	{
	case LogType::_ERROR:
	case LogType::_DBSYSTEM_ERROR:
	case LogType::_SESSIONCRASH:
	case LogType::_ROOMCRASH:
	case LogType::_ROOMFINALIZECRASH:
	case LogType::_UNREACHABLE:
	case LogType::_ACCEPTORERROR:
	case LogType::_INVALIDACTOR:
	case LogType::_DELOBJECT_NAMEKEYFINDERR:
	case LogType::_PARTYERROR:
	case LogType::_LASTVILLAGEERROR:
		return L"ERROR";

	case LogType::_DANGERPOINT:
	case LogType::_HACK:
	case LogType::_HACKSHIELD:
	case LogType::_QUESTHACK:
	case LogType::_ITEMDUPLICATE:
	case LogType::_ZOMBIEUSER:
	case LogType::_GAMESERVERDELAY:
	case LogType::_LOADINGDISCONNECT:
	case LogType::_DROPITEMPENALTY:
	case LogType::_TREASUREPENALTY:
	case LogType::_CLEARGRADEPENALTY:
	case LogType::_MOVEPACKET_SEQ:
	case LogType::_RESTRICTIP:
	case LogType::_REMAINEDACTOR:
	case LogType::_ZEROPOPULATION:
	case LogType::_WEAPONTYPE:
		return L"WARN";

	default:
		return L"INFO";
	}
}

#if defined( _GAMESERVER ) || defined( _VILLAGESERVER )
class CDNUserSession;
#elif defined( _MASTERSERVER )
class CDNUser;
#elif defined( _LOGINSERVER )
class CDNUserConnection;
#elif defined( _CASHSERVER )
class CDNUserRepository;
#endif // #if defined( _GAMESERVER )

class CLog;

class CLogSite
{
	CLog&         m_Log;
	SLogSourceLoc m_Loc;

public:
	CLogSite( CLog& log, const char* pszFile, int nLine, const char* pszFunc )
		: m_Log( log )
	{
		m_Loc.pszFile = pszFile;
		m_Loc.nLine = nLine;
		m_Loc.pszFunc = pszFunc;
	}

	void Log( LogType::eLogType eType, const char* fmt, ... ) const;
	void Log( LogType::eLogType eType, const wchar_t* fmt, ... ) const;
	void Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, ... ) const;
	void Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, ... ) const;

#if defined( _GAMESERVER ) || defined( _VILLAGESERVER )
	void Log( LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, ... ) const;
#elif defined( _MASTERSERVER )
	void Log( LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, ... ) const;
#elif defined( _LOGINSERVER )
	void Log( LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, ... ) const;
#elif defined( _CASHSERVER )
	void Log( LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, ... ) const;
#endif

	void Init( BYTE nType ) const;
	void Init( const WCHAR* pwszFileName, BYTE nType ) const;
	void Init( const WCHAR* pwszDirName, const WCHAR* pwszFileName, BYTE nType ) const;
	void SetServerID( int nServerID ) const;
	void SetActive( bool bActive ) const;
	bool GetActive() const;
};

class CLog
{
protected:
#if defined( PRE_DELETE_LOGLOCK )
#else
	CSyncLock m_LogLock;
#endif // #if defined( PRE_DELETE_LOGLOCK )

#if defined (_DBSERVER) || defined (_CASHSERVER)
	CSyncLock m_LogLock;
#endif

	WCHAR m_wszFileName[256];
	WCHAR m_wszDirName[256];
	int m_nCounter;

	FILE* m_pFile;

	int	m_nOldDay;
	int	m_nOldHour;
	BYTE m_cType;
	bool m_bActive;
	int m_nServerID;

	void LogTypeLog(LogType::eLogType eType, WCHAR* buf);

	void Display(WCHAR* buf);
	bool Filelog(WCHAR* buf);
	bool DBLog( LogType::eLogType eType, int nServerID, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, wchar_t* buf );

	void CalcCounter();

public:
	CLog(void);
	virtual ~CLog();

	CLogSite At( const char* pszFile, int nLine, const char* pszFunc ) { return CLogSite( *this, pszFile, nLine, pszFunc ); }

	void Init(BYTE nType);										// Default directory and filename: "log"
	void Init(const WCHAR *pwszFileName, BYTE nType);
	void Init(const WCHAR *pwszDirName, const WCHAR *pwszFileName, BYTE nType);

	void SetServerID(int nServerID) { m_nServerID = nServerID;};

	void LogVA( const SLogSourceLoc& loc, LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, va_list args );
	void LogVW( const SLogSourceLoc& loc, LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, va_list args );

	void Log( LogType::eLogType eType, const char* fmt, ... );
	void Log( LogType::eLogType eType, const wchar_t* fmt, ... );

	void Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const char* fmt, ... );
	void Log( LogType::eLogType eType, int iWorldID, UINT uiAccountDBID, INT64 biCharDBID, UINT uiSessionID, const wchar_t* fmt, ... );

#if defined( _GAMESERVER ) || defined( _VILLAGESERVER )
	void Log( LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, ... );
	void LogSessionV( const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserSession* pSession, const wchar_t* fmt, va_list args );
#elif defined( _MASTERSERVER )
	void Log( LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, ... );
	void LogSessionV( const SLogSourceLoc& loc, LogType::eLogType eType, CDNUser* pUser, const wchar_t* fmt, va_list args );
#elif defined( _LOGINSERVER )
	void Log( LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, ... );
	void LogSessionV( const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserConnection* pUser, const wchar_t* fmt, va_list args );
#elif defined( _CASHSERVER )
	void Log( LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, ... );
	void LogSessionV( const SLogSourceLoc& loc, LogType::eLogType eType, CDNUserRepository* pUser, const wchar_t* fmt, va_list args );
#endif // #if defined( _GAMESERVER )

	void SetActive(bool bActive) {m_bActive = bActive;}
	bool GetActive() {return m_bActive;}
};

inline void CLogSite::Init( BYTE nType ) const { m_Log.Init( nType ); }
inline void CLogSite::Init( const WCHAR* pwszFileName, BYTE nType ) const { m_Log.Init( pwszFileName, nType ); }
inline void CLogSite::Init( const WCHAR* pwszDirName, const WCHAR* pwszFileName, BYTE nType ) const { m_Log.Init( pwszDirName, pwszFileName, nType ); }
inline void CLogSite::SetServerID( int nServerID ) const { m_Log.SetServerID( nServerID ); }
inline void CLogSite::SetActive( bool bActive ) const { m_Log.SetActive( bActive ); }
inline bool CLogSite::GetActive() const { return m_Log.GetActive(); }

#if defined( _GAMESERVER )

#if defined( PRE_TRIGGER_UNITTEST_LOG )
extern CLog g_TriggerUnitTestLog;
#endif // #if defined( PRE_TRIGGER_UNITTEST_LOG )

#if defined( PRE_TRIGGER_LOG )
extern CLog g_TriggerLog;
#endif // #if defined( PRE_TRIGGER_LOG )

#if defined( PRE_QUESTSCRIPT_LOG )
extern CLog g_QuestLog;
#endif // #if defined( PRE_QUESTSCRIPT_LOG )

extern CLog g_Log;
#else
extern CLog g_Log;
extern CLog g_SPLog;
#endif

#ifdef _FINAL_BUILD
#define	_DANGER_POINT() {}
#define	_DANGER_POINT_MSG(msg) {}
#else
#define	_DANGER_POINT() \
{	g_Log.Log( LogType::_DANGERPOINT, L"DANGER_POINT" ); }

#define	_DANGER_POINT_MSG(msg) \
{	g_Log.Log( LogType::_DANGERPOINT, L"DANGER_POINT msg[%s]", msg ); }
#endif

#if defined( _GAMESERVER )
#if defined( PRE_QUESTSCRIPT_LOG )

class CScopeScriptLog
{
public:

	CScopeScriptLog( int iMapIndex, const char* pszFuncName );
	~CScopeScriptLog();

private:

	int				m_iMapIndex;
	std::string		m_strFuncName;
	LARGE_INTEGER	m_liFrequency;
	LARGE_INTEGER	m_liStartTime;
	LARGE_INTEGER	m_liCurTime;

};

#endif // #if defined( PRE_QUESTSCRIPT_LOG )
#endif // #if defined( _GAMESERVER )

#if !defined( _FINAL_BUILD )

class CPerformanceLog
{
public:

	CPerformanceLog( const char* pszName, int iOverTick=0 );
	~CPerformanceLog();

private:

	std::string		m_strName;
	int				m_iOverTick;
	LARGE_INTEGER	m_liFrequency;
	LARGE_INTEGER	m_liStartTime;
	LARGE_INTEGER	m_liCurTime;
};
#else
class CPerformanceLog
{
public:
	CPerformanceLog( const char* pszName ) {}
	~CPerformanceLog() {}
};

#endif // #if !defined( _FINAL_BUILD )

class CExceptionCodeLog
{
public:
	CExceptionCodeLog(DWORD pExceptionCode, const wchar_t* pMsg, int nMainCmd=0, int nSubCmd=0) {
		::g_Log.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, L"[Exception Occurred] TID : %d, Code : 0x%X - %s MainCmd : %d SubCmd : %d", ::GetCurrentThreadId(), pExceptionCode, ((pMsg)?(pMsg):(L"<NONE>")), nMainCmd, nSubCmd);
	}
};

#if defined( _LOGINSERVER )
#if defined( PRE_LOGINDUMP_LOG )
class CExceptionCodeLogLoginServer
{	
public:
	CExceptionCodeLogLoginServer(DWORD pExceptionCode, const wchar_t* pMsg, int nMainCmd, int nSubCmd, int nReconnectStep, CSReconnectLogin *ReconnectData) {
		if(ReconnectData)
			::g_Log.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, L"[Exception Occurred] TID : %d, Code : 0x%X - %s MainCmd : %d SubCmd : %d Step:%d SessionID:%d AccountDBID:%d", ::GetCurrentThreadId(), pExceptionCode, ((pMsg)?(pMsg):(L"<NONE>")), nMainCmd, nSubCmd, nReconnectStep, ReconnectData->nSessionID, ReconnectData->nAccountDBID);
		else
			::g_Log.At(__FILE__, __LINE__, __FUNCTION__).Log(LogType::_FILELOG, L"[Exception Occurred] TID : %d, Code : 0x%X - %s MainCmd : %d SubCmd : %d Step:%d", ::GetCurrentThreadId(), pExceptionCode, ((pMsg)?(pMsg):(L"<NONE>")), nMainCmd, nSubCmd, nReconnectStep);
	}
};
#endif
#endif

#if defined( PRE_ADD_QUERYTIMELOG )

class CQueryTimeLog
{
public:
	CQueryTimeLog( const char* pszQuery );
	~CQueryTimeLog();
	const char* GetQueryString() {return m_strQuery.c_str();}

private:
	std::string m_strQuery;
	DWORD		m_dwStartTime;
};

#else

class CQueryTimeLog
{
public:
	CQueryTimeLog( const char* pszQuery ){}
	~CQueryTimeLog(){}
};

#endif // #if defined( PRE_ADD_QUERYTIMELOG )

#if defined( PRE_ADD_QUERYTIMELOG ) && !defined (_CLIENT)
class CTimeParamSet
{
public:
	CTimeParamSet(CQueryTimeLog* pQuery, __time64_t& pTime64, TIMESTAMP_STRUCT& dbtime, int iWorldID = 0, UINT uiAccountDBID = 0, INT64 biCharDBID = 0);
	bool IsValid() {return m_bValid;}
private:
	bool m_bValid;
};

#endif

#if !defined( DN_LOG_IMPL )
#define g_Log   g_Log.At(__FILE__, __LINE__, __FUNCTION__)
#define g_SPLog g_SPLog.At(__FILE__, __LINE__, __FUNCTION__)
#if defined( PRE_TRIGGER_LOG )
#define g_TriggerLog g_TriggerLog.At(__FILE__, __LINE__, __FUNCTION__)
#endif
#if defined( PRE_QUESTSCRIPT_LOG )
#define g_QuestLog g_QuestLog.At(__FILE__, __LINE__, __FUNCTION__)
#endif
#endif
