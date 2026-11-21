#include "StdAfx.h"
#include "DnChatRoomTask.h"
#include "DnInterface.h"
#include "ChatRoomSendPacket.h"
#include "DnPlayerActor.h"
#include "DnChatRoomDlg.h"
#include "DnChatOption.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnChatRoomTask::CDnChatRoomTask()
: CTaskListener( false )
, m_bRequestWait( false )
, m_nChatRoomAllow( 0 )
, m_nRequestRoomID( 0 )
{
}

CDnChatRoomTask::~CDnChatRoomTask()
{
	Finalize();
}

void CDnChatRoomTask::OnDisconnectTcp( bool bValidDisconnect )
{
}

void CDnChatRoomTask::OnDisconnectUdp()
{
}

void CDnChatRoomTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	switch( nMainCmd ) {
		case SC_CHATROOM: OnRecvChatRoomMessage( nSubCmd, pData, nSize ); break;
		case SC_ROOM: OnRecvRoomMessage( nSubCmd, pData, nSize ); break;
	}
}

bool CDnChatRoomTask::Initialize()
{
	return true;
}

void CDnChatRoomTask::Finalize()
{
}

void CDnChatRoomTask::OnRecvChatRoomMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eChatRoom::SC_CREATECHATROOM:		OnRecvChatRoomCreateRoom( (SCCreateChatRoom*)pData ); break;
		case eChatRoom::SC_ENTERCHATROOM:		OnRecvChatRoomEnterRoom( (SCChatRoomEnterRoom*)pData ); break;
		case eChatRoom::SC_ENTERUSERCHATROOM:	OnRecvChatRoomEnterUser( (SCChatRoomEnterUser*)pData ); break;
		case eChatRoom::SC_CHANGEROOMOPTION:	OnRecvChatRoomChangeRoomOption( (SCChatRoomChangeRoomOption*)pData ); break;
		case eChatRoom::SC_KICKUSER:			OnRecvChatRoomKickUser( (SCChatRoomKickUser*)pData ); break;
		case eChatRoom::SC_LEAVEUSER:			OnRecvChatRoomLeave( (SCChatRoomLeaveUser*)pData ); break;

		case eChatRoom::SC_CHATROOMVIEW:		OnRecvChatRoomView( (SCChatRoomView*)pData ); break;
	}
}

void CDnChatRoomTask::OnRecvRoomMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eRoom::SC_SYNC_CHATROOMINFO:		OnRecvRoomSyncChatRoomInfo( (SCROOM_SYNC_CHATROOMINFO*)pData ); break;
	}
}

void CDnChatRoomTask::OnRecvChatRoomCreateRoom( SCCreateChatRoom *pPacket )
{
	m_bRequestWait = false;

	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	if( pPacket->nRet == ERROR_NONE )
	{
		// After setting info
		m_ChatRoomView.Set( pPacket->ChatRoomView );
		GetInterface().GetChatRoomDlg()->SetRoomInfo( pPacket->ChatRoomView.m_nChatRoomID, pPlayer->GetUniqueID(), pPacket->ChatRoomView.m_wszChatRoomName, true );
		GetInterface().ShowChatRoomDialog( true );
		OnSetChatRoomView( pPlayer, &pPacket->ChatRoomView );

		bool bResetAction = true;
		if( strcmp( pPlayer->GetCurrentAction(), "Normal_Stand" ) == 0 )
			bResetAction = false;
		if( strcmp( pPlayer->GetCurrentAction(), "PutIn_Weapon" ) == 0 )
			bResetAction = false;
		if( bResetAction )
			pPlayer->CmdStop( "Normal_Stand", 0, 15.f, 0.f, true );
	}
	else
	{
		// Is it really necessary to close the creation window..
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnChatRoomTask::OnRecvChatRoomView( SCChatRoomView *pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	// Check if self
	if( pPacket->nSessionID == pPlayer->GetUniqueID() ) {
		// Heard that rooms created by self are not sent back to self.
		return;
	}

	// First check if it is in the list
	TChatRoomView *pView = FindChatRoomView( pPacket->nSessionID );
	if( pView )
	{
		pView->Set( pPacket->ChatRoomView );
	}
	else
	{
		m_listSCChatRoomView.push_back( *pPacket );
	}

	// Find nearby actor with SessionID and apply if found.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pActor ) return;

	OnSetChatRoomView( pActor, &pPacket->ChatRoomView );

	// If the host changed the room name while in the chat room, it can be detected here.
	if( pPacket->ChatRoomView.m_nChatRoomID == GetInterface().GetChatRoomDlg()->GetRoomID() )
	{
		GetInterface().GetChatRoomDlg()->SetRoomInfo( pPacket->ChatRoomView.m_nChatRoomID, pPacket->nSessionID, pPacket->ChatRoomView.m_wszChatRoomName );
	}
}

void CDnChatRoomTask::OnRecvChatRoomLeave( SCChatRoomLeaveUser *pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	// Check if self
	if( pPacket->nUserSessionID == pPlayer->GetUniqueID() ) {
		bool bMaster = GetInterface().GetChatRoomDlg()->IsMaster();
		if( bMaster ) {
			m_ChatRoomView.Reset();
			OnSetChatRoomView( pPlayer, &m_ChatRoomView );
		}
		else {
			OnNotifyRoomEnter( pPlayer, false );
		}
		bool bShow = GetInterface().GetChatRoomDlg()->IsShow();
		GetInterface().ShowChatRoomDialog( false );

		if( bMaster || !bShow )
			return;

		int nMsgIndex = 0;
		switch( pPacket->nLeaveReason )
		{
		case CHATROOMLEAVE_KICKED:			nMsgIndex = 121059; break;
		case CHATROOMLEAVE_FOREVERKICKED:	nMsgIndex = 121060; break;
		case CHATROOMLEAVE_DESTROYROOM:		nMsgIndex = 8138; break;
		}
		if( nMsgIndex )
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgIndex ), false );

		return;
	}

	// If it is another person..
	// Find nearby actor with SessionID and apply if found.
	if( GetInterface().GetChatRoomDlg()->IsMyChatRoomMember( pPacket->nUserSessionID ) )
	{
		GetInterface().GetChatRoomDlg()->DelUser( pPacket->nUserSessionID, pPacket->nLeaveReason );
	}

	// Seems to have left from another room. Find nearby and clear the speech bubble.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nUserSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pActor ) return;

	OnNotifyRoomEnter( pActor, false );
}

void CDnChatRoomTask::OnRecvChatRoomEnterRoom( SCChatRoomEnterRoom *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		int nChatRoomID = 0;
		int nChatRoomType = 0;
		std::wstring wszRoomName;
		TChatRoomView *pView = FindChatRoomView( pPacket->LeaderSID );
		if( pView ) {
			nChatRoomID = pView->m_nChatRoomID;
			nChatRoomType = pView->m_nChatRoomType;
			wszRoomName = pView->m_wszChatRoomName;
		}
		GetInterface().GetChatRoomDlg()->SetRoomInfo( nChatRoomID, pPacket->LeaderSID, wszRoomName.c_str() );
		for( int i = 0; i < CHATROOMMAX; ++i )
		{
			if( pPacket->UserSessionIDs[i] == 0 ) continue;
			if( pPacket->UserSessionIDs[i] == pPacket->LeaderSID ) continue;
			GetInterface().GetChatRoomDlg()->AddUser( pPacket->UserSessionIDs[i] );
		}
		GetInterface().ShowChatRoomDialog( true );

		// Display own participation speech bubble.
		if( !CDnActor::s_hLocalActor ) return;
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		OnNotifyRoomEnter( pPlayer, true, nChatRoomType );

		// And sit.
		pPlayer->CmdAction( "Social_Sit01" );
	}
	else if( pPacket->nRet == ERROR_CHATROOM_WRONGPASSWORD )
	{
		GetInterface().MessageBox( 120113 );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnChatRoomTask::OnRecvChatRoomEnterUser( SCChatRoomEnterUser *pPacket )
{
	// Search nearby, process if found and finish,
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nUserSessionID );
	if( hActor )
	{
		// It would be really weird if player conversion fails, in that case skip.
		CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( pActor )
		{
			if( GetInterface().GetChatRoomDlg()->GetRoomID() == pPacket->nChatRoomID )
			{
				GetInterface().GetChatRoomDlg()->AddUser( pPacket->nUserSessionID, true );
			}

			// Show join speech bubble here.
			OnNotifyRoomEnter( pActor, true, pPacket->cChatRoomType );
		}
	}
	else
	{
		// If missing, it is likely still loading, so put it in the list and process next time.
		m_listSCChatRoomEnterUser.push_back( *pPacket );
	}
}

void CDnChatRoomTask::OnRecvChatRoomChangeRoomOption( SCChatRoomChangeRoomOption *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		m_ChatRoomView.Set( pPacket->ChatRoomView );

		if( !CDnActor::s_hLocalActor ) return;
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		OnSetChatRoomView( pPlayer, &m_ChatRoomView );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnChatRoomTask::OnRecvChatRoomKickUser( SCChatRoomKickUser *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		// If error is NONE, doesn't it go to LeaveUser?
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnChatRoomTask::RequestCreateChatRoom( const WCHAR *pRoomName, const WCHAR *pPassword, int nRoomType, int nRoomAllow, const WCHAR *pPR1, const WCHAR *pPR2, const WCHAR *pPR3 )
{
	m_wszPassword = pPassword;
	m_nChatRoomAllow = nRoomAllow;
	SendCreateChatRoom( pRoomName, pPassword, nRoomType, nRoomAllow, pPR1, pPR2, pPR3 );

	m_bRequestWait = true;
}

void CDnChatRoomTask::RequestConfigChatRoom( const WCHAR *pRoomName, const WCHAR *pPassword, int nRoomType, int nRoomAllow, const WCHAR *pPR1, const WCHAR *pPR2, const WCHAR *pPR3 )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	m_wszPassword = pPassword;
	m_nChatRoomAllow = nRoomAllow;
	SendChangeRoomOption( pRoomName, pPassword, nRoomType, nRoomAllow, pPR1, pPR2, pPR3 );
	GetInterface().GetChatRoomDlg()->SetRoomInfo( m_ChatRoomView.m_nChatRoomID, pPlayer->GetUniqueID(), pRoomName, true );
}

bool CDnChatRoomTask::IsEnableChatRoomEnter( UINT nSessionID )
{
	// Otherwise, search nearby for one with SessionID and apply if found.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor ) return false;

	TChatRoomView *pView = FindChatRoomView( nSessionID );
	if( !pView ) return false;

	if( pView->m_nChatRoomID == 0 ) return false;
	return true;
}

void CDnChatRoomTask::RequestEnterChatRoom( UINT nSessionID )
{
	// Otherwise, search nearby for one with SessionID and apply if found.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor ) return;

	TChatRoomView *pView = FindChatRoomView( nSessionID );
	if( !pView ) return;

	if( pView->m_nChatRoomID == 0 ) return;

	if( pView->m_bHasPassword )
	{
		// Show password input dialog and handle via Callback.
		m_nRequestRoomID = pView->m_nChatRoomID;
		GetInterface().ShowChatRoomPassWordDlg( true, CHATROOM_PASSWORD_DIALOG, this );
	}
	else
	{
		SendEnterChatRoom( pView->m_nChatRoomID, L"" );
	}
}

void CDnChatRoomTask::RequestKickUser( LPCWSTR pwszName, bool bForever )
{
	DnActorHandle hActor = CDnActor::FindActorFromName( (TCHAR *)pwszName );
	if( !hActor ) return;

	SendChatRoomKickUser( hActor->GetUniqueID(), bForever ? CHATROOMLEAVE_FOREVERKICKED : CHATROOMLEAVE_KICKED );
}

void CDnChatRoomTask::OnSetChatRoomView( CDnPlayerActor *pActor, TChatRoomView *pView )
{
	if( pView->m_nChatRoomID > 0 )
	{
		pActor->GetChatBalloon().ForceShowState( true );
		pActor->GetChatBalloon().StopChatBalloonCustom();
		pActor->GetChatBalloon().ClearCustomChatBalloonText();

		if( pView->m_nChatRoomType == CHATROOMTYPE_NORMAL )
		{
			pActor->GetChatBalloon().SetChatBalloonCustom( "ChatNormalDlg.ui", pView->m_wszChatRoomName, chatroomcolor::NORMAL, GetTickCount(), 1 );
		}
		else if( pView->m_nChatRoomType == CHATROOMTYPE_TRADE )
		{
			pActor->GetChatBalloon().SetChatBalloonCustom( "ChatDealDlg.ui", pView->m_wszChatRoomName, chatroomcolor::TRADE, GetTickCount(), 1 );
		}
		else if( pView->m_nChatRoomType == CHATROOMTYPE_PR )
		{
			pActor->GetChatBalloon().SetChatBalloonCustom( "ChatProfileDlg.ui", pView->m_wszChatRoomName, chatroomcolor::PROFILE, GetTickCount(), 1 );

			std::wstring wszPR[3];
			wszPR[0] = pView->m_wszChatRoomPRLine1;
			wszPR[1] = pView->m_wszChatRoomPRLine2;
			wszPR[2] = pView->m_wszChatRoomPRLine3;
			for( int i = 0; i < 3; ++i ) wszPR[i] = boost::algorithm::trim_copy( wszPR[i] );

			// Skip blank or whitespace-only lines and insert the rest as-is.
			// If none are set, nothing is added via AddCustom, so only the room name will keep showing like other rooms.
			if( wszPR[0].size() ) pActor->GetChatBalloon().AddCustomChatBalloonText( pView->m_wszChatRoomPRLine1 );
			if( wszPR[1].size() ) pActor->GetChatBalloon().AddCustomChatBalloonText( pView->m_wszChatRoomPRLine2 );
			if( wszPR[2].size() ) pActor->GetChatBalloon().AddCustomChatBalloonText( pView->m_wszChatRoomPRLine3 );
		}
	}
	else
	{
		pActor->GetChatBalloon().ForceShowState( false );
		pActor->GetChatBalloon().StopChatBalloonCustom();
		pActor->GetChatBalloon().ClearCustomChatBalloonText();
	}
}

void CDnChatRoomTask::OnNotifyRoomEnter( CDnPlayerActor *pActor, bool bEnter, int nRoomType )
{
	int nChatType = 0;
	switch( nRoomType )
	{
	case CHATROOMTYPE_NORMAL:	nChatType = CHAT_CHATROOM1; break;
	case CHATROOMTYPE_TRADE:	nChatType = CHAT_CHATROOM2; break;
	case CHATROOMTYPE_PR:		nChatType = CHAT_CHATROOM3; break;
	}

	if( bEnter )
	{
		pActor->GetChatBalloon().ForceShowState( true );
		pActor->GetChatBalloon().SetChatBalloonText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8133 ), 0, nChatType );
	}
	else
	{
		pActor->GetChatBalloon().ForceShowState( false );
	}
}

void CDnChatRoomTask::OnCharUserEntered( DnActorHandle hActor )
{
	// First check if the user was not handled during OnRecvChatRoomEnterUser and process them.
	if( !m_listSCChatRoomEnterUser.empty() )
	{
		std::list<SCChatRoomEnterUser>::iterator iter = m_listSCChatRoomEnterUser.begin();
		for( ; iter != m_listSCChatRoomEnterUser.end(); )
		{
			if( iter->nUserSessionID == hActor->GetUniqueID() )
			{
				// It would be really weird if player conversion fails, in that case skip.
				CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
				if( pActor )
				{
					// AddUser below is unlikely to be called so it could be removed, but keep it in case loading takes a long time.
					if( GetInterface().GetChatRoomDlg()->GetRoomID() == iter->nChatRoomID )
					{
						GetInterface().GetChatRoomDlg()->AddUser( hActor, true );
					}

					// Show join speech bubble here.
					OnNotifyRoomEnter( pActor, true, iter->cChatRoomType );
				}
				m_listSCChatRoomEnterUser.erase( iter );
				break;
			}
			++iter;
		}
	}

	// Afterward, process items accumulated in the room view list as usual.
	if( m_listSCChatRoomView.empty() ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pActor ) return;

	TChatRoomView *pView = FindChatRoomView( pActor->GetUniqueID() );
	if( !pView ) return;

	OnSetChatRoomView( pActor, pView );
}

void CDnChatRoomTask::OnCharUserLeaved( DnActorHandle hActor )
{
	if( m_listSCChatRoomView.empty() ) return;
	if( !hActor ) return;

	std::list<SCChatRoomView>::iterator iter = m_listSCChatRoomView.begin();
	for( ; iter != m_listSCChatRoomView.end(); )
	{
		if( iter->nSessionID == hActor->GetUniqueID() )
		{
			m_listSCChatRoomView.erase( iter );
			return;
		}
		++iter;
	}
}

TChatRoomView *CDnChatRoomTask::FindChatRoomView( UINT nSessionID )
{
	std::list<SCChatRoomView>::iterator iter = m_listSCChatRoomView.begin();
	for( ; iter != m_listSCChatRoomView.end(); )
	{
		if( iter->nSessionID == nSessionID )
		{
			return &iter->ChatRoomView;
		}
		++iter;
	}
	return NULL;
}

void CDnChatRoomTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) 
	{
	case CHATROOM_PASSWORD_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
				{
					LPCWSTR pwszPassword = GetInterface().GetChatRoomPassWord();
					SendEnterChatRoom( m_nRequestRoomID, (pwszPassword == NULL) ? L"" : pwszPassword );
				}
				else
				{

					GetInterface().ShowChatRoomPassWordDlg( false );
				}
			}
		}
		break;
	}
}

void CDnChatRoomTask::OnRecvRoomSyncChatRoomInfo( SCROOM_SYNC_CHATROOMINFO *pPacket )
{
	// When dropping into a game in progress, receive and sync the chat room info already created in that game room.
	// To match village behavior, convert the packet to SCChatRoomView and SCChatRoomEnterUser.
	// Note: unlike Village, all users in the room have already had their info received and actors created, so
	// Handling like OnCharUserEntered on user entry in village is not needed.
	if( pPacket->nLeaderID && pPacket->ChatRoomView.m_nChatRoomID )
	{
		SCChatRoomView RoomView;
		RoomView.nSessionID = pPacket->nLeaderID;
		RoomView.ChatRoomView.Set( pPacket->ChatRoomView );
		OnRecvChatRoomView( &RoomView );

		if( pPacket->nCount )
		{
			for( int i = 0; i < pPacket->nCount; ++i )
			{
				if( pPacket->nMembers[i] == 0 ) continue;
				if( pPacket->nMembers[i] == pPacket->nLeaderID ) continue;

				SCChatRoomEnterUser EnterUser;
				EnterUser.nChatRoomID = pPacket->ChatRoomView.m_nChatRoomID;
				EnterUser.cChatRoomType = pPacket->ChatRoomView.m_nChatRoomType;
				EnterUser.nUserSessionID = pPacket->nMembers[i];
				OnRecvChatRoomEnterUser( &EnterUser );
			}
		}
	}
}

void CDnChatRoomTask::ClearChatRoomInfoList()
{
	m_listSCChatRoomView.clear();
	m_listSCChatRoomEnterUser.clear();
}

void CDnChatRoomTask::ForceCloseChatRoom()
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

	// Check if self
	bool bMaster = GetInterface().GetChatRoomDlg()->IsMaster();
	if( bMaster ) {
		m_ChatRoomView.Reset();
		OnSetChatRoomView( pPlayer, &m_ChatRoomView );
	}
	GetInterface().ShowChatRoomDialog( false );
}