
#include "stdafx.h"
#include "DNLadderSystemManager.h"
#include "DNLadderRoomRepository.h"
#include "DNUserSession.h"
#include "DNLadderRoom.h"
#include "DNMasterConnection.h"
#include "DNDBConnection.h"
#include "DNUserSessionManager.h"
#include "DNField.h"

using namespace LadderSystem;

CManager::CManager()
:m_pRoomRepository( new CRoomRepository() )
,m_unChannelID(0)
{
}

CManager::~CManager()
{
	delete m_pRoomRepository;
}

void CManager::Process( DWORD dwCurTick )
{
	m_pRoomRepository->Process( dwCurTick );
}

int CManager::OnEnterChannel( CDNUserSession* pSession, MatchType::eCode MatchType )
{
	bool bAlreadyLadder = m_pRoomRepository->bIsExist( pSession->GetCharacterName() );

	if( bAlreadyLadder == true )
	{
		int iLeaveRet = OnLeaveChannel( pSession);
		if( iLeaveRet == ERROR_NONE )
			return OnEnterChannel( pSession, MatchType );
		return iLeaveRet;
	}

	// Enter ladder room
	int iRet = m_pRoomRepository->Create( pSession, MatchType );

	// Process EnterChannel Ret value
	pSession->SendLadderEnterChannel( iRet, MatchType );

	if( iRet == ERROR_NONE )
	{
		CRoom* pRoom = m_pRoomRepository->GetRoomPtr( pSession->GetCharacterName() );
		
		if( MatchType == MatchType::_1vs1 )
		{
			// Request per-class match record for 1:1 Ladder
			pSession->GetDBConnection()->QueryGetListPvPLadderScoreByJob( pSession );
		}

		// Set RoomState
		pRoom->ChangeRoomState( RoomState::WaitUser );

		// Set room master
		pRoom->AdjustNewLeader();		
	}
	return iRet;
}

int	CManager::OnJoinRoom( CDNUserSession* pSession, CRoom* pRoom )
{
	// Process EnterChannel Ret value
	pSession->SendLadderEnterChannel( ERROR_NONE, pRoom->GetMatchType() );

	// Enter ladder room
	m_pRoomRepository->Join( pSession, pRoom );

	return ERROR_NONE;
}

int CManager::OnLeaveChannel( CDNUserSession* pSession )
{
	// Leave ladder room
	int iRet = m_pRoomRepository->Leave( pSession, Reason::LeaveRoom );
	pSession->SendLadderLeaveChannel( iRet );
	return iRet;
}

int CManager::OnMatching( CDNUserSession* pSession, bool bIsCancel )
{
	int iRet = m_pRoomRepository->Matching( pSession, bIsCancel );
	pSession->SendLadderMatching( iRet, bIsCancel );
	return iRet;
}

void CManager::OnDisconnectUser( INT64 biCharDBID, WCHAR* pwszCharName )
{
	m_pRoomRepository->DisconnectUser( biCharDBID, pwszCharName );
}

bool CManager::OnReconnect( CDNUserSession* pSession )
{
	CRoom* pRoom = GetRoomPtr( pSession->GetCharacterName() );
	if( pRoom == NULL )
		return false;

	if( pRoom->bIsValidUser( pSession ) == false )
		return false;

	switch( pRoom->GetMatchType() )
	{
		case MatchType::_1vs1:
		{
			// Configure ladder user
			pSession->SetLadderUser( true );
			// Ladder channel entry notification
			pSession->SendLadderEnterChannel( ERROR_NONE, pRoom->GetMatchType() );
			// PvPLadderScore Refresh
			pSession->SendPvPLadderScoreInfo( pSession->GetPvPLadderScoreInfoPtr() );
			// Request per-class match record for 1:1 Ladder
			pSession->GetDBConnection()->QueryGetListPvPLadderScoreByJob( pSession );
			// Change RoomState
			pRoom->ChangeRoomState( RoomState::WaitUser );
			break;
		}
		default:
		{
			// PvPLadderScore Refresh
			pSession->SendPvPLadderScoreInfo( pSession->GetPvPLadderScoreInfoPtr() );

			if( pRoom->GetRoomState()&(RoomState::GameFinished|RoomState::GameRoomDestroy|RoomState::WaitUser) )
			{
				// Configure ladder user
				pSession->SetLadderUser( true );
				// Ladder channel entry notification
				pSession->SendLadderEnterChannel( ERROR_NONE, pRoom->GetMatchType() );
				// Update user info
				pRoom->UpdateUserInfo( pSession );
				// Update room master info
				const WCHAR* pLeaderName = pRoom->GetCharName( pRoom->GetLeaderCharDBID() );
				if( pLeaderName )
					pSession->SendLadderNotifyLeader( pLeaderName );
				// Change RoomState
				pRoom->ChangeRoomState( RoomState::WaitUser );
				break;
			}

			m_pRoomRepository->Leave( pSession, Reason::LeaveRoom );
			return false;
		}
	}

	return true;
}

int CManager::OnEnterObserver( CDNUserSession* pSession, CS_OBSERVER* pPacket )
{
	CRoom* pMyRoom = GetRoomPtr( pSession->GetCharacterName() );
	if( pMyRoom == NULL )
		return ERROR_GENERIC_INVALIDREQUEST;

	switch( pMyRoom->GetRoomState() )
	{
		case RoomState::WaitUser:
		{
			break;
		}
		default:
		{
			return ERROR_LADDERSYSTEM_CANT_OBSERVER_ROOMSTATE;
		}
	}

	if( pMyRoom->GetUserInfo().size() > 1 )
		return ERROR_LADDERSYSTEM_CANT_OBSERVER_ROOMSTATE;

	CRoom* pRoom = GetRoomPtr( pPacket->wszCharName );
	if( pRoom == NULL )
		return ERROR_LADDERSYSTEM_CANT_EXIST_ROOM;

	if( pRoom->GetRoomState() != RoomState::Playing )
		return ERROR_LADDERSYSTEM_CANT_OBSERVER_ROOMSTATE;

	if( pRoom->bIsInviting() == true )
		return ERROR_LADDERSYSTEM_INVITING;

	int iGameServerID,iRoomID;
	pRoom->CopyGameServerInfo( iGameServerID, iRoomID );

	g_pMasterConnection->SendLadderObserver( pSession->GetCharacterDBID(), iGameServerID, iRoomID );
	pMyRoom->SendPvPGameModeTableID( pRoom->GetGameModeTableID(), false );
	pMyRoom->ChangeRoomState( RoomState::Observer );

	return ERROR_NONE;
}

int	CManager::OnInviteUser( CDNUserSession* pInviteSession, const WCHAR* pwszCharName )
{
	CDNUserSession* pDestSession = g_pUserSessionManager->FindUserSessionByName( pwszCharName );
	if( pDestSession == NULL || pDestSession->GetField() == NULL )
		return ERROR_GENERIC_USER_NOT_FOUND;

	// Cannot invite if not in PvP lobby, already in a PvP room, or already in a ladder room
	if( pDestSession->GetField()->bIsPvPLobby() == false || pDestSession->GetPvPIndex() > 0 || GetRoomPtr( pDestSession->GetCharacterName() ) )
		return ERROR_LADDERSYSTEM_CANT_RECVINVITESTATE;

	// Check if I am in a ladder room
	CRoom* pLadderRoom = GetRoomPtr( pInviteSession->GetCharacterName() );
	if( pLadderRoom == NULL )
		return ERROR_LADDERSYSTEM_CANT_INVITESTATE;

	// Check RoomState
	if( pLadderRoom->GetRoomState() != LadderSystem::RoomState::WaitUser )
		return ERROR_LADDERSYSTEM_CANT_INVITESTATE;

	// Check if my ladder room is full
	if( pLadderRoom->bIsFullUser() == true )
		return ERROR_LADDERSYSTEM_ROOM_FULLUSER;

	//Check the target's game options.
	if( pDestSession->IsAcceptAbleOption( 0, 0, _ACCEPTABLE_CHECKTYPE_LADDERINVITE ) == false)
		return ERROR_LADDERSYSTEM_INVITE_DENY;
	
		// Check target's level
		if( pDestSession->GetLevel() < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPLadder_LimitLevel )) )
			return ERROR_LADDERSYSTEM_CANT_INVITE_LEVELLIMIT;
	// Add to invitation list
	pLadderRoom->AddInviteUser( pDestSession->GetCharacterName() );

	// Send invitation
	pDestSession->SendPvPLadderInviteConfirmReq( pInviteSession->GetCharacterName(), pLadderRoom->GetMatchType(), static_cast<int>(pLadderRoom->GetUserInfo().size()), pLadderRoom->GetAvgGradePoint() );
	return ERROR_NONE;
}

void CManager::OnInviteUserConfirm( const VIMALadderInviteConfirm* pPacket )
{
	CDNUserSession* pInviteSession	= g_pUserSessionManager->FindUserSessionByName( pPacket->sInviteComfirm.wszCharName );
	CDNUserSession* pConfirmSession	= g_pUserSessionManager->FindUserSessionByAccountDBID( pPacket->uiAccountDBID );

	// If there is no inviter
	if( pConfirmSession && pInviteSession == NULL )
	{
		pConfirmSession->SendPvPLadderInviteConfirm( ERROR_LADDERSYSTEM_CANT_EXIST_ROOM, pPacket->sInviteComfirm.wszCharName );
		return;
	}

	if( pInviteSession && pConfirmSession )
	{
		// Check ladder room
		CRoom* pLadderRoom = GetRoomPtr( pInviteSession->GetCharacterName() );
		if( pLadderRoom == NULL )
		{
			pConfirmSession->SendPvPLadderInviteConfirm( ERROR_LADDERSYSTEM_CANT_EXIST_ROOM, pInviteSession->GetCharacterName() );
			return;
		}

		// Check if invited
		if( pLadderRoom->bIsInviteUser( pConfirmSession->GetCharacterName() ) == false )
		{
			pConfirmSession->SendPvPLadderInviteConfirm( ERROR_LADDERSYSTEM_INVALID_INVITEUSER, pInviteSession->GetCharacterName() );
			return;
		}

		// Remove from invitation list
		pLadderRoom->DelInviteUser( pConfirmSession->GetCharacterName() );

		// Check if room is full
		if( pLadderRoom->bIsFullUser() == true )
		{
			pConfirmSession->SendPvPLadderInviteConfirm( ERROR_LADDERSYSTEM_ROOM_FULLUSER, pInviteSession->GetCharacterName() );
			return;
		}

		// Check RoomState
		if( pLadderRoom->GetRoomState() != LadderSystem::RoomState::WaitUser )
		{
			pConfirmSession->SendPvPLadderInviteConfirm( ERROR_LADDERSYSTEM_CANT_JOIN_ROOMSTATE, pInviteSession->GetCharacterName() );
			return;
		}

		pConfirmSession->SendPvPLadderInviteConfirm( ERROR_NONE, pInviteSession->GetCharacterName() );

		if( pPacket->sInviteComfirm.bAccept == true )
		{
			pInviteSession->SendPvPLadderInviteUser( ERROR_NONE, pConfirmSession->GetCharacterName() );
			OnJoinRoom( pConfirmSession, pLadderRoom );
		}
		else
		{
			pInviteSession->SendPvPLadderInviteUser( ERROR_LADDERSYSTEM_INVITE_DENY, pConfirmSession->GetCharacterName() );
		}
		
		return;
	}	
}

int	CManager::OnKickOut( CDNUserSession* pSession, INT64 biCharacterDBID )
{
	CRoom* pRoom = GetRoomPtr( pSession->GetCharacterName() );
	if( pRoom == NULL )
		return ERROR_LADDERSYSTEM_CANT_EXIST_ROOM;

	switch( pRoom->GetRoomState() )
	{
		case RoomState::WaitUser:
			break;
		default:
			return ERROR_LADDERSYSTEM_CANT_KICKOUT_ROOMSTATE;
	}

	if( pRoom->GetLeaderCharDBID() != pSession->GetCharacterDBID() )
		return ERROR_LADDERSYSTEM_NOLEADER;

	std::wstring wstrCharName;
	for( UINT i=0 ; i<pRoom->GetUserInfo().size() ; ++i )
	{
		if( pRoom->GetUserInfo()[i].biCharDBID == biCharacterDBID )
			wstrCharName = pRoom->GetUserInfo()[i].wszCharName;
	}

	if( wstrCharName.empty() == true )
		return ERROR_INVALIDUSER_LADDERROOM;

	CDNUserSession* pKickOutSession = g_pUserSessionManager->FindUserSessionByName( wstrCharName.c_str() );
	if( pKickOutSession == NULL )
		return ERROR_INVALIDUSER_LADDERROOM;

	int iRet = m_pRoomRepository->Leave( pKickOutSession, Reason::KickOut );
	if( iRet != ERROR_NONE )
		return iRet;

	pKickOutSession->SendLadderLeaveChannel( ERROR_NONE );
	pKickOutSession->SendPvPLadderKickOut( ERROR_LADDERSYSTEM_KICKOUT );

	return ERROR_NONE;
}

void CManager::SetChannelID( USHORT unChannelID )
{ 
	if( m_unChannelID != 0 )
		_ASSERT(0);
	m_unChannelID=unChannelID; 
}

USHORT CManager::GetChannelID()
{ 
	if( m_unChannelID == 0 )
		_ASSERT(0);
	return m_unChannelID; 
}

int CManager::GetAvgMatchingTimeSec( MatchType::eCode MatchType )
{ 
	return m_pRoomRepository->GetAvgMatchingTimeSec(MatchType); 
}

CMatchingSystem* CManager::GetMatchingSystemPtr()
{
	return m_pRoomRepository ? m_pRoomRepository->GetMatchingSystemPtr() : NULL;
}

CRoom* CManager::GetRoomPtr( const WCHAR* pwszCharName )
{
	return m_pRoomRepository->GetRoomPtr( pwszCharName );
}

CRoom* CManager::GetRoomPtr( INT64 biRoomIndex )
{
	return m_pRoomRepository->GetRoomPtr( biRoomIndex );
}

bool CManager::bIsValidPairRoom( INT64 biRoomIndex, INT64 biRoomIndex2 )
{
	CRoom* pRoom	= GetRoomPtr( biRoomIndex );
	CRoom* pRoom2	= GetRoomPtr( biRoomIndex2 );

	if( pRoom && pRoom2 )
	{
		if( pRoom->GetMatchType() != pRoom2->GetMatchType() )
			return false;

		if( pRoom->GetOpponentRoomIndex() == pRoom2->GetRoomIndex() && pRoom2->GetOpponentRoomIndex() == pRoom->GetRoomIndex() )
			return true;
	}

	return false;
}

void CManager::ChangeRoomState( INT64 biRoomIndex, RoomState::eCode State, RoomStateReason::eCode Reason/*=RoomStateReason::ERROR_NONE*/ )
{
	CRoom* pRoom = GetRoomPtr( biRoomIndex );
	if( pRoom == NULL )
		return;

	pRoom->ChangeRoomState( State, Reason );
}

void CManager::InsertPlayingList( CRoom* pRoom )
{
	m_pRoomRepository->InsertPlayingList( pRoom );
}

void CManager::DeletePlayingList( CRoom* pRoom )
{
	m_pRoomRepository->DeletePlayingList( pRoom );
}
