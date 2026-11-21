#include "StdAfx.h"
#include "DnPVPPopupDlg.h"
#include "DnInterface.h"
#include "DnFriendTask.h"
#include "TaskManager.h"
#include "DnPlayerActor.h"
#include "DnPVPLobbyChatTabDlg.h"
#include "DnPVPLobbyVillageTask.h"
#include "DnWorld.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



CDnPVPPopupDlg::CDnPVPPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pButtonPrivate(NULL)
, m_pButtonFriend(NULL)
{
}

CDnPVPPopupDlg::~CDnPVPPopupDlg(void)
{
}

void CDnPVPPopupDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpPopupDlg.ui" ).c_str(), bShow );
}

void CDnPVPPopupDlg::InitialUpdate()
{
	m_pButtonPrivate = GetControl<CEtUIButton>("ID_BUTTON_CHAT");
	m_pButtonPrivate->Show(true);

	m_pButtonFriend = GetControl<CEtUIButton>("ID_BUTTON_FRIEND");
	m_pButtonFriend->Show(true);
}


void CDnPVPPopupDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		InitControl();
	}
	else
	{
		m_pButtonPrivate->Enable(false);
		m_pButtonFriend->Enable(false);
	}

	CEtUIDialog::Show( bShow );
}

void CDnPVPPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
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

		Show(false);
		return;
	}
}


void CDnPVPPopupDlg::InitControl()
{

	// Whisper and add friend are available for all guild members except self

	CDnPVPLobbyVillageTask *pPVPLobbyTask = (CDnPVPLobbyVillageTask *)CTaskManager::GetInstance().GetTask( "PVPLobbyTask" );
	if(!pPVPLobbyTask)
		return;

	m_pButtonPrivate->Enable(true); // Since only online users are shown, whisper is 100% possible.

	if( __wcsicmp_l( m_wstTargetName.c_str(), pPVPLobbyTask->GetUserInfo().wszCharacterName ) != 0 ) {
		m_pButtonFriend->Enable(true);
	}
	else
	{
		m_pButtonPrivate->Enable(false); // Name matches mine => cannot whisper 
	}

	// Need to double-check friend add; return false if already added.
	if( !CheckFriendCondition(m_wstTargetName.c_str()) )
		m_pButtonFriend->Enable(false);

}

void CDnPVPPopupDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
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

bool CDnPVPPopupDlg::CheckFriendCondition(const WCHAR* name)
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	if( !pFriendTask )
		return false;

	if (pFriendTask->IsFriend(name))
		return false;
	return true;
}

bool CDnPVPPopupDlg::IsFriend( const WCHAR *name )
{
	CDnFriendTask* pFriendTask = static_cast<CDnFriendTask*>(CTaskManager::GetInstance().GetTask( "FriendTask" ));
	bool bFriend = ( pFriendTask != NULL ) && pFriendTask->IsFriend(name);
	return bFriend;
}

bool CDnPVPPopupDlg::CheckValidCommand()
{
	if( CDnWorld::GetInstance().GetMapType() != CDnWorld::MapTypeVillage )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3797 ) );
		return false;
	}
	return true;
}

