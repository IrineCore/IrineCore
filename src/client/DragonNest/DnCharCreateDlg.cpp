#include "StdAfx.h"
#include "DnCharCreateDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnMessageBox.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "MAPartsBody.h"
#include "DnInterfaceDlgID.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DNCountryUnicodeSet.h"



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCharCreateDlg::CDnCharCreateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pEditBoxName( NULL )
, m_pButtonFacePrior( NULL )
, m_pButtonFaceNext( NULL )
, m_pButtonTurn( NULL )
, m_pButtonCreate( NULL )
, m_bRequestWaitCreate( false )
, m_bCallbackProcessed( false )
#ifdef PRE_MOD_SELECT_CHAR
, m_pComboBoxServerList( NULL )
#endif // PRE_MOD_SELECT_CHAR
{
	memset( m_pButtonHairColor, 0, sizeof( m_pButtonHairColor ) );
	memset( m_pButtonSkinColor, 0, sizeof( m_pButtonSkinColor ) );
	memset( m_pButtonEyeColor, 0, sizeof( m_pButtonEyeColor ) );
#ifdef PRE_MOD_SELECT_CHAR
	memset( m_pCostumeRadioButton, 0, sizeof( m_pCostumeRadioButton ) );
#endif // PRE_MOD_SELECT_CHAR
}

CDnCharCreateDlg::~CDnCharCreateDlg(void)
{
}

void CDnCharCreateDlg::Initialize( bool bShow )
{
#ifdef PRE_MOD_SELECT_CHAR
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharCreate_CharOptionDlg.ui" ).c_str(), bShow );
#else // PRE_MOD_SELECT_CHAR
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharCreate.ui" ).c_str(), bShow );
#endif // PRE_MOD_SELECT_CHAR
}

void CDnCharCreateDlg::InitialUpdate()
{
	m_pEditBoxName = GetControl<CEtUIIMEEditBox>( "ID_CHARNAME" );
	m_pButtonFacePrior = GetControl<CEtUIButton>( "ID_FACE_PRIOR" );
	m_pButtonFaceNext = GetControl<CEtUIButton>( "ID_FACE_NEXT" );
	m_pButtonTurn = GetControl<CEtUIButton>( "ID_CHARTURN" );
	m_pButtonCreate = GetControl<CEtUIButton>( "ID_BUTTON_CREATE" );

	for( int i = 0; i < 4; i++)
	{
		m_pButtonHairColor[i] = GetControl<CDnColorButton>( FormatA("ID_HAIR_COLOR_%c", 'A'+i).c_str() );
		m_pButtonEyeColor[i] = GetControl<CDnColorButton>( FormatA("ID_EYE_COLOR_%c", 'A'+i).c_str() );
	}
	for( int i = 0; i < 4; i++)
	{
		m_pButtonSkinColor[i] = GetControl<CDnColorButton>( FormatA("ID_SKIN_COLOR_%c", 'A'+i).c_str() );
	}
	m_bCallbackProcessed = false;

	CGlobalInfo::GetInstance().m_nClientCharNameLenMax = m_pEditBoxName->GetMaxChar() - 2;
#ifdef PRE_MOD_SELECT_CHAR
	m_pComboBoxServerList = GetControl<CEtUIComboBox>( "ID_SERVER_LIST" );

	char szControlName[32];
	for( int i=0; i<MAX_COSTUME; ++i )
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_RBT_COSTUME%d", i );
		m_pCostumeRadioButton[i] = GetControl<CEtUIRadioButton>( szControlName );
	}
#endif // PRE_MOD_SELECT_CHAR
}

void CDnCharCreateDlg::SetClassHairColor( int nClass )
{
	DNTableFileFormat* pDefaultSox = GetDNTable( CDnTableDB::TDEFAULTCREATE );

	int nRed=0,nGreen=0,nBlue=0;
	for( int i = 0; i < 4; i++)
	{
		nRed = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_HairButtonColor%dR", i+1).c_str() )->GetInteger();
		nGreen = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_HairButtonColor%dG", i+1).c_str() )->GetInteger();
		nBlue = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_HairButtonColor%dB", i+1).c_str() )->GetInteger();
		m_pButtonHairColor[i]->GetElement(0)->TextureColor.dwCurrentColor =  D3DCOLOR_XRGB(nRed, nGreen, nBlue );
		
		for( int j = 0; j < UI_STATE_COUNT; j++)
			m_pButtonHairColor[i]->GetElement(0)->TextureColor.dwColor[ j ] = D3DCOLOR_XRGB(nRed, nGreen, nBlue );

		nRed = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_EyeButtonColor%dR", i+1).c_str() )->GetInteger();
		nGreen = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_EyeButtonColor%dG", i+1).c_str() )->GetInteger();
		nBlue = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_EyeButtonColor%dB", i+1).c_str() )->GetInteger();
		m_pButtonEyeColor[i]->GetElement(0)->TextureColor.dwCurrentColor =  D3DCOLOR_XRGB(nRed, nGreen, nBlue );
	
		for( int j = 0; j < UI_STATE_COUNT; j++)
			m_pButtonEyeColor[i]->GetElement(0)->TextureColor.dwColor[ j ] = D3DCOLOR_XRGB(nRed, nGreen, nBlue );
	}
	
	for( int i = 0; i < 4; i++ )
	{
		nRed = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_SkinButtonColor%dR", i+1).c_str() )->GetInteger();
		nGreen = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_SkinButtonColor%dG", i+1).c_str() )->GetInteger();
		nBlue = pDefaultSox->GetFieldFromLablePtr( nClass, FormatA("_SkinButtonColor%dB", i+1).c_str() )->GetInteger();
		m_pButtonSkinColor[i]->GetElement(0)->TextureColor.dwCurrentColor =  D3DCOLOR_XRGB(nRed, nGreen, nBlue );

		for( int j = 0; j < UI_STATE_COUNT; j++)
			m_pButtonSkinColor[i]->GetElement(0)->TextureColor.dwColor[ j ] = D3DCOLOR_XRGB(nRed, nGreen, nBlue );
	}

}

void CDnCharCreateDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) return;

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_FACE_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Face, false );	return;}
		if( IsCmdControl( "ID_FACE_NEXT" ) )	{pTask->RotateCreateParts( CDnParts::Face, true );	return;}
		if( IsCmdControl( "ID_HAIR_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Hair, false );	return;}
		if( IsCmdControl( "ID_HAIR_NEXT" ) )	{pTask->RotateCreateParts( CDnParts::Hair, true );	return;}
		if( IsCmdControl( "ID_BODY_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Body, false );	return;}
		if( IsCmdControl( "ID_BODY_NEXT" ) )	{pTask->RotateCreateParts( CDnParts::Body, true );	return;}
		if( IsCmdControl( "ID_LEG_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Leg, false );	return;}
		if( IsCmdControl( "ID_LEG_NEXT" ) )		{pTask->RotateCreateParts( CDnParts::Leg, true );	return;}
		if( IsCmdControl( "ID_HAND_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Hand, false );	return;}
		if( IsCmdControl( "ID_HAND_NEXT" ) )	{pTask->RotateCreateParts( CDnParts::Hand, true );	return;}
		if( IsCmdControl( "ID_BOOTS_PRIOR" ) )	{pTask->RotateCreateParts( CDnParts::Foot, false );	return;}
		if( IsCmdControl( "ID_BOOTS_NEXT" ) )	{pTask->RotateCreateParts( CDnParts::Foot, true );	return;}

		for( int i = 0; i < 4; i++)
		{
			if( IsCmdControl( FormatA( "ID_HAIR_COLOR_%c", 'A' + i ).c_str()) ) { pTask->SetCharColor( MAPartsBody::HairColor, i ); return; }
			if( IsCmdControl( FormatA( "ID_SKIN_COLOR_%c", 'A' + i ).c_str()) ) { pTask->SetCharColor( MAPartsBody::SkinColor, i ); return; }
			if( IsCmdControl( FormatA( "ID_EYE_COLOR_%c", 'A' + i ).c_str()) ) { pTask->SetCharColor( MAPartsBody::EyeColor, i ); return; }
		}
#ifndef PRE_MOD_SELECT_CHAR
		if( IsCmdControl( "ID_CHARTURN" )) { pTask->TurnCharacter(); return; }
#endif // PRE_MOD_SELECT_CHAR
		if( IsCmdControl( "ID_BUTTON_BACK" ) ) 
		{
				pTask->ChangeState( CDnLoginTask::CharCreate_SelectClass );
			return;
		}

		if( IsCmdControl( "ID_BUTTON_CREATE" ) )
		{
			tstring szName = m_pEditBoxName->GetText();

			if( szName.empty() ) 
			{
				GetInterface().MessageBox( MESSAGEBOX_13, MB_OK, 0, this );
				return;
			}

			if( szName.size() < CHARNAMEMIN )
			{
				WCHAR wszTemp[80];
				swprintf_s( wszTemp, 80, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100303 ), CHARNAMEMIN );
				GetInterface().MessageBox( wszTemp, MB_OK, 0, this );
				return;
			}

			DWORD dwCheckType = ALLOW_STRING_DEFAULT;
			if (g_CountryUnicodeSet.Check(szName.c_str(), dwCheckType) == false)
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 83 ), MB_OK, 0, this );
				return;
			}

			if( DN_INTERFACE::UTIL::CheckAccount( szName ) || DN_INTERFACE::UTIL::CheckChat( szName ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 84 ), MB_OK, 0, this );
				return;
			}
#ifdef PRE_MOD_SELECT_CHAR
			int nValue = -1;
			m_pComboBoxServerList->GetSelectedValue( nValue );
			pTask->CreateCharacter( (TCHAR*)szName.c_str(), nValue );
#else // PRE_MOD_SELECT_CHAR
			pTask->CreateCharacter( (TCHAR*)szName.c_str() );
#endif // PRE_MOD_SELECT_CHAR
			m_bRequestWaitCreate = true;
			return;
		}
	}
	else if( nCommand == EVENT_EDITBOX_STRING )	// Temporary enter handling.
	{
		if( m_bCallbackProcessed )
		{
			// Description is in LoginDlg.
			m_bCallbackProcessed = false;
			return;
		}
		else
		{
			// Must clear focus from the edit box so hotkeys work on the invalid ID/password message box.
			focus::ReleaseControl();
		}
	}
#ifdef PRE_MOD_SELECT_CHAR
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( strstr( pControl->GetControlName(), "ID_RBT_COSTUME" ) )
		{
			int nSelectIndex = static_cast<CEtUIRadioButton*>( pControl )->GetTabID();
			pTask->ShowPartsList( nSelectIndex );
		}
	}
#endif // PRE_MOD_SELECT_CHAR

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnCharCreateDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_pEditBoxName->ClearText();
		RequestFocus(m_pEditBoxName);
		m_bRequestWaitCreate = false;
	}

#ifdef PRE_MOD_SELECT_CHAR
	GetInterface().ShowCharRotateDlg( bShow );
	GetInterface().ShowCharGestureDlg( bShow, true );

	if( bShow )
	{
		if( m_pComboBoxServerList->GetItemCount() > 0 )
			m_pComboBoxServerList->RemoveAllItems();
		std::map<int, std::wstring>::iterator iter = CGlobalInfo::GetInstance().m_mapServerList.begin();
		int i=0;
		for( ; iter!= CGlobalInfo::GetInstance().m_mapServerList.end(); iter++, i++ )
		{
			std::wstring strServerName = iter->second;
			int nServerIndex = iter->first;
			m_pComboBoxServerList->AddItem( strServerName.c_str(), NULL, nServerIndex );
		}

		m_pComboBoxServerList->SetSelectedByValue( CGlobalInfo::GetInstance().m_nSelectedServerIndex );
		for( int i=0; i<MAX_COSTUME; ++i )
		{
			if( m_pCostumeRadioButton[i] )
				m_pCostumeRadioButton[i]->SetChecked( false );
		}
	}
#endif // PRE_MOD_SELECT_CHAR
}

void CDnCharCreateDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			m_pEditBoxName->ClearText();
			m_bRequestWaitCreate = false;
			RequestFocus(m_pEditBoxName);
		}
	}
}

void CDnCharCreateDlg::EnableCharCreateBackDlgControl( bool bEnable )
{
	bool bResultEnable = ( !m_bRequestWaitCreate ) ? bEnable : false;
	GetControl( "ID_CHARNAME" )->Enable( bResultEnable );
	GetControl( "ID_BUTTON_CREATE" )->Enable( bResultEnable );
	GetControl( "ID_BUTTON_BACK" )->Enable( bResultEnable );
#ifndef PRE_MOD_SELECT_CHAR
	GetControl( "ID_CHARTURN" )->Enable( bResultEnable );
#endif // PRE_MOD_SELECT_CHAR
}

#ifdef PRE_MOD_CREATE_CHAR

void CDnCharCreateDlg::SetCharCreatePartsName( CDnParts::PartsTypeEnum PartsIndex, std::wstring strPartsName )
{
	switch( PartsIndex )
	{
	case CDnParts::Hair:
		GetControl( "ID_STATIC_HEAD" )->SetText( strPartsName );
		break;
	case CDnParts::Face:
		GetControl( "ID_STATIC_FACE" )->SetText( strPartsName );
		break;
	case CDnParts::Body:
		GetControl( "ID_STATIC_BODY" )->SetText( strPartsName );
		break;
	case CDnParts::Leg:
		GetControl( "ID_STATIC_LEG" )->SetText( strPartsName );
		break;
	case CDnParts::Hand:
		GetControl( "ID_STATIC_GLOVE" )->SetText( strPartsName );
		break;
	case CDnParts::Foot:
		GetControl( "ID_STATIC_BOOTS" )->SetText( strPartsName );
		break;
	}
}

#endif // PRE_MOD_CREATE_CHAR
