#include "StdAfx.h"
#include "DnGuildRecruitDlg.h"
#include "DnGuildTask.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define MIN_PARTY_LEVEL 1
#define MAX_PARTY_LEVEL 100

CDnGuildRecruitDlg::CDnGuildRecruitDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pCheckLevel(NULL)
, m_pEditBoxMin(NULL)
, m_pEditBoxMax(NULL)
, m_pLineIMEEditBoxAD(NULL)
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
, m_bModify(false)
#ifdef PRE_ADD_GUILD_EASYSYSTEM
, m_pComboCategory(NULL)
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
, m_pCheckHomepage(NULL)
#endif
#endif
{
	for( int i = 0; i < CLASSKINDMAX; ++i )
		m_pCheckJob[i] = NULL;
}

CDnGuildRecruitDlg::~CDnGuildRecruitDlg(void)
{
}

void CDnGuildRecruitDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("GuildWantedDlg.ui").c_str(), bShow);

}

void CDnGuildRecruitDlg::InitialUpdate()
{
	char szName[32] = {0,};
	for( int i = 0; i < CLASSKINDMAX; ++i )
	{
		sprintf_s( szName, _countof(szName), "ID_CHECKBOX_CLASS%d", i );
		m_pCheckJob[i] = GetControl<CEtUICheckBox>(szName);
	}
#if !defined(PRE_ADD_ACADEMIC) || defined(PRE_REMOVE_ACADEMIC)
	GetControl<CEtUICheckBox>("ID_CHECKBOX_CLASS4")->Show( false );
	GetControl<CEtUIStatic>("ID_TEXT5")->Show( false );
#endif
#if !defined(PRE_ADD_KALI) || defined(PRE_REMOVE_KALI)
	GetControl<CEtUICheckBox>("ID_CHECKBOX_CLASS5")->Show( false );
	GetControl<CEtUIStatic>("ID_TEXT6")->Show( false );
#endif
	GetControl<CEtUICheckBox>("ID_CHECKBOX_CLASS6")->Show( false );
	GetControl<CEtUIStatic>("ID_TEXT7")->Show( false );

	m_pCheckLevel = GetControl<CEtUICheckBox>("ID_CHECKBOX_LEVEL");
	m_pEditBoxMin = GetControl<CEtUIEditBox>("ID_EDITBOX_LV_MIN");
	m_pEditBoxMax = GetControl<CEtUIEditBox>("ID_EDITBOX_LV_MAX");
	m_pLineIMEEditBoxAD = GetControl<CEtUILineIMEEditBox>("ID_LINEIMEEDITBOX_AD");
	m_pLineIMEEditBoxAD->SetMaxEditLength( GUILDRECRUITINTRODUCE+1 );
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");
	m_pButtonOK->SetDisableTime( 1.0f );
	m_pButtonCancel->SetDisableTime( 1.0f );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_pComboCategory = GetControl<CEtUIComboBox>("ID_COMBOBOX_PURPOSE");
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
	m_pCheckHomepage = GetControl<CEtUICheckBox>("ID_CHECKBOX_HOMEPAGE");
#endif
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3857 ), NULL, GuildRecruitSystem::PuposeCodeType::FriendlyCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3858 ), NULL, GuildRecruitSystem::PuposeCodeType::WarCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3859 ), NULL, GuildRecruitSystem::PuposeCodeType::NestCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3860 ), NULL, GuildRecruitSystem::PuposeCodeType::PvPCode );
#endif
}

void CDnGuildRecruitDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		// Check like this when entering the tab dialog.
		if( m_pParentDialog && m_pParentDialog->GetHWnd() )
			GetGuildTask().RequestGetGuildRecruitRegisterInfo();	// Request saved values when opened to get current settings.
		SetEnableControl();
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildRecruitDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK") )
		{
			BYTE cClassGrade[CLASSKINDMAX] = {0,};
			for( int i = 0; i < CLASSKINDMAX; ++i )
				cClassGrade[i] = (m_pCheckJob[i]->IsChecked()) ? (i+1) : 0;
			int nMinLevel = (m_pCheckLevel->IsChecked()) ? m_pEditBoxMin->GetTextToInt() : MIN_PARTY_LEVEL;
			int nMaxLevel = (m_pCheckLevel->IsChecked()) ? m_pEditBoxMax->GetTextToInt() : MAX_PARTY_LEVEL;
			if( nMinLevel == -1 ) nMinLevel = 1;
			if( nMaxLevel == -1 ) nMaxLevel = 1;
			if( nMaxLevel < nMinLevel ) nMaxLevel = nMinLevel;
			wchar_t wszTemp[GUILDRECRUITINTRODUCE+1] = {0, };
			m_pLineIMEEditBoxAD->GetTextCopy( wszTemp, GUILDRECRUITINTRODUCE+1 );
			int nLen = (int)wcslen( wszTemp );
			if( nLen > 0 && wszTemp[nLen-1] == L'\n' ) wszTemp[nLen-1] = '\0';
			std::wstring wszStr = wszTemp;
			if( DN_INTERFACE::UTIL::CheckChat( wszStr ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK );
				return;
			}
#ifdef PRE_ADD_GUILD_EASYSYSTEM
			BYTE cPurposeCode = 1;
			int nPurposeCode = 0;
			if( m_pComboCategory->GetSelectedValue( nPurposeCode ) )
				cPurposeCode = (BYTE)nPurposeCode;
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
			bool bCheckHomepage = false;
#else
			bool bCheckHomepage = m_pCheckHomepage->IsChecked();
#endif
#endif
#ifdef PRE_ADD_GUILD_EASYSYSTEM
			GetGuildTask().RequestRegisterGuildRecruit( m_bModify, cClassGrade, nMinLevel, nMaxLevel, wszTemp, cPurposeCode, bCheckHomepage );
#else
			GetGuildTask().RequestRegisterGuildRecruit( m_bModify, cClassGrade, nMinLevel, nMaxLevel, wszTemp );
#endif
			if (m_pParentDialog) m_pParentDialog->Show(false);
			return;
		}

		else if (IsCmdControl("ID_CANCEL"))
		{
			GetGuildTask().RequestCancelGuildRecruit();

			if (m_pParentDialog) m_pParentDialog->Show(false);
			return;
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl("ID_CHECKBOX_LEVEL") )
		{
			if( GetGuildTask().GetMyRole() != GUILDROLE_TYPE_MASTER )
				return;

			bool bEnableLevel = m_pCheckLevel->IsChecked();
			m_pEditBoxMin->Enable( bEnableLevel );
			m_pEditBoxMax->Enable( bEnableLevel );
			GetControl<CEtUIStatic>("ID_STATIC10")->Enable( bEnableLevel );
			GetControl<CEtUIStatic>("ID_STATIC11")->Enable( bEnableLevel );
			GetControl<CEtUIStatic>("ID_TEXT8")->Enable( bEnableLevel );
		}
		else
		{
			// Recruiting class checkboxes
			CheckRecruit();
		}
	}
	else if( nCommand == EVENT_EDITBOX_CHANGE || nCommand == EVENT_EDITBOX_IME_CHANGE )
	{
		if( IsCmdControl("ID_EDITBOX_LV_MIN") )
		{
			if( m_pEditBoxMin->GetTextToInt() > MAX_PARTY_LEVEL )
				m_pEditBoxMin->SetIntToText( MAX_PARTY_LEVEL );
		}
		else if( IsCmdControl("ID_EDITBOX_LV_MAX") )
		{
			if( m_pEditBoxMax->GetTextToInt() > MAX_PARTY_LEVEL )
				m_pEditBoxMax->SetIntToText( MAX_PARTY_LEVEL );
		}
		CheckRecruit();
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildRecruitDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		if( m_pCheckLevel->IsChecked() == false )
		{
			m_pEditBoxMin->SetIntToText( 1 );
			m_pEditBoxMax->SetIntToText( 100 );
		}
	}
}

void CDnGuildRecruitDlg::SetEnableControl()
{
	bool bEnable = false;
	if( GetGuildTask().GetMyRole() == GUILDROLE_TYPE_MASTER )
		bEnable = true;

	for( int i = 0; i < CLASSKINDMAX; ++i )
		m_pCheckJob[i]->Enable( bEnable );
	m_pCheckLevel->Enable( bEnable );
	m_pEditBoxMin->Enable( bEnable );
	m_pEditBoxMax->Enable( bEnable );
	GetControl<CEtUIStatic>("ID_STATIC10")->Enable( bEnable );
	GetControl<CEtUIStatic>("ID_STATIC11")->Enable( bEnable );
	GetControl<CEtUIStatic>("ID_TEXT8")->Enable( bEnable );
	m_pLineIMEEditBoxAD->Enable( bEnable );

	m_pButtonOK->Enable( false );
	m_pButtonCancel->Enable( false );

	// To handle smoothly 
	if( bEnable ) m_pCheckLevel->SetChecked( false );

#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_pComboCategory->Enable( bEnable );
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
	m_pCheckHomepage->Enable( bEnable );
#endif
#endif
}

void CDnGuildRecruitDlg::CheckRecruit()
{
	bool bEnable = false;
	if( GetGuildTask().GetMyRole() == GUILDROLE_TYPE_MASTER )
		bEnable = true;

	bool bCheckJob = false;
	for( int i = 0; i < CLASSKINDMAX; ++i )
	{
		if( m_pCheckJob[i]->IsChecked() )
			bCheckJob = true;
	}

	bool bGuildMemberCount = false;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( pGuild->IsSet() )
	{
		if( GetGuildTask().GetGuildMemberCount() < pGuild->wGuildSize )
			bGuildMemberCount = true;
	}

	m_pButtonOK->Enable( bEnable && bCheckJob && bGuildMemberCount );
}

void CDnGuildRecruitDlg::OnRecvGuildRecruitRegisterInfo( GuildRecruitSystem::SCGuildRecruitRegisterInfo *pPacket )
{
	if(pPacket)
	{
		bool bEnable = false;
		if( GetGuildTask().GetMyRole() == GUILDROLE_TYPE_MASTER )
			bEnable = true;

		for( int i = 0; i < CLASSKINDMAX; ++i )
			m_pCheckJob[i]->SetChecked( false ); 

		m_bModify = false;
		for( int i = 0; i < CLASSKINDMAX; ++i )
		{
			int nValue = pPacket->cClassGrade[i];
			if( nValue > 0 && nValue <= CLASSKINDMAX )
			{
				m_pCheckJob[nValue-1]->SetChecked( true );
				m_bModify = true;
			}
		}

		m_pEditBoxMin->SetIntToText( pPacket->nMinLevel );
		m_pEditBoxMax->SetIntToText( pPacket->nMaxLevel );
		bool bCheckLevel = !( pPacket->nMinLevel == MIN_PARTY_LEVEL && pPacket->nMaxLevel == MAX_PARTY_LEVEL );
		m_pCheckLevel->SetChecked( bCheckLevel );
		m_pLineIMEEditBoxAD->ClearText();
		m_pLineIMEEditBoxAD->SetTextBuffer( pPacket->wszGuildIntroduction );

		m_pButtonCancel->Enable( bEnable && m_bModify );

#ifdef PRE_ADD_GUILD_EASYSYSTEM
		m_pComboCategory->SetSelectedByValue( pPacket->cPurposeCode );
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
		m_pCheckHomepage->SetChecked( pPacket->bCheckHomePage );
#endif
#endif
	}
}