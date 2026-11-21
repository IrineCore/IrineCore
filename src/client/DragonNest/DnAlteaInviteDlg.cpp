#include "stdafx.h"
#include "DnAlteaInviteDlg.h"
#include "GameOption.h"
#include "DnFriendTask.h"
#include "DnGuildTask.h"
#include "DnAlteaTask.h"
#include "DnBridgeTask.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )

#define INVITE_BUTTON_DISABLE_TIME 3

CDnAlteaInviteDlg::CDnAlteaInviteDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pFriend_Tree( NULL )
, m_pGuild_Tree( NULL )
, m_pEditBox( NULL )
, m_pFriendCount_Static( NULL )
, m_pGuildCount_Static( NULL )
, m_pInviteEnable_Static( NULL )
, m_pInviteCount_Static( NULL )
, m_pNickName_Static( NULL )
, m_pNotice_Static( NULL )
, m_pInvite_Button( NULL )
, m_bInviteDisable( false )
{

}

CDnAlteaInviteDlg::~CDnAlteaInviteDlg(void)
{

}

void CDnAlteaInviteDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AlteaInviteDlg.ui" ).c_str(), bShow );
}

void CDnAlteaInviteDlg::InitialUpdate()
{
	m_pFriend_Tree = GetControl<CEtUITreeCtl>( "ID_TREE_FRIEND" );
	m_pGuild_Tree = GetControl<CEtUITreeCtl>( "ID_TREE_GUILD" );

	m_pEditBox = GetControl<CEtUIEditBox>( "ID_IMEEDITBOX_NAME" );

	m_pFriendCount_Static = GetControl<CEtUIStatic>( "ID_TEXT_FRIENDCOUNT" );
	m_pGuildCount_Static = GetControl<CEtUIStatic>( "ID_TEXT_GUILDCOUNT" );
	m_pInviteEnable_Static = GetControl<CEtUIStatic>( "ID_TEXT0" );
	m_pInviteCount_Static = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
	m_pNickName_Static = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pNotice_Static = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );

	m_pInvite_Button = GetControl<CEtUIButton>( "ID_BT_SEND" );

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSCHEDULE );
	if( NULL == pSox )
		return;

	int nHour = pSox->GetFieldFromLablePtr( 4, "_Hour" )->GetInteger();

	WCHAR wszString[512] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8313 ), nHour );
	m_pNotice_Static->SetText( wszString );
}

void CDnAlteaInviteDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		RefreshDialog();
		GetAlteaTask().RefreshInviteDlg();
	}

	CEtUIDialog::Show( bShow );
}

void CDnAlteaInviteDlg::Process( float fElapsedTime )
{
	if( false == m_bShow )
		return;

	if( true == m_bInviteDisable )
	{
		static float fDisableTime = 0.0f;

		if( INVITE_BUTTON_DISABLE_TIME <= fDisableTime )
		{
			m_bInviteDisable =  false;
			fDisableTime = 0.0f;
			m_pInvite_Button->Enable( true );
		}

		fDisableTime += fElapsedTime;
	}

	bool bText = ( 0 == m_pEditBox->GetFullTextLength() );
	m_pNickName_Static->Show( bText );
	if( m_pEditBox->IsFocusEditBox() )
		m_pInvite_Button->Enable( !bText );

	if( 0 == m_pInviteCount_Static->GetTextToInt() || true == m_bInviteDisable )
		m_pInvite_Button->Enable( false );

	CEtUIDialog::Process( fElapsedTime );
}

void CDnAlteaInviteDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BT_SEND") )
		{
			m_bInviteDisable = true;

			CTreeItem * pItem = m_pFriend_Tree->GetSelectedItem();
			if( NULL != pItem && pItem->GetItemValueInt64() != 0 )
			{
				GetAlteaTask().RequestAlteaSendTicket( pItem->GetText() );
				return;
			}

			pItem = m_pGuild_Tree->GetSelectedItem();
			if( NULL != pItem && pItem->GetItemValueInt64() != 0 )
			{
				GetAlteaTask().RequestAlteaSendTicket( pItem->GetText() );
				return;
			}

			if( !CDnActor::s_hLocalActor )
				return;
			CDnLocalPlayerActor * pLocalActor = (CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer());
			if( NULL == pLocalActor )
				return;

			if( 0 != wcscmp( pLocalActor->GetName(), m_pEditBox->GetText() ))
				GetAlteaTask().RequestAlteaSendTicket( m_pEditBox->GetText() );
			else
				GetInterface().MessageBox(8331);	// UISTRING : Cannot send an entry ticket to yourself.
		}
	}
	else if( nCommand == EVENT_TREECONTROL_SELECTION )
	{
		if( IsCmdControl("ID_TREE_FRIEND") )
		{
			m_pGuild_Tree->ResetSelectedItem();
			m_pEditBox->ClearText();
			m_pInvite_Button->Enable( false );

			CTreeItem * pItem = m_pFriend_Tree->GetSelectedItem();
			if( NULL == pItem || pItem->GetItemValueInt64() == 0 )
				return;

			m_pInvite_Button->Enable( true );
		}
		else if( IsCmdControl("ID_TREE_GUILD") )
		{
			m_pFriend_Tree->ResetSelectedItem();
			m_pEditBox->ClearText();
			m_pInvite_Button->Enable( false );

			CTreeItem * pItem = m_pGuild_Tree->GetSelectedItem();
			if( NULL == pItem || pItem->GetItemValueInt64() == 0 )
				return;

			m_pInvite_Button->Enable( true );
		}
	}
	else if( nCommand == EVENT_EDITBOX_FOCUS )
	{
		if( IsCmdControl("ID_IMEEDITBOX_NAME") )
		{
			m_pFriend_Tree->ResetSelectedItem();
			m_pGuild_Tree->ResetSelectedItem();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAlteaInviteDlg::SetSendTicketCount( const int nCount )
{
	m_pInviteCount_Static->SetIntToText( nCount );
}

void CDnAlteaInviteDlg::SetSendUser( const std::vector<INT64> vecSendUser )
{
	for( DWORD itr = 0; itr < vecSendUser.size(); ++itr )
	{
		CTreeItem * pItem = m_pFriend_Tree->FindItemInt64( vecSendUser[itr] );
		if( NULL != pItem )
			pItem->SetClassIconType( CTreeItem::eMail );

		pItem = m_pGuild_Tree->FindItemInt64( vecSendUser[itr] );
		if( NULL != pItem )
			pItem->SetClassIconType( CTreeItem::eMail );
	}
}

void CDnAlteaInviteDlg::RefreshDialog()
{
	m_pInvite_Button->Enable( false );
	m_pEditBox->ClearText();
	RefreshFriendTree();
	RefreshGuildTree();
}

void CDnAlteaInviteDlg::RefreshFriendTree()
{
	m_pFriend_Tree->DeleteAllItems();

	CDnFriendTask::FRIENDINFO_MAP *pFriendInfoList(NULL);

	CTreeItem *pItemGroup(NULL);
	CTreeItem *pItemFriend(NULL);
	WCHAR wszTitle[128] = {0,};

	pFriendInfoList = GetFriendTask().GetFriendInfoList( 0 );

	int nConnectUserCount = 0;
	if( pFriendInfoList )
	{
		CDnFriendTask::SFriendGroupCountInfo info = GetFriendTask().GetGroupCountInfo( 0 );
		swprintf_s(wszTitle, _countof(wszTitle), _T("%s (%d/%d)"), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1346 ), info.logonFriends, info.wholeFriends);

		pItemGroup = m_pFriend_Tree->AddItem( CTreeItem::typeOpen, wszTitle, textcolor::FONT_GREEN );

		CDnFriendTask::FRIENDINFO_MAP_ITER iter = pFriendInfoList->begin();
		for( ; iter != pFriendInfoList->end(); ++iter )
		{
			const CDnFriendTask::SFriendInfo& info = iter->second;
			if( info.Location.cServerLocation == CDnFriendTask::_LOCATION_NONE )
				continue;

			pItemFriend = m_pFriend_Tree->AddChildItem( pItemGroup, CTreeItem::typeOpen, iter->second.wszFriendName.c_str(), textcolor::WHITE );
			pItemFriend->SetItemValueInt64( iter->first );
			pItemFriend->SetClassIconType( CTreeItem::eNoneMail );

			++nConnectUserCount;
		}
	}

	const CDnFriendTask::FRIENDGROUPINFO_VEC &friendGroupInfo = GetFriendTask().GetFriendGroupInfoList();
	for( int i=0; i<(int)friendGroupInfo.size(); ++i )
	{
		CDnFriendTask::SFriendGroupCountInfo info = GetFriendTask().GetGroupCountInfo( friendGroupInfo[i].nGroupDBID );
		swprintf_s(wszTitle, _countof(wszTitle), _T("%s (%d/%d)"), friendGroupInfo[i].wszGroupName.c_str(), info.logonFriends, info.wholeFriends);

		pItemGroup = m_pFriend_Tree->AddItem( CTreeItem::typeOpen, wszTitle, textcolor::FONT_GREEN );
		pItemGroup->SetItemValueFloat( (float)friendGroupInfo[i].nGroupDBID );

		pFriendInfoList = GetFriendTask().GetFriendInfoList( friendGroupInfo[i].nGroupDBID );
		if( pFriendInfoList )
		{
			CDnFriendTask::FRIENDINFO_MAP_ITER iter = pFriendInfoList->begin();
			for( ; iter != pFriendInfoList->end(); ++iter )
			{
				CDnFriendTask::SFriendInfo& info = iter->second;
				if( info.Location.cServerLocation == CDnFriendTask::_LOCATION_NONE )
					continue;

				pItemFriend = m_pFriend_Tree->AddChildItem( pItemGroup, CTreeItem::typeOpen, info.wszFriendName.c_str(), textcolor::WHITE );
				pItemFriend->SetItemValueInt64( iter->first );
				pItemFriend->SetClassIconType( CTreeItem::eNoneMail);

				++nConnectUserCount;
			}
		}
	}

	SetCount( nConnectUserCount, m_pFriendCount_Static );
	m_pFriend_Tree->ExpandAll();
}

void CDnAlteaInviteDlg::RefreshGuildTree()
{
	m_pGuild_Tree->DeleteAllItems();

	const UINT nLocalAccountDBID = CDnBridgeTask::GetInstance().GetAccountDBID();

	TGuild *pGuild = GetGuildTask().GetGuildInfo();
	if( !pGuild->IsSet() )
		return;

	CTreeItem *pItemGroup(NULL);
	CTreeItem *pItemGuild(NULL);

	const int nCount = GetGuildTask().GetGuildMemberCount();

	pItemGroup = m_pGuild_Tree->AddItem( CTreeItem::typeOpen, pGuild->GuildView.wszGuildName, textcolor::FONT_GREEN );

	int nConnectUserCount = 0;
	for( int itr = 0; itr < nCount; ++itr )
	{
		TGuildMember *pMember = GetGuildTask().GetGuildMemberFromArrayIndex( itr );
		if( NULL == pMember || pMember->Location.cServerLocation == _LOCATION_NONE || nLocalAccountDBID == pMember->nAccountDBID )
			continue;

		pItemGuild = m_pGuild_Tree->AddChildItem( pItemGroup, CTreeItem::typeOpen, pMember->wszCharacterName, textcolor::WHITE );
		pItemGuild->SetItemValueInt64( pMember->nCharacterDBID );
		pItemGuild->SetClassIconType( CTreeItem::eNoneMail );

		++nConnectUserCount;
	}

	SetCount( nConnectUserCount, m_pGuildCount_Static );
	m_pGuild_Tree->ExpandAll();
}

void CDnAlteaInviteDlg::SetCount( const int nConnectUserCount, CEtUIStatic * pCount_Static )
{
	wchar_t szTemp[64] = {0};
	wchar_t szString[64] = {0,};
	swprintf_s( szTemp, _countof(szTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1572 ), nConnectUserCount );			// UISTRING : %d players
	swprintf_s( szString, _countof(szString), L"%s %s", szTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 16 ) );		// UISTRING : Online
	pCount_Static->SetText( szString );
}

#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )