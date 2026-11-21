#include "StdAfx.h"
#include "DnGuildWarPopupDlg.h"
#include "DnInterface.h"
#include "DnFriendTask.h"
#include "TaskManager.h"
#include "DnPlayerActor.h"
#include "DnPVPLobbyChatTabDlg.h"
#include "DnPVPLobbyVillageTask.h"
#include "DnWorld.h"
#include "PvPSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



CDnGuildWarPopupDlg::CDnGuildWarPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pButtonMaster(NULL)
, m_pButtonPrivate(NULL)
, m_pButtonFriend(NULL)
, m_bMasterDelegate(false)
, m_uiSessionID(0)
{
}

CDnGuildWarPopupDlg::~CDnGuildWarPopupDlg(void)
{
}

void CDnGuildWarPopupDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildWarReadyPopupDlg.ui" ).c_str(), bShow );
}

void CDnGuildWarPopupDlg::InitialUpdate()
{
	m_pButtonMaster = GetControl<CEtUIButton>("ID_BUTTON_MARSTER");
	m_pButtonMaster->Show(true);

	m_pButtonPrivate = GetControl<CEtUIButton>("ID_BUTTON_CHAT");
	m_pButtonPrivate->Show(true);

	m_pButtonFriend = GetControl<CEtUIButton>("ID_BUTTON_FRIEND");
	m_pButtonFriend->Show(true);
}


void CDnGuildWarPopupDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		InitControl();
	}
	else
	{
		m_pButtonMaster->Enable(false);
		m_pButtonPrivate->Enable(false);
		m_pButtonFriend->Enable(false);

		m_bMasterDelegate = false;
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildWarPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );


	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CHAT")) 
		{
			CDnChatTabDlg *pChatDialog = (CDnChatTabDlg*)GetInterface().GetPVPLobbyChatTabDlg();
			if(pChatDialog)
				pChatDialog->ShowEx(true);
			pChatDialog->SetPrivateName(m_wstTargetName );
		}
		else if( IsCmdControl("ID_BUTTON_FRIEND") )
		{
			wchar_t wszQuery[255];
			swprintf_s(wszQuery,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1418 ), m_wstTargetName.c_str() );	// UISTRING : Add %s to your friend list?
			GetInterface().MessageBox( wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this);
		}
		else if( IsCmdControl("ID_BUTTON_MARSTER") )
		{
			SendChangeMemberGrade( PvPCommon::UserState::GuildWarCaptain, m_uiSessionID, true );
		}

		Show(false);
		return;
	}
}


void CDnGuildWarPopupDlg::InitControl()
{

	// Whisper and add friend are available for all guild members except self

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(!pPVPLobbyTask)
		return;

	m_pButtonPrivate->Enable(true); // Since only online users are shown, whisper is 100% possible.

	if( __wcsicmp_l( m_wstTargetName.c_str(), pPVPLobbyTask->GetUserInfo().wszCharacterName ) != 0 ) 
	{
		m_pButtonFriend->Enable(true);

		if( m_bMasterDelegate )
			m_pButtonMaster->Enable( true );
	}
	else
	{
		m_pButtonPrivate->Enable(false); // Name matches mine => cannot whisper 
	}

	// Need to double-check friend add; return false if already added.
	if( !CheckFriendCondition(m_wstTargetName.c_str()) )
		m_pButtonFriend->Enable(false);
}

void CDnGuildWarPopupDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
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
					GetFriendTask().RequestFriendAdd( 0, m_wstTargetName.c_str() );
				}
			}
		}
		break;
	}
}

bool CDnGuildWarPopupDlg::CheckFriendCondition(const WCHAR* name)
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	if( !pFriendTask )
		return false;

	if (pFriendTask->IsFriend(name))
		return false;
	return true;
}

bool CDnGuildWarPopupDlg::IsFriend( const WCHAR *name )
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	bool bFriend = ( pFriendTask != NULL ) && pFriendTask->IsFriend(name);
	return bFriend;
}

bool CDnGuildWarPopupDlg::CheckValidCommand()
{
	if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3797 ) );
		return false;
	}
	return true;
}

