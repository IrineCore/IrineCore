#include "StdAfx.h"
#include "DnGuildRequestDlg.h"
#include "DnGuildRequestItemDlg.h"
#include "DnGuildTask.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnGuildWarTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildRequestDlg::CDnGuildRequestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pListBoxEx(NULL)
, m_pButtonSortJob(NULL)
, m_pButtonSortLevel(NULL)
, m_pButtonSortName(NULL)
, m_pStaticWhiteText(NULL)
, m_pStaticRedText(NULL)
, m_pStaticCount(NULL)
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
{
	for( int i = 0; i < 3; ++i )
		m_bCurrentReverse[i] = false;
}

CDnGuildRequestDlg::~CDnGuildRequestDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnGuildRequestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildRequestDlg.ui" ).c_str(), bShow );
}

void CDnGuildRequestDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");

	m_pButtonSortJob = GetControl<CEtUIButton>("ID_BT_CLASS");
	m_pButtonSortLevel = GetControl<CEtUIButton>("ID_BT_LEVEL");
	m_pButtonSortName = GetControl<CEtUIButton>("ID_BUTTON0");
	m_pStaticWhiteText = GetControl<CEtUIStatic>("ID_TEXT_NOTICE1");
	m_pStaticRedText = GetControl<CEtUIStatic>("ID_TEXT_NOTICE0");
	m_pStaticCount = GetControl<CEtUIStatic>("ID_TEXT_GUILDMEMBERCOUNT");

	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");

	for( int i = 0; i < 3; ++i )
		m_bCurrentReverse[i] = false;
}

void CDnGuildRequestDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		GetGuildTask().RequestGetGuildRecruitCharacter();
		RefreshCount();
		RefreshText();
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
		m_pButtonOK->Enable( false );
		m_pButtonCancel->Enable( false );
	}

	CEtUIDialog::Show( bShow );
}

static bool CompareGuildMemberJob( TGuildRecruitCharacter s1, TGuildRecruitCharacter s2 )
{
	if( s1.nJob < s2.nJob ) return false;
	else if( s1.nJob > s2.nJob ) return true;
	return false;
}
static bool CompareGuildMemberLevel( TGuildRecruitCharacter s1, TGuildRecruitCharacter s2 )
{
	if( s1.cLevel > s2.cLevel ) return false;
	else if( s1.cLevel < s2.cLevel ) return true;
	return false;
}

static bool CompareGuildMemberName( TGuildRecruitCharacter s1, TGuildRecruitCharacter s2 )
{
	if( __wcsicmp_l( s1.wszCharacterName, s2.wszCharacterName ) > 0 ) return true;
	else if( __wcsicmp_l( s1.wszCharacterName, s2.wszCharacterName ) < 0 ) return false;
	return false;
}

void CDnGuildRequestDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		
		if (IsCmdControl("ID_OK"))
		{
			int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
			if( nSelectIndex != -1 )
			{
				CDnGuildRequestItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildRequestItemDlg>(nSelectIndex);
				if( pItem )
					GetGuildTask().RequestAcceptJoinGuild( true, pItem->GetCharacterDBID(), pItem->GetName() );
			}
			return;
		}
		else if (IsCmdControl("ID_CANCEL"))
		{
			int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
			if( nSelectIndex != -1 )
			{
				CDnGuildRequestItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildRequestItemDlg>(nSelectIndex);
				if( pItem )
					GetGuildTask().RequestAcceptJoinGuild( false, pItem->GetCharacterDBID(), pItem->GetName() );
			}
		}
		else if( IsCmdControl("ID_BT_CLASS") )
		{
			std::sort( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end(), CompareGuildMemberJob );
			if( m_bCurrentReverse[0] ) std::reverse( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end() );
			m_bCurrentReverse[0] ^= true;
			RefreshList();
		}
		else if( IsCmdControl("ID_BT_LEVEL") )
		{
			std::sort( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end(), CompareGuildMemberLevel );
			if( m_bCurrentReverse[1] ) std::reverse( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end() );
			m_bCurrentReverse[1] ^= true;
			RefreshList();
		}
		else if( IsCmdControl("ID_BUTTON0") )
		{
			std::sort( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end(), CompareGuildMemberName );
			if( m_bCurrentReverse[2] ) std::reverse( m_vecRequestCharacterInfo.begin(), m_vecRequestCharacterInfo.end() );
			m_bCurrentReverse[2] ^= true;
			RefreshList();
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( GetGuildTask().GetAuth( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()), GUILDAUTH_TYPE_INVITE ) )
		{
			m_pButtonOK->Enable( true );
			m_pButtonCancel->Enable( true );

			// Disable accept during ongoing preliminaries or finals if the guild has qualified.
			eGuildWarStepType eWarStep = CDnGuildWarTask::GetInstance().GetCurrentGuildWarEventTimeType();
			if( eWarStep >= GUILDWAR_STEP_TRIAL && eWarStep <= GUILDWAR_STEP_REWARD && GetGuildTask().IsNotRecruitMember() )
				m_pButtonOK->Enable( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildRequestDlg::OnRecvGuildRecruitCharacter( GuildRecruitSystem::SCGuildRecruitCharacterList *pPacket )
{
	m_vecRequestCharacterInfo.clear();
	for( int i = 0; i < pPacket->cCount; ++i ) {
		TGuildRecruitCharacter CharacterInfo = pPacket->GuildRecruitCharacterList[i];
		m_vecRequestCharacterInfo.push_back( CharacterInfo );
	}
	RefreshList();
}

void CDnGuildRequestDlg::OnRecvGuildRecruitAcceptResult( GuildRecruitSystem::SCGuildRecruitAccept *pPacket )
{
	for( std::vector<TGuildRecruitCharacter>::iterator it = m_vecRequestCharacterInfo.begin(); it != m_vecRequestCharacterInfo.end(); ++it ) {
		if( it->nCharacterDBID == pPacket->biAcceptCharacterDBID ) {
			int nMsgIndex = 0;
			if( pPacket->cAcceptType == GuildRecruitSystem::AcceptType::AcceptOn ) nMsgIndex = 1577;
			else if( pPacket->cAcceptType == GuildRecruitSystem::AcceptType::AcceptOff ) nMsgIndex = 1580;
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgIndex ) );
			m_vecRequestCharacterInfo.erase( it );
			RefreshList();
			RefreshCount();
			break;
		}
	}
}

void CDnGuildRequestDlg::RefreshList()
{
	m_pListBoxEx->RemoveAllItems();

	WCHAR wszLevel[32] = {0,};
	for( int i = 0; i < (int)m_vecRequestCharacterInfo.size(); ++i ) {
		TGuildRecruitCharacter *pCharacter = &m_vecRequestCharacterInfo[i];
		if( !pCharacter ) continue;
		CDnGuildRequestItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnGuildRequestItemDlg>();
		swprintf_s( wszLevel, _countof(wszLevel), L"%d", pCharacter->cLevel );
		pItemDlg->SetInfo( pCharacter->nCharacterDBID, pCharacter->nJob, DN_INTERFACE::STRING::GetJobString( pCharacter->nJob ), wszLevel, pCharacter->wszCharacterName );
	}

	m_pButtonOK->Enable( false );
	m_pButtonCancel->Enable( false );
}

void CDnGuildRequestDlg::RefreshCount()
{
	if( !CDnGuildTask::IsActive() ) return;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() ) return;

	WCHAR wszText[256] = {0,};
	swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3348 ), GetGuildTask().GetGuildMemberCount(), pGuild->wGuildSize );
	m_pStaticCount->SetText( wszText );
}

void CDnGuildRequestDlg::RefreshText()
{
	if( !CDnGuildTask::IsActive() ) return;
	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() ) return;

	m_pStaticWhiteText->Show( false );
	m_pStaticRedText->Show( false );

	// Output message while registration, preliminaries, or finals are in progress.
	WCHAR wszText[256] = {0,};
	eGuildWarStepType eWarStep = CDnGuildWarTask::GetInstance().GetCurrentGuildWarEventTimeType();
	if( eWarStep >= GUILDWAR_STEP_PREPARATION && eWarStep <= GUILDWAR_STEP_REWARD )
	{
		int nMsgIndex = 3396;
		if( GetGuildTask().IsNotRecruitMember() )
		{
			if( eWarStep == GUILDWAR_STEP_TRIAL ) nMsgIndex = 3394;
			else if( eWarStep == GUILDWAR_STEP_REWARD ) nMsgIndex = 3395;
			swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgIndex ) );
			m_pStaticRedText->SetText( wszText );
			m_pStaticRedText->Show( true );
		}
		else
		{
			swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgIndex ) );
			m_pStaticWhiteText->SetText( wszText );
			m_pStaticWhiteText->Show( true );
		}
	}
}