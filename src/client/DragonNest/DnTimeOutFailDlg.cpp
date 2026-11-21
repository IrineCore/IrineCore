#include "StdAfx.h"
#include "DnTimeOutFailDlg.h"
#include "DnPartyTask.h"
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTimeOutFailDlg::CDnTimeOutFailDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_fDisplayTime( 0.0f )
{
}

CDnTimeOutFailDlg::~CDnTimeOutFailDlg(void)
{
}

void CDnTimeOutFailDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "TimeOverDlg.ui" ).c_str(), bShow );
}

void CDnTimeOutFailDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_fDisplayTime = 5.0f;
	}
	else
	{
		m_fDisplayTime = 0.0f;
	}

	CEtUIDialog::Show( bShow );
}

void CDnTimeOutFailDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );

	if( IsShow() )
	{
		if( m_fDisplayTime > 0.0f )
		{
			m_fDisplayTime -= fElapsedTime;
		}
		else
		{
			if( CDnPartyTask::GetInstance().GetPartyRole() != CDnPartyTask::MEMBER )
			{
				Show( false );
				GetInterface().OpenMissionFailMoveDialog();
			}
		}
	}
}

void CDnTimeOutFailDlg::OnBlindOpen() 
{
	CDnLocalPlayerActor::LockInput(true);
}

void CDnTimeOutFailDlg::OnBlindOpened()
{
	Show( true );
	GetInterface().ShowChatDialog();
}

void CDnTimeOutFailDlg::OnBlindClose()
{
	Show( false );
}

void CDnTimeOutFailDlg::OnBlindClosed()
{
	GetInterface().OpenBaseDialog();
	CDnLocalPlayerActor::LockInput(false);
}
