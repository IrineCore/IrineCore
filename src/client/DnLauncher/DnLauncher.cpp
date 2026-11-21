// DnLauncher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DnLauncher.h"
#include "DnLauncherDlg.h"
#include "DnServiceModule.h"
#include "DnFIrstPatchDlg.h"
#include <locale.h>
#include <process.h>
#include "DnControlData.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment( lib, "shlwapi.lib" )

// CDnLauncherApp
BEGIN_MESSAGE_MAP(CDnLauncherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

int __cdecl DefautAllocHook( int, void *, size_t, int, long, const unsigned char *, int )
{
	return 1;
}

int __cdecl MyAllocHook( int nAllocType, void * pvData, size_t nSize, int nBlockUse,
						long lRequest, const unsigned char * szFileName, int nLine )
{
	if( nAllocType == _HOOK_ALLOC && nSize == 106 )
	{
		int a = 0;
	}
	return 1;
}


// The one and only CDnLauncherApp object
CDnLauncherApp	theApp;
CString			g_szCmdLine;
CString			g_szOriginalCmdLine;		// Preserve initial value for launcher patch
CString			g_szCommandLinePatchURL;	// When passing the URL where PatchConfigList.xml is located via command line (Korea)
int				g_nInitErrorCode = 0;		// Error code on launcher initialization

extern CDnFistPatchDownloadThread*  g_pFirstPatchDownloadThread;
CDnFIrstPatchDlg					g_FirstPatchDlg;
HANDLE								g_FirstPatchDlgHandle;
extern FirstPatchReturn				g_nFirstPatchErrorMessage;
unsigned int WINAPI ShowFirstPatchDlg(void* arg)
{
	INT_PTR nResult = g_FirstPatchDlg.DoModal();
	return 0;
}

#ifndef _DEBUG
static const char g_szMutexName[] = "Global\\___DN___";
HANDLE g_hMutex = NULL;
#endif // _DEBUG


// CDnLauncherApp construction
CDnLauncherApp::CDnLauncherApp()
{

#ifdef _USE_LOG
	DeleteOldLogFile();
	UseLogFile();
#endif // _USE_LOG

#ifdef _DEBUG
	//When debugging in VS 2005, using TRACE with a long string causes an error, so locale must be set. 
	_tsetlocale( LC_ALL, _T("korean") );

//	_CrtSetAllocHook( MyAllocHook );
//	_CrtSetBreakAlloc( 2599 );
#endif

}

CDnLauncherApp::~CDnLauncherApp()
{
#ifndef _DEBUG
	if( g_hMutex )
	{
		CloseHandle( g_hMutex );
		g_hMutex = NULL;
	}
#endif // _DEBUG

	if( g_pServiceModule )
		g_pServiceModule->Destroy();

	SAFE_DELETE( g_pServiceModule );

	LogWnd::DestroyLog();
}

// CDnLauncherApp initialization
BOOL CDnLauncherApp::InitInstance()
{
	g_szOriginalCmdLine = g_szCmdLine = m_lpCmdLine;
	ParseCommandLine();	// Parse command line

/*	while(1)
	{

	}*/




	//Check launcher
#ifndef _DEBUG
	g_hMutex = ::OpenMutexA( MUTEX_ALL_ACCESS, FALSE, g_szMutexName );
	if( g_hMutex == NULL )
	{
		g_hMutex = ::CreateMutexA( NULL, FALSE, g_szMutexName );
	}
	else
	{
		ErrorMessageBoxLog( _S( STR_ALEADY_RUN + DNPATCHINFO.GetLanguageOffset() ) );
		return FALSE;
	}
#endif // _DEBUG

	CHECK_ALIGNMENT();

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	SetRegistryKey(_T("DragonNest Launcher"));

	ULONG_PTR gdiplusToken = 0;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

//	ErrorMessageBoxLog( g_szCmdLine );
//	LogWnd::TraceLog( g_szCmdLine );

	if( InitLauncher() )
	{
		CDnLauncherDlg* pDlg = CDnLauncherDlg::CreateLauncherDlg();
		m_pMainWnd			 = pDlg;
		INT_PTR nResponse	 = pDlg->DoModal();

		SAFE_DELETE( pDlg );
	}

	if( gdiplusToken != 0 )
		Gdiplus::GdiplusShutdown( gdiplusToken );

	return FALSE;
}

BOOL CDnLauncherApp::InitLauncher()
{
	LogWnd::TraceLog( L"InitLauncher Start!" );

#ifdef _USE_SINGLE_CLIENT
	if( FindProcessName( _T(DNCLIENT_NAME) ) )	// Not run while the game is executing
	{
		ErrorMessageBoxLog( _S( STR_CLIENT_ALEADY_RUN + DNPATCHINFO.GetLanguageOffset() ) );
		g_nInitErrorCode = INIT_ERROR_CLIENT_ALEADY_RUN;
		return FALSE;
	}
#endif // _USE_SINGLE_CLIENT
		
#ifdef _FIRST_PATCH
	if( !InitFirstPatchThread() )			// Create the FirstPatch thread.
	{	
		return InitFail();
	}
#endif

	if( !CheckDirectXVersion() )			// Check DirectX version
	{
		return InitFail();
	}

	if( DNPATCHINFO.Init() != S_OK )		// Initialize PatchInfo (download and parse PatchConfigList.xml, set client path, delete xml)
	{
		return InitFail();
	}

#ifndef _USE_PARTITION_SELECT
	if( !DNPATCHINFO.SetPatchInfo() )		// Set PatchInfo (set PatchConfigList.xml contents into member variables, configure parameters, and check patch version here)	
	{
		return InitFail();
	}
#endif // _USE_PARTITION_SELECT


#if defined(_FIRST_PATCH)
#ifdef _USE_PARTITION_SELECT
	DNPATCHINFO.LoadVersionData();			// Read partition info && version.
#endif
	if( DNFIRSTPATCHINFO.GetFirstPatchStatus() == DNFIRSTPATCHINFO.EM_FIRSTPATCH_OK )
	{
		if( !DNFIRSTPATCHINFO.SetPatchInfo() ) // First Patch "Initialize" & "Start Thread"
		{
			return InitFail();
		}
	}
	TerminateFirstPatchThread();			// * Clean up threads used in First Patch * 
#endif // _FIRST_PATCH

#if defined( _USE_COMMAND_LINE )			// When using command line with no parameters, redirect to homepage
	if( g_szCmdLine.GetLength() == 0 )
	{
		ShellExecute( NULL, L"open", DNPATCHINFO.GetHomepageUrl(), NULL, NULL, SW_SHOW );
		return FALSE;
	}
#endif // _USE_COMMAND_LINE

	g_pServiceModule = DnServiceModule::CreateServiceModule();
	if( g_pServiceModule && !g_pServiceModule->Initialize() )	// Initialize per service provider
	{
		g_nInitErrorCode = INIT_ERROR_SERVICE_MODULE;
		return InitFail();
	}
	
	if( !RemoveUsedFile() )	 // Delete files used at launcher startup and tmp files created during launcher update
	{
		return InitFail();
	}

#ifdef _FIRST_PATCH
#if !defined(_TEST)
	if(g_nFirstPatchErrorMessage == FPR_NEED_FULLPATCH)
	{
		// Client reinstallation required. Just exit.
		LogWnd::TraceLog( L"FirstPatch Failed / need Client Full Patch!" );
		LogWnd::Log( LogLevel::Error, _T("FirstPatch Failed / Need Full Patch") );
		return InitFail();
	}
#endif // _TEST
#endif // _FIRST_PATCH

	LogWnd::TraceLog( L"InitLauncher Success!" );
	return TRUE;
}

#ifdef _FIRST_PATCH
BOOL CDnLauncherApp::InitFirstPatchThread()
{
	// * Delete Tmp file if it exists.
	if(_access(DNLAUNCHER_NAME_TMP, 0) == 0)
	{
		if( !ClientDeleteFile(_T(DNLAUNCHER_NAME_TMP)) )
		{
			LogWnd::Log( LogLevel::Warning, _T("dnlauncher.tmp File Delete Failed") );
		}
	}

	// * Start Dialog Thread showing FirstPatch progress *
	g_FirstPatchDlgHandle = (HANDLE)_beginthreadex(NULL, 0, ShowFirstPatchDlg, NULL, 0, NULL);
	if(g_FirstPatchDlgHandle == NULL)
	{
		LogWnd::Log( LogLevel::Error, _T("FirstPatch Dialog Thread Create Failed") );
		g_nInitErrorCode = INIT_ERROR_FIRSTPATCH_THREAD_CREATE_FAILED;
		return FALSE;
	}

	// Module patch started.
	DNFIRSTPATCHINFO.SetFirstPatchProcess(DNFIRSTPATCHINFO.EM_FIRSTPATCH_ING);
	return TRUE;
}

void CDnLauncherApp::TerminateFirstPatchThread()
{
	if( g_pFirstPatchDownloadThread ) // Download thread.
	{
		// The thread downloading FirstPatch has terminated.
		WaitForSingleObject( g_pFirstPatchDownloadThread->GetThreadHandle(), INFINITE );
	}

	// Close dialog.
	if( g_FirstPatchDlg )
	{
		// * Terminate FirstPatch-related dialog & thread * //
		DWORD dwExitCode;
		if( GetExitCodeThread( g_FirstPatchDlgHandle, &dwExitCode ) )
		{
			TerminateThread( g_FirstPatchDlgHandle, dwExitCode );
		}

		g_FirstPatchDlg.EndDialog( IDYES );
	}

	// Module patch finished.
	DNFIRSTPATCHINFO.SetFirstPatchProcess(DNFIRSTPATCHINFO.EM_FIRSTPATCH_NOT);
}
#endif // _FIRST_PATCH


BOOL CDnLauncherApp::InitFail()
{
	CString strError;
	strError.Format( _S( STR_LAUNCHER_INIT_FAILED + DNPATCHINFO.GetLanguageOffset() ), g_nInitErrorCode );
	LogWnd::Log( LogLevel::Error, strError );
	AfxMessageBox( strError );
	
#ifdef _FIRST_PATCH
	HANDLE	pHandles[2];
	pHandles[0] = g_FirstPatchDlgHandle;
	if(g_pFirstPatchDownloadThread)
	{
		pHandles[1] = g_pFirstPatchDownloadThread->GetThreadHandle();
	}
	
	DWORD dwExitCode;
	GetExitCodeThread(g_FirstPatchDlgHandle, &dwExitCode);
	TerminateThread(g_FirstPatchDlgHandle, dwExitCode);
	
	if( WAIT_FAILED != WaitForMultipleObjects(2, pHandles, TRUE, INFINITE) )
	{
		// Terminate once all threads have finished.
		return FALSE;
	}
#endif
	
	return FALSE;
}

BOOL CDnLauncherApp::CheckDirectXVersion()
{
	DWORD dwDirectXVersion = 0;
	TCHAR strDirectXVersion[10] = {0,};

	std::wstring wstrVersion;

	int nResult = GetDXVersion( &dwDirectXVersion, strDirectXVersion, 10);

	CString szStr;
	LogWnd::TraceLog( _T("DirectX Version (%u) (%s)"), dwDirectXVersion, strDirectXVersion );

	if( dwDirectXVersion < 0x00090000 )
	{
		wstrVersion = strDirectXVersion;
		if( wstrVersion != L"9.0c" )
		{
			CString szStr;
			szStr.Format ( _S( STR_DX_LOW_VER + DNPATCHINFO.GetLanguageOffset() ), wstrVersion.c_str() );
			ErrorMessageBoxLog( szStr );
			g_nInitErrorCode = INIT_ERROR_DX_VER;
			return FALSE;
		}
	}

	return TRUE;
}

void CDnLauncherApp::ParseCommandLine()
{
	if( g_szCmdLine.GetLength() == 0 )
		return;

	LPCTSTR szlpURL = L"http://";

	std::wstring wszCmdLine;	 
	wszCmdLine = g_szCmdLine;
	std::vector<std::wstring> tokens;
	TokenizeW( wszCmdLine, tokens, std::wstring(L" ") );

	for( DWORD i=0; i<tokens.size(); i++ )
	{
		if( NULL != StrStr( tokens[i].c_str(), szlpURL ) )			// Contains the URL of PatchConfigList.xml
		{
			g_szCommandLinePatchURL = tokens[i].c_str();			// Save address
			g_szCmdLine.Replace( g_szCommandLinePatchURL, L"" );	// Remove address from command line
			break;
		}
	}

	tokens.clear();

	if( g_szCommandLinePatchURL.GetLength() > 0 && g_szCommandLinePatchURL.GetBuffer()[g_szCommandLinePatchURL.GetLength()-1] != '/' )
		g_szCommandLinePatchURL += '/';
}

void CDnLauncherApp::DeleteOldLogFile()
{
	TCHAR szCurFileName[MAX_PATH]= {0,};
	::GetModuleFileName( NULL, szCurFileName, MAX_PATH );
	TCHAR szDrive[MAX_PATH] = { 0, };
	TCHAR szDir[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	TCHAR szExt[MAX_PATH] = { 0, };
	_tsplitpath( szCurFileName, szDrive, szDir, szFileName, szExt );

	CString _szCurPath;
	_szCurPath.Format( _T("%s%s\\Log\\"), szDrive, szDir );

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	CString szFindLogFile;
	szFindLogFile = _szCurPath;
	szFindLogFile += "DNLauncher_*_Log.txt";
	hFind = FindFirstFile( szFindLogFile.GetBuffer(), &FindFileData );

	std::vector<CString> vecStrLogFileName;
	while( hFind != INVALID_HANDLE_VALUE ) 
	{
		if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			CString szLogFile;
			szLogFile = _szCurPath;
			szLogFile += FindFileData.cFileName;

			vecStrLogFileName.push_back( szLogFile );
		}
		if( !FindNextFile( hFind, &FindFileData ) ) break;
	}
	FindClose( hFind );

	std::vector<CString> vecStrDeleteLogFileName;
	for( int i=0; i<static_cast<int>( vecStrLogFileName.size() ); i++ )
	{
		CString strTemp = vecStrLogFileName[i];
		strTemp.Delete( 0, strTemp.Find( L"_" ) + 1 );
		int nIndex = strTemp.Find( L"_" );
		strTemp.Delete( nIndex, strTemp.GetLength() - nIndex );
		int nLogDate = _wtoi( strTemp.GetBuffer() );

		struct tm now;
		time_t systemTime;

		time( &systemTime );				// Get current time in seconds
		localtime_s( &now, &systemTime );	// Split time in seconds and populate struct

		TCHAR szCurDate[1024];
		_sntprintf_s( szCurDate, 1024, _T("%04d%02d%02d"), 1900+now.tm_year, now.tm_mon, now.tm_mday );	// Delete items older than one month
		int nCurDate = _wtoi( szCurDate );

		if( nLogDate < nCurDate )
		{
			vecStrDeleteLogFileName.push_back( vecStrLogFileName[i] );
		}
	}

	for( int i=0; i<static_cast<int>( vecStrDeleteLogFileName.size() ); i++ )
	{
		ClientDeleteFile( vecStrDeleteLogFileName[i] );
	}

	vecStrLogFileName.clear();
	vecStrDeleteLogFileName.clear();
}

void CDnLauncherApp::UseLogFile()
{
	TCHAR szCurFileName[MAX_PATH]= {0,};
	::GetModuleFileName( NULL, szCurFileName, MAX_PATH );
	TCHAR szDrive[MAX_PATH] = { 0, };
	TCHAR szDir[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	TCHAR szExt[MAX_PATH] = { 0, };
	_tsplitpath( szCurFileName, szDrive, szDir, szFileName, szExt );

	CString _szCurPath;
	_szCurPath.Format( _T("%s%s\\Log"), szDrive, szDir );

	if( GetFileAttributes( _szCurPath.GetBuffer() ) != FILE_ATTRIBUTE_DIRECTORY )
		CreateDirectory( _szCurPath.GetBuffer(), NULL);

	LogWnd::CreateLog( true, LogWnd::LOG_TARGET_FILE );
}

void CDnLauncherApp::UseLogWindow()
{
	TCHAR szCurFileName[MAX_PATH]= {0,};
	::GetModuleFileName( NULL, szCurFileName, MAX_PATH );
	TCHAR szDrive[MAX_PATH] = { 0, };
	TCHAR szDir[MAX_PATH] = { 0, };
	TCHAR szFileName[MAX_PATH] = { 0, };
	TCHAR szExt[MAX_PATH] = { 0, };
	_tsplitpath( szCurFileName, szDrive, szDir, szFileName, szExt );

	CString _szCurPath;
	_szCurPath.Format( _T("%s%s\\Log"), szDrive, szDir );

	if( GetFileAttributes( _szCurPath.GetBuffer() ) != FILE_ATTRIBUTE_DIRECTORY )
		CreateDirectory( _szCurPath.GetBuffer(), NULL);

	LogWnd::CreateLog( true, LogWnd::LOG_TARGET_WINDOW );
}

BOOL CDnLauncherApp::RemoveUsedFile()
{
	BOOL bRtn = TRUE;
	LogWnd::TraceLog( L"RemoveUsedFile" );
	if( ClientDeleteFile( _T(DNLAUNCHER_NAME_TMP) ) == FALSE )
	{		
		KillProcess( _T(DNLAUNCHER_NAME) );	// Force-terminate all dnlauncher.exe instances in the current process list.
		if( ClientDeleteFile( _T(DNLAUNCHER_NAME_TMP) ) == FALSE )
			bRtn = FALSE;
	}

	if( ClientDeleteFile( PATCHCONFIG_LIST ) == FALSE ) 
		bRtn = FALSE;

	if( !bRtn )
		g_nInitErrorCode = INIT_ERROR_REMOVE_FILE;

	return bRtn;
}
