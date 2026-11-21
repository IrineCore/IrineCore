#pragma once

namespace LogWnd
{

struct LogLevel
{
	// 0 is the most critical severity; higher numbers are less severe.
	enum Enum
	{
		None = -1,	// Do not emit log
		Error = 0,
		Warning,
		Log
	};
};


const DWORD LOG_TARGET_WINDOW =	0x01;
const DWORD LOG_TARGET_FILE =	0x02;
const DWORD LOG_TARGET_ALL =	(LOG_TARGET_WINDOW|LOG_TARGET_FILE);

const DWORD LOG_WINDOW_CX = 500;
const DWORD LOG_WINDOW_CY = 800;

typedef bool (CALLBACK* PCMD_PROC)(const TCHAR* szCmd);

bool	CreateLog(bool bAttachDate = false, DWORD nLogTarget = LOG_TARGET_WINDOW|LOG_TARGET_FILE, const TCHAR* szFileName = NULL);
void	LogEx( const char* pszFile, int nLine, const char* pszFunc, int nLogLevel, const TCHAR* fmt, ... );
void	Log( int nLogLevel, const TCHAR* fmt, ... );
void	CmdLog( const char* fmt, ... );
void	SetCommandCallBack(PCMD_PROC pCallBack);
bool	DestroyLog();

#define LOGWND_LOG(...) LogWnd::LogEx(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define FUNC_LOG()      LogWnd::LogEx(__FILE__, __LINE__, __FUNCTION__, 7, __FUNCTIONW__)
};
