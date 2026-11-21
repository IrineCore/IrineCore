#include "StdAfx.h"
#include "DnChatRoomPopupDlg.h"
#include "DnInterface.h"
#include "DnFriendTask.h"
#include "TaskManager.h"
#include "DnChatRoomTask.h"
#include "DnGuildTask.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnPartyKickDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChatRoomPopupDlg::CDnChatRoomPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pButtonFriend(NULL)
, m_pButtonPrivate(NULL)
, m_pButtonGuild(NULL)
, m_pButtonExile(NULL)
, m_pButtonNameCopy(NULL)
, m_pKickDlg(NULL)
, m_bMaster( false )
{
}

CDnChatRoomPopupDlg::~CDnChatRoomPopupDlg(void)
{
	SAFE_DELETE(m_pKickDlg);
}

void CDnChatRoomPopupDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChatPopup.ui" ).c_str(), bShow );
}

void CDnChatRoomPopupDlg::InitialUpdate()
{
	m_pButtonFriend = GetControl<CEtUIButton>("ID_BUTTON_FRIEND");
	m_pButtonPrivate = GetControl<CEtUIButton>("ID_BUTTON_TRADE");
	m_pButtonGuild = GetControl<CEtUIButton>("ID_BUTTON_GUILD");
	m_pButtonExile = GetControl<CEtUIButton>("ID_BUTTON_EXILE");
	m_pButtonNameCopy = GetControl<CEtUIButton>("ID_BUTTON_SAVE");

	m_pKickDlg = new CDnPartyKickDlg( UI_TYPE_MODAL, NULL, KICK_DIALOG, this );
	m_pKickDlg->Initialize( false );

	// Could inherit and override InitialUpdate, but this is easier.
	m_pKickDlg->GetControl( "ID_STATIC_TITLE" )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8135 ) );
	m_pKickDlg->GetControl( "ID_STATIC_TEXT" )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8136 ) );
}

void CDnChatRoomPopupDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		InitControl();
	}
	else
	{
		m_pButtonFriend->Enable( false );
		m_pButtonPrivate->Enable( false );
		m_pButtonGuild->Enable( false );
		m_pButtonExile->Enable( false );
		m_pButtonNameCopy->Enable( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnChatRoomPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_FRIEND") )
		{
			wchar_t wszQuery[255];
			swprintf_s(wszQuery, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1418 ), m_wszName.c_str() );	// UISTRING : Add %s to your friend list?
			GetInterface().MessageBox( wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this );
		}
		else if( IsCmdControl("ID_BUTTON_TRADE") )
		{
			DnActorHandle hActor = CDnActor::FindActorFromName( (TCHAR *)m_wszName.c_str() );
			if( hActor )
				GetTradeTask().GetTradePrivateMarket().RequestPrivateMarket( hActor->GetUniqueID() );
		}
		else if( IsCmdControl("ID_BUTTON_GUILD") )
		{
			GetGuildTask().RequestInviteGuildMember( m_wszName.c_str() );
		}
		else if( IsCmdControl("ID_BUTTON_EXILE") )
		{
			m_pKickDlg->Show( true );
		}
		else if( IsCmdControl("ID_BUTTON_SAVE") )
		{
			CEtUILineEditBox::CopyStringToClipboard( m_wszName.c_str() );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1422 ), false );	// UISTRING : Saved the character's name. You can now paste it.
		}

		Show(false);
		return;
	}
}

void CDnChatRoomPopupDlg::SetInfo( bool bMaster, LPCWSTR pwszName )
{
	m_bMaster = bMaster;
	m_wszName = pwszName;
}

void CDnChatRoomPopupDlg::InitControl()
{
	if( !CDnActor::s_hLocalActor ) return;

	if( __wcsicmp_l( m_wszName.c_str(), CDnActor::s_hLocalActor->GetName() ) != 0 ) {
		m_pButtonPrivate->Enable( true );
		m_pButtonFriend->Enable( true );
		m_pButtonGuild->Enable( true );
		m_pButtonNameCopy->Enable( true );
	}

	// Unlike direct menu, option check validation is not performed here.
	// Assume entering the room itself means intending to trade or join a guild.

	// If self is requesting a trade with someone, or someone is requesting a trade with self,
	if( GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting() )
		m_pButtonPrivate->Enable( false );

	// Need to double-check friend add; return false if already added.
	if( !CheckFriendCondition(m_wszName.c_str()) )
		m_pButtonFriend->Enable( false );

	// Whether guild invitation is possible.
	if( m_pButtonGuild->IsEnable() ) {
		if( GetGuildTask().GetGuildInfo()->IsSet() &&
			GetGuildTask().GetAuth( static_cast<eGuildRoleType>(GetGuildTask().GetMyRole()), GUILDAUTH_TYPE_INVITE ) &&
			CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) {
				m_pButtonGuild->Enable( true );
		}
		else {
			m_pButtonGuild->Enable( false );
		}
	}

	// Only the room master can kick.
	if( m_bMaster ) {
		if( __wcsicmp_l( m_wszName.c_str(), CDnActor::s_hLocalActor->GetName() ) != 0 )
			m_pButtonExile->Enable( m_bMaster );
	}
}

void CDnChatRoomPopupDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case MESSAGEBOX_FRIEND_QUERY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if (IsCmdControl("ID_YES"))
				{
					GetFriendTask().RequestFriendAdd( 0, m_wszName.c_str() );
				}
			}
		}
		break;
	case KICK_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( IsCmdControl("ID_NORMAL_KICK") )
				{
					GetChatRoomTask().RequestKickUser( m_wszName.c_str(), false );
				}
				else if( IsCmdControl("ID_PERMAN_KICK") )
				{
					GetChatRoomTask().RequestKickUser( m_wszName.c_str(), true );
				}
				m_pKickDlg->Show( false );
			}
		}
		break;
	}
}

bool CDnChatRoomPopupDlg::CheckFriendCondition(const WCHAR* name)
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	if( !pFriendTask )
		return false;

	if (pFriendTask->IsFriend(name))
		return false;
	return true;
}

bool CDnChatRoomPopupDlg::IsFriend( const WCHAR *name )
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	bool bFriend = ( pFriendTask != NULL ) && pFriendTask->IsFriend(name);
	return bFriend;
}