#include "StdAfx.h"
#include "DnSecurityNUMPADChangeDlg.h"
#include "DnAuthTask.h"
#include "TaskManager.h"
#include "DnInterface.h"
#include <MMSystem.h>


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSecurityNUMPADChangeDlg::CDnSecurityNUMPADChangeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback , true  )
{
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_vecNumberStr.clear();
	m_pEditNewPw = NULL;
	m_pEditEnterPw = NULL;
	m_pEditOldPw = NULL;
}

CDnSecurityNUMPADChangeDlg::~CDnSecurityNUMPADChangeDlg(void)
{
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_vecNumberStr.clear();
}

void CDnSecurityNUMPADChangeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SecurityNUMPAD_Change.ui" ).c_str(), bShow );
}

void CDnSecurityNUMPADChangeDlg::InitialUpdate()
{
	char Strtmp[1024]={0,};
	WCHAR wszBuffer[32]={0,};

	for(int i = 0 ; i< 10 ; ++i)
	{
		sprintf_s(Strtmp , 1024 , "ID_BUTTON_NUM%d" , i );
		m_pBtnNumPad.push_back(GetControl<CEtUIButton>( Strtmp));
		//	_itow_s( i, wszBuffer, _countof(wszBuffer), 10 ) ;
		m_vecNumberStr.push_back( m_pBtnNumPad[i]->GetText() );
	}

	m_pEditOldPw = GetControl<CEtUIEditBox>("ID_EDITBOX0") ; 
	m_pEditOldPw->KeyLock(false);
	m_pEditOldPw->ClearTextMemory();

	m_pEditNewPw = GetControl<CEtUIEditBox>("ID_EDITBOX1") ; 
	m_pEditNewPw->KeyLock(false);
	m_pEditNewPw->ClearTextMemory();

	m_pEditEnterPw = GetControl<CEtUIEditBox>("ID_EDITBOX2");
	m_pEditEnterPw->KeyLock(false);
	m_pEditEnterPw->ClearTextMemory();

	m_pBtnDelete = GetControl<CEtUIButton>("ID_BTN_DELETE");

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	m_pBtnShuffle = GetControl<CEtUIButton>("ID_BTN_SHUFFLE");
#endif
}


void CDnSecurityNUMPADChangeDlg::Show( bool bShow )
{
	if( bShow == m_bShow )
		return;
	//Due to call order... show first... 
	CEtUIDialog::Show( bShow );

	if( !bShow )
	{
	}
	else 
	{
		InitializeNumber();
		SetCurrentFocus( emAuthPwFocus::OldPw) ;
		ShuffleNumber();
		m_SecondPass1.nSeed = m_SecondPass2.nSeed = m_SecondPass0.nSeed;
	}
}




bool CDnSecurityNUMPADChangeDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		return true;
	}
	else if ( uMsg == WM_LBUTTONDOWN )
	{
		POINT MousePoint;
		float fMouseX, fMouseY;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( m_pBtnDelete->IsInside(fMouseX ,fMouseY ) ) 
		{
			SetCurrentFocus(m_eAuthPwFocus);
			MouseLClickPress = true; 
			m_fTime = 0.0f; 
		}
	}
#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	else if ( uMsg == WM_MOUSEMOVE )
	{
		POINT MousePoint;
		float fMouseX, fMouseY;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( CheckAreaSecurityNumber( fMouseX ,fMouseY ) )
		{
			CDnMouseCursor::GetInstance().ShowCursor( false );

			if( m_vecNumberStrCopy.size() == ( m_pBtnNumPad.size() + 2 ) )
				return false;

			for( int i = 0 ; i< (int)m_pBtnNumPad.size() ; ++i )
			{
				m_vecNumberStrCopy.push_back ( m_pBtnNumPad[i]->GetText() );
				m_pBtnNumPad[i]->ClearText();
			}
			m_vecNumberStrCopy.push_back ( m_pBtnDelete->GetText() );
			m_vecNumberStrCopy.push_back ( m_pBtnShuffle->GetText() );

			m_pBtnDelete->ClearText();
			m_pBtnShuffle->ClearText();

		}
		else 
		{
			for( int i = 0 ; i < (int)m_vecNumberStrCopy.size(); ++i )
			{
				if( i < 10)	m_pBtnNumPad[i]->SetText( m_vecNumberStrCopy[i].c_str() );
				else if( i == 10 ) m_pBtnDelete->SetText( m_vecNumberStrCopy[10].c_str() );
				else if( i == 11) m_pBtnShuffle->SetText( m_vecNumberStrCopy[11].c_str() );
			}
			m_vecNumberStrCopy.clear();

			CDnMouseCursor::GetInstance().ShowCursor( true );
		}
	}
#endif 

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
bool CDnSecurityNUMPADChangeDlg::CheckAreaSecurityNumber( float fMouseX, float fMouseY )
{	
	// Check all mouse keypads 
	int nSize = (int)m_pBtnNumPad.size();
	for( int i = 0; i < nSize; ++i )
		if( m_pBtnNumPad[i]->IsInside( fMouseX, fMouseY ) )	return true;

	if( m_pBtnDelete->IsInside( fMouseX, fMouseY ) )	return true;
	if( m_pBtnShuffle->IsInside( fMouseX, fMouseY ) )	return true;

	return false;
}
#endif 

void CDnSecurityNUMPADChangeDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() ) {
		bool bCanShuffle = false;

		if( IsCurPwFocus(emAuthPwFocus::OldPw) && m_SecondPass0.IsEmpty() ) bCanShuffle = true;
		else if( IsCurPwFocus(emAuthPwFocus::NewPw) && m_SecondPass1.IsEmpty() ) bCanShuffle = true;
		else if( IsCurPwFocus(emAuthPwFocus::EnterPw) && m_SecondPass2.IsEmpty() ) bCanShuffle = true;

		if( bCanShuffle != GetControl<CEtUIButton>("ID_BTN_SHUFFLE")->IsEnable() )
			GetControl<CEtUIButton>("ID_BTN_SHUFFLE")->Enable( bCanShuffle );
	}

	if( MouseLClickPress )
	{
		m_fTime += fElapsedTime ; 

		if( m_fTime > DELETE_BTN_DELAY)
		{
			m_fTime = 0.0f;
			MouseLClickPress = false;

			if(IsCurPwFocus(emAuthPwFocus::OldPw) ) 
			{
				if( !m_SecondPass0.IsEmpty() ) {
					m_SecondPass0.Clear( false );
					m_pEditOldPw->SetText( m_SecondPass0.GetStr().c_str() );
				}
			}
			else if(IsCurPwFocus(emAuthPwFocus::NewPw)) 
			{
				if( !m_SecondPass1.IsEmpty() ) {
					m_SecondPass1.Clear( false );
					m_pEditNewPw->SetText( m_SecondPass1.GetStr().c_str() );
				}
			}
			else if(IsCurPwFocus(emAuthPwFocus::EnterPw) )
			{
				if( !m_SecondPass2.IsEmpty() ) {
					m_SecondPass2.Clear( false );
					m_pEditEnterPw->SetText( m_SecondPass2.GetStr().c_str() );
				}
			}
		}
	}
	else 
	{
		if( m_fTime > 0.00001f )
		{
			KeyDelete();
			m_fTime = 0.0f; 
		}
	}




}


void CDnSecurityNUMPADChangeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{

		if ( IsCmdControl("ID_BTN_SHUFFLE"))
		{
			SetCurrentFocus(m_eAuthPwFocus);
			ShuffleNumber();
		}
		else if ( IsCmdControl("ID_BTN_DELETE"))
		{
			MouseLClickPress = false; 
		}
		else if( IsCmdControl( "ID_BUTTON_CLOSE" ) )
		{
			Show( false );
		}
		else if( IsCmdControl( "ID_BUTTON_CANCEL" ) )
		{
#if defined(PRE_ADD_23829)
			SecondPassCheck(m_pCheckBox ? m_pCheckBox->IsChecked() : false);
#endif // PRE_ADD_23829

#ifdef PRE_MDD_USA_SECURITYNUMPAD
			// For US, secondary password entry is mandatory. 
			Show(false);
#else
			CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");
			int nValue[4] = { 0, };
			pLoginTask->SetAuthPassword(0, nValue);
			pLoginTask->ChangeState(CDnLoginTask::LoginStateEnum::ChannelList);
			//SelectChannel();

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
			pLoginTask->ResetPreviewCashCostume();
#endif

#ifdef PRE_ADD_SETCHECK
			SetCheckShow(m_pCheckBox->IsChecked());
#endif 

#endif // PRE_MDD_USA_SECURITYNUMPAD
			Show(false);
		}
		else if( IsCmdControl( "ID_BUTTON_OK" ))
		{
			if(CheckMaxCountPw(m_SecondPass1)) 
				return;

			CDnAuthTask *pTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
			if( CheckCompareWord(m_SecondPass0, m_SecondPass1) )
			{
				ShuffleNumber();
				GetInterface().MessageBox( 6441 ); //HAN_MSG
			}
			else if( CheckCompareWord(m_SecondPass1, m_SecondPass2) )
			{
				pTask->RequestChangeAuthPW( m_SecondPass0.nSeed, m_SecondPass0.nValue, m_SecondPass1.nSeed, m_SecondPass1.nValue );
				Show(false);
			}
			else 
			{
				ShuffleNumber();
				GetInterface().MessageBox( 6304 ); //HAN_MSG
			}

			//Send packet to server.
		}
		else if( IsCmdControl("ID_BTN_OLD_PW") )
		{
			SetCurrentFocus(emAuthPwFocus::OldPw);
		}
		else if( IsCmdControl("ID_BTN_NEW_PW") )
		{			
			SetCurrentFocus(emAuthPwFocus::NewPw);
		}
		else if( IsCmdControl("ID_BTN_ENTER_PW") )
		{
			SetCurrentFocus(emAuthPwFocus::EnterPw);
		}

		char CheckName[256]={0,};

		if( strstr( pControl->GetControlName() , "ID_BUTTON_NUM"))
		{
			for( int i = 0 ; i < (int)m_pBtnNumPad.size() ; ++i )
			{
				if( m_pBtnNumPad[i] == pControl ) 
				{	
					if( IsCurPwFocus(emAuthPwFocus::OldPw))
					{
						SetCurrentFocus(emAuthPwFocus::OldPw); 
						if(m_SecondPass0.GetSize() < SecondAuth::Common::PWMaxLength )
						{
							m_SecondPass0.AddValue(i);
							m_pEditOldPw->SetText(m_SecondPass0.GetStr().c_str());
						}
					}
					else if( IsCurPwFocus(emAuthPwFocus::NewPw)  ) 
					{
						SetCurrentFocus(emAuthPwFocus::NewPw); 

						if(m_SecondPass1.GetSize() < SecondAuth::Common::PWMaxLength )
						{
							m_SecondPass1.AddValue(i);
							m_pEditNewPw->SetText(m_SecondPass1.GetStr().c_str());
						}

						//Word wrap
						if(m_pEditNewPw->GetTextLength() == SecondAuth::Common::PWMaxLength ) 
						{
							SetCurrentFocus(emAuthPwFocus::EnterPw);

						}
					}
					else if( IsCurPwFocus(emAuthPwFocus::EnterPw)  )
					{
						SetCurrentFocus(emAuthPwFocus::EnterPw);

						if(m_SecondPass2.GetSize() < SecondAuth::Common::PWMaxLength )
						{
							m_SecondPass2.AddValue(i);
							m_pEditEnterPw->SetText(m_SecondPass2.GetStr().c_str());
						}
					}
				}
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnSecurityNUMPADChangeDlg::KeyDelete()
{
	SetCurrentFocus(m_eAuthPwFocus); 

	if(IsCurPwFocus(emAuthPwFocus::OldPw) ) 
	{
		if( !m_SecondPass0.IsEmpty() ) {
			m_SecondPass0.RemoveValue();
			m_pEditOldPw->SetText(m_SecondPass0.GetStr().c_str());
		}
	}
	else if(IsCurPwFocus(emAuthPwFocus::NewPw) ) 
	{
		if( !m_SecondPass1.IsEmpty() ) {
			m_SecondPass1.RemoveValue();
			m_pEditNewPw->SetText(m_SecondPass1.GetStr().c_str());
		}
	}
	else if(IsCurPwFocus(emAuthPwFocus::EnterPw))
	{
		if( !m_SecondPass2.IsEmpty() ) {
			m_SecondPass2.RemoveValue();
			m_pEditEnterPw->SetText(m_SecondPass2.GetStr().c_str());
		}
	}

}

bool CDnSecurityNUMPADChangeDlg::IsCurPwFocus(emAuthPwFocus AuthPwFocus)
{  
	return 	(m_eAuthPwFocus == AuthPwFocus) ? TRUE : FALSE; 
}

void CDnSecurityNUMPADChangeDlg::SetCurrentFocus(emAuthPwFocus AuthPwFocus)
{
	m_eAuthPwFocus = AuthPwFocus; 

	switch( AuthPwFocus  ) 
	{
	case emAuthPwFocus::OldPw:
		{
			RequestFocus(m_pEditOldPw);
			break; 
		}
	case emAuthPwFocus::NewPw:
		{
			RequestFocus(m_pEditNewPw);
			break; 
		}

	case emAuthPwFocus::EnterPw:
		{	
			RequestFocus(m_pEditEnterPw);
			break; 
		}
	}

};

void CDnSecurityNUMPADChangeDlg::ShuffleNumber()
{

	std::vector<std::string> tokens;
	TokenizeA(std::string(SecondPassword::GetVersion()), tokens, ".");
	int nCount = rand()%( atoi(tokens[1].c_str())/20);
	int nSeed = 0;
	if( nCount%6 == 0 ) SuffleNumber1(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );
	else if( nCount%6 == 1 ) SuffleNumber2(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );
	else if( nCount%6 == 2 ) SuffleNumber3(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );
	else if( nCount%6 == 3 ) SuffleNumber4(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );
	else if( nCount%6 == 4 ) SuffleNumber5(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );
	else if( nCount%6 == 5 ) SuffleNumber6(m_vecNumberStr, m_pBtnNumPad, nCount, &nSeed );

	if( IsCurPwFocus(emAuthPwFocus::OldPw) )
	{
		m_SecondPass0.nSeed = nSeed;
		m_SecondPass0.Clear( false );
		m_pEditOldPw->SetText( m_SecondPass0.GetStr().c_str() );
	}
	else if( IsCurPwFocus(emAuthPwFocus::NewPw) )
	{
		m_SecondPass1.nSeed = nSeed;
		m_SecondPass1.Clear( false );
		m_pEditNewPw->SetText( m_SecondPass1.GetStr().c_str() );
	}
	else if( IsCurPwFocus(emAuthPwFocus::EnterPw) )
	{
		m_SecondPass2.nSeed = nSeed;
		m_SecondPass2.Clear( false );
		m_pEditEnterPw->SetText( m_SecondPass2.GetStr().c_str() );
	}
}

void CDnSecurityNUMPADChangeDlg::InitializeNumber()
{
	m_SecondPass0.Clear();
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_pEditOldPw->ClearTextMemory();
	m_pEditNewPw->ClearTextMemory();
	m_pEditEnterPw->ClearTextMemory();

	WCHAR wszBuffer[32]={0,};

	for(int i = 0 ; i< 10 ; ++i)
	{
		m_pBtnNumPad[i]->SetText(m_vecNumberStr[i].c_str());
	}
}
