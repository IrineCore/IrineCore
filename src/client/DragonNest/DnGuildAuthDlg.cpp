#include "StdAfx.h"
#include "DnGuildAuthDlg.h"
#include "DnGuildTask.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildAuthDlg::CDnGuildAuthDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
, m_pButtonOK(NULL)
{
	m_pComboGrade = NULL;
	for( int i = 0; i < GUILDAUTH_TYPE_CNT; ++i )
		m_pCheckOption[i] = NULL;
	m_pComboTakeItem = NULL;
	m_pComboWithDraw = NULL;
	m_dwPrevComboGradeIndex = 0;
	m_bShowChangeMsg = true;
}

CDnGuildAuthDlg::~CDnGuildAuthDlg(void)
{
}

void CDnGuildAuthDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildGradeOptionDlg.ui" ).c_str(), bShow );
}

void CDnGuildAuthDlg::InitialUpdate()
{
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");

	m_pComboGrade = GetControl<CEtUIComboBox>("ID_COMBOBOX0");
	char szName[32] = {0,};
	for( int i = 0; i < GUILDAUTH_TYPE_CNT; ++i )
	{
		sprintf_s( szName, _countof(szName), "ID_CHECKBOX_MASTER%d", i );
		m_pCheckOption[i] = GetControl<CEtUICheckBox>(szName);
	}
	m_pComboTakeItem = GetControl<CEtUIComboBox>("ID_COMBOBOX_ITEM");
	m_pComboWithDraw = GetControl<CEtUIComboBox>("ID_COMBOBOX_MONEY");

	InitControl();
}

void CDnGuildAuthDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_dwPrevComboGradeIndex = m_pComboGrade->GetSelectedIndex();
		RefreshAuth();
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildAuthDlg::InitControl()
{

#ifdef PRE_ADD_CHANGEGUILDROLE
	// Guild master and sub-master have all permissions.
#else
	// Both guild master and sub-master see the same UI; sub-master and above can set permissions for new members.
	m_pComboGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3734 ), NULL, GUILDROLE_TYPE_SUBMASTER );
#endif
	m_pComboGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3735 ), NULL, GUILDROLE_TYPE_SENIOR );
	m_pComboGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3736 ), NULL, GUILDROLE_TYPE_REGULAR );
	m_pComboGrade->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3737 ), NULL, GUILDROLE_TYPE_JUNIOR );

	// Populate combo box with table values (assuming sequential indices).
	WCHAR wszTemp[64] = {0,};
	for( int i = 0; i < GetGuildTask().GetMaxTakeItemIndexCount(); ++i )
	{
		int nData = GetGuildTask().GetMaxTakeItemData( i );
		if( nData == 0 )
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3816 ) );
		else
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4123 ), nData );
		m_pComboTakeItem->AddItem( wszTemp, NULL, nData );
	}
	for( int i = 0; i < GetGuildTask().GetMaxWithDrawIndexCount(); ++i )
	{
		int nData = GetGuildTask().GetMaxWithDrawData( i );
		if( nData == 0 )
			swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3816 ) );
		else
			swprintf_s( wszTemp, _countof(wszTemp), L"%d%s", nData / 10000, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 507 ) );
		m_pComboWithDraw->AddItem( wszTemp, NULL, nData );
	}
}

void CDnGuildAuthDlg::RefreshAuth()
{
	if( !CDnGuildTask::IsActive() ) return;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() ) return;

	int nCurRole = -1;
	m_pComboGrade->GetSelectedValue( nCurRole );
	if( nCurRole == -1 ) return;

	for( int i = 0 ; i < GUILDAUTH_TYPE_CNT; ++i ) {
		m_pCheckOption[i]->SetChecked( GetGuildTask().GetAuth( (eGuildRoleType)nCurRole, static_cast<eGuildAuthType>(i) ), false );
		m_pCheckOption[i]->Enable( true );
	}
	// Combo box contents come from guild task items
	m_pComboTakeItem->SetSelectedByValue( GetGuildTask().GetMaxTakeItemByRole( (eGuildRoleType)nCurRole ) );
	m_pComboWithDraw->SetSelectedByValue( GetGuildTask().GetMaxWithDrawByRole( (eGuildRoleType)nCurRole ) );
	m_pComboTakeItem->Enable( true );
	m_pComboWithDraw->Enable( true );

	switch( nCurRole )
	{
	case GUILDROLE_TYPE_SENIOR:
		{
			m_pCheckOption[0]->Enable( false );
		}
		break;
	case GUILDROLE_TYPE_REGULAR:
		{
			m_pCheckOption[0]->Enable( false );
			m_pCheckOption[1]->Enable( false );
			m_pCheckOption[2]->Enable( false );
			m_pCheckOption[3]->Enable( false );
		}
		break;
	case GUILDROLE_TYPE_JUNIOR:
		{
			for( int i = 0 ; i < GUILDAUTH_TYPE_CNT; ++i )
				m_pCheckOption[i]->Enable( false );
			m_pComboTakeItem->Enable( false );
			m_pComboWithDraw->Enable( false );
		}
		break;
	}
}

void CDnGuildAuthDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK") )
		{
			Apply();

			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100060) );

			return;
		}
		else if( IsCmdControl("ID_CANCEL") || IsCmdControl("ID_BUTTON_CLOSE") )
		{
			Show(false);
			return;
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser )
	{
		if( IsCmdControl("ID_COMBOBOX0") )
		{
			if( m_dwPrevComboGradeIndex != m_pComboGrade->GetSelectedIndex() )
			{
				if( m_bShowChangeMsg )
				{
					if( IsChanged() )
					{
						m_bShowChangeMsg = false;
						m_pComboGrade->SetSelectedByIndex( m_dwPrevComboGradeIndex );
						GetInterface().MessageBox( 3170 );
						return;
					}
				}
				else
				{
					m_bShowChangeMsg = true;
				}
				m_dwPrevComboGradeIndex = m_pComboGrade->GetSelectedIndex();
				RefreshAuth();
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildAuthDlg::Apply()
{
	if( !CDnGuildTask::IsActive() ) return;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() ) return;

	// If equal to the value when the window opened, send no request; if changed, send a change request to the server.
	int nCurRole = -1;
	m_pComboGrade->GetSelectedValue( nCurRole );
	if( nCurRole == -1 ) return;

	bool bChanged = false;
	int iInt2 = 0;	// Save permission bit flags.
	int iInt3 = 0;  // Withdrawal count
	int iInt4 = 0;  // Withdrawal amount 

	// Check for permission setting changes
	for( int i = 0; i < GUILDAUTH_TYPE_CNT; ++i ) 
	{
		if( GetGuildTask().GetAuth( (eGuildRoleType)nCurRole, static_cast<eGuildAuthType>(i) ) != m_pCheckOption[i]->IsChecked() ) 
		{
			bChanged = true;
			break;
		}
	}

	// Check withdrawal count and amount settings (please handle this)
	if( !bChanged )
	{
		int nSelectedValue = 0;
		m_pComboTakeItem->GetSelectedValue( nSelectedValue );
		if( GetGuildTask().GetMaxTakeItemByRole( (eGuildRoleType)nCurRole ) != nSelectedValue )
			bChanged = true;

		m_pComboWithDraw->GetSelectedValue( nSelectedValue );
		if( GetGuildTask().GetMaxWithDrawByRole( (eGuildRoleType)nCurRole ) != nSelectedValue )
			bChanged = true;
	}

	// Send Auth change packet if any permission checkbox, withdrawal count, or withdraw limit changed.
	if( bChanged )
	{
		for( int i = 0; i < GUILDAUTH_TYPE_CNT; ++i ) 
		{
			if( m_pCheckOption[i]->IsChecked() )
				iInt2 |= (static_cast<int>(1) << i);
		}

		// Set withdrawal count and amount
		m_pComboTakeItem->GetSelectedValue( iInt3 );
		m_pComboWithDraw->GetSelectedValue( iInt4 );

		GetGuildTask().RequestChangeGuildInfoEx( GUILDUPDATE_TYPE_ROLEAUTH, nCurRole, iInt2, iInt3, iInt4 );		
	}
}

bool CDnGuildAuthDlg::IsChanged()
{
	bool bChanged = false;
	if( !CDnGuildTask::IsActive() ) return false;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() ) return false;

	// Called when combo box item changes, so get CurRole from the previously selected item.
	int nCurRole = -1;
	m_pComboGrade->GetItemValue( m_dwPrevComboGradeIndex, nCurRole );
	if( nCurRole == -1 ) return false;

	for( int i = 0; i < GUILDAUTH_TYPE_CNT; ++i ) {
		if( GetGuildTask().GetAuth( (eGuildRoleType)nCurRole, static_cast<eGuildAuthType>(i) ) != m_pCheckOption[i]->IsChecked() ) {
			bChanged = true;
			break;
		}
	}

	int nSelectedValue = 0;
	m_pComboTakeItem->GetSelectedValue( nSelectedValue );
	if( GetGuildTask().GetMaxTakeItemByRole( (eGuildRoleType)nCurRole ) != nSelectedValue )
		bChanged = true;
	m_pComboWithDraw->GetSelectedValue( nSelectedValue );
	if( GetGuildTask().GetMaxWithDrawByRole( (eGuildRoleType)nCurRole ) != nSelectedValue )
		bChanged = true;

	return bChanged;
}
