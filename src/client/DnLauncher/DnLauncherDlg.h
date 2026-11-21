//////////////////////////////////////////////////////////////////////////
// DnLauncherDlg.h : header file
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "xSkinButton.h"
#include "SkinProgress.h"
#include "ColorStatic.h"
#include "DnPatchThread.h"

#define DRAGGING	0x01	// dragging flag

enum emDlgPosIndex 
{
	em_WebPageHtml = 0,
	em_BtnQuit,
	em_BtnStartGame,
	em_BtnOption, 
	em_BtnWinClose,
	em_BtnWinMiniMum,
	em_DownloadProgress,
	em_FileCountProgress,
	em_Text_LauncherVersion,
	em_Text_FileCount,
	em_Text_FileName,
	em_Text_TimeLeft,
	em_Text_FileSize,
	em_Text_DownloadPercentage,
	em_Text_DownloadState,
	em_Text_DirectStart,
	em_CheckBox_DIrectStart,
#ifdef _USE_PARTITION_SELECT
	em_BtnStartPatch,
	em_BtnSelectPartition,
	em_Text_SelectPartition,
	em_Text_SelectPartitionName,
	em_Text_SelectPartitionGuide,
#endif // _USE_PARTITION_SELECT
	em_MaxNum,
};


class CDnHtmlView;
class CDnLauncherDlg : public CDialog
{
// Construction
public:
	CDnLauncherDlg( CWnd* pParent = NULL );
	virtual ~CDnLauncherDlg() {}

public:
#ifdef _USE_PARTITION_SELECT
	void SetSelectPartition( int nSelectChannelNum, int nSelectPartitionNum );		// Handle selection info received from partition selection Html
	void InitSelectPartition();
#endif // _USE_PARTITION_SELECT

#ifdef _FIRST_PATCH
	void SetOnlySelectPartition( int nSelectChannelNum, int nSelectPartitionNum );		// Handle selection info received from partition selection Html
	void CheckVersionAndReboot();
	BOOL SaveModuleVersion( int nVersion );
#endif
	
	static CDnLauncherDlg* CreateLauncherDlg();

	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg LRESULT OnStatusMsg( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg HCURSOR OnQueryDragIcon();
	// Button Ctrl Click
	afx_msg void OnClickedCheckDirectGamestart();
	afx_msg void OnClickButtonCancel();
	afx_msg void OnClickButtonGameStart();
	afx_msg void OnClickButtonOption();
	afx_msg void OnClickButtonClose();
	afx_msg void OnClickMinimumBtn();
#ifdef _USE_PARTITION_SELECT
	afx_msg void OnClickButtonStartPatch();
	afx_msg void OnClickButtonSelectPartition();
#endif // _USE_PARTITION_SELECT

	DECLARE_MESSAGE_MAP()

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void InitControl();			// Initialize UI Control
	virtual void MakeFont() {}			// Create font
	virtual void SetStaticText();		// Set text
	virtual void RefreshTextInfo();		// Update text info
	virtual void EnableStartButton();	// Enable start button
	virtual void NavigateHtml();		// Navigate to web page
	virtual BOOL InitWebpage();			// Initialize guide web page
	virtual BOOL InitBackground();		// Initialize background image
#ifdef _USE_PARTITION_SELECT
	virtual void CheckVersion();		// Check client and server versions to configure the launcher
#endif // _USE_PARTITION_SELECT

	BOOL InitFail();					// Launcher initialization failed
	BOOL InitInterface();				// Initialize UI

	void StartPatchDownloadThread();	// Start patch download thread
	void CtlEraseBkgnd( int nCtlID );

	// Update Download Info
	void ResetDownloadInfo();					// Initialize download info
	void UpdateDownloadInfo();					// Update download info
	CString GetAdjustFileSize( double size );	// Build file size string
	void UpdatProgressInfo();					// Update progress info
	void UpdateDownloadLeftTime();				// Update remaining download time
	CRect* GetControlPos( emDlgPosIndex index ) { return &m_pRectCtrl[index]; }

// Implementationz
public:
	// Dialog Data
	enum { IDD = IDD_DNLAUNCHER_DIALOG };
	enum { TEVENT_REFRESH_TEXTINFO = 0 };	// Timer event
	std::vector<POINT> m_vecDisplayMode;	// For storing local resolution info

protected:
	// Interface-related member variables
	CRect*	m_pRectCtrl;
	int		m_nMaxCtrlNum;

	HICON	m_hIcon;
	DWORD	m_dwWidth;
	DWORD	m_dwHeight;
	HDC		m_dcBkGrnd;			// background bitmap dc
	HBITMAP	m_hBmpBkGrnd;		// owned background bitmap handle
	HGDIOBJ	m_hOldBmpBkGrnd;	// previous bitmap selected in m_dcBkGrnd
	DWORD	m_dwFlags;			// flags
	POINT	m_pntMouse;			// Mouse position for dragging
	CFont	m_Font;				// Font

	DownloadPatchState	m_emDownloadPatchState;	// Download status
	
	// ProgressBar
	CSkinProgress*	m_pDownloadProgress;
	CSkinProgress*	m_pFileCountProgress;
	CBitmap*		m_pDownloadProgressBitmap;
	CBitmap*		m_pFileCountProgressBitmap;
	CStatic			m_StaticDownloadProgress;
	CStatic			m_StaticFileCountProgress;
	__int64			m_nDownloadProgressMin;
	__int64			m_nDownloadProgressMax;
	int				m_nCurrentFileCount;
	int				m_nTotalFileCount;
	float			m_fDownloadRate;
	BOOL			m_bDownloading;

	// Button
	CButton			m_BtnCheckDirectGameStart;
	CxSkinButton	m_BtnCancel;
	CxSkinButton	m_BtnStartGame;
	CxSkinButton	m_BtnGameOption;
	CxSkinButton	m_BtnClose;
	CxSkinButton	m_BtnMinimum;
#ifdef _USE_PARTITION_SELECT
	CxSkinButton	m_BtnStartPatch;
	CxSkinButton	m_BtnSelectPartition;
#endif // _USE_PARTITION_SELECT

	// String
	CString		m_strLauncherVersion;		// Launcher version
	CString		m_strFileName;				// Downloading file name
	CString		m_strFileSize;				// Downloading file size
	CString		m_strTimeLeft;				// Remaining download time
	CString		m_strFileCount;				// Number of files to download
	CString		m_strDownloadPercentage;	// Download percentage
	CString		m_strDownloadState;			// Download status
	CString		m_strDirectGameStart;		// Start game immediately after patch completes
#ifdef _USE_PARTITION_SELECT
	CString		m_strSelectPartition;		// Selected partition
	CString		m_strSelectPartitionName;	// Selected partition name
	CString		m_strSelectPartitionGuide;	// Partition selection notice
#endif // _USE_PARTITION_SELECT

	// Static
	CColorStatic	m_StaticLauncherVersion;
	CColorStatic	m_StaticFileName;
	CColorStatic	m_StaticFileSize;
	CColorStatic	m_StaticTimeLeft;
	CColorStatic	m_StaticFileCount;
	CColorStatic	m_StaticDownloadPercentage;
	CColorStatic	m_StaticDownloadState;
	CColorStatic	m_StaticDirectGameStart;
#ifdef _USE_PARTITION_SELECT
	CColorStatic	m_StaticSelectPartition;
	CColorStatic	m_StaticSelectPartitionName;
	CColorStatic	m_StaticSelectPartitionGuide;
#endif // _USE_PARTITION_SELECT
	CColorStatic	m_StaticszDirectStart;	// Static text for auto-launching game after patch completes
	COLORREF		m_staticBaseColor;		// Static default color

	CDnHtmlView*	m_pDnHtmlView;			// Service provider webpage in launcher
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
