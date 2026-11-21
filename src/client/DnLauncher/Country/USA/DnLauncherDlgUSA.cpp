#include "stdafx.h"
#include "DnLauncherDlgUSA.h"


extern CDnPatchDownloadThread* g_pPatchDownloadThread;

CDnLauncherDlgUSA::CDnLauncherDlgUSA( CWnd* pParent )
: CDnLauncherDlg( pParent )
{
	m_nMaxCtrlNum = em_MaxNum;
	m_pRectCtrl = new CRect[em_MaxNum];
	m_staticBaseColor = RGB( 146, 146, 146 );
}


BEGIN_MESSAGE_MAP( CDnLauncherDlgUSA, CDnLauncherDlg )
	ON_WM_DESTROY()
	ON_BN_CLICKED( IDC_BTN_CANCEL, &CDnLauncherDlgUSA::OnClickButtonCancel )
	ON_BN_CLICKED( IDC_BTN_GAMESTART, &CDnLauncherDlgUSA::OnClickButtonGameStart )
	ON_BN_CLICKED( IDC_BTN_CLOSE, &CDnLauncherDlgUSA::OnClickButtonClose )
END_MESSAGE_MAP()


void CDnLauncherDlgUSA::OnDestroy()
{
	SAFE_DELETE_ARRAY( m_pRectCtrl );

	CDnLauncherDlg::OnDestroy();
}

void CDnLauncherDlgUSA::MakeFont()
{
	m_Font.CreateFont( 14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, _T(" Lucida Sans Unicode") );
}

BOOL CDnLauncherDlgUSA::InitWebpage()
{
	return CDnLauncherDlg::InitWebpage();
}

void CDnLauncherDlgUSA::OnClickButtonCancel()
{
	int nResult = AfxMessageBox( STR_PATCH_CANCEL, MB_YESNO );
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

void CDnLauncherDlgUSA::OnClickButtonGameStart()
{
	CDnLauncherDlg::OnClickButtonGameStart();
}

void CDnLauncherDlgUSA::OnClickButtonClose()
{
	int nResult = AfxMessageBox( STR_LAUNCHER_EXIT, MB_YESNO );

	if( nResult == IDYES )
	{
		LogWnd::TraceLog(_T("* Launcher Closed"));
		CDialog::OnCancel();
	}
}
