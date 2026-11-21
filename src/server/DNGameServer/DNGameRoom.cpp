#include "stdafx.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPvPGameTask.h"
#include "DnDLGameTask.h"
#include "DnItemTask.h"
#include "DnPartyTask.h"
#include "DnDLPartyTask.h"
#include "DNGuildTask.h"
#include "DnPvPPartyTask.h"
#include "DnSkillTask.h"
#include "DnWorld.h"
#include "PerfCheck.h"
#include "GameListener.h"
#include "DNGameServerManager.h"
#include "DNRUDPGameServer.h"
#include "DNBackGroundLoader.h"
#include "EtCollisionMng.h"
#include "GameSendPacket.h"
#include "DnSkill.h"
#include "DNUserTcpConnection.h"
#include "DNIocpManager.h"
#include "DnWorldSector.h"
#include "DnWorldActProp.h"
#include "DNLogConnection.h"
#include "DNBreakIntoUserSession.h"
#include "DnBlow.h"
#include "ExceptionReport.h"
#include "DnWeapon.h"
#include "DnParts.h"
#include "DnPlayerActor.h"
#include "NoticeSystem.h"
#if defined( STRESS_TEST )
#include "DNDBConnectionManager.h"
#endif // #if defined( STRESS_TEST )
#include "DNDBConnectionManager.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "DnTaskFactory.hpp"
#include "DNMissionSystem.h"
#include "DnMonsterActor.h"
#include "DNDBConnection.h"
#include "CloseSystem.h"
#include "DNFriend.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "ReputationSystemRepository.h"
#include "ReputationSystemEventHandler.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "MasterRewardSystem.h"
#include "DNMasterConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNFarmUserSession.h"
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriend.h"
#endif
#ifdef PRE_ADD_BEGINNERGUILD
#include "DNGuildSystem.h"
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
#include "DnChatTask.h"
#include "DNChatRoom.h"
#include "DNTimeEventSystem.h"
#include "PvPGameMode.h"
#if defined( PRE_ADD_VIP_FARM )
#include "DNCashRepository.h"
#endif // #if defined( PRE_ADD_VIP_FARM )
#include "DNGameDataManager.h"
#include "DnTableDB.h"
#include "DnItemTask.h"

CDNGameRoom::CDNGameRoom( CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket )
: CMultiRoom( iRoomID )
{
#ifdef PRE_ADD_FRAMEDELAY_LOG
	m_dwProcessElapsedTime = 0;
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
	m_bRoomCrash = false;
	m_lRoomProcessInterLocked = 0;

	// Reset all SmartPtr Managers.
	CDnActor::ReleaseClass( this );
	CDnDropItem::ReleaseClass( this );
	CDnWeapon::ReleaseClass( this );

	CDnParts::DeleteAllObject( this );
	CEtObject::DeleteAllObject( this );
	CDnBlow::DeleteAllObject( this );
	CDnSkill::DeleteAllObject( this );
	CDnWorldProp::DeleteAllObject( this );

	/////////////////////////////////////////
	m_pGameServer = pServer;
	m_iWorldID = 0;

	
#if defined( PRE_PARTY_DB )
	m_PartyStructData = pPacket->PartyData;	
#else
	memset(m_wszPartyPass, 0, sizeof(m_wszPartyPass));
	memset(m_wszPartyName, 0, sizeof(m_wszPartyName));
	_wcscpy( m_wszPartyName, _countof(m_wszPartyName), pPacket->wszPartyName, (int)wcslen(pPacket->wszPartyName) );
	m_nUserLvMin = pPacket->cUserLvLimitMin;
	m_nTargetMapIdx = pPacket->nTargetMapIdx;
	m_PartyDifficulty = pPacket->PartyDifficulty;
	m_nMemberMax = pPacket->cMemberMax;
#endif // #if defined( PRE_PARTY_DB )

	m_GameTaskType = pPacket->GameTaskType;
	m_cReqGameIDType = pPacket->cReqGameIDType;
	m_InstanceID = pPacket->InstanceID;
	m_iPartMemberCnt = pPacket->cMemberCount;
	m_iMapIdx = pPacket->nMapIndex;
#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	m_iRootMapIndex = pPacket->nMapIndex;
#endif
	m_iGateIdx = pPacket->cGateNo;
	m_iGateSelect = pPacket->cGateSelect;
	m_StageDifficulty = pPacket->StageDifficulty;
	m_iRandomSeed = pPacket->nRandomSeed;
#if defined(PRE_FIX_INITSTATEANDSYNC)	
	m_bInitStateAndSyncReserved = false;
	m_nReservedMapIdx = 0;
	m_nReservedGateIdx = 0;
	m_nReservedRandomSeed = 0;
	m_ReservedStageDifficulty = TDUNGEONDIFFICULTY::Easy; 
	m_bReservedDirectConnect = false;
	m_bReservedGateSelect = 0;
#endif
#if defined( PRE_PARTY_DB )	
#else
	m_ItemLootRule = pPacket->ItemLootRule;
	m_ItemLootRank = pPacket->ItemLootRank;
#endif
	m_nMeritBonusID = pPacket->nMeritBonusID;
	m_bDirectConnect = pPacket->bDirectConnect;
	m_nChannelID = pPacket->nChannelID;
	
#if defined( PRE_PARTY_DB )	
#else
	m_nUserLvMax = pPacket->cUserLvLimitMax;
	m_cIsJobDice = pPacket->cIsJobDice;
	_wcscpy( m_wszPartyPass, _countof(m_wszPartyPass), pPacket->wszPartyPass, (int)wcslen(pPacket->wszPartyPass) );
#endif // #if defined( PRE_PARTY_DB )

	m_nEventRoomIndex = 0;

	m_nCompleteExperience = 0;
	m_dwDungeonPlayerTime = 0;
	m_nDungeonGateID = 0;	

	memset(m_nKickedMemberList, 0, sizeof(m_nKickedMemberList));

	//test
	m_ActorSendTick = 0;
	m_ActorTick = DEFAULT_ACTOR_BROADTICK;
	m_nCullingDistance = DEFAULT_PACKETCULLING_DISTANCE;

	m_pWorld = NULL;
	m_pTaskMng = NULL;
	m_pGameTask = NULL;
	m_pPartyTask = NULL;
	m_pItemTask	= NULL;
	m_pSkillTask = NULL;
	m_pGuildTask = NULL;
	m_pCollisionMng = NULL;
	m_iPivotTick = timeGetTime();
	m_iNextGameState = 0;
	m_nCurItemLooterInOrder = 0;

	if (m_cReqGameIDType == REQINFO_TYPE_FARM)
		m_GameState	= _GAME_STATE_FARM_NONE;
	else
		m_GameState	= _GAME_STATE_READY2CONNECT;

	m_pRandom = new CRandom( this );
	_srand( this, timeGetTime() );

	m_pCollisionMng = new CEtCollisionMng( this );

	m_VecMember.reserve( PvPCommon::Common::MaxPlayer );
	m_DeleteList.reserve( PvPCommon::Common::MaxPlayer );
#if defined( PRE_FIX_49129 )	
	m_MapFirstPartyMember.clear();
#endif
#ifdef _USE_VOICECHAT
	memset(&m_nVoiceChannelID, 0, sizeof(m_nVoiceChannelID));
	m_nTalkingTick = 0;
	if (g_pVoiceChat)
	{
		for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
		{
			if (pPacket->nVoiceChannelID[i] <= 0) continue;
			if (g_pVoiceChat->CreateVoiceChannel(pPacket->nVoiceChannelID[i]))
			{
				m_nVoiceChannelID[i] = pPacket->nVoiceChannelID[i];
				m_nTalkingTick = timeGetTime();
			}
			else
				_DANGER_POINT();
		}
	}
#endif // #ifdef _USE_VOICECHAT

#if defined( PRE_TRIGGER_LOG )
	// test
	m_dTriggerTime = 0.f;
	m_dProcessTime = 0.f;
	if( pServer )
	{
#if !defined( _FINAL_BUILD )
		m_bLog = true;
#else // #if !defined( _FINAL_BUILD )
		m_bLog = (pServer->GetServerID() == 1);
#endif // #if !defined( _FINAL_BUILD )
		if( m_bLog )
			QueryPerformanceFrequency(&m_liFrequency);
	}
	else
	{
		m_bLog = false;
	}
#endif // #if defined( PRE_TRIGGER_LOG )

	_CalcDBConnection( iRoomID );

	m_cSeqLevel = 0;

	m_bStageStargLog = false;
	m_bStageEndLog = false;
	m_bGotoVillageFlag = false;
#if defined( PRE_ADD_DIRECTNBUFF )	
	m_DirectPartyBuffItemList.clear();
#endif
	m_iHackPenalty = 0;
	m_bForceDestroyRoom = false;
	m_bFinalizeRoom = false;
	m_i64RoomLogIndex = (static_cast<INT64>(g_Config.nManagedID)<<32)|((iRoomID&0xFFFF)<<16)|(rand()&0xFFFF);
#if defined( PRE_PARTY_DB )	
#else
	m_nUpkeepCount = pPacket->cUpkeepCount;
#endif

	memset(m_nPartyMemberIndex, 0, sizeof(m_nPartyMemberIndex));
	memset(m_wszInvitedCharacterName, 0, sizeof(m_wszInvitedCharacterName));
	m_nInivitedTime = 0;

	m_pMasterRewardSystem = new MasterSystem::CRewardSystem( this );

	m_bCheckTick = false;
#if defined( PRE_THREAD_ROOMDESTROY )
	m_lRoomDestroyInterLocked = eRoomDestoryStep::None;
#endif // #if defined( PRE_THREAD_ROOMDESTROY )

	m_mtRandom.srand(timeGetTime());

#if defined( PRE_ADD_36870 )
	m_iDungeonClearRound = 0;
#endif // #if defined( PRE_ADD_36870 )
#if defined( PRE_WORLDCOMBINE_PVP )
	m_eWorldReqRoom = pPacket->eWorldReqType;
	m_nWorldPvPRoomDBIndex = 0;
	m_bWorldPvPRoomStart = 0;
	m_nCreateGMAccountDBID = 0;
	memset(&m_tPvPRoomDBData, 0, sizeof(m_tPvPRoomDBData));
#endif
#if defined(PRE_ADD_ACTIVEMISSION)
	m_bIsFirstInitializeDungeon = false;
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	m_bAlteiaWorld = false;
	m_bAlteiaWorldMap = false;
	m_dwAlteiaWorldStartTime = 0;
	m_dwAlteiaWorldPlayTime = 0;
#endif
#if defined(PRE_ADD_CP_RANK)
	memset(&m_sLegendClearBest, 0, sizeof(m_sLegendClearBest));
	memset(&m_sMonthlyClearBest, 0, sizeof(m_sMonthlyClearBest));
#endif //#if defined(PRE_ADD_CP_RANK)
#if defined( PRE_PVP_GAMBLEROOM )
	m_nGambleRoomDBID = 0;
#endif
}

CDNGameRoom::~CDNGameRoom()
{
	SAFE_DELETE( m_pMasterRewardSystem );
}

void CDNGameRoom::FinalizeGameRoom()
{
	m_bFinalizeRoom = true;
	DespawnAndRemoveAIBots( false );

#if defined( PRE_PARTY_DB )	
	DelPartyDB();	
#endif

#if !defined( PRE_PARTY_DB )
	// Party end log
	if( GetGameType() == REQINFO_TYPE_PARTY && !GetGoToVillageFlag() )
	{
		BYTE cThreadID;
		CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
		if( pDBCon )
			pDBCon->QueryAddPartyEndLog( cThreadID, GetWorldSetID(), 0, GetPartyLogIndex() );
	}
#endif

	// Stage end log
	if( GetStageStartLogFlag() && !GetStageEndLogFlag() )
	{
		BYTE cThreadID;
		CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
		if( pDBCon )
		{
			pDBCon->QueryAddStageEndLog( cThreadID, GetWorldSetID(), 0, GetRoomLogIndex(), static_cast<DBDNWorldDef::WhereToGoCode::eCode>(0) );
		}
	}

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	if( bIsAlteiaWorld() )
	{
		GetDBConnection()->QueryAddStageEndLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), DBDNWorldDef::WhereToGoCode::Village );
	}
#endif

#if defined( PRE_ADD_58761 )
	if( GetDungeonGateID() > 0 )
		EndDungeonGateTime( GetDungeonGateID() );		
#endif


#ifdef _USE_VOICECHAT
	if (g_pVoiceChat)
	{
		for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
		{
			if (m_nVoiceChannelID[i] <= 0) continue;
			g_pVoiceChat->DestroyVoiceChannel(m_nVoiceChannelID[i]);
		}
	}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
	if( bIsWorldPvPRoom() && !bIsWorldPvPRoomStart() )
	{
		GetDBConnection()->QueryDelWorldPvPRoom( GetWorldSetID(), GetWorldPvPRoomDBIndex() );
	}
#endif

	std::list<CDNUserSession*>::iterator ii;
	for (ii = m_UserList.begin(); ii != m_UserList.end(); ii++)
		(*ii)->DetachConnection(L"~CDNGameRoom()");

	FinalizeProcess();
	TcpProcess();
	Process();

	if( m_pTaskMng ) 
		m_pTaskMng->RemoveAllTask();

	SAFE_DELETE( m_pTaskMng );
	SAFE_DELETE( m_pWorld );
	SAFE_DELETE( m_pRandom );
	SAFE_DELETE( m_pCollisionMng );
	SAFE_DELETE_VEC( m_GameListener );

	for( ii = m_UserList.begin() ; ii != m_UserList.end() ; ++ii )
	{
		delete *ii;
	}

#if defined( PRE_TRIGGER_LOG )
	if( m_bLog && m_dProcessTime >= 1.f )
	{
		wchar_t szBuf[MAX_PATH];
		wsprintf( szBuf, L"[Map:%d] Total %.3f ms Trigger %.3f ms rate=%d%%\r\n", m_iMapIdx, m_dProcessTime, m_dTriggerTime, static_cast<int>(m_dTriggerTime/m_dProcessTime*100) );
		g_TriggerLog.Log( LogType::_FILELOG, szBuf );
	}
#endif // #if defined( PRE_TRIGGER_LOG )

	//Server is not closing, yet 
	if (bIsFarmRoom())
	{
		//If the farm is destroyed, it means there is an issue; notify Master to handle it.
		g_pMasterConnectionManager->SendFarmIntendedDestroy(GetWorldSetID(), GetFarmIndex());
	}
}

void CDNGameRoom::_CalcDBConnection( UINT uiRoomID )
{
	// RootRoom exception handling
	if( m_pGameServer == NULL )
	{
		m_pDBConnection = NULL;
		m_cDBThreadID	= 0;
		return;
	}

	m_pDBConnection = g_pDBConnectionManager->GetDBConnection( uiRoomID, m_cDBThreadID );	

	if( !m_pDBConnection || !m_pDBConnection->GetActive() )
	{
		// Search for an active DBConnection at random for a certain number of attempts.
		for( int i=0 ; i<(int)g_pDBConnectionManager->GetCount()*2 ; ++i )
		{
			m_pDBConnection = g_pDBConnectionManager->GetDBConnection( m_cDBThreadID );
			
			// Exit once an Active DBConnection is found.
			if( m_pDBConnection && m_pDBConnection->GetActive() )
				break;
		}
	}
}

bool CDNGameRoom::bIsBreakIntoUser( CDNUserSession* pGameSession )
{
	std::list<CDNUserSession*>::iterator itor = std::find( m_BreakIntoUserList.begin(), m_BreakIntoUserList.end(), pGameSession );
	return (itor!=m_BreakIntoUserList.end());
}

void CDNGameRoom::GetBreakIntoUserTeamCount( int& iATeam, int& iBTeam )
{
	iATeam = 0;
	iBTeam = 0;
	for( std::list<CDNUserSession*>::iterator itor=m_BreakIntoUserList.begin() ; itor!=m_BreakIntoUserList.end() ; ++itor )
	{
		CDNUserSession* pSession = (*itor);

		// Only those not yet pushed to m_VecMember
		if( GetUserSession( pSession->GetSessionID() ) == NULL )
		{
			if( pSession->GetTeam() == PvPCommon::Team::A )
				++iATeam;
			else if( pSession->GetTeam() == PvPCommon::Team::B )
				++iBTeam;
		}
	}
}

CDNUserSession* CDNGameRoom::CreateBreakIntoGameSession( WCHAR* wszAccountName, const WCHAR * pwszChracterName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, 
														bool bAdult, char cPCBangGrade, int BreakIntoType )
{
	CDNUserSession* pSession = NULL;
	// Drop-in is only allowed in _GAME_STATE_PLAY state.
	if( m_GameState == _GAME_STATE_PLAY )
	{
#if defined( PRE_PARTY_DB )
		if (GetGameType() == REQINFO_TYPE_PARTY && GetPartyIndex() > 0 && (BreakIntoType==BreakInto::Type::WorldZoneParty || BreakIntoType==BreakInto::Type::PartyJoin) )
#else
		if (GetGameType() == REQINFO_TYPE_PARTY && GetPartyIndex() > 0 && BreakIntoType==BreakInto::Type::WorldZoneParty )
#endif // #if defined( PRE_PARTY_DB )
		{
			//Let's prevent cases where steps come in out of order~
			if ((int)m_VecMember.size() >= GetPartyMemberMax())
				return NULL;		//When trying to insert more than max count

#if defined( PRE_PARTY_DB )
			if ((int)(m_VecMember.size()+m_BreakIntoUserList.size()) >= GetPartyMemberMax())
				return NULL;		//When trying to enter exceeding max count including drop-in user
#endif // #if defined( PRE_PARTY_DB )
			if ( (BreakIntoType==BreakInto::Type::WorldZoneParty) && IsInvitingUser(pwszChracterName) == false)
				return NULL;		//Not allowed unless it is the inviter

			//Also check if currently in a world zone
			if (CDnWorld::GetInstance(this).GetMapType() != GlobalEnum::eMapTypeEnum::MAP_WORLDMAP)
				return NULL;
		}

		if( bIsFarmRoom() )
			pSession = new CDNFarmUserSession( nSessionID, m_pGameServer, this );
		else
			pSession = new IBoostPoolDNBreakIntoUserSession( nSessionID, m_pGameServer, this );
		if( pSession )
		{
			pSession->SetDBConInfo( m_pDBConnection, m_cDBThreadID );
			pSession->PreInitializeUser( wszAccountName, nAccountDBID, nSessionID, biCharacterDBID, iTeam, nWorldID, nVillageID, false, bAdult, cPCBangGrade );
		}
		else
			return pSession;

		m_BreakIntoUserList.push_back( pSession );
		m_UserList.push_back(pSession);
	}
	else
		g_Log.Log(LogType::_ERROR, nWorldID, nAccountDBID, biCharacterDBID, nSessionID, _T("Connect|GameStart Flow err [state:addmember]\n"));

	return pSession;
}

void CDNGameRoom::OnSendPartyMemberInfo( CDNUserSession* pBreakIntoSession )
{
	if( !pBreakIntoSession )
	{
		if( !HasAIBots() || bIsPvPRoom() )
			return;

		SCROOM_SYNC_MEMBERINFO TxPacket;
		memset( &TxPacket, 0, sizeof(TxPacket) );
		TxPacket.bIsStart = true;
		TxPacket.bIsBreakInto = false;

		UINT uiSendCount = 0;
		for( UINT i = 0; i < m_VecMember.size(); ++i )
		{
			m_VecMember[i].pSession->GetPartyMemberInfo( TxPacket.Member[i - uiSendCount] );
			if( TxPacket.Member[i - uiSendCount].cMemberIndex >= PARTYMAX )
				TxPacket.Member[i - uiSendCount].cMemberIndex = (BYTE)i;
			++TxPacket.nCount;

			if( TxPacket.nCount % _countof(TxPacket.Member) == 0 )
			{
				for( UINT m = 0; m < m_VecMember.size(); ++m )
				{
					if( m_VecMember[m].pSession && !m_VecMember[m].pSession->bIsAIBot() )
						m_VecMember[m].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				}
				uiSendCount += _countof(TxPacket.Member);
				TxPacket.nCount = 0;
				TxPacket.bIsStart = false;
			}
		}

		if( TxPacket.nCount > 0 )
		{
			int iSize = sizeof(TxPacket) - sizeof(TxPacket.Member) + TxPacket.nCount * sizeof(TxPacket.Member[0]);
			for( UINT m = 0; m < m_VecMember.size(); ++m )
			{
				if( m_VecMember[m].pSession && !m_VecMember[m].pSession->bIsAIBot() )
					m_VecMember[m].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), iSize );
			}
		}
		return;
	}

	SCROOM_SYNC_MEMBERINFO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.bIsStart		= true;
	TxPacket.bIsBreakInto	= true;

	UINT uiSendCount = 0;

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		m_VecMember[i].pSession->GetPartyMemberInfo( TxPacket.Member[i-uiSendCount] );
		++TxPacket.nCount;

		// Page completed
		if( TxPacket.nCount%_countof(TxPacket.Member) == 0 )
		{
			pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount		   += _countof(TxPacket.Member);
			TxPacket.nCount		= 0;
			TxPacket.bIsStart	= false;
		}
	}

	if( TxPacket.nCount > 0 )
	{
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.Member)+TxPacket.nCount*sizeof(TxPacket.Member[0]);

		pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}

void CDNGameRoom::OnSetLoad2SyncState( const int iCurTick )
{
	SendConnectedResult();


	m_GameState	= _GAME_STATE_LOAD2SYNC;		//Move to the next state.
	m_iNextGameState = iCurTick + WAIT_FOR_LOAD_TIME_LIMIT;
}

// Functions starting with On are virtual functions.
void CDNGameRoom::SetSync2SyncStateTemplateMethod( const int iCurTick, CDNUserSession* pBreakIntoSession/*=NULL*/)
{
	if( !pBreakIntoSession )
	{
		int nTargetMap = ResolveTargetMapIndex( m_iMapIdx, m_iGateIdx, m_iGateSelect, m_bDirectConnect );
		EnsureAIBotsForMap( nTargetMap, false, m_StageDifficulty );
	}
	InitRoomState( pBreakIntoSession );
	OnSendPartyMemberInfo( pBreakIntoSession );
	OnSendTeamData( pBreakIntoSession );	// Send team info at the very top for observer mode.
	SendEquipData( pBreakIntoSession );
	SendSkillData( pBreakIntoSession );
	SendEtcData( pBreakIntoSession );	
	SendSecondAuthInfo( pBreakIntoSession );
	SendMaxLevelCharacterCount( pBreakIntoSession );
	SendHPSP( pBreakIntoSession );
	SendBattleMode( pBreakIntoSession );
#if !defined( PRE_FIX_BREAKINTO_BLOW_SYNC )
	SendAddStateEffect( pBreakIntoSession );
#endif // #if !defined( PRE_FIX_BREAKINTO_BLOW_SYNC )
	SendGuildData( pBreakIntoSession );
	SendMasterSystemSimpleInfo( pBreakIntoSession );
	SendMasterSystemCountInfo( pBreakIntoSession );
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	SendReputationList( pBreakIntoSession );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#if defined (PRE_ADD_BESTFRIEND)
	SendBestFriendData( pBreakIntoSession );
#endif
#if defined( PRE_PARTY_DB )
	AddPartyDB( pBreakIntoSession );	
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	SendPartyEffectSkillItemData(pBreakIntoSession);
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	ApplyJoinMemberAppellation( pBreakIntoSession );
#endif
	SendCompleteGameReady( pBreakIntoSession ); // Must call this at the very end.
	if( !pBreakIntoSession )
	{
		m_GameState = bIsFarmRoom() ? _GAME_STATE_FARM_READY2LOAD : _GAME_STATE_SYNC2SYNC;
		
		m_iNextGameState = iCurTick + GOGO_SING_TO_PLAY_FOR_WAIT_TIME_LIMIT;
	}
	else
	{
		pBreakIntoSession->SendSeqLevel(m_cSeqLevel);
	}
}

void CDNGameRoom::SetSync2PvPModeStateTemplateMethod(const int iCurTick)
{
	if (m_VecMember.empty())		//Falls directly into here only when the user does not exist
		m_GameState = _GAME_STATE_PVP_SYNC2GAMEMODE;
}

void CDNGameRoom::OnSetPlayState()
{
	if (m_GameState == _GAME_STATE_DESTROYED) return;

	m_pGameTask->SetSyncComplete( true );
	m_pPartyTask->RequestPartyMember();
	m_pPartyTask->RequestSyncStart();

	//g_Log.LogA( _GREEN, "Member : %d\r\n", m_VecMember.size() );

	__time64_t _tNow, _tOderedTime, _tCloseTime;
	if (g_pCloseSystem)
	{
		_tNow = 0;
		if (g_pCloseSystem->IsClosing(_tOderedTime, _tCloseTime))
			time(&_tNow);
	}

	TNoticeInfo Notice;
	memset(&Notice, 0, sizeof(Notice));

	if (g_pNoticeSystem)
		g_pNoticeSystem->GetNotice(0, 0, Notice);
		
	for( DNVector(PartyStruct)::iterator itor=m_VecMember.begin() ; itor!=m_VecMember.end() ; ++itor )
	{
		if( (*itor).pSession->GetState() != SESSION_STATE_READY_TO_PLAY )
			continue;

		(*itor).pSession->SetLoadingComplete( true );
		if( (*itor).pSession->IsConnected() )
		{
			(*itor).pSession->SetSessionState( SESSION_STATE_GAME_PLAY );
			(*itor).pSession->SetSecurityUpdateFlag( true );

			if (Notice.nCreateTime > 0)
				(*itor).pSession->SendNotice(Notice.wszMsg, (int)wcsnlen(Notice.wszMsg, CHATLENMAX), Notice.TypeInfo.nSlideShowSec);

			if (_tNow > 0)
				(*itor).pSession->SendCloseService(_tNow, _tOderedTime, _tCloseTime);
#if defined(PRE_ADD_MISSION_COUPON)
			if( (*itor).pSession->GetExpiredPetID() )
			{
				(*itor).pSession->GetEventSystem()->OnEvent( EventSystem::OnPetExpired, 1, EventSystem::ItemID, (*itor).pSession->GetExpiredPetID() );
				(*itor).pSession->SetExpiredPetID(0);
			}
#endif
		}
	}

#ifdef _USE_PEERCONNECT
	ConnectPeerRequest();			//Peer Connect Request
#endif

	UpdateAppliedEventValue();

	m_GameState			= _GAME_STATE_PLAY;
	m_iNextGameState	= 0;
#if defined( STRESS_TEST )
	m_iNextGameState = timeGetTime();
#endif

	if( GetMasterRewardSystem() )
		GetMasterRewardSystem()->RequestRefresh();

#if defined(PRE_ADD_ACTIVEMISSION)
	if(GetIsFirstInitializeDungeon())
		InitActiveMission();
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
	if(m_nCreateGMAccountDBID > 0)
	{
		int nPvPIndex = (GetWorldPvPRoomDBIndex() % 100) + WorldPvPMissionRoom::Common::GMWorldPvPRoomStartIndex;
		g_pMasterConnectionManager->SendWorldPvPRoomGMCreateResult( GetWorldSetID(), ERROR_NONE, m_nCreateGMAccountDBID, &m_tPvPRoomDBData, nPvPIndex );
	}
#endif

	// When buffering packets before sending, sending GenerateMonster and SC_Slave_OF together may break synchronization because SC_SLAVE_OF is sent immediately
	// So configure to send the summon packet at a point when it can be sent properly.
	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		CDNGameRoom::PartyStruct* pPartyMember = GetPartyData( i );
		CDnPlayerActor* pPlayerActor = pPartyMember->pSession->GetPlayerActor();
		if( pPlayerActor )
			pPlayerActor->OnInitializeNextStageFinished();
	}

	if( HasAIBots() && !bIsPvPRoom() )
		SendHPSP( NULL );
}

void CDNGameRoom::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	if (!hActor || !hActor->IsPlayerActor())
		return;

	// Reset cooldowns of owned skills to 0. (Issue #2422)
	if( hActor )
		hActor->ResetSkillCoolTime();

	bool bCallMissionEvent = false;

	// Mission processing
	if( hHitter && hHitter->IsMonsterActor() )
	{
		CDnMonsterActor* pMonster = (CDnMonsterActor *)hHitter.GetPointer();

		if( hActor )
		{
			CDNUserSession* pSession = GetUserSession( hActor->GetSessionID() );
			if( pSession )
			{
				pSession->GetEventSystem()->OnEvent( EventSystem::OnDie, 1, EventSystem::MonsterID, pMonster->GetMonsterClassID() );
				bCallMissionEvent = true;
			}
		}
	}

	if( bCallMissionEvent == false && hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		if( pPlayerActor->GetUserSession() )
			pPlayerActor->GetUserSession()->GetEventSystem()->OnEvent( EventSystem::OnDie );
	}

	
}

void CDNGameRoom::OnSuccessBreakInto( CDNUserSession* pGameSession )
{
	if( !pGameSession )
		return;

	DnActorHandle hActor = pGameSession->GetActorHandle();
	if( !hActor )
		return;

	if( pGameSession->bIsGMTrace() )
	{
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_071, -1, NULL, true );		// Cannot act
	}

	GetWorld()->OnTriggerEventCallback( "CPvPGameMode::OnSuccessBreakInto", 0, 0 );
}

void CDNGameRoom::OnSuccessBreakInto( std::list<CDNUserSession*>::iterator& itor )
{
	itor = m_BreakIntoUserList.erase( itor );
}

void CDNGameRoom::_BreakIntoProcess()
{
	if( m_BreakIntoUserList.empty() )
		return;

	for( std::list<CDNUserSession*>::iterator itor=m_BreakIntoUserList.begin() ; itor!=m_BreakIntoUserList.end() ;  )
	{
		if( (*itor)->BreakIntoProcess() )
		{
			OnSuccessBreakInto( itor );
		}
		else
			++itor;
	}
}

void CDNGameRoom::InitEvent()
{
	m_EventList.clear();
	if( g_pEvent )
		g_pEvent->GetEvent(m_iWorldID, m_iMapIdx, &m_EventList);
}

void CDNGameRoom::OnGameStatePlay(ULONG iCurTick)
{
	// Drop-in process
	UINT uiTick = timeGetTime();
	PROFILE_TIME_TEST( _BreakIntoProcess() );
	if( timeGetTime()-uiTick >= 80 )
	{
		if( GetGameServer()->bIsFrameAlert() )
		{
			g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] OnGameStatePlay::_BreakIntoProcess() Delay RoomID:%d MapIndex=%d Delay=%d UserCount=%d\n", g_Config.nManagedID, GetRoomID(), m_iMapIdx, timeGetTime()-uiTick, m_BreakIntoUserList.size() );

			// Write user log
			for( std::list<CDNUserSession*>::iterator itor=m_BreakIntoUserList.begin() ; itor!=m_BreakIntoUserList.end() ; ++itor )
			{
				CDNUserSession* pSession = (*itor);
				if( pSession )
				{
					g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] OnGameStatePlay::_BreakIntoProcess() Delay RoomID:%d MapIndex=%d CharName=%s JobID=%d UserState=%d\n", g_Config.nManagedID, GetRoomID(), m_iMapIdx, pSession->GetCharacterName(), (int)pSession->GetUserJob(), pSession->GetState() );
				}
			}
		}			
	}
	
	// TaskMng process
	if( m_pTaskMng ) 
	{
		bool bResult;
		PROFILE_TIME_TEST_BLOCK_START( "m_pTaskMng->Excute()" );
		bResult = m_pTaskMng->Excute();
		PROFILE_TIME_TEST_BLOCK_END();
		if( !bResult ) 
			OutputDebug( "Finalize Task\n" );
	}
	if( !m_vChangeMapQueue.empty() )
	{
		int iMapIndex	= m_vChangeMapQueue.begin()->first;
		int iGateNo		= m_vChangeMapQueue.begin()->second;
		m_vChangeMapQueue.clear();
		m_pGameTask->RequestChangeMap( iMapIndex, static_cast<char>(iGateNo) );
	}
	FarmUpdate();
}

#ifdef _USE_VOICECHAT
void CDNGameRoom::OnInitVoice()
{
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if (m_VecMember[i].pSession->IsConnected())
			m_VecMember[i].pSession->MakeMute();
	}
}
#endif

void CDNGameRoom::OnDelPartyMember( UINT iDelMemberSessionID, char cKickKind )
{
	// When GM intrudes, it is REQINFO_TYPE_SINGLE, but OnDelPartyMember() is called.
	// Commented out below because it is filtered in SwapLeader. by Kimbap
	//if (m_cReqGameIDType == REQINFO_TYPE_SINGLE) return; // Must not send if single.

	UINT nNewLeaderSessionID = 0;
	if (m_cReqGameIDType == REQINFO_TYPE_PARTY)		//A new leader is needed only if it is a party
	{
		bool bRet = SwapLeader( iDelMemberSessionID, nNewLeaderSessionID );
		if (bRet == false)	return;		//User has already left.
	}

	if( m_pTaskMng )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(m_pTaskMng->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->OutPartyMember( iDelMemberSessionID, nNewLeaderSessionID, cKickKind );
	}
}

void CDNGameRoom::OnRequestSyncStartMsg( CDNUserSession* pGameSession )
{
	SendGameSyncStart( pGameSession );
	if( pGameSession && pGameSession->GetTimeEventSystem() ) 
		pGameSession->GetTimeEventSystem()->RequestSyncTimeEvent();
}


#if defined( STRESS_TEST )

void CDNGameRoom::OrgProcess()
{
#ifdef _FINAL_BUILD
	__try { 
#endif

		//Main process of game room! No synchronization when calling process from game room!
		unsigned long iCurTick = GetGameTick();

		UserUpdate( iCurTick );
		CheckRemovedMember();
		CheckRudpDisconnectedMember();

		switch (m_GameState)
		{
		case _GAME_STATE_READY2CONNECT:
			{
				int iConnectedCnt = 0;

				/*PartyStruct * pStruct = &m_VecMember[100];
				TUserData * pData = pStruct->pSession->GetUserData();
				pData->Attribute.nExp = 1000;*/

				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					if (m_VecMember[i].pSession->IsConnected())
						iConnectedCnt++;

				if (m_iPartMemberCnt == iConnectedCnt)
				{
					m_GameState = _GAME_STATE_CONNECT2LOAD;		//Move to the next state.
					m_iNextGameState = 0;
					break;
				}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
				if (iConnectedCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else	// #if defined( PRE_WORLDCOMBINE_PVP )
				if (iConnectedCnt == 0 && bIsWorldCombineParty() == false )
#endif	// #if defined( PRE_WORLDCOMBINE_PVP )
#else	// #if defined( PRE_WORLDCOMBINE_PARTY )
				if (iConnectedCnt == 0)
#endif	// #if defined( PRE_WORLDCOMBINE_PARTY )
				{
					if (iCurTick > (GetPartyIndex() <= 0 ? RUDP_CONNECT_TIME_LIMIT_FOR_SINGLE : RUDP_CONNECT_TIME_LIMIT_FOR_PARTY))
					{
						g_Log.Log(LogType::_NORMAL, L"Destroy|READY2CONNECT\n");
						//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
						DestroyGameRoom();
					}
					break;
				}

				if (m_iNextGameState == 0)		//At least one user connected. Wait for a set amount of time.
					m_iNextGameState = iCurTick + WAIT_FOR_ANOTHER_USER_TIME_LIMIT;

				if (iConnectedCnt != 0 && iCurTick > m_iNextGameState)
				{
					m_GameState = _GAME_STATE_CONNECT2LOAD;		//Move to the next state.
					m_iNextGameState = 0;
				}
				break;
			}

		case _GAME_STATE_CONNECT2LOAD:
			{
				if (m_iNextGameState == 0)
				{
					for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					{
						if (m_VecMember[i].pSession->IsConnected() == false)
						{
							g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Connection Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
							m_VecMember[i].pSession->DetachConnection( L"" );		//Disconnect clients not yet connected.
						}
					}

					CheckRemovedMember();
					OnInitGameRoomUser();
					m_iNextGameState = iCurTick + WAIT_FOR_LOAD_TIME_LIMIT;
				}

				int iLoadedUser = 0;
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					if (m_VecMember[i].pSession->GetState() == SESSION_STATE_LOADED)
						iLoadedUser++;

				if (m_iPartMemberCnt == iLoadedUser)
				{
					OnSetLoad2SyncState( iCurTick );
					break;
				}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
				if (iLoadedUser == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else	// #if defined( PRE_WORLDCOMBINE_PVP )
				if (iLoadedUser == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else
				if (iLoadedUser == 0)
#endif
				{
					if (iCurTick > m_iNextGameState)
					{
						g_Log.Log(LogType::_ERROR, L"Destroy|CONNECT2LOAD\n");
						//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
						DestroyGameRoom();
					}
					break;
				}

				if (iLoadedUser != 0 && iCurTick > m_iNextGameState)
				{
					for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					{
						if (m_VecMember[i].pSession->GetState() != SESSION_STATE_LOADED)
						{
							g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Load Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
							m_VecMember[i].pSession->DetachConnection( L"" );		//Disconnect clients not yet connected.
						}
					}

					OnSetLoad2SyncState( iCurTick );
				}
				break;
			}

		case _GAME_STATE_LOAD2SYNC:
			{
				int iReadyUserCnt = 0;
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					if (m_VecMember[i].pSession->GetState() == SESSION_STATE_READY_TO_SYNC)
						iReadyUserCnt++;

				if (m_iPartMemberCnt == iReadyUserCnt)
				{
					SetSync2SyncStateTemplateMethod( iCurTick );
					break;
				}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
				if (iReadyUserCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else	// #if defined( PRE_WORLDCOMBINE_PVP )
				if (iReadyUserCnt == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )				
#else
				if (iReadyUserCnt == 0)
#endif
				{
					if (iCurTick > m_iNextGameState)
					{
						g_Log.Log(LogType::_ERROR, L"Destroy|LOAD2SYNC\n");
						//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
						DestroyGameRoom();
					}
					break;
				}

				if (iReadyUserCnt != 0 && iCurTick > m_iNextGameState)
				{
					for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					{
						if (m_VecMember[i].pSession->GetState() != SESSION_STATE_READY_TO_SYNC)
						{
							g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Load Sync Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
							m_VecMember[i].pSession->DetachConnection( L"" );		//Disconnect clients not yet connected.
						}
					}

					//Interference with load thread can occur at this timing, so check and truly cut/stop it before starting load
					CheckRemovedMember();
					if (m_GameState != _GAME_STATE_DESTROYED)			//Just in case...
						SetSync2SyncStateTemplateMethod( iCurTick );
					else
						_DANGER_POINT();
				}
				break;
			}

		case _GAME_STATE_SYNC2SYNC:
			{
				if (g_pBackLoader->IsLoaded(m_iRoomID) == true)
				{
					if (g_pBackLoader->LoadConfirm(m_iRoomID) == false)
					{
						g_Log.Log(LogType::_ERROR, L"Destroy|DataLoad Failed CheckOut ResourceData roomID [%d]\n", m_iRoomID);
						DestroyGameRoom();
						break;
					}

					SyncProp( NULL );

					m_pWorld->EnableTriggerEventCallback(true);
					m_pWorld->OnTriggerEventCallback( "CDnGameTask::PostInitializeStage", 0, 0.f, false );
					m_pWorld->EnableTriggerEventCallback(false);

					m_GameState = _GAME_STATE_SYNC2PLAY;
					break;
				}

				if (iCurTick > m_iNextGameState)
				{
					g_Log.Log(LogType::_ERROR, L"Destroy|Delay Load SYNC2SYNC\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
					m_iNextGameState = 0;
					break;
				}
				break;
			}

		case _GAME_STATE_SYNC2PLAY:
			{
				int iConnectedCnt = 0;
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->IsConnected() && m_VecMember[i].pSession->GetState() == SESSION_STATE_SYNC_READY_2_DELAY)
					{
						m_VecMember[i].pSession->FlushStoredPacket();
						m_VecMember[i].pSession->FlushPacketQueue();
						for( UINT i2=0 ; i2<m_VecMember.size() ; ++i2 )
							SendGameSyncWait( m_VecMember[i2].pSession, m_VecMember[i2].pSession->GetSessionID() );
						m_VecMember[i].pSession->SetSessionState(SESSION_STATE_READY_TO_PLAY);
					}

					if (m_VecMember[i].pSession->IsConnected() && m_VecMember[i].pSession->GetState() == SESSION_STATE_READY_TO_PLAY)
						iConnectedCnt++;
				}

				if (m_iPartMemberCnt == iConnectedCnt)
				{
					OnSetPlayState();
					break;
				}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
				if (iConnectedCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else	// #if defined( PRE_WORLDCOMBINE_PVP )
				if (iConnectedCnt == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )				
#else
				if (iConnectedCnt == 0)
#endif
				{
					if (iCurTick > m_iNextGameState)
					{
						g_Log.Log(LogType::_ERROR, L"Destroy|SYNC2PLAY\n");
						//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
						DestroyGameRoom();
					}
					break;
				}

				if (iCurTick > m_iNextGameState)
				{
					for( UINT i=0 ; i<m_VecMember.size() ; ++i )
					{
						if (m_VecMember[i].pSession->GetState() != SESSION_STATE_READY_TO_PLAY)
						{
							g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Sync Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
							m_VecMember[i].pSession->DetachConnection( L"" );		//Disconnect clients not yet processed.
						}
					}

					CheckRemovedMember();
					OnSetPlayState();
					break;
				}
				break;
			}

		case _GAME_STATE_CANCEL_LOADING:
			{
				if (g_pBackLoader->LoadCancel(m_iRoomID))
					m_GameState = _GAME_STATE_DESTROYED;
				break;
			}

		case _GAME_STATE_PLAY:
			{
				OnGameStatePlay(iCurTick);
				break;
			}
		}	

		/*
Allow catching Windows SEH exceptions as well
Compile with Enable C++ Exceptions set to /EHa option.
		*/
#ifdef _FINAL_BUILD
	}  // __try {  
	__except(CExceptionReport::GetInstancePtr()->Proc(GetExceptionInformation(), MiniDumpNormal)) {
	{
		// If an exception occurs during room process, close only the game room.
		g_Log.Log(LogType::_ROOMCRASH, L"[%d] Destroy|Process Crash RoomID=%d RoomState=%d\n", g_Config.nManagedID, m_iRoomID, m_GameState );
		if (g_pBackLoader)
		{
			//for( UINT i=0 ; i<m_VecMember.size() ; ++i )
			//	m_VecMember[i].pSession->SetSessionState(SESSION_STATE_CRASH);
			DestroyGameRoom();
		}

	}
#endif
}

#endif

void CDNGameRoom::DestroyGameRoom(bool bForce/* = false*/)
{
	if (g_pBackLoader->LoadCancel(m_iRoomID) == false)
		m_GameState = _GAME_STATE_CANCEL_LOADING;
	else
	{
		if (bForce == false)		//Check if not forced
		{
			// Do not destroy the room if there is a drop-in user.
			//If room crashes, kill it even with drop-in active
			if( IsRoomCrash() == false && bIsExistBreakIntoUser() )
				return;
		}

		m_GameState = _GAME_STATE_DESTROYED;
	}
}

void CDNGameRoom::FinalizeProcess()
{		
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
#ifdef _FINAL_BUILD
		__try 
		{ 
#endif // #ifdef _FINAL_BUILD

			m_VecMember[i].pSession->FinalUser();
			GetGameServer()->RemoveConnection(GetRoomID(), m_VecMember[i].pSession->GetNetID(), m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID());

			m_VecMember[i].ReleaseEquipInventory();

#ifdef _FINAL_BUILD
		}  // __try {  
		__except(CExceptionReport::GetInstancePtr()->Proc(GetExceptionInformation(), MiniDumpNormal))
		{
			m_bRoomCrash = true;
			m_VecMember[i].pSession->SetSessionState(SESSION_STATE_DISCONNECTED);
			g_Log.Log( LogType::_ROOMFINALIZECRASH, L"[%d] FinalizeProcess Crash RoomID=%d\n", g_Config.nManagedID, m_iRoomID );
		}
#endif // #ifdef _FINAL_BUILD
	}

	m_DeleteList.clear();
	m_VecMember.clear();
#if defined( PRE_FIX_49129 )	
	m_MapFirstPartyMember.clear();
#endif
}

void CDNGameRoom::_DeleteSession()
{
	for( UINT i=0 ; i<m_vDeleteSession.size() ; ++i )
	{
		SAFE_DELETE( m_vDeleteSession[i] );
	}
	m_vDeleteSession.clear();
}

void CDNGameRoom::Process()
{
//#ifdef _FINAL_BUILD
	__try {
		_ProcessImpl();
	}
	__except(CExceptionReport::GetInstancePtr()->Proc(GetExceptionInformation(), MiniDumpNormal))
	{
		g_Log.Log(LogType::_ROOMCRASH, L"[%d] Destroy|Process Crash RoomID=%d RoomState=%d\n", g_Config.nManagedID, m_iRoomID, m_GameState );
		if (g_pBackLoader)
		{
			for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				m_VecMember[i].pSession->SetDecreaseFatigue(0);
			m_bRoomCrash = true;
			DestroyGameRoom();
		}
		if (g_pMasterConnectionManager && bIsPvPRoom())
		{
			if (bIsPvPRoom() && bIsGuildWarSystem())
				g_pMasterConnectionManager->SendPvPDetectCrash(GetWorldSetID(), GetRoomID(), GetPvPIndex());
		}
	}
}

void CDNGameRoom::_ProcessImpl()
{

#if defined( STRESS_TEST )
		if( m_cReqGameIDType == REQINFO_TYPE_PARTY )
			return OrgProcess();
#endif

	//Main process of game room! No synchronization when calling process from game room!
	unsigned long iCurTick = GetGameTick();
	unsigned long iCurTick2 = timeGetTime();
	
	_DeleteSession();
	UserUpdate( iCurTick2 );
	CheckRemovedMember();
	CheckRudpDisconnectedMember();
#ifdef _USE_VOICECHAT
	TalkingUpdate(iCurTick2);
#endif
	if (bIsGuildWarSystem() && m_VecMember.empty())
	{
		if (GetPvPGameMode() && GetPvPGameMode()->bIsFinishFlag())
			DestroyGameRoom();
	}

	if (m_nEventRoomIndex > 0 && 
		(iCurTick > 1000 * 60 * 60)	&& 	// When exceeding 1 hour
		!m_bCheckTick)
	{
		m_bCheckTick = true;
		g_Log.Log(LogType::_PVPROOM, L"PVP OverTime [Index:%d][Room:%d][Event:%d][User:%d][Break:%d][Vec:%d][MID:%d][State:%d] \r\n", 
			GetPvPIndex(), GetRoomID(), m_nEventRoomIndex, m_UserList.size(), m_BreakIntoUserList.size(), m_VecMember.size(), g_Config.nManagedID, m_GameState);

		for( m_ItorSession=m_BreakIntoUserList.begin() ; m_ItorSession!=m_BreakIntoUserList.end() ; ++m_ItorSession )
		{
			CDNUserSession* pSession = (*m_ItorSession);
			if (pSession)
				g_Log.Log(LogType::_PVPROOM, L"PVP OverTime MemberInfo [CHRID:%d][State:%d]\r\n", pSession->GetCharacterDBID(), pSession->GetState());			
		}
		
		m_GameState = _GAME_STATE_DESTROYED;	// Proceeding with room destruction.
	}

	switch (m_GameState)
	{
	case _GAME_STATE_READY2CONNECT:
		{
			int iConnectedCnt = 0;

			/*PartyStruct * pStruct = &m_VecMember[100];
			TUserData * pData = pStruct->pSession->GetUserData();
			pData->Attribute.nExp = 1000;*/

			for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				if (m_VecMember[i].pSession->IsConnected())
					iConnectedCnt++;

#if defined( STRESS_TEST )
			if(true)
#else
			if (m_iPartMemberCnt == iConnectedCnt)
#endif
			{
				m_GameState = _GAME_STATE_CONNECT2CHECKAUTH;	//Move to the next state.
				m_iNextGameState = timeGetTime();
#ifdef _USE_VOICECHAT
				OnInitVoice();
#endif
				break;
			}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
			if (iConnectedCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else // #if defined( PRE_WORLDCOMBINE_PVP )
			if (iConnectedCnt == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
			if (iConnectedCnt == 0)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			{
				if (iCurTick > (GetPartyIndex() <= 0 ? RUDP_CONNECT_TIME_LIMIT_FOR_SINGLE : RUDP_CONNECT_TIME_LIMIT_FOR_PARTY))
				{
					g_Log.Log(LogType::_NORMAL, L"Destroy|READY2CONNECT\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
				}
				break;
			}

			if (m_iNextGameState == 0)		//At least one user connected. Wait for a set amount of time.
				m_iNextGameState = iCurTick + WAIT_FOR_ANOTHER_USER_TIME_LIMIT;

			if (iConnectedCnt != 0 && iCurTick > m_iNextGameState)
			{
				m_GameState = _GAME_STATE_CONNECT2CHECKAUTH;	//Move to the next state.
				m_iNextGameState = timeGetTime();

#ifdef _USE_VOICECHAT
				OnInitVoice();
#endif

				//Filter once before moving to the next state.
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->IsConnected() == false)
					{
						g_Log.Log(LogType::_GAMECONNECTLOG, m_VecMember[i].pSession, L"Connect|Connection Fail\n" );
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Connection Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_READY2CONNECT");		//Disconnect clients not yet connected.
					}
				}
			}
			break;
		}

	case _GAME_STATE_CONNECT2CHECKAUTH:
		{
			int iCertifiedCnt = 0;

			for( UINT i=0 ; i<m_VecMember.size() ; ++i ) {
				if (false == m_VecMember[i].pSession->IsConnected()) {
					continue;
				}
				if (true == m_VecMember[i].pSession->IsCertified()) {
					++iCertifiedCnt;
				}
			}

#if defined( STRESS_TEST )
			if( true ) { 
#else
			if (m_iPartMemberCnt == iCertifiedCnt) {
#endif
				m_GameState = _GAME_STATE_CONNECT2LOAD;		//Move to the next state.
				m_iNextGameState = 0;
				break;
			}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
			if (0 == iCertifiedCnt && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false) {
#else // #if defined( PRE_WORLDCOMBINE_PVP )
			if (0 == iCertifiedCnt && bIsWorldCombineParty() == false ) {
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
			if (0 == iCertifiedCnt) {
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
				if (CHECKAUTHLIMITTERM < GetTickTerm(m_iNextGameState, iCurTick2)) {
					g_Log.Log(LogType::_NORMAL, L"Destroy|CONNECT2CHECKAUTH\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
				}
				break;
			}

			if (0 < iCertifiedCnt && 
				CHECKAUTHLIMITTERM < GetTickTerm(m_iNextGameState, iCurTick2))
			{
				//Add handling to kick unauthenticated entries before moving to the next state
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->IsCertified() != true)
					{
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Certified Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_CONNECT2CHECKAUTH");		//Disconnect clients not yet authenticated.
					}
				}

				m_GameState = _GAME_STATE_CONNECT2LOAD;		//Move to the next state.
				m_iNextGameState = 0;
			}
			break;
		}
		break;

	case _GAME_STATE_CONNECT2LOAD:
		{
			if (m_iNextGameState == 0)
			{
#if !defined( STRESS_TEST )
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->IsConnected() == false)
					{
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Connection Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_CONNECT2LOAD");		//Disconnect clients not yet connected.
					}
				}
#endif

				CheckRemovedMember();
				OnInitGameRoomUser();
				m_iNextGameState = iCurTick + WAIT_FOR_LOAD_TIME_LIMIT;
			}

			int iLoadedUser = 0;
			for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				if (m_VecMember[i].pSession->GetState() == SESSION_STATE_LOADED)
					iLoadedUser++;

#if defined( STRESS_TEST )
			if( true )
#else
			if (m_iPartMemberCnt == iLoadedUser)
#endif
			{
				OnSetLoad2SyncState( iCurTick );
				break;
			}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
			if (iLoadedUser == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false )
#else // #if defined( PRE_WORLDCOMBINE_PVP )
			if (iLoadedUser == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
			if (iLoadedUser == 0)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			{
				if (iCurTick > m_iNextGameState)
				{
					g_Log.Log(LogType::_NORMAL, L"Destroy|CONNECT2LOAD\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
				}
				break;
			}

			if (iLoadedUser != 0 && iCurTick > m_iNextGameState)
			{
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->GetState() != SESSION_STATE_LOADED)
					{
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Load Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_CONNECT2LOAD");		//Disconnect clients not yet connected.
					}
				}

				OnSetLoad2SyncState( iCurTick );
			}
			break;
		}

	case _GAME_STATE_LOAD2SYNC:
		{
			int iReadyUserCnt = 0;
			for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				if (m_VecMember[i].pSession->GetState() == SESSION_STATE_READY_TO_SYNC)
					iReadyUserCnt++;

#if defined( STRESS_TEST )
			if( true )
#else
			if (m_iPartMemberCnt == iReadyUserCnt)
#endif
			{
				SetSync2SyncStateTemplateMethod( iCurTick );
				break;
			}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
			if (iReadyUserCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false)
#else // #if defined( PRE_WORLDCOMBINE_PVP )
			if (iReadyUserCnt == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
			if (iReadyUserCnt == 0)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			{
				if (iCurTick > m_iNextGameState)
				{
					g_Log.Log(LogType::_NORMAL, L"Destroy|LOAD2SYNC\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
				}
				break;
			}

			if (iReadyUserCnt != 0 && iCurTick > m_iNextGameState)
			{
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->GetState() != SESSION_STATE_READY_TO_SYNC)
					{
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Load Sync Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_LOAD2SYNC");		//Disconnect clients not yet connected.
					}
				}
				
				//Interference with load thread can occur at this timing, so check and truly cut/stop it before starting load
				CheckRemovedMember();
				if (m_GameState != _GAME_STATE_DESTROYED)			//Just in case...
					SetSync2SyncStateTemplateMethod( iCurTick );
				else
					_DANGER_POINT();
			}
			break;
		}

		case _GAME_STATE_SYNC2SYNC:
		{
			if (g_pBackLoader->IsLoaded(m_iRoomID) == true)
			{
				if (g_pBackLoader->LoadConfirm(m_iRoomID) == false)
				{
					g_Log.Log(LogType::_ERROR, L"[%d] Destroy|DataLoad Failed CheckOut ResourceData roomID [%d]\n", g_Config.nManagedID, m_iRoomID);
					DestroyGameRoom();
					break;
				}

				// Moved UpdateRoomCountInfo to GameThread.
				if( m_pGameServer && m_pGameServer->GetServerManager() )
					m_pGameServer->GetServerManager()->UpdateRoomCountInfo( GetRoomID(), m_iMapIdx );

				SyncProp( NULL );
				OnSync2Sync( NULL );

				m_pWorld->EnableTriggerEventCallback(true);
				m_pWorld->OnTriggerEventCallback( "CDnGameTask::PostInitializeStage", 0, 0.f, false );
				m_pWorld->EnableTriggerEventCallback(false);

				m_GameState = _GAME_STATE_SYNC2PLAY;
				break;
			}
			
			if (iCurTick > m_iNextGameState)
			{
				g_Log.Log(LogType::_NORMAL, L"Destroy|Delay Load SYNC2SYNC\n");
				//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
				DestroyGameRoom();
				m_iNextGameState = 0;
				break;
			}
			break;
		}

		case _GAME_STATE_SYNC2PLAY:
		{
			int iConnectedCnt = 0;
#if !defined( STRESS_TEST )
			for( UINT i=0 ; i<m_VecMember.size() ; ++i )
			{
				if( m_VecMember[i].pSession->bIsAIBot() &&
					( m_VecMember[i].pSession->GetState() == SESSION_STATE_READY_TO_SYNC || m_VecMember[i].pSession->GetState() == SESSION_STATE_GAME_PLAY ) )
				{
					m_VecMember[i].pSession->SetSessionState( SESSION_STATE_SYNC_READY_2_DELAY );
				}

				if (m_VecMember[i].pSession->IsConnected() && m_VecMember[i].pSession->GetState() == SESSION_STATE_SYNC_READY_2_DELAY)
				{
					m_VecMember[i].pSession->FlushStoredPacket();
					m_VecMember[i].pSession->FlushPacketQueue();
					for( UINT i2=0 ; i2<m_VecMember.size() ; ++i2 )
						SendGameSyncWait( m_VecMember[i2].pSession, m_VecMember[i].pSession->GetSessionID() );
					m_VecMember[i].pSession->SetSessionState(SESSION_STATE_READY_TO_PLAY);
				}

				if (m_VecMember[i].pSession->IsConnected() && m_VecMember[i].pSession->GetState() == SESSION_STATE_READY_TO_PLAY)
					iConnectedCnt++;
			}
#endif
			
#if defined( STRESS_TEST )
			if( true )
#else
			if (m_iPartMemberCnt == iConnectedCnt)
#endif
			{
				OnSetPlayState();
				break;
			}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
			if (iConnectedCnt == 0 && bIsWorldCombineParty() == false && bIsWorldPvPRoom() == false)
#else // #if defined( PRE_WORLDCOMBINE_PVP )
			if (iConnectedCnt == 0 && bIsWorldCombineParty() == false )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
			if (iConnectedCnt == 0)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
			{
				if (iCurTick > m_iNextGameState)
				{
					g_Log.Log(LogType::_NORMAL, L"Destroy|SYNC2PLAY\n");
					//If there are no valid players and the time limit expires, the room enters deleted state (deleted by parent process)
					DestroyGameRoom();
				}
				break;
			}

			if (iCurTick > m_iNextGameState)
			{
				for( UINT i=0 ; i<m_VecMember.size() ; ++i )
				{
					if (m_VecMember[i].pSession->GetState() != SESSION_STATE_READY_TO_PLAY)
					{
						g_Log.Log(LogType::_ERROR, m_VecMember[i].pSession, L"Connect|Sync Fail AID[%d] SID[%d] CID[%I64d]\n", m_VecMember[i].pSession->GetAccountDBID(), m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetCharacterDBID());
						m_VecMember[i].pSession->DetachConnection(L"_GAME_STATE_SYNC2PLAY");		//Disconnect clients not yet processed.
					}
				}

				CheckRemovedMember();
				OnSetPlayState();
				break;
			}
			break;
		}

		case _GAME_STATE_FARM_READY2LOAD:
		{
			if (m_cReqGameIDType != REQINFO_TYPE_FARM)
			{
				_DANGER_POINT();
				DestroyGameRoom();
				break;
			}

			if (g_pBackLoader->IsLoaded(m_iRoomID) == true)
			{
				if (g_pBackLoader->LoadConfirm(m_iRoomID) == false)
				{
					g_Log.Log(LogType::_FARM, L"[%d] Destroy|DataLoad Failed CheckOut ResourceData roomID [%d]\n", g_Config.nManagedID, m_iRoomID);
					DestroyGameRoom();
					break;
				}
				
				QueryGetListField();
			}
			break;
		}

		case _GAME_STATE_FARM_LOAD2PLAY:
			{
				// Entered only for farm
				if (iCurTick > m_iNextGameState)
				{
					// Resend query after a certain time has passed since sending
					QueryGetListField();
				}
			}
			break;
		case _GAME_STATE_CANCEL_LOADING:
		{
			if (g_pBackLoader->LoadCancel(m_iRoomID))
			{
				// Do not destroy the room if there is a drop-in user.
				if( bIsExistBreakIntoUser() )
					break;
				m_GameState = _GAME_STATE_DESTROYED;
			}
			break;
		}
		
		case _GAME_STATE_PLAY:
		{
#if defined( PRE_TRIGGER_LOG )
			if( m_bLog )
				QueryPerformanceCounter(&m_liStartTime);
#endif // #if defined( PRE_TRIGGER_LOG )

			OnGameStatePlay(iCurTick);

#if defined( PRE_TRIGGER_LOG )
			if( m_bLog )
			{
				QueryPerformanceCounter(&m_liCurTime);
				double dElapsed = (1000 * ( m_liCurTime.QuadPart - m_liStartTime.QuadPart ) / static_cast<double>( m_liFrequency.QuadPart ));

				m_dProcessTime += dElapsed;
			}
#endif // #if defined( PRE_TRIGGER_LOG )

#if defined( STRESS_TEST )
			if( m_iRandomSeed > 0 )
			{
				if( timeGetTime()-m_iNextGameState >= static_cast<UINT>(m_iRandomSeed) )
				{
					if( m_VecMember[0].pSession )
						m_VecMember[0].pSession->ChangeServerUserData();
					DestroyGameRoom();
				}
			}
			/*
			else
			{
				if( timeGetTime()-m_iNextGameState >= 500 )
				{
					g_pDBConnectionManager->QueryLevelExp( m_VecMember[0].pSession->GetWorldSetID(), m_VecMember[0].pSession->GetCharacterDBID(), 1, 0 );
					m_iNextGameState = timeGetTime();
				}
			}
			*/
#endif
			break;
		}
	}

#if defined(PRE_FIX_INITSTATEANDSYNC)
	if(m_bInitStateAndSyncReserved)
		InitStateAndSync();
#endif

	/*
Allow catching Windows SEH exceptions as well
Compile with Enable C++ Exceptions set to /EHa option.
	*/
}

void CDNGameRoom::TcpProcess()
{
	DNVector(PartyStruct)::iterator ii;
	for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
		if ((*ii).pSession->GetTcpConnection() != NULL && (*ii).pSession->GetTcpConnection()->GetDelete() != true && (*ii).pSession->GetTcpConnection()->IsAttachedToSession() )
			if ((*ii).pSession->GetTcpConnection()->FlushRecvData(0) == false)
				RemoveMember((*ii).pSession, L"TcpProcess()");
}

void CDNGameRoom::InitRoomState( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession )
		return;

	OutputDebug( "InitSession\n" );

	// Task Initialize
	if( m_pTaskMng ) m_pTaskMng->RemoveAllTask();
	SAFE_DELETE( m_pTaskMng );
	m_pTaskMng = new CTaskManager(this);

	m_pWorld = new CDnWorld(this);

	// Party setup
	m_pPartyTask = CDnTaskFactory::CreatePartyTask( m_GameTaskType, this );
	m_pPartyTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pPartyTask, "PartyTask", -1 );

	// Item
	m_pItemTask = new CDnItemTask( this );
	m_pItemTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pItemTask, "ItemTask", -1 );
	// Set up inventory.

	// Skill task
	m_pSkillTask = new CDnSkillTask( this );
	m_pSkillTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pSkillTask, "SkillTask", -1 );

	// After game task
	m_pGameTask = CDnTaskFactory::CreateGameTask( m_GameTaskType, this );
	m_pGameTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pGameTask, "GameTask", -1 );

	m_pGuildTask = new CDnGuildTask( this );
	m_pGuildTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pGuildTask, "GuildTask", -1 );

	m_pChatTask = new CDnChatTask( this );
	m_pChatTask->Initialize();
	CTaskManager::GetInstance(this).AddTask( m_pChatTask, "ChatTask", -1 );

	AddGameListener( m_pPartyTask );
	AddGameListener( m_pItemTask );
	AddGameListener( m_pGameTask );
	AddGameListener( m_pSkillTask );
	AddGameListener( m_pGuildTask );
	AddGameListener( m_pChatTask );

	// Allow setting later.

	if (g_pBackLoader != NULL)
	{
		InitEvent();
		if (g_pBackLoader->PushToLoadProcess( this ) == false)
		{
			//This shouldn't happen! It hasn't even started, just kill the room
			g_Log.Log(LogType::_ERROR, L"Destroy|OnInitRoomState LoadError\n");
			DestroyGameRoom();
		}
	}
	else
	{
		g_Log.Log(LogType::_ERROR, L"Destroy|BackLoader Not Found\n");
		DestroyGameRoom();
	}

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if( !m_VecMember[i].pSession )
			continue;
		m_VecMember[i].pSession->SetLoadingComplete( false );
	}
}

void CDNGameRoom::SendSeqLevel()
{
	m_cSeqLevel++;			//Increment sequencing level on move. Intermediate values are ignored when raised (for now, only ignore actor-related packets).
	for( DWORD i=0; i<GetUserCount(); i++ )
		GetUserData(i)->SendSeqLevel(m_cSeqLevel);
}

// To fix synchronization issues when InitStateAndSync is called while GameRoom process is running (e.g., on trigger call) 
// Changed process to store info via ReserveInitStateAndSync and check/handle it at GameRoom process termination
#if defined(PRE_FIX_INITSTATEANDSYNC)
void CDNGameRoom::ReserveInitStateAndSync(int iMapIdx, int iGateIdx, int iRandomSeed, TDUNGEONDIFFICULTY StageDifficulty, bool bDirectConnect, int iGateSelect)
{
	m_bInitStateAndSyncReserved = true;
	m_nReservedMapIdx = iMapIdx;
	m_nReservedGateIdx = iGateIdx;
	m_nReservedRandomSeed = iRandomSeed;
	m_ReservedStageDifficulty = StageDifficulty;
	m_bReservedDirectConnect = bDirectConnect;
	m_bReservedGateSelect = iGateSelect;
}

void CDNGameRoom::ResetReservedStateAndSyncData()
{
	m_bInitStateAndSyncReserved = false;
	m_nReservedMapIdx = 0;
	m_nReservedGateIdx = 0;
	m_nReservedRandomSeed = 0;
	m_ReservedStageDifficulty =TDUNGEONDIFFICULTY::Easy; 
	m_bReservedDirectConnect = false;
	m_bReservedGateSelect = 0;
}

void CDNGameRoom::InitStateAndSync()
{
	if (g_pBackLoader != NULL)
	{
		if (GetFarmIndex() > 0)
		{
			_DANGER_POINT();
			return ;
		}

		DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
		if( pSox && !pSox->IsExistItem( m_nReservedMapIdx ) ) {
			DestroyGameRoom();
			return;
		}

		int nNextTargetMap = ResolveTargetMapIndex( m_nReservedMapIdx, m_nReservedGateIdx, m_bReservedGateSelect, m_bReservedDirectConnect );
		bool bAIBotListChanged = EnsureAIBotsForMap( nNextTargetMap, true, m_ReservedStageDifficulty );
		if( bAIBotListChanged && HasAIBots() && !bIsPvPRoom() )
		{
			OnSendPartyMemberInfo( NULL );
			OnSendTeamData( NULL );
			SendEquipData( NULL );
			SendSkillData( NULL );
			SendEtcData( NULL );
			SendHPSP( NULL );
			SendBattleMode( NULL );
		}

		InitEvent();
#if defined(PRE_ADD_WORLD_EVENT)
		TEvent * pEvent = GetApplyEventType(WorldEvent::EVENT5);		
#else
		TEvent * pEvent = GetApplyEvent(_EVENT_1_DROPITEM);
#endif //#if defined(PRE_ADD_WORLD_EVENT)

		for( DWORD i=0; i<GetUserCount(); i++ ) {
			GetUserData(i)->SetMapIndex( m_nReservedMapIdx );
			GetUserData(i)->SendStartStage( m_bReservedDirectConnect, m_nReservedMapIdx, m_nReservedGateIdx, m_nReservedRandomSeed, m_ReservedStageDifficulty, pEvent == NULL ? 0 : pEvent->nAtt1 );
			GetUserData(i)->ChangeStageUserData();
			if( GetUserData(i)->bIsAIBot() )
			{
				GetUserData(i)->SetSessionState( SESSION_STATE_READY_TO_SYNC );
				GetUserData(i)->SetLoadingComplete( true );
			}
			else
			{
				GetUserData(i)->SetLoadingComplete( false );
			}

			//Do not send HackShield data during mid-game loading, as gate transitions mid-game can drop it on the client -2hogi
			GetUserData(i)->SetSecurityUpdateFlag(false);
			// GetUserData(i)->UpdateUserData();
		}

		m_iMapIdx = m_nReservedMapIdx;
		m_iGateIdx = m_nReservedGateIdx;
		m_iGateSelect = m_bReservedGateSelect;
		m_StageDifficulty = m_ReservedStageDifficulty;
		m_iRandomSeed = m_nReservedRandomSeed;
		m_bDirectConnect = m_bReservedDirectConnect;

		if (g_pBackLoader->PushToLoadProcess( this, true ) == false)
		{
			//This shouldn't happen! It hasn't even started, just kill the room
			g_Log.Log(LogType::_ERROR, L"Destroy|InitStateAndSync LoadError\n");
			DestroyGameRoom();
		}
		else
		{
			m_GameState = _GAME_STATE_SYNC2SYNC;
			m_iNextGameState = GetGameTick() + GOGO_SING_TO_PLAY_FOR_WAIT_TIME_LIMIT;
		}

		SendSeqLevel();
	}
	else
	{
		g_Log.Log(LogType::_ERROR, L"Destroy|BackLoader Not Found\n");
		DestroyGameRoom();
	}
	
	ResetReservedStateAndSyncData();
}

#else	//#if defined(PRE_FIX_INITSTATEANDSYNC)
// iGateSelect value required only when moving from village -> game server.
// When moving to a world zone, dungeon, cheat, or trigger in game server, there is no gate selection, so use default (0).
void CDNGameRoom::InitStateAndSync(int iMapIdx, int iGateIdx, int iRandomSeed, TDUNGEONDIFFICULTY StageDifficulty, bool bDirectConnect, int iGateSelect)
{
	if (g_pBackLoader != NULL)
	{
		if (GetFarmIndex() > 0)
		{
			_DANGER_POINT();
			return ;
		}

		DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
		if( pSox && !pSox->IsExistItem( iMapIdx ) ) {
			DestroyGameRoom();
			return;
		}

		InitEvent();
#if defined(PRE_ADD_WORLD_EVENT)
		TEvent * pEvent = GetApplyEventType(WorldEvent::EVENT5);		
#else
		TEvent * pEvent = GetApplyEvent(_EVENT_1_DROPITEM);
#endif //#if defined(PRE_ADD_WORLD_EVENT)

		for( DWORD i=0; i<GetUserCount(); i++ ) {
			GetUserData(i)->SetMapIndex( iMapIdx );
			GetUserData(i)->SendStartStage( bDirectConnect, iMapIdx, iGateIdx, iRandomSeed, StageDifficulty, pEvent == NULL ? 0 : pEvent->nAtt1 );
			GetUserData(i)->ChangeStageUserData();
			GetUserData(i)->SetLoadingComplete(false);

			//Do not send HackShield data during mid-game loading, as gate transitions mid-game can drop it on the client -2hogi
			GetUserData(i)->SetSecurityUpdateFlag(false);
			// GetUserData(i)->UpdateUserData();
		}

		m_iMapIdx = iMapIdx;
		m_iGateIdx = iGateIdx;
		m_iGateSelect = iGateSelect;
		m_StageDifficulty = StageDifficulty;
		m_iRandomSeed = iRandomSeed;
		m_bDirectConnect = bDirectConnect;
		
		if (g_pBackLoader->PushToLoadProcess( this, true ) == false)
		{
			//This shouldn't happen! It hasn't even started, just kill the room
			g_Log.Log(LogType::_ERROR, L"Destroy|InitStateAndSync LoadError\n");
			DestroyGameRoom();
		}
		else
		{
			m_GameState = _GAME_STATE_SYNC2SYNC;
			m_iNextGameState = GetGameTick() + GOGO_SING_TO_PLAY_FOR_WAIT_TIME_LIMIT;
		}

		SendSeqLevel();
	}
	else
	{
		g_Log.Log(LogType::_ERROR, L"Destroy|BackLoader Not Found\n");
		DestroyGameRoom();
	}
}
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)

bool CDNGameRoom::LoadData(bool bContinue)
{//Called externally without holding lock (please do not acquire lock when modifying). Be careful when changing.
	bool bResult;
	if (bContinue == false)
	{
		PROFILE_TICK_TEST_BLOCK_START( "LoadData" );
		m_pItemTask->InitializePlayerItem();
		bResult = m_pGameTask->InitializeStage( m_iMapIdx, m_iGateIdx, m_StageDifficulty, m_iRandomSeed, bContinue, m_bDirectConnect, m_iGateSelect );
		if( !bResult )		
		{
			g_Log.Log(LogType::_REMAINEDACTOR, _T("m_pGameTask->InitializeStage Failed\n"));
			return false;
		}

		bResult = m_pGameTask->InitializePlayerActor();
		if( !bResult )
		{
			g_Log.Log(LogType::_REMAINEDACTOR, _T("m_pGameTask->InitializePlayerActor Failed\n"));
			return false;
		}
		bResult = m_pGameTask->PostInitializeStage( m_iRandomSeed );
		if( !bResult )
		{
			g_Log.Log(LogType::_REMAINEDACTOR, _T("m_pGameTask->PostInitializeStage Failed\n"));
			return false;
		}	
		PROFILE_TICK_TEST_BLOCK_END();
	}	
	else {
		PROFILE_TICK_TEST_BLOCK_START( "LoadData" );
		bResult = m_pGameTask->InitializeStage( m_iMapIdx, m_iGateIdx, m_StageDifficulty, m_iRandomSeed, bContinue, m_bDirectConnect, m_iGateSelect );
		if( !bResult ){
			g_Log.Log(LogType::_REMAINEDACTOR, _T("m_pGameTask->InitializeStage(bContinue==true) Failed\n"));
			return false;
		}
		PROFILE_TICK_TEST_BLOCK_END();
	}
	return true;
}

CDNUserSession * CDNGameRoom::CreateGameSession(WCHAR * wszAccountName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bIsMaster, bool bTutorial, bool bAdult, 
												char cPCBangGrade, TMemberVoiceInfo * pInfo)
{
	CDNUserSession * pSession = NULL;
	if (m_GameState == _GAME_STATE_READY2CONNECT)
	{//Currently session creation and connect are only allowed in standby state. Later adjust so party members can rejoin etc. (toward keeping the session object)
		pSession = new IBoostPoolDNGameSession( nSessionID, m_pGameServer, this );

		if (pSession)
		{
			pSession->PreInitializeUser(wszAccountName, nAccountDBID, nSessionID, biCharacterDBID, iTeam, nWorldID, nVillageID, bTutorial, bAdult, cPCBangGrade, pInfo);
		}
		else	
			return pSession;

#if defined( STRESS_TEST )
		PartyStruct Party;
		Party.pSession = pSession;
		Party.bLeader = bIsMaster;
	
		if( nAccountDBID == 0 )
		{
			for( UINT i=0 ; i<4 ; ++i )
				m_VecMember.push_back(Party);
		}
		else
		{
			m_VecMember.push_back(Party);
		}
#else
		AddPartyStruct( pSession, bIsMaster );
#endif
		m_UserList.push_back(pSession);
	}	
	else
		g_Log.Log(LogType::_ERROR, nWorldID, nAccountDBID, biCharacterDBID, 0, _T("Connect|GameStart Flow err [state:addmember]\n"));

	return pSession;
}

bool CDNGameRoom::VerifyMember(CDNUserSession * pSession)
{//When master requests adding a member
	for (unsigned int i = 0; i < m_VecMember.size(); i ++)
		if (m_VecMember[i].pSession == pSession)
			return true;
	return false;
}

void CDNGameRoom::RemoveMember(CDNUserSession * pSession, wchar_t * pwszIdent)
{//Delete member; if everyone leaves, change to destroy state
	m_DeleteList.push_back(pSession->GetSessionID());
	OnLeaveUser(pSession->GetSessionID());

	g_Log.Log(LogType::_NORMAL, pSession, L"[%d] RemoveMember(%s) SessionState=%d RoomState=%d\r\n", g_Config.nManagedID, pwszIdent == NULL ? L"NULL" : pwszIdent, pSession->GetState(), GetRoomState());
}

void CDNGameRoom::CheckRemovedMember()
{
#ifdef STRESS_TEST
	return;
#endif
	if (m_DeleteList.empty() ) 
		return;

	// Wait and clean up user here due to sync issue with BackGround thread.
	//if( m_GameState == _GAME_STATE_SYNC2SYNC || m_GameState == _GAME_STATE_LOAD2SYNC )
	if( g_pBackLoader && g_pBackLoader->IsLoading( GetRoomID() ) )
		return;

	DNVector(PartyStruct)::iterator ii;
	for (int i = 0; i < (int)m_DeleteList.size(); i++)
	{
		bool bDelFalg = false;	// Flag indicating whether m_VecMember.erase() was called
		// Handle disconnected sessions before pushing back to m_VecMember
		CDNUserSession* pDeleteSession = NULL;

		// Handle disconnect of drop-in user
		for( std::list<CDNUserSession*>::iterator bi=m_BreakIntoUserList.begin() ; bi!=m_BreakIntoUserList.end() ;  )
		{
			if( (*bi)->GetSessionID() == m_DeleteList[i] )
			{
				pDeleteSession = (*bi);
				bi = m_BreakIntoUserList.erase( bi );
				break;
			}
			else
				++bi;
		}		

		for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
		{
			if ((*ii).pSession->GetSessionID() == m_DeleteList[i])
			{
#ifndef _FINAL_BUILD
				// Adding validation because something already deleted elsewhere is being deleted again.
				for( int j=EQUIP_FACE; j<=EQUIP_RING2; j++ ) {
					if( !(*ii).pEquip[j] ) continue;
					for( int k=0; k<INVENTORYMAX; k++ ) {
						if( !(*ii).pInventory[k] ) continue;
						if( (*ii).pEquip[j] == (*ii).pInventory[k] ) {
							_ASSERT(0&&"Unexpected null state in DNGameRoom");
						}
					}
				}
				/////////////////////////////////////////////////////
#endif
				pDeleteSession = NULL;

				m_vDeleteSession.push_back( (*ii).pSession );
				m_UserList.remove( (*ii).pSession );
				(*ii).pSession->FinalUser();
				GetGameServer()->RemoveConnection(GetRoomID(), (*ii).pSession->GetNetID(), (*ii).pSession->GetAccountDBID(), (*ii).pSession->GetSessionID());

				(*ii).ReleaseEquipInventory();

				CheckCurrentItemLooterIdx( &(*ii) );
				std::map<INT64,CDNUserSession*>::iterator itor = m_mCharDBIDMember.find( (*ii).pSession->GetCharacterDBID() );
				if( itor != m_mCharDBIDMember.end() )
					m_mCharDBIDMember.erase( itor );
				m_VecMember.erase(ii);
				bDelFalg = true;
				break;
			}
		}

		if( bDelFalg && GetMasterRewardSystem() )
			GetMasterRewardSystem()->RequestRefresh();

		if( bDelFalg == false && pDeleteSession == NULL )
		{
			for( std::list<CDNUserSession*>::iterator itor=m_UserList.begin() ; itor!=m_UserList.end() ; ++itor )
			{
				CDNUserSession* pZombieSession = (*itor);
				if( pZombieSession )
				{
					if( pZombieSession->GetSessionID() == m_DeleteList[i] )
					{
						g_Log.Log( LogType::_ZOMBIEUSER, pZombieSession, L"[%d] RoomID:%d", g_Config.nManagedID, GetRoomID() );
						pZombieSession->FinalUser();
						GetGameServer()->RemoveConnection(GetRoomID(), pZombieSession->GetNetID(), pZombieSession->GetAccountDBID(), pZombieSession->GetSessionID());
					}
				}
			}
		}

		if( pDeleteSession )
		{
			m_UserList.remove( pDeleteSession );
			pDeleteSession->FinalUser();
			GetGameServer()->RemoveConnection(GetRoomID(), pDeleteSession->GetNetID(), pDeleteSession->GetAccountDBID(), pDeleteSession->GetSessionID());
		}
	}
	m_DeleteList.clear();

	if( GetRealUserCountWithoutGM() == 0 && HasAIBots() )
		DespawnAndRemoveAIBots( false );

	if( m_VecMember.empty() )
	{
		//For a farm, the room does not close even if users leave
#if defined( PRE_WORLDCOMBINE_PARTY )
		if (bIsFarmRoom() == false && CheckDestroyWorldCombineParty() == true )
#else
		if (bIsFarmRoom() == false)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
		{
			bool bDestroy = true;
			if (bIsPvPRoom())
			{
				if (bIsGuildWarSystem() && m_bForceDestroyRoom == false)
				{
					if (GetPvPGameMode() && GetPvPGameMode()->bIsFinishFlag() == false)
						bDestroy = false;
				}
#if defined( PRE_WORLDCOMBINE_PVP )
				if( bIsWorldPvPRoom() && bIsWorldPvPRoomStart() == false && GetWorldPvPRoomReqType() != WorldPvPMissionRoom::Common::GMRoom )
					bDestroy =false;				
#endif
			}

			if (bDestroy)
				DestroyGameRoom();
		}
	}
	else
	{
		if (m_pTaskMng)
		{
			CDnPartyTask * pPartyTask = (CDnPartyTask*)m_pTaskMng->GetTask("PartyTask");
			if (pPartyTask)
				pPartyTask->UpdateGateInfo();
			else _DANGER_POINT();
		}

#if defined( PRE_PARTY_DB )
		if( GetUserCount() <= 1 ) m_PartyStructData.iUpkeepCount = 0;
#else
		if( GetUserCount() <= 1 ) m_nUpkeepCount = 0;
#endif
		UpdateAppliedEventValue();
	}
}

void CDNGameRoom::CheckRudpDisconnectedMember()
{
	//Prompted clients with active TCP but disconnected RUDP to reconnect; disconnect those that fail to reconnect within a set time.
	DNVector(PartyStruct)::iterator ii;
	for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
		if ((*ii).pSession->IsRudpDisconnected() && GetGameTick() - (*ii).pSession->GetRudpDisconnectedTick() > (5*1000)) //About 5 seconds?
			(*ii).pSession->DetachConnection(L"CheckRudpDisconnectedMember");
}

void CDNGameRoom::UserUpdate( DWORD dwCurTick )
{
#ifdef PRE_MOD_INDUCE_TCPCONNECT
	if (m_GameState == _GAME_STATE_SYNC2PLAY || m_GameState == _GAME_STATE_PLAY || \
		m_GameState == _GAME_STATE_READY2CONNECT || m_GameState == _GAME_STATE_CONNECT2CHECKAUTH)
#else		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
	if (m_GameState == _GAME_STATE_SYNC2PLAY || m_GameState == _GAME_STATE_PLAY)
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			CDNUserSession* pSession = m_VecMember[i].pSession;
			if( pSession )
				pSession->DoUpdate( dwCurTick );
		}
	}

	if (m_nInivitedTime > 0 && m_nInivitedTime + (GAMEINVITEWAITTIME) < timeGetTime())
	{
		ResetInvite(ERROR_PARTY_INVITEFAIL);
	}
}

void CDNGameRoom::AddGameListener(CGameListener * pListener)
{
	m_GameListener.push_back(pListener);
}

void CDNGameRoom::RemoveGameListener(CGameListener * pListener)
{
	std::vector <CGameListener*>::iterator ii;
	for (ii = m_GameListener.begin(); ii != m_GameListener.end(); ii++)
	{
		if ((*ii) == pListener)
		{
			m_GameListener.erase(ii);
			return;
		}
	}
}

void CDNGameRoom::_AddPacketQueue( CDNUserSession* pSession, const DWORD dwUniqueID, const BYTE cSubCmd, const BYTE* pBuffer, const int iSize, const int iPrior )
{
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession == pSession )
			continue;

		if( bIsFarmRoom() )
		{
			// Farm session sends only in SESSION_STATE_GAME_PLAY to avoid unreachable state from excessive fishing Actor packets.
			if( m_VecMember[i].pSession->GetState() != SESSION_STATE_GAME_PLAY )
			{
				bool bSkip = false;
				switch( cSubCmd )
				{
					case eActor::CS_FISHINGROD_CAST:
					case eActor::CS_FISHINGROD_LIFT:
					case eActor::CS_FISHINGROD_HIDE:
					{
						bSkip = true;
					}
				}

				if( bSkip )
					continue;
			}
		}
	
//#ifdef PRE_ADD_PACKETSIZE_CHECKER
//		m_VecMember[i].pSession->AddPacketQueue( dwUniqueID, cSubCmd, pBuffer, iSize, iPrior, true );
//#else		//#ifdef PRE_ADD_PACKETSIZE_CHECKER
		m_VecMember[i].pSession->AddPacketQueue( dwUniqueID, cSubCmd, pBuffer, iSize, iPrior );
//#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER
	}
}

int CDNGameRoom::OnDispatchMessage(CDNUserSession * pSession, int iMainCmd, int iSubCmd, char * pData, int iLen, BYTE cSeqLevel)
{
#ifdef _FINAL_BUILD
	__try { 
#endif

	/*
	if( g_pBackLoader && g_pBackLoader->IsLoading( GetRoomID() ) )
	{
		if( !(iMainCmd == CS_ACTOR && iSubCmd == eActor::CS_CMDSTOP) )
			g_Log.Log( LogType::_ERROR, pSession, L"[%d] BackGroundLoading MCMD[%d] SCMD[%d]\n", g_Config.nManagedID, iMainCmd, iSubCmd );
	}
	*/

	if (m_GameState == _GAME_STATE_DESTROYED) return ERROR_NONE;
	if (m_GameState == _GAME_STATE_SYNC2SYNC)
	{
		bool bSkip = true;
		switch( iMainCmd )
		{
			case CS_ROOM:
			case CS_SYSTEM:
			{
				bSkip = false;
				break;
			}
			case IN_SC_ITEM:
			{
				if( iSubCmd == eItem::SC_REFRESHINVEN  )
					bSkip = false;
				break;
			}
		}

		if( bSkip )
		{
			//For checking what message arrives at this timing
			//If you need to skip this part, please notify Myungjae.
#ifndef _FINAL_BUILD
//			if( !(iMainCmd == CS_ACTOR && iSubCmd == eActor::CS_CMDSTOP) )
//				g_Log.Log( LogType::_ERROR, pSession, L"MapIndex[%d] MCMD[%d] SCMD[%d]\n", pSession->GetMapIndex(), iMainCmd, iSubCmd);
#endif
			return ERROR_NONE;
		}
	}

	if (iMainCmd == CS_ACTOR || iMainCmd == CS_ACTORBUNDLE)
	{
		//Process actor messages only in play state. Receiving them during movement or invalid timing causes position desync.
		if ((m_GameState != _GAME_STATE_PLAY) || cSeqLevel != m_cSeqLevel)
			return ERROR_NONE;
	}

	switch (iMainCmd)
	{
		case CS_SYSTEM:	return pSession->OnRecvSystemMessage(iSubCmd, pData, iLen);
		case CS_FRIEND: return pSession->OnRecvFriendMessage(iSubCmd, pData, iLen);
		case CS_ISOLATE: return pSession->OnRecvIsolateMessage(iSubCmd, pData, iLen);
		case CS_GAMEOPTION: return pSession->OnRecvGameOptionMessage(iSubCmd, pData, iLen);
		case CS_RADIO: return pSession->OnRecvRadioMessage(iSubCmd, pData, iLen);
		case CS_ETC: return pSession->OnRecvEtcMessage(iSubCmd, pData, iLen);
//		case CS_GUILD: return pSession->OnRecvGuildMessage(iSubCmd, pData, iLen); // Guild messages on game server are handled in CDnGuildTask::OnRecvGuildMessage(...) (20100210 b4nfter)
#ifdef _USE_VOICECHAT
		case CS_VOICECHAT: return pSession->OnRecvVoiceChatMessage(iSubCmd, pData, iLen);
#endif
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		case CS_REPUTATION: return pSession->OnRecvReputationMessage( iSubCmd, pData, iLen );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		case CS_MASTERSYSTEM: return pSession->OnRecvMasterSystemMessage( iSubCmd, pData, iLen );
#if defined( PRE_ADD_SECONDARY_SKILL )
		case CS_SECONDARYSKILL: return pSession->OnRecvSecondarySkillMessage( iSubCmd, pData, iLen );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#if defined( PRE_PRIVATECHAT_CHANNEL )
		case CS_PRIVATECHAT_CHANNEL: return pSession->OnRecvPrivateChatChannelMessage( iSubCmd, pData, iLen );
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
		case CS_ALTEIAWORLD: return pSession->OnRecvWorldAlteiaMessage( iSubCmd, pData, iLen );
#endif
#if defined( PRE_ADD_DWC )
		case CS_DWC: return pSession->OnRecvDWCMessage( iSubCmd, pData, iLen );
#endif
#if defined(PRE_ADD_CHAT_MISSION)
		case CS_MISSION: return pSession->OnRecvMissionMessage(iSubCmd, pData, iLen);
#endif
		case CS_ACTOR:
		{
			CSActorMessage * pActorMsg = reinterpret_cast<CSActorMessage*>(pData);

			/*if (sizeof(CSActorMessage) - sizeof(pActorMsg->cBuf) + pActorMsg->cSize != iLen)
				return ERROR_INVALIDPACKET;*/

			_AddPacketQueue( pSession, pActorMsg->nSessionID, iSubCmd, reinterpret_cast<const BYTE*>(pActorMsg->cBuf), iLen, _FAST );
			break;
		}
		case CS_ACTORBUNDLE:
		{
			CSActorBundleMessage *pBundlePacket = reinterpret_cast<CSActorBundleMessage*>(pData);

			/*if (sizeof(CSActorBundleMessage) - sizeof(pBundlePacket->cBuf) + (pBundlePacket->cSize) != iLen)
				return ERROR_INVALIDPACKET;*/

			int nOffset = 0;
			DWORD	dwUniqueID;
			BYTE	cSubCmd;
			WORD	wSize;
			void*	pLocalData;

			int iDataLen = iLen-sizeof(pBundlePacket->nSessionID);
			if( iDataLen < 0 )
				return ERROR_INVALIDPACKET;

			for( int i=0; i<iSubCmd; ++i ) 
			{
				if( iDataLen < sizeof(DWORD) )
					return ERROR_INVALIDPACKET;
				memcpy( &dwUniqueID, pBundlePacket->cBuf+nOffset, sizeof(DWORD) );	nOffset += sizeof(DWORD);
				iDataLen -= sizeof(DWORD);

				if( iDataLen < sizeof(BYTE) )
					return ERROR_INVALIDPACKET;
				memcpy( &cSubCmd, pBundlePacket->cBuf+nOffset, sizeof(BYTE) );		nOffset += sizeof(BYTE);
				iDataLen -= sizeof(BYTE);

				if( iDataLen < sizeof(WORD) )
					return ERROR_INVALIDPACKET;
				memcpy( &wSize, pBundlePacket->cBuf+nOffset, sizeof(WORD) );		nOffset += sizeof(WORD);
				iDataLen -= sizeof(WORD);

				if( iDataLen < wSize )
					return ERROR_INVALIDPACKET;
				pLocalData = pBundlePacket->cBuf + nOffset;								nOffset += wSize;
				iDataLen -= wSize;
				
				_AddPacketQueue( pSession, dwUniqueID, cSubCmd, static_cast<const BYTE*>(pLocalData), wSize, _FAST );
			}

			break;
		}
	}

	for (int i = 0; i < (int)m_GameListener.size(); i++)
		m_GameListener[i]->OnDispatchMessage(pSession, iMainCmd, iSubCmd, pData, iLen);

	/*
Allow catching Windows SEH exceptions as well
Compile with Enable C++ Exceptions set to /EHa option.
	*/
#ifdef _FINAL_BUILD
	}  // __try {  
	__except(CExceptionReport::GetInstancePtr()->Proc(GetExceptionInformation(), MiniDumpNormal))
	{
		// Disconnect here if an exception occurs while handling user packets.
		g_Log.Log(LogType::_SESSIONCRASH, L"[%d] Session Crash! RoomID=%d\n", g_Config.nManagedID, m_iRoomID );
		pSession->SetDecreaseFatigue(0);
		//pSession->SetSessionState(SESSION_STATE_CRASH);
		pSession->DetachConnection(L"SESSION_STATE_CRASH");//L"SESSION_STATE_CRASH");

	}

#endif
	return ERROR_NONE;
}

CDNUserSession * CDNGameRoom::GetUserData(DWORD dwIndex) 
{
	if( dwIndex >= (DWORD)m_VecMember.size() )
		return NULL;
	return m_VecMember[dwIndex].pSession;
}

CDNUserSession * CDNGameRoom::GetUserSession(UINT nSessionID)
{
	DNVector(PartyStruct)::iterator ii;
	for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
		if ((*ii).pSession->GetSessionID() == nSessionID)
			return (*ii).pSession;
	return NULL;
}


//PartyData
bool CDNGameRoom::AddPartyStruct( CDNUserSession* pSession, bool bLeader)
{
	if( bIsFarmRoom() == false )
	{
		UINT uiMax = bIsPvPRoom() ? PvPCommon::Common::MaxPlayer : PARTYCOUNTMAX;

		if (pSession->bIsGMTrace())
			uiMax = PARTYMAX;

		if( m_VecMember.size() >= uiMax )
			return false;
	}

	PartyStruct Party;
	Party.pSession	= pSession;
	Party.bLeader	= bLeader;

	m_VecMember.push_back( Party );
	m_mCharDBIDMember.insert( std::make_pair(pSession->GetCharacterDBID(),pSession) );

	AddLastPartyDungeonInfo( pSession );
	return true;
}

CDNGameRoom::PartyStruct * CDNGameRoom::GetPartyData(CDNUserSession * pSession)
{ 
	for( DWORD i=0; i<m_VecMember.size(); i++ ) 
		if( m_VecMember[i].pSession == pSession) 
			return &m_VecMember[i]; 
	return NULL; 
}

CDNGameRoom::PartyStruct * CDNGameRoom::GetPartyData( WCHAR *pwszCharacterName )
{
	for( DWORD i=0; i<m_VecMember.size(); i++ )
		if( __wcsicmp_l( m_VecMember[i].pSession->GetCharacterName(), pwszCharacterName ) == 0 )
			return &m_VecMember[i];
	return NULL;
}

CDNGameRoom::PartyStruct * CDNGameRoom::GetPartyDatabySessionID(UINT nSessionID, int &Seq)
{
	for( DWORD i=0; i<m_VecMember.size(); i++ )
		if (m_VecMember[i].pSession->GetSessionID() == nSessionID)
		{
			Seq = i;
			return &m_VecMember[i];
		}
	return NULL;
}

CDNUserSession* CDNGameRoom::GetUserSessionByCharDBID( INT64 biCharDBID )
{
	std::map<INT64,CDNUserSession*>::iterator itor = m_mCharDBIDMember.find( biCharDBID );
	if( itor != m_mCharDBIDMember.end() )
		return (*itor).second;

	return NULL;
}

bool CDNGameRoom::SwapLeader(UINT nDelSessionID, UINT &nNewLeaderSessionID)
{
	bool bCheck, bNewLeader;
	bCheck = bNewLeader = false;

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].pSession->GetSessionID() == nDelSessionID)
		{
			if (m_VecMember[i].bLeader == true && m_VecMember.size() > 1)
			{
				m_VecMember[i].bLeader = false;
				bNewLeader = true;
			}
			bCheck = true;
			break;
		}
	}

#if defined( PRE_PARTY_DB )
	CDNUserSession* pNewLeaderSession = NULL;
#endif // #if defined( PRE_PARTY_DB )

	if (bNewLeader)
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ )
		{
			if (m_VecMember[i].pSession->GetSessionID() != nDelSessionID && nNewLeaderSessionID == 0 && m_VecMember[i].pSession->bIsGMTrace() == false && m_VecMember[i].pSession->bIsAIBot() == false && m_VecMember[i].pSession->GetOutedMember() == false)
			{
				m_VecMember[i].bLeader = true;
				nNewLeaderSessionID = m_VecMember[i].pSession->GetSessionID();
#if defined( PRE_PARTY_DB )
				pNewLeaderSession = m_VecMember[i].pSession;
#endif // #if defined( PRE_PARTY_DB )
			}
			else
				m_VecMember[i].bLeader = false;
		}

		for( DWORD i=0; i<m_VecMember.size(); i++ )
		{
			//If leader changed, notify all users of the change
			m_VecMember[i].pSession->SendUpdatePartyUI(nDelSessionID, false);
			m_VecMember[i].pSession->SendUpdatePartyUI(nNewLeaderSessionID, true);
		}		
	}	

	if( pNewLeaderSession )
	{
		m_PartyStructData.biLeaderCharacterDBID = pNewLeaderSession->GetCharacterDBID();
#if defined( PRE_PARTY_DB )
		GetDBConnection()->QueryModPartyLeader( pNewLeaderSession, GetPartyIndex() );
#endif // #if defined( PRE_PARTY_DB )
	}
	
	return bCheck;
}

bool CDNGameRoom::IsPartyLeader(UINT nSessionID)
{
	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].pSession->GetSessionID() == nSessionID && m_VecMember[i].bLeader == true)
			return true;
	}
	return false;
}

int CDNGameRoom::GetLeftMemberIndex()
{
	for (int i = 0; i < PARTYMAX; i++)
	{
		if (m_nPartyMemberIndex[i] <= 0)
			return i;
	}
	return -1;
}

bool CDNGameRoom::SetPartyMemberIndex(int nIdx, UINT nSessionID, int nTeam /*= -1*/)
{
	if (GetPartyIndex() > 0)
		m_nPartyMemberIndex[nIdx] = nSessionID;
	else if (bIsPvPRoom() && bIsOccupationMode())
	{
		if (nTeam == -1) return false;
		if (nTeam == PvPCommon::Team::A)
			m_nPartyMemberIndex[nIdx] = nSessionID;
		else
		{
			if (GetPvPMaxUser() <= 0)
			{
				_DANGER_POINT();
				return false;
			}
			m_nPartyMemberIndex[nIdx+(GetPvPMaxUser()/2)] = nSessionID;
		}
	}
	return true;
}
bool CDNGameRoom::PartySwapMemberIndex(CSPartySwapMemberIndex * pPacket)
{
	if( bIsFarmRoom() )
		return true;

	//verify count
#if defined( PRE_PARTY_DB )
	if (pPacket->cCount > m_PartyStructData.nPartyMaxCount )
		return false;							//You must not do this.
#else
	if (pPacket->cCount > m_nMemberMax)
		return false;							//You must not do this.
#endif

	//verify index range
	std::vector <BYTE> vDuplicate;
	std::vector <BYTE>::iterator iDuplicater;
	for (int i = 0; i < pPacket->cCount; i++)
	{
#if defined( PRE_PARTY_DB )
		if (pPacket->Index[i].cIndex > (m_PartyStructData.nPartyMaxCount-1))
			return false;
#else
		if (pPacket->Index[i].cIndex > (m_nMemberMax-1))
			return false;
#endif

		iDuplicater = std::find(vDuplicate.begin(), vDuplicate.end(), pPacket->Index[i].cIndex);
		if (vDuplicate.end() != iDuplicater)
			return false;						//There shouldn't be duplicate indices

		vDuplicate.push_back(pPacket->Index[i].cIndex);
	}

	//Create backup copy
	UINT nPartyMemberIndex[PARTYMAX];
	memcpy(nPartyMemberIndex, m_nPartyMemberIndex, sizeof(m_nPartyMemberIndex));
	memset(m_nPartyMemberIndex, 0, sizeof(m_nPartyMemberIndex));

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		bool bCheck = false;
		for (int j = 0; j < PARTYMAX; j++)
		{
			if (pPacket->Index[j].nSessionID == m_VecMember[i].pSession->GetSessionID())
			{
				m_VecMember[i].pSession->SetPartyMemberIndex(pPacket->Index[j].cIndex);
				m_nPartyMemberIndex[m_VecMember[i].pSession->GetPartyMemberIndex()] = m_VecMember[i].pSession->GetSessionID();
				bCheck = true;
				break;
			}
		}

		if (bCheck == false)
		{
			//Revert.
			memcpy(m_nPartyMemberIndex, nPartyMemberIndex, sizeof(m_nPartyMemberIndex));
			for( DWORD h=0; h<m_VecMember.size(); h++ )
			{
				for (int k = 0; k < PARTYMAX; k++)
				{
					if (m_nPartyMemberIndex[k] == m_VecMember[h].pSession->GetSessionID())
					{
						m_VecMember[i].pSession->SetPartyMemberIndex(k);
						break;
					}
				}

			}
			return false;
		}
	}

	SendRefreshParty(0, NULL);
	return true;
}

bool CDNGameRoom::IsRaidParty()
{
#if defined( PRE_PARTY_DB )
	return m_PartyStructData.nPartyMaxCount >= RAIDPARTYCOUNTMIN ? true : false;
#else
	return m_nMemberMax >= RAIDPARTYCOUNTMIN ? true : false;
#endif
}

void CDNGameRoom::SortMemberIndex(int nOutIndex)
{
	//if (IsRaidParty() == false || nOutIndex < 0) return; // No need to do this if not a raid party.
	if (nOutIndex < 0) return;		//Not needed unless it is a raid party.
#if defined( PRE_PARTY_DB )
	if (nOutIndex < 0 || m_PartyStructData.nPartyMaxCount-1 < nOutIndex) return;
#else
	if (nOutIndex < 0 || m_nMemberMax-1 < nOutIndex) return;
#endif

	//Verify if it is truly the disconnected index
	if (m_nPartyMemberIndex[nOutIndex] != 0)
		m_nPartyMemberIndex[nOutIndex] = 0;
	else
		return;

	//Max members of normal party is option count.
	int nOffSetRemainIndex = (nOutIndex+1)%NORMPARTYCOUNTMAX;
	if (nOffSetRemainIndex <= 0) return;		//Nothing to sort if remaining count is 0

	int nOffSetIndex = (nOutIndex+1) > NORMPARTYCOUNTMAX ? (nOutIndex)/NORMPARTYCOUNTMAX : 0;
	int nSortEnd = (nOffSetIndex*NORMPARTYCOUNTMAX) + NORMPARTYCOUNTMAX;

	CDNUserSession * pSession = NULL;
	for (int i = nOutIndex; i < (nSortEnd-1); i++)
	{
		if ((i+1) >= PARTYCOUNTMAX) break;
		if (m_nPartyMemberIndex[i+1] <= 0) continue;
		m_nPartyMemberIndex[i] = m_nPartyMemberIndex[i+1];
		m_nPartyMemberIndex[i+1] = 0;
	}

	for (int h = 0; h < PARTYMAX; h++)
	{
		if (m_nPartyMemberIndex[h] <= 0) continue;
		for( DWORD i=0; i<m_VecMember.size(); i++ )
		{
			if (m_VecMember[i].pSession->GetSessionID() == m_nPartyMemberIndex[h])
				m_VecMember[i].pSession->SetPartyMemberIndex(h);
		}
	}
}

#if defined( PRE_FATIGUE_DROPITEM_PENALTY )
int CDNGameRoom::GetFatigueDropRate()
{
	int iUserCount = 0;
	int iDropRate = 0;

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		CDNUserSession* pSession = m_VecMember[i].pSession;

		if( pSession && !pSession->bIsAIBot() && pSession->GetState() == SESSION_STATE_GAME_PLAY )
		{
			++iUserCount;

			if( pSession->bIsNoFatigueEnter() )
			{
				iDropRate += static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FatigueDropItemPenalty_Rate )*100);
			}
			else
			{
				iDropRate += 100;
			}
		}
	}
	
	if( iUserCount <= 0 )
		return 0;

	return iDropRate/iUserCount;
}

#endif // #if defined( PRE_FATIGUE_DROPITEM_PENALTY )

int CDNGameRoom::GetPartyAvrLevel()
{
	int nUserCnt, nMountLv;
	nUserCnt = nMountLv = 0;

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		nMountLv += m_VecMember[i].pSession->GetLevel();
		nUserCnt++;
	}
	return nUserCnt == 0 ? 0 : nMountLv / nUserCnt;
}

void CDNGameRoom::GetLeaderSessionID(UINT &nSessionID)
{
	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].bLeader)
		{
			nSessionID = m_VecMember[i].pSession->GetSessionID();
			return;
		}
	}
}

void CDNGameRoom::SetLeaderSession(UINT nSessionID)
{
	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].pSession->GetSessionID() == nSessionID)
		{
			m_VecMember[i].bLeader = true;
			break;
		}
	}
}

void CDNGameRoom::SendRefreshParty(UINT nSessionID, TProfile * pProfile)
{
	if (GetPartyIndex() <= 0) return;		//0 if not entering party state.

	if (nSessionID > 0)
		m_pPartyTask->RequestPartyMember();

	SPartyMemberInfo Info[PARTYMAX];
	memset(&Info, 0, sizeof(Info));

	int nCount = 0;
	UINT nLeaderSessionID = 0;
	GetLeaderSessionID(nLeaderSessionID);

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].pSession->GetOutedMember()) continue;
		
		m_VecMember[i].pSession->GetPartyMemberInfo(Info[nCount]);
		nCount++;
	}

	if (nCount <= 0 && nLeaderSessionID <= 0)	
		return;

	bool bAvailable = false;
#ifdef _USE_VOICECHAT
	bAvailable = m_nVoiceChannelID[0] > 0 ? true : false;		//Only index 0 exists for now
#endif
#if defined( PRE_WORLDCOMBINE_PARTY )
	int nWorldCombinePartyTableIndex = 0;
	if(Party::bIsWorldCombineParty(m_PartyStructData.Type))
	{
		WorldCombineParty::WrldCombinePartyData* WorldCombinePartyData = g_pDataManager->GetCombinePartyData( m_PartyStructData.nPrimaryIndex );
		if(WorldCombinePartyData)
			nWorldCombinePartyTableIndex = WorldCombinePartyData->cIndex;
	}
#endif

	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
#if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PARTY )
		if (m_VecMember[i].pSession->GetOutedMember()) 
			continue;
		m_VecMember[i].pSession->SendRefreshParty(nLeaderSessionID, nCount, bAvailable, Info, PARTYREFRESH_NONE, &m_PartyStructData, nWorldCombinePartyTableIndex );
#else
		m_VecMember[i].pSession->SendRefreshParty(nLeaderSessionID, m_PartyStructData.wszPartyName, (BYTE)m_PartyStructData.nPartyMaxCount, m_PartyStructData.cMinLevel, nCount, m_PartyStructData.LootRule, m_PartyStructData.LootItemRank, bAvailable, m_PartyStructData.iTargetMapIndex, m_PartyStructData.TargetMapDifficulty, Info, PARTYREFRESH_NONE, m_PartyStructData.Type, m_PartyStructData.iBitFlag, m_PartyStructData.iPassword );
#endif
#else
		m_VecMember[i].pSession->SendRefreshParty(nLeaderSessionID, m_wszPartyName, m_wszPartyPass, m_nMemberMax, m_nUserLvMin, m_nUserLvMax, nCount, m_ItemLootRule, m_ItemLootRank, bAvailable, m_nTargetMapIdx, m_PartyDifficulty, Info, PARTYREFRESH_NONE, m_cIsJobDice, m_nMemberMax >= RAIDPARTYCOUNTMIN ? _RAID_PARTY_8 : _NORMAL_PARTY);
#endif // #if defined( PRE_PARTY_DB )

		if (pProfile)
			m_VecMember[i].pSession->SendDisplayProfile(nSessionID, *pProfile);
	}
}

bool CDNGameRoom::SetInviteCharacterName(const WCHAR * pwszInvitedCharacterName)
{
	if (IsInviting())
		return false;

	_wcscpy(m_wszInvitedCharacterName, _countof(m_wszInvitedCharacterName), pwszInvitedCharacterName, (int)wcslen(pwszInvitedCharacterName));
	m_nInivitedTime = timeGetTime();

	for( DWORD i=0; i<m_VecMember.size(); i++ )
		m_VecMember[i].pSession->SendPartyIniviteNotice(pwszInvitedCharacterName, ERROR_NONE);

	return true;
}

bool CDNGameRoom::IsInviting()
{
	if (m_nInivitedTime == 0 || m_nInivitedTime + (GAMEINVITEWAITTIME) < timeGetTime())		//If 2 minutes have passed since the invite, assume not coming
		return false;
	return true;
}

bool CDNGameRoom::IsInvitingUser(const WCHAR * pwszName)
{
	if (!__wcsicmp_l(m_wszInvitedCharacterName, pwszName))
		return true;
	return false;
}

void CDNGameRoom::ResetInvite(int nRetCode, bool bNotice)
{
	if (bNotice)
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ )
			m_VecMember[i].pSession->SendPartyIniviteNotice(m_wszInvitedCharacterName, nRetCode);
	}

	memset(m_wszInvitedCharacterName, 0, sizeof(m_wszInvitedCharacterName));
	m_nInivitedTime = 0;
}

int CDNGameRoom::AdjustBreakintoUser(const WCHAR * pwszName, UINT nSessionID, int nRetCode, bool bNotice)
{
	int nIndex = -1;
	if (GetPartyIndex() > 0)
	{
		ResetInvite(nRetCode, bNotice);			//Clear invite flag
		//Assign index to incoming user
		nIndex = GetLeftMemberIndex();

		//Handle case where the inviting user leaves the game server during a drop-in
		UINT nLeaderSessionID = 0;
		GetLeaderSessionID(nLeaderSessionID);

		if (nLeaderSessionID <= 0)
			SetLeaderSession(nSessionID);
	}

	return nIndex;
}

// Pass the last DropItem handle created in DropItemList.
DnDropItemHandle CDNGameRoom::RequestItemDropTable( const UINT uiTableID, EtVector3* pPos )
{
	DNVector(CDnItem::DropItemStruct) vDropItem;
	CDnDropItem::CalcDropItemList( this, uiTableID, vDropItem );

	DnDropItemHandle hDropItem;

	for( UINT i=0 ; i<vDropItem.size() ; ++i )
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		hDropItem = m_pItemTask->RequestDropItem( vDropItem[i].dwUniqueID, *pPos, vDropItem[i].nItemID, vDropItem[i].nSeed, vDropItem[i].nCount, 0, -1, vDropItem[i].nEnchantID );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		hDropItem = m_pItemTask->RequestDropItem( vDropItem[i].dwUniqueID, *pPos, vDropItem[i].nItemID, vDropItem[i].nSeed, vDropItem[i].nCount, 0 );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)

	return hDropItem;
}

void CDNGameRoom::AddDropItem( const DWORD dwUniqueID, DnDropItemHandle hDropItem )
{
	m_mDropItem.insert( std::make_pair(dwUniqueID, hDropItem) );
}

void CDNGameRoom::EraseDropItem( const DWORD dwUniqueID )
{
	m_mDropItem.erase( dwUniqueID );
}

DnDropItemHandle CDNGameRoom::FindDropItem( const DWORD dwUniqueID )
{
	std::map<const DWORD,DnDropItemHandle>::iterator itor = m_mDropItem.find( dwUniqueID );
	if( itor == m_mDropItem.end() )
		return CDnDropItem::Identity();

	return itor->second;
}

void CDNGameRoom::RequestChangeMapFromTrigger( int iMapIndex, int iGateNo )
{
	if( m_vChangeMapQueue.empty() )
	{
		m_vChangeMapQueue.push_back( std::make_pair(iMapIndex,iGateNo) );
	}
	else
	{
		_DANGER_POINT();
	}
}

void CDNGameRoom::SendRefreshRebirthCoin(UINT nSessionID, BYTE cRebirthCoin, BYTE cPCBangRebirthCoin, short nCashRebirthCoin, BYTE cVIPRebirthCoin)
{
	for(DWORD i=0; i < m_VecMember.size(); i++)
		m_VecMember[i].pSession->SendRefreshRebirthCoin(nSessionID, cRebirthCoin, cPCBangRebirthCoin, nCashRebirthCoin, cVIPRebirthCoin);
}

#ifdef _USE_PEERCONNECT
void CDNGameRoom::ConnectPeerRequest()
{
	std::vector<PartyStruct>::iterator i1, i2;
	for (i1 = m_VecMember.begin(); i1 != m_VecMember.end(); i1++)
	{
		if ((*i1).pSession->IsConnected())
		for (i2 = m_VecMember.begin(); i2 != m_VecMember.end(); i2++)
		{
			if ((*i1).pSession != (*i2).pSession && (*i2).pSession->IsConnected() && 
				m_PeerManager.IsConnectedPeer((*i1).pSession->GetSessionID(), (*i2).pSession->GetSessionID()) == false)
			{//Notify client to connect.
				//Connect all nearby clients as peers for now, but later we should dynamically connect/disconnect using distance and other factors
				SCPeerConnectRequest packet;
				(*i2).pSession->GetUDPAddr(&packet.nDestAddrIP, &packet.nDestAddrPort);
				packet.nSessionID[0] = (*i1).pSession->GetSessionID();
				packet.nSessionID[1] = (*i2).pSession->GetSessionID();

				(*i1).pSession->SendPacket(SC_SYSTEM, eSystem::SC_PEER_CONNECT_REQUEST, &packet, sizeof(packet), _RELIABLE);
			}
		}
	}
}
#endif

void CDNGameRoom::OnInitGameRoomUser()
{
	if (m_VecMember.empty()) return;

	for(DWORD i=0; i < m_VecMember.size(); i++)
	{
#if defined( STRESS_TEST )
		if( true )
#else
		if (m_VecMember[i].pSession->IsConnected())
#endif
			m_VecMember[i].pSession->InitialUser();
	}
}

void CDNGameRoom::SendConnectedResult()
{
	if (m_VecMember.empty()) return;

	for(DWORD i=0; i < m_VecMember.size(); i++)
		if (m_VecMember[i].pSession->GetState() == SESSION_STATE_LOADED)
			m_VecMember[i].pSession->SendConnectedResult();
}

void CDNGameRoom::SendEquipData( CDNUserSession* pBreakIntoSession )
{
	if( m_VecMember.empty() ) 
		return;

	std::vector<TPartyMemberDefaultParts> vDefaultParts;
	std::vector<TPartyMemberWeaponOrder> vWeaponOreder;
	std::vector<TPartyMemberEquip> vEquip;
	std::vector<TPartyMemberCashEquip> vCashEquip;
	std::vector<TPartyMemberGlyphEquip> vGlyphEquip;
#ifdef PRE_MOD_SYNCPACKET
	std::vector<TPartyVehicle> vVehicle;
	std::vector<TPartyVehicle> vPet;
#else		//#ifdef PRE_MOD_SYNCPACKET
	std::vector<TVehicle> vVehicle;
	std::vector<TVehicle> vPet;
#endif		//#ifdef PRE_MOD_SYNCPACKET

	vDefaultParts.reserve( m_VecMember.size() );
	vWeaponOreder.reserve( m_VecMember.size() );
	vEquip.reserve( m_VecMember.size() );
	vCashEquip.reserve( m_VecMember.size() );
	vGlyphEquip.reserve( m_VecMember.size() );
	vVehicle.reserve( m_VecMember.size() );
	vPet.reserve( m_VecMember.size() );
	
	for( DWORD i=0; i<m_VecMember.size(); i++ ) 
	{
		// DefaultParts
		TPartyMemberDefaultParts DefaultParts;
#ifdef PRE_MOD_SYNCPACKET
		DefaultParts.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET
		DefaultParts.iDefaultPartsIndex[0] = m_VecMember[i].pSession->GetDefaultBody();
		DefaultParts.iDefaultPartsIndex[1] = m_VecMember[i].pSession->GetDefaultLeg();
		DefaultParts.iDefaultPartsIndex[2] = m_VecMember[i].pSession->GetDefaultHand();
		DefaultParts.iDefaultPartsIndex[3] = m_VecMember[i].pSession->GetDefaultFoot();
		vDefaultParts.push_back( DefaultParts );

		// WeaponOrder
		TPartyMemberWeaponOrder WeaponOrder;
#ifdef PRE_MOD_SYNCPACKET
		WeaponOrder.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET
		memcpy(WeaponOrder.cViewCashEquipBitmap, m_VecMember[i].pSession->GetViewCashEquipBitmap(), sizeof(WeaponOrder.cViewCashEquipBitmap));
		vWeaponOreder.push_back( WeaponOrder );

		// Equip
		TPartyMemberEquip Equip;
		memset( &Equip, 0, sizeof(Equip) );

#ifdef PRE_MOD_SYNCPACKET
		Equip.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET

		for( int j=0 ; j<EQUIPMAX; ++j )
		{
			if( m_VecMember[i].pSession->GetItem()->GetEquip(j) == NULL ) 
				continue;

#ifdef PRE_MOD_SYNCPACKET
			Equip.EquipArray[Equip.cCount].cSlotIndex	= j;
			Equip.EquipArray[Equip.cCount].Item			= *(m_VecMember[i].pSession->GetItem()->GetEquip(j));
#else		//#ifdef PRE_MOD_SYNCPACKET
			Equip.EquipArray[Equip.cCount].cSlotIndex	= j;
			Equip.EquipArray[Equip.cCount].Item			= *(m_VecMember[i].pSession->GetItem()->GetEquip(j));
#endif		//#ifdef PRE_MOD_SYNCPACKET

			++Equip.cCount;
		}
		vEquip.push_back( Equip );

		// CashEquip
		TPartyMemberCashEquip CashEquip;
		memset( &CashEquip, 0, sizeof(CashEquip) );

#ifdef PRE_MOD_SYNCPACKET
		CashEquip.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET

		for( int j=0 ; j<CASHEQUIPMAX ; ++j )
		{
			if( m_VecMember[i].pSession->GetItem()->GetCashEquip(j) == NULL ) 
				continue;

#ifdef PRE_MOD_SYNCPACKET
			CashEquip.EquipArray[CashEquip.cCount].cSlotIndex	= j;
			CashEquip.EquipArray[CashEquip.cCount].Item			= *(m_VecMember[i].pSession->GetItem()->GetCashEquip(j));
#else		//#ifdef PRE_MOD_SYNCPACKET
			CashEquip.EquipArray[CashEquip.cCount].cSlotIndex	= j;
			CashEquip.EquipArray[CashEquip.cCount].Item			= *(m_VecMember[i].pSession->GetItem()->GetCashEquip(j));
#endif		//#ifdef PRE_MOD_SYNCPACKET

			CashEquip.cCount++;
		}
		vCashEquip.push_back( CashEquip );

		// Glyph
		TPartyMemberGlyphEquip GlyphEquip;
		memset( &GlyphEquip, 0, sizeof(GlyphEquip) );

#ifdef PRE_MOD_SYNCPACKET
		GlyphEquip.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET

		for( int j=0 ; j<GLYPHMAX ; ++j ) 
		{
			if( m_VecMember[i].pSession->GetItem()->GetGlyph(j) == NULL ) 
				continue;
#ifdef PRE_MOD_SYNCPACKET
			GlyphEquip.EquipArray[GlyphEquip.cCount].cSlotIndex = j;
			GlyphEquip.EquipArray[GlyphEquip.cCount].Item = *(m_VecMember[i].pSession->GetItem()->GetGlyph(j));
#else		//#ifdef PRE_MOD_SYNCPACKET
			GlyphEquip.EquipArray[GlyphEquip.cCount].cSlotIndex = j;
			GlyphEquip.EquipArray[GlyphEquip.cCount].Item = *(m_VecMember[i].pSession->GetItem()->GetGlyph(j));
#endif		//#ifdef PRE_MOD_SYNCPACKET
			GlyphEquip.cCount++;
		}
		vGlyphEquip.push_back( GlyphEquip );
		

		// Vehicle
#ifdef PRE_MOD_SYNCPACKET
		TPartyVehicle Vehicle;
		memset( &Vehicle, 0, sizeof(Vehicle) );
		Vehicle.nSessionID = m_VecMember[i].pSession->GetSessionID();
		if( m_VecMember[i].pSession->GetItem()->GetVehicleEquip() )
			Vehicle.tVehicle = *(m_VecMember[i].pSession->GetItem()->GetVehicleEquip());
#else		//#ifdef PRE_MOD_SYNCPACKET
		TVehicle Vehicle;
		memset( &Vehicle, 0, sizeof(Vehicle) );
		if( m_VecMember[i].pSession->GetItem()->GetVehicleEquip() )
			Vehicle = *(m_VecMember[i].pSession->GetItem()->GetVehicleEquip());
#endif		//#ifdef PRE_MOD_SYNCPACKET
		vVehicle.push_back( Vehicle );

		// Pet
#ifdef PRE_MOD_SYNCPACKET
		TPartyVehicle Pet;
		memset( &Pet, 0, sizeof(Pet) );
		Pet.nSessionID = m_VecMember[i].pSession->GetSessionID();
		if( m_VecMember[i].pSession->GetItem()->GetPetEquip() )
			Pet.tVehicle = *(m_VecMember[i].pSession->GetItem()->GetPetEquip());
#else		//#ifdef PRE_MOD_SYNCPACKET
		TVehicle Pet;
		memset( &Pet, 0, sizeof(Pet) );
		if( m_VecMember[i].pSession->GetItem()->GetPetEquip() )
			Pet = *(m_VecMember[i].pSession->GetItem()->GetPetEquip());
#endif		//#ifdef PRE_MOD_SYNCPACKET
		vPet.push_back( Pet );
	}

	if( pBreakIntoSession )
	{
		pBreakIntoSession->SendDefaultPartsData( vDefaultParts );
		pBreakIntoSession->SendWeaponOrderData( vWeaponOreder );
		pBreakIntoSession->SendEquipData( vEquip );
		pBreakIntoSession->SendCashEquipData( vCashEquip );
		pBreakIntoSession->SendGlyphEquipData( vGlyphEquip );
		pBreakIntoSession->SendVehicleEquipData( vVehicle );
		pBreakIntoSession->SendPetEquipData(vPet);
	}
	else
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ ) 
		{
			m_VecMember[i].pSession->SendDefaultPartsData( vDefaultParts );
			m_VecMember[i].pSession->SendWeaponOrderData( vWeaponOreder );
			m_VecMember[i].pSession->SendEquipData( vEquip );
			m_VecMember[i].pSession->SendCashEquipData( vCashEquip );
			m_VecMember[i].pSession->SendGlyphEquipData( vGlyphEquip );
			m_VecMember[i].pSession->SendVehicleEquipData( vVehicle );
			m_VecMember[i].pSession->SendPetEquipData( vPet );
		}
	}
}


void CDNGameRoom::SendSkillData( CDNUserSession* pBreakIntoSession )
{
	if( m_VecMember.empty() ) 
		return;

	std::vector<TPartyMemberSkill> vSkill;
	vSkill.reserve( m_VecMember.size() );

	for( UINT i=0 ; i<m_VecMember.size() ; ++i ) 
	{
		TPartyMemberSkill MemberSkill;
		memset( &MemberSkill, 0, sizeof(MemberSkill) );

#ifdef PRE_MOD_SYNCPACKET
		MemberSkill.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET
		for( int j=0; j<SKILLMAX; j++ ) 
		{
			if( m_VecMember[i].pSession->GetSkillData()->SkillList[j].nSkillID <= 0 ) 
				continue;

			// Action file optimization work for Jiho; send all party members' skills.
			if( m_VecMember[i].pSession->GetSkillData()->SkillList[j].cSkillLevel < 1 ) 
				continue;

			//// Select and send passive type and enhancement passive skills.
			//if( CDnSkill::GetSkillType( m_VecMember[i].pSession->GetSkillData()->SkillList[j].nSkillID ) != CDnSkill::Passive &&
			//	CDnSkill::GetSkillType( m_VecMember[i].pSession->GetSkillData()->SkillList[j].nSkillID ) != CDnSkill::EnchantPassive )	
			//	continue;

			MemberSkill.SkillArray[MemberSkill.cCount] = m_VecMember[i].pSession->GetSkillData()->SkillList[j];
			++MemberSkill.cCount;
		}
		vSkill.push_back( MemberSkill );
	}

	if( pBreakIntoSession )
	{
		pBreakIntoSession->SendSkillData( vSkill );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
			m_VecMember[i].pSession->SendSkillData( vSkill );
	}
}


void CDNGameRoom::SendEtcData( CDNUserSession *pBreakIntoSession )
{
	if( m_VecMember.empty() ) 
		return;

	std::vector<TPartyEtcData> vEtcData;
	vEtcData.reserve( m_VecMember.size() );

	for( UINT i=0 ; i<m_VecMember.size() ; ++i ) 
	{
		TPartyEtcData EtcData;
		memset( &EtcData, 0, sizeof(EtcData) );

#ifdef PRE_MOD_SYNCPACKET
		EtcData.nSessionID = m_VecMember[i].pSession->GetSessionID();
#endif		//#ifdef PRE_MOD_SYNCPACKET
		EtcData.nSelectAppellation	= m_VecMember[i].pSession->GetSelectAppellation();
		EtcData.nCoverAppellation	= m_VecMember[i].pSession->GetCoverAppellation();
		EtcData.cGMTrace			= m_VecMember[i].pSession->bIsGMTrace() ? 1 : 0;
#if defined(PRE_ADD_VIP)
		EtcData.bVIP				= m_VecMember[i].pSession->IsVIP();
#endif	// #if defined(PRE_ADD_VIP)
		EtcData.cAccountLevel		= m_VecMember[i].pSession->GetAccountLevel();
		vEtcData.push_back(EtcData);
	}

	if( pBreakIntoSession )
	{
		pBreakIntoSession->SendPartyEtcData( vEtcData );
#if defined( PRE_ADD_VIP_FARM )
		if( pBreakIntoSession->GetEffectRepository() )
			pBreakIntoSession->GetEffectRepository()->SendEffectItem();
#endif // #if defined( PRE_ADD_VIP_FARM )
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			m_VecMember[i].pSession->SendPartyEtcData( vEtcData );
#if defined( PRE_ADD_VIP_FARM )
			if( m_VecMember[i].pSession->GetEffectRepository() )
				m_VecMember[i].pSession->GetEffectRepository()->SendEffectItem();
#endif // #if defined( PRE_ADD_VIP_FARM )
		}
	}
}


void CDNGameRoom::SendSecondAuthInfo( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession  )
	{
		pBreakIntoSession->SendSecondAuthInfo( pBreakIntoSession->bIsSetSecondAuthPW(), pBreakIntoSession->bIsSetSecondAuthLock() );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			CDNUserSession* pSession = m_VecMember[i].pSession;
			if( pSession )
			{
				pSession->SendSecondAuthInfo( pSession->bIsSetSecondAuthPW(), pSession->bIsSetSecondAuthLock() );
			}
		}
	}
}

void CDNGameRoom::SendMaxLevelCharacterCount( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession  )
	{
		pBreakIntoSession->SendMaxLevelCharacterCount( pBreakIntoSession->GetMaxLevelCharacterCount() );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			CDNUserSession* pSession = m_VecMember[i].pSession;
			if( pSession )
			{
				pSession->SendMaxLevelCharacterCount( pSession->GetMaxLevelCharacterCount() );
			}
		}
	}	
}

void CDNGameRoom::SendCompleteGameReady( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession )
	{
		if( bIsPvPRoom() )
		{
			pBreakIntoSession->SendStartStage( m_bDirectConnect, m_iMapIdx, m_iGateIdx, m_iRandomSeed, m_StageDifficulty, 0);
		}
		else
		{
			if( m_pGameTask)
			{
#if defined(PRE_ADD_WORLD_EVENT)
				TEvent * pEvent = GetApplyEventType(WorldEvent::EVENT5);		
#else
				TEvent * pEvent = GetApplyEvent(_EVENT_1_DROPITEM);
#endif //#if defined(PRE_ADD_WORLD_EVENT)
				pBreakIntoSession->SendStartStage( true, m_pGameTask->GetMapTableID(), m_pGameTask->GetStartPositionIndex(), m_iRandomSeed, m_StageDifficulty, pEvent == NULL ? 0 : pEvent->nAtt1);
			}
			else
				_DANGER_POINT();
		}
#if defined(PRE_ADD_VIP)
		pBreakIntoSession->SendVIPInfo(pBreakIntoSession->m_nVIPTotalPoint, pBreakIntoSession->m_tVIPEndDate, pBreakIntoSession->m_bVIPAutoPay, pBreakIntoSession->m_bVIP);
#endif	// #if defined(PRE_ADD_VIP)
	}
	else
	{
#if defined(PRE_ADD_WORLD_EVENT)
		TEvent * pEvent = GetApplyEventType(WorldEvent::EVENT5);
#else
		TEvent * pEvent = GetApplyEvent(_EVENT_1_DROPITEM);
#endif //#if defined(PRE_ADD_WORLD_EVENT)
		for( DWORD i=0; i<m_VecMember.size(); i++ )
		{
			m_VecMember[i].pSession->SendStartStage(m_bDirectConnect, m_iMapIdx, m_iGateIdx, m_iRandomSeed, m_StageDifficulty, pEvent == NULL ? 0 : pEvent->nAtt1);

#if defined(PRE_ADD_VIP)
			GetUserData(i)->SendVIPInfo(GetUserData(i)->m_nVIPTotalPoint, GetUserData(i)->m_tVIPEndDate, GetUserData(i)->m_bVIPAutoPay, GetUserData(i)->m_bVIP);
#endif	// #if defined(PRE_ADD_VIP)
		}
	}
}

void CDNGameRoom::SendGuildData( CDNUserSession* pBreakIntoSession )
{
	if (m_VecMember.empty()) {
		return;
	}

	std::vector<TPartyMemberGuild::TPartyMemberGuildView> vMemberGuildView;
	vMemberGuildView.reserve( m_VecMember.size() );

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		TPartyMemberGuild::TPartyMemberGuildView MemberGuildView;
		MemberGuildView.Set( m_VecMember[i].pSession->GetSessionID(), m_VecMember[i].pSession->GetGuildSelfView() );
		vMemberGuildView.push_back( MemberGuildView );
	}

	if (pBreakIntoSession) 
	{
		pBreakIntoSession->SendPartyGuildData( vMemberGuildView );
	}
	else 
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
			m_VecMember[i].pSession->SendPartyGuildData( vMemberGuildView );
	}
}

void CDNGameRoom::SendMasterSystemSimpleInfo( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession )
	{
		pBreakIntoSession->SendMasterSystemSimpleInfo( const_cast<TMasterSystemSimpleInfo&>(pBreakIntoSession->GetMasterSystemData()->SimpleInfo) );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			m_VecMember[i].pSession->SendMasterSystemSimpleInfo( const_cast<TMasterSystemSimpleInfo&>(m_VecMember[i].pSession->GetMasterSystemData()->SimpleInfo) );
		}
	}
}

void CDNGameRoom::SendMasterSystemCountInfo( CDNUserSession* pBreakIntoSession )
{
	MasterSystem::CRewardSystem::TCountInfo CountInfo;

	if( pBreakIntoSession )
	{
		m_pMasterRewardSystem->GetCountInfo( pBreakIntoSession, CountInfo );
		pBreakIntoSession->SendMasterSystemCountInfo( CountInfo.iMasterCount, CountInfo.iPupilCount, CountInfo.iClassmateCount );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			m_pMasterRewardSystem->GetCountInfo( m_VecMember[i].pSession, CountInfo );
			m_VecMember[i].pSession->SendMasterSystemCountInfo( CountInfo.iMasterCount, CountInfo.iPupilCount, CountInfo.iClassmateCount );
		}
	}
}
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
void CDNGameRoom::SendReputationList( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession )
	{
		_ASSERT( pBreakIntoSession->GetReputationSystem() && pBreakIntoSession->GetReputationSystem()->GetEventHandler() );
		pBreakIntoSession->GetReputationSystem()->GetEventHandler()->OnConnect( 0, true );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			_ASSERT( m_VecMember[i].pSession->GetReputationSystem() && m_VecMember[i].pSession->GetReputationSystem()->GetEventHandler() );
			m_VecMember[i].pSession->GetReputationSystem()->GetEventHandler()->OnConnect( 0, true );
		}
	}
}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

#if defined (PRE_ADD_BESTFRIEND)
void CDNGameRoom::SendBestFriendData( CDNUserSession* pBreakIntoSession )
{
	if (m_VecMember.empty())
		return;

	std::vector<TPartyBestFriend> vMemberBestFriend;
	vMemberBestFriend.reserve(m_VecMember.size());

	for (UINT i=0; i<m_VecMember.size(); i++)
	{
		TPartyBestFriend MemberBestFriend = {0,};
		MemberBestFriend.nSessionID = m_VecMember[i].pSession->GetSessionID();

		TBestFriendInfo& Info = m_VecMember[i].pSession->GetBestFriend()->GetInfo();
		MemberBestFriend.biBFItemSerial = Info.biItemSerial;
		_wcscpy(MemberBestFriend.wszBFName, _countof(MemberBestFriend.wszBFName), Info.wszName, (int)wcslen(Info.wszName));
		
		vMemberBestFriend.push_back( MemberBestFriend );
	}

	if (pBreakIntoSession)
	{
		pBreakIntoSession->SendPartyBestFriend( vMemberBestFriend );
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
			m_VecMember[i].pSession->SendPartyBestFriend( vMemberBestFriend );
	}
}
#endif

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNGameRoom::SendPartyEffectSkillItemData( CDNUserSession* pBreakIntoSession )
{
	if ( !pBreakIntoSession || m_VecMember.empty())
		return;			

	for (UINT i=0; i<m_VecMember.size(); i++)
	{
		if( m_VecMember[i].pSession && m_VecMember[i].pSession->GetSessionID() != pBreakIntoSession->GetSessionID() )
		{
			std::vector<TEffectSkillData> vEffectSkill;
			vEffectSkill.clear();
			m_VecMember[i].pSession->GetItem()->GetEffectSkillItem( vEffectSkill);
			pBreakIntoSession->SendEffectSkillItemData(m_VecMember[i].pSession->GetSessionID(), vEffectSkill, false);			
		}		
	}
}
#endif

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
void CDNGameRoom::SendTotalLevelSkillInfo( CDNUserSession* pBreakIntoSession )
{
	if ( !pBreakIntoSession || m_VecMember.empty())
		return;	

	int* nTotalLevelSkillData = pBreakIntoSession->GetTotalLevelSkillData();

	pBreakIntoSession->SendTotalLevel(pBreakIntoSession->GetSessionID(), pBreakIntoSession->GetTotalLevelSkillLevel());	
	for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
	{
		if(g_pDataManager->bIsTotalLevelSkillCashSlot(i))
			pBreakIntoSession->SendTotalLevelSkillCashSlot(pBreakIntoSession->GetSessionID(), i, pBreakIntoSession->bIsTotalLevelSkillCashSlot(i), pBreakIntoSession->GetTotalLevelSkillCashSlot(i));
	}
	pBreakIntoSession->SendTotalLevelSkillList(pBreakIntoSession->GetSessionID(), nTotalLevelSkillData);	
}
#endif


float CDNGameRoom::GetEventExpWhenMonsterDie(float fExp, bool bFriendBonus, char cClassID, BYTE cJobID)
{
	if( fExp <= 0.f )
		return 0.f;

	time_t Time;
	time(&Time);

	float fEvent = 0.0f;
#if defined(PRE_ADD_WORLD_EVENT)	
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime)
		{
			switch ((*ii).nEventType2)
			{
			case WorldEvent::EVENT1 : //1. EXP on monster death (party)			
				{
					if( (*ii).nEventType1 == 0 || (*ii).nEventType1 == cClassID )
					{
						if (GetUserCount() >= (DWORD)(*ii).nAtt1)
							fEvent += (float)((fExp * (float)((float)((*ii).nAtt2)/100)) + 0.5f);
					}
					break;
				}
			case WorldEvent::EVENT2 : //2. EXP on monster death	
				{	
					if( (*ii).nEventType1 == 0 || (*ii).nEventType1 == cClassID )
					{
						fEvent += (float)((fExp * (float)((float)((*ii).nAtt1)/100)) + 0.5f);
					}
					break;
				}
			case WorldEvent::EVENT9 : //9. Party maintenance
				{
#if defined( PRE_PARTY_DB )
					if (m_PartyStructData.iUpkeepCount > 0)
#else
					if (m_nUpkeepCount > 0)
#endif
					{
#if defined( PRE_PARTY_DB )
						int nRate = (*ii).nAtt1 * m_PartyStructData.iUpkeepCount;
#else
						int nRate = (*ii).nAtt1 * m_nUpkeepCount;
#endif
						nRate = nRate > (*ii).nAtt2 ? (*ii).nAtt2 : nRate;
						fEvent += (float)((fExp * (float)((float)(nRate)/100)) + 0.5f);
					}
					break;
				}
			case WorldEvent::EVENT10 : //10. Party with friends
				{
					if (bFriendBonus){
						int nFriendExp = (*ii).nAtt1;
						fEvent += (float)((fExp * (float)((float)(nFriendExp)/100)) + 0.5f);
					}
					break;
				}
			}
		}
	}
#else //#if defined(PRE_ADD_WORLD_EVENT)	
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime && (*ii).nEventType1 == _EVENT_1_EXP && (*ii).nEventType2 == _EVENT_2_MONSTERDIE)
		{
			switch ((*ii).nEventType3)
			{
				case _EVENT_3_NONE: fEvent += (float)((fExp * (float)((float)((*ii).nAtt1)/100)) + 0.5f); break;
				case _EVENT_3_PARTY:
					{
						if (GetUserCount() >= (DWORD)(*ii).nAtt1)
							fEvent += (float)((fExp * (float)((float)((*ii).nAtt2)/100)) + 0.5f);
						break;
					}
				case _EVENT_3_UPKEEPPARTY:
					{
#if defined( PRE_PARTY_DB )
						if (m_PartyStructData.iUpkeepCount > 0)
#else
						if (m_nUpkeepCount > 0)
#endif
						{
#if defined( PRE_PARTY_DB )
							int nRate = (*ii).nAtt1 * m_PartyStructData.iUpkeepCount;
#else
							int nRate = (*ii).nAtt1 * m_nUpkeepCount;
#endif
							nRate = nRate > (*ii).nAtt2 ? (*ii).nAtt2 : nRate;
							fEvent += (float)((fExp * (float)((float)(nRate)/100)) + 0.5f);
						}
						break;
					}
				case _EVENT_3_FRIENDPARTY:
					{
						if (bFriendBonus){
							int nFriendExp = (*ii).nAtt1;
							fEvent += (float)((fExp * (float)((float)(nFriendExp)/100)) + 0.5f);
						}
						break;
					}
				case _EVENT_3_CLASSEXP:
					{	
						int nRate = 0;
						// nAtt1 Class ID, nAtt2 Job ID, nAtt3 Exp %
						if( cClassID == (*ii).nAtt1 )
						{
							if( (*ii).nAtt2 == 0 || cJobID == (*ii).nAtt2 )
								nRate = (*ii).nAtt3;						
						}
						if( nRate )
							fEvent += (float)((fExp * (float)((float)(nRate)/100)) + 0.5f);
						break;
					}
			}
		}
	}
#endif // #if defined(PRE_ADD_WORLD_EVENT)
	return fEvent;
}

#if defined(PRE_ADD_WORLD_EVENT)
int CDNGameRoom::GetEventType(int nType, char cClassID)
{
	time_t Time;
	time(&Time);

	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime)
		{
			if ((*ii).nEventType2 == nType)
			{
				if( (*ii).nEventType1 == 0 || cClassID == 0 || (*ii).nEventType1 == cClassID )
					return (*ii).nAtt1;
			}
		}
	}
	return 0;
}

TEvent * CDNGameRoom::GetApplyEventType(int nType)
{
	time_t Time;
	time(&Time);

	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime && (*ii).nEventType2 == nType)
			return &(*ii);
	}
	return NULL;
}

#else
TEvent * CDNGameRoom::GetApplyEvent(int nEventType, int nEventType2, int nEventType3)
{
	time_t Time;
	time(&Time);

	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime && (*ii).nEventType1 == nEventType && (*ii).nEventType2 == nEventType2 && (*ii).nEventType3 == nEventType3)
			return &(*ii);
	return NULL;
}
#endif

bool CDNGameRoom::GetExtendDropRateIgnoreTime(int &nRate)
{
	//Drop-rate handling is slightly different; on GetEvent, if a drop event exists, skip time check when getting intermediate data.
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
#if defined(PRE_ADD_WORLD_EVENT)
		if ((*ii).nEventType2 == WorldEvent::EVENT5)
#else
		if ((*ii).nEventType1 == _EVENT_1_DROPITEM)
#endif
		{
			TEvent * pEvent = &(*ii);
			nRate = pEvent->nAtt1;
			return true;
		}
	}
	return false;
}

void CDNGameRoom::GetEventExpWhenStageClear(CDNUserSession * pSession, int nCalcVal, BYTE &cCount, sEventStageClearBonus * pBonus)
{
	time_t Time;
	time(&Time);
#if defined(PRE_ADD_WORLD_EVENT)	
	/*
	*/
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime )
		{
			switch ((*ii).nEventType2)
			{
			case WorldEvent::EVENT3 : //3. Stage clear EXP			
				{
					if( (*ii).nEventType1 == 0 || (*ii).nEventType1 == pSession->GetClassID() )
					{
						pBonus[cCount].nType = WorldEvent::EVENT3;
						pBonus[cCount].nClearEventBonusExperience = (int)(((float)nCalcVal * (float)((float)((*ii).nAtt1)/100)) + 0.5f);
						cCount++;
					}					
					break;
				}
			case WorldEvent::EVENT8 ://8. Secondary authentication
				{
					if( pSession->bIsSetSecondAuthPW() && ((*ii).nEventType1 == 0 || (*ii).nEventType1 == pSession->GetClassID()) )
					{
						pBonus[cCount].nType = WorldEvent::EVENT8;
						pBonus[cCount].nClearEventBonusExperience = (int)(((float)nCalcVal * (float)((float)((*ii).nAtt1)/100)) + 0.5f);
						cCount++;
					}					
					break;
				}
			}
		}	
	}
#else // #if defined(PRE_ADD_WORLD_EVENT)
	
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime && (*ii).nEventType1 == _EVENT_1_EXP && (*ii).nEventType2 == _EVENT_2_CLEAR)
		{
			switch ((*ii).nEventType3)
			{
				case _EVENT_3_SECONDAUTH:
					{
						if (pSession->bIsSetSecondAuthPW())
						{
							pBonus[cCount].nType = _EVENT_3_SECONDAUTH;
							pBonus[cCount].nClearEventBonusExperience = (int)(((float)nCalcVal * (float)((float)((*ii).nAtt1)/100)) + 0.5f);
							cCount++;
						}
						break;
					}
				case _EVENT_3_EKEYANDECARD:
					{
						break;
					}
				case _EVENT_3_CLASSEXP:
					{
						
						int nRate = 0;
						// nAtt1 Class ID, nAtt2 Job ID, nAtt3 Exp %
						if( pSession->GetClassID() == (*ii).nAtt1 )
						{
							if( (*ii).nAtt2 == 0 || pSession->GetUserJob() == (*ii).nAtt2 )
								nRate = (*ii).nAtt3;
						}
						if( nRate )
						{
							pBonus[cCount].nType = _EVENT_3_CLASSEXP;
							pBonus[cCount].nClearEventBonusExperience = (int)(((float)nCalcVal * (float)((float)(nRate)/100)) + 0.5f);
							cCount++;
						}
						break;
					}
			}

			if (cCount >= EVENTCLEARBONUSMAX)
				break;
		}
	}
#endif //#if defined(PRE_ADD_WORLD_EVENT)
}


void CDNGameRoom::GetAppliedEventValue(int &nDropRate, int &nUpkeepRate, int &nFriendBonusRate, bool * pUpkeepMax)
{
	time_t Time;
	time(&Time);
	
	DNVector(TEvent)::iterator ii;
	for (ii = m_EventList.begin(); ii != m_EventList.end(); ii++)
	{
		if (Time >= (*ii)._tBeginTime && Time <= (*ii)._tEndTime)
		{
#if defined(PRE_ADD_WORLD_EVENT)
			switch ((*ii).nEventType2)
			{
			case WorldEvent::EVENT5 : //5. Item drop rate boost:
				{
					nDropRate = (*ii).nAtt1;
					break;
				}
			case WorldEvent::EVENT9 : //9. Party maintenance_EVENT_1_EXP:
				{					
					if (GetUserCount() > 1)
					{
#if defined( PRE_PARTY_DB )
						nUpkeepRate = (*ii).nAtt1 * m_PartyStructData.iUpkeepCount;
#else
						nUpkeepRate = (*ii).nAtt1 * m_nUpkeepCount;
#endif
						if (nUpkeepRate > (*ii).nAtt2)
						{
							if (pUpkeepMax)
								*pUpkeepMax = true;
							nUpkeepRate = (*ii).nAtt2;
						}
					}
					else
						nUpkeepRate = 0;
				}
				break;
			case WorldEvent::EVENT10 : //10. Party with friends
				{
					nFriendBonusRate = (*ii).nAtt1;
				}
				break;
			}
#else //#if defined(PRE_ADD_WORLD_EVENT)
			switch ((*ii).nEventType1)
			{
			case _EVENT_1_DROPITEM:
				{
					nDropRate = (*ii).nAtt1;
					break;
				}
			case _EVENT_1_EXP:
				{
					if ((*ii).nEventType2 == _EVENT_2_MONSTERDIE)
					{
						if ((*ii).nEventType3 == _EVENT_3_UPKEEPPARTY)
						{
							if (GetUserCount() > 1)
							{
#if defined( PRE_PARTY_DB )
								nUpkeepRate = (*ii).nAtt1 * m_PartyStructData.iUpkeepCount;
#else
								nUpkeepRate = (*ii).nAtt1 * m_nUpkeepCount;
#endif
								if (nUpkeepRate > (*ii).nAtt2)
								{
									if (pUpkeepMax)
										*pUpkeepMax = true;
									nUpkeepRate = (*ii).nAtt2;
								}
							}
							else
								nUpkeepRate = 0;
						}
						else if ((*ii).nEventType3 == _EVENT_3_FRIENDPARTY)
						{
							nFriendBonusRate = (*ii).nAtt1;
						}
					}
				}
			}
#endif //#if defined(PRE_ADD_WORLD_EVENT)
		}
	}
}

void CDNGameRoom::CheckFriendBonus(UINT nDelSessionID)
{
	//if (m_VecMember.size() <= 1) return;
#if defined(PRE_ADD_WORLD_EVENT)
	if ( GetApplyEventType( WorldEvent::EVENT10 ) == NULL ) return;
#else
	if (GetApplyEvent(_EVENT_1_EXP, _EVENT_2_MONSTERDIE, _EVENT_3_FRIENDPARTY) == NULL) return;
#endif 

	for(DWORD i=0; i < m_VecMember.size(); i++)
	{
		bool bCheck = false;
		if (nDelSessionID != m_VecMember[i].pSession->GetSessionID())		//If leaving, skip check and return false
		{			
			CDNFriend * pFriend = m_VecMember[i].pSession->GetFriend();
			for(DWORD j=0; j < m_VecMember.size(); j++)
			{
				if (m_VecMember[j].pSession->GetSessionID() == nDelSessionID) continue;		//Skip if the leaving player is the check target

				CDNFriend * pTempFriend = m_VecMember[j].pSession->GetFriend();
				if (pTempFriend == NULL) continue;

				if (pFriend->HasFriend(m_VecMember[j].pSession->GetCharacterDBID()) && pTempFriend->HasFriend(m_VecMember[i].pSession->GetCharacterDBID()))
				{
					bCheck = true;
					break;
				}
			}
		}
		m_VecMember[i].pSession->SetFriendBonus(bCheck);
	}
}

#ifdef PRE_ADD_BEGINNERGUILD
void CDNGameRoom::CheckBeginnerGuildBonus()
{
	m_PartyStructData.bPartyBeginnerGuild = false;

	CDNUserSession * pUser = NULL;
	int nBegginerGuildCount = 0;
	DNVector(PartyStruct)::iterator ii;
	CDNGuildBase * pGuild = NULL;
	for(ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
	{
		pUser = (*ii).pSession;
		if (pUser == NULL) continue;
		if (pUser->bIsGMTrace()) continue;

		pUser->m_bPartyBegginerGuild = false;

		if (pUser->GetGuildUID().IsSet() == false)
			continue;
		
		pGuild = g_pGuildManager->At(pUser->GetGuildUID());
		if (pGuild) 
		{
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (TRUE == pGuild->IsEnable())
			{
				if (pGuild->GetInfo()->cGuildType == BeginnerGuild::Type::Beginner && pGuild->IsMemberExist(pUser->GetCharacterDBID()))
				{
					nBegginerGuildCount++;
					pUser->m_bPartyBegginerGuild = true;
				}
			}
#else		//#if !defined( PRE_ADD_NODELETEGUILD )
			if (pGuild->GetInfo()->cGuildType == BeginnerGuild::Type::Beginner && pGuild->IsMemberExist(pUser->GetCharacterDBID()))
			{
				nBegginerGuildCount++;
				pUser->m_bPartyBegginerGuild = true;
			}
#endif		//#if !defined( PRE_ADD_NODELETEGUILD )
		}
	}

	if (nBegginerGuildCount >= 2)
		m_PartyStructData.bPartyBeginnerGuild = true;
}
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

void CDNGameRoom::UpdateAppliedEventValue()
{
	CheckFriendBonus();
#ifdef PRE_ADD_BEGINNERGUILD
	int nDropRate = 0, nUpkeepRate = 0, nFriendRate = 0, nBeginnerGuildRate = 0;
#else		//#ifdef PRE_ADD_BEGINNERGUILD
	int nDropRate = 0, nUpkeepRate = 0, nFriendRate = 0;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
	bool bMax = false;
	GetAppliedEventValue(nDropRate, nUpkeepRate, nFriendRate, &bMax);
#ifdef PRE_ADD_BEGINNERGUILD
	CheckBeginnerGuildBonus();
	nBeginnerGuildRate = m_PartyStructData.bPartyBeginnerGuild == true ? BeginnerGuild::Common::PartyBonusRate : 0;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

	for( DWORD i=0; i<GetUserCount(); i++ )
	{
#ifdef PRE_ADD_BEGINNERGUILD
		GetUserData(i)->SendPartyBonusValue(nUpkeepRate, nFriendRate, bMax, nBeginnerGuildRate);
#else		//#ifdef PRE_ADD_BEGINNERGUILD
		GetUserData(i)->SendPartyBonusValue(nUpkeepRate, nFriendRate, bMax);
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
	}
}

void CDNGameRoom::ResetCompleteExperience()
{
	m_nCompleteExperience = 0;
}

void CDNGameRoom::AddCompleteExperience( int nValue )
{
	m_nCompleteExperience += nValue;
}

int CDNGameRoom::GetCompleteExperience()
{
	return m_nCompleteExperience;
}

void CDNGameRoom::ResetDungeonPlayTime()
{
	m_dwDungeonPlayerTime = 0;
}

void CDNGameRoom::AddDungeonPlayTime( DWORD dwValue )
{
	m_dwDungeonPlayerTime += dwValue;
}

DWORD CDNGameRoom::GetDungeonPlayTime()
{
	return m_dwDungeonPlayerTime;
}

#if defined( PRE_ADD_58761 )
void CDNGameRoom::StartDungeonGateTime(int nGateID)
{
	// Gate start log	
	GetDBConnection()->QueryAddNestGateStartLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), static_cast<BYTE>(GetUserCount()), nGateID );
	m_nDungeonGateID = nGateID;
}

void CDNGameRoom::EndDungeonGateTime(int nGateID, bool bClearFlag/*=false*/)
{	
	//Gate end log

	if (GetDBConnection())
	{
		CDNUserSession * pUser;
		for (int i = 0; i < (int)GetUserCount(); i++)
		{
			pUser = GetUserData(i);
			if (pUser == NULL || pUser->bIsAIBot()) continue;
			GetDBConnection()->QueryAddNestGateClearLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), bClearFlag, pUser->GetCharacterDBID(), pUser->GetUserJob(), pUser->GetLevel());
		}

		GetDBConnection()->QueryAddNestGateEndLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), static_cast<BYTE>(GetRealUserCountWithoutGM()));
	}
	// Initialize
	m_nDungeonGateID = 0;
}

void CDNGameRoom::NestDeathLog(CDNUserSession *pSession, int nMonsterID, int nSkillID, BYTE cCharacterJob, BYTE cCharacterLevel)
{
	if (pSession && pSession->bIsAIBot()) return;
	if (m_nDungeonGateID > 0)
		GetDBConnection()->QueryNestDeathLog( pSession, GetRoomLogIndex(), nMonsterID, nSkillID, cCharacterJob, cCharacterLevel);	
}
#endif

int CDNGameRoom::GetServerID() const
{
	return m_pGameServer->GetServerID();
}

#ifdef _USE_VOICECHAT
void CDNGameRoom::GetUserTalking(TTalkingInfo * pInfo, int &nCount)
{
	CDNUserSession * pSession = NULL;
	DNVector(PartyStruct):: iterator ii;
	for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
	{
		pSession = (*ii).pSession;
		if (pSession->IsTalking(&pInfo[nCount].cTalking))
		{
			pInfo[nCount].nSessionID = pSession->GetSessionID();
			nCount++;
		}
	}
}

void CDNGameRoom::TalkingUpdate(ULONG nCurTick)
{
	if (m_nVoiceChannelID[0] <= 0) return;
	if (m_nTalkingTick + VOICEUPDATETIME > nCurTick) return;	

	TTalkingInfo Info[PARTYMAX];
	memset(&Info, 0, sizeof(Info));
	int nCount = 0;

	GetUserTalking(Info, nCount);

	if (nCount > 0)
	{
		CDNUserSession * pSession = NULL;
		DNVector(PartyStruct):: iterator ii;
		for (ii = m_VecMember.begin(); ii != m_VecMember.end(); ii++)
		{
			pSession = (*ii).pSession;
			pSession->SendTalkingUpdate(nCount, Info);
		}
	}
	m_nTalkingTick = nCurTick;
}

void CDNGameRoom::SendRefreshVoiceInfo(UINT nOutAccountDBID)
{
	if (m_nVoiceChannelID[0] <= 0) return;
	TVoiceMemberInfo Info[PARTYMAX] = { 0, };
	int nCount = 0;

	CDNUserSession * pUser;
	for(DWORD i=0; i<GetUserCount(); i++) 
	{
		pUser = GetUserData(i);
		if(pUser)
		{
			Info[nCount].nSessionID = pUser->GetSessionID();
			Info[nCount].cVoiceAvailable = pUser->GetVoiceAvailable() == true ? 1 : 0;
			nCount++;

			if (nOutAccountDBID > 0)
				pUser->SetVoiceMute(nOutAccountDBID, false);
		}
	}

	UINT nMuteSessionList[PARTYCOUNTMAX];
	CDNUserSession * pTempUser;
	for(DWORD i=0; i<GetUserCount(); i++) 
	{
		pUser = GetUserData(i);
		if (pUser)
		{
			pTempUser = NULL;
			memset(nMuteSessionList, 0, sizeof(nMuteSessionList));
			for (int j = 0; j < PARTYCOUNTMAX; j++)
			{
				if (pUser->m_nVoiceMutedList[j] <= 0) continue;
				for(DWORD h=0; h<GetUserCount(); h++)
				{
					pTempUser = GetUserData(h);
					if (pTempUser->GetAccountDBID() == pUser->m_nVoiceMutedList[j])
						nMuteSessionList[j] = pTempUser->GetSessionID();
				}
			}
			pUser->SendVoiceMemberInfoRefresh(nCount, nMuteSessionList, Info);
		}
	}
}
#endif

DWORD CDNGameRoom::GetLiveUserCount()
{
	DWORD dwCount = 0;

	DnActorHandle hActor;
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		hActor = GetUserData(i)->GetActorHandle();
		if( hActor && !hActor->IsDie() ) dwCount++;
	}
	return dwCount;
}

DWORD CDNGameRoom::GetUserCountWithoutPartyOutUser()
{
	DWORD dwCount = 0;
	for (int i = 0; i < (int)m_VecMember.size(); i++)
	{
		if (m_VecMember[i].pSession->GetOutedMember() == false)
			dwCount++;
	}
	return dwCount;
}

DWORD CDNGameRoom::GetUserCountWithoutGM()
{
	DWORD dwCount = 0;
	for (int i = 0; i < (int)m_VecMember.size(); i++)
	{
		if (m_VecMember[i].pSession->bIsGMTrace() == false)
			dwCount++;
	}
	return dwCount;
}

DWORD CDNGameRoom::GetRealUserCountWithoutGM()
{
	DWORD dwCount = 0;
	for( int i = 0; i < (int)m_VecMember.size(); ++i )
	{
		if( m_VecMember[i].pSession && !m_VecMember[i].pSession->bIsGMTrace() && !m_VecMember[i].pSession->bIsAIBot() )
			++dwCount;
	}
	return dwCount;
}

DWORD CDNGameRoom::GetGMCount()
{
	DWORD dwCount = 0;
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		if( GetUserData(i) && GetUserData(i)->bIsGMTrace() )
			++dwCount;
	}

	return dwCount;
}

DWORD CDNGameRoom::GetUserCount( int iTeam )
{
	DWORD dwCount = 0;
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		if( GetUserData(i) && GetUserData(i)->GetTeam() == iTeam )
			++dwCount;
	}

	return dwCount;
}

bool CDNGameRoom::GetOutMemberExist()
{
	DWORD dwCount = 0;
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		if(GetUserData(i) && GetUserData(i)->GetOutedMember())
			return true;
	}
	return false;
}

DWORD CDNGameRoom::GetPartyUserCount(eGetUserCountType type)
{
	DWORD dwCount = 0, i = 0;

	DnActorHandle hActor;
	for (; i<GetUserCount(); i++)
	{
		if (type == ePICKUPITEM)
		{
			CDNUserSession *pSession = GetUserData(i);
			if (pSession && !pSession->bIsAIBot())
			{
				hActor = pSession->GetActorHandle();
				if (hActor && !hActor->IsDie()
					)
					dwCount++;
			}
		}
	}
	return dwCount;
}

bool CDNGameRoom::IsItemLootUserValid(DWORD userIdx)
{
	CDNUserSession *pSession = GetUserData(userIdx);
	if (pSession)
	{
		if( pSession->bIsGMTrace() || pSession->bIsAIBot() )
			return false;

		DnActorHandle hActor = pSession->GetActorHandle();
		if( !hActor || hActor->IsDie() || hActor->bIsObserver() )
			return false;
	}
	else
	{
		return false;
	}

	return true;
}

UINT CDNGameRoom::GetCurrentItemLooterIdx()
{
	if (m_nCurItemLooterInOrder < 0 || m_nCurItemLooterInOrder >= static_cast<int>(GetUserCount()) || GetUserCount() == 0)
	{
		m_nCurItemLooterInOrder = 0;
		return m_nCurItemLooterInOrder;
	}

	m_nCurItemLooterInOrder %= GetUserCount();

	for( int i=m_nCurItemLooterInOrder; i<(int)(GetUserCount()+m_nCurItemLooterInOrder); i++ ) {
		int nIndex = i % GetUserCount();
		if( IsItemLootUserValid( nIndex ) ) {
			m_nCurItemLooterInOrder = (nIndex+1) % GetUserCount();
			return nIndex;
		}
	}

	_ASSERT(0);
	g_Log.Log(LogType::_ERROR, L"[PARTY] ITEM LOOTING ERROR! NO VALID USER TO LOOT!! party_idx:%d\n", GetPartyIndex());
	return 0;
}

void CDNGameRoom::CheckCurrentItemLooterIdx( PartyStruct *pEraseUser )
{
	for( DWORD i=0; i<m_VecMember.size(); i++ ) {
		if( pEraseUser == &m_VecMember[i] ) {
			if( m_nCurItemLooterInOrder >= static_cast<int>(i) ) {
				m_nCurItemLooterInOrder--;
				if( m_nCurItemLooterInOrder < 0 ) m_nCurItemLooterInOrder = 0;
			}
			break;
		}
	}
}

void CDNGameRoom::SendNextVillageInfo(const char * pIP, USHORT nPort, int nMapIndex, int nNextMapIndex, char cNextGateNo, short nRet, INT64 nItemSerial)
{
#if defined( PRE_PARTY_DB )
	//Delete party here.	
	DelPartyDB();	
#endif
	if (nRet == ERROR_NONE)
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ )
		{
			if (m_VecMember[i].pSession->bIsGMTrace()) continue;
			if (cNextGateNo > 0) m_VecMember[i].pSession->VerifyValidMap(nNextMapIndex);

			if (!m_VecMember[i].pSession->DeleteWarpVillageItemByUse(nItemSerial))
				continue;

			m_VecMember[i].pSession->SetNextVillageData(pIP, nPort, nMapIndex, nNextMapIndex, cNextGateNo);
		}
	}
	else
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ )
			m_VecMember[i].pSession->SendVillageInfo(pIP, nPort, nRet, 0, 0);
	}
}

void CDNGameRoom::SendPvPUserState(UINT nAccountDBID, UINT uiUserState)
{
	UINT nSessionID = 0;
	for( DWORD i=0; i<m_VecMember.size(); i++ )
	{
		if (m_VecMember[i].pSession->GetAccountDBID() == nAccountDBID)
		{
			nSessionID = m_VecMember[i].pSession->GetSessionID();
			m_VecMember[i].pSession->SetPvPUserState(uiUserState);
			break;
		}
	}

	if (nSessionID > 0)
	{
		for( DWORD i=0; i<m_VecMember.size(); i++ )
			m_VecMember[i].pSession->SendPvPUserState(nSessionID, uiUserState);
	}
}

// Send intruding user info to existing users
void CDNGameRoom::SendBreakIntoUser( CDNUserSession* pBreakIntoSession )
{
	if( !pBreakIntoSession )
		return;

	SCROOM_SYNC_MEMBER_BREAKINTO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	char* pBuffer		= reinterpret_cast<char*>(TxPacket.EquipArray);
	char* pOrgBuffer	= pBuffer;

	pBreakIntoSession->GetPartyMemberInfo( TxPacket.Member );
	TxPacket.usTeam					= pBreakIntoSession->GetTeam();
	TxPacket.cGMTrace				= pBreakIntoSession->bIsGMTrace() ? 1: 0;
	TxPacket.iDefaultPartsIndex[0]	= pBreakIntoSession->GetDefaultBody();
	TxPacket.iDefaultPartsIndex[1]	= pBreakIntoSession->GetDefaultLeg();
	TxPacket.iDefaultPartsIndex[2]	= pBreakIntoSession->GetDefaultHand();
	TxPacket.iDefaultPartsIndex[3]	= pBreakIntoSession->GetDefaultFoot();

	memcpy(TxPacket.cViewCashEquipBitmap, pBreakIntoSession->GetViewCashEquipBitmap(), sizeof(TxPacket.cViewCashEquipBitmap));

	// Title
	TxPacket.iSelectAppellation		= pBreakIntoSession->GetSelectAppellation();
	TxPacket.nCoverAppellation		= pBreakIntoSession->GetCoverAppellation();
	TxPacket.cAccountLevel			= pBreakIntoSession->GetAccountLevel();
	if( pBreakIntoSession->GetItem()->GetVehicleEquip() )
		TxPacket.VehicleInfo.SetCompact( *pBreakIntoSession->GetItem()->GetVehicleEquip() );
	if( pBreakIntoSession->GetItem()->GetPetEquip() )
		TxPacket.PetInfo.SetCompact( *pBreakIntoSession->GetItem()->GetPetEquip() );

	// Equipment
	for( int i=0; i<EQUIPMAX ; ++i )
	{
		if( pBreakIntoSession->GetItem()->GetEquip(i) == NULL )
			continue;
		// SlotIndex
		memcpy( pBuffer, &i, sizeof(char) );
		pBuffer += sizeof(char);
		// TItem
		memcpy( pBuffer, pBreakIntoSession->GetItem()->GetEquip(i), sizeof(TItem) );	
		pBuffer += sizeof(TItem);

		++TxPacket.nEquipCount;
	}

	// Cash equipment
	for( int i=0 ; i<CASHEQUIPMAX ; ++i )
	{
		if( pBreakIntoSession->GetItem()->GetCashEquip(i) == NULL )
			continue;
		// SlotIndex
		memcpy( pBuffer, &i, sizeof(char) );
		pBuffer += sizeof(char);
		// TItem
		memcpy( pBuffer, pBreakIntoSession->GetItem()->GetCashEquip(i), sizeof(TItem) );	
		pBuffer += sizeof(TItem);

		++TxPacket.nCashEquipCount;
	}

	// Crest
	for( int i=0; i<GLYPHMAX ; ++i )
	{
		if( pBreakIntoSession->GetItem()->GetGlyph(i) == NULL )
			continue;
		// SlotIndex
		memcpy( pBuffer, &i, sizeof(char) );
		pBuffer += sizeof(char);
		// TItem
		memcpy( pBuffer, pBreakIntoSession->GetItem()->GetGlyph(i), sizeof(TItem) );	
		pBuffer += sizeof(TItem);

		++TxPacket.nGlyphCount;
	}


	// Skill
	for( int i=0 ; i<SKILLMAX ; ++i ) 
	{
		if( pBreakIntoSession->GetSkillData()->SkillList[i].nSkillID <= 0 )
			continue;

		// Action file optimization work for Jiho; send all party members' skills.

		if( pBreakIntoSession->GetSkillData()->SkillList[i].cSkillLevel < 1 )
			continue;


		//if( CDnSkill::GetSkillType( pBreakIntoSession->GetSkillData()->SkillList[i].nSkillID ) != CDnSkill::Passive &&
		//	CDnSkill::GetSkillType( pBreakIntoSession->GetSkillData()->SkillList[i].nSkillID ) != CDnSkill::EnchantPassive ) 
		//	continue;

		memcpy( pBuffer, &(pBreakIntoSession->GetSkillData()->SkillList[i]), sizeof(TSkill) );
		pBuffer += sizeof(TSkill);
		++TxPacket.nSkillCount;
	}

	TxPacket.GuildSelfView = pBreakIntoSession->GetGuildSelfView();

	int iSize = static_cast<int>(sizeof(TxPacket)-sizeof(TxPacket.EquipArray)-sizeof(TxPacket.SkillArray)+(pBuffer-pOrgBuffer));
	for( UINT i=0; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession == pBreakIntoSession )
			continue;
		m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBER_BREAKINTO, reinterpret_cast<char*>(&TxPacket), iSize );
	}

#if defined (PRE_ADD_BESTFRIEND)
	//Send best friend info
	std::vector<TPartyBestFriend> vMemberBestFriend;
	vMemberBestFriend.reserve(1);

	TPartyBestFriend MemberBestFriend = {0,};
	MemberBestFriend.nSessionID = pBreakIntoSession->GetSessionID();

	TBestFriendInfo& Info = pBreakIntoSession->GetBestFriend()->GetInfo();
	MemberBestFriend.biBFItemSerial = Info.biItemSerial;
	_wcscpy(MemberBestFriend.wszBFName, _countof(MemberBestFriend.wszBFName), Info.wszName, (int)wcslen(Info.wszName));

	vMemberBestFriend.push_back( MemberBestFriend );

	for( UINT i=0; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession == pBreakIntoSession )
			continue;		
		m_VecMember[i].pSession->SendPartyBestFriend( vMemberBestFriend );
	}
#endif	// #if defined (PRE_ADD_BESTFRIEND)
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNGameRoom::SendCompleteBreakIntoUser( CDNUserSession* pBreakIntoSession )
{
	if(!pBreakIntoSession)
		return;

	// Handling differs between world map and dungeon
	CDnWorld *pWorld = GetWorld();
	bool bDungeon = false;
	if( pWorld && pWorld->GetMapType() == EWorldEnum::MapTypeDungeon )
	{
		bDungeon = true;
	}

	std::vector<TEffectSkillData> vEffectSkill;
	vEffectSkill.clear();
	pBreakIntoSession->GetItem()->GetEffectSkillItem( vEffectSkill );
	CDNUserSession::ApplyEffectSkill(pBreakIntoSession->GetPlayerActor(), vEffectSkill , !bDungeon , pBreakIntoSession->GetGameRoom()->bIsPvPRoom() );

	pBreakIntoSession->CheckEffectSkillItemData();
	pBreakIntoSession->GetItem()->BroadcastEffectSkillItemData(false);
#if defined( PRE_WORLDCOMBINE_PARTY )
	if(bIsWorldCombineParty())
	{
		ApplyWorldCombinePartyBuff(pBreakIntoSession);
	}
#endif
	
#if defined( PRE_ADD_DIRECTNBUFF )	
	if( pWorld && pWorld->GetMapType() == EWorldEnum::MapTypeDungeon && GetGameTaskType() != GameTaskType::Farm )
	{
		if( bIsDirectPartyBuff() )
		{
			ApplyDirectPartyBuff( true );
			if( pBreakIntoSession->bIsDirectPartyBuff() )
				SendDirectPartyBuffMsg(pBreakIntoSession);
		}		
	}				
#endif

	SendPartyEffectSkillItemData(pBreakIntoSession);
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	SendTotalLevelSkillInfo(pBreakIntoSession);
#endif
#if defined( PRE_ADD_GUILDREWARDITEM )
	pBreakIntoSession->SetGuildRewardItem();
	pBreakIntoSession->ApplyGuildRewardSkill(bDungeon);
#endif

#if defined( PRE_FIX_BREAKINTO_BLOW_SYNC )
	SendAddStateEffect( pBreakIntoSession );
#endif

}
#endif

void CDNGameRoom::SendHPSP( CDNUserSession* pBreakIntoSession )
{
	if( !pBreakIntoSession )
		return;

	SCROOM_SYNC_MEMBERHPSP TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT uiSendCount = 0;

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		INT64 iHP = 0;
		int iSP = 0;

		if( m_VecMember[i].pSession && m_VecMember[i].pSession->GetActorHandle() )
		{
			if( m_VecMember[i].pSession == pBreakIntoSession )
			{
				// Send backup info for drop-in user
				PartyBackUpStruct BackupInfo;
				if( GetBackupPartyInfo( pBreakIntoSession->GetCharacterDBID(), BackupInfo ) == true )
				{										
					iHP = (INT64)(m_VecMember[i].pSession->GetActorHandle()->GetMaxHP() * BackupInfo.nHPPercent * 0.01);
					iSP = m_VecMember[i].pSession->GetActorHandle()->GetSP();
				}
				else
				{
					iHP = m_VecMember[i].pSession->GetActorHandle()->GetHP();
					iSP = m_VecMember[i].pSession->GetActorHandle()->GetSP();
				}
			}
			else
			{
				iHP = m_VecMember[i].pSession->GetActorHandle()->GetHP();
				iSP = m_VecMember[i].pSession->GetActorHandle()->GetSP();
			}
#if !defined( _FINAL_BUILD )
			/*
			WCHAR wszBuf[MAX_PATH];
			wsprintf( wszBuf, L"[%s] HP:%d SP:%d\r\n", m_VecMember[i].pSession->GetActorHandle()->GetName(), iHP, iSP );
			g_Log.Log( LogType::_NORMAL, pBreakIntoSession, wszBuf );
			*/
#endif
		}

#ifdef PRE_MOD_SYNCPACKET
		TxPacket.tHPSP[TxPacket.cCount].nSessionID = m_VecMember[i].pSession->GetSessionID();
		TxPacket.tHPSP[TxPacket.cCount].iHP	= iHP;
		TxPacket.tHPSP[TxPacket.cCount].iSP	= iSP;
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.iHPSPArr[TxPacket.cCount*2] = (int)iHP;
		TxPacket.iHPSPArr[(TxPacket.cCount*2)+1] = iSP;
#endif		//#ifdef PRE_MOD_SYNCPACKET
		++TxPacket.cCount;

		// Page completed
#ifdef PRE_MOD_SYNCPACKET
		if( TxPacket.cCount%(_countof(TxPacket.tHPSP)) == 0 )
		{
#else		//#ifdef PRE_MOD_SYNCPACKET
		if( TxPacket.cCount%(_countof(TxPacket.iHPSPArr)/2) == 0 )
		{
			TxPacket.unStartIndex = static_cast<USHORT>(uiSendCount);
#endif		//#ifdef PRE_MOD_SYNCPACKET
			pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERHPSP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount		+= TxPacket.cCount;
			TxPacket.cCount  = 0;
		}
	}

	if( TxPacket.cCount > 0 )
	{
#ifdef PRE_MOD_SYNCPACKET
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.tHPSP)+TxPacket.cCount*sizeof(TSyncHPSP);
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.iHPSPArr)+TxPacket.cCount*sizeof(TxPacket.iHPSPArr[0])*2;
#endif		//#ifdef PRE_MOD_SYNCPACKET

		pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERHPSP, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}

void CDNGameRoom::SendBattleMode( CDNUserSession* pBreakIntoSession )
{
	if( !pBreakIntoSession )
		return;

	SCROOM_SYNC_MEMBERBATTLEMODE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT uiSendCount = 0;

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		BYTE cBattleMode = 0;
		if( m_VecMember[i].pSession && m_VecMember[i].pSession->GetActorHandle() )
		{
			CDnPlayerActor* pActor = static_cast<CDnPlayerActor*>(m_VecMember[i].pSession->GetActorHandle().GetPointer() );
			if( pActor )
				cBattleMode = pActor->IsBattleMode() ? 1 : 0;
		}

#ifdef PRE_MOD_SYNCPACKET
		TxPacket.tBattleMode[TxPacket.cCount].nSessionID = m_VecMember[i].pSession->GetSessionID();
		TxPacket.tBattleMode[TxPacket.cCount].cBattleMode = cBattleMode;
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.cBattleModeArr[TxPacket.cCount] = cBattleMode;
#endif		//#ifdef PRE_MOD_SYNCPACKET
		++TxPacket.cCount;

		// Page completed
#ifdef PRE_MOD_SYNCPACKET
		if( TxPacket.cCount%_countof(TxPacket.tBattleMode) == 0 )
		{
#else		//#ifdef PRE_MOD_SYNCPACKET
		if( TxPacket.cCount%_countof(TxPacket.cBattleModeArr) == 0 )
		{
			TxPacket.unStartIndex = static_cast<USHORT>(uiSendCount);
#endif		//#ifdef PRE_MOD_SYNCPACKET
			pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERBATTLEMODE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount	+= TxPacket.cCount;

			TxPacket.cCount = 0;
		}
	}

	if( TxPacket.cCount > 0 )
	{
#ifdef PRE_MOD_SYNCPACKET
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.tBattleMode)+TxPacket.cCount*sizeof(TBattleMode);
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.cBattleModeArr)+TxPacket.cCount*sizeof(TxPacket.cBattleModeArr[0]);
#endif		//#ifdef PRE_MOD_SYNCPACKET

		pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERBATTLEMODE, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}


// Send Blow info to the drop-in user.
void CDNGameRoom::SendAddStateEffect( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession == pBreakIntoGameSession )
			continue;

		DnActorHandle hActor = m_VecMember[i].pSession->GetActorHandle();
		if( hActor )
			hActor->CmdSyncBlow( pBreakIntoGameSession );
	}
}

// Send current user position info to the drop-in user.
void CDNGameRoom::SendPosition( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession == pBreakIntoGameSession )
		{
			DnActorHandle hActor = m_VecMember[i].pSession->GetActorHandle();
			if( hActor )
				hActor->CmdWarp( *hActor->GetPosition(), EtVec3toVec2( *hActor->GetLookDir() ) );

			break;
		}
	}
}

// Send DropItem info to the drop-in user.
void CDNGameRoom::SendDropItemList( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	DnActorHandle hActor = pBreakIntoGameSession->GetActorHandle();
	if( !hActor )
		return;

	if( CDnDropItem::s_pVecProcessList[GetRoomID()].empty() )
		return;

	size_t stCount = CDnDropItem::s_pVecProcessList[GetRoomID()].size();

	DNVector(DnDropItemHandle) vDropItem;
	vDropItem.reserve( stCount );

	for( UINT i=0 ; i<stCount ; ++i )
		vDropItem.push_back( CDnDropItem::s_pVecProcessList[GetRoomID()][i]->GetMySmartPtr() );

	//CDnDropItem::ScanItem( this, *hActor->GetPosition(), FLT_MAX, vDropItem );

	//
	SCROOM_SYNC_DROPITEMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( UINT i=0 ; i<vDropItem.size() ; ++i )
	{
		TDropItemSync* pDropItemSync = &TxPacket.sDropItemList[TxPacket.cCount];

		pDropItemSync->nSessionID	= vDropItem[i]->GetUniqueID();
		pDropItemSync->fpos[0]		= vDropItem[i]->GetPosition()->x;
		pDropItemSync->fpos[1]		= vDropItem[i]->GetPosition()->y;
		pDropItemSync->fpos[2]		= vDropItem[i]->GetPosition()->z;
		pDropItemSync->nItemID		= vDropItem[i]->GetItemID();
		pDropItemSync->nRandomSeed	= vDropItem[i]->GetRandomSeed();
		pDropItemSync->nCount		= vDropItem[i]->GetOverlapCount();
		pDropItemSync->nRotate		= vDropItem[i]->GetRotate();
		pDropItemSync->uiLifeTime	= static_cast<UINT>(vDropItem[i]->GetLifeTime()*1000);
		pDropItemSync->fDistance	= vDropItem[i]->GetDistance();

		if( ++TxPacket.cCount >= SENDDROPITEMLISTMAX )
		{
			pBreakIntoGameSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_DROPITEMLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
			memset( &TxPacket, 0, sizeof(TxPacket) );
		}
	}

	// SEND remaining items
	if( TxPacket.cCount > 0 )
	{
		UINT uiSize = sizeof(TxPacket)-sizeof(TxPacket.sDropItemList)+sizeof(TDropItemSync)*TxPacket.cCount;
		pBreakIntoGameSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_DROPITEMLIST, reinterpret_cast<char*>(&TxPacket), uiSize );
	}
}

// Sync Monster
void CDNGameRoom::SyncMonster( CDNUserSession* pBreakIntoGameSession )
{
	DN_ASSERT( pBreakIntoGameSession != NULL, "CDNGameRoom::SyncMonster() pBreakIntoGameSession != NULL" );
	DN_ASSERT( m_pGameTask != NULL, "CDNGameRoom::SyncMonster() m_pGameTask != NULL ");

	m_pGameTask->SyncMonster( pBreakIntoGameSession );
}

void CDNGameRoom::SyncNpc( CDNUserSession* pBreakIntoGameSession )
{
	DN_ASSERT( pBreakIntoGameSession != NULL, "CDNGameRoom::SyncNpc() pBreakIntoGameSession != NULL" );
	DN_ASSERT( m_pGameTask != NULL, "CDNGameRoom::SyncNpc() m_pGameTask != NULL ");

	m_pGameTask->SyncNpc( pBreakIntoGameSession );
}

// Prop synchronization
void CDNGameRoom::SyncProp( CDNUserSession* pBreakIntoGameSession )
{
	CDnWorldSector* pSector = static_cast<CDnWorldSector*>(CDnWorld::GetInstance( this ).GetSector( 0.0f, 0.0f ));
	for( UINT i=0 ; i<pSector->GetPropCount() ; ++i )
	{
		CEtWorldProp* pProp = pSector->GetPropFromIndex( i );
		if( pProp )
		{
			if( static_cast<CDnWorldProp*>(pProp)->IsShow() )
				pProp->OnSyncComplete( pBreakIntoGameSession );
			else
				static_cast<CDnWorldProp*>(pProp)->CmdShow( false, pBreakIntoGameSession );
		}
	}
}

void CDNGameRoom::CheckDiePlayer( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	PartyBackUpStruct BackupInfo;
	// Warp to leader position on party recovery drop-in
	if( GetBackupPartyInfo( pBreakIntoGameSession->GetCharacterDBID(), BackupInfo ) == true )
	{
		// Warp to leader position
		DnActorHandle hActor = pBreakIntoGameSession->GetActorHandle();
		if( hActor )
		{			
			hActor->SetHP( 0 );
		}
	}
}

// Player synchronization
void CDNGameRoom::SyncPlayer( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	// Synchronize party member count
	++m_iPartMemberCnt;

	// Sync fatigue
	if( !bIsPvPRoom() )
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			if( !m_VecMember[i].pSession )
				continue;

			m_VecMember[i].pSession->SetFatigue( m_VecMember[i].pSession->GetFatigue(), m_VecMember[i].pSession->GetWeeklyFatigue(), m_VecMember[i].pSession->GetPCBangFatigue(), 
				m_VecMember[i].pSession->GetEventFatigue(), m_VecMember[i].pSession->GetVIPFatigue() );
		}
	}

	PartyBackUpStruct BackupInfo;
	// Warp to leader position on party recovery drop-in
	if( GetBackupPartyInfo( pBreakIntoGameSession->GetCharacterDBID(), BackupInfo ) == true )
	{
		// Warp to leader position
		DnActorHandle hActor = pBreakIntoGameSession->GetActorHandle();

		UINT uiSessionID = 0;
		GetLeaderSessionID( uiSessionID );
		if( uiSessionID > 0 )
		{
			int Seq;
			PartyStruct* pLeader = GetPartyDatabySessionID( uiSessionID, Seq );
			if( pLeader && pLeader->pSession  )
			{
				DnActorHandle hLeader = pLeader->pSession->GetActorHandle();
				if( hActor && hLeader )
					hActor->CmdWarp( *hLeader->GetPosition(), EtVec3toVec2(*hLeader->GetLookDir()) );
			}
		}

		Ghost::Type::eCode Type = ( BackupInfo.nHPPercent > 0 ) ? Ghost::Type::PartyRestore : Ghost::Type::Normal;
		pBreakIntoGameSession->SendGhostType( pBreakIntoGameSession->GetSessionID(), Type );

		// Process death
		if( hActor )
			hActor->CmdRefreshHPSP( 0,0 );

		// Restore RebirthCoin
		PartyStruct* pStruct = GetPartyData( pBreakIntoGameSession );
		if( pStruct )
		{
			pStruct->nUsableRebirthCoin = BackupInfo.nUsableRebirthCoin;
			for( std::list<int>::iterator itor = BackupInfo.ReverseItemList.begin(); itor != BackupInfo.ReverseItemList.end(); itor++ )
				pStruct->ReverseItemList.push_back( (*itor) );			
			pStruct->nUsableRebirthItemCoin = BackupInfo.nUsableRebirthItemCoin;
			pBreakIntoGameSession->SendRebirthCoin( ERROR_NONE, BackupInfo.nUsableRebirthCoin, _REBIRTH_SELF, pBreakIntoGameSession->GetSessionID());
			for( std::list<int>::iterator itor = BackupInfo.ReverseItemList.begin(); itor != BackupInfo.ReverseItemList.end(); itor++ )
				pBreakIntoGameSession->SendSpecialRebirthItem( *itor, BackupInfo.nUsableRebirthItemCoin);			
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
			for( std::map<int,int>::iterator itor = BackupInfo.UseLimitItem.begin(); itor != BackupInfo.UseLimitItem.end(); itor++ )
			{
				pStruct->UseLimitItem.insert(make_pair(itor->first, itor->second));
				pBreakIntoGameSession->SendStageUseLimitItem( itor->first, itor->second );
			}
#endif
		}
	}

	if( GetGameTask() )
		GetGameTask()->SyncTimeAttack( pBreakIntoGameSession );
}

// Gate synchronization
void CDNGameRoom::SyncGate( CDNUserSession* pBreakIntoGameSession )
{
	if( !pBreakIntoGameSession )
		return;

	DN_ASSERT( m_pGameTask != NULL, "CDNGameRoom::SyncGate() m_pGameTask != NULL" );
	m_pGameTask->RequestGateInfo( pBreakIntoGameSession );
}

void CDNGameRoom::AddRequestGetReversionItem(const TItem& itemInfo, DnDropItemHandle hDropItem)
{
	if (m_pPartyTask)
		m_pPartyTask->AddRequestGetReversionItem(itemInfo, hDropItem);
	else
		_DANGER_POINT();
}

bool CDNGameRoom::IsEnableAddRequestGetReversionItem(DnDropItemHandle hDropItem) const
{
	if (m_pPartyTask)
		return m_pPartyTask->IsEnableAddRequestGetReversionItem(hDropItem);
	else
		_DANGER_POINT();

	return false;
}

void CDNGameRoom::BroadcastChatRoomView(CDNUserSession* pSender, TChatRoomView& ChatRoomView)
{
	CDNUserSession* pUserSession;
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		pUserSession = 	m_VecMember[i].pSession;
		if (pSender == pUserSession)
			continue;

		pUserSession->SendChatRoomView( pSender->GetSessionID(), ChatRoomView );
	}
}

void CDNGameRoom::BroadcastChatRoomEnterUser(CDNUserSession* pSender, CDNChatRoom* pChatRoom)
{
	CDNUserSession* pUserSession;
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		pUserSession = 	m_VecMember[i].pSession;
		if (pSender == pUserSession)
			continue;

		pUserSession->SendChatRoomEnterUser( pChatRoom->GetChatRoomID(), pChatRoom->GetChatRoomType(), pSender->GetSessionID() );
	}
}

void CDNGameRoom::BroadcastChatRoomLeaveUser(CDNUserSession* pSender, int nErrorCode)
{
	CDNUserSession* pUserSession;
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		pUserSession = 	m_VecMember[i].pSession;
		if (pSender == pUserSession)
			continue;

		pUserSession->SendChatRoomLeaveUser( pSender->GetSessionID(), nErrorCode );
	}
}

void CDNGameRoom::SendChatRoomInfo(CDNUserSession* pBreakIntoGameSession)
{
	m_ChatRoomManager.SendChatRoomInfo(pBreakIntoGameSession);
}

#if defined( PRE_WORLDCOMBINE_PARTY )

bool CDNGameRoom::CheckDestroyWorldCombineParty()
{
	if( bIsWorldCombineParty() )
	{
		if( GetGameTask() )
		{
			const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( GetGameTask()->GetMapTableID() );
			if( pMapInfo )
				return pMapInfo->MapType != EWorldEnum::MapTypeWorldMap;
		}
	}

	return true;
}

void CDNGameRoom::ApplyWorldCombinePartyBuff( CDNUserSession* pBreakIntoSession )
{
	if(!pBreakIntoSession)
		return;

	if( bIsWorldCombineParty() )
	{
		WorldCombineParty::WrldCombinePartyData* pCombinePartyData = g_pDataManager->GetCombinePartyData(GetWorldPartyPrimaryIndex());
		if(pCombinePartyData)
		{
			// Forcibly construct and pass CSUseItem and item data
			for( int i=0;i<WorldCombineParty::eType::MAXSKILLCOUNT;i++ )
			{
				if(pCombinePartyData->nSkillID[i] <= 0) continue;

				CSUseItem pPacket;
				memset(&pPacket, 0, sizeof(pPacket));
				TItemData *pItemData = g_pDataManager->GetItemData(pCombinePartyData->nSkillID[i]);

				bool bUseItem = true;
				if (CDnWorld::GetInstance(this).GetMapType() == GlobalEnum::eMapTypeEnum::MAP_WORLDMAP)
					bUseItem = false;

				pBreakIntoSession->GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pItemData, pBreakIntoSession->GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, false, bUseItem);
			}			
		}
	}	
}

#endif // #if defined( PRE_WORLDCOMBINE_PARTY )


bool CDNGameRoom::bIsIgnorePrefixSkill()
{
	if( bIsLadderRoom() )
		return false;
	if( bIsGuildWarSystem() )
		return false;
	if( bIsLevelRegulation() )
		return true;

	return false;
}

CDNGameRoom::PartyBackUpStruct& CDNGameRoom::PartyBackUpStruct::operator=( CDNGameRoom::PartyStruct& Struct )
{
	this->nUsableRebirthCoin = Struct.nUsableRebirthCoin;
	for( std::list<int>::iterator itor = Struct.ReverseItemList.begin(); itor != Struct.ReverseItemList.end(); itor++ )
		this->ReverseItemList.push_back( (*itor) );
	
	this->nUsableRebirthItemCoin = Struct.nUsableRebirthItemCoin;
	this->nHPPercent = 0;
	this->nSPPercent = 0;

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )

	for( std::map<int,int>::iterator itor = Struct.UseLimitItem.begin(); itor != Struct.UseLimitItem.end(); itor++ )
	{
		this->UseLimitItem.insert(make_pair(itor->first, itor->second));		
	}	
#endif

	if( Struct.pSession )
	{
		DnActorHandle hActor = Struct.pSession->GetActorHandle();
		if( hActor )
		{
			this->nHPPercent = hActor->GetHPPercent();
			this->nSPPercent = hActor->GetSPPercent();

			if( this->nHPPercent == 0 )
			{
				if( hActor->IsDie() == false )
					this->nHPPercent = 1;
			}
		}
	}
	return (*this);
}

void CDNGameRoom::AddLastPartyDungeonInfo( CDNUserSession* pSession )
{
	if( pSession == NULL )
		return;

	bool bModDungeonInfo = false;
	switch( m_cReqGameIDType )
	{
		case REQINFO_TYPE_PARTY:
		{
			bModDungeonInfo = true;
			break;
		}
	}

	INT64 biValue = 0;
	if( bModDungeonInfo == true )
		biValue = MAKELONG64( g_Config.nManagedID, GetRoomID() );
	pSession->ModCommonVariableData( CommonVariable::Type::LastPartyDungeonInfo, biValue );

}

void CDNGameRoom::DelLastPartyDungeonInfo( CDNUserSession* pSession )
{
	if( pSession == NULL )
		return;

	pSession->ModCommonVariableData( CommonVariable::Type::LastPartyDungeonInfo, 0 );

}

void CDNGameRoom::AddBackupPartyInfo( CDNUserSession* pSession )
{
	std::map<INT64,PartyBackUpStruct>::iterator itor = m_mPartyBackUp.find( pSession->GetCharacterDBID() );
	if( itor != m_mPartyBackUp.end() )
	{
		if( itor->second.nHPPercent > 0 )
			return;
	}

	INT64 biValue = 0;
	if( pSession->GetCommonVariableDataValue( CommonVariable::Type::LastPartyDungeonInfo, biValue ) == false )
		return;

	if( biValue <= 0 )
		return;

	const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pSession->GetMapIndex() );
	if( pMapInfo == NULL || pMapInfo->bUserReturnSystem == false )
	{
		return;
	}

	PartyStruct* pStruct = GetPartyData( pSession );
	m_mPartyBackUp[pSession->GetCharacterDBID()] = (*pStruct);

}

void CDNGameRoom::DelBackupPartyInfo()
{
	m_mPartyBackUp.clear();

}

void CDNGameRoom::DelBackupPartyInfo( INT64 biCharacterDBID )
{
	if( m_mPartyBackUp.find( biCharacterDBID ) == m_mPartyBackUp.end() )
	{
		return;
	}

	m_mPartyBackUp.erase( biCharacterDBID );
}

bool CDNGameRoom::bIsBackupPartyInfo( INT64 biCharacterDBID )
{
	if( m_mPartyBackUp.find( biCharacterDBID ) == m_mPartyBackUp.end() )
	{
		return false;
	}

	return true;
}

bool CDNGameRoom::GetBackupPartyInfo( INT64 biCharacterDBID, PartyBackUpStruct& BackupInfo )
{
	std::map<INT64,PartyBackUpStruct>::iterator itor = m_mPartyBackUp.find( biCharacterDBID );
	if( itor == m_mPartyBackUp.end() )
		return false;

	BackupInfo = (*itor).second;
	return true;
}

#if defined( PRE_FIX_49129 )
bool CDNGameRoom::AddFirstPartyStruct( CDNUserSession* pSession )
{
	if( bIsFarmRoom() == false )
	{
		UINT uiMax = bIsPvPRoom() ? PvPCommon::Common::MaxPlayer : PARTYCOUNTMAX;

		if (pSession->bIsGMTrace())
			uiMax = PARTYMAX;
		
		if( m_MapFirstPartyMember.size() >= uiMax )
			return false;
	}
	else
		return false;

	PartyFirstStruct PartyFirst;
	PartyFirst.biCharacterDBID = pSession->GetCharacterDBID();	
	
	PartyFirst.nIndex = (int)m_MapFirstPartyMember.size();
	m_MapFirstPartyMember.insert( std::make_pair(PartyFirst.biCharacterDBID,PartyFirst) );	

	return true;
}
CDNGameRoom::PartyFirstStruct* CDNGameRoom::GetFirstPartyData( DWORD dwIndex )
{
	std::map<INT64, PartyFirstStruct>::iterator itor = m_MapFirstPartyMember.begin();
	for(;itor != m_MapFirstPartyMember.end(); itor++)
	{
		if( itor->second.nIndex == dwIndex )
			return &itor->second;
	}
	return NULL;
}
CDNGameRoom::PartyFirstStruct* CDNGameRoom::GetFirstPartyData( INT64 biCharacterID )
{
	std::map<INT64, PartyFirstStruct>::iterator itor = m_MapFirstPartyMember.find( biCharacterID );
	if( itor == m_MapFirstPartyMember.end() )
		return NULL;
	return &itor->second;	
}
#endif

#if defined( PRE_PARTY_DB )
void CDNGameRoom::SetPartyID( TPARTYID PartyID )
{
	m_PartyStructData.PartyID = PartyID;
}

void CDNGameRoom::AddPartyDB( CDNUserSession* pBreakIntoSession )	
{
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->AddPartyDB( pBreakIntoSession );
	}
}

void CDNGameRoom::AddPartyMemberDB( CDNUserSession* pSession )
{
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->AddPartyMemberDB( pSession );
	}
}

void CDNGameRoom::DelPartyDB()
{
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->DelPartyDB();
	}
}

void CDNGameRoom::DelPartyMemberDB( CDNUserSession *pSession )
{
#if defined( PRE_WORLDCOMBINE_PARTY )
	if( bIsWorldCombineParty() == true )
		return;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->DelPartyMemberDB( pSession );
	}
}
#endif

#if defined (PRE_WORLDCOMBINE_PARTY)
bool CDNGameRoom::IsCloseTargetMap(int nMapIndex)
{
	if( bIsWorldCombineParty() )
	{
		if (nMapIndex != GetPartyTargetMapID())
			return true;
	}

	return false;
}

bool CDNGameRoom::bIsWorldCombineParty()
{
	if (GetPartyIndex() > 0)
	{
		return Party::bIsWorldCombineParty( m_PartyStructData.Type );
	}

	return false;
}

#endif // #if defined (PRE_WORLDCOMBINE_PARTY)

#if defined( PRE_ADD_DIRECTNBUFF )

void CDNGameRoom::ApplyDirectPartyBuff( bool bUseItem/*=false*/ )
{	
	CDNUserSession * pSession = NULL;
	for( std::list<int>::iterator itor = m_DirectPartyBuffItemList.begin();itor != m_DirectPartyBuffItemList.end(); itor++ )
	{
		for(int i=0;i<(int)GetUserCount();i++)
		{		
			if( GetUserData(i) != NULL )
			{			
				pSession = GetUserData(i);					
				CSUseItem pPacket;
				memset(&pPacket, 0, sizeof(pPacket));
				TItemData *pItemData = g_pDataManager->GetItemData(*itor);
				if( g_pDataManager->IsUseItemAllowMapTypeCheck( pItemData->nItemID, m_iMapIdx ) )
				{
					TEffectSkillData* EffectSkill = pSession->GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pItemData->nSkillID );
					if( !EffectSkill )
					{						
						pSession->GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pItemData, pSession->GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, false, bUseItem);		
					}
				}
			}
		}		
	}
}

void CDNGameRoom::RemoveDirectPartyBuff( int nItemID, bool bAll /*= false*/ )
{		
	if(bAll)
	{
		CDNUserSession * pSession = NULL;
		for( std::list<int>::iterator itor = m_DirectPartyBuffItemList.begin();itor != m_DirectPartyBuffItemList.end();itor++)
		{
			TItemData *pItemData = g_pDataManager->GetItemData(*itor);
			if( pItemData )
			{
				for(int i=0;i<(int)GetUserCount();i++)
				{		
					if( GetUserData(i) != NULL )
					{			
						pSession = GetUserData(i);
						if( pSession && pSession->GetPlayerActor() )
						{
							TEffectSkillData* EffectSkill = pSession->GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pItemData->nSkillID );
							if( EffectSkill )
							{
								CDNUserSession::RemoveEffectSkill(pSession->GetPlayerActor(), EffectSkill);
								pSession->GetItem()->DelEffectSkillItem( pItemData->nSkillID );								
							}	
						}
					}
				}
			}
		}
	}
	else
	{		
		int nItemCount = 0;
		for( std::list<int>::iterator itor = m_DirectPartyBuffItemList.begin();itor != m_DirectPartyBuffItemList.end();)
		{
			if( nItemID == *itor)
			{
				if(nItemCount == 0)
					itor = m_DirectPartyBuffItemList.erase(itor);
				else
					itor++;
				nItemCount++;			
			}
			else
				itor++;
		}

		if(nItemCount <= 1)
		{
			TItemData *pItemData = g_pDataManager->GetItemData(nItemID);
			if( pItemData )
			{
				CDNUserSession * pSession = NULL;
				for(int i=0;i<(int)GetUserCount();i++)
				{		
					if( GetUserData(i) != NULL )
					{			
						pSession = GetUserData(i);	
						if( pSession && pSession->GetPlayerActor() )
						{
							TEffectSkillData* EffectSkill = pSession->GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pItemData->nSkillID );
							if( EffectSkill )
							{
								CDNUserSession::RemoveEffectSkill(pSession->GetPlayerActor(), EffectSkill);
								pSession->GetItem()->DelEffectSkillItem( pItemData->nSkillID );								
							}
						}
					}
				}
			}	
		}
	}	
}
void CDNGameRoom::SendDirectPartyBuffMsg(CDNUserSession * pSession/*=NULL*/)
{
	if(pSession)
		pSession->SendDirectPartyBuffMsg();
	else
	{
		for(int i=0;i<(int)GetUserCount();i++)
		{		
			if( GetUserData(i) != NULL )
			{			
				pSession = GetUserData(i);
				if(pSession->bIsDirectPartyBuff())
				{
					pSession->SendDirectPartyBuffMsg();
				}
			}
		}
	}	
}
#endif

#if defined( PRE_ADD_NEWCOMEBACK )

void CDNGameRoom::DelPartyMemberAppellation( int nAppellationID )
{
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->DelPartyMemberAppellation( nAppellationID );
	}
}

bool CDNGameRoom::CheckPartyMemberAppellation( int nAppellationID/*=0*/ )
{
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			 return pPartyTask->CheckPartyMemberAppellation( nAppellationID );
	}
	return false;
}

void CDNGameRoom::ApplyJoinMemberAppellation( CDNUserSession * pSession )
{
	if( !pSession )
		return;
	if( GetTaskMng() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->ApplyJoinMemberAppellation( pSession );
	}	
}

#endif

#if defined( PRE_FIX_BUFFITEM )
void CDNGameRoom::CheckEffectSkillItemData()
{	
	CDNUserSession * pSession = NULL;
	
	for(int i=0;i<(int)GetUserCount();i++)
	{		
		if( GetUserData(i) != NULL )
		{			
			pSession = GetUserData(i);					
			if( pSession )
			{
				pSession->CheckEffectSkillItemData();
			}
		}
	}		
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )

void CDNGameRoom::AddDBWorldPvPRoom( TWorldPvPMissionRoom *pMissionRoom, UINT nGMAccountDBID  )
{
	if( bIsWorldPvPRoom() )
	{
		if(bIsPvPRoom())
		{			
			GetDBConnection()->QueryAddWorldPvPRoom( 0, GetWorldSetID(), nGMAccountDBID, g_Config.nManagedID, GetRoomID(), pMissionRoom );			
		}
	}
}

void CDNGameRoom::AddDBWorldPvPRoomMember( CDNUserSession* pSession )
{
	if( bIsWorldPvPRoom() )
	{
		if(bIsPvPRoom())
		{		
			bool bObserverFlag = false;
			if( pSession->GetTeam() == PvPCommon::Team::Observer )			
			{
				bObserverFlag = true;
			}
			GetDBConnection()->QueryAddWorldPvPRoomMember( 0, GetWorldSetID(), pSession, GetPvPMaxUser(), GetWorldPvPRoomDBIndex(), bObserverFlag );		
		}
	}
}

void CDNGameRoom::DelDBWorldPvPRoomMember( CDNUserSession* pSession )
{
	if( bIsWorldPvPRoom() )
	{
		if(bIsPvPRoom())
		{	
			GetDBConnection()->QueryDelWorldPvPRoomMember( 0, GetWorldSetID(), GetWorldPvPRoomDBIndex(), pSession );		
		}
	}
}

void CDNGameRoom::SetWorldPvPRoomStart(bool bFlag)
{
	if( bIsPvPRoom() )
	{
		m_bWorldPvPRoomStart = bFlag;
		if( m_bWorldPvPRoomStart )
		{
			GetDBConnection()->QueryDelWorldPvPRoom( GetWorldSetID(), GetWorldPvPRoomDBIndex() );			
			if( GetPvPGameMode() && GetPvPGameMode()->GetPvPGameModeTable() )
			{
				switch( GetPvPGameMode()->GetPvPGameModeTable()->uiGameMode )
				{
				case PvPCommon::GameMode::PvP_AllKill:
					{
						GetPvPGameMode()->SetWorldPvPRoomStart();
					}
					break;
				case PvPCommon::GameMode::PvP_Tournament:
					{
						// Bracket complete
						GetPvPGameMode()->SetWorldPvPRoomStart();						
					}
					break;
				default:
					printf("CDNGameRoom::SetWorldPvPRoomStart DEFAULT PVP CASE INVALID\n");
					break;

				}
			}
			
		}
	}	
}

bool CDNGameRoom::CheckWorldPvPRoomBreakInto( MAGABreakIntoRoom* pPacket )
{
	DWORD nPlayerCount = 0;
	DWORD nObserverCount = 0;

	nObserverCount = GetBreakIntoUserTeamCount( PvPCommon::Team::Observer );
	nPlayerCount = GetBreakIntoUserCount() - nObserverCount;

	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		CDNGameRoom::PartyStruct* pStruct = GetPartyData(i);
		if( pStruct->pSession && pStruct->pSession->GetTeam() == PvPCommon::Team::Observer )
			nObserverCount++;
		else if( pStruct->pSession )
			nPlayerCount++;

	}

	int nRet = ERROR_NONE;
	if(bIsWorldPvPRoomStart())			
		nRet = ERROR_PVP_JOINROOM_NOTFOUNDROOM;			

	if( pPacket->uiTeam == PvPCommon::Team::Observer )
	{		
		if( nObserverCount >= PvPCommon::Common::ExtendMaxObserverPlayer)
			nRet = ERROR_PVP_JOINROOM_MAXPLAYER;
	}
	else if( nPlayerCount >= GetPvPMaxUser() )
	{
		nRet = ERROR_PVP_JOINROOM_MAXPLAYER;
	}

	if( nRet != ERROR_NONE )
	{
		g_pMasterConnectionManager->SendWorldPvPRoomJoinResult( pPacket->cWorldSetID, nRet, pPacket->uiAccountDBID );
		return false;
	}			

	return true;
}

DWORD CDNGameRoom::GetBreakIntoUserTeamCount( int nTeam )
{	
	int nCount = 0;
	for( std::list<CDNUserSession*>::iterator itor=m_BreakIntoUserList.begin() ; itor!=m_BreakIntoUserList.end() ; ++itor )
	{
		CDNUserSession* pSession = (*itor);

		// Only those not yet pushed to m_VecMember
		if( GetUserSession( pSession->GetSessionID() ) == NULL )
		{
			if( pSession->GetTeam() == nTeam )
				nCount++;			
		}
	}
	return nCount;
}

#endif

#if defined(PRE_ADD_ACTIVEMISSION)

void CDNGameRoom::InitActiveMission()
{
	CDnWorld *pWorld = GetWorld();
	if(!pWorld || pWorld->GetMapType() != EWorldEnum::MapTypeDungeon) return;
	if(!GetIsFirstInitializeDungeon()) return;
	
	int nActiveMissionGainRate = 0;
	bool bGuildBonus = false;

 	// Increase active mission acquisition rate if all full party members are in the same guild
	if( pWorld->GetMaxPartyCount() > 1 && pWorld->GetMaxPartyCount() == GetUserCount() )
	{
		bGuildBonus = true;
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			CDNUserSession* pSession = m_VecMember[i].pSession;
			if( !pSession->GetGuildUID().IsSet() || pSession->GetGuildUID() != m_VecMember[0].pSession->GetGuildUID() )
			{
				bGuildBonus = false;
				break;
			}
		}
	}
	
	switch(pWorld->GetMapSubType())
	{
	case EWorldEnum::MapSubTypeNone:		
		nActiveMissionGainRate = bGuildBonus ? ActiveMission::NoneTypeActiveMissionGainBonusRate : ActiveMission::NoneTypeActiveMissionGainRate;
		break;

	case EWorldEnum::MapSubTypeNest:
	case EWorldEnum::MapSubTypeNestNormal:
		nActiveMissionGainRate = bGuildBonus ? ActiveMission::NestTypeActiveMissionGainBonusRate : ActiveMission::NestTypeActiveMissionGainRate;
		break;
	}

	if(nActiveMissionGainRate <= 0) return;

	//Probability cheat
	int nActiveMissionCheatID = 0;
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{		
		if( m_VecMember[i].pSession->GetActiveMissionCheatID() > 0 )
		{
			nActiveMissionCheatID = m_VecMember[i].pSession->GetActiveMissionCheatID();
			m_VecMember[i].pSession->SetActiveMissionCheatID(0);	//Reset once cheat is executed
			break;
		}
	}

	TActiveMissionData *pActiveMissionData = NULL;

	if(nActiveMissionCheatID > 0)
	{
		pActiveMissionData = g_pDataManager->GetActiveMissionData(nActiveMissionCheatID-1);
	}
	else
	{
		if ((rand() % 100) >= nActiveMissionGainRate ) return;
		pActiveMissionData = g_pDataManager->GetRandomActiveMission(m_iMapIdx, rand());		
	}

	if(!pActiveMissionData) return;
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		if( m_VecMember[i].pSession && !m_VecMember[i].pSession->bIsAIBot() )
			m_VecMember[i].pSession->GetMissionSystem()->SetActiveMission(pActiveMissionData);
	}
}

#endif

#if defined(PRE_ADD_RENEW_RANDOM_MAP)
void CDNGameRoom::OnSelectRandomMap( int nRootMapIndex, int nRandomMapIndex )
{
	m_iRootMapIndex = nRootMapIndex;
	m_iMapIdx = nRandomMapIndex;

	g_Log.Log(LogType::_NORMAL, L"CDnGameRoom OnSelectRandomMap %d %d", nRootMapIndex, nRandomMapIndex );
}
#endif


DWORD CDNGameRoom::GetPartyUserClassCount(int nClassID)
{
	DWORD nClassCount = 0;
	for (UINT i = 0; i < m_VecMember.size(); i++)
	{
		if ( (m_VecMember[i].pSession->bIsGMTrace() == false) && (m_VecMember[i].pSession->GetClassID() == nClassID))
			nClassCount++;
	}
	return nClassCount;
}

#if defined( PRE_ALTEIAWORLD_EXPLORE )
void CDNGameRoom::SetAlteiaWorld(bool bFlag)
{
	m_bAlteiaWorld = bFlag;
	if( bFlag )
	{
		m_dwAlteiaWorldStartTime = timeGetTime();		
		GetDBConnection()->QueryAddStageStartLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), GetPartyLogIndex(), static_cast<BYTE>(GetUserCount()), GetGameTask()->GetMapTableID(),  static_cast<DBDNWorldDef::DifficultyCode::eCode>(Dungeon::Difficulty::Easy+1) );
		SetStageStartLogFlag( true );
		SetStageEndLogFlag( false );
	}
	else
	{
		m_dwAlteiaWorldPlayTime = timeGetTime() - m_dwAlteiaWorldStartTime;
		
		for (UINT i = 0; i < m_VecMember.size(); i++)
		{
			if( m_VecMember[i].pSession )
			{
				m_VecMember[i].pSession->AddAlteiaWorldResult(true);
			}
		}

		GetDBConnection()->QueryAddStageEndLog( GetDBThreadID(), GetWorldSetID(), 0, GetRoomLogIndex(), DBDNWorldDef::WhereToGoCode::Village );		
	}
}

void CDNGameRoom::ResetAlteiaWorldmap()
{
	if( GetGameTask() )
		GetGameTask()->ResetAlteiaWorldmap();	
}

DWORD CDNGameRoom::GetAlteiaPlayTime()
{
	if( m_dwAlteiaWorldPlayTime > 0 )
		return m_dwAlteiaWorldPlayTime;
	
	return 0;
}
#endif
#if defined(PRE_ADD_CP_RANK)
void CDNGameRoom::SetAbyssStageClearBest(const TAGetStageClearBest* pA)
{
	memcpy(&m_sLegendClearBest, &pA->sLegendClearBest, sizeof(m_sLegendClearBest));
	memcpy(&m_sMonthlyClearBest, &pA->sMonthlyClearBest, sizeof(m_sMonthlyClearBest));
	if( pA->sLegendClearBest.biCharacterDBID > 0 && m_sLegendClearBest.cRank > 0)	
		m_sLegendClearBest.cRank = m_sLegendClearBest.cRank - DBDNWorldDef::ClearGradeCode::eCode::SSS;
	if( pA->sMonthlyClearBest.biCharacterDBID > 0 && m_sMonthlyClearBest.cRank > 0)
		m_sMonthlyClearBest.cRank = m_sMonthlyClearBest.cRank - DBDNWorldDef::ClearGradeCode::eCode::SSS;
}
#endif //#if defined(PRE_ADD_CP_RANK)


#if defined( PRE_PVP_GAMBLEROOM )
void CDNGameRoom::CreateGambleRoom( BYTE cGambleType, int nGamblePrice )
{
	if( cGambleType == PvPGambleRoom::Gold )
		nGamblePrice *= 10000 ;

	GetDBConnection()->QueryAddGambleRoom(m_cDBThreadID, GetWorldSetID(), GetRoomID(), cGambleType, nGamblePrice );
}
#endif