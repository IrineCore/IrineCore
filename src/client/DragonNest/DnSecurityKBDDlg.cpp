#include "StdAfx.h"
#include "DnSecurityKBDDlg.h"
#include "DnInterface.h"
#include "DnLoginDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define MAX_KEY_NUMBER 47

CDnSecurityKBDDlg::CDnSecurityKBDDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	bStateCapslock	=false; 
	bPushShift		=false; 
	MouseLClickPress =false; 
	
	for( int i=0  ; i< (int)KeyName::emkeyMax ; ++i)
	{
		m_BtnKey[i] = NULL; 
	}
	m_pEditBox = NULL ; 
	m_pBtnKeyBodyPad.clear();
	m_wszCurStr.clear();
	m_SpecialChar.clear();
	
	InitSpecialChar();
	bClickDelSec = false;
	m_State = 0;
	m_dwNowTime = 0;
	m_fOldTime = 0.0f;
	m_fTime = 0.0f;
}

CDnSecurityKBDDlg::~CDnSecurityKBDDlg(void)
{
}

void CDnSecurityKBDDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SecurityKBD.ui" ).c_str(), bShow );
}

WCHAR CDnSecurityKBDDlg::ChangeChar(WCHAR *ch , bool bFlag /* = true */)
{
	if( bFlag )
	{
		for(int i = 0 ; i < (int)m_SpecialChar.size() ; ++i )
		{
			if( m_SpecialChar[i].LowerChar == *ch ) 
			{
				return m_SpecialChar[i].UpperChar; 
			}
		}
	}
	else 
	{
		for(int i = 0 ; i < (int)m_SpecialChar.size() ; ++i )
		{
			if( m_SpecialChar[i].UpperChar == *ch ) 
			{
				return m_SpecialChar[i].LowerChar; 
			}
		}
	}

	return 0;
}


void CDnSecurityKBDDlg::InitSpecialChar()
{
	m_SpecialChar.push_back(SSpecialChar(0x60, 0x7e));//` ~
	m_SpecialChar.push_back(SSpecialChar(0x31,	0x21));	//1 !
	m_SpecialChar.push_back(SSpecialChar(0x32,	0x40));//2 @
	m_SpecialChar.push_back(SSpecialChar(0x33,	0x23));	//3 #
	m_SpecialChar.push_back(SSpecialChar(0x34,	0x24)); //4 $
	m_SpecialChar.push_back(SSpecialChar(0x35,	0x25)); //5 %
	m_SpecialChar.push_back(SSpecialChar(0x36,	0x5e)); //6 ^
	m_SpecialChar.push_back(SSpecialChar(0x37,	0x26));	//7 &
	m_SpecialChar.push_back(SSpecialChar(0x38,	0x2a));	//8 *
	m_SpecialChar.push_back(SSpecialChar(0x39,	0x28));	//9 (
	m_SpecialChar.push_back(SSpecialChar(0x30,	0x29));	//0 )
	m_SpecialChar.push_back(SSpecialChar(0x2d,	0x5f));//- _
	m_SpecialChar.push_back(SSpecialChar(0x3d,	0x2b));	//= +
	m_SpecialChar.push_back(SSpecialChar(0x5c, 0x7c));//\ |
	m_SpecialChar.push_back(SSpecialChar(0x5b, 0x7b));//[ {
	m_SpecialChar.push_back(SSpecialChar(0x5d, 0x7d));//] }
	m_SpecialChar.push_back(SSpecialChar(0x3b,	0x3a));	//; :
	m_SpecialChar.push_back(SSpecialChar(0x27,	0x22));	//' "
	m_SpecialChar.push_back(SSpecialChar(0x2c,	0x3c));	//, <
	m_SpecialChar.push_back(SSpecialChar(0x2e,	0x3e));	//. >
	m_SpecialChar.push_back(SSpecialChar(0x2f,	0x3f));	// / ?

}

void CDnSecurityKBDDlg::InitialUpdate()
{
	//Must remember. 
	InitSpecialChar();

	char Strtmp[1024]={0,};
	m_wszCurStr.clear();

	for(int i = 0 ; i< MAX_KEY_NUMBER  ; ++i)
	{
		sprintf_s(Strtmp , 1024 , "ID_BUTTON_KEY%d" , i );
		m_pBtnKeyBodyPad.push_back(GetControl<CEtUIButton>( Strtmp));
		m_vecKeyPadStr.push_back( m_pBtnKeyBodyPad[i]->GetText() );
	}

	m_BtnKey[KeyName::emKeyShuffle] = GetControl<CEtUIButton>("ID_BUTTON_SHUFFLE") ;
	m_BtnKey[KeyName::emKeyReturn] = GetControl<CEtUIButton>("ID_BUTTON_DEFAULT") ;
	m_BtnKey[KeyName::emKeyDelete] = GetControl<CEtUIButton>("ID_BUTTON_CLEAN") ;
	m_BtnKey[KeyName::emKeyLShift] = GetControl<CEtUIButton>("ID_BUTTON_LSHIFT") ;
	m_BtnKey[KeyName::emKeyRshift] = GetControl<CEtUIButton>("ID_BUTTON_RSHIFT") ;
	m_BtnKey[KeyName::emKeyOk] = GetControl<CEtUIButton>("ID_BUTTON_OK") ;
	m_BtnKey[KeyName::emKeyClose] = GetControl<CEtUIButton>("ID_BUTTON_CLOSE") ;			
	

//	GetControl<CEtUIEditBox>("ID_EDITBOX0")->Show(false);
//	GetControl<CEtUIEditBox>("ID_EDITBOX1")->Show(false);


//	m_pEditBox = GetControl<CEtUIEditBox>("ID_EDITBOX0") ; 
//	m_pEditBox->KeyLock(false);
//	m_pEditBox->ClearTextMemory();
	
//	GetControl<CEtUIButton>("ID_BUTTON_CLEAN")->Enable( false );


}

void CDnSecurityKBDDlg::Show( bool bShow , int State )
{
	if(m_bShow ==  bShow )
		return;
	
	KeyDefault();
	m_State = State; 
	MouseLClickPress = false; 


	GetControl<CEtUIEditBox>("ID_EDITBOX0")->Show(false);
	GetControl<CEtUIEditBox>("ID_EDITBOX1")->Show(false);

	if( m_State == 0 )
	{
		m_pEditBox = GetControl<CEtUIEditBox>("ID_EDITBOX0");
	}
	else 
	{
		m_pEditBox = GetControl<CEtUIEditBox>("ID_EDITBOX1");
	}
	
	m_pEditBox->ClearTextMemory();
	m_pEditBox->Show(true);
	m_pEditBox->KeyLock(false);
	m_wszCurStr.clear();
	
	CEtUIDialog::Show(bShow);
}


bool CDnSecurityKBDDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		return true;
	}
	else if( uMsg == WM_LBUTTONDOWN )
	{
		POINT MousePoint;
		float fMouseX, fMouseY;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( m_BtnKey[KeyName::emKeyDelete]->IsInside(fMouseX ,fMouseY ) ) 
		{
			RequestFocus(m_pEditBox);
			MouseLClickPress = true; 
			m_fTime = 0.0f; 
		}
	}
	
	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

void CDnSecurityKBDDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
	if( MouseLClickPress )
	{
		m_fTime += fElapsedTime ; 
		
		if( m_fTime > DELETE_BTN_DELAY )
		{
			if( !m_wszCurStr.empty() )
			{
				//m_wszCurStr.erase(m_wszCurStr.size()-1);
				m_wszCurStr.clear();
				m_pEditBox->SetText(m_wszCurStr.c_str());
				MouseLClickPress = false;
				m_fTime = 0.0f; 

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

void CDnSecurityKBDDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( pControl == m_BtnKey[KeyName::emKeyShuffle] ) 
		{
			ShuffleNumber();
		}
		else if( pControl == m_BtnKey[KeyName::emKeyReturn] ) 
		{	
			KeyDefault();
		}
		else if( pControl == m_BtnKey[KeyName::emKeyDelete] ) 
		{
			MouseLClickPress = false; 		
		}
		else if( pControl == m_BtnKey[KeyName::emKeyLShift] 
				||pControl == m_BtnKey[KeyName::emKeyRshift] ) 
		{
			//Must be uppercase.
			if( !bPushShift )
			{
				bPushShift = true; 
				KeyUpper();
			}
			else 
			{
				bPushShift = false; 
				KeyLower();
			}
		}
		else if( pControl == m_BtnKey[KeyName::emKeyOk] ) 
		{
			if( m_State	== 0 )
			{
				GetInterface().GetLoginDlg()->SetId((WCHAR*)m_pEditBox->GetText());
			}
			else if ( m_State == 1 )
			{
				GetInterface().GetLoginDlg()->SetPw((WCHAR*)m_pEditBox->GetText());
			}
			CEtUIDialog::Show(false);
		}
		else if( pControl == m_BtnKey[KeyName::emKeyClose] ) 
		{
			CEtUIDialog::Show(false);
		}

		for(int i =0 ; i < (int)m_pBtnKeyBodyPad.size() ; ++i )
		{
			if(m_pBtnKeyBodyPad[i] == pControl )
			{
				if( m_State == 0 )
				{
					if((int)m_wszCurStr.size() < GetInterface().GetLoginDlg()->GetMaxCharID()-2)
					{
						RequestFocus(m_pEditBox);
						m_wszCurStr.append( m_pBtnKeyBodyPad[i]->GetText());
						m_pEditBox->SetText(m_wszCurStr.c_str()) ;
					}
				}
				else
				{
					if((int)m_wszCurStr.size() < GetInterface().GetLoginDlg()->GetMaxCharPW()-2)
					{
						RequestFocus(m_pEditBox);
						m_wszCurStr.append( m_pBtnKeyBodyPad[i]->GetText());
						m_pEditBox->SetText(m_wszCurStr.c_str()) ;
					}
				}
				break; 
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnSecurityKBDDlg::ShuffleNumber()
{
	std::vector<std::wstring> m_vecTmp;
	m_vecTmp.resize( m_vecKeyPadStr.size() );
	std::copy( m_vecKeyPadStr.begin(), m_vecKeyPadStr.end(), m_vecTmp.begin() );

	random_shuffle(m_vecTmp.begin(), m_vecTmp.end() );

	for( int i = 0 ; i < (int)m_vecTmp.size() ; ++i )
	{
		m_pBtnKeyBodyPad[i]->SetText(m_vecTmp[i].c_str());
	}

	m_vecTmp.clear();

}

void CDnSecurityKBDDlg::KeyDefault()
{
	bPushShift = false; 
	for( int i = 0 ; i < (int)m_vecKeyPadStr.size() ; ++i )
	{
		m_pBtnKeyBodyPad[i]->SetText(m_vecKeyPadStr[i].c_str());
	}
}

void CDnSecurityKBDDlg::KeyDelete()
{
	if( !m_wszCurStr.empty() )
	{
		RequestFocus(m_pEditBox);
		m_wszCurStr.erase(m_wszCurStr.size()-1);
		m_pEditBox->SetText(m_wszCurStr.c_str());
	}
}

void CDnSecurityKBDDlg::KeyUpper()
{
	std::wstring Str;
	std::wstring Ch;
	for( int i = 0 ; i < (int)m_pBtnKeyBodyPad.size() ; ++i )
	{	
		Str = m_pBtnKeyBodyPad[i]->GetText();
		
		ToUpperW(Str);	//Convert case for English letters only.
		Ch = ChangeChar((WCHAR*)Str.c_str() , true  );
		if(Ch[0] != 0 )	m_pBtnKeyBodyPad[i]->SetText( Ch );
		else m_pBtnKeyBodyPad[i]->SetText( Str );
	}
}

void CDnSecurityKBDDlg::KeyLower()
{
	std::wstring Str;
	std::wstring Ch;
	for(int i =0 ; i < (int)m_pBtnKeyBodyPad.size() ; ++i)
	{
		Str = m_pBtnKeyBodyPad[i]->GetText();
		ToLowerW(Str);
		Ch = ChangeChar((WCHAR*)Str.c_str() , false );
		if(Ch[0] != 0 )	m_pBtnKeyBodyPad[i]->SetText( Ch );
		else m_pBtnKeyBodyPad[i]->SetText( Str );
	}

}

