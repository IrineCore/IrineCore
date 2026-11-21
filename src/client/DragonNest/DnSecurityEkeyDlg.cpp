#include "StdAfx.h"
#include "DnSecurityEkeyDlg.h"
#include "DnInterface.h"
#include "DnChannelListDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include "DnLoginDlg.h"


/*
#if defined(PRE_ADD_CH_SNDAAUTH_02)
#else	// #if defined(PRE_ADD_CH_SNDAAUTH_02)
#ifdef PRE_ADD_CH_SNDAAUTH_01
// When using EKey 
//--- Full number usage mode when using A8
//	A8 First press the document icon to the left of the center button.
//	Press the challenge number received from server. 
//	Pressing the center OK button shows the full A8 number 
//	Just pass this pool number.
//
//--- D6 X6 
//	Receive challenge number from server.
//	This number indicates the digit position; place the digit into that position. 
//	ex) 929101 Challenge : 4623 => 1129 
//


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSecurityEkeyDlg::CDnSecurityEkeyDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
}

CDnSecurityEkeyDlg::~CDnSecurityEkeyDlg(void)
{
}

void CDnSecurityEkeyDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Security_Ekey_Number.ui" ).c_str(), bShow );
}

void CDnSecurityEkeyDlg::InitialUpdate()
{
	m_ChallengeKey.clear();
}

void CDnSecurityEkeyDlg::Show( bool bShow )
{
	if( !bShow )
	{
		GetControl<CEtUIEditBox>("ID_EDITBOX1")->ClearTextMemory();
	}

	CEtUIDialog::Show(bShow );
}

void CDnSecurityEkeyDlg::SetType(emSecurityType Stype )
{
	m_SecurityType = Stype; 
	WCHAR szStr[256]={0,};
	switch(Stype)
	{
		case emEKey:
GetControl<CEtUIStatic>("ID_TEXT0")->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6469)); // 6469 Ekey input break;
swprintf_s(szStr, _countof(szStr) ,  _T("%s : %s") ,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6470) , m_ChallengeKey.c_str() ); // 6470 Code value to enter
			GetControl<CEtUIStatic>("ID_TEXT1")->SetText(szStr);
			GetControl<CEtUIStatic>("ID_TEXT1")->Show(true);
			break; 
		case emECradNo: 			
			GetControl<CEtUIStatic>("ID_TEXT0")->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6473));break; //6473 EcardNo 
			GetControl<CEtUIStatic>("ID_TEXT1")->Show(false);
			break; 
		case emResvPass: 
GetControl<CEtUIStatic>("ID_TEXT0")->SetText(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6472));break; //6472 Reserve key
			GetControl<CEtUIStatic>("ID_TEXT1")->Show(false);
			break; 
	}
}

void CDnSecurityEkeyDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) || IsCmdControl("ID_BUTTON_CANCEL")) 
		{
			GetInterface().GetLoginDlg()->EnableLoginButton( true );
			Show( false );
		}
		else if(IsCmdControl( "ID_BUTTON_OK" ))
		{
			std::wstring str; 
			str = GetControl<CEtUIEditBox>("ID_EDITBOX1")->GetText();
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");

			switch(m_SecurityType)
			{
			case emEKey: 
				{
					if( pTask )
					{
						pTask->RequestCheckEKey((WCHAR*)str.c_str());
					}
					break; 
				}
			
			case emECradNo: 
				{
					if( pTask )
					{
						pTask->RequestCheckECardNo((WCHAR*)str.c_str());
					}
				
					break; 
				}
			case emResvPass: 
				{
					if( pTask )
					{
						pTask->RequestCheckEKeyResvPass((WCHAR*)str.c_str());
					}
					break; 
				}
			}
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnSecurityEkeyDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime ); 
	
	if( !IsShow() )
		return;

}

void CDnSecurityEkeyDlg::SetChallengeKey(WCHAR *pStr)
{
	m_ChallengeKey = pStr; 
}

#endif 
#endif	// #if defined(PRE_ADD_CH_SNDAAUTH_02)

*/