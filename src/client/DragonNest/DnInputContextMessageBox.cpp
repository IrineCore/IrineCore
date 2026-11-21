#include "StdAfx.h"
#include "DnInputContextMessageBox.h"
#include "DnTableDB.h"
#include "DNTableFile.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnInputContextMessageBox::CDnInputContextMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnOK( NULL )
	, m_pBtnCancel( NULL )
	, m_pEditBox( NULL )
	, m_pStaticMain( NULL )
	, m_pStaticSub( NULL )
{
}

CDnInputContextMessageBox::~CDnInputContextMessageBox()
{
}

void CDnInputContextMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InputContextMessageBox.ui" ).c_str(), bShow );
}

void CDnInputContextMessageBox::InitialUpdate()
{
	m_pBtnOK = GetControl<CEtUIButton>( "ID_OK" );
	m_pBtnCancel = GetControl<CEtUIButton>( "ID_CANCEL" );
	m_pEditBox = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX" );
	m_pStaticMain = GetControl<CEtUIStatic>( "ID_MESSAGE_MAIN" );
	m_pStaticSub = GetControl<CEtUIStatic>( "ID_MESSAGE_SUB" );
}

void CDnInputContextMessageBox::SetPopupTableID( int nPopupTableID )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TINPUTTEXTPOPUP );
	if( !pSox || !pSox->IsExistItem( nPopupTableID ) )
		return;

	int nMsgStringID = 0;
	int nTextCodeID = 0;
	DNTableCell *pCell = pSox->GetFieldFromLablePtr( nPopupTableID, "_PopUpMessageString" );
	if( pCell ) nMsgStringID = pCell->GetInteger();
	pCell = pSox->GetFieldFromLablePtr( nPopupTableID, "_TextCode" );
	if( pCell ) nTextCodeID = pCell->GetInteger();

	const WCHAR *wszMsg = ( nMsgStringID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgStringID ) : L"";
	const WCHAR *wszCode = ( nTextCodeID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTextCodeID ) : L"";
	SetContextMessage( wszMsg, wszCode, wszCode );
}

void CDnInputContextMessageBox::SetContextMessage( const WCHAR *wszMain, const WCHAR *wszSub, const WCHAR *wszRequiredInput )
{
	m_strRequiredText = wszRequiredInput ? wszRequiredInput : L"";
	if( m_pStaticMain ) m_pStaticMain->SetText( wszMain ? wszMain : L"" );
	if( m_pStaticSub ) m_pStaticSub->SetText( wszSub ? wszSub : L"" );
	if( m_pEditBox ) m_pEditBox->ClearText();
	if( m_pBtnOK ) m_pBtnOK->Enable( m_strRequiredText.empty() );
}

bool CDnInputContextMessageBox::IsInputMatched() const
{
	if( m_strRequiredText.empty() )
		return true;
	if( !m_pEditBox )
		return false;
	return ( m_strRequiredText == m_pEditBox->GetText() );
}

void CDnInputContextMessageBox::Show( bool bShow )
{
	if( bShow && m_pEditBox )
	{
		m_pEditBox->ClearText();
		RequestFocus( m_pEditBox );
	}
	CEtUIDialog::Show( bShow );
}

void CDnInputContextMessageBox::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_EDITBOX_CHANGE || nCommand == EVENT_EDITBOX_IME_CHANGE )
	{
		if( IsCmdControl( "ID_IMEEDITBOX" ) && m_pBtnOK )
			m_pBtnOK->Enable( IsInputMatched() );
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) || IsCmdControl( "ID_CANCEL" ) )
		{
			if( m_pCallback )
				m_pCallback->OnUICallbackProc( m_nDialogID, nCommand, pControl, uMsg );
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
