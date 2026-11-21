#include "StdAfx.h"
#include "DnSecurityNUMPADCreateDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnAuthTask.h"
#include "DnInterface.h"
#include "DnSecurityBase.h"
#include <MMSystem.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnSecurityNUMPADCreateDlg::CDnSecurityNUMPADCreateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback , true  )
{
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_vecNumberStr.clear();
	m_pEditCurPw = NULL;
	m_pEditEnterPw = NULL;
	m_eAuthPwFocus = NewPw;
}

CDnSecurityNUMPADCreateDlg::~CDnSecurityNUMPADCreateDlg(void)
{
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_vecNumberStr.clear();
}

void CDnSecurityNUMPADCreateDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SecurityNUMPAD_Create.ui" ).c_str(), bShow );
}

void CDnSecurityNUMPADCreateDlg::InitialUpdate()
{
	char Strtmp[1024]={0,};
	WCHAR wszBuffer[32]={0,};

	for(int i = 0 ; i< 10 ; ++i)
	{
		sprintf_s(Strtmp , 1024 , "ID_BUTTON_NUM%d" , i );
		m_pBtnNumPad.push_back(GetControl<CEtUIButton>( Strtmp));
		m_vecNumberStr.push_back( m_pBtnNumPad[i]->GetText() );
	}

	m_pEditCurPw = GetControl<CEtUIEditBox>("ID_EDITBOX1") ; 
	m_pEditCurPw->KeyLock(false);
	m_pEditCurPw->ClearTextMemory();

	m_pEditEnterPw = GetControl<CEtUIEditBox>("ID_EDITBOX2");
	m_pEditEnterPw->KeyLock(false);
	m_pEditEnterPw->ClearTextMemory();

	m_pBtnDelete = GetControl<CEtUIButton>("ID_BTN_DELETE");

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
	m_pBtnShuffle = GetControl<CEtUIButton>("ID_BTN_SHUFFLE");
#endif 
	ShuffleNumber();

}

void CDnSecurityNUMPADCreateDlg::Show( bool bShow )
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
		SetCurrentFocus( emAuthPwFocus::NewPw);
		ShuffleNumber();
		m_SecondPass2.nSeed = m_SecondPass1.nSeed;
	}
}

bool CDnSecurityNUMPADCreateDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		return true;
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

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

#ifdef PRE_MOD_SECURITYNUMPAD_RENEWAL
bool CDnSecurityNUMPADCreateDlg::CheckAreaSecurityNumber( float fMouseX, float fMouseY )
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

void CDnSecurityNUMPADCreateDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime ); 

	if( IsShow() ) {
		bool bCanShuffle = false;

		if( IsCurPwFocus(emAuthPwFocus::NewPw) && m_SecondPass1.IsEmpty() ) bCanShuffle = true;
		else if( IsCurPwFocus(emAuthPwFocus::EnterPw) && m_SecondPass2.IsEmpty() ) bCanShuffle = true;

		if( bCanShuffle != GetControl<CEtUIButton>("ID_BTN_SHUFFLE")->IsEnable() )
			GetControl<CEtUIButton>("ID_BTN_SHUFFLE")->Enable( bCanShuffle );
	}

	if( MouseLClickPress )
	{
		m_fTime += fElapsedTime ; 

		if( m_fTime > DELETE_BTN_DELAY )
		{
			m_fTime = 0.0f;
			MouseLClickPress = false;
			SetCurrentFocus(m_eAuthPwFocus);	

			if( IsCurPwFocus(emAuthPwFocus::NewPw) )
			{
				if( !m_SecondPass1.IsEmpty() ) {
					m_SecondPass1.Clear( false );
					m_pEditCurPw->SetText(m_SecondPass1.GetStr().c_str());
				}
			}
			else 
			{
				if( !m_SecondPass2.IsEmpty() ) {
					m_SecondPass2.Clear( false );
					m_pEditEnterPw->SetText(m_SecondPass2.GetStr().c_str());
				}
			}
		}
	}
	else 
	{
		if( m_fTime > 0.00001f)
		{
			KeyDelete();
			m_fTime = 0.0f; 
		}
	}
}
void CDnSecurityNUMPADCreateDlg::KeyDelete()
{
	if(IsCurPwFocus(emAuthPwFocus::NewPw) ) 
	{
		SetCurrentFocus(emAuthPwFocus::NewPw); 
		if( !m_SecondPass1.IsEmpty() ) {
			m_SecondPass1.RemoveValue();
			m_pEditCurPw->SetText( m_SecondPass1.GetStr().c_str() );
		}
	}
	else if(IsCurPwFocus(emAuthPwFocus::EnterPw) )
	{
		SetCurrentFocus(emAuthPwFocus::EnterPw); 
		if( !m_SecondPass2.IsEmpty() )
		{
			m_SecondPass2.RemoveValue();
			m_pEditEnterPw->SetText(m_SecondPass2.GetStr().c_str());
		}
	}
}


void CDnSecurityNUMPADCreateDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
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
		else if( IsCmdControl( "ID_BUTTON_CLOSE" )  || IsCmdControl( "ID_BUTTON_CANCEL" ) )
		{
			Show( false );
		}
		else if( IsCmdControl( "ID_BUTTON_OK" ))
		{
			if(CheckMaxCountPw(m_SecondPass1)) 
				return;

			if( CheckCompareWord( m_SecondPass1, m_SecondPass2 ) ) 
			{
				CDnAuthTask *pTask = (CDnAuthTask*)CTaskManager::GetInstance().GetTask("LoginTask");
				pTask->RequestCreateAuthPW( m_SecondPass1.nSeed, m_SecondPass1.nValue );
				Show(false);
			}
			else 
			{
				GetInterface().MessageBox( 6301 ) ;	//HAN_MSG
				ShuffleNumber();
			}
		}
		else if( IsCmdControl("ID_BUTTON_FIRST_PW") )
		{			
			SetCurrentFocus(emAuthPwFocus::NewPw);
		}
		else if( IsCmdControl("ID_BUTTON_SECOND_PW") )
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
					if( IsCurPwFocus(emAuthPwFocus::NewPw)  ) 
					{
						SetCurrentFocus(emAuthPwFocus::NewPw); 

						if(m_SecondPass1.GetSize() < SecondAuth::Common::PWMaxLength )
						{
							m_SecondPass1.AddValue(i);
							m_pEditCurPw->SetText(m_SecondPass1.GetStr().c_str());
						}

						//Word wrap
						if(m_pEditCurPw->GetTextLength() == SecondAuth::Common::PWMaxLength ) 
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

bool CDnSecurityNUMPADCreateDlg::IsCurPwFocus(emAuthPwFocus AuthPwFocus)
{  
	return 	(m_eAuthPwFocus == AuthPwFocus) ? TRUE : FALSE; 
}

void CDnSecurityNUMPADCreateDlg::SetCurrentFocus(emAuthPwFocus AuthPwFocus)
{
	m_eAuthPwFocus = AuthPwFocus; 

	switch( AuthPwFocus  ) 
	{
	case emAuthPwFocus::NewPw:
		{
			RequestFocus(m_pEditCurPw);
		}
		break; 
	case emAuthPwFocus::EnterPw:
		{	
			RequestFocus(m_pEditEnterPw);
		}
		break; 
	}

};

void CDnSecurityNUMPADCreateDlg::ShuffleNumber()
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

	m_SecondPass1.nSeed = nSeed;
	m_SecondPass1.Clear( false );
	m_pEditCurPw->SetText(m_SecondPass1.GetStr().c_str());

	m_SecondPass2.nSeed = nSeed;
	m_SecondPass2.Clear( false );
	m_pEditEnterPw->SetText(m_SecondPass2.GetStr().c_str());
	// Previously shuffled per position, but shuffling below, deleting above, and renumbering causes an error.
	// For example, in a state with one shuffle -> 1234 and seed 2345
	// After shuffling, suppose the seed became 3333; if you go up and delete all of 1234 or delete one or two, in reality it's not the previously saved seed value
	// Because input is re-entered based on the newly set seed value, the above value using the old seed becomes useless.
	// To make it perfect, we could structure each 4-digit code to have 8 seed values total, but for now just refresh all keys and seed values when shuffling.

}

void CDnSecurityNUMPADCreateDlg::InitializeNumber()
{
	m_SecondPass1.Clear();
	m_SecondPass2.Clear();
	m_pEditCurPw->ClearTextMemory();
	m_pEditEnterPw->ClearTextMemory();

	WCHAR wszBuffer[32]={0,};

	for(int i = 0 ; i< 10 ; ++i)
	{
		m_pBtnNumPad[i]->SetText(m_vecNumberStr[i].c_str());
	}
}
