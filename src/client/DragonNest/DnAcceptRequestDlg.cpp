#include "StdAfx.h"
#include "DnAcceptRequestDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAcceptRequestDlg::CDnAcceptRequestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pProgressBarTime(NULL)
, m_pButtonOK(NULL)
, m_pButtonReject(NULL)
, m_pButtonCancel(NULL)
, m_fTotalTime(0.0f)
, m_fElapsedTime(0.0f)
, m_bAccept(false)
{
	int i = 0;
	for (; i < eTypeMax; ++i)
	{
		m_pMainText[i] = NULL;
	}
}

CDnAcceptRequestDlg::~CDnAcceptRequestDlg(void)
{
}

void CDnAcceptRequestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AcceptRequestDlg.ui" ).c_str(), bShow );
}

void CDnAcceptRequestDlg::InitialUpdate()
{
	int i = 0;
	std::string ctrlName;
	for (; i < eTypeMax; ++i)
	{
		ctrlName = FormatA("ID_HTMLTEXTBOX%d", i);
		m_pMainText[i] = GetControl<CEtUIHtmlTextBox>(ctrlName.c_str());
	}

	m_pProgressBarTime = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR_TIME");
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonReject = GetControl<CEtUIButton>("ID_REJECT");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");
}

void CDnAcceptRequestDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( m_fElapsedTime <= 0.0f )
	{
		if( m_bAccept )
			CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonReject, 0 );

		// Request cancellation is not the default.
		//else
		//	CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonCancel, 0 );

		Show(false);
	}
	else
	{
		// The dialog only displays it. (Does not directly adjust the progress bar.)
		//m_fElapsedTime -= fElapsedTime;

		if( m_fTotalTime != 0.0f)
		{
			m_pProgressBarTime->SetProgress( m_fElapsedTime / m_fTotalTime * 100.0f );
		}
		else
		{
			m_pProgressBarTime->SetProgress( 0.0f );
		}
	}
}

void CDnAcceptRequestDlg::Show( bool bShow )
{ 
	if( bShow == m_bShow )
		return;

	if( !bShow )
	{
#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
#else
		SetDialogID(-1);
		SetCallback(NULL);
#endif
		m_fTotalTime = 0.0f;
		m_fElapsedTime = 0.0f;
	}
	else
	{
		//m_pProgressBarTime->SetProgress(100.0f);
	}

	CEtUIDialog::Show( bShow );
	//CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

void CDnAcceptRequestDlg::SetInfo( LPCWSTR pwszMessage, float fTotalTime, bool bAccept, eAcceptRequestType type, int nID, CEtUICallback *pCall )
{
	m_nDialogID = nID;
	m_pCallback = pCall;
	m_fTotalTime = fTotalTime;
	m_fElapsedTime = fTotalTime;

	if (type < 0 || type >= eTypeMax)
	{
		_ASSERT(0);
		return;
	}

	int i = 0;
	for (; i < eTypeMax; ++i)
	{
		m_pMainText[i]->ClearText();
		m_pMainText[i]->Show(false);
	}

	m_pMainText[type]->ClearText();
	m_pMainText[type]->ReadHtmlString(pwszMessage);
	m_pMainText[type]->Show(true);

	m_bAccept = bAccept;

	if( m_bAccept )
	{
		m_pButtonOK->Show( true );
		m_pButtonReject->Show( true );
		m_pButtonCancel->Show( false );
	}
	else
	{
		m_pButtonOK->Show( false );
		m_pButtonReject->Show( false );
		m_pButtonCancel->Show( true );
	}
}

void CDnAcceptRequestDlg::SetElapsedTime( float fElapsedTime )
{
	fElapsedTime = min(fElapsedTime, m_fTotalTime);
	fElapsedTime = max(fElapsedTime, 0.0f);
	m_fElapsedTime = fElapsedTime;
}

void CDnAcceptRequestDlg::OnTimeIsUp()
{
	if (GetCallBack())
		GetCallBack()->OnUICallbackProc(m_nDialogID, EVENT_BUTTON_CLICKED, m_pButtonReject, 0);

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
	SetDialogID(-1);
	SetCallback(NULL);
#endif
}
