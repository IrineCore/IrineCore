#include "stdafx.h"
#include "DNUserSession.h"
#include "DNEventSystem.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "ReputationSystemRepository.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

#if defined(_GAMESERVER)
#include "DNGameRoom.h"
#include "DnPlayerActor.h"
#include "DnGameTask.h"
#include "DNGameDataManager.h"
#include "DnDLGameTask.h"
#include "MasterRewardSystem.h"
#include "PvPGameMode.h"
#endif //#if defined(_GAMESERVER)

#if defined(_VILLAGESERVER)
#include "DNPartyManager.h"
#endif // #if defined(_VILLAGESERVER)

int api_Mission_None( CDNUserSession *pUser )
{
	return -1;
}

int api_Mission_PlayerLevel( CDNUserSession *pUser )
{
	return pUser->GetLevel();
}

int api_Mission_PartyCount( CDNUserSession *pUser )
{
#if defined(_VILLAGESERVER)
	if( pUser->GetPartyID() <= 0 ) return 1;

	CDNParty *pParty = g_pPartyManager->GetParty( pUser->GetPartyID() );
	if( !pParty ) return 1;

	return pParty->GetMemberCount();
#elif defined(_GAMESERVER)
	CDNGameRoom *pRoom = pUser->GetGameRoom();
	return pRoom->GetUserCount();
#endif
}

int api_Mission_MapID( CDNUserSession *pUser )
{
#if defined(_VILLAGESERVER)
	return pUser->GetMapIndex();
#elif defined(_GAMESERVER)
	CDNGameRoom *pRoom = pUser->GetGameRoom();
	return pRoom->GetGameTask()->GetMapTableID();
#endif
}

int api_Mission_MapLevel( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom *pRoom = pUser->GetGameRoom();
	return pRoom->GetGameTask()->GetStageDifficulty();
#else
	// Village is always 0
	return 0;
#endif
}

int api_Mission_InvenCoin( CDNUserSession *pUser )
{
	return(int)pUser->GetCoin();
}

int api_Mission_WarehouseCoin( CDNUserSession *pUser )
{
	return(int)pUser->GetWarehouseCoin();
}

int api_Mission_JobID( CDNUserSession *pUser )
{
	return pUser->GetUserJob();
}

int api_Mission_ClassID( CDNUserSession *pUser )
{
	return pUser->GetClassID();
}

// Variable per Event
int api_Mission_MonsterID( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::MonsterID );
#else
	// There is no monster ID in village.
	return 0;
#endif
}

int api_Mission_NpcID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::NpcID );
}

int api_Mission_SkillID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::SkillID );
}

int api_Mission_SkillLevel( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::SkillLevel );
}

int api_Mission_ItemID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::ItemID );
}

int api_Mission_StageClearRank( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::StageClearRank );
}

int api_Mission_StageClearTime( CDNUserSession *pUser )
{

#if defined(_GAMESERVER) 
	return pUser->GetGameRoom()->GetDungeonPlayTime() / 1000;
#else // #if defined(_GAMESERVER) 
	return 0;
#endif // #if defined(_GAMESERVER) 
}

int api_Mission_MaxComboCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();	
	return pPlayer->GetMaxComboCount();
#else
	return 0;
#endif
}

int api_Mission_UseCoinCount( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::UseCoinCount );
}

int api_Mission_EnchantLevel( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::EnchantLevel );
}

int api_Mission_TotalDamageCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();
	return pPlayer->GetAttackedCount();
#else // #if defined(_GAMESERVER)
	return 0;
#endif // #if defined(_GAMESERVER)
}

int api_Mission_MissionID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::MissionID );
}

int api_Mission_DieCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER) 
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();
	return pPlayer->GetDieCount();
#else
	return 0;
#endif
}

int api_Mission_GenocideCount( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::GenocideCount );
}

int api_Mission_PartyMemberDieCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();
	return pPlayer->GetPartyMemberDieCount();
#else // #if defined(_GAMESERVER)
	return 0;
#endif // #if defined(_GAMESERVER)
}

int api_Mission_KOClassIDinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KOClassIDinPvP );
}

int api_Mission_KOClassCountinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KOClassCountinPvP );
}

int api_Mission_KObyClassIDinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KObyClassIDinPvP );
}

int api_Mission_KObyClassCountinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KObyClassCountinPvP );
}

int api_Mission_PvPWin( CDNUserSession *pUser )
{
	return pUser->GetPvPData()->uiWin;
}

int api_Mission_PvPLose( CDNUserSession *pUser )
{
	return pUser->GetPvPData()->uiLose;
}

int api_Mission_AllPartsMinLevel( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::AllPartsMinLevel );
}

int api_Mission_QuestID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::QuestID );
}

int api_Mission_DarkLairRoundCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom *pRoom = pUser->GetGameRoom();
	if( !pRoom ) 
		return 0;

	CDnGameTask *pTask = pRoom->GetGameTask();
	if( !pTask )
		return 0;

	if( pTask->GetGameTaskType() != GameTaskType::DarkLair ) 
		return 0;

	return((CDnDLGameTask*)pTask)->GetRound();
#else // #if defined(_GAMESERVER)
	return 0;
#endif // #if defined(_GAMESERVER)
}

int api_Mission_SocialActionID( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::SocialActionID );
}

int api_Mission_KOCountinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KOCountinPvP );
}

int api_Mission_KObyCountinPvP( CDNUserSession *pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::KObyCountinPvP );
}

int api_Mission_StartPartyCount( CDNUserSession *pUser )
{
#if defined(_GAMESERVER)
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();
	return pPlayer->GetStartPartyCount();
#else // #if defined(_GAMESERVER)
	return 0;
#endif // #if defined(_GAMESERVER)
}

int api_Mission_NpcReputaionFavor( CDNUserSession* pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::NpcReputaionFavor );
}

int api_Mission_NpcReputaionMalice( CDNUserSession* pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::NpcReputaionMalice );
}

int api_Mission_MasterSystem_MasterCount( CDNUserSession* pUser )
{
	return pUser->GetMasterSystemData()->SimpleInfo.iMasterCount;
}

int api_Mission_MasterSystem_PupilCount( CDNUserSession* pUser )
{
	return pUser->GetMasterSystemData()->SimpleInfo.iPupilCount;
}

int api_Mission_MasterSystem_GraduateCount( CDNUserSession* pUser )
{
	return pUser->GetMasterSystemData()->SimpleInfo.iGraduateCount;
}

int api_Mission_MasterSystem_PartyMasterCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	const MasterSystem::CRewardSystem::TCountInfo* pCountInfo = pGameRoom->GetMasterRewardSystem()->GetCountInfo( pUser );
	if( pCountInfo == NULL )
		return 0;

	return pCountInfo->iMasterCount;
#else // #if defined(_GAMESERVER)
	// Not used in Village.
	return 0;
#endif // #if defined(_GAMESERVER)
}

int api_Mission_MasterSystem_PartyPupilCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	const MasterSystem::CRewardSystem::TCountInfo* pCountInfo = pGameRoom->GetMasterRewardSystem()->GetCountInfo( pUser );
	if( pCountInfo == NULL )
		return 0;

	return pCountInfo->iPupilCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_MasterSystem_PartyClassmateCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	const MasterSystem::CRewardSystem::TCountInfo* pCountInfo = pGameRoom->GetMasterRewardSystem()->GetCountInfo( pUser );
	if( pCountInfo == NULL )
		return 0;

	return pCountInfo->iClassmateCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_MasterSystem_PartyMasterAndPupilCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return (api_Mission_MasterSystem_PartyMasterCount(pUser) + api_Mission_MasterSystem_PartyPupilCount(pUser));
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_GuildPartyCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	if (!pUser->GetGuildUID().IsSet())
		return 0;

	int iGuildPartyCount = 0;
	for (DWORD i= 0; i<pGameRoom->GetUserCount(); i++)
	{
		CDNGameRoom::PartyStruct *pStruct = pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;

		if (pUser->GetGuildUID() == pStruct->pSession->GetGuildUID())
			iGuildPartyCount++;
	}

	return iGuildPartyCount;
#else
	return 0;
#endif
}

int api_Mission_PartyType( CDNUserSession* pUser )
{
#if defined(_GAMESERVER) && defined (PRE_WORLDCOMBINE_PARTY)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return -1;

	return pGameRoom->GetPartyType();

#elif defined(_VILLAGESERVER) && defined (PRE_WORLDCOMBINE_PARTY)
	if (pUser->GetPartyID() <= 0) return -1;
	CDNParty* pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
	if (!pParty) return -1;
		
	return pParty->GetPartyType();

#else
	return -1;
#endif
}

int api_Mission_DailyFatigue( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	int nMax = g_pDataManager->GetFatigue(pUser->GetUserJob(), pUser->GetLevel());

	int nSpendFatigue = nMax - pUser->GetFatigue();

	if(nSpendFatigue <=0)
		nSpendFatigue = 0;

	return nSpendFatigue;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_WeeklyFatigue( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	int nMax = g_pDataManager->GetWeeklyFatigue(pUser->GetUserJob(), pUser->GetLevel());

	int nSpendFatigue = nMax - pUser->GetWeeklyFatigue();

	if(nSpendFatigue <=0)
		nSpendFatigue = 0;

	return nSpendFatigue;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PCBangFatigue( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	if(!pUser->IsPCBang())
		return 0;

	int nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, PCBang::Grade::Normal);
	int nSpendFatigue = nMax - pUser->GetPCBangFatigue();

	if(nSpendFatigue <=0)
		nSpendFatigue = 0;

	return nSpendFatigue;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_VIPFatigue( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	int nMax = 0;
#if defined(PRE_ADD_VIP)
	nMax = g_pDataManager->GetVIPFatigue(pUser->GetClassID(), pUser->GetLevel());
#endif //#if defined(PRE_ADD_VIP)
	if(nMax <= 0)
		return 0;

	int nSpendFatigue = nMax - pUser->GetVIPFatigue();

	if(nSpendFatigue <=0)
		nSpendFatigue = 0;

	return nSpendFatigue;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_CountingDungeonClear( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetDungeonClearCount();
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_ItemType( CDNUserSession* pUser )
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::ItemType );
}

int api_Mission_MapSubType( CDNUserSession* pUser )
{
#if defined(PRE_ADD_MISSION_COUPON)
#if defined(_GAMESERVER)
	if( pUser->GetGameRoom() )
		return CDnWorld::GetInstance(pUser->GetGameRoom()).GetMapSubType();
#endif
	return GlobalEnum::MAPSUB_NONE;	//Not used in Village.

#else	// #if defined(PRE_ADD_MISSION_COUPON)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::MapSubType );
#endif	// #if defined(PRE_ADD_MISSION_COUPON)
}

int api_Mission_PCBangGrade( CDNUserSession* pUser )
{
	return pUser->GetPCBangGrade();
}

int api_Mission_PvPGameMode( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return PvPCommon::GameMode::Max;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return PvPCommon::GameMode::Max;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return PvPCommon::GameMode::Max;

	return(int)pPvPGameModeTable->uiGameMode;
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_PvPTeam( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPTeam );
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_PvPRoundCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPRoundCount );
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_PvPWinCondition( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPWinCondition );
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_HolyWaterUse( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::HolyWaterUse );
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_GhoulModeWin( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::GhoulModeWin );
#else
	// Not used in Village.
	return 0;
#endif
}
int api_Mission_BossKillCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDnPlayerActor* pPlayer = pUser->GetPlayerActor();
	return pPlayer->GetKillBossCount();
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PvPModePlayCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return 0;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return 0;

	INT64 biCount = 0;
	switch(pPvPGameModeTable->uiGameMode) // PvPCommon::GameMode
	{
	case PvPCommon::GameMode::PvP_Occupation:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPOccupation_PlayCount, biCount);
		break;
	case PvPCommon::GameMode::PvP_AllKill:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPAllKill_PlayCount, biCount);
		break;
	}

	return(int)biCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PvPModeWinCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return 0;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return 0;

	INT64 biCount = 0;
	switch(pPvPGameModeTable->uiGameMode) // PvPCommon::GameMode
	{
	case PvPCommon::GameMode::PvP_Occupation:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPOccupation_WinCount, biCount);
		break;
	case PvPCommon::GameMode::PvP_AllKill:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPAllKill_WinCount, biCount);
		break;
	}

	return(int)biCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PvPModeOccupyCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return 0;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return 0;

	INT64 biCount = 0;
	switch(pPvPGameModeTable->uiGameMode) // PvPCommon::GameMode
	{
	case PvPCommon::GameMode::PvP_Occupation:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPOccupation_OccupyArea, biCount);
		break;
	}

	return(int)biCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PvPModeStealCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return 0;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return 0;

	INT64 biCount = 0;
	switch(pPvPGameModeTable->uiGameMode) // PvPCommon::GameMode
	{
	case PvPCommon::GameMode::PvP_Occupation:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPOccupation_StealArea, biCount);
		break;
	}

	return(int)biCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_PvPModeKillCount( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if( pGameRoom == NULL )
		return 0;

	CPvPGameMode* pPvPGameMode = pGameRoom->GetPvPGameMode();
	if( pPvPGameMode == NULL )
		return 0;

	const TPvPGameModeTable* pPvPGameModeTable = pPvPGameMode->GetPvPGameModeTable();
	if( pPvPGameModeTable == NULL )
		return 0;

	INT64 biCount = 0;
	switch(pPvPGameModeTable->uiGameMode) // PvPCommon::GameMode
	{
	case PvPCommon::GameMode::PvP_Occupation:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPOccupation_KillCount, biCount);
		break;
	case PvPCommon::GameMode::PvP_AllKill:
		pUser->GetCommonVariableDataValue(CommonVariable::Type::PvPAllKill_KillCount, biCount);
		break;
	}

	return(int)biCount;
#else
	// Not used in Village.
	return 0;
#endif
}

int api_Mission_MonsterGrade( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::MonsterGrade );
#else
	return 0;
#endif 
}
int api_Mission_MonsterRaceID( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::MonsterRaceID );
#else
	return 0;
#endif 
}

int api_Mission_DiffMapToUserLevel( CDNUserSession* pUser )
{
#if defined(_GAMESERVER)
	CDNGameRoom* pGameRoom = pUser->GetGameRoom();
	if(!pGameRoom)
		return CHARLEVELMAX;

	CDnGameTask* pGameTask = pGameRoom->GetGameTask();
	if(!pGameTask)
		return CHARLEVELMAX;

	int nEnterMapTableID = pGameTask->GetEnterMapTableID();
	TDUNGEONDIFFICULTY StageDifficulty = pGameTask->GetStageDifficulty();
	int nRecommandLevel = CHARLEVELMAX;

	DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat *pEnterDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

	if( !pMapSox->IsExistItem( nEnterMapTableID ) ) return CHARLEVELMAX;
	int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( nEnterMapTableID, "_EnterConditionTableID" )->GetInteger();
	if( nDungeonEnterTableID < 1 || !pEnterDungeonSox->IsExistItem( nDungeonEnterTableID ) ) return CHARLEVELMAX;

	switch(StageDifficulty)
	{
#if defined( PRE_PARTY_DB )
		case Dungeon::Difficulty::Easy:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendEasyLevel" )->GetInteger();
			break;

		case Dungeon::Difficulty::Normal:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNormalLevel" )->GetInteger();
			break;

		case Dungeon::Difficulty::Hard:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendHardLevel" )->GetInteger();
			break;

		case Dungeon::Difficulty::Master:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendVeryHardLevel" )->GetInteger();
			break;

		case Dungeon::Difficulty::Abyss:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNightmareLevel" )->GetInteger();
			break;

		case Dungeon::Difficulty::Chaos:
			if (DNTableCell* pCell = pEnterDungeonSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_RecommendChaosLevel"))
				nRecommandLevel = pCell->GetInteger();
			break;

		case Dungeon::Difficulty::Hell:
			if (DNTableCell* pCell = pEnterDungeonSox->GetFieldFromLablePtr(nDungeonEnterTableID, "_RecommendHellLevel"))
				nRecommandLevel = pCell->GetInteger();
			break;
#else
		case 0:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendEasyLevel" )->GetInteger();
			break;

		case 1:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNormalLevel" )->GetInteger();
			break;

		case 2:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendHardLevel" )->GetInteger();
			break;

		case 3:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendVeryHardLevel" )->GetInteger();
			break;

		case 4:
			nRecommandLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNightmareLevel" )->GetInteger();
			break;
#endif // #if defined( PRE_PARTY_DB )
	}

	int nGapLevel = abs( pUser->GetLevel() - nRecommandLevel );

	return nGapLevel;

#else
	return CHARLEVELMAX;
#endif
}
int api_Mission_PetLevel( CDNUserSession* pUser )
{
	return pUser->GetItem()->GetPetLevel();	
	return 0;
}

int api_Mission_PvPRevengeSuccess( CDNUserSession* pUser )
{
#if defined(PRE_ADD_REVENGE)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPRevengeSuccess );
#endif
	return 0;
}

int api_Mission_PvPLevel( CDNUserSession *pUser )
{
#if defined(PRE_ADD_PVPLEVEL_MISSION)
	TPvPGroup* pPvPData = pUser->GetPvPData();
	if( !pPvPData ) return 0;
	return pPvPData->cLevel;
#endif
	return 0;
}

int api_Mission_PvPTournamentWin( CDNUserSession* pUser )
{
#if defined(PRE_ADD_PVP_TOURNAMENT)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPTournamentWin );
#endif
	return 0;
}
int api_Mission_PvPTournamentType( CDNUserSession* pUser )
{
#if defined(PRE_ADD_PVP_TOURNAMENT)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPTournamentType );
#endif
	return 0;
}
int api_Mission_PvPTournamentAdvance( CDNUserSession* pUser )
{
#if defined(PRE_ADD_PVP_TOURNAMENT)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::PvPTournamentAdvance );
#endif
	return 0;
}
int api_Mission_ItemCount(CDNUserSession* pUser)
{
	return pUser->GetEventSystem()->GetEventParam( EventSystem::ItemCount );
}
int api_Mission_SkillPoint(CDNUserSession* pUser)
{
	return pUser->GetSkillPoint();
}
int api_Mission_TotalRebirthCount(CDNUserSession* pUser)
{
	return pUser->GetTotalRebirthCoin();
}
int api_Mission_PlayerHpPercent(CDNUserSession* pUser)
{
#if defined(_GAMESERVER)
	if( pUser->GetActorHandle() )
		return pUser->GetActorHandle()->GetHPPercent();
#endif
	return 0;
}
int api_Mission_PlayerMpPercent(CDNUserSession* pUser)
{
#if defined(_GAMESERVER)
	if( pUser->GetActorHandle() )
		return pUser->GetActorHandle()->GetSPPercent();
#endif
	return 0;
}

int api_Mission_PvPWorldCombine(CDNUserSession* pUser)
{
#if defined(PRE_WORLDCOMBINE_PVP) && (_GAMESERVER)
	CDNGameRoom *pRoom = pUser->GetGameRoom();
	if(pRoom)
		return pRoom->bIsWorldPvPRoom();
#endif
	return 0;
}

int api_Mission_ActiveMissionID(CDNUserSession* pUser)
{
#if defined(PRE_ADD_ACTIVEMISSION) && defined(_GAMESERVER)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::ActiveMissionID );
#endif
	return 0;
}

int api_Mission_UIStringMID(CDNUserSession* pUser)
{
#if defined(PRE_ADD_CHAT_MISSION)
	return pUser->GetEventSystem()->GetEventParam( EventSystem::UIStringMID );
#endif
	return 0;
}