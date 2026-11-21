#include "StdAfx.h"
#include "DnGameKeySetListDlg.h"
#include "DnGameKeySetDlg.h"
#include "DnGameControlTask.h"
#include "InputWrapper.h"
#include "DnLocalPlayerActor.h"

#include <mmsystem.h>

#define MESSAGE_CHANGE_KEY 100
#define MESSAGE_CHANGE_PAD 101

CDnGameKeySetListDlg::CDnGameKeySetListDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_pGameKeySetDlg( NULL )
, m_pButtonDummy( NULL )
, m_pStaticKeyBox( NULL )
, m_pStaticDisableText( NULL )
, m_pStaticEnableText( NULL )
, m_pCheckBox( NULL )
, m_bEnable( false )
, m_bPressKey( false )
, m_cKey( 0 )
, m_PressTime( 0 )
{

}

CDnGameKeySetListDlg::~CDnGameKeySetListDlg()
{

}

void CDnGameKeySetListDlg::Initialize(bool bShow)
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "GameKeySetListDlg.ui" ).c_str(), bShow );
}

void CDnGameKeySetListDlg::InitialUpdate()
{
	m_pButtonDummy = GetControl<CEtUIButton>( "ID_KEYSET" );
	m_pStaticKeyBox = GetControl<CEtUIStatic>( "ID_KEYBOX" );
	m_pStaticDisableText = GetControl<CEtUIStatic>( "ID_TEXT_GRAY" );
	m_pStaticEnableText = GetControl<CEtUIStatic>( "ID_TEXT_WHITE" );
	m_pCheckBox = GetControl<CEtUICheckBox>( "ID_CHECKBOX" );

	m_pCheckBox->Show( false );
}

void CDnGameKeySetListDlg::InitCustomControl(CEtUIControl *pControl)
{

}

void CDnGameKeySetListDlg::Show(bool bShow)
{
	if( m_bShow == bShow )
		return;

	CDnCustomDlg::Show(bShow);
}

void CDnGameKeySetListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CDnCustomDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnGameKeySetListDlg::SetData( CDnGameControlTask::SKeyData * pData, CDnGameKeySetDlg * pGameKeySetDlg )
{
	if( NULL == pData )
		return;

	m_Data = *pData;
	m_pGameKeySetDlg = pGameKeySetDlg;

	m_pStaticEnableText->Show( m_Data.bConversion );
	m_pStaticDisableText->Show( !m_Data.bConversion );
	m_pStaticEnableText->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_Data.nLowCategoryStringID ) );
	m_pStaticDisableText->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_Data.nLowCategoryStringID ) );

	m_pStaticKeyBox->SetTextColor( m_Data.bConversion == true ? textcolor::WHITE : textcolor::DARKGRAY );

	SetKeyString();
}

void CDnGameKeySetListDlg::SetKeyString()
{
	WCHAR wszString[256] = {0,};

	std::pair<int, int> nWrappingData; 

	if( CDnGameControlTask::eControlType_Keyboard == m_Data.nControlType || CDnGameControlTask::eControlType_CustomPad == m_Data.nControlType )
		nWrappingData = GetGameControlTask().GetWrappingData( m_Data.nControlType, m_Data.nLowCategorySection, m_Data.nLowCategoryType );
	else
		nWrappingData = std::pair<int, int>( m_Data.nDefaultKey_first, m_Data.nDefaultKey_second );

	if( CDnGameControlTask::eControlType_Keyboard == m_Data.nControlType )
	{
		if( 0 != nWrappingData.first && 0 == nWrappingData.second )
			swprintf_s( wszString, _countof(wszString), L"%s", g_szKeyString[nWrappingData.first] );
		else if( 0 != nWrappingData.first && 0 != nWrappingData.second )
			swprintf_s( wszString, _countof(wszString), L"%s + %s", g_szKeyString[nWrappingData.first], g_szKeyString[nWrappingData.second] );
	}
	else
	{
		if( CInputJoyPad::NULL_VALUE == nWrappingData.first && CInputJoyPad::NULL_VALUE == nWrappingData.second )
			swprintf_s( wszString, _countof(wszString), L"" );
		else if( CInputJoyPad::NULL_VALUE != nWrappingData.first && CInputJoyPad::NULL_VALUE == nWrappingData.second )
			swprintf_s( wszString, _countof(wszString), L"%s", g_szJoypadString[nWrappingData.first] );
		else
			swprintf_s( wszString, _countof(wszString), L"%s + %s", g_szJoypadString[nWrappingData.first], g_szJoypadString[nWrappingData.second] );
	}

	m_pStaticKeyBox->SetText( wszString );
}

bool CDnGameKeySetListDlg::EnableKeyListData()
{
	if( false == m_Data.bConversion )
		return false;

	m_bEnable = true;
	m_pStaticKeyBox->SetBlink( true );

	m_pGameKeySetDlg->SetNotice( CDnGameKeySetDlg::eNotice_Enable );

	return true;
}

void CDnGameKeySetListDlg::DisableKeyListData( const bool bNotice )
{
	m_bEnable = false;
	m_pStaticKeyBox->SetBlink( false );

	if( true == bNotice )
		m_pGameKeySetDlg->SetNotice( CDnGameKeySetDlg::eNotice_Disable );
}

void CDnGameKeySetListDlg::ProcessPad()
{
	if( false == m_bEnable 
		|| CDnGameControlTask::eControlType_CustomPad != m_Data.nControlType 
		|| !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor * localActor = ((CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer()));
	if( NULL == localActor )
		return;

	if( false == m_bPressKey )	//If there is no input value
	{
		if( localActor->GetEventJoypadCount() > 0 )	
		{
			m_cKey = localActor->GetEventJoypad(0);
			m_bPressKey = true;
			m_PressTime = timeGetTime();
		}
	}
	else
	{
		BYTE cKeySecond = CInputJoyPad::NULL_VALUE;
		std::pair<BYTE, BYTE> ckeys = std::make_pair( m_cKey, cKeySecond );
		for( DWORD itr = 0; itr < localActor->GetEventJoypadCount(); ++itr )
		{
			if( m_cKey != localActor->GetEventJoypad( itr ) )
				cKeySecond = localActor->GetEventJoypad( itr );
		}

		if( localActor->IsPushJoypadButton( ckeys ) )	// When held down continuously
		{
			if( cKeySecond != CInputJoyPad::NULL_VALUE )
				RequestChangePad( m_cKey, cKeySecond );
			else
			{
				WCHAR wszTitle[32] = {0,};
				wsprintf( wszTitle, L"%s", IW_PAD_STRING(m_cKey) );
				m_pStaticKeyBox->SetText( wszTitle );
			}
		}
		else	// Can register immediately.
			RequestChangePad( m_cKey, CInputJoyPad::NULL_VALUE );
	}
}

void CDnGameKeySetListDlg::RequestChangePad( const BYTE cFirstKey, const BYTE cSecondKey )
{
	m_bPressKey = false;

	std::pair<int, int> nWrappingData = GetGameControlTask().GetWrappingData( m_Data.nControlType, m_Data.nLowCategorySection, m_Data.nLowCategoryType );

	if( nWrappingData.first == cFirstKey && nWrappingData.second == cSecondKey )
	{
		SetKeySuccess();
		return;
	}

	std::wstring wszControlNameString;

	CDnGameControlTask::SChangeValue sChangeValue;
	sChangeValue.nLowCategorySection = m_Data.nLowCategorySection;
	sChangeValue.nLowCategoryType = m_Data.nLowCategoryType;
	sChangeValue.cFirstKey = cFirstKey;
	sChangeValue.cSecondKey = cSecondKey;

	if( true == GetGameControlTask().RequestChangePad( sChangeValue, wszControlNameString ) )
		SetKeySuccess();
	else
	{
		WCHAR wszMessageString[256] = {0,};
		WCHAR wszKeyName[128] = {0,};

		if( CInputJoyPad::NULL_VALUE == cSecondKey )
			swprintf_s( wszKeyName, _countof(wszKeyName), L"%s", g_szJoypadString[cFirstKey] );
		else
			swprintf_s( wszKeyName, _countof(wszKeyName), L"%s + %s", g_szJoypadString[cFirstKey], g_szJoypadString[cSecondKey] );

		swprintf_s( wszMessageString, _countof(wszMessageString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120 )
			, wszControlNameString.c_str()
			, wszKeyName );	// UISTRING : Key %s is already used by %s. Do you want to reassign it?

		GetInterface().MessageBox( wszMessageString, MB_YESNO, MESSAGE_CHANGE_PAD, this );
	}

	m_pGameKeySetDlg->ResetSelectListItem();
}

void CDnGameKeySetListDlg::SetKeySuccess()
{
	m_pGameKeySetDlg->SetNotice( CDnGameKeySetDlg::eNotice_Success );
	m_pGameKeySetDlg->RefreshListData();

	DisableKeyListData( false );
	m_pGameKeySetDlg->ResetSelectListItem( false );
}

void CDnGameKeySetListDlg::Process( float fElapsedTime )
{
	ProcessPad();

	CDnCustomDlg::Process( fElapsedTime );
}

bool CDnGameKeySetListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam  )
{
	if( false == IsShow() 
		|| false == m_bEnable 
		|| CDnGameControlTask::eControlType_Keyboard != m_Data.nControlType )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		bool bTransNumLock = ( !(GetKeyState(VK_NUMLOCK)&0x01) && !(lParam & 0x01000000) ) ? true : false;
		BYTE cDik = _GetCurrentPushKey( bTransNumLock );

		std::pair<int, int> nWrappingData = GetGameControlTask().GetWrappingData( m_Data.nControlType, m_Data.nLowCategorySection, m_Data.nLowCategoryType );

		if( nWrappingData.first == cDik )
			SetKeySuccess();
		else if( 0 == cDik )
			m_pGameKeySetDlg->SetNotice( CDnGameKeySetDlg::eNotice_Fail );
		else
		{
			std::wstring wszControlNameString;
			CDnGameControlTask::SChangeValue sChangeValue;
			sChangeValue.nLowCategorySection = m_Data.nLowCategorySection;
			sChangeValue.nLowCategoryType = m_Data.nLowCategoryType;
			sChangeValue.cFirstKey = cDik;

			if( true == GetGameControlTask().RequestChangeKey( sChangeValue, wszControlNameString ) )
				SetKeySuccess();
			else
			{
				WCHAR wszString[256] = {0,};
				swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120 )
					, wszControlNameString.c_str()
					, g_szKeyString[cDik] );	// UISTRING : Key %s is already used by %s. Do you want to reassign it?

				GetInterface().MessageBox( wszString, MB_YESNO, MESSAGE_CHANGE_KEY, this );
			}
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnGameKeySetListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) 
	{
	case MESSAGE_CHANGE_KEY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_YES" ) == 0 ) 
				{
					GetGameControlTask().ChangeKey();
					SetKeySuccess();
				}
			}
		}
		break;

	case MESSAGE_CHANGE_PAD:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_YES" ) == 0 ) 
				{
					GetGameControlTask().ChangePad();
					SetKeySuccess();
				}
				else
					SetKeyString();
			}
		}
		break;
	}
}