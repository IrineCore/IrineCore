#pragma once

#include "Timer.h"
#include "Singleton.h"

class CInputDevice;
class CTaskManager;
class CDnTableDB;
class CTask;
class CEtSoundEngine;
class CClientSessionManager;
class CDnInterface;
class CEtUIXML;
class CDnInCodeResource;
class CDnLoadingTask;
class CDnBridgeTask;
class CDnHangCrashTask;
class CEtBCLEngine;
class CDnRecordDlg;
class CSyncTimer;
class CDnUISound;
class CDnAuthTask;
class CEtBVLEngine;
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
class CNameAutoComplete;
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE

class CDnMainFrame : public CSingleton<CDnMainFrame> {
public:
	CDnMainFrame( HINSTANCE hInst );
	~CDnMainFrame();

	// Things to do before creating Window, such as hacking tool checks, etc.
	bool PreInitialize();

	// Window creation & in-game initializations
	bool InitializeWindow();

	// Create Engine
	bool PreInitializeDevice();
	bool InitializeDevice();
	bool ReInitializeDevice();
	bool FinalizeDevice();

	// Create game
	bool ThreadInitialize();
	bool Initialize();
	bool Finalize();

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	bool ThreadPreloadPlayer();
#endif
	// Kernel main loop
	bool Execute();
	LRESULT WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	HWND GetHWnd() { return m_hWnd; }
	HINSTANCE GetHInst() { return m_hInstance; }

	static LPTOP_LEVEL_EXCEPTION_FILTER s_PrevExceptionFilter;
	static LONG	__stdcall UnhandledExceptionFilter( _EXCEPTION_POINTERS* pExceptionInfo );

	void DestroyMainFrame() { m_bDestroy = true; }
	void SaveScreenShotToFile( D3DXIMAGE_FILEFORMAT Format = D3DXIFF_JPG );

	bool IsBeginCaption() { return m_bBeginCaption; }
	CDnRecordDlg *GetRecordDlg() { return m_pRecordDlg; }

	static void	__cdecl BadAllocFilter();
	static void __stdcall DrawCaptureStateCallback();

	DWORD GetMainThreadID() { return m_dwMainThreadID; }
	void FlushWndMessage();

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	bool IsFinishPreloadPlayer() {
		ScopeLock<CSyncLock> Lock(m_PreLoadLock);
		return m_bFinishPlayerPreload; 
	}
	void SetFinishPreload( bool bFinish ) { 
		ScopeLock<CSyncLock> Lock(m_PreLoadLock);
		m_bFinishPlayerPreload = bFinish; 
	}
	void ThreadInitializePreloadPlayer();
	void FlushPreloadPlayer();
#endif

	void SaveWindowPos();
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	CNameAutoComplete* GetNameAutoComplete() { return m_pNameAutoComplete; }
#endif // PRE_ADD_ITEMNAME_AUTOCOMPLETE

#ifdef PRE_ADD_RELOAD_ACTFILE
	void AddResourcePathByFolderName( CFileNameString& szPath );
	void ReLoadActionFilePath();
#endif 

protected:
	bool CheckRecordDiskSpace( bool bBegin );

	void ToggleRecord();
	void RecordBegin();
	void RecordEnd();

	bool IMEProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
#ifndef _FINAL_BUILD
	void TemporaryKeyProc();
#endif // _FINAL_BUILD

protected:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	CTaskManager *m_pTaskMng;
	CInputDevice *m_pInputDevice;
	CEtSoundEngine *m_pSoundEngine;
	CDnUISound *m_pUISound;
	CDnMouseCursor *m_pCursor;
	CDnInterface *m_pInterface;
	CDnInCodeResource *m_pInCodeRes;
	CEtUIXML *m_pUIXML;
	CEtResourceMng *m_pResMng;
	CSyncTimer *m_pSyncTimer;

	CEtBCLEngine *m_pCaptureEngine;
	CDnRecordDlg *m_pRecordDlg;
	bool m_bCheckRecordDiskSpace[4];
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	CNameAutoComplete *m_pNameAutoComplete;
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
	CEtExceptionalUIXML* m_pExceptionalUIXML;
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING

	/*std::vector<DWORD> m_dwVecTaskHandle;*/	// Removing because it appears unused.
	CDnTableDB *m_pTableDB;
	CClientSessionManager *m_pSessionMng;

	CDnLoadingTask *m_pLoadingTask;
	CDnBridgeTask *m_pBridgeTask;
	CDnHangCrashTask *m_pHangCrashTask;
	CDnAuthTask *m_pAuthTask; 

	bool m_bDestroy;

	POINT m_MousePos;
	POINT m_WindowPos;
	bool m_bBeginCaption;
	bool m_bCreateWindow;
	bool m_bMaximize;
	DWORD m_dwMainThreadID;

	int	m_nWinVer;

	struct DnOutOfMemoryFunc : public FnDeviceOutOfMemory
	{
		virtual void Run();
	};

	DnOutOfMemoryFunc m_DnOutOfMemoryFunc;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	static bool __stdcall FlushWaitDelete();
#else
	HANDLE m_hPreloadPlayerThreadHandle;
	unsigned m_dwPreloadPlayerThreadID;
	bool m_bFinishPlayerPreload;
	DnActorHandle m_hPreLoadPlayerClass[10];
	CSyncLock m_PreLoadLock;
#endif
};
 