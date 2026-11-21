// DnLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DnLauncher.h"
#include "DnLauncherDlg.h"
#include "DnHtmlView.h"
#include "DnPatchThread.h"
#include "DnOptionDlg.h"
#include "DnExplorerDlg.h"


#include "DnLauncherDlgUSA.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CDnPatchDownloadThread* g_pPatchDownloadThread;
extern stDownloadInfoUpdateData	g_DownloadInfoData;
extern CString g_szCmdLine;
extern int g_nInitErrorCode;


struct DownloadStateString
{
	int nStateIndex;
	int nStringID;
};

DownloadStateString g_DownloadStateString[] = 
{
	{ PATCH_NONE,						-1	},
	{ PATCH_CHECK_VERSION,				(STR_CONFIRM_PATCH_VER)	},
	{ PATCH_CHECK_VERSION_FAILED,		(STR_CONFIRM_PATCH_VER_FAILED)	},
	{ PATCH_DOWNLOAD_DATA,				(STR_PATCH_DATA_DOWNLOADING)	},
	{ PATCH_DOWNLOAD_DATA_FAILED,		(STR_PATCH_DATA_DOWNLOADING_FAILED)	},
	{ PATCH_MD5_CHECK,					(STR_PATCH_MD5_CHECK)	},
	{ PATCH_MD5_CHECK_FAILED,			(STR_PATCH_MD5_CHECK_FAILED)	},
	{ PATCH_APPLY_PAK_FILE,				(STR_PATCH_APPLY_PAK_FILE	)	},
	{ PATCH_APPLY_PAK_FILE_FAILED,		(STR_PATCH_APPLY_PAK_FILE_FAILED)	},
	{ PATCH_APPLY_PAK_FILE_END,			-1	},
	{ PATCH_COMPLETE,					(STR_PATCH_COMPLETE)	},
	{ PATCH_LAUNCHER,					-1	},
	{ PATCH_FAILED,						(STR_PATCH_FAILED)	},
	{ PATCH_TERMINATE,					(STR_PATCH_TERMINATE)	},
};

// CDnLauncherDlg dialog
CDnLauncherDlg::CDnLauncherDlg( CWnd* pParent )
: CDialog(CDnLauncherDlg::IDD, pParent)
, m_dwWidth( 0 )
, m_dwHeight( 0 )
, m_dwFlags ( 0 )
, m_emDownloadPatchState( PATCH_NONE )
, m_nMaxCtrlNum( 0 )
, m_pDnHtmlView( NULL )
, m_pDownloadProgress( NULL )
, m_pFileCountProgress( NULL )
, m_pDownloadProgressBitmap( NULL )
, m_pFileCountProgressBitmap( NULL )
, m_nDownloadProgressMin( 0 )
, m_nDownloadProgressMax( 0 )
, m_nCurrentFileCount( 0 )
, m_nTotalFileCount( 0 )
, m_fDownloadRate( 0.0f )
, m_bDownloading( FALSE )
, m_staticBaseColor( WHITE )
, m_dcBkGrnd( NULL )
, m_hBmpBkGrnd( NULL )
, m_hOldBmpBkGrnd( NULL )
, m_pRectCtrl( NULL )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	EnumDisplayMode( m_vecDisplayMode );
	m_pntMouse.x = 0;
	m_pntMouse.y = 0;
}

CDnLauncherDlg* CDnLauncherDlg::CreateLauncherDlg()
{
	return ( new CDnLauncherDlgUSA() );
	return NULL;
}

void CDnLauncherDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );

	// Button Ctrl
	DDX_Control( pDX, IDC_CHECK_DIRECT_GAMESTART, m_BtnCheckDirectGameStart );
	DDX_Control( pDX, IDC_BTN_CANCEL, m_BtnCancel );
	DDX_Control( pDX, IDC_BTN_GAMESTART, m_BtnStartGame );
	DDX_Control( pDX, IDC_BTN_OPTION, m_BtnGameOption );
	DDX_Control( pDX, IDC_BTN_CLOSE, m_BtnClose );
	DDX_Control( pDX, IDC_BTN_MINIMUM, m_BtnMinimum );
#ifdef _USE_PARTITION_SELECT
	DDX_Control( pDX, IDC_BTN_SELECT_PARTITION, m_BtnSelectPartition );
	DDX_Control( pDX, IDC_BTN_START_PATCH, m_BtnStartPatch );
#endif // _USE_PARTITION_SELECT

	// ProgressBar Ctrl
	DDX_Control( pDX, IDC_STATIC_PROGRESS_DOWNLOAD, m_StaticDownloadProgress );
	DDX_Control( pDX, IDC_STATIC_PROGRESS_COUNT, m_StaticFileCountProgress );

	// Static Text
	DDX_Text( pDX, IDC_LAUNCHER_VERSION, m_strLauncherVersion );
	DDX_Text( pDX, IDC_FILE_NAME, m_strFileName );
	DDX_Text( pDX, IDC_FILE_SIZE, m_strFileSize );
	DDX_Text( pDX, IDC_TIME_LEFT, m_strTimeLeft );
	DDX_Text( pDX, IDC_FILE_COUNT, m_strFileCount );
	DDX_Text( pDX, IDC_DOWNLOAD_PERCENTAGE, m_strDownloadPercentage );
	DDX_Text( pDX, IDC_DOWNLOAD_STATE, m_strDownloadState );
	DDX_Text( pDX, IDC_DIRECT_START, m_strDirectGameStart );
#ifdef _USE_PARTITION_SELECT
	DDX_Text( pDX, IDC_SELECT_PARTITION, m_strSelectPartition );
	DDX_Text( pDX, IDC_SELECT_PARTITION_NAME, m_strSelectPartitionName );
	DDX_Text( pDX, IDC_SELECT_PARTITION_GUIDE, m_strSelectPartitionGuide );
#endif // _USE_PARTITION_SELECT

	// Static Text Ctrl
	DDX_Control( pDX, IDC_LAUNCHER_VERSION, m_StaticLauncherVersion );
	DDX_Control( pDX, IDC_FILE_NAME, m_StaticFileName );
	DDX_Control( pDX, IDC_FILE_SIZE, m_StaticFileSize );
	DDX_Control( pDX, IDC_TIME_LEFT, m_StaticTimeLeft );
	DDX_Control( pDX, IDC_FILE_COUNT, m_StaticFileCount );
	DDX_Control( pDX, IDC_DOWNLOAD_PERCENTAGE, m_StaticDownloadPercentage );
	DDX_Control( pDX, IDC_DOWNLOAD_STATE, m_StaticDownloadState );
	DDX_Control( pDX, IDC_DIRECT_START, m_StaticDirectGameStart );
#ifdef _USE_PARTITION_SELECT
	DDX_Control( pDX, IDC_SELECT_PARTITION, m_StaticSelectPartition );
	DDX_Control( pDX, IDC_SELECT_PARTITION_NAME, m_StaticSelectPartitionName );
	DDX_Control( pDX, IDC_SELECT_PARTITION_GUIDE, m_StaticSelectPartitionGuide );
#endif // _USE_PARTITION_SELECT
}


BEGIN_MESSAGE_MAP( CDnLauncherDlg, CDialog )
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE( WM_STATUS_MSG, OnStatusMsg )
	// Button Ctrl Click Message
	ON_BN_CLICKED( IDC_CHECK_DIRECT_GAMESTART, &CDnLauncherDlg::OnClickedCheckDirectGamestart )
	ON_BN_CLICKED( IDC_BTN_CANCEL, &CDnLauncherDlg::OnClickButtonCancel )
	ON_BN_CLICKED( IDC_BTN_GAMESTART, &CDnLauncherDlg::OnClickButtonGameStart )
	ON_BN_CLICKED( IDC_BTN_OPTION, &CDnLauncherDlg::OnClickButtonOption )
	ON_BN_CLICKED( IDC_BTN_CLOSE, &CDnLauncherDlg::OnClickButtonClose )
	ON_BN_CLICKED( IDC_BTN_MINIMUM, &CDnLauncherDlg::OnClickMinimumBtn )
#ifdef _USE_PARTITION_SELECT
	ON_BN_CLICKED( IDC_BTN_SELECT_PARTITION, &CDnLauncherDlg::OnClickButtonSelectPartition )
	ON_BN_CLICKED( IDC_BTN_START_PATCH, &CDnLauncherDlg::OnClickButtonStartPatch )
#endif // _USE_PARTITION_SELECT
END_MESSAGE_MAP()


void CDnLauncherDlg::OnDestroy()
{
	m_vecDisplayMode.clear();
	m_Font.DeleteObject();

	SAFE_DELETE( m_pDownloadProgress );
	SAFE_DELETE( m_pFileCountProgress );
	SAFE_DELETE( m_pDownloadProgressBitmap );
	SAFE_DELETE( m_pFileCountProgressBitmap );

	// Thread Terminate
	if( g_pPatchDownloadThread )
	{
		g_pPatchDownloadThread->TerminateThread();
		g_pPatchDownloadThread->WaitForTerminate();
	}
	LogWnd::TraceLog( _T( "Thread End") );
	SAFE_DELETE( g_pPatchDownloadThread );

	if( m_dcBkGrnd != NULL )
	{
		if( m_hOldBmpBkGrnd != NULL )
			::SelectObject( m_dcBkGrnd, m_hOldBmpBkGrnd );
		::DeleteDC( m_dcBkGrnd );
		m_dcBkGrnd = NULL;
		m_hOldBmpBkGrnd = NULL;
	}
	if( m_hBmpBkGrnd != NULL )
	{
		::DeleteObject( m_hBmpBkGrnd );
		m_hBmpBkGrnd = NULL;
	}

	LogWnd::DestroyLog();

	CDialog::OnDestroy();
}

BOOL CDnLauncherDlg::InitFail()
{
	CString strError;
	strError.Format( _S( STR_LAUNCHER_INIT_FAILED + DNPATCHINFO.GetLanguageOffset() ), g_nInitErrorCode );
	LogWnd::Log( LogLevel::Error, strError );
	AfxMessageBox( strError );
	CDialog::DestroyWindow();
	return FALSE;
}

// CDnLauncherDlg message handlers
BOOL CDnLauncherDlg::OnInitDialog()
{
	LogWnd::TraceLog( L"InitDialog Start!" );
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon( m_hIcon, TRUE );	// Set big icon
	SetIcon( m_hIcon, FALSE );	// Set small icon

	this->SetWindowText( _S( STR_DRAGONNEST_LAUNCHER + DNPATCHINFO.GetLanguageOffset() ) );
	DNOPTIONDATA.LoadConfigOption( this->m_hWnd );		// Read Option

	if( !InitInterface() )		// Initialize UI
	{
		g_nInitErrorCode = INIT_ERROR_INTERFACE;
		return InitFail();
	}

	if( !InitWebpage() )		// Initialize guide web page
	{
		g_nInitErrorCode = INIT_ERROR_WEBPAGE;
		return InitFail();
	}

#ifdef _USE_PARTITION_SELECT
#ifdef _FIRST_PATCH
	InitSelectPartition();
#else // _FIRST_PATCH
	DNOPTIONDATA.LoadPartitionOption( this->m_hWnd );

	// In partition selection mode, init Patch Info if selection exists, otherwise read client version only
	if( DNOPTIONDATA.m_nSelectChannelNum > 0 && DNOPTIONDATA.m_nSelectPartitionId > 0 )
	{
		if( DNPATCHINFO.SetPatchInfo() )
			InitSelectPartition();
		else	// If setting patch info with saved partition info fails, clear partition info to trigger re-setup
		{
			DNOPTIONDATA.m_nSelectChannelNum = 0;
			DNOPTIONDATA.m_nSelectPartitionId = 0;
			DNOPTIONDATA.SavePartitionOption( this->m_hWnd );
		}
	}
	else
		DNPATCHINFO.LoadClientVersion();
#endif // _FIRST_PATCH
#endif // _USE_PARTITION_SELECT
	
	NavigateHtml();

	m_strLauncherVersion.Format( _T("%s Ver.%d"), _S( STR_DLG_DRAGON_NEST + DNPATCHINFO.GetLanguageOffset() ), DNPATCHINFO.GetClientVersion() );	// Top-left version display string
	SetTimer( TEVENT_REFRESH_TEXTINFO, 40, NULL );		//Update screen text
	LogWnd::TraceLog( L"InitDialog Success!" );

#ifndef _USE_PARTITION_SELECT
	StartPatchDownloadThread();	// Start patch thread immediately if not in partition selection
#endif // _USE_PARTITION_SELECT

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDnLauncherDlg::NavigateHtml()
{
#ifdef _USE_PARTITION_SELECT
	if( DNPATCHINFO.GetGuidepageUrl().GetLength() == 0 )	// If there is no prior selection record, get and show the default guide page (prevents broken image!)
	{
		if( m_pDnHtmlView )
			m_pDnHtmlView->Navigate( DNPATCHINFO.GetBaseGuidePageUrl() );

		return;
	}
#endif // _USE_PARTITION_SELECT
	if( m_pDnHtmlView )
		m_pDnHtmlView->Navigate( DNPATCHINFO.GetGuidepageUrl() );
}

BOOL CDnLauncherDlg::InitWebpage()
{
	LogWnd::TraceLog( L"InitWebpage" );
	// Set guide web page View/Html
	CCreateContext pContext;
	CWnd* pFrameWnd = this;
	pContext.m_pCurrentDoc = NULL;
	pContext.m_pNewViewClass = RUNTIME_CLASS( CDnView );
	CDnView *pView = (CDnView*)((CFrameWnd*)pFrameWnd)->CreateView( &pContext );
	if( pView == NULL )
		return FALSE;
	pView->ShowWindow( SW_SHOW );

	pContext.m_pCurrentDoc = NULL;
	pContext.m_pNewViewClass = RUNTIME_CLASS( CDnHtmlView );
	m_pDnHtmlView = (CDnHtmlView*)((CFrameWnd*)pFrameWnd)->CreateView( &pContext );
	if( m_pDnHtmlView == NULL )
		return FALSE;
	m_pDnHtmlView->ShowWindow( SW_SHOW );
	m_pDnHtmlView->MoveWindow( GetControlPos( em_WebPageHtml ) );

	return TRUE;
}

BOOL CDnLauncherDlg::InitInterface()
{
	LogWnd::TraceLog( L"InitInterface" );
	// -- Background -- //
	if( !InitBackground() )
		return FALSE;

	// -- Control -- //
	InitCtrlRect();
	GetCtrlRect( m_pRectCtrl, m_nMaxCtrlNum );	// Set Ctrl Rect
	InitControl();

	// -- Transparent -- //
	LONG_PTR window_style = GetWindowLongPtr( m_hWnd, GWL_EXSTYLE );

	if( !( window_style & WS_EX_LAYERED ) )
		SetWindowLongPtr( m_hWnd, GWL_EXSTYLE, window_style | WS_EX_LAYERED );

	HMODULE h_user32_dll = GetModuleHandle( L"USER32.DLL" );
	if( h_user32_dll != NULL )
	{
		BOOL (WINAPI *fp_set_layered_window_attributes)(HWND, COLORREF, BYTE, DWORD) = 
			(BOOL (WINAPI *)(HWND, COLORREF, BYTE, DWORD))GetProcAddress(h_user32_dll, 
			"SetLayeredWindowAttributes");

		if( fp_set_layered_window_attributes != NULL )	// Indirectly invoke SetLayeredWindowAttributes using a function pointer.
			(*fp_set_layered_window_attributes)( m_hWnd, RGB( 255, 0, 255 ), 0, LWA_COLORKEY );
	}

	return TRUE;
}

BOOL CDnLauncherDlg::InitBackground()
{
	CLauncherImage xImage;
	if( !xImage.LoadFromResource( IDR_JPG_BACK_GROUND + DNPATCHINFO.GetLanguageOffset(), L"JPG" ) )
		return FALSE;

	HBITMAP hBmp = xImage.DetachBitmap();
	if( hBmp == NULL )
		return FALSE;

	// set window size
	BITMAP bmp = { 0 };
	GetObject( hBmp, sizeof(BITMAP), &bmp );
	m_dwWidth	= bmp.bmWidth;		// bitmap width
	m_dwHeight	= bmp.bmHeight;		// bitmap height
	// alpha color 
	HRGN hRgn = CreateRgnFromFile( hBmp, RGB( 255, 0, 255 ) );
	// build memory dc for background
	CDC* dc = GetDC();
	m_dcBkGrnd = CreateCompatibleDC( dc->m_hDC );
	ReleaseDC( dc );
	
	// select background image (owned and released in OnDestroy)
	m_hBmpBkGrnd = hBmp;
	m_hOldBmpBkGrnd = SelectObject( m_dcBkGrnd, hBmp );
	
	// set window size the same as image size (set as topmost)
	//::SetWindowPos( this->GetSafeHwnd(), HWND_TOPMOST, 0, 0, m_dwWidth, m_dwHeight, SWP_NOZORDER | SWP_NOMOVE );
	SetWindowPos( &wndTopMost, 0, 0, m_dwWidth, m_dwHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE );
	::SetForegroundWindow( this->GetSafeHwnd() );
	::SetActiveWindow( this->GetSafeHwnd() );

	// assign region to window
	SetWindowRgn( hRgn, FALSE );

	return TRUE;
}

void CDnLauncherDlg::InitControl()
{
	// Button Control
	m_BtnCheckDirectGameStart.MoveWindow( m_pRectCtrl[em_CheckBox_DIrectStart] );
	m_BtnCheckDirectGameStart.SetCheck( DNOPTIONDATA.m_bAutoGameStart );

	m_BtnCancel.SetSkinJpg( IDR_JPG_CANCEL + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_CANCEL_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_CANCEL_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnCancel.MoveWindow( m_pRectCtrl[em_BtnQuit] );
	m_BtnCancel.SetWindowText( L"" );

	m_BtnStartGame.SetSkinJpg( IDR_JPG_GAMESTART + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_GAMESTART_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_GAMESTART_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnStartGame.MoveWindow( m_pRectCtrl[em_BtnStartGame] );
	m_BtnStartGame.ShowWindow( SW_HIDE );
	m_BtnStartGame.SetWindowText( L"" );

	m_BtnGameOption.SetSkinJpg( IDR_JPG_OPTION + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_OPTION_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_OPTION_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnGameOption.MoveWindow( m_pRectCtrl[em_BtnOption] );
	m_BtnGameOption.ShowWindow( SW_HIDE );
	m_BtnGameOption.SetWindowText( L"" );

	m_BtnClose.SetSkinJpg( IDR_JPG_CLOSE + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_CLOSE_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_CLOSE_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnClose.MoveWindow( m_pRectCtrl[em_BtnWinClose] );
	m_BtnClose.SetWindowText( L"" );

	m_BtnMinimum.SetSkinJpg( IDR_JPG_MINIMUM + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_MINIMUM_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_MINIMUM_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnMinimum.MoveWindow( m_pRectCtrl[em_BtnWinMiniMum] );
	m_BtnMinimum.SetWindowText( L"" );

#ifdef _USE_PARTITION_SELECT
	m_BtnSelectPartition.SetSkinJpg( IDR_JPG_SELECT_PARTITION + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_SELECT_PARTITION_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_SELECT_PARTITION_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnSelectPartition.MoveWindow( m_pRectCtrl[em_BtnSelectPartition] );
	m_BtnSelectPartition.SetWindowText( L"" );

	m_BtnStartPatch.SetSkinJpg( IDR_JPG_PARTCHSTART + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_PARTCHSTART_CLICK + DNPATCHINFO.GetLanguageOffset(), IDR_JPG_PARTCHSTART_ON + DNPATCHINFO.GetLanguageOffset() );
	m_BtnStartPatch.MoveWindow( m_pRectCtrl[em_BtnStartPatch] );
	m_BtnStartPatch.SetWindowText( L"" );
#endif // _USE_PARTITION_SELECT

	// ProgressBar Control
	m_pDownloadProgressBitmap = new CBitmap();
	HBITMAP hDownloadBitmap = (HBITMAP)::LoadImage( ::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_PROGRESS_DOWNLOAD), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS );
	m_pDownloadProgressBitmap->Attach( hDownloadBitmap );
	m_pDownloadProgress = new CSkinProgress( &m_StaticDownloadProgress, 100, m_pDownloadProgressBitmap );
	m_pDownloadProgress->SetRange( 0, 100 );
	m_StaticDownloadProgress.MoveWindow( m_pRectCtrl[em_DownloadProgress] );
	m_StaticDownloadProgress.SetWindowText( L"" );

	m_pFileCountProgressBitmap = new CBitmap();
	HBITMAP hCountBitmap = (HBITMAP)::LoadImage( ::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_PROGRESS_COUNT), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS );
	m_pFileCountProgressBitmap->Attach( hCountBitmap );
	m_pFileCountProgress = new CSkinProgress( &m_StaticFileCountProgress, 100, m_pFileCountProgressBitmap );
	m_pFileCountProgress->SetRange( 0, 100 );
	m_StaticFileCountProgress.MoveWindow( m_pRectCtrl[em_FileCountProgress] );
	m_StaticFileCountProgress.SetWindowText( L"" );

	// Text Control
	MakeFont();
	SetStaticText();
}

void CDnLauncherDlg::SetStaticText()
{
	m_StaticLauncherVersion.SetTextColor( m_staticBaseColor );
	m_StaticLauncherVersion.SetFont( &m_Font );
	m_StaticLauncherVersion.MoveWindow( m_pRectCtrl[em_Text_LauncherVersion] );

	m_StaticFileName.SetTextColor( m_staticBaseColor );
	m_StaticFileName.SetFont( &m_Font );
	m_StaticFileName.MoveWindow( m_pRectCtrl[em_Text_FileName] );

	m_StaticFileSize.SetTextColor( m_staticBaseColor );
	m_StaticFileSize.SetFont( &m_Font );
	m_StaticFileSize.MoveWindow( m_pRectCtrl[em_Text_FileSize] );

	m_StaticTimeLeft.SetTextColor( m_staticBaseColor );
	m_StaticTimeLeft.SetFont( &m_Font );
	m_StaticTimeLeft.MoveWindow( m_pRectCtrl[em_Text_TimeLeft] );

	m_StaticFileCount.SetTextColor( m_staticBaseColor );
	m_StaticFileCount.SetFont( &m_Font );
	m_StaticFileCount.MoveWindow( m_pRectCtrl[em_Text_FileCount] );

	m_StaticDownloadPercentage.SetTextColor( m_staticBaseColor );
	m_StaticDownloadPercentage.SetFont( &m_Font );
	m_StaticDownloadPercentage.MoveWindow( m_pRectCtrl[em_Text_DownloadPercentage] );

	m_StaticDownloadState.SetTextColor( m_staticBaseColor );
	m_StaticDownloadState.SetFont( &m_Font );
	m_StaticDownloadState.MoveWindow( m_pRectCtrl[em_Text_DownloadState] );

	m_StaticDirectGameStart.SetTextColor( m_staticBaseColor );
	m_StaticDirectGameStart.SetFont( &m_Font );
	m_StaticDirectGameStart.MoveWindow( m_pRectCtrl[em_Text_DirectStart] );
	m_StaticDirectGameStart.SetWindowText( _S( STR_AFTER_UPDATE_ATUOSTART + DNPATCHINFO.GetLanguageOffset() ) );

#ifdef _USE_PARTITION_SELECT
	m_StaticSelectPartition.SetTextColor( WHITE );
	m_StaticSelectPartition.SetFont( &m_Font );
	m_StaticSelectPartition.MoveWindow( m_pRectCtrl[em_Text_SelectPartition] );
	m_strSelectPartition = _S( STR_SELECT_PARTITION + DNPATCHINFO.GetLanguageOffset() );

	m_StaticSelectPartitionName.SetTextColor( RED );
	m_StaticSelectPartitionName.SetFont( &m_Font );
	m_StaticSelectPartitionName.MoveWindow( m_pRectCtrl[em_Text_SelectPartitionName] );

	m_StaticSelectPartitionGuide.SetTextColor( WHITE );
	m_StaticSelectPartitionGuide.SetFont( &m_Font );
	m_StaticSelectPartitionGuide.MoveWindow( m_pRectCtrl[em_Text_SelectPartitionGuide] );
	m_strSelectPartitionGuide = _S( STR_SELECT_PARTITION_GUIDE + DNPATCHINFO.GetLanguageOffset() );
#endif // _USE_PARTITION_SELECT
}

void CDnLauncherDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics( SM_CXICON );
		int cyIcon = GetSystemMetrics( SM_CYICON );
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon( x, y, m_hIcon );
	}
	else
	{
		CDialog::OnPaint();
	}
}

HBRUSH CDnLauncherDlg::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr = CDialog::OnCtlColor( pDC, pWnd, nCtlColor );
	RECT rect;

	switch(nCtlColor)
	{
	case CTLCOLOR_STATIC:
//	case CTLCOLOR_EDIT:
//	case CTLCOLOR_LISTBOX:
//	case CTLCOLOR_SCROLLBAR:
		{
			pDC->SetBkMode(TRANSPARENT);
			pWnd->GetWindowRect(&rect);
			RedrawWindow(&rect);
			return (HBRUSH)GetStockObject(NULL_BRUSH);;
		}
		break;
	}

	return hbr;
}

BOOL CDnLauncherDlg::OnEraseBkgnd( CDC* pDC )
{
	BitBlt( pDC->m_hDC, 0, 0, m_dwWidth, m_dwHeight, m_dcBkGrnd, 0, 0, SRCCOPY );
	return FALSE;
}

HCURSOR CDnLauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDnLauncherDlg::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( !(m_dwFlags & DRAGGING) )
	{
		m_pntMouse = point;
		m_dwFlags |= DRAGGING;
		SetCapture();
	}

	CDialog::OnLButtonDown( nFlags, point );
}

void CDnLauncherDlg::OnLButtonUp( UINT nFlags, CPoint point )
{
	if( m_dwFlags & DRAGGING )
	{
		m_dwFlags &= ~DRAGGING;
		ReleaseCapture();
	}

	CDialog::OnLButtonUp( nFlags, point );
}

void CDnLauncherDlg::OnMouseMove( UINT nFlags, CPoint point )
{
	if ( m_dwFlags & DRAGGING )
	{
		RECT rect;
		GetWindowRect( &rect );

		rect.left += point.x - m_pntMouse.x;
		rect.top += point.y - m_pntMouse.y;

		SetWindowPos( &wndTopMost, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		SendMessage( WM_PAINT );
	}

	CDialog::OnMouseMove( nFlags, point );
}

void CDnLauncherDlg::OnClickedCheckDirectGamestart()
{
	if( m_BtnCheckDirectGameStart.GetCheck() != DNOPTIONDATA.m_bAutoGameStart )
	{
		DNOPTIONDATA.m_bAutoGameStart = m_BtnCheckDirectGameStart.GetCheck();
		DNOPTIONDATA.SaveConfigOption( this->m_hWnd );
	}
}

void CDnLauncherDlg::OnClickButtonCancel()
{
	int nResult = AfxMessageBox( _S( STR_PATCH_CANCEL + DNPATCHINFO.GetLanguageOffset() ), MB_YESNO );
	if ( nResult == IDYES && m_emDownloadPatchState != PATCH_COMPLETE )	// Prevent issues if patching completes while confirmation dialog is open
	{
		if( g_pPatchDownloadThread )
		{
			g_pPatchDownloadThread->TerminateThread();
			g_pPatchDownloadThread->WaitForTerminate();

			SAFE_DELETE( g_pPatchDownloadThread );
			g_pPatchDownloadThread = NULL;
		}
		LogWnd::TraceLog(_T("* Patch Canceled"));

#ifdef _USE_PARTITION_SELECT
		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_SHOW );
#else // _USE_PARTITION_SELECT
		CDialog::OnCancel();
#endif // _USE_PARTITION_SELECT
	}
}

void CDnLauncherDlg::EnableStartButton()
{
	m_BtnCancel.ShowWindow( SW_HIDE );
	m_BtnStartGame.ShowWindow( SW_SHOW );
	m_BtnGameOption.ShowWindow( SW_SHOW );
}

void CDnLauncherDlg::OnClickButtonGameStart()
{

	// Minimum memory check
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);

	GlobalMemoryStatusEx( &statex );
	if( statex.ullAvailPhys < 800000000 )	// Output warning message when available memory is 800MB or less
	{
		AfxMessageBox( _S( STR_WARNNING_AVAILABLE_MEMORY + DNPATCHINFO.GetLanguageOffset() ), MB_OK );
		LogWnd::Log( LogLevel::Error, L"Warnning! Check Available Memory. You need atleast 800MB Ram free to play the game." );
	}

	CString strExeFile = DNPATCHINFO.GetClientPath();
	strExeFile += DNCLIENT_NAME;

	DNOPTIONDATA.SaveConfigOption( this->m_hWnd );
#ifdef _USE_PARTITION_SELECT
	DNOPTIONDATA.SavePartitionOption( this->m_hWnd );
#endif // _USE_PARTITION_SELECT

#ifdef ACTOZ_TEST_MESSAGE
	AfxMessageBox(DNPATCHINFO.GetTotalParameter().GetBuffer(), MB_OK);
#endif // ACTOZ_TEST_MESSAGE
	ShellExecute( m_hWnd, NULL, strExeFile.GetBuffer(), DNPATCHINFO.GetTotalParameter().GetBuffer(), NULL, SW_SHOWNORMAL );

#ifdef _USE_AUTOUPDATE	// Run auto-update.
	int nNextVersion = DNPATCHINFO.GetNextVersion();
	LogWnd::Log( LogLevel::Info, L"Check Next Version Ver:%d, Server Ver:%d", nNextVersion, DNPATCHINFO.GetServerVersion() );
	if( nNextVersion > 0 && nNextVersion > DNPATCHINFO.GetServerVersion() )
	{
		CString strAutoupdateParameter;
		strAutoupdateParameter.Format( L"%s|%s|%d-%d", DNPATCHINFO.GetPatchUrl(), DNPATCHINFO.GetClientPath(), DNPATCHINFO.GetServerVersion()+1, nNextVersion );

		CString strAutoUpdateExe = DNPATCHINFO.GetClientPath();
		strAutoUpdateExe += DNAUTOUPDATE_NAME;

		LogWnd::Log( LogLevel::Info, L"Execute AutoUpdate.exe %s", strAutoUpdateExe.GetBuffer( 0 ) );
		ShellExecute( m_hWnd, NULL, strAutoUpdateExe, strAutoupdateParameter, NULL, SW_SHOWNORMAL );
	}
#endif // _USE_AUTOUPDATE
	Sleep( 1000 );	// End after 1 second..

	CDialog::OnOK();
}

void CDnLauncherDlg::OnClickButtonOption()
{
	CDnOptionDlg dlg( this );
	INT_PTR nResponse = dlg.DoModal();
}

void CDnLauncherDlg::OnClickButtonClose()
{
	int nResult = AfxMessageBox( _S( STR_LAUNCHER_EXIT + DNPATCHINFO.GetLanguageOffset() ), MB_YESNO );
	if( nResult == IDYES )
	{
		LogWnd::TraceLog(_T("* Launcher Closed"));
		CDialog::OnCancel();
	}
}

void CDnLauncherDlg::OnClickMinimumBtn()
{
	ShowWindow( SW_SHOWMINIMIZED );
}

#ifdef _USE_PARTITION_SELECT
void CDnLauncherDlg::OnClickButtonStartPatch()
{
	if( !m_bDownloading && DNPATCHINFO.IsSetPatchInfo() )
	{
		int nResult = AfxMessageBox( _S( STR_PATCH_START + DNPATCHINFO.GetLanguageOffset() ), MB_YESNO );
		if( nResult == IDYES )
		{
			m_BtnCancel.ShowWindow( SW_SHOW );
			m_BtnStartPatch.ShowWindow( SW_HIDE );

			StartPatchDownloadThread();
		}
	}
	else
		ErrorMessageBoxLog( _S( STR_MUST_SELECT_PARTITION + DNPATCHINFO.GetLanguageOffset() ) );
}

void CDnLauncherDlg::OnClickButtonSelectPartition()
{
	if( !m_bDownloading )
	{
		// Open Server Select Page.
		CDnSelectPartitionDlg DnSelectPartitionDlg( this );
		WCHAR szUrl[4096]={0,};
		TCHAR szCurDir[ _MAX_PATH ]={0,};
		GetCurrentDirectory( _MAX_PATH, szCurDir );
		swprintf_s( szUrl , _T("%s\\Html\\HTMLPage1.htm"), szCurDir );

		DnSelectPartitionDlg.SetURL(szUrl);
		DnSelectPartitionDlg.DoModal();
	}
}

void CDnLauncherDlg::SetSelectPartition( int nSelectChannelNum, int nSelectPartitionNum )
{
	DNOPTIONDATA.m_nSelectChannelNum = nSelectChannelNum;
	DNOPTIONDATA.m_nSelectPartitionId = DNPATCHINFO.GetPartitionId( nSelectChannelNum, nSelectPartitionNum );

	if( !DNPATCHINFO.SetPatchInfo() )
		return;

	DNOPTIONDATA.SavePartitionOption( this->m_hWnd );

#ifdef _FIRST_PATCH
	CheckVersionAndReboot();
#else
	InitSelectPartition();
#endif

	m_pDnHtmlView->Navigate( DNPATCHINFO.GetGuidepageUrl() );
}

#ifdef _FIRST_PATCH
void CDnLauncherDlg::SetOnlySelectPartition( int nSelectChannelNum, int nSelectPartitionNum )
{
	//----------------------------------------------
	// This function unconditionally only selects and saves the partition.

	DNOPTIONDATA.m_nSelectChannelNum = nSelectChannelNum;
	DNOPTIONDATA.m_nSelectPartitionId = DNPATCHINFO.GetPartitionId( nSelectChannelNum, nSelectPartitionNum );

	if( !DNPATCHINFO.SetPatchInfo() )
	{
		return;
	}
	DNOPTIONDATA.SavePartitionOption( this->m_hWnd );
}

void CDnLauncherDlg::CheckVersionAndReboot()
{
	// Check version info of changed partition
	// If equal to current local version -> enable Game Start button
	// If current local version is higher -> request Full Client installation message
	// If current local version is lower -> enable Patch Start button	

	
	/*
----------- Module patch related -----------
	
During module patch, version comparison checks (module version == server version).
When running launcher patch, compare (launcher version == server version).
After completing module patch, if you reselect partition while the launcher is open and run [Restart Launcher] -> [Module Patch], the module version is already matched to the server. For example.
	
ex) When only module-patching and changing partition without pressing PatchStart on launcher.
	ex) Server 100 , Module 100 , Launcher 50
- An issue occurs when comparing versions to proceed with module patch.

To prevent this, when restarting due to partition change, if Module version is greater than launcher version, set it equal to launcher version.
Since [launcher version == module version] is enforced anyway, modified like this. (Module and launcher versions must always match)
And since module patching rarely happens, re-patching does not seem likely to impact performance much.

	*/

	int nClientVersion = DNPATCHINFO.GetClientVersion();
	int nSereverVersion = DNPATCHINFO.GetServerVersion();
	if( nClientVersion == nSereverVersion )
	{
		LogWnd::TraceLog( _T("Current Version is latest version") );
		OnStatusMsg( PATCH_COMPLETE, 0 );

		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_HIDE );
		m_pDownloadProgress->SetRange( 0, 100 );
		m_pDownloadProgress->SetPos( 100 );
		m_pFileCountProgress->SetRange( 0, 100 );
		m_pFileCountProgress->SetPos( 100 );
		m_strDownloadPercentage.Format( _T("%d%%"), 100 );

		if(	DNOPTIONDATA.m_bAutoGameStart )	// Start automatically if auto-start is checked.
			OnClickButtonGameStart();
	}
	else if( nClientVersion > nSereverVersion )
	{
		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_HIDE );
		m_BtnStartGame.ShowWindow( SW_HIDE );
		AfxMessageBox( _S( STR_ERROR_CLIENT_VERSION + DNPATCHINFO.GetLanguageOffset() ), MB_OK );
	}
	else if( nClientVersion < nSereverVersion )
	{
		// Reboot launcher.
		int nCurModuleVersion = DNFIRSTPATCHINFO.GetLocalModuleVersion();
		int nCurClientVersion = DNPATCHINFO.GetClientVersion();
		if( nCurModuleVersion > nCurClientVersion )
			nCurModuleVersion = nCurClientVersion;

		BOOL bSaveResult = SaveModuleVersion( nCurModuleVersion );
		if(bSaveResult)
		{
			CString strParam	= DNPATCHINFO.GetTotalParameter();	// Parameter
			CString strExeFile	= DNPATCHINFO.GetClientPath();		// Download path
			strExeFile += DNLAUNCHER_NAME;

#ifdef _USE_COMMAND_LINE
			ShellExecute( m_hWnd, NULL, strExeFile, g_szOriginalCmdLine.GetBuffer(), NULL, SW_SHOWNORMAL );
#else
			ShellExecute( m_hWnd, NULL, strExeFile, strParam.GetBuffer(), NULL, SW_SHOWNORMAL );
#endif
			KillMyProcess(_T(DNLAUNCHER_NAME)); // Kill old process.
		}
		else
		{
			ErrorMessageBoxLog( _S( STR_PATCH_FAILED+DNPATCHINFO.GetLanguageOffset() ) );
		}
	}
}

BOOL CDnLauncherDlg::SaveModuleVersion( int nVersion )
{

	//------------------------------------------
	// Save to version.cfg..
	// Version change
	CString szPath;
	szPath = DNPATCHINFO.GetClientPath();
	szPath += CLIENT_VERSION_NAME;

	char*   buffer = NULL;
	HANDLE	hFile  = INVALID_HANDLE_VALUE;
	hFile = CreateFile( szPath.GetBuffer(), GENERIC_WRITE, FILE_SHARE_READ , NULL, CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL,NULL );

	if( hFile == INVALID_HANDLE_VALUE )
	{
		LogWnd::Log( LogLevel::Error, L"Save Version File Failed!" );
		return FALSE;
	}

	char  szVersion[ 256 ]= {0,};
	DWORD dwWriteSize;

	sprintf_s( szVersion, sizeof(szVersion), "Version %d\r\nModule %d", DNPATCHINFO.GetClientVersion(), nVersion );
	WriteFile( hFile, szVersion,(int)strlen(szVersion)+1 , &dwWriteSize, NULL );
	CloseHandle(hFile);



	//------------------------------------------
	// Save to Pak file.
	CString szFindPackingFile;
	szFindPackingFile =  DNPATCHINFO.GetClientPath();
	szFindPackingFile += _T("Resource00.pak");

	CEtPackingFile *pPackingFile = new CEtPackingFile();
	char strVersion[256]={0,};
	sprintf_s( strVersion, sizeof(strVersion), "version %d\r\nModule %d", DNPATCHINFO.GetClientVersion(), nVersion );

	USES_CONVERSION;
	char szTemp[ _MAX_PATH ] = {0,};
	char StrVersionName[32]= "\\version.cfg";
	WideCharToMultiByte( CP_ACP, 0, szFindPackingFile.GetBuffer(), -1, szTemp, _MAX_PATH, NULL, NULL );

	// No choice. Pick one exceeding 512M and add it. 
	if( pPackingFile->OpenFileSystem( szTemp ) )
	{
		pPackingFile->Remove( StrVersionName );
		pPackingFile->AddFile( StrVersionName, strVersion, sizeof(strVersion) );
	}
	else 
	{
		AfxMessageBox( _T("Version can not be saved") );
		SAFE_DELETE( pPackingFile );
		return FALSE;
	}

	pPackingFile->CloseFileSystem();
	SAFE_DELETE( pPackingFile );

	return TRUE;
}
#endif

void CDnLauncherDlg::InitSelectPartition()
{
	ResetDownloadInfo();

	m_strSelectPartitionName = DNPATCHINFO.GetPartitionName();
	m_strSelectPartition = _S( STR_SELECT_PARTITION + DNPATCHINFO.GetLanguageOffset() );
	m_strSelectPartitionGuide = _S( STR_SELECT_PARTITION_GUIDE + DNPATCHINFO.GetLanguageOffset() );

	GetDlgItem( IDC_SELECT_PARTITION_NAME )->ShowWindow( SW_SHOW );
	GetDlgItem( IDC_SELECT_PARTITION_GUIDE )->ShowWindow( SW_SHOW );

	CheckVersion();
}

void CDnLauncherDlg::CheckVersion()
{
	// Check version info of changed partition
	// If equal to current local version -> enable Game Start button
	// If current local version is higher -> request Full Client installation message
	// If current local version is lower -> enable Patch Start button			
	int nClientVersion = DNPATCHINFO.GetClientVersion();
	int nSereverVersion = DNPATCHINFO.GetServerVersion();

	if( nClientVersion == nSereverVersion )
	{
		LogWnd::TraceLog( _T("Current Version is latest version") );
		OnStatusMsg( PATCH_COMPLETE, 0 );

		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_HIDE );
		m_pDownloadProgress->SetRange( 0, 100 );
		m_pDownloadProgress->SetPos( 100 );
		m_pFileCountProgress->SetRange( 0, 100 );
		m_pFileCountProgress->SetPos( 100 );
		m_strDownloadPercentage.Format( _T("%d%%"), 100 );

		if(	DNOPTIONDATA.m_bAutoGameStart )	// Start automatically if auto-start is checked.
			OnClickButtonGameStart();
	}
	else if( nClientVersion > nSereverVersion )
	{
		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_HIDE );
		m_BtnStartGame.ShowWindow( SW_HIDE );
		AfxMessageBox( _S( STR_ERROR_CLIENT_VERSION + DNPATCHINFO.GetLanguageOffset() ), MB_OK );
	}
	else if( nClientVersion < nSereverVersion )
	{
		m_BtnCancel.ShowWindow( SW_HIDE );
		m_BtnStartPatch.ShowWindow( SW_SHOW );
		m_BtnStartPatch.EnableWindow( TRUE );
		m_BtnStartGame.ShowWindow( SW_HIDE );
		m_BtnGameOption.ShowWindow( SW_HIDE );
		OnStatusMsg( PATCH_NONE, 0 );
	}
}
#endif // _USE_PARTITION_SELECT

void CDnLauncherDlg::OnTimer( UINT_PTR nIDEvent )
{
	if( nIDEvent == TEVENT_REFRESH_TEXTINFO )	// Update screen text info
		RefreshTextInfo();

	CDialog::OnTimer(nIDEvent);
}

void CDnLauncherDlg::StartPatchDownloadThread()
{
	if( g_pPatchDownloadThread == NULL )
	{
		g_pPatchDownloadThread = new CDnPatchDownloadThread();
		g_pPatchDownloadThread->SetWindowHandle( GetSafeHwnd() );
		g_pPatchDownloadThread->Start();
	}
}

void CDnLauncherDlg::CtlEraseBkgnd( int nCtlID )
{
	RECT rect;
	GetDlgItem( nCtlID )->GetWindowRect( &rect );
	this->ScreenToClient( &rect );
	InvalidateRect( &rect );
}

void CDnLauncherDlg::RefreshTextInfo()
{
	UpdateDownloadInfo();

	CString str; 
	GetDlgItem( IDC_LAUNCHER_VERSION )->GetWindowText( str );
	if( str != m_strLauncherVersion ) CtlEraseBkgnd( IDC_LAUNCHER_VERSION );

	GetDlgItem( IDC_FILE_NAME )->GetWindowText( str );
	if( str != m_strFileName ) CtlEraseBkgnd( IDC_FILE_NAME );

	GetDlgItem( IDC_FILE_SIZE )->GetWindowText( str );
	if( str != m_strFileSize ) 	CtlEraseBkgnd( IDC_FILE_SIZE );

	GetDlgItem( IDC_TIME_LEFT )->GetWindowText( str );
	if( str != m_strTimeLeft ) CtlEraseBkgnd( IDC_TIME_LEFT );
	
	GetDlgItem( IDC_FILE_COUNT )->GetWindowText( str );
	if( str != m_strFileCount ) CtlEraseBkgnd( IDC_FILE_COUNT );

	GetDlgItem( IDC_DOWNLOAD_PERCENTAGE )->GetWindowText( str );
	if( str != m_strDownloadPercentage ) CtlEraseBkgnd( IDC_DOWNLOAD_PERCENTAGE );

	GetDlgItem( IDC_DOWNLOAD_STATE )->GetWindowText( str );
	if( str != m_strDownloadState ) CtlEraseBkgnd( IDC_DOWNLOAD_STATE );

	GetDlgItem( IDC_DIRECT_START)->GetWindowText( str );
	if( str != m_strDirectGameStart) CtlEraseBkgnd( IDC_DIRECT_START);

#ifdef _USE_PARTITION_SELECT
	GetDlgItem( IDC_SELECT_PARTITION )->GetWindowText( str );
	if( str != m_strSelectPartition ) CtlEraseBkgnd( IDC_SELECT_PARTITION );
	GetDlgItem( IDC_SELECT_PARTITION_NAME )->GetWindowText( str );
	if( str != m_strSelectPartitionName ) CtlEraseBkgnd( IDC_SELECT_PARTITION_NAME );
	GetDlgItem( IDC_SELECT_PARTITION_GUIDE )->GetWindowText( str );
	if( str != m_strSelectPartitionGuide ) CtlEraseBkgnd( IDC_SELECT_PARTITION_GUIDE );
#endif // _USE_PARTITION_SELECT

	UpdateData( FALSE );
}

LRESULT CDnLauncherDlg::OnStatusMsg( WPARAM wParam, LPARAM lParam )
{
	m_emDownloadPatchState = static_cast<DownloadPatchState>(wParam);

	if( g_DownloadStateString[m_emDownloadPatchState].nStringID + DNPATCHINFO.GetLanguageOffset() != -1 )
	{
		m_strDownloadState = _S( g_DownloadStateString[m_emDownloadPatchState].nStringID + DNPATCHINFO.GetLanguageOffset() );
	}
	UpdateDownloadInfo();

	switch( m_emDownloadPatchState )
	{
	case PATCH_NONE:
		ResetDownloadInfo();
		m_bDownloading = FALSE;
		break;
	case PATCH_CHECK_VERSION:
		m_bDownloading = TRUE;
		break;
	case PATCH_COMPLETE:
		{
			EnableStartButton();
			SAFE_DELETE( g_pPatchDownloadThread );

			m_pDownloadProgress->SetRange( 0, 100 );
			m_pDownloadProgress->SetPos( 100 );
			m_pFileCountProgress->SetRange( 0, 100 );
			m_pFileCountProgress->SetPos( 100 );
			m_strDownloadPercentage.Format( _T("%d%%"), 100 );

			if(	DNOPTIONDATA.m_bAutoGameStart )	// Start automatically if auto-start is checked.
				OnClickButtonGameStart();

			m_bDownloading = FALSE;
		}
		break;
	case PATCH_FAILED:
	case PATCH_TERMINATE:
		{
			SAFE_DELETE( g_pPatchDownloadThread );
			m_bDownloading = FALSE;
		}
		break;
	case PATCH_LAUNCHER:
		{
			CDialog::DestroyWindow();
			m_bDownloading = FALSE;
		}
		break;
	}

	return 0;
}

void CDnLauncherDlg::ResetDownloadInfo()
{
	m_strFileName.Empty();
	m_strFileSize.Empty();
	m_strTimeLeft.Empty();
	m_strFileCount.Empty();
	m_strDownloadPercentage.Empty();
	m_strDownloadState.Empty();

	if( m_pDownloadProgress )
	{
		m_pDownloadProgress->SetRange( 0, 100 );
		m_pDownloadProgress->SetPos( 0 );
	}
	if( m_pFileCountProgress )
	{
		m_pFileCountProgress->SetRange( 0, 100 );
		m_pFileCountProgress->SetPos( 0 );
	}
}

void CDnLauncherDlg::UpdateDownloadInfo()
{
	// Downloading - display file, size, remaining time
	if( m_emDownloadPatchState == PATCH_DOWNLOAD_DATA || m_emDownloadPatchState == PATCH_MD5_CHECK
		|| m_emDownloadPatchState == PATCH_APPLY_PAK_FILE )
	{
		if( g_DownloadInfoData.m_strFileName.GetLength() > 0 )
			m_strFileName.Format( _T("%s:%s"), _S( STR_DLG_FILENAME + DNPATCHINFO.GetLanguageOffset() ), g_DownloadInfoData.m_strFileName );
		if( g_DownloadInfoData.m_nFileSize > 0 )
			m_strFileSize.Format( _T("%s:%s"), _S( STR_DLG_FILE_SIZE + DNPATCHINFO.GetLanguageOffset() ), GetAdjustFileSize( (double)g_DownloadInfoData.m_nFileSize ) );
		m_fDownloadRate = g_DownloadInfoData.m_fDownloadRate;
		UpdateDownloadLeftTime();

		// Downloading - progress bar
		m_nDownloadProgressMin = g_DownloadInfoData.m_nDownloadProgressMin;
		m_nDownloadProgressMax = g_DownloadInfoData.m_nDownloadProgressMax;
		m_nCurrentFileCount = g_DownloadInfoData.m_nCurrentFileCount;
		m_nTotalFileCount = g_DownloadInfoData.m_nTotalFileCount;
		UpdatProgressInfo();
	}


	// After patch data installation completes
	if( m_emDownloadPatchState == PATCH_APPLY_PAK_FILE_END )
		m_strLauncherVersion.Format( _T("%s Ver.%d"), _S( STR_DLG_DRAGON_NEST + DNPATCHINFO.GetLanguageOffset() ), DNPATCHINFO.GetClientVersion() );
}

void CDnLauncherDlg::UpdatProgressInfo()
{
	if( m_pDownloadProgress == NULL || m_pFileCountProgress == NULL )
		return;

	if( m_nDownloadProgressMax == 0 || m_nTotalFileCount == 0 )		// error divide by zero
		return;

	// Update download progress bar & % display
	m_pFileCountProgress->SetRange( 0, 100 );
	int nDownloadPercentage = 0;
	if( m_emDownloadPatchState == PATCH_DOWNLOAD_DATA || m_emDownloadPatchState == PATCH_APPLY_PAK_FILE )
		nDownloadPercentage = static_cast<int>( (100 * m_nDownloadProgressMin) / m_nDownloadProgressMax );
	else if( m_emDownloadPatchState == PATCH_MD5_CHECK )
		nDownloadPercentage = 100;

	if( nDownloadPercentage < 0 ) nDownloadPercentage = 0;
	if( nDownloadPercentage > 100 ) nDownloadPercentage = 100;

	m_strDownloadPercentage.Format( _T("%d%%"), nDownloadPercentage );
	m_pDownloadProgress->SetPos( nDownloadPercentage );

	// Update total file count progress bar (split each file into 100 units: 50 for download, 50 for packing)
	m_pFileCountProgress->SetRange( 0, m_nTotalFileCount * 100 );	// Divide file count range into 100
	int nCurrentFileCountPercentage = 0;

	if( m_emDownloadPatchState == PATCH_DOWNLOAD_DATA || m_emDownloadPatchState == PATCH_MD5_CHECK )
		nCurrentFileCountPercentage = ((m_nCurrentFileCount - 1) * 100) + (nDownloadPercentage / 2);
	else if( m_emDownloadPatchState == PATCH_APPLY_PAK_FILE )
		nCurrentFileCountPercentage = ((m_nCurrentFileCount - 1) * 100) + (nDownloadPercentage / 2) + 50;

	if( nCurrentFileCountPercentage < 0 ) nCurrentFileCountPercentage = 0;
	if( nCurrentFileCountPercentage > m_nTotalFileCount * 100 ) nCurrentFileCountPercentage = m_nTotalFileCount * 100;

	m_pFileCountProgress->SetPos( nCurrentFileCountPercentage );
	m_strFileCount.Format( _T("%d/%d"), m_nCurrentFileCount, m_nTotalFileCount );

}

CString CDnLauncherDlg::GetAdjustFileSize( double size )
{
	CString str;

	if( size >= 1024 * 1024 )
		str.Format( _T("%.1f MB") , size / (1024.0f * 1024.0f) );
	else if( size >= 1024 )
		str.Format( _T("%.1f KB") , size / 1024.0f );
	else
		str.Format( _T("%d B") , size );

	return str;
}

void CDnLauncherDlg::UpdateDownloadLeftTime()
{
	if( m_fDownloadRate > 0 )
	{
		// Divide by 1000 since it is in KB/s
		int nEstimateTimeLeft = static_cast<int>( static_cast<float>( m_nDownloadProgressMax - m_nDownloadProgressMin ) / m_fDownloadRate / 1000.0f );

		if( nEstimateTimeLeft > 3600 )
		{
			int nHour = nEstimateTimeLeft / 3600;
			int nMin = (nEstimateTimeLeft - (nHour * 3600)) / 60;
			int nSec = nEstimateTimeLeft - (nHour * 3600) - (nMin * 60);
			m_strTimeLeft.Format( _S( STR_DLG_TIME_HOUR_MIN_SEC + DNPATCHINFO.GetLanguageOffset() ), nHour, nMin, nSec );
		}
		else if( nEstimateTimeLeft <= 3600 && nEstimateTimeLeft > 60 )
		{
			int nMin = nEstimateTimeLeft / 60;
			int nSec = nEstimateTimeLeft - (nMin * 60);
			m_strTimeLeft.Format( _S( STR_DLG_TIME_MIN_SEC + DNPATCHINFO.GetLanguageOffset() ), nMin, nSec );
		}
		else if( nEstimateTimeLeft <= 60 && nEstimateTimeLeft >= 0)
			m_strTimeLeft.Format( _S( STR_DLG_TIME_SEC + DNPATCHINFO.GetLanguageOffset() ), nEstimateTimeLeft );
	}
}

BOOL CDnLauncherDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add specialized code here and/or call the base class.

	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN )
	{
		if(m_BtnStartGame && m_BtnStartGame.IsWindowVisible())
		{
			OnClickButtonGameStart();
			return true;
		}
		return true;
	}

	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE )
	{
		if(m_BtnClose && m_BtnClose.IsWindowVisible())
		{
			OnClickButtonClose();
			return true;
		}
		return true;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
