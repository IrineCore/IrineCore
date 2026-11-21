#include "stdafx.h"
#include "LogWnd.h"
#include "DnDebugConsole.h"
#include "LogFormat.h"
#include <time.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#pragma warning(disable:4311)
#pragma warning(disable:4312)

namespace LogWnd {


PCMD_PROC g_pCallBack = NULL;
#define CMD_EDIT 100

class __LogWnd
{
public:
	__LogWnd( bool bAttachDate = false , DWORD nTarget = LOG_TARGET_WINDOW, const TCHAR* szFilename = NULL );
	virtual ~__LogWnd();

	int		Log( int nLogLevel, const TCHAR* szLog, const char* pszFile = "unknown", int nLine = 0, const char* pszFunc = "unknown" );

private:

	void	CreateLogWindow();
	void	OnResize();
	static void	RePaint();

	bool			m_bAttachDate;
	unsigned int	m_nTarget;
	TCHAR			m_szFilename[1024];
	static HWND		m_hwnd;
	static HWND		m_hwndList;
	static HWND		m_hwndEdit;
	static WNDPROC  m_OldEditProc;

private:
	static	LRESULT CALLBACK __LogWnd::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static	LRESULT CALLBACK __LogWnd::EditSubWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

HWND		__LogWnd::m_hwnd= NULL;
HWND		__LogWnd::m_hwndList= NULL;
HWND		__LogWnd::m_hwndEdit= NULL;
WNDPROC		__LogWnd::m_OldEditProc= NULL;

__LogWnd* g_pLog = NULL;


//-------------------------------------------------------------------------------------------------------------
__LogWnd::__LogWnd( bool bAttachDate /* = false */, DWORD nTarget /* = LOG_TARGET_WINDOW */, const TCHAR* szFilename /* = NULL */ )
{
	m_bAttachDate = bAttachDate;
	m_nTarget = nTarget;
	ZeroMemory(m_szFilename, sizeof(TCHAR) * 1024);

	if( nTarget & LOG_TARGET_FILE )
	{
		if ( szFilename == NULL )
		{
				struct tm now;
				time_t systemTime;

				time(&systemTime); // Get current time in seconds
				localtime_s(&now, &systemTime); // Split time in seconds and populate struct



			// Find the currently running module.
			TCHAR szCurFileName[MAX_PATH];
			::GetModuleFileName(NULL, szCurFileName, MAX_PATH);

			TCHAR szTail[256] = _T("");
			ZeroMemory(szTail, sizeof(TCHAR) * 256);

			_sntprintf_s(szTail, 256, _T("_%04d-%02d-%02d_%02d-%02d-%02d_Log.txt"),
				1900+now.tm_year, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

			_tcscpy_s( m_szFilename, szCurFileName );
			_tcscat_s( m_szFilename, szTail);
		}
		else
		{
			_tcscpy_s( m_szFilename, szFilename );
		}
	}
	else
	{
		m_szFilename[0] = NULL;
	}

	if( nTarget & LOG_TARGET_WINDOW )
	{
		CreateLogWindow();
	}
	else
	{
		m_hwnd = NULL;
	}

	Log( LogLevel::Log, _T("Logging Start..") );
}

__LogWnd::~__LogWnd()
{
	DestroyWindow( m_hwnd );
}

void __LogWnd::CreateLogWindow()
{
	int			x, y, cx, cy;
	WNDCLASS	wc;
	RECT		rc;

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.lpfnWndProc		= (WNDPROC)__LogWnd::WndProc;
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= GetModuleHandle( NULL );
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= (HBRUSH)GetStockObject( WHITE_BRUSH) ;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("LogWindow");

	RegisterClass(&wc);


	cx = LOG_WINDOW_CX;
	cy = LOG_WINDOW_CY;
	x = GetSystemMetrics( SM_CXSCREEN ) - cx;
	y = 0;

	// Find the currently running module.
	TCHAR szCurFileName[MAX_PATH];
	::GetModuleFileName(NULL, szCurFileName, MAX_PATH);


	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	_tsplitpath_s( szCurFileName, drive, dir, fname, ext ); 


	m_hwnd = CreateWindow( _T("LogWindow"), fname, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, x, y, cx, cy, NULL, NULL, GetModuleHandle( NULL ), NULL );
	DWORD nError = GetLastError();
	GetClientRect( m_hwnd, &rc );
	m_hwndList = CreateWindow( _T("LISTBOX"), _T(""), WS_CHILD | WS_VISIBLE|WS_BORDER|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY, 0, 0, rc.right, rc.bottom-30, m_hwnd, NULL, GetModuleHandle( NULL ), NULL );

	m_hwndEdit = CreateWindow( _T("EDIT"), _T(""), WS_CHILD|WS_VISIBLE|WS_BORDER|ES_WANTRETURN , 0, rc.bottom-30, rc.right, 30, m_hwnd, (HMENU)CMD_EDIT, GetModuleHandle( NULL ), NULL );

	
	ShowWindow( m_hwnd, SW_SHOW );
	ShowWindow( m_hwndList, SW_SHOW );
	ShowWindow( m_hwndEdit, SW_SHOW );

	m_OldEditProc = (WNDPROC)SetWindowLongPtr(m_hwndEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(__LogWnd::EditSubWndProc));
	OnResize();
	RePaint();
}

void __LogWnd::OnResize()
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	MoveWindow(m_hwndList,  0, 0, rc.right, rc.bottom-30, FALSE);
	MoveWindow(m_hwndEdit,  0, rc.bottom-30, rc.right, 30, FALSE);
	ShowWindow( m_hwnd, SW_SHOW );
	ShowWindow( m_hwndList, SW_SHOW );
	ShowWindow( m_hwndEdit, SW_SHOW );

}

void __LogWnd::RePaint()
{
	return;
	RedrawWindow(m_hwnd, 0, 0, 0);
	RedrawWindow(m_hwndList, 0, 0, 0);
	RedrawWindow(m_hwndEdit, 0, 0, 0);
	
	InvalidateRect(m_hwnd, 0 , TRUE);
	InvalidateRect(m_hwndList, 0 , TRUE);
	InvalidateRect(m_hwndEdit, 0 , TRUE);
	ShowWindow( m_hwnd, SW_SHOW );
	ShowWindow( m_hwndList, SW_SHOW );
	ShowWindow( m_hwndEdit, SW_SHOW );
}

int __LogWnd::Log( int nLogLevel, const TCHAR* szLog, const char* pszFile, int nLine, const char* pszFunc )
{
	const wchar_t* pSev = L"INFO";
	if( nLogLevel == LogLevel::Error )
		pSev = L"ERROR";
	else if( nLogLevel == LogLevel::Warning )
		pSev = L"WARN";
	else if( nLogLevel >= 7 && nLogLevel != 99 )
		pSev = L"DEBUG";

	SLogSourceLoc loc = { pszFile, nLine, pszFunc };
	TCHAR szTotal[2048] = _T("");
	ZeroMemory(szTotal, sizeof(TCHAR) * 2048);
#ifdef _UNICODE
	DNFormatLogLineW( szTotal, _countof(szTotal), pSev, loc, szLog );
#else
	const char* pSevA = (nLogLevel == LogLevel::Error) ? "ERROR" : ((nLogLevel == LogLevel::Warning) ? "WARN" : ((nLogLevel >= 7 && nLogLevel != 99) ? "DEBUG" : "INFO"));
	DNFormatLogLineA( szTotal, _countof(szTotal), pSevA, loc, szLog );
#endif

	// Strip trailing newline for listbox/file consistency
	size_t nTotalLen = _tcslen( szTotal );
	while( nTotalLen > 0 && ( szTotal[nTotalLen - 1] == _T('\n') || szTotal[nTotalLen - 1] == _T('\r') ) )
		szTotal[--nTotalLen] = 0;
	
	// When outputting to Log File
	if( m_nTarget & LOG_TARGET_FILE )
	{
#ifdef _UNICODE
		char szTemp[2048] = { 0, };
		WideCharToMultiByte( CP_ACP, 0, szTotal, -1, szTemp , 2048, NULL, NULL );
#endif 

		FILE*	fp = NULL;

		errno_t error = _tfopen_s( &fp, m_szFilename, _T("a+") );
		if( fp )
		{
#ifdef _UNICODE
			fprintf_s( fp, "%s\n", szTemp);
#else
			ftprintf_s( fp, "%s\n", szTotal);
#endif
			fclose( fp );
		}
	}

	RECT rc;
	GetClientRect(m_hwndList, &rc);
	int nSize = rc.right - rc.left;

	int nEnterLineLen = nSize / 8;
	if ( nEnterLineLen < 1 )
		nEnterLineLen = 0;

	// When outputting to Log Window 
	if( m_nTarget & LOG_TARGET_WINDOW )
	{
		std::wstring szLogMsg;
		szLogMsg = szTotal;

		while ( szLogMsg.size() > 0 )
		{
			
			std::wstring szTemp;
			szTemp = szLogMsg.substr(0, min(nEnterLineLen, (int)szLogMsg.size()));
			int nMin =  min(nEnterLineLen, (int)szLogMsg.size());
			szLogMsg.erase(0, nMin);

			if ( nMin < 1 )
				break;

			SendMessage( m_hwndList, LB_ADDSTRING, 0, (LPARAM) szTemp.c_str() );
			LRESULT n = SendMessage( m_hwndList, LB_GETCOUNT, 0, 0L ) - 1;
			SendMessage( m_hwndList, LB_SETCURSEL, (WPARAM)n, 0L );

		}

	}

	return 1;
}



LRESULT CALLBACK 
__LogWnd::EditSubWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_ACTIVATE:
		RePaint();
		break;

	case WM_PAINT:
		{
			RePaint();
		}
		break;
	case WM_CHAR:
		{
			//if ( wParam == VK_RETURN )				return 1;
		}
		break;
	case WM_KEYDOWN:
		{
			if ( wParam == VK_RETURN )
			{
				static TCHAR s_CmdBuff[2048] = _T("");
				GetWindowText(m_hwndEdit, s_CmdBuff, 2048-1);
				if ( g_pCallBack )
				{
					g_pCallBack(s_CmdBuff);
				}

				SetWindowText(m_hwndEdit, _T("") );
				RePaint();
				//return 0;
			}
		}
		break;

	}

	return CallWindowProc(m_OldEditProc,hwnd,uMsg,wParam,lParam);

}

LRESULT CALLBACK 
__LogWnd::WndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_ACTIVATE:
		RePaint();
		SendMessage(hwnd, WM_PAINT,0,0);
		break;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		{
			RePaint();
		};
		break;
	case WM_KEYDOWN:
		{
			RePaint();

			if ( wParam == VK_RETURN )
			{
				return 0;
			}
		}
		break;
	case WM_DESTROY:
		{
			SetWindowLongPtr(m_hwndEdit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_OldEditProc));
		}
		break;
	case WM_SIZING:
	case WM_SIZE:
		{
			if ( g_pLog )
				g_pLog->OnResize();

			RePaint();
		}
		break;
	case WM_PAINT:
		{
			RePaint();
		}
		break;

	}

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

//-------------------------------------------------------------------------------------------------------------
bool	CreateLog(bool bAttachDate /* = false */, DWORD nLogTarget /* = LOG_TARGET_FILE */ , const TCHAR* szFileName /* = NULL */)
{
	if ( g_pLog )
		return false;

	g_pLog = new __LogWnd(bAttachDate, nLogTarget, szFileName);

	return true;
}

void	SetCommandCallBack(PCMD_PROC pCallBack)
{
	g_pCallBack = pCallBack;

}

void	LogEx( const char* pszFile, int nLine, const char* pszFunc, int nLogLevel, const TCHAR* fmt, ... )
{
	if ( nLogLevel < 0 )
		return;

	TCHAR gs_Buffer[1024] = _T("");
	ZeroMemory(gs_Buffer, sizeof(TCHAR) * 1024);
	va_list args;
	va_start( args, fmt );
	_vsntprintf_s( gs_Buffer, _countof(gs_Buffer), _TRUNCATE, fmt, args );
	va_end( args );
	gs_Buffer[1024-1] = 0;

	if( nLogLevel <= 6 )
	{
		const wchar_t* pCat = (nLogLevel == 0) ? L"ERROR" : ((nLogLevel == 1) ? L"WARN" : L"INFO");
		DnDebugConsole::LogWEx( pszFile, nLine, pszFunc, pCat, L"%s", gs_Buffer );
	}

	if ( g_pLog )
	{
		g_pLog->Log( nLogLevel, gs_Buffer, pszFile, nLine, pszFunc );
	}
}

void	Log( int nLogLevel, const TCHAR* fmt, ... )
{
	if ( nLogLevel < 0 )
		return;

	TCHAR gs_Buffer[1024] = _T("");
	ZeroMemory(gs_Buffer, sizeof(TCHAR) * 1024);
	va_list args;
	va_start( args, fmt );
	_vsntprintf_s( gs_Buffer, _countof(gs_Buffer), _TRUNCATE, fmt, args );
	va_end( args );
	gs_Buffer[1024-1] = 0;

	if( nLogLevel <= 6 )
	{
		const wchar_t* pCat = (nLogLevel == 0) ? L"ERROR" : ((nLogLevel == 1) ? L"WARN" : L"INFO");
		DnDebugConsole::LogWEx( "LogWnd.cpp", __LINE__, "LogWnd::Log", pCat, L"%s", gs_Buffer );
	}

	if ( g_pLog )
	{
		g_pLog->Log( nLogLevel, gs_Buffer, "LogWnd.cpp", __LINE__, "LogWnd::Log" );
	}
}


void  CmdLog(  const char* fmt, ... )
{
	char gs_Buffer[1024] = { 0 , };
	ZeroMemory(gs_Buffer, sizeof(char) * 1024);
	va_list args;
	va_start( args, fmt );
	_vsnprintf_s(gs_Buffer, _countof(gs_Buffer), _TRUNCATE, fmt, args);
	va_end( args );
	gs_Buffer[1024-1] = 0;

	DnDebugConsole::LogAEx( "LogWnd.cpp", __LINE__, "LogWnd::CmdLog", "CMD", "%s", gs_Buffer );

	if ( LogWnd::g_pLog )
	{
#ifdef _UNICODE
		WCHAR szTemp[1024] = { 0, };
		MultiByteToWideChar( CP_ACP, 0, gs_Buffer, -1, szTemp, 1024 );
		LogWnd::g_pLog->Log(99, szTemp, "LogWnd.cpp", __LINE__, "LogWnd::CmdLog");
#else
		LogWnd::g_pLog->Log(99, gs_Buffer, "LogWnd.cpp", __LINE__, "LogWnd::CmdLog");
#endif
	}
}


bool	DestroyLog()
{
	if ( g_pLog )
	{
		delete g_pLog;
		g_pLog = NULL;
		return true;
	}
	return false;
}

}; // namespace LogWnd;

