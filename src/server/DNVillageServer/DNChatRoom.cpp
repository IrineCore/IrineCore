#include "stdafx.h"
#include "DNChatRoom.h"
#include "DNUserSession.h"
#include "DNUserSessionManager.h"
#include "DNFriend.h"

CDNChatRoom::CDNChatRoom(UINT nChatRoomID, WCHAR * pwzRoomName, BYTE nRoomType, USHORT uRoomAllow, WCHAR * pwzPassword,
						 WCHAR * pwzPRLine1, WCHAR * pwzPRLine2, WCHAR * pwzPRLine3 )
{
	memset(m_nChatRoomMemberArray, 0, sizeof(m_nChatRoomMemberArray));
	m_nLeaderAccountDBID = 0;

	m_nChatRoomID		= nChatRoomID;

	m_nChatRoomType		= nRoomType;
	m_nChatRoomAllow	= uRoomAllow;

	m_wstrChatRoomName	= pwzRoomName;
	m_wstrChatRoomPW	= pwzPassword;

	m_wstrChatRoomPRLine1	= pwzPRLine1;
	m_wstrChatRoomPRLine2	= pwzPRLine2;
	m_wstrChatRoomPRLine3	= pwzPRLine3;

	m_VecKickedList.clear();
}

CDNChatRoom::~CDNChatRoom()
{
	m_VecKickedList.clear();
}

UINT CDNChatRoom::GetMemberAIDFromIndex( int nIndex )
{
	if( nIndex < 0 || nIndex >= CHATROOMMAX )	return 0;

	return m_nChatRoomMemberArray[ nIndex ];
}

int CDNChatRoom::GetUserCount()
{
	int nUserCount = 0;
	for( int i=0; i<CHATROOMMAX; i++ )
	{
		if( m_nChatRoomMemberArray[i] <= 0 )	continue;

		nUserCount++;
	}

	return nUserCount;
}

bool CDNChatRoom::IsLeader( UINT nUserAID )
{
	if( m_nLeaderAccountDBID <= 0 )			return false;
	if( nUserAID != m_nLeaderAccountDBID )	return false;

	return true;
}

int CDNChatRoom::LeaveUser( UINT nUserAID, int nReason )
{
	int idx = _FindUserFromAID( nUserAID );
	if (idx < 0 )
		return ERROR_CHATROOM_UNKNOWNUSER;		// User is not in the room.

	if( nReason == CHATROOMLEAVE_FOREVERKICKED )
	{
		m_VecKickedList.push_back( nUserAID );
	}

	m_nChatRoomMemberArray[idx] = 0;

	// Re-sort since a member left
	for( int i=idx; i < CHATROOMMAX-1; i++ )
	{
		m_nChatRoomMemberArray[i] = m_nChatRoomMemberArray[i+1];
	}
	m_nChatRoomMemberArray[CHATROOMMAX-1] = 0;

	// Clear user's chat room ID - pointer may be null for disconnected users.
	CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID( nUserAID );
	if( pUser != NULL )
	{
		pUser->m_nChatRoomID = 0;
	}

	return ERROR_NONE;
}

int CDNChatRoom::EnterRoom( UINT nUserAID, WCHAR * pwzPassword, bool bLeader )
{
	CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID( nUserAID );
	if( pUser == NULL )					return ERROR_GENERIC_USER_NOT_FOUND;

	int nCanEnter = 0;
	if( (nCanEnter = _CanEnterRoom( nUserAID, pwzPassword, bLeader )) != ERROR_NONE )		return nCanEnter;

	int nEmptyIndex = _FindEmptyIndex();
	if( nEmptyIndex < 0 )				return ERROR_CHATROOM_NOMORESPACE;		// Room is full

	m_nChatRoomMemberArray[nEmptyIndex] = nUserAID;
	if( bLeader )
		m_nLeaderAccountDBID = nUserAID;

	// Set user's chat room ID.
	pUser->m_nChatRoomID = m_nChatRoomID;

	return ERROR_NONE;
}

void CDNChatRoom::ChangeRoomOption( WCHAR * pwzRoomName, BYTE nRoomType, USHORT nRoomAllow, WCHAR * pwzPassword,
									WCHAR * pwzPRLine1, WCHAR * pwzPRLine2, WCHAR * pwzPRLine3 )
{
	m_wstrChatRoomName = pwzRoomName;
	m_nChatRoomType = nRoomType;
	m_nChatRoomAllow = nRoomAllow;
	m_wstrChatRoomPW = pwzPassword;
	m_wstrChatRoomPRLine1 = pwzPRLine1;
	m_wstrChatRoomPRLine2 = pwzPRLine2;
	m_wstrChatRoomPRLine3 = pwzPRLine3;
}

void CDNChatRoom::SendChatMessageToAll( int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg )
{
	for( int i=0; i<CHATROOMMAX; i++ )
	{
		if( m_nChatRoomMemberArray[i] <= 0 )		continue;

		CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID( m_nChatRoomMemberArray[i] );
		if( pUser == NULL )							continue;

		pUser->SendChat( CHATTYPE_CHATROOM, cLen, pwszCharacterName, pwszChatMsg );
	}
}

void CDNChatRoom::SendProfileToAll( int nSessionID, TProfile & profile )
{
	for( int i=0; i<CHATROOMMAX; i++ )
	{
		if( m_nChatRoomMemberArray[i] <= 0 )		continue;

		CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID( m_nChatRoomMemberArray[i] );
		if( pUser == NULL )							continue;

		pUser->SendDisplayProfile( nSessionID, profile );
	}
}

int CDNChatRoom::_CanEnterRoom( UINT nUserAID, WCHAR * pwzPassword, bool bLeader )
{
	CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID(nUserAID);
	if( pUser == NULL )					return ERROR_CHATROOM_CANNOTENTER;


	// 1. General check
	// 1.1 Check empty space
	int nEmptyIndex = _FindEmptyIndex();
	if( nEmptyIndex < 0 )			return ERROR_CHATROOM_NOMORESPACE;		// Room is full
	
	// 1.2 Is currently in a room?
	int nMyIndex = _FindUserFromAID( nUserAID );
	if( nMyIndex >= 0 )				return ERROR_CHATROOM_ALREADYINROOM;	// Already participating in a room.

	// 1.3 Exists in permanent kick list?
	for( UINT i=0; i<m_VecKickedList.size(); i++)
	{
		if( m_VecKickedList[i] == nUserAID )
			return ERROR_CHATROOM_KICKEDFOREVER;
	}

	// 2. Check entry conditions
	bool bCanEnterRoom = true;

	// 2.1 Password
	if( m_wstrChatRoomPW.length() > 0 && m_wstrChatRoomPW != pwzPassword )	return ERROR_CHATROOM_WRONGPASSWORD;

	// 2.2 Check entry restrictions
	if( bLeader == false )	// Leader is not subject to entry restrictions.
	{
		CDNUserSession * pLeader = g_pUserSessionManager->FindUserSessionByAccountDBID(m_nLeaderAccountDBID);
		if( pLeader == NULL )		return ERROR_CHATROOM_UNKNOWNUSER;

		bool bFriend = pLeader->GetFriend()->HasFriend( pUser->GetCharacterDBID() );
		bool bSameGuild = pLeader->GetGuildUID().IsSet() && pUser->GetGuildUID().IsSet() && pLeader->GetGuildUID() == pUser->GetGuildUID();

		switch( m_nChatRoomAllow )
		{
		case CHATROOMALLOW_ALL:
			break;
		case CHATROOMALLOW_FRIEND:
			if( bFriend == false )						return ERROR_CHATROOM_CANNOTENTER;
			break;
		case CHATROOMALLOW_GUILD:
			if( bSameGuild == false )					return ERROR_CHATROOM_CANNOTENTER;
			break;
		case CHATROOMALLOW_FRIENDANDGUILD:
			if( (bFriend || bSameGuild) == false )		return ERROR_CHATROOM_CANNOTENTER;
			break;
		}
	}

	return ERROR_NONE;
}

int CDNChatRoom::_FindUserFromAID( UINT nUserAID )
{
	for( int i=0; i < CHATROOMMAX; i++ )
	{
		if( m_nChatRoomMemberArray[i] == nUserAID )
		{
			return i;
		}
	}

	return -1;
}

int CDNChatRoom::_FindEmptyIndex()
{
	for( int i=0; i < CHATROOMMAX; i++ )
	{
		if( m_nChatRoomMemberArray[i] <= 0 )
			return i;
	}

	return -1;
}