#include "stdafx.h"
#include "DnPartyTask.h"

#include "GameSendPacket.h"

#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DNTableFile.h"
#include "DnTableDB.h"
#include "DNGameDataManager.h"
#include "DnPlayerActor.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"

#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNLogConnection.h"
#include "DnDropItem.h"
#include "DnItemTask.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#include "DNIsolate.h"
#if defined( PRE_ADD_NEWCOMEBACK )
#include "DnBlow.h"
#endif

CDnPartyTask::CDnPartyTask(CDNGameRoom * pRoom)
: CTask( pRoom )
, CMultiSingleton<CDnPartyTask, MAX_SESSION_COUNT>( pRoom )
{
	//m_nPartyIndex = 0;
	m_nEnteredGateIndex = -1;
	m_bSyncComplete = false;
	m_nRandomSeed = 0;
	m_bWaitGate = false;
	//m_nZoneCountTick = 0;
	//m_nZoneEnterMapIdx = m_nZoneEnterGate = -1;
	m_bHoldSharingReversionItem = false;
}

CDnPartyTask::~CDnPartyTask()
{
}

bool CDnPartyTask::Initialize()
{
	m_Random.srand(timeGetTime());
	return true;
}

void CDnPartyTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	ULONG nCurTick = GetRoom()->GetGameTick();
	DNVector(_KICKMEMBER)::iterator ii;
	for (ii = m_KickList.begin(); ii != m_KickList.end(); )
	{
		if ((*ii).nKickTick < nCurTick && GetRoom()->GetRoomState() == _GAME_STATE_PLAY)
		{
			int nSeq = 0;
			CDNGameRoom::PartyStruct * pStruct = GetRoom()->GetPartyDatabySessionID((*ii).Kick.nSessionID, nSeq);
			if (pStruct)
			{
				pStruct->pSession->SendBackToVillage( true, (*ii).Kick.cKickKind );
			}
			ii = m_KickList.erase(ii);
		}
		else
			ii++;
	}

	ProcessGetReversionItemList(LocalTime, fDelta);
	/*if (m_nZoneCountTick != 0 && GetRoom()->GetGameTick() > m_nZoneCountTick)
	{
		CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );

		int nStageConstructionLevel = -1;
		if( nStageConstructionLevel == -1 ) nStageConstructionLevel = pTask->GetStageConstructionLevel();
		if( nStageConstructionLevel == -1 ) nStageConstructionLevel = 0;

		m_nRandomSeed = CRandom::Seed(GetRoom());
		m_nEnteredGateIndex = m_nZoneEnterGate;
		pTask->SetGateIndex(m_nZoneEnterGate);

		GetRoom()->InitStateAndSync( m_nZoneEnterMapIdx, m_nZoneEnterGate, CRandom::Seed(GetRoom()), nStageConstructionLevel, true );

		m_nZoneCountTick = 0;
		m_nZoneEnterMapIdx = m_nZoneEnterGate = -1;
	}*/
}

int CDnPartyTask::OnDispatchMessage(CDNUserSession * pSession, int nMainCmd, int nSubCmd, char *pData, int nLen )
{
	switch( nMainCmd ) {
		case CS_ROOM:	return OnRecvRoomMessage(pSession, nSubCmd, pData, nLen );
		case CS_PARTY: return OnRecvPartyMessage(pSession, nSubCmd, pData, nLen );
		default : return ERROR_UNKNOWN_HEADER;
	}
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvRoomMessage(CDNUserSession * pSession, int nSubCmd, char *pData, int nLen )
{
	switch( nSubCmd ) {
		case eRoom::CS_SYNC_WAIT: return OnRecvRoomSyncWait( pSession, (CSSyncWait*)pData, nLen );
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDnPartyTask::OnRecvPartyMessage( CDNUserSession *pSession, int nSubCmd, char *pData, int nLen )
{
	if (pSession->GetState() != SESSION_STATE_GAME_PLAY || pSession->IsCertified() == false)
	{
		g_Log.Log(LogType::_MOVEPACKET_SEQ, pSession, L"OnRecvPartyMessage RState[%d] UState[%d] SCMD[%d]\n", GetRoom()->GetRoomState(), pSession->GetState(), nSubCmd);
		return ERROR_NONE;
	}

	switch( nSubCmd ) {
		case eParty::CS_REFRESHGATEINFO: return OnRecvPartyRefreshGateInfo( pSession, (CSRefreshGateInfo *)pData, nLen );
		case eParty::CS_STARTSTAGE: return OnRecvPartyStartStage( pSession, (CSStartStage *)pData, nLen );
		case eParty::CS_CANCELSTAGE: return OnRecvPartyCancelStage( pSession, pData, nLen );
		case eParty::CS_PARTYMEMBER_KICK: return OnRecvPartyMemberKick(pSession, pData, nLen);
		case eParty::CS_SELECTDUNGEON: return OnRecvPartySelectDungeon(pSession, pData, nLen);
		case eParty::CS_PARTYLEADER_SWAP: return OnRecvPartyLeaderSwap(pSession, pData, nLen);
		case eParty::CS_JOINGETREVERSEITEM: return OnRecvPartyJoinGetReversionItem(pSession, pData);
		case eParty::CS_ABSENT: return OnRecvPartyMemberAbsent(pSession, (CSMemberAbsent*)pData, nLen);
		case eParty::CS_PARTYINVITE: return OnRecvPartyInviteMember(pSession, (CSInvitePartyMember*)pData, nLen);
		case eParty::CS_SWAPMEMBERINDEX: return OnRecvPartySwapMemberIndex(pSession, (CSPartySwapMemberIndex*)pData, nLen);
#if defined( PRE_PARTY_DB )
		case eParty::CS_PARTYMODIFY: return OnRecvPartyModify( pSession, (CSPartyInfoModify*)pData, nLen );
#endif // #if defined( PRE_PARTY_DB )
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
		case eParty::CS_PARTYASKJOINDECISION: return OnRecvPartyAskJoinDecision( pSession, (CSPartyAskJoinDecision*)pData, nLen );
#endif
	}
	_DANGER_POINT();
	return ERROR_UNKNOWN_HEADER;
}

int CDnPartyTask::OnRecvRoomSyncWait( CDNUserSession *pSession, CSSyncWait *pPacket, int nLen )
{
	if (sizeof(CSSyncWait) != nLen)
		return ERROR_INVALIDPACKET;

	if (GetRoom()->GetRoomState() == _GAME_STATE_SYNC2SYNC)
	{
		pSession->SetSessionState(SESSION_STATE_SYNC_READY_2_DELAY);
#ifdef _DEBUG
		g_Log.Log(LogType::_NORMAL, pSession, L"on PT_GAME_SYNC_WAIT : %s\n", pSession->GetCharacterName());
#endif
		return ERROR_NONE;
	}
	
	int iPrevState = pSession->GetState();
	pSession->SetSessionState(SESSION_STATE_READY_TO_PLAY);

	pSession->FlushStoredPacket();
	pSession->FlushPacketQueue();
	for( DWORD i=0; i<GetUserCount(); i++ )
		SendGameSyncWait( GetUserData(i), pSession->GetSessionID() );

#ifdef _DEBUG
	g_Log.Log(LogType::_NORMAL, pSession, L"on PT_GAME_SYNC_WAIT[2] : %s RoomState:%d SessionState=%d\n", pSession->GetCharacterName(), GetRoom()->GetRoomState(), iPrevState );
#endif

	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyRefreshGateInfo( CDNUserSession *pSession, CSRefreshGateInfo *pPacket, int nLen )
{
	if (sizeof(CSRefreshGateInfo) != nLen)
		return ERROR_INVALIDPACKET;

	CDNGameRoom::PartyStruct *pStruct = GetPartyData( pSession );
	if( pStruct ) 
	{
		if( pSession->GetActorHandle() )
		{
			CDnPlayerActor *pPlayer = (CDnPlayerActor*)pSession->GetActorHandle().GetPointer();
			if( pPlayer ) 
				pPlayer->RecvPartyRefreshGateInfo( pPacket->Position );
		}

		pStruct->nEnteredGateIndex = ( pPacket->boEnter ) ? GetCheckGateIndex( pSession ) : -1;

		// Keep AI bots synchronized with human gate entry/exit
		if( !pSession->bIsAIBot() && GetRoom() && GetRoom()->HasAIBots() )
		{
			GateStruct* pTargetGate = ( pStruct->nEnteredGateIndex != -1 )
				? CDnWorld::GetInstance(GetRoom()).GetGateStruct( (char)pStruct->nEnteredGateIndex )
				: NULL;

			for( DWORD i = 0; i < GetUserCount(); ++i )
			{
				CDNUserSession* pMate = GetUserData( i );
				CDNGameRoom::PartyStruct* pMateParty = GetPartyData( i );
				if( !pMate || !pMateParty || !pMate->bIsAIBot() )
					continue;

				if( pStruct->nEnteredGateIndex == -1 )
				{
					pMateParty->nEnteredGateIndex = -1;
				}
				else if( pTargetGate && pTargetGate->pGateArea && pMate->GetActorHandle() && !pMate->GetActorHandle()->IsDie() )
				{
					CDnPlayerActor* pBotActor = static_cast<CDnPlayerActor*>( pMate->GetActorHandle().GetPointer() );
					EtVector3 vBotPos = *pBotActor->GetPosition();
					pBotActor->SetStaticPosition( vBotPos );
					if( pTargetGate->pGateArea->CheckArea( vBotPos, true ) )
						pMateParty->nEnteredGateIndex = pStruct->nEnteredGateIndex;
				}
			}
		}

		if (CheckAndRequestGateIndex(pStruct->nEnteredGateIndex, pSession->GetPartyMemberIndex()) == false)
			pStruct->nEnteredGateIndex = -1;
	}	
	else
		_DANGER_POINT();			//Is it okay if this is missing?

	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyStartStage( CDNUserSession *pSession, CSStartStage *pPacket, int nLen )
{
	if (sizeof(CSStartStage) != nLen)
		return ERROR_INVALIDPACKET;

	m_bWaitGate = false;

	//Ask the master at this flow change timing.
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );

	if( pTask->IsPartyLeader( pSession ) == false )
		return ERROR_NONE;

	int nMapIndex = pTask->GetMapTableID();

	GateStruct *pStruct = CDnWorld::GetInstance(GetRoom()).GetGateStruct( m_nEnteredGateIndex );
	if( m_nEnteredGateIndex == -1 || pStruct == NULL ) {
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
			g_Log.Log(LogType::_ERROR, pSession, L"Failed to start game (party/member validation failed).\n");
		}
		return ERROR_NONE;
	}
	TDUNGEONDIFFICULTY StageDifficulty = pPacket->Difficulty;
	if( StageDifficulty >= Dungeon::Difficulty::Max ) 
		StageDifficulty = pTask->GetStageDifficulty();
	if( StageDifficulty < Dungeon::Difficulty::Easy ) 
		StageDifficulty = Dungeon::Difficulty::Easy;

	switch( pStruct->MapType ) 
	{
		case GlobalEnum::eMapTypeEnum::MAP_VILLAGE:
		{
			// For GM intrusion, send GM along when the user returns to town.
			if (g_pMasterConnectionManager->SendRequestNextVillageInfo(pSession->GetWorldSetID(), nMapIndex, m_nEnteredGateIndex, pPacket->bReturnVillage, GetRoom()) == false)
			{
				for( DWORD i=0; i<GetUserCount(); i++ )
					GetUserData(i)->SendCancelStage( ERROR_GENERIC_MASTERCON_NOT_FOUND );
			}
			break;
		}
		case GlobalEnum::eMapTypeEnum::MAP_WORLDMAP:
			// Also check later here whether cross-world transfer is actually possible.
#if defined(PRE_FIX_INITSTATEANDSYNC)
			GetRoom()->ReserveInitStateAndSync( nMapIndex, m_nEnteredGateIndex, m_nRandomSeed, StageDifficulty, false );
#else
			GetRoom()->InitStateAndSync( nMapIndex, m_nEnteredGateIndex, m_nRandomSeed, StageDifficulty, false );
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
			break;
		case GlobalEnum::eMapTypeEnum::MAP_DUNGEON:
			{
				if (GetRoom()->IsInviting())
				{
					for( DWORD i=0; i<GetUserCount(); i++ )
						GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
					return ERROR_NONE;
				}

#if defined( PRE_PARTY_DB )
				if( GetRoom()->bIsExistBreakIntoUser() )
				{
					for( DWORD i=0; i<GetUserCount(); i++ )
						GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
					return ERROR_NONE;
				}
#endif // #if defined( PRE_PARTY_DB )

#if defined (PRE_WORLDCOMBINE_PARTY)
				if( GetRoom()->bIsWorldCombineParty())
				{
					TDUNGEONDIFFICULTY TargetDifficulty = GetRoom()->GetPartyDifficulty();
					if (TargetDifficulty != StageDifficulty && TargetDifficulty != Dungeon::Difficulty::Max)
					{
						for( DWORD i=0; i<GetUserCount(); i++ )
							GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );

						return ERROR_NONE;
					}
				}
#endif // #if defined (PRE_WORLDCOMBINE_PARTY)

				bool bDirectConnect = false;
				int nGateIndex = m_nEnteredGateIndex;
				switch( CDnWorld::GetInstance(GetRoom()).GetMapType() ) {
					case EWorldEnum::MapTypeDungeon:
						// If previously in a dungeon, continue carrying over the dungeon level.
						StageDifficulty = pTask->GetStageDifficulty();
						break;
					case EWorldEnum::MapTypeWorldMap:
					case EWorldEnum::MapTypeVillage:
						// If previously in a world map or town 
						// Quick check at start here... can't trust the client... only check when entering dungeon from world map.
						// If returned, either an unavailable level was selected or started despite level restriction.
						bool bCheckFatigue = true;
						
#if defined(PRE_ADD_WORLD_EVENT) // This will be removed.
#else
						TEvent * pEvent = GetRoom()->GetApplyEvent(_EVENT_1_FATIGUE);
						if (pEvent)
							bCheckFatigue = false;
#endif //#if defined(PRE_ADD_WORLD_EVENT)

						if( m_nEnteredGateIndex != -1 ) {
							if( pStruct->MapType == EWorldEnum::MapTypeDungeon ) {
								DungeonGateStruct *pDungeonGate = NULL;

								short nCancelCode = ERROR_NONE;
								//bool bCancel = false;
								if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
									pDungeonGate = (DungeonGateStruct *)pStruct;
									if( pPacket->cSelectDungeonIndex < 0 || pPacket->cSelectDungeonIndex >= (char)pDungeonGate->pVecMapList.size() )
										nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL;
									//bCancel = true;
									else {
										pDungeonGate = (DungeonGateStruct *)pDungeonGate->pVecMapList[pPacket->cSelectDungeonIndex];

										nGateIndex = pDungeonGate->cStartGateIndex;
										nMapIndex = pDungeonGate->nMapIndex;
										bDirectConnect = true;		
										pSession->VerifyValidMap(nMapIndex);
									}
								}
								else {
									pDungeonGate = (DungeonGateStruct *)pStruct;
								}
								if( pDungeonGate->nFatigue[ StageDifficulty ] == 0 ) bCheckFatigue = false;

								// Check fatigue
								if (bCheckFatigue)
								{
									if (CheckPartyFatigue() == false)		//If in dungeon, check fatigue once.
									{
										for( DWORD i=0; i<GetUserCount(); i++ )
											GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL_FATIGUE_SHORTAGE );
										return ERROR_NONE;
									}
								}					

								switch( pTask->GetGameTaskType() ) 
								{
									case GameTaskType::DarkLair:
										StageDifficulty = Dungeon::Difficulty::Abyss;
										break;
								}
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
								if( pDungeonGate->cCanDifficult[ StageDifficulty ] != TRUE )
									nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL;

								if( nCancelCode == ERROR_NONE )
								{
									EWorldEnum::PermitGateEnum PermitFlag = EWorldEnum::PermitEnter;
									if( g_pDataManager->IsCloseGateByTime(nMapIndex) )
										PermitFlag =  EWorldEnum::PermitClose;

									if(PermitFlag == EWorldEnum::PermitEnter)
										PermitFlag = pDungeonGate->CanEnterDungeon( GetRoom());

									if( PermitFlag !=  EWorldEnum::PermitEnter )
									{
										if( PermitFlag & EWorldEnum::PermitLessPlayerLevel )
											nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL_ACTLEVEL_SHORTAGE;
										else if( PermitFlag & EWorldEnum::PermitExceedTryCount )
											nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL_EXCEED_TRY_COUNT;
										else if( PermitFlag & EWorldEnum::PermitNotEnoughItem )
											nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL_DUNGEONNEEDITEM_SHORTAGE;
										else if( PermitFlag & EWorldEnum::PermitClose )
											nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL_NOT_AVAILABLE_TIME;
										else
											nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL;
									}
								}
#else
								if( nCancelCode == ERROR_NONE && ( pDungeonGate->cCanDifficult[ StageDifficulty ] != TRUE || pDungeonGate->CanEnterDungeon( GetRoom() ) != EWorldEnum::PermitEnter ) ) {
									nCancelCode = ERROR_PARTY_STARTSTAGE_FAIL_ACTLEVEL_SHORTAGE;
								}
#endif	//#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
								if( nCancelCode != ERROR_NONE ) {
									for( DWORD i=0; i<GetUserCount(); i++ ) {
										GetUserData(i)->SendCancelStage( nCancelCode );
									}
									return ERROR_NONE;
								}
#if defined(PRE_ADD_CP_RANK)
								// Query best ranking when in Abyss mode.
								if(pTask->GetGameTaskType() != GameTaskType::DarkLair && StageDifficulty == Dungeon::Difficulty::Abyss)
								{
									CDNUserSession* pSession = NULL;
									bool bFirst = true;

									for( DWORD i=0; i<GetUserCount(); i++ ) 
									{
										pSession = GetUserData(i);
										if( pSession )
										{
											if( bFirst )
											{
												pSession->GetDBConnection()->QueryGetStageClearBest(pSession, nMapIndex);
												bFirst = false;
											}
											pSession->GetDBConnection()->QueryGetStageClearPersonalBest(pSession, nMapIndex);
										}
									}
								}
#endif //#if defined(PRE_ADD_CP_RANK)

								// Stage start log
								BYTE cThreadID;
								CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
								if( pDBCon )
								{
									pDBCon->QueryAddStageStartLog( cThreadID, GetRoom()->GetWorldSetID(), 0, GetRoom()->GetRoomLogIndex(), GetRoom()->GetPartyLogIndex(), static_cast<BYTE>(GetUserCount()), nMapIndex, static_cast<DBDNWorldDef::DifficultyCode::eCode>(StageDifficulty+1) );
									GetRoom()->SetStageStartLogFlag( true );
									GetRoom()->SetStageEndLogFlag( false );
								}
							}
						}
						break;
				}

#if defined(PRE_FIX_INITSTATEANDSYNC)
				GetRoom()->ReserveInitStateAndSync( nMapIndex, nGateIndex, m_nRandomSeed, StageDifficulty, bDirectConnect );
#else
				GetRoom()->InitStateAndSync( nMapIndex, nGateIndex, m_nRandomSeed, StageDifficulty, bDirectConnect );
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
			}
			break;
		default:
			{
				// Map Type is Unknown
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
				}
				g_Log.Log(LogType::_ERROR, pSession, L"Failed to start game (session/map validation failed).\n");
			}
			break;
	}
		
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyCancelStage( CDNUserSession *pSession, char *pPacket, int nLen )
{
	if (nLen != 0)
		return ERROR_INVALIDPACKET;

	m_bWaitGate = false;
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if( GetUserData(i) && GetUserData(i)->bIsAIBot() )
			GetPartyData(i)->nEnteredGateIndex = -1;
		GetUserData(i)->SendCancelStage( ERROR_NONE );
	}
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyMemberKick(CDNUserSession * pSession, char * pPacket, int nLen)
{
	CSPartyMemberKick * pKick = (CSPartyMemberKick*)pPacket;

	if (sizeof(CSPartyMemberKick) != nLen)
		return ERROR_INVALIDPACKET;

	CDNGameRoom::PartyStruct * pPartyStruct = GetPartyData(pSession);
	if (pPartyStruct && pPartyStruct->bLeader == true)
	{
		if (pPartyStruct->nEnteredGateIndex != -1)	//Only possible when not standing on a gate
		{
			_DANGER_POINT();		//Added because client must not send this
			return ERROR_NONE;
		}


		//If normal, do not run immediately; wait 5 seconds
		int nSeq = 0;
		CDNGameRoom::PartyStruct * pStruct = GetRoom()->GetPartyDatabySessionID(pKick->nSessionID, nSeq);
		if (pStruct)
		{
			if (pStruct->pSession->GetAbsent() == false)
				return ERROR_NONE;

			_KICKMEMBER kick;
			memset(&kick, 0, sizeof(kick));

			kick.nKickTick = GetRoom()->GetGameTick() + (5 * 1000);
			kick.nSessionID = pSession->GetSessionID();
			kick.nAccountDBID = pSession->GetAccountDBID();
			kick.cWorldID = (BYTE)pSession->GetWorldSetID();
			kick.biCharacterDBID = pSession->GetCharacterDBID();
			memcpy(&kick.Kick, pKick, sizeof(CSPartyMemberKick));

			m_KickList.push_back(kick);

			pStruct->pSession->SendCountDownMsg(true, 5, 3537, 2);
		}
	}
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartySelectDungeon(CDNUserSession * pSession, char * pPacket, int nLen)
{
	CSSelectDunGeon * pMsg = (CSSelectDunGeon*)pPacket;

	if (sizeof(CSSelectDunGeon) != nLen)
		return ERROR_INVALIDPACKET;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if( GetUserData(i) == pSession ) continue;
		GetUserData(i)->SendSelectDungeonInfo(pMsg->nMapIndex, pMsg->cDiffyculty);
	}
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyLeaderSwap(CDNUserSession * pSession, char * pPacket, int nLen)
{
	CSPartyLeaderSwap * pSwap = (CSPartyLeaderSwap*)pPacket;

	if (sizeof(CSPartyLeaderSwap) != nLen)
		return ERROR_INVALIDPACKET;

	if (GetRoom())
	{
		int nSeq = 0;
		CDNGameRoom::PartyStruct * pLeaderStruct = GetRoom()->GetPartyDatabySessionID(pSession->GetSessionID(), nSeq);
		if ( pLeaderStruct && pLeaderStruct->bLeader == true)
		{
			int nNewSeq = 0;
			CDNGameRoom::PartyStruct * pNewLeaderStruct = GetRoom()->GetPartyDatabySessionID(pSwap->nSessionID, nNewSeq);
			if (pNewLeaderStruct)
			{
				if (pNewLeaderStruct->pSession->bIsGMTrace())
					return ERROR_PARTY_LEADER_APPOINTMENTFAIL;

				pLeaderStruct->bLeader = false;
				pNewLeaderStruct->bLeader = true;
				UpdateGateInfo();

				for(DWORD i=0; i<GetUserCount(); i++)
				{
					if (GetUserData(i) != NULL)
						GetUserData(i)->SendSwapPartyLeader(pSwap->nSessionID, ERROR_NONE);
				}

#ifdef PRE_FIX_PARTY_STAGECLEAR_CHANGEMASTER
				CDnGameTask *pTask = static_cast<CDnGameTask *>(GetRoom()->GetTaskMng()->GetTask( "GameTask" ));
				if( pTask ) {
					pTask->SetWarpDungeonClearToLeaderCheck(true);
				}
#endif

				return ERROR_NONE;
			}
		}
	}

	pSession->SendSwapPartyLeader(0, ERROR_PARTY_LEADER_APPOINTMENTFAIL);	
	return ERROR_NONE;
}

void CDnPartyTask::RequestPartyMember()
{
	int nCnt = 0;
	UINT nMember[PARTYMAX];	
	memset(nMember, 0, sizeof(nMember));

	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		if (GetUserData(i) != NULL)
		{
			nMember[nCnt] = GetUserData(i)->GetSessionID();
			nCnt++;
		}
	}

	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		SendRefreshPartyMember(GetUserData(i), nMember, nCnt);
	}
}

void CDnPartyTask::RequestSyncStart()
{
	SetSyncComplete( true );

	CDnGameTask *pTask = static_cast<CDnGameTask *>(GetRoom()->GetTaskMng()->GetTask( "GameTask" ));
	if( pTask ) {
		pTask->RequestGateInfo();
	}
	_RequestSyncStartMsg();
}

void CDnPartyTask::_RequestSyncStartMsg()
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		CDNUserSession* pGameSession = GetUserData(i);
		if( GetRoom() && pGameSession )
			GetRoom()->OnRequestSyncStartMsg( pGameSession );
	}
}

bool CDnPartyTask::CheckPartyFatigue()
{
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if (GetUserData(i)->bIsAIBot())
			continue;
		if ((GetUserData(i)->GetFatigue() <= 0) && (GetUserData(i)->GetWeeklyFatigue() <= 0) && (GetUserData(i)->GetPCBangFatigue() <= 0) && (GetUserData(i)->GetEventFatigue() <= 0) && (GetUserData(i)->GetVIPFatigue() <= 0))
			return false;
	}
	return true;
}

bool CDnPartyTask::CheckAndRequestGateIndex(int nGateNo, int nMemberIdx)
{
	if( m_bWaitGate ) 
		return false;

	char cGateNo[PARTYMAX];
	memset(cGateNo, -1, sizeof(cGateNo));

	if( GetRoom() && GetRoom()->bIsFarmRoom() )
		return false;

	// Block gate movement if someone is dropping in.
	if (GetRoom())
	{
		bool bCheck = false;
		int nRet = ERROR_NONE;
		if (GetRoom()->bIsExistBreakIntoUser())
		{
#if !defined( _FINAL_BUILD )
			g_Log.Log( LogType::_NORMAL, L"Gate transition blocked: a player is currently breaking into the room.\r\n" );
#endif
			bCheck = true;
			nRet = ERROR_PARTY_REFRESHGATEFAIL_REQRETREAT;
		}

		if (GetRoom()->IsInviting())
		{
#if !defined( _FINAL_BUILD )
			g_Log.Log( LogType::_NORMAL, L"Gate transition blocked: party invitation is in progress.\r\n" );
#endif
			bCheck = true;
			nRet = ERROR_PARTY_REFRESHGATEFAIL_REQRETREAT;
		}

		if (IsPartySharingReversionItem())
		{
			bCheck = true;
			nRet = ERROR_PARTY_DUNGEONWARPOUT_FAIL_ITEMSHARING;
		}

		if (bCheck)
		{
			//If an error occurred, include the gate value and send.
			if (nGateNo >= 0 && nMemberIdx >= 0 && nMemberIdx < PARTYMAX)
				cGateNo[nMemberIdx] = (char)nGateNo;

			SendRefreshGate(cGateNo, nRet);
			return false;
		}
	}

	bool bAllSame = false;
	std::vector<std::pair<int, int>> vGate;
	std::vector<std::pair<int, int>>::iterator ii;

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	m_nEnteredGateIndex = -1;
	pTask->SetGateIndex( -1 );

	int nDeadCount = 0;
	int nGMCount   = 0;

	int nUserCount = GetUserCount();

	for(int i = 0; i < nUserCount; i++)
	{	
		if(GetUserData(i)->GetActorHandle() && GetUserData(i)->GetActorHandle()->IsDie())
		{
			nDeadCount++;
			continue;
		}
		if( GetUserData(i)->bIsGMTrace() )
		{
			++nGMCount;
			continue;
		}

		int nMemberIndex = GetUserData(i)->GetPartyMemberIndex();
		if( nMemberIndex < 0 || nMemberIndex >= PARTYMAX )
			nMemberIndex = i;
		if( nMemberIndex < 0 || nMemberIndex >= PARTYMAX ) continue;
		cGateNo[nMemberIndex] = (char)GetPartyData(i)->nEnteredGateIndex;

		if (cGateNo[nMemberIndex] != -1)
		{
			bool bFoundGate = false;
			for (ii = vGate.begin(); ii != vGate.end(); ii++)
			{
				if ((*ii).first == cGateNo[nMemberIndex])
				{
					(*ii).second++;
					bFoundGate = true;
					break;
				}
			}
			if( !bFoundGate )
				vGate.push_back(std::make_pair(cGateNo[nMemberIndex], 1));
		}
	}

	//Update gate info.
	SendRefreshGate(cGateNo, ERROR_NONE);

	//Find which condition is satisfied.
	int nTargetGate = -1;
	for (ii = vGate.begin(); ii != vGate.end(); ii++)
	{
		if ((*ii).second == (nUserCount - nDeadCount - nGMCount)) {
			bAllSame = true;
			nTargetGate = (*ii).first;
		}
	}

	char cDungeonEnterCount = 0;
	char cCanEnter[5][5] = { false, };
	char cPermitFlag[5] = { EWorldEnum::PermitEnter, };
	if (bAllSame)
	{
		if( nTargetGate != -1 )
		{
			GateStruct *pStruct = CDnWorld::GetInstance(GetRoom()).GetGateStruct( nTargetGate );
			if( pStruct && pStruct->PermitFlag != EWorldEnum::PermitEnter )
				return false;

#if defined( PRE_PARTY_DB )
			if( pStruct->MapType == EWorldEnum::MapTypeVillage )
			{
				int iPermitLevel = g_pDataManager->GetMapPermitLevel( pStruct->nMapIndex );
				for( DWORD i=0 ; i<GetUserCount() ; ++i )
				{
					CDNUserSession* pTempSession  = GetUserData(i);
					if( pTempSession == NULL || pTempSession->bIsAIBot() )
						continue;
					if( pTempSession->GetLevel() < iPermitLevel )
						return false;
				}
			}
#endif // #if defined( PRE_PARTY_DB )
		}

		m_nRandomSeed = CRandom::Seed(GetRoom());//_rand(GetRoom());
		m_nEnteredGateIndex = nTargetGate;
		pTask->SetGateIndex( nTargetGate );

		if( nTargetGate != -1 ) {
			GateStruct *pStruct = CDnWorld::GetInstance(GetRoom()).GetGateStruct( nTargetGate );
			if( pStruct ) 
			{
				if( pStruct->MapType == EWorldEnum::MapTypeDungeon ) 
				{
					m_nRandomSeed = pTask->CheckRecoveryDungeonHistorySeed( pStruct->nMapIndex, m_nRandomSeed );

					// For a dungeon entrance, also send CanEnter info in GateInfo.
					if( CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeWorldMap ) {
						DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pStruct;
						if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
							cDungeonEnterCount = (char)pDungeonStruct->pVecMapList.size();
							for( int i=0; i<cDungeonEnterCount; i++ ) {
								memcpy( cCanEnter[i], ((DungeonGateStruct *)pDungeonStruct->pVecMapList[i])->cCanDifficult, sizeof(bool)*5 );
								cPermitFlag[i] = ((DungeonGateStruct *)pDungeonStruct->pVecMapList[i])->PermitFlag;
							}
						}
						else {
							cDungeonEnterCount = 1;
							memcpy( cCanEnter[0], pDungeonStruct->cCanDifficult, sizeof(char)*5 );
							cPermitFlag[0] = pDungeonStruct->PermitFlag;
						}
					}
				}
			}
		}

		for( DWORD i=0; i<GetUserCount(); i++ ) {
			GetUserData(i)->SendGateInfo( -1, (char)nTargetGate, cDungeonEnterCount, (char*)cCanEnter, (char*)cPermitFlag );
		}
		m_bWaitGate = true;
	}
	return true;
}

/*
void CDnPartyTask::CalcDungeonEnterGateCondition( CDnWorld::DungeonGateStruct *pStruct, short &nDungeonEnterPermit )
{
	//Check required items when entering dungeon
	UINT nNeedItemID = pStruct->nNeedItemID;
	int nNeedItemCount = pStruct->nNeedItemCount;
	if (nNeedItemID > 0)
	{
		for( DWORD j=0; j<GetUserCount(); j++ ) {
			nDungeonEnterPermit = GetUserData(j)->GetItem()->GetInventoryItemCount(nNeedItemID) >= nNeedItemCount ? ERROR_NONE : ERROR_PARTY_STARTSTAGE_FAIL_DUNGEONNEEDITEM_SHORTAGE;
			if( nDungeonEnterPermit != ERROR_NONE ) break;
		}
	}
}
*/

int CDnPartyTask::GetCheckGateIndex( CDNUserSession *pSession )
{
	if( !pSession ) return -1;
	if( !pSession->GetActorHandle() ) return -1;
	if (!GetRoom()) return -1;

	CDnPlayerActor *pPlayer = (CDnPlayerActor*)pSession->GetActorHandle().GetPointer();
	if(! pPlayer ) return -1;
	if( pPlayer->IsDie() || pPlayer->IsHit() ) return -1;

	for( DWORD i=0; i<CDnWorld::GetInstance(GetRoom()).GetGateCount(); i++ ) {
		GateStruct *pStruct = CDnWorld::GetInstance(GetRoom()).GetGateStructFromIndex(i);
		if (pStruct == NULL) continue;
		if( !pStruct->pGateArea ) continue;
		if( !pStruct->pGateArea->IsActive() ) continue;
		if( pStruct->pGateArea->CheckArea( *pPlayer->GetStaticPosition(), true ) == false ) continue;
		return (int)pStruct->cGateIndex;
	}

	return -1;
}

//void CDnPartyTask::ResetSync()
//{
//	m_bSyncComplete = false;
//	for( DWORD i=0; i<GetUserCount(); i++ ) {
//		GetPartyData(i)->pSession->SetSessionState(SESSION_STATE_READY_TO_SYNC);
//		//bSync = false;
//	}
//}

void CDnPartyTask::ResetGateIndex()
{
	m_bWaitGate = false;
	m_nEnteredGateIndex = -1;
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		GetPartyData(i)->nEnteredGateIndex = -1;
	}
}

void CDnPartyTask::OutPartyMember( UINT nOutPartyMemberUID, UINT nNewLeaderUID, char cKickKind )
{
	CDNUserSession *pSession;
	CDNUserSession *pOutSession = GetRoom()->GetUserSession( nOutPartyMemberUID );

	if (pOutSession)
		pOutSession->SetOutedMember();

#if defined( PRE_PARTY_DB )
	if(pOutSession)
	{
		if ( pOutSession->GetGameRoom()->GetGameType() != REQINFO_TYPE_SINGLE )
		{
			DelPartyMemberDB( pOutSession );
		}
	}
#endif

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		pSession = GetUserData(i);
		bool bSendPartyOut = false;

		if (pSession->GetGameRoom()->GetGameType() != REQINFO_TYPE_SINGLE || pSession->bIsGMTrace() || pSession->GetGameRoom()->bIsFarmRoom() )
			bSendPartyOut = true;

		if( pOutSession && pOutSession->bIsGMTrace() ) bSendPartyOut = true;

#if defined( PRE_WORLDCOMBINE_PARTY )
		if(pSession->GetOutedMember() && pSession->GetSessionID() != nOutPartyMemberUID)
			bSendPartyOut = false;
#endif

		if( bSendPartyOut )
			pSession->SendPartyOut(ERROR_NONE, cKickKind, nOutPartyMemberUID);
		
		if (pSession->GetSessionID() == nOutPartyMemberUID)
		{
#if !defined( PRE_PARTY_DB )
			if (GetUserCountWithoutPartyOutUser() == 1)
			{
				// Party end log
				BYTE cThreadID;
				CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
				if( pDBCon )
					pDBCon->QueryAddPartyEndLog( cThreadID, GetRoom()->GetWorldSetID(), 0, GetRoom()->GetPartyLogIndex() );
			}
#endif
			continue;
		}

		if (nNewLeaderUID > 0)
		{
			pSession->SendChangeLeader(nNewLeaderUID);
#ifdef PRE_FIX_PARTY_STAGECLEAR_CHANGEMASTER
			CDnGameTask *pTask = static_cast<CDnGameTask *>(GetRoom()->GetTaskMng()->GetTask( "GameTask" ));
			if( pTask ) {
				pTask->SetWarpDungeonClearToLeaderCheck(true);
			}
#endif
		}
	}
#if defined( PRE_ADD_NEWCOMEBACK )
	if( pOutSession && pOutSession->GetComebackAppellation() > 0 )
	{
		DelPartyMemberAppellation( pOutSession->GetComebackAppellation() );
	}
#endif
}

void CDnPartyTask::UpdateGateInfo()
{
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	// Refresh and send gate info if there are any gate changes.
	if( CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeWorldMap ) {
		CDnGameTask *pLocalTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
		// On exit, refresh and send even if there is no gate change.
		// Do not use an if check because we need to update the list of entries that do not meet the condition.
		CDnWorld::GetInstance(GetRoom()).RefreshGate( pLocalTask->GetMapTableID() );
		CDnWorld::GetInstance(GetRoom()).RefreshDungeonDifficult( pLocalTask->GetMapTableID() );
		pLocalTask->RequestGateInfo();
	}

	// If in dungeon-entry state, cancel it.
	if( m_bWaitGate ) {
		m_bWaitGate = false;
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			GetUserData(i)->SendCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
		}
	}

	// If in dungeon clear state, handle according to each state.
	pTask->RefreshDungeonClearState();
}

#define MAX_DUPLICATE_DICE_COUNT	5
#define MAX_SERVER_DICE_WAIT_MSEC	60

void CDnPartyTask::AddRequestGetReversionItemUserInfo(ReversionItemGetUnit& unit, bool bSendUserJoin)
{
	unit.userInfo.clear();

	if (!unit.hDropItem)
	{
		return;
	}

	UINT i = 0;
	for (; i < GetUserCount() ; ++i)
	{
		CDNUserSession* pGameSession = GetUserData(i);
		if (GetRoom() && pGameSession )
		{
			if (pGameSession->bIsGMTrace() || pGameSession->bIsAIBot())
				continue;

			if( !pGameSession->GetActorHandle() )
				continue;

			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(pGameSession->GetActorHandle().GetPointer());
			if (pPlayer->IsDie())
				continue;

			ReversionItemGetUserInfo getUserinfo;
			getUserinfo.sessionID = pGameSession->GetSessionID();
			unit.userInfo.push_back(getUserinfo);

			if (bSendUserJoin)
				pGameSession->SendRequestJoinGetReversionItem(unit.itemInfo, unit.hDropItem->GetUniqueID());
		}
	}
}

void CDnPartyTask::AddRequestGetReversionItem(const TItem& itemInfo, DnDropItemHandle hDropItem)
{
	// todo : set?
	if (IsEnableAddRequestGetReversionItem(hDropItem) == false)
		return;

	ReversionItemGetUnit unit;
	unit.itemInfo	= itemInfo;
	unit.hDropItem	= hDropItem;

	AddRequestGetReversionItemUserInfo(unit, m_RequestGetReversionItemList.empty());

	m_RequestGetReversionItemList.push_back(unit);
}

void CDnPartyTask::SendRequestJointGetReversionItem(const TItem& orgItemInfo, const DWORD& dropUniqueID)
{
	UINT i = 0;
	TItem itemInfo = orgItemInfo;
	for (; i < GetUserCount() ; ++i)
	{
		CDNUserSession* pGameSession = GetUserData(i);
		if (GetRoom() && pGameSession )
		{
			if (pGameSession->bIsGMTrace() || pGameSession->bIsAIBot())
				continue;

			if( !pGameSession->GetActorHandle() )
				continue;

			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(pGameSession->GetActorHandle().GetPointer());
			if (pPlayer->IsDie())
				continue;

			pGameSession->SendRequestJoinGetReversionItem(itemInfo, dropUniqueID);
		}
	}
}

bool CDnPartyTask::IsEnableAddRequestGetReversionItem(DnDropItemHandle hDropItem)
{
	if (!hDropItem)
		return false;

	std::deque<ReversionItemGetUnit>::iterator iter = m_RequestGetReversionItemList.begin();
	for (; iter != m_RequestGetReversionItemList.end(); ++iter)
	{
		ReversionItemGetUnit& info = *iter;
		if (!info.hDropItem)
			return false;
		if (info.hDropItem->GetUniqueID() == hDropItem->GetUniqueID())
			return false;
	}

	return true;
}

bool CDnPartyTask::IsSameDiceNumber(const std::vector<ReversionItemGetUserInfo>& userList, int diceNumber) const
{
	std::vector<ReversionItemGetUserInfo>::const_iterator iter = userList.begin();
	for (; iter != userList.end(); ++iter)
	{
		const ReversionItemGetUserInfo& curUserInfo = *iter;
		if (curUserInfo.diceValue == diceNumber)
			return true;
	}

	return false;
}

int CDnPartyTask::Dice(const std::vector<ReversionItemGetUserInfo>& userList)
{
	int i = 0;
	int diceValue = 0;

	for (; i < MAX_DUPLICATE_DICE_COUNT; ++i)
	{
		diceValue = m_Random.rand(0, MAXPARTYDICEGAMENUMBER);
		if (IsSameDiceNumber(userList, diceValue) == false)
			break;
	}

	if (i >= MAX_DUPLICATE_DICE_COUNT)
	{
		_DANGER_POINT();
		return 0;			//	You are REALLY Unlucky guy!
	}

	return diceValue;
}

int CDnPartyTask::OnRecvPartyJoinGetReversionItem(CDNUserSession *pSession, char *pPacket)
{
	CSJoinGetReversionItem* pJoinGet = (CSJoinGetReversionItem*)pPacket;

	if (m_RequestGetReversionItemList.empty())
	{
		_DANGER_POINT();
		return ERROR_ITEM_FAIL;
	}

	if (pSession && pJoinGet)
	{
		int diceValue = 0;
		ReversionItemGetUnit& current = m_RequestGetReversionItemList.front();
		std::vector<ReversionItemGetUserInfo>& curUserList = current.userInfo;
		std::vector<ReversionItemGetUserInfo>::iterator iter = curUserList.begin();
		for (; iter != curUserList.end(); ++iter)
		{
			ReversionItemGetUserInfo& info = *iter;
			if (info.sessionID == pSession->GetSessionID())
			{
#ifdef PRE_FIX_RECURSIVE_DICE
				if (info.bJoinDecided)
					return ERROR_NONE;
#endif
				diceValue = info.diceValue = (pJoinGet->bJoin) ? Dice(curUserList) : -1;
				info.bJoinDecided = true;
				break;
			}
		}

		UINT i = 0;
		for (; i < GetUserCount() ; ++i)
		{
			CDNUserSession* pGameSession = GetUserData(i);
			if (GetRoom() && pGameSession)
			{
				int weight = 0;

				if (GetRoom()->bIsPartyJobDice() )
				{
					if (pSession)
					{
						if (g_pDataManager->IsItemNeedJobHistory(current.itemInfo.nItemID, pSession->GetStatusData()->cJobArray, true))
							weight = MAXPARTYDICEGAMENUMBER + 1;
					}
				}
				pGameSession->SendRollDiceForGetReversionItem(pSession->GetSessionID(), pJoinGet->bJoin, diceValue + weight);
			}
		}
	}

	return ERROR_NONE;
}

void CDnPartyTask::ClearReversionItemList()
{
	std::deque<ReversionItemGetUnit>::iterator iter = m_RequestGetReversionItemList.begin();
	for (; iter != m_RequestGetReversionItemList.end(); ++iter)
	{
		ReversionItemGetUnit& unit = *iter;
		SAFE_RELEASE_SPTR(unit.hDropItem);
	}

	m_RequestGetReversionItemList.clear();
	m_bHoldSharingReversionItem = false;
}

void CDnPartyTask::ProcessGetReversionItemList(LOCAL_TIME LocalTime, float fDelta)
{
	if (m_RequestGetReversionItemList.empty() == false && m_bHoldSharingReversionItem == false)
	{
		DWORD decideUserCount = 0;
		ReversionItemGetUserInfo winnerInfo;

		ReversionItemGetUnit& current = m_RequestGetReversionItemList.front();
		std::vector<ReversionItemGetUserInfo>& curUserList = current.userInfo;
		std::vector<ReversionItemGetUserInfo>::iterator iter = curUserList.begin();
		for (; iter != curUserList.end();)
		{
			ReversionItemGetUserInfo& userInfo = *iter;
			int partyIdx = 0;
			if (GetRoom()->GetUserSession(userInfo.sessionID) == NULL || 
				GetRoom()->GetPartyDatabySessionID(userInfo.sessionID, partyIdx) == NULL)
			{
				iter = curUserList.erase(iter);
				continue;
			}

			if (userInfo.bJoinDecided != false)
			{
				if (userInfo.diceValue >= 0)
				{
					int jobWeight = 0;

					if (GetRoom() && GetRoom()->bIsPartyJobDice() )
					{
						CDNUserSession* pSession = GetRoom()->GetUserSession(userInfo.sessionID);
						if (pSession)
						{
							if (g_pDataManager->IsItemNeedJobHistory(current.itemInfo.nItemID, pSession->GetStatusData()->cJobArray, true))
								jobWeight = MAXPARTYDICEGAMENUMBER + 1;
						}
					}
					int resultValue = userInfo.diceValue + jobWeight;
					if (resultValue > winnerInfo.diceValue)
					{
						winnerInfo = userInfo;
						winnerInfo.diceValue = resultValue;
					}
				}
				++decideUserCount;
			}

			 ++iter;
		}

		/*
		if (decideUserCount > (int)curUserList.size())
		{
			_DANGER_POINT();
			return;
		}
		*/

		if (decideUserCount >= (int)curUserList.size() || current.time > MAX_SERVER_DICE_WAIT_MSEC)
		{
			if (winnerInfo.IsEmpty() == false)
			{
				CDNUserSession* pWinnerSession = GetRoom()->GetUserSession(winnerInfo.sessionID);
				if (pWinnerSession)
				{
					int cSlotIndex = -1;
					// SC_ITEM-related message is sent only to the player who actually picked up the item
					// Since sending a separate message like '[Who] acquired [which item]' is awkward
					// Send the info together when sending to DropItem.
					char cLootRule = ITEMLOOTRULE_NONE;
					DnActorHandle hActor = pWinnerSession->GetActorHandle();
					DWORD dwOnwerID = hActor->GetUniqueID();					
					if (current.hDropItem)
					{
						current.hDropItem->SetOwnerUniqueID(dwOnwerID);
						current.hDropItem->LockReversionItem(false);
						CDnItemTask::GetInstance(GetRoom()).PickUpItem( hActor, current.hDropItem, ITEMLOOTRULE_NONE );
					}

					UINT i = 0;
					for (; i < GetUserCount() ; ++i)
					{
						CDNUserSession* pGameSession = GetUserData(i);
						if (GetRoom() && pGameSession)
						{
							DWORD sessionId = UINT_MAX;
							if (current.hDropItem)
								sessionId = current.hDropItem->GetUniqueID();
							pGameSession->SendResultGetReversionItem(pWinnerSession->GetSessionID(), current.itemInfo, sessionId);
						}
					}

					m_RequestGetReversionItemList.pop_front();

					if (m_RequestGetReversionItemList.empty() == false)
					{
						ReversionItemGetUnit& unit = m_RequestGetReversionItemList.front();

						if (!unit.hDropItem)
						{
							g_Log.Log(LogType::_DANGERPOINT, pWinnerSession, L"ProcessGetReversionItemList nItemID:%d m_RequestGetReversionItemList.size(%d) UserCount(%d) == userInfoCount(%d) RState[%d] UState[%d] \n", 
									unit.itemInfo.nItemID, m_RequestGetReversionItemList.size(), GetUserCount(), unit.userInfo.size(), GetRoom()->GetRoomState(), pWinnerSession->GetState());
						}
						else
						{
							SendRequestJointGetReversionItem(unit.itemInfo, unit.hDropItem->GetUniqueID());
						}
					}
				}
				else
				{
					_DANGER_POINT();
				}
			}
			else
			{
				UINT i = 0;
				for (; i < GetUserCount() ; ++i)
				{
					CDNUserSession* pGameSession = GetUserData(i);
					if (GetRoom() && pGameSession)
						pGameSession->SendResultGetReversionItem(0, current.itemInfo, UINT_MAX);
				}

				if (current.hDropItem)
					current.hDropItem->LockReversionItem(false);

				m_RequestGetReversionItemList.pop_front();

				if (m_RequestGetReversionItemList.empty() == false)
				{
					ReversionItemGetUnit& unit = m_RequestGetReversionItemList.front();
					if (!unit.hDropItem)
					{
						g_Log.Log(LogType::_DANGERPOINT, L"ProcessGetReversionItemList(NoWINNER_Info) nItemID:%d m_RequestGetReversionItemList.size(%d) UserCount(%d) == userInfoCount(%d) RState[%d]\n", 
							unit.itemInfo.nItemID, m_RequestGetReversionItemList.size(), GetUserCount(), unit.userInfo.size(), GetRoom()->GetRoomState());
					}
					else
					{
						SendRequestJointGetReversionItem(unit.itemInfo, unit.hDropItem->GetUniqueID());
					}
				}
			}

			OutputDebug("---------- DICE END ----------\n");
		}

		current.time += fDelta;
	}
}

bool CDnPartyTask::IsPartySharingReversionItem() const
{
	return (m_RequestGetReversionItemList.empty() == false);
}

void CDnPartyTask::SendRefreshGate(char * pGateNo, int nRet)
{
	//Update gate info.
	int nUserCount = GetRoom()->GetPartyIndex() > 0 ? GetRoom()->GetPartyMemberMax() : (int)GetUserCount();
	if( nUserCount < (int)GetUserCount() )
		nUserCount = (int)GetUserCount();
	for( DWORD i = 0; i < GetUserCount(); ++i )
	{
		CDNUserSession* pSess = GetUserData( i );
		if( pSess )
		{
			int nMemberIdx = pSess->GetPartyMemberIndex();
			if( nMemberIdx >= 0 && nMemberIdx < PARTYMAX && nMemberIdx + 1 > nUserCount )
				nUserCount = nMemberIdx + 1;
		}
	}
	if( nUserCount > PARTYMAX )
		nUserCount = PARTYMAX;

	for(int i = 0; i < (int)GetUserCount(); i++)
	{
		CDNUserSession* pSess = GetUserData(i);
		if( pSess && !pSess->bIsAIBot() )
			pSess->SendRefreshGateInfo(pGateNo, nUserCount, nRet);
	}
}

int CDnPartyTask::GetHumanEnteredGateIndex()
{
	for( DWORD i = 0; i < GetUserCount(); ++i )
	{
		CDNUserSession* pSess = GetUserData( i );
		CDNGameRoom::PartyStruct* pStruct = GetPartyData( i );
		if( !pSess || !pStruct || pSess->bIsAIBot() || pSess->bIsGMTrace() )
			continue;
		if( pStruct->nEnteredGateIndex != -1 )
			return pStruct->nEnteredGateIndex;
	}
	return -1;
}

bool CDnPartyTask::SyncAIBotGateReady( CDNUserSession* pBotSession, int nGateIndex )
{
	if( !pBotSession || !pBotSession->bIsAIBot() || !GetRoom() )
		return false;
	if( m_bWaitGate && nGateIndex != -1 )
		return true;

	CDNGameRoom::PartyStruct* pStruct = GetPartyData( pBotSession );
	if( !pStruct )
		return false;
	if( pStruct->nEnteredGateIndex == nGateIndex )
		return true;

	if( pBotSession->GetActorHandle() )
	{
		CDnPlayerActor* pBotActor = static_cast<CDnPlayerActor*>( pBotSession->GetActorHandle().GetPointer() );
		if( pBotActor )
			pBotActor->SetStaticPosition( *pBotActor->GetPosition() );
	}

	pStruct->nEnteredGateIndex = nGateIndex;
	int nMemberIdx = pBotSession->GetPartyMemberIndex();
	if( nMemberIdx < 0 || nMemberIdx >= PARTYMAX )
		nMemberIdx = 0;

	if( CheckAndRequestGateIndex( nGateIndex, nMemberIdx ) == false )
	{
		pStruct->nEnteredGateIndex = -1;
		return false;
	}

	g_Log.Log( LogType::_NORMAL, pBotSession->GetWorldSetID(), pBotSession->GetAccountDBID(), pBotSession->GetCharacterDBID(), pBotSession->GetSessionID(),
		L"[AIBot] GateReady Bot=%s MemberIdx=%d Gate=%d WaitGate=%d\n",
		pBotSession->GetCharacterName(), nMemberIdx, nGateIndex, m_bWaitGate ? 1 : 0 );
	return true;
}

void CDnPartyTask::HoldSharingReversionItem()
{
	if (m_RequestGetReversionItemList.empty() == false)
	{
		ReversionItemGetUnit& current = m_RequestGetReversionItemList.front();
		current.time = 0;
		m_bHoldSharingReversionItem = true;
	}
}

void CDnPartyTask::PickOutInvalidReversionItemUnit()
{
	std::deque<ReversionItemGetUnit>::iterator riguIter = m_RequestGetReversionItemList.begin();
	for (; riguIter != m_RequestGetReversionItemList.end();)
	{
		ReversionItemGetUnit& rigu = (*riguIter);
		if (!rigu.hDropItem)
			riguIter = m_RequestGetReversionItemList.erase(riguIter);
		else
			++riguIter;
	}
}

void CDnPartyTask::ReleaseSharingReversionItem()
{
	if (m_RequestGetReversionItemList.empty() == false && m_bHoldSharingReversionItem)
	{
		if (GetRoom() == NULL)
			return;

		PickOutInvalidReversionItemUnit();

		if (m_RequestGetReversionItemList.empty() == false)
		{
			ReversionItemGetUnit& current = m_RequestGetReversionItemList.front();

			std::vector<ReversionItemGetUserInfo>::iterator iter = current.userInfo.begin();
			for (; iter != current.userInfo.end();)
			{
				ReversionItemGetUserInfo& curUserInfo = *iter;
				CDNUserSession* pGameSession = GetRoom()->GetUserSession(curUserInfo.sessionID);
				if (pGameSession)
				{
					if (curUserInfo.bJoinDecided == false)
						pGameSession->SendRequestJoinGetReversionItem(current.itemInfo, current.hDropItem->GetUniqueID());
				}
				else
				{
					iter = current.userInfo.erase(iter);
					continue;
				}

				++iter;
			}
		}

		m_bHoldSharingReversionItem = false;
	}
}

int CDnPartyTask::OnRecvPartyMemberAbsent(CDNUserSession * pSession, CSMemberAbsent * pPacket, int nLen)
{
	pSession->SetAbsent(pPacket->cAbsentFlag == 0 ? true : false);
	UINT i = 0;
	for (; i < GetUserCount() ; ++i)
	{
		CDNUserSession* pGameSession = GetUserData(i);
		if (GetRoom() && pGameSession)
			pGameSession->SendMemberAbsent(pSession->GetSessionID(), pPacket->cAbsentFlag);
	}
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartyInviteMember(CDNUserSession * pSession, CSInvitePartyMember * pPacket, int nLen)
{
	if (sizeof(CSInvitePartyMember) - sizeof(pPacket->wszBuf) + (pPacket->cInviteMemberSize * sizeof(WCHAR)) != nLen)
		return ERROR_INVALIDPACKET;	

	if (pPacket->cInviteMemberSize <= 0 || pPacket->cInviteMemberSize >= NAMELENMAX)
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_NONE;
	}

	if (GetRoom()->IsInviting())
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL_ALREADYINVITED);
		return ERROR_NONE;
	}

	if (GetEnteredGate() != -1)
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_NONE;
	}

	WCHAR wszName[NAMELENMAX];
	memset(wszName, 0, sizeof(wszName));
	_wcscpy(wszName, NAMELENMAX, pPacket->wszBuf, pPacket->cInviteMemberSize);

	if (!__wcsicmp_l(wszName, pSession->GetCharacterName()))
	{
		_DANGER_POINT();
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if (GetRoom()->GetGameType() != REQINFO_TYPE_PARTY)
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}	

	if (CDnWorld::GetInstance(GetRoom()).GetMapType()!= GlobalEnum::eMapTypeEnum::MAP_WORLDMAP)
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
	
	if (pSession->GetIsolate()->IsIsolateItem(wszName))
	{
		pSession->SendIsolateResult(ERROR_ISOLATE_REQUESTFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if( IsPartyLeader(pSession) == false )
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

#if defined( PRE_WORLDCOMBINE_PARTY )
	if( GetRoom()->bIsWorldCombineParty() )
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

#if defined( PRE_PARTY_DB )
	if (GetRoom()->GetUserCount()+GetRoom()->GetBreakIntoUserCount() >= PARTYCOUNTMAX || (int)GetRoom()->GetUserCount()+GetRoom()->GetBreakIntoUserCount() >= GetRoom()->GetPartyMemberMax())
#else
	if (GetRoom()->GetUserCount() >= PARTYCOUNTMAX || (int)GetRoom()->GetUserCount() >= GetRoom()->GetPartyMemberMax())
#endif // #if defined( PRE_PARTY_DB )
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}	

	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	if (pTask == NULL)
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	int nPermitCnt = g_pDataManager->GetMapPermitPartyCount(pTask->GetMapTableID());
	if (nPermitCnt > 0 && nPermitCnt <= (int)GetRoom()->GetUserCountWithoutGM())
	{
		pSession->SendPartyInviteFail(ERROR_PARTY_INVITEFAIL);
		return ERROR_NONE;
	}

	int nClassPassArr[PERMITMAPPASSCLASSMAX];
	int nPermitLevel = 0;
	memset(nClassPassArr, 0, sizeof(nClassPassArr));

	pTask->GetPermitPassClassIDs(nClassPassArr, PERMITMAPPASSCLASSMAX);
	nPermitLevel = pTask->GetPermitMapLevel();

	//After performing all basic checks, send it to the master.
	bool bret = g_pMasterConnectionManager->SendInvitePartyMember(GetRoom(), pSession->GetWorldSetID(), pTask->GetMapTableID(), pSession->GetGameRoom()->GetPartyChannelID(), pSession->GetAccountDBID(), pSession->GetCharacterName(), wszName, nClassPassArr, nPermitLevel);
	if (bret)
	{
		if (GetRoom()->SetInviteCharacterName(wszName) == false)
			_DANGER_POINT();		//This shouldn't happen~
	}	
	return ERROR_NONE;
}

int CDnPartyTask::OnRecvPartySwapMemberIndex(CDNUserSession * pSession, CSPartySwapMemberIndex * pPacket, int nLen)
{
	if (GetRoom()->GetPartyIndex() == 0)
	{	//Lying kids go to hell.
		pSession->SendPartySwapMemberIndex(NULL, ERROR_PARTY_SWAPINDEX_FAIL);
		return ERROR_NONE;
	}

	if (GetRoom()->IsPartyLeader(pSession->GetSessionID()) == false)
	{
		pSession->SendPartySwapMemberIndex(NULL, ERROR_PARTY_SWAPINDEX_FAIL);
		return ERROR_NONE;
	}

	if (GetRoom()->PartySwapMemberIndex(pPacket) == false)
		pSession->SendPartySwapMemberIndex(NULL, ERROR_PARTY_SWAPINDEX_FAIL);
	return ERROR_NONE;
}

#if defined( PRE_PARTY_DB )
int CDnPartyTask::OnRecvPartyModify(CDNUserSession * pSession, CSPartyInfoModify * pPacket, int nLen)
{
	if (GetRoom()->GetPartyIndex() <= 0)
		return ERROR_NONE;

	if (GetRoom()->IsPartyLeader(pSession->GetSessionID()) == false)
		return ERROR_NONE;

	// Check if world zone
	const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pSession->GetMapIndex() );
	if( pMapInfo == NULL )
		return ERROR_NONE;
	if( pMapInfo->MapType != GlobalEnum::MAP_WORLDMAP )
		return ERROR_NONE;

	// On game server, only Party::BitFlag::WorldmapAllow bit among BitFlag options can be changed.
	int iBitFlag = GetRoom()->GetPartyStructData().iBitFlag;
	if( pPacket->iBitFlag&Party::BitFlag::WorldmapAllow && !(GetRoom()->GetPartyStructData().iBitFlag&Party::BitFlag::WorldmapAllow) )
	{
		iBitFlag |= Party::BitFlag::WorldmapAllow;
	}
	else if( !(pPacket->iBitFlag&Party::BitFlag::WorldmapAllow) && GetRoom()->GetPartyStructData().iBitFlag&Party::BitFlag::WorldmapAllow )
	{
		iBitFlag &= ~Party::BitFlag::WorldmapAllow;
	}
	else
	{
		return ERROR_NONE;
	}
	
	pSession->GetDBConnection()->QueryModParty( pSession, GetRoom()->GetPartyStructData(), iBitFlag );
	return ERROR_NONE;
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
int CDnPartyTask::OnRecvPartyAskJoinDecision(CDNUserSession * pSession, CSPartyAskJoinDecision * pPacket, int nLen)
{
	if (GetRoom()->GetPartyIndex() <= 0)
	{
		pSession->SendAskJoinResult(ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL);
		return ERROR_NONE;
	}
	if (GetRoom()->IsPartyLeader(pSession->GetSessionID()) == false)
	{
		pSession->SendAskJoinResult(ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL);
		return ERROR_NONE;
	}

	// Check if world zone
	const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pSession->GetMapIndex() );
	if( pMapInfo == NULL || pMapInfo->MapType != GlobalEnum::MAP_WORLDMAP )
	{
		pSession->SendAskJoinResult(ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL);
		return ERROR_NONE;
	}

	// Release the state first.
	pSession->IsWindowStateNoneSet(WINDOW_ISACCEPT);
	
	if(pPacket->cRet == 0)
		g_pMasterConnectionManager->SendResPartyAskJoin( pSession->GetWorldSetID(), ERROR_PARTY_ASKJOIN_RESULT_PARTYLEADER_DENIED, 0, pPacket->wszPartyAsker );
	else
		g_pMasterConnectionManager->SendAskJoinAgreeInfo( pSession->GetWorldSetID(), pPacket->wszPartyAsker, GetRoom()->GetPartyStructData().PartyID, GetRoom()->GetPartyStructData().iPassword );	

	return ERROR_NONE;
}
#endif	//#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)

void CDnPartyTask::AddPartyDB( CDNUserSession* pBreakIntoSession )
{		
	if( pBreakIntoSession || GetRoom()->GetGameType() != REQINFO_TYPE_PARTY )
		return;

#if defined( PRE_WORLDCOMBINE_PARTY )
	
	if( GetRoom()->bIsWorldCombineParty() == true )
	{
		Party::Data PartyData = GetRoom()->GetPartyStructData();
		GetRoom()->GetDBConnection()->QueryAddWorldParty( g_Config.nManagedID, Party::LocationType::Worldmap, PartyData.iLocationID, &PartyData );
		return;
	}
#endif

	CDNUserSession * pSession = NULL;
	for(int i=0;i<(int)GetUserCount();i++)
	{
		if( GetUserData(i) != NULL )
		{
			CDNGameRoom::PartyStruct *pStruct = GetPartyData( GetUserData(i) );
			if(pStruct->bLeader)
			{
				pSession = GetUserData(i);
				break;
			}
		}
	}	

	if(pSession)
	{	
		Party::AddPartyAndMemberGame PartyData;
		memset(&PartyData, 0, sizeof(PartyData));

		PartyData.PartyData = GetRoom()->GetPartyStructData();
		
		GetRoom()->CopyPartyKickedMemberList( PartyData.nKickedMemberList );

		int nCount = 0;
		for(int i=0;i<(int)GetUserCount();i++)
		{
			if( GetUserData(i) != NULL )
			{
				CDNGameRoom::PartyStruct *pStruct = GetPartyData( GetUserData(i) );
				if( !pStruct->bLeader && pStruct->pSession )
				{					
					PartyData.biCharacterDBID[nCount] = pStruct->pSession->GetCharacterDBID();
					PartyData.nSessionID[nCount] = pStruct->pSession->GetSessionID();
#if defined( PRE_ADD_NEWCOMEBACK )
					if( pStruct->pSession->GetComebackAppellation() > 0 )
						PartyData.bCheckComebackAppellation[nCount] = true;
#endif
					nCount++;
				}
			}
		}

		int iMapIndex = 0;
		if( GetRoom()->m_bDirectConnect )
		{
			iMapIndex = GetRoom()->m_iMapIdx;
		}
		else
		{
			char szLabel[MAX_PATH];
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
			sprintf_s( szLabel, "_Gate%d_MapIndex_txt", GetRoom()->m_iGateIdx );
			char* pszGateMapIndexs = pSox->GetFieldFromLablePtr( GetRoom()->m_iMapIdx, szLabel )->GetString();
			std::vector<string> vGateMapIndexs;
			TokenizeA(pszGateMapIndexs, vGateMapIndexs, ";");
			if (GetRoom()->m_iGateSelect < vGateMapIndexs.size())
				iMapIndex = atoi(vGateMapIndexs[GetRoom()->m_iGateSelect].c_str());
			else
				return;
		}

		GetRoom()->GetDBConnection()->QueryAddPartyAndMemberGame( pSession, g_Config.nManagedID, Party::LocationType::Worldmap, iMapIndex, &PartyData, GetRoom()->GetRoomID() );
	}	
}

void CDnPartyTask::AddPartyMemberDB( CDNUserSession* pSession )
{
	CDNDBConnection* pDBCon = GetRoom()->GetDBConnection();

	if( pDBCon && GetRoom()->GetPartyIndex() > 0 )
	{
		pDBCon->QueryJoinParty( pSession, GetRoom()->GetWorldSetID(), GetRoom()->GetPartyIndex(), GetRoom()->GetPartyMemberMax() );
	}
}

void CDnPartyTask::DelPartyDB()
{
	CDNDBConnection* pDBCon = GetRoom()->GetDBConnection();

	if( pDBCon && GetRoom()->GetPartyIndex() > 0 )
	{
		pDBCon->QueryDelParty( GetRoom()->GetWorldSetID(), GetRoom()->GetPartyIndex() );		
	}
}

void CDnPartyTask::DelPartyMemberDB( CDNUserSession *pSession )
{
	if( !pSession || GetRoom()->GetPartyIndex() == 0 )
		return;
	if (pSession->GetGameRoom()->GetGameType() != REQINFO_TYPE_SINGLE)
	{
		if( pSession->GetDBConnection() )
			pSession->GetDBConnection()->QueryOutParty( GetRoom()->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), pSession->GetSessionID(), GetRoom()->GetPartyIndex(), Party::QueryOutPartyType::Disconnect );
	}		
}
#endif	// #if defined( PRE_PARTY_DB )

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnPartyTask::ApplyPartyEffectSkillItemData( CSUseItem *pPacket, TItemData *pItemData, UINT nUseSessionID, EffectSkillNameSpace::ShowEffectType::eType eType, bool bSendDB, bool bUseItem )
{
	CDNUserSession * pSession = NULL;
	for(int i=0;i<(int)GetUserCount();i++)
	{		
		if( GetUserData(i) != NULL )
		{			
			pSession = GetUserData(i);
			pSession->GetItem()->ApplyPartyEffectSkillItemData(pPacket, pItemData, nUseSessionID, eType, bSendDB, bUseItem);
		}
	}
}
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
bool CDnPartyTask::CheckPartyMemberAppellation( int nAppellationID )
{
	CDNUserSession * pSession = NULL;
	if( nAppellationID > 0 )
	{
		for(int i=0;i<(int)GetUserCount();i++)
		{		
			if( GetUserData(i) != NULL )
			{			
				pSession = GetUserData(i);
				if(pSession)
				{
					if( pSession->GetComebackAppellation() == nAppellationID && !pSession->GetOutedMember() )
					{
						return true;
					}
				}

			}
		}
	}
	else
	{
		for(int i=0;i<(int)GetUserCount();i++)
		{		
			if( GetUserData(i) != NULL )
			{			
				pSession = GetUserData(i);
				if(pSession)
				{
					if( pSession->GetComebackAppellation() > 0 && !pSession->GetOutedMember() )
					{
						return true;
					}
				}

			}
		}
	}
	return false;
}

void CDnPartyTask::DelPartyMemberAppellation( int nAppellationID )
{
	if( !CheckPartyMemberAppellation( nAppellationID ) )
	{
		TAppellationData *pAData = g_pDataManager->GetAppellationData( nAppellationID );
		if( pAData )
		{
			TItemData* pBuffItemData = g_pDataManager->GetItemData( pAData->nSkillItemID );
			if(pBuffItemData)
			{
				CDNUserSession * pSession = NULL;
				for(int i=0;i<(int)GetUserCount();i++)
				{		
					if( GetUserData(i) != NULL )
					{			
						pSession = GetUserData(i);
						if(pSession && pSession->GetPlayerActor() )
						{
							TEffectSkillData* EffectSkill = pSession->GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pBuffItemData->nSkillID );
							if( EffectSkill )
							{
								CDNUserSession::RemoveEffectSkill(pSession->GetPlayerActor(), EffectSkill);
								pSession->GetItem()->DelEffectSkillItem( pBuffItemData->nSkillID );								
							}	
						}

					}
				}
				if( !CheckPartyMemberAppellation(0) && GetRoom() )
					GetRoom()->SetComebackParty(false);

			}							
		}
	}
}

void CDnPartyTask::ApplyJoinMemberAppellation( CDNUserSession * pSession )
{
	if( !pSession )
		return;
	CDNUserSession *pUserObj = NULL;
	
	float fValue = 0.f;
	for(int i=0;i<(int)GetUserCount();i++)
	{		
		if( GetUserData(i) != NULL )
		{			
			pUserObj = GetUserData(i);
			if( pUserObj->GetComebackAppellation() > 0 )
			{
				TAppellationData *pAData = g_pDataManager->GetAppellationData( pUserObj->GetComebackAppellation() );
				if( pAData )
				{
					if( pAData->nSkillItemID > 0 )
					{
						CSUseItem pPacket;
						memset(&pPacket, 0, sizeof(pPacket));
						pPacket.biInvenSerial = pSession->GetItem()->MakeItemSerial();
						TItemData* pBuffItemData = g_pDataManager->GetItemData( pAData->nSkillItemID );
						if( pBuffItemData && g_pDataManager->IsUseItemAllowMapTypeCheck( pBuffItemData->nItemID, GetRoom()->m_iMapIdx ) )
							pSession->GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pBuffItemData, pSession->GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, false, true);
					}
				}			
			}
		}
	}
}

float CDnPartyTask::GetPlayerDropUpBlowValue()
{
	CDNUserSession * pSession = NULL;
	float fValue = 0.f;
	for(int i=0;i<(int)GetUserCount();i++)
	{		
		if( GetUserData(i) != NULL )
		{			
			pSession = GetUserData(i);
			if( pSession->GetPlayerActor() )
			{
				if ( pSession->GetPlayerActor()->IsAppliedThisStateBlow(STATE_BLOW::BLOW_270))
				{
					DNVector(DnBlowHandle) vlBlows;
					pSession->GetPlayerActor()->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_270, vlBlows);
					{
						int nCount = (int)vlBlows.size();
						for (int j = 0; j < nCount; ++j)
						{
							DnBlowHandle hBlow = vlBlows[j];
							if (hBlow && hBlow->IsEnd() == false)
							{
								fValue += hBlow->GetFloatValue();
							}
						}
					}
					// Return immediately if even one person has drop rate boost
					return fValue;
				}
			}
		}
	}
	return fValue;
}
#endif