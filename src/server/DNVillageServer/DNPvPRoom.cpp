
#include "stdafx.h"
#include "DNPvPRoom.h"
#include "DNUserSession.h"
#include "DNUserSessionManager.h"
#include "DNGameDataManager.h"
#include "DNAuthManager.h"

extern TVillageConfig g_Config;

CDNPvPRoom::CDNPvPRoom()
:m_unVillageChannelID(0),m_iGameServerID(0),m_iGameRoomID(0),m_uiIndex(0),m_uiMaxUser(0),m_uiMapIndex(0),m_uiRoomState(PvPCommon::RoomState::None)
,m_uiGameMode(PvPCommon::GameMode::Max),m_uiWinCondition(0),m_uiPlayTimeSec(0),m_uiGameModeTableID(0), m_bIsGuildWarSystem(false)
{
	memset(m_nVoiceChannelID, 0, sizeof(m_nVoiceChannelID));
	m_cMinLevel		= 1;
	m_cMaxLevel		= CHARLEVELMAX;
	m_nEventRoomIndex	= 0;
	memset( &m_EventData, 0, sizeof(m_EventData) );
	m_unRoomOptionBit	= PvPCommon::RoomOption::None;
	memset(m_nGuildWarDBID, 0, sizeof(m_nGuildWarDBID));
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_eRoomType = PvPCommon::RoomType::max;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_bExtendObserverCount = false;
#if defined( PRE_WORLDCOMBINE_PVP )
	m_nWorldPvPRoomType = false;
	m_cWorldPvPRoomCurUserCount = 0;
	m_nWorldPvPRoomDBIndex = 0;
	m_nWorldPvPRoomPw = 0;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	m_cGambleType = 0;
	m_nGamblePrice = 0;
#endif
}

CDNPvPRoom::~CDNPvPRoom()
{
}

bool CDNPvPRoom::bIsEmpty()
{
	return m_mUserInfo.empty();
}

bool CDNPvPRoom::bIsEmptySlot() const
{
	return m_uiMaxUser > m_mUserInfo.size();
}

void CDNPvPRoom::SetRoomState( const UINT uiRoomState )
{
	m_uiRoomState = uiRoomState;
	_SendRoomState();
}

int CDNPvPRoom::Create( const MAVIPVP_CREATEROOM* pPacket )
{
	m_unVillageChannelID	= pPacket->unVillageChannelID;

	m_uiIndex				= pPacket->uiPvPIndex;
	m_nEventRoomIndex		= pPacket->sCSPVP_CREATEROOM.uiEventItemID;
	if( m_nEventRoomIndex >0 )
		memcpy( &m_EventData, &pPacket->EventData, sizeof(m_EventData) );
	m_uiMaxUser				= pPacket->sCSPVP_CREATEROOM.cMaxUser;
	m_uiMapIndex			= pPacket->sCSPVP_CREATEROOM.uiMapIndex;
	m_uiGameMode			= pPacket->cGameMode;
	m_uiWinCondition		= pPacket->sCSPVP_CREATEROOM.uiSelectWinCondition;
	m_uiPlayTimeSec			= pPacket->sCSPVP_CREATEROOM.uiSelectPlayTimeSec;
	m_uiGameModeTableID		= pPacket->sCSPVP_CREATEROOM.uiGameModeTableID;
	m_cMinLevel				= pPacket->sCSPVP_CREATEROOM.cMinLevel;
	m_cMaxLevel				= pPacket->sCSPVP_CREATEROOM.cMaxLevel;
	m_unRoomOptionBit		= pPacket->sCSPVP_CREATEROOM.unRoomOptionBit;
	for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
		m_nGuildWarDBID[i] = pPacket->nGuildWarDBID[i];	
	m_bIsGuildWarSystem = m_nGuildWarDBID[0] > 0 ? true : false;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_eRoomType = static_cast<PvPCommon::RoomType::eRoomType>(pPacket->sCSPVP_CREATEROOM.cRoomType);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_MOD_PVPOBSERVER
	m_bExtendObserverCount = pPacket->sCSPVP_CREATEROOM.bExtendObserver;
#endif		//#ifdef PRE_MOD_PVPOBSERVER

#if defined( PRE_WORLDCOMBINE_PVP )
	m_nWorldPvPRoomType = pPacket->sCSPVP_CREATEROOM.nWorldPvPRoomType;
	if( m_nWorldPvPRoomType )
		m_bExtendObserverCount = true;
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	m_cGambleType = pPacket->cGambleType;
	m_nGamblePrice = pPacket->nPrice;
#endif

	WCHAR wszBuf[MAX_PATH];
	memset( &wszBuf, 0, sizeof(WCHAR)*MAX_PATH );
	_wcscpy( wszBuf, _countof(wszBuf), pPacket->sCSPVP_CREATEROOM.wszBuf, pPacket->sCSPVP_CREATEROOM.cRoomNameLen );
	m_wstrRoomName	= wszBuf;

	if( pPacket->sCSPVP_CREATEROOM.cRoomPWLen > 0 )
	{
		memset( &wszBuf, 0, sizeof(WCHAR)*MAX_PATH );
		_wcscpy( wszBuf, _countof(wszBuf), pPacket->sCSPVP_CREATEROOM.wszBuf+pPacket->sCSPVP_CREATEROOM.cRoomNameLen, pPacket->sCSPVP_CREATEROOM.cRoomPWLen );
		m_wstrRoomPW = wszBuf;
	}
	return ERROR_NONE;
}

void CDNPvPRoom::ModifyRoom( const MAVIPVP_MODIFYROOM* pPacket )
{
	m_uiMapIndex			= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.uiMapIndex;
	m_uiGameMode			= pPacket->cGameMode;
	m_uiWinCondition		= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.uiSelectWinCondition;
	m_uiPlayTimeSec			= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.uiSelectPlayTimeSec;
	m_uiGameModeTableID		= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.uiGameModeTableID;
	m_cMinLevel				= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cMinLevel;
	m_cMaxLevel				= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cMaxLevel;
	m_unRoomOptionBit		= pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.unRoomOptionBit;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	m_eRoomType = static_cast<PvPCommon::RoomType::eRoomType>(pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomType);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

	WCHAR wszBuf[MAX_PATH];
	memset( &wszBuf, 0, sizeof(WCHAR)*MAX_PATH );
	_wcscpy( wszBuf, _countof(wszBuf), pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.wszBuf, pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomNameLen );
	m_wstrRoomName	= wszBuf;

	if( pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomPWLen > 0 )
	{
		memset( &wszBuf, 0, sizeof(WCHAR)*MAX_PATH );
		_wcscpy( wszBuf, _countof(wszBuf), pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.wszBuf+pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomNameLen, pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomPWLen );
		m_wstrRoomPW = wszBuf;
	}
	else
		m_wstrRoomPW.clear();

	// Send packet
	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
				pUserObj->SendPvPModifyRoom( pPacket->nRetCode, &pPacket->sCSPVP_MODIFYROOM );
		}
	}
	else
		_DANGER_POINT();
}

int CDNPvPRoom::LeaveUser( const UINT uiLeaveAccountDBID, PvPCommon::LeaveType::eLeaveType Type, const short nRetCode )
{
	UINT uiLeaveSessionID = 0;
	UINT nVoiceChannelID = 0;

	std::map<UINT,SPvPUserInfo>::iterator itor = m_mUserInfo.find( uiLeaveAccountDBID );
	if( itor != m_mUserInfo.end() )
	{
		uiLeaveSessionID = (*itor).second.uiSessionID;

		switch ((*itor).second.usTeam)
		{
		case PvPCommon::Team::A: nVoiceChannelID = m_nVoiceChannelID[PvPCommon::TeamIndex::A]; break;
		case PvPCommon::Team::B: nVoiceChannelID = m_nVoiceChannelID[PvPCommon::TeamIndex::B]; break;
		}
	}
	else
		_DANGER_POINT();

	for( itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; )
	{
		CDNUserSession* pUserObj	= g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
		if( pUserObj )
		{
			pUserObj->SendPvPLeaveRoom( nRetCode, Type, uiLeaveSessionID );
		}
		
		if( uiLeaveAccountDBID == (*itor).second.uiAccountDBID )
		{
			if( pUserObj )
			{
				pUserObj->SetPvPIndex( 0 );
				pUserObj->SetPvPRoomListRefreshTime( 0 );	// Reset refresh time
			}

			if( nRetCode == ERROR_NONE )
				itor = m_mUserInfo.erase( itor );
			else
				++itor;
		}
		else
		{
			++itor;
		}
	}

	return ERROR_NONE;
}

void CDNPvPRoom::_UpdatePvPLevel( CDNUserSession* pUserSession, std::map<UINT,SPvPUserInfo>::iterator itor )
{
	BYTE cPrevPvPLevel = (*itor).second.cPvPLevel;
	if( cPrevPvPLevel == pUserSession->GetPvPData()->cLevel )
		return;
	
	(*itor).second.cPvPLevel = pUserSession->GetPvPData()->cLevel;

	SCPVP_LEVEL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID	= pUserSession->GetSessionID();
	TxPacket.cLevel			= (*itor).second.cPvPLevel;

	_BroadCast( SC_PVP, ePvP::SC_LEVEL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket), pUserSession );
}

void CDNPvPRoom::JoinUser( const MAVIPVP_JOINROOM* pPacket )
{
	// Exception handling when returning from game server to village server
	std::map<UINT,SPvPUserInfo>::iterator itor =  m_mUserInfo.find( pPacket->uiAccountDBID );
	if( itor != m_mUserInfo.end() )
	{
		if( g_pUserSessionManager )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( pPacket->uiAccountDBID );
			if( pUserObj )
			{
				// PvP settings
				pUserObj->SetPvPIndex( m_uiIndex );
				// Update PvP level since it can change
				_UpdatePvPLevel( pUserObj, itor );
				// Send			
				pUserObj->SendPvPJoinRoom( pPacket->nRetCode, this );
			}
			else
				_DANGER_POINT();
		}
		else
			_DANGER_POINT();
		return;
	}

	if( g_pUserSessionManager )
	{
		CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( pPacket->uiAccountDBID );
		if( pUserObj && pUserObj->GetCharacterName() )
		{
			SPvPUserInfo sPvPUserInfo;
			sPvPUserInfo.uiAccountDBID	= pPacket->uiAccountDBID;
			sPvPUserInfo.usTeam			= pPacket->unPvPTeam;
			sPvPUserInfo.cSlotIndex = pPacket->cIndex;
			sPvPUserInfo.uiUserState	= pPacket->uiUserState;
			sPvPUserInfo.uiSessionID	= pUserObj->GetSessionID();
			sPvPUserInfo.iJobID			= pUserObj->GetUserJob();
			sPvPUserInfo.cLevel			= pUserObj->GetLevel();
			sPvPUserInfo.cPvPLevel		= pUserObj->GetPvPData()->cLevel;
			_wcscpy( sPvPUserInfo.wszCharName, NAMELENMAX, pUserObj->GetCharacterName(), static_cast<int>(wcslen(pUserObj->GetCharacterName())) );

			m_mUserInfo.insert( std::make_pair(pPacket->uiAccountDBID,sPvPUserInfo) );
			// PvP settings
			pUserObj->SetPvPIndex( m_uiIndex );
			// Send
			pUserObj->SendPvPJoinRoom( pPacket->nRetCode, this );
			_SendJoinUser( pUserObj, sPvPUserInfo );
		}
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::ChangeTeam( const MAVIPVP_CHANGETEAM* pPacket )
{
	std::map<UINT,SPvPUserInfo>::iterator itor = m_mUserInfo.find( pPacket->sVIMAPVP_CHANGETEAM.uiAccountDBID );
	if( itor == m_mUserInfo.end() )
	{
		_DANGER_POINT();
		return;
	}

	// Change team
	(*itor).second.usTeam = pPacket->sVIMAPVP_CHANGETEAM.sCSPVP_CHANGETEAM.usTeam;
	(*itor).second.cSlotIndex = pPacket->cTeamSlotIndex;

	// Send
	_SendChangeTeam( ERROR_NONE, (*itor).second.uiSessionID, (*itor).second.usTeam, (*itor).second.cSlotIndex );
}

void CDNPvPRoom::ChangeUserState( const MAVIPVP_CHANGEUSERSTATE* pPacket )
{
#if defined( PRE_WORLDCOMBINE_PVP )
	if( m_nWorldPvPRoomType )
	{
		if( g_pUserSessionManager )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( pPacket->uiAccountDBID );
			if( pUserObj )
			{
				SCPVP_USERSTATE TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.uiSessionID = pUserObj->GetSessionID();
				TxPacket.uiUserState = pPacket->uiUserState;
				pUserObj->Send( SC_PVP, ePvP::SC_USERSTATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
			}
		}		
		return;
	}
#endif
	std::map<UINT,SPvPUserInfo>::iterator itor = m_mUserInfo.find( pPacket->uiAccountDBID );
	if( itor == m_mUserInfo.end() )
	{
		_DANGER_POINT();
		return;
	}

	(*itor).second.uiUserState = pPacket->uiUserState;

	_SendUserState( (*itor).second.uiSessionID, (*itor).second.uiUserState );
}

UINT CDNPvPRoom::GetObserverUser()
{
	UINT uiCount = 0;
	for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
	{
		if( (*itor).second.usTeam == PvPCommon::Team::Observer )
			++uiCount;
	}
	
	return uiCount;
}

WCHAR* CDNPvPRoom::MakeRoomInfoList( CDNUserSendManager* pUserSendManager, PvPCommon::RoomInfoList* pDest, int iCountOffset, WCHAR* pwOffset )
{
	pDest->uiPvPIndex[iCountOffset]			= m_uiIndex;
	pDest->uiMapIndex[iCountOffset]			= m_uiMapIndex;
	pDest->uiGameModeTableID[iCountOffset]	= m_uiGameModeTableID;
	pDest->cRoomState[iCountOffset]			= m_uiRoomState;
	pDest->cRoomNameLen[iCountOffset]		= static_cast<BYTE>(m_wstrRoomName.size());
	pDest->cCurUserCount[iCountOffset]		= static_cast<BYTE>(m_mUserInfo.size()-GetObserverUser());
	pDest->cMaxUserCount[iCountOffset]		= m_uiMaxUser;
	pDest->unWinCondition[iCountOffset]		= static_cast<USHORT>(m_uiWinCondition);
	pDest->cMinLevel[iCountOffset]			= m_cMinLevel;
	pDest->cMaxLevel[iCountOffset]			= m_cMaxLevel;
	pDest->unRoomOptionBit[iCountOffset]	= m_unRoomOptionBit;
	pDest->nEventID[iCountOffset]			= GetEventRoomIndex();
	if (IsGuildWarSystem())
	{
		for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
			pDest->nGuildDBID[iCountOffset][i] = m_nGuildWarDBID[i];
	}
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	pDest->cRoomType[iCountOffset] = static_cast<BYTE>(m_eRoomType);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPOBSERVER
	pDest->bExtendObserver[iCountOffset] = m_bExtendObserverCount;
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_WORLDCOMBINE_PVP )
	if(m_nWorldPvPRoomType)
	{
		pDest->nWorldPvPRoomType[iCountOffset] = m_nWorldPvPRoomType;
		pDest->cCurUserCount[iCountOffset] = m_cWorldPvPRoomCurUserCount;
	}	
#endif
	
#if defined( PRE_PVP_GAMBLEROOM )
	pDest->cGambleType[iCountOffset] = m_cGambleType;
	pDest->nGamblePrice[iCountOffset] = m_nGamblePrice;
#endif
	if( !m_wstrRoomName.empty() )
	{
		memcpy( pwOffset, m_wstrRoomName.c_str(), m_wstrRoomName.size()*sizeof(WCHAR) );
		pwOffset += m_wstrRoomName.size();
	}

	return pwOffset;
}

bool CDNPvPRoom::bMakeJoinRoom( SCPVP_JOINROOM* pDest, int& riSize )
{
	_MakeRoomInfo( &pDest->RoomInfo );

	pDest->cUserCount = 0;

	WCHAR* pwOffset = pDest->UserInfoList.wszBuffer;

	for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
	{
		pDest->UserInfoList.uiSessionID[pDest->cUserCount]			= (*itor).second.uiSessionID;
		pDest->UserInfoList.iJobID[pDest->cUserCount]				= (*itor).second.iJobID;
		pDest->UserInfoList.cLevel[pDest->cUserCount]				= (*itor).second.cLevel;
		pDest->UserInfoList.cPvPLevel[pDest->cUserCount]			= (*itor).second.cPvPLevel;
		pDest->UserInfoList.usTeam[pDest->cUserCount]				= (*itor).second.usTeam;
		pDest->UserInfoList.cTeamIndex[pDest->cUserCount]				= (*itor).second.cSlotIndex;
		pDest->UserInfoList.uiUserState[pDest->cUserCount]			= (*itor).second.uiUserState;
		pDest->UserInfoList.cCharacterNameLen[pDest->cUserCount]	= static_cast<BYTE>(wcslen((*itor).second.wszCharName));
		memcpy( pwOffset, (*itor).second.wszCharName, sizeof(WCHAR)*pDest->UserInfoList.cCharacterNameLen[pDest->cUserCount] );
		pwOffset += pDest->UserInfoList.cCharacterNameLen[pDest->cUserCount];

		++pDest->cUserCount;
	}

	riSize = static_cast<int>(sizeof(SCPVP_JOINROOM)-sizeof(pDest->UserInfoList.wszBuffer)+(sizeof(WCHAR)*(pwOffset-pDest->UserInfoList.wszBuffer)));
	return true;
}

void CDNPvPRoom::StartPvP( const MAVIPVP_START* pPacket )
{
	SCPVP_START TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = pPacket->nRetCode;

	UINT uiRoomState = m_uiRoomState&~PvPCommon::RoomState::Password;
	if( uiRoomState == PvPCommon::RoomState::None )
	{
		_BroadCast( SC_PVP, ePvP::SC_START, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	}
	else
	{
		if( g_pUserSessionManager )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( pPacket->uiAccountDBID );
			if( pUserObj )
				pUserObj->SendPvPStart( pPacket->nRetCode );
			else
				_DANGER_POINT();
		}
		else
			_DANGER_POINT();
	}	
}

void CDNPvPRoom::StartPvPMsg( const MAVIPVP_STARTMSG* pPacket )
{
	SCPVP_STARTMSG TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cSec = pPacket->cSec;

	_BroadCast( SC_PVP, ePvP::SC_STARTMSG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNPvPRoom::SendAllCancelStage( short cRetCode )
{
	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
			{
				pUserObj->m_cGateNo = -1;
				pUserObj->m_cGateSelect = -1;
				pUserObj->m_eUserState = STATE_NONE;

				pUserObj->SendPvPStart( cRetCode );
			}
			else
				_DANGER_POINT();
		}
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::SendAllRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, USHORT wGameID, int nRoomID )
{
	m_iGameServerID	= wGameID;
	m_iGameRoomID	= nRoomID;

	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
			{
				if( (*itor).second.uiUserState&PvPCommon::UserState::Syncing )
				{
					pUserObj->m_GameTaskType = GameTaskType;
					pUserObj->m_cReqGameIDType	= cReqGameIDType;
					pUserObj->SetGameID(wGameID);
					pUserObj->SetRoomID(nRoomID);
				}
			}
			else
				_DANGER_POINT();
		}
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::SendAllReadyToGame( ULONG nIP, USHORT nPort, USHORT nTcpPort )
{
	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
			{
				if( (*itor).second.uiUserState&PvPCommon::UserState::Syncing )
				{
					pUserObj->m_nGameServerIP		= nIP;
					pUserObj->m_nGameServerPort		= nPort;
					pUserObj->m_nGameServerTcpPort	= nTcpPort;
					pUserObj->m_eUserState			= STATE_READYTOGAME;

					pUserObj->m_biCertifyingKey = g_pAuthManager->GetCertifyingKey();
					DN_ASSERT(0 != pUserObj->m_biCertifyingKey,	"Invalid!");	// Auth key must not be generated as 0! (0 means none)

					g_pAuthManager->QueryStoreAuth(SERVERTYPE_VILLAGE, pUserObj);
				}
			}
			else
				_DANGER_POINT();
		}
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::SendChat( eChatType eType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, int nRet )
{
	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
				pUserObj->SendChat( eType, cLen, pwszCharacterName, pwszChatMsg, NULL, nRet );
			//else
			//	_DANGER_POINT();
		}
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::SendMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex, int nRetCode)
{
	if( g_pUserSessionManager )
	{
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{			
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
			{
				//change index
				for (int i = 0; i < cCount && i < PARTYMAX; i++)
				{
					if (pIndex[i].nSessionID == pUserObj->GetSessionID())
					{
						(*itor).second.cSlotIndex = pIndex[i].cIndex;
						break;
					}
				}

				pUserObj->SendPvPMemberIndex(nTeam, cCount, pIndex, nRetCode);
			}
		}
	}
	else
		_DANGER_POINT();
}

#if defined(PRE_ADD_PVP_TOURNAMENT)
void CDNPvPRoom::SendPvPSwapTounamentIndex(char cSourceIndex, char cDestIndex)
{
	if( g_pUserSessionManager )
	{		
		for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
		{
			if( (*itor).second.cSlotIndex == cSourceIndex )
				(*itor).second.cSlotIndex = cDestIndex;
			else if( (*itor).second.cSlotIndex == cDestIndex )
				(*itor).second.cSlotIndex = cSourceIndex;

			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )			
				pUserObj->SendPvPSwapTournamentIndex(ERROR_NONE, cSourceIndex, cDestIndex);
		}		
	}
	else
		_DANGER_POINT();
}

void CDNPvPRoom::SetPvPShuffleIndex(MAVIPVPShuffleTournamentIndex* pPacket)
{
	int nCount = 0;
	for( char i=0; i<pPacket->nCount; ++i)
	{
		if( pPacket->uiAccountDBID[i] > 0)
		{
			std::map<UINT,SPvPUserInfo>::iterator itor = m_mUserInfo.find( pPacket->uiAccountDBID[i] );
			if( itor != m_mUserInfo.end() )			
				(*itor).second.cSlotIndex = i;			
		}
	}
}
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

void CDNPvPRoom::_MakeRoomInfo( PvPCommon::RoomInfo* pDest )
{
	pDest->uiIndex				= m_uiIndex;
	pDest->uiMapIndex			= m_uiMapIndex;
	pDest->uiGameModeTableID	= m_uiGameModeTableID;
	pDest->uiWinCondition		= m_uiWinCondition;
	pDest->uiPlayTimeSec		= m_uiPlayTimeSec;
	pDest->cMaxUserCount		= m_uiMaxUser;
	pDest->cRoomState			= m_uiRoomState;
	pDest->cRoomNameLen			= static_cast<BYTE>(m_wstrRoomName.size());
	pDest->cMinLevel			= m_cMinLevel;
	pDest->cMaxLevel			= m_cMaxLevel;
	pDest->unRoomOptionBit		= m_unRoomOptionBit;
	pDest->nEventID				= m_nEventRoomIndex;
	pDest->bIsGuildWarSystem = m_bIsGuildWarSystem;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	pDest->cRoomType = static_cast<BYTE>(m_eRoomType);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPOBSERVER
	pDest->bExtendObserver = m_bExtendObserverCount;
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_WORLDCOMBINE_PVP )
	pDest->nWorldPvPRoomType = m_nWorldPvPRoomType;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	pDest->cGambleType = m_cGambleType;
	pDest->nGamblePrice = m_nGamblePrice;
#endif
	_wcscpy( pDest->wszBuffer, _countof(pDest->wszBuffer), m_wstrRoomName.c_str(), pDest->cRoomNameLen );
}

void CDNPvPRoom::_SendRoomState()
{
	SCPVP_ROOMSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiRoomState = m_uiRoomState;

	_BroadCast( SC_PVP, ePvP::SC_ROOMSTATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNPvPRoom::_SendJoinUser( CDNUserSession* pUserObj, const SPvPUserInfo& sPvPUserInfo )
{
	SCPVP_JOINUSER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.UserInfo.uiSessionID		= sPvPUserInfo.uiSessionID;
	TxPacket.UserInfo.iJobID			= sPvPUserInfo.iJobID;
	TxPacket.UserInfo.cLevel			= sPvPUserInfo.cLevel;
	TxPacket.UserInfo.cPvPLevel			= sPvPUserInfo.cPvPLevel;
	TxPacket.UserInfo.usTeam			= sPvPUserInfo.usTeam;
	TxPacket.UserInfo.uiUserState		= sPvPUserInfo.uiUserState;
	TxPacket.UserInfo.cSlotIndex = sPvPUserInfo.cSlotIndex;
	TxPacket.UserInfo.cCharacterNameLen	= static_cast<BYTE>(wcslen(sPvPUserInfo.wszCharName));
	memcpy( TxPacket.UserInfo.wszCharacterName, sPvPUserInfo.wszCharName, sizeof(WCHAR)*TxPacket.UserInfo.cCharacterNameLen );

	int iLen = static_cast<int>(sizeof(SCPVP_JOINUSER)-sizeof(TxPacket.UserInfo.wszCharacterName)+TxPacket.UserInfo.cCharacterNameLen*sizeof(WCHAR));
	_BroadCast( SC_PVP, ePvP::SC_JOINUSER, reinterpret_cast<char*>(&TxPacket), iLen, pUserObj );
}

void CDNPvPRoom::_SendChangeTeam( const short nRetCode, const UINT uiSessionID, const USHORT usTeam, char cTeamSlotIndex )
{
	SCPVP_CHANGETEAM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode		= nRetCode;
	TxPacket.uiSessionID	= uiSessionID;
	TxPacket.usTeam			= usTeam;
	TxPacket.cTeamSlotIndex = cTeamSlotIndex;

	_BroadCast( SC_PVP, ePvP::SC_CHANGETEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNPvPRoom::_SendUserState( const UINT uiSessionID, const UINT uiUserState )
{
	SCPVP_USERSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID	= uiSessionID;
	TxPacket.uiUserState	= uiUserState;

	_BroadCast( SC_PVP, ePvP::SC_USERSTATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNPvPRoom::_BroadCast( int iMainCmd, int iSubCmd, char* pData, int iLen, CDNUserSession* pExceptUserObject )
{
	for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
	{
		if( g_pUserSessionManager )
		{
			CDNUserSession* pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID( (*itor).second.uiAccountDBID );
			if( pUserObj )
			{
				// Excluded user
				if( pExceptUserObject == pUserObj )
					continue;

				pUserObj->Send( iMainCmd, iSubCmd, pData, iLen );
			}
		}
	}
}

void CDNPvPRoom::OnMasterServerDisconnected()
{
	for( std::map<UINT,SPvPUserInfo>::iterator itor=m_mUserInfo.begin() ; itor!=m_mUserInfo.end() ; ++itor )
	{
		LeaveUser( (*itor).second.uiAccountDBID, PvPCommon::LeaveType::Normal, ERROR_PVP_FORCELEAVEROOM_MASTERSERVER_DOWN );
	}
}

#if defined( PRE_WORLDCOMBINE_PVP )
bool CDNPvPRoom::CheckLevel( BYTE cLevel )
{
	if( cLevel >= m_cMinLevel && cLevel <= m_cMaxLevel )
		return true;
	return false;
}
#endif

#if defined( PRE_PVP_GAMBLEROOM )
int CDNPvPRoom::CheckGambleRoomJoin( CDNUserSession* pUserSession )
{	
	if( pUserSession )
	{
		if( m_cGambleType == PvPGambleRoom::Gold )
		{
			// Since it is gold, convert to coin and check
			int nCoin = m_nGamblePrice * 10000;
			if( nCoin <= pUserSession->GetCoin() )
				return ERROR_NONE;
			else
				return ERROR_PVP_GAMBLEROOM_NOTCOIN;
		}
		else if( m_cGambleType == PvPGambleRoom::Petal )
		{
			if( m_nGamblePrice <= pUserSession->GetPetal() )
				return ERROR_NONE;
			else
				return ERROR_PVP_GAMBLEROOM_NOTPETAL;
		}
		else
			return ERROR_NONE;
	}
	return ERROR_GENERIC_UNKNOWNERROR;
}
#endif