#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"



inline void SendPvPCreateRoom( const UINT uiMapIndex, 
							  const BYTE cMaxUser, 
							  const bool bIsBreakInto, 
							  const UINT uiGameModeTableID, 
							  const UINT uiWinCondition, 
							  const UINT uiPlayTimeSec, 
							  const BYTE cRoomNameLen, 
							  const BYTE cRoomPWLen, 
							  const WCHAR* pwszRoomName ,
							  const WCHAR* pwszRoomPW, 
							  const bool bDropItem ,
							  const bool bShowHp ,
							  const bool bRandomTeamMode ,
							  const bool bRevision, 
							  const bool bRandomOrder,
#ifdef PRE_MOD_PVPOBSERVER
							  const bool bObserverAccess,	  	
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
							  const char cRoomType,
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_WORLDCOMBINE_PVP
							  const int nWorldPvPRoomType,
#endif // PRE_WORLDCOMBINE_PVP
							  const BYTE uiMinLevel, const BYTE uiMaxLevel )
{
	_ASSERT( cMaxUser > 0 && cMaxUser <= PvPCommon::Common::MaxPlayer && "PvPVSMode Invalid" );
	_ASSERT( cRoomNameLen <= PvPCommon::TxtMax::RoomName && cRoomNameLen > 0 && "PvPRoomNameLen Invalid" );
	_ASSERT( cRoomPWLen <= PvPCommon::TxtMax::RoomPW && "PvPRoomPWLen Invalid" );
	_ASSERT( uiGameModeTableID > 0 && "PvPGameModeTableID Invalid" );
	_ASSERT( pwszRoomName && "PvPRoomName Invalid" );
	if( cRoomPWLen )
		_ASSERT( pwszRoomPW && "PvPRoomPW Invalid" );
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	_ASSERT( static_cast<char>(PvPCommon::RoomType::eRoomType::beginner) <= cRoomType && 
			 cRoomType < static_cast<char>(PvPCommon::RoomType::eRoomType::max) &&
			 "cRoomType Invalid" );
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	FUNC_LOG();
	CSPVP_CREATEROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiMapIndex				= uiMapIndex;
	TxPacket.uiGameModeTableID		= uiGameModeTableID;
	TxPacket.uiSelectWinCondition	= uiWinCondition;
	TxPacket.uiSelectPlayTimeSec	= uiPlayTimeSec;
	TxPacket.cMaxUser				= cMaxUser;
	TxPacket.cRoomNameLen			= cRoomNameLen;
	TxPacket.cRoomPWLen				= cRoomPWLen;

	TxPacket.unRoomOptionBit |= bIsBreakInto ? PvPCommon::RoomOption::BreakInto : 0;
	TxPacket.unRoomOptionBit |= bDropItem ? PvPCommon::RoomOption::DropItem : 0;
	TxPacket.unRoomOptionBit |= bShowHp ? PvPCommon::RoomOption::ShowHP : 0;
	TxPacket.unRoomOptionBit |= bRandomTeamMode ? PvPCommon::RoomOption::RandomTeam : 0;
	TxPacket.unRoomOptionBit |= bRevision ? PvPCommon::RoomOption::NoRegulation : 0;

	TxPacket.unRoomOptionBit |= bRandomOrder ? PvPCommon::RoomOption::AllKill_RandomOrder : 0;
#ifdef PRE_MOD_PVPOBSERVER
	TxPacket.unRoomOptionBit |= bObserverAccess ? PvPCommon::RoomOption::AllowObserver : 0;	  	
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
	TxPacket.nWorldPvPRoomType = nWorldPvPRoomType;
#endif // PRE_WORLDCOMBINE_PVP
	TxPacket.cMaxLevel              = uiMaxLevel;
	TxPacket.cMinLevel              = uiMinLevel; // Rotha: Add PVP level limit

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	TxPacket.cRoomType				= cRoomType;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	memcpy( TxPacket.wszBuf, pwszRoomName, sizeof(WCHAR)*cRoomNameLen );
	if( cRoomPWLen )
		memcpy( TxPacket.wszBuf+cRoomNameLen, pwszRoomPW, sizeof(WCHAR)*cRoomPWLen );
	
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_CREATEROOM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket)-sizeof(TxPacket.wszBuf)+sizeof(WCHAR)*(cRoomNameLen+cRoomPWLen) );
}

inline void SendPvPModifyRoom( const UINT uiMapIndex, 
							  const BYTE cMaxUser, 
							  const bool bIsBreakInto, 
							  const UINT uiGameModeTableID, 
							  const UINT uiWinCondition, 
							  const UINT uiPlayTimeSec, 
							  const BYTE cRoomNameLen, 
							  const BYTE cRoomPWLen, 
							  const WCHAR* pwszRoomName, 
                              const WCHAR* pwszRoomPW, 
							  const bool bDropItem , 
							  const bool bShowHp ,
							  const bool bRandomTeamMode,
							  const bool bRevision ,
							  const bool bRandomOrder,
#ifdef PRE_MOD_PVPOBSERVER
							  const bool bObserverAccess,
							  const bool bExtendObserver,
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
							  const char cRoomType,
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
							  const BYTE uiMinLevel, const BYTE uiMaxLevel)
							  

{
	_ASSERT( cMaxUser > 0 && cMaxUser <= PvPCommon::Common::MaxPlayer && "PvPVSMode Invalid" );
	_ASSERT( cRoomNameLen <= PvPCommon::TxtMax::RoomName && cRoomNameLen > 0 && "PvPRoomNameLen Invalid" );
	_ASSERT( cRoomPWLen <= PvPCommon::TxtMax::RoomPW && "PvPRoomPWLen Invalid" );
	_ASSERT( uiGameModeTableID > 0 && "PvPGameModeTableID Invalid" );
	_ASSERT( pwszRoomName && "PvPRoomName Invalid" );
	if( cRoomPWLen )
		_ASSERT( pwszRoomPW && "PvPRoomPW Invalid" );
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	_ASSERT( static_cast<char>(PvPCommon::RoomType::eRoomType::beginner) <= cRoomType && 
		cRoomType < static_cast<char>(PvPCommon::RoomType::eRoomType::max) &&
		"cRoomType Invalid" );
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	FUNC_LOG();
	CSPVP_MODIFYROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.sCSPVP_CREATEROOM.uiMapIndex			= uiMapIndex;
	TxPacket.sCSPVP_CREATEROOM.uiGameModeTableID	= uiGameModeTableID;
	TxPacket.sCSPVP_CREATEROOM.uiSelectWinCondition	= uiWinCondition;
	TxPacket.sCSPVP_CREATEROOM.uiSelectPlayTimeSec	= uiPlayTimeSec;
	TxPacket.sCSPVP_CREATEROOM.cMaxUser				= cMaxUser;
	TxPacket.sCSPVP_CREATEROOM.cRoomNameLen			= cRoomNameLen;
	TxPacket.sCSPVP_CREATEROOM.cRoomPWLen			= cRoomPWLen;
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bIsBreakInto ? PvPCommon::RoomOption::BreakInto : 0;
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bDropItem ? PvPCommon::RoomOption::DropItem : 0;
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bShowHp ? PvPCommon::RoomOption::ShowHP : 0;
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bRandomTeamMode ? PvPCommon::RoomOption::RandomTeam : 0;
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bRevision ? PvPCommon::RoomOption::NoRegulation : 0;	
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bRandomOrder ? PvPCommon::RoomOption::AllKill_RandomOrder : 0;	
#ifdef PRE_MOD_PVPOBSERVER
	TxPacket.sCSPVP_CREATEROOM.unRoomOptionBit |= bObserverAccess ? PvPCommon::RoomOption::AllowObserver : 0;	  	
	TxPacket.sCSPVP_CREATEROOM.bExtendObserver = bExtendObserver;
#endif // PRE_MOD_PVPOBSERVER
	TxPacket.sCSPVP_CREATEROOM.cMaxLevel            = uiMaxLevel;
	TxPacket.sCSPVP_CREATEROOM.cMinLevel            = uiMinLevel;

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	TxPacket.sCSPVP_CREATEROOM.cRoomType			= cRoomType;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	memcpy( TxPacket.sCSPVP_CREATEROOM.wszBuf, pwszRoomName, sizeof(WCHAR)*cRoomNameLen );
	if( cRoomPWLen )
		memcpy( TxPacket.sCSPVP_CREATEROOM.wszBuf+cRoomNameLen, pwszRoomPW, sizeof(WCHAR)*cRoomPWLen );

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_MODIFYROOM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket)-sizeof(TxPacket.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(cRoomNameLen+cRoomPWLen) );
}

inline void SendPvPLeaveRoom()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LEAVEROOM, NULL, 0 );
}

inline void SendPvPChangeCaptain( const UINT uiNewCaptainSessionID , PvPCommon::CaptainType::eCode eCaptainType )
{
	FUNC_LOG();
	CSPVP_CHANGECAPTAIN TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiNewCaptainSessionID = uiNewCaptainSessionID;
	TxPacket.Type = eCaptainType;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_CHANGECAPTAIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPRoomList( const UINT uiPage, const BYTE cRoomSortType=PvPCommon::RoomSortType::Index, const bool bIsAscend=true, const UINT uiSortData=0 )
{
	_ASSERT( cRoomSortType < PvPCommon::RoomSortType::Max && "RoomSortType Invalid" );

	FUNC_LOG();
	CSPVP_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiPage			= uiPage;
	TxPacket.cRoomSortType	= cRoomSortType;
	TxPacket.cIsAscend		= bIsAscend ? 1: 0;
	TxPacket.uiSortData		= uiSortData;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_ROOMLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPWaitUserList( const UINT uiPage, const PvPCommon::WaitUserList::SortType::eSortType cUserSortType = PvPCommon::WaitUserList::SortType::eSortType::Level, const bool bIsAscend=true)
{
	_ASSERT( cUserSortType < PvPCommon::WaitUserList::SortType::MAX && "RoomSortType Invalid" );

	FUNC_LOG();

	CSPVP_WAITUSERLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsAscend			=  bIsAscend ? 1: 0;
	TxPacket.unPage         = uiPage;
	TxPacket.SortType       = cUserSortType; // Temporary value

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_WAITUSERLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
inline void SendPVPChangeChannel( const PvPCommon::RoomType::eRoomType eSelectedRoomType )
{
	_ASSERT( eSelectedRoomType < PvPCommon::RoomType::eRoomType::max );

	FUNC_LOG();

	CSPVP_CHANGECHANNEL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cType = (const char)( eSelectedRoomType );

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_PVP_CHANGECHANNEL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

inline void SendPvPEnterChannel(LadderSystem::MatchType::eCode eMatchType)
{
	_ASSERT( (eMatchType == LadderSystem::MatchType::eCode::None )&& "RoomSortType Invalid" );
	FUNC_LOG();

	LadderSystem::CS_ENTER_CHANNEL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.MatchType = eMatchType;
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_ENTER_LADDERCHANNEL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPLeaveChannel()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LEAVE_LADDERCHANNEL, NULL , 0);
}

inline void SendPvPSearchLadder(bool bCancel)
{
	FUNC_LOG();

	LadderSystem::CS_LADDER_MATCHING TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.bIsCancel = bCancel;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_MATCHING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendRefreshLadderRoomList(UINT nPage)
{
	FUNC_LOG();

	LadderSystem::CS_PLAYING_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.uiReqPage = nPage;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_PLAYING_ROOMLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendLadderObserverJoin(std::wstring wszName)
{
	FUNC_LOG();

	LadderSystem::CS_OBSERVER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	wsprintf(TxPacket.wszCharName, wszName.c_str());

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_OBSERVER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendLadderInviteUser(std::wstring wszName)
{
	FUNC_LOG();

	LadderSystem::CS_INVITE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	wsprintf(TxPacket.wszCharName, wszName.c_str());

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_INVITE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendLadderInviteConfirm(std::wstring wszName , bool bAccept)
{
	FUNC_LOG();

	LadderSystem::CS_INVITE_CONFIRM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	wsprintf(TxPacket.wszCharName, wszName.c_str());
	TxPacket.bAccept = bAccept;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_INVITE_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendLadderKickUser(INT64 nDBID)
{
	FUNC_LOG();

	LadderSystem::CS_KICKOUT TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.biCharacterDBID = nDBID;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_LADDER_KICKOUT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPJoinRoom( const UINT uiPvPIndex, const BYTE cRoomPWLen, const WCHAR* pwszRoomPW ,bool bObserver = false)
{
	_ASSERT( cRoomPWLen <= PvPCommon::TxtMax::RoomPW && "PvPRoomPWLen Invalid" );
	if( PvPCommon::TxtMax::RoomPW < cRoomPWLen )
		return;

	if( cRoomPWLen )
		_ASSERT( pwszRoomPW && "PvPRoomPW Invalid" );

	FUNC_LOG();
	CSPVP_JOINROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiPvPIndex = uiPvPIndex;
	TxPacket.cRoomPWLen	= cRoomPWLen;

	TxPacket.bIsObserver = bObserver;

	if( cRoomPWLen )
		memcpy( TxPacket.wszRoomPW, pwszRoomPW, sizeof(WCHAR)*cRoomPWLen );

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_JOINROOM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket)-sizeof(TxPacket.wszRoomPW)+sizeof(WCHAR)*cRoomPWLen  );
}

inline void SendPvPReady( const BYTE cReady )
{
	FUNC_LOG();
	CSPVP_READY TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cReady = cReady;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_READY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPStart( const UINT unCheck )
{
	FUNC_LOG();
	CSPVP_START TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.unCheck = unCheck;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_START, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPRandomJoinRoom()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_RANDOMJOINROOM, NULL, 0 );
}

inline void SendPvPChangeTeam( const USHORT usTeam )
{
	_ASSERT( PvPCommon::CheckTeam( usTeam ) && "PvPTeam Invalid" );

	FUNC_LOG();
	CSPVP_CHANGETEAM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.usTeam = usTeam;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_CHANGETEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPBan( const UINT uiSessionID, PvPCommon::BanType::eBanType eType )
{
	FUNC_LOG();
	CSPVP_BAN TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID = uiSessionID;
	TxPacket.eType = eType;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_BAN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendPvPFatigueOption( const bool bFatigue)
{
	FUNC_LOG();
	CSPVP_FATIGUE_OPTION TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bOption = bFatigue;
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_FATIGUE_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendSwapMemberIndex(BYTE cCount, const BYTE * pIndex, const UINT * pSessionID)
{
	FUNC_LOG();
	if (pIndex == NULL || pSessionID == NULL) return;

	CSPvPTeamSwapMemberIndex packet;
	memset(&packet, 0, sizeof(CSPvPTeamSwapMemberIndex));

	packet.cCount = cCount;
	for (int i = 0; i < cCount; i++)
	{
		packet.Index[i].cIndex = pIndex[i];
		packet.Index[i].nSessionID = pSessionID[i];
	}
		
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_SWAPMEMBERINDEX, reinterpret_cast<char*>(&packet), sizeof(packet) - sizeof(packet.Index) + (sizeof(TSwapMemberIndex) * packet.cCount));
}

inline void SendChangeMemberGrade(USHORT nType, UINT nTargetSessionID, bool bAsign)
{
	FUNC_LOG();
	if (nType != PvPCommon::UserState::GuildWarCaptain && nType != PvPCommon::UserState::GuildWarSedcondCaptain)
	{
		_ASSERT(0);
		return;
	}

	CSPvPGuildWarChangeMemberGrade packet;
	memset(&packet, 0, sizeof(CSPvPGuildWarChangeMemberGrade));

	packet.nType = nType;
	packet.nSessionID = nTargetSessionID;
	packet.bAsign = bAsign;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_GUILDWAR_CHANGEMEMBER_GRADE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

inline void SendGuildWarConcentrateOrder( EtVector3 vPosition )
{
	FUNC_LOG();
	CSOrderConcentrate packet;
	memset(&packet, 0, sizeof(CSOrderConcentrate));
	
	packet.vPosition = vPosition;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_CONCENTRATE_ORDER, reinterpret_cast<char*>(&packet), sizeof(packet));
}

inline void SendTryAcquirePoint( int nAreaID )
{
	FUNC_LOG();
	CSPvPTryAcquirePoint packet;
	memset(&packet, 0, sizeof(CSPvPTryAcquirePoint));

	packet.nAreaID = nAreaID;
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_TRYCAPTURE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

inline void SendTryAcquirePointCancle()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_CANCELCAPTURE, NULL, 0 );
}

inline void SendPvPAllKillSelectPlayer( const UINT uiUSerSessionID )
{
	FUNC_LOG();

	SCPVP_ALLKILL_SELECTPLAYER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.uiSelectPlayerSessionID = uiUSerSessionID;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_PVP_ALLKILL_SELECTPLAYER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#ifdef PRE_ADD_QUICK_PVP
inline void SendQuickPvPInvite(UINT nReceiverSessionID)
{
	FUNC_LOG();

	CSQuickPvPInvite packet;
	memset(&packet, 0, sizeof(packet));
	packet.nReceiverSessionID = nReceiverSessionID;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_QUICKPVP_INVITE, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

inline void SendQuickPvPResult(bool bAccept, UINT nSenderSessionID)
{
	FUNC_LOG();

	CSQuickPvPResult packet;
	memset(&packet, 0, sizeof(packet));
	packet.bAccept = bAccept;
	packet.nSenderSessionID = nSenderSessionID;

	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_QUICKPVP_RESULT, reinterpret_cast<char*>(&packet), sizeof(packet) );
}
#endif

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
inline void SendRequestPVPVillageAcess()
{
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::CS_PVP_LIST_OPEN_UI, NULL, 0 );
}
#endif



#ifdef PRE_ADD_PVP_RANKING
// My info - Colosseum.
inline void OnSendPvPRankMyInfo()
{		
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_RANK_BOARD, NULL, 0 );
}

// My info - Ladder.
inline void OnSendPvPLadderRankMyInfo( LadderSystem::MatchType::eCode LadderType )
{	
	CSPvPLadderRankBoard2 packet;
	packet.MatchType = LadderType;
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_LADDER_RANK_BOARD, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

// Character name search - Colosseum.
inline void OnSendPvPRankCharacter( LPCWSTR strName )
{
	CSPvPRankInfo packet;
	_wcscpy( packet.wszCharName, _countof(packet.wszCharName), strName, (int)wcslen(strName) );	
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_RANK_INFO, reinterpret_cast<char*>(&packet), sizeof(packet) ); 
}


// Character name search - Ladder.
inline void OnSendPvPRankLadderCharacter( LadderSystem::MatchType::eCode LadderType, LPCWSTR strName )
{
	CSPvPLadderRankInfo packet;	
	packet.MatchType = LadderType;	
	_wcscpy( packet.wszCharName, _countof(packet.wszCharName), strName, (int)wcslen(strName) );
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_LADDER_RANK_INFO, reinterpret_cast<char*>(&packet), sizeof(packet) );
}


// Page search - Colosseum.
inline void OnSendPvPRankList( int iPage, BYTE cClassCode,	BYTE cSubClassCode,	LPCWSTR pStrGuildName )
{
	CSPvPRankList packet;
	packet.iPage = iPage;
	packet.cClassCode = cClassCode;
	packet.cSubClassCode = cSubClassCode;	
	_wcscpy( packet.wszGuildName, _countof(packet.wszGuildName), pStrGuildName, (int)wcslen(pStrGuildName) );
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_RANK_LIST, reinterpret_cast<char*>(&packet), sizeof(packet) );
	
}

// Page search - Ladder.
inline void OnSendPvPRankLadderList( LadderSystem::MatchType::eCode LadderType, int iPage, BYTE cClassCode,	BYTE cSubClassCode,	LPCWSTR pStrGuildName )
{
	CSPvPLadderRankList packet;
	packet.iPage = iPage;
	packet.cClassCode = cClassCode;
	packet.cSubClassCode = cSubClassCode;
	packet.MatchType = LadderType;	
	_wcscpy( packet.wszGuildName, _countof(packet.wszGuildName), pStrGuildName, (int)wcslen(pStrGuildName) );
	CClientSessionManager::GetInstance().SendPacket( CS_ETC, eEtc::eCSEtc::CS_PVP_LADDER_RANK_LIST, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_ADD_PVP_TOURNAMENT
inline void SendPvPTournamentSwapPosition(int srcIndex, int destIndex)
{
	CSPvPSwapTournamentIndex packet;
	packet.cSourceIndex = srcIndex;
	packet.cDestIndex = destIndex;
	CClientSessionManager::GetInstance().SendPacket(CS_PVP, ePvP::eCSPvP::CS_PVP_SWAP_TOURNAMENT_INDEX, reinterpret_cast<char*>(&packet), sizeof(packet));
}
#endif

#ifdef PRE_WORLDCOMBINE_PVP
inline void SendWorldPVPRoomStart( UINT uiPVPIndex )
{
	WorldPvPMissionRoom::CSWorldPvPRoomStartMsg packet;
	packet.uiPvPIndex = uiPVPIndex;
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::eCSPvP::CS_PVP_WORLDPVPROOM_STARTMSG, reinterpret_cast<char*>(&packet), sizeof(packet) );
}
#endif // PRE_WORLDCOMBINE_PVP


#ifdef PRE_ADD_PVP_COMBOEXERCISE
inline void SendPvPSummonMonster()
{	
	CClientSessionManager::GetInstance().SendPacket( CS_PVP, ePvP::eCSPvP::CS_PVP_COMBOEXERCISE_RECALLMONSTER, NULL, 0 );
}
#endif // PRE_ADD_PVP_COMBOEXERCISE