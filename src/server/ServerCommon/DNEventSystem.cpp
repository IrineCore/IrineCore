#include "stdafx.h"
#include "DNEventSystem.h"
#include "DNMissionSystem.h"
#include "DNMissionCommonAPI.h"
#include "DNUserSession.h"

CDNEventSystem::EventValueFuncStruct CDNEventSystem::s_EventValueFuncList[EventSystem::EventValueTypeEnum_Amount] = {
	{ EventSystem::None, NULL },
	{ EventSystem::PlayerLevel, &api_Mission_PlayerLevel },
	{ EventSystem::PartyCount, &api_Mission_PartyCount },
	{ EventSystem::MapID, &api_Mission_MapID },
	{ EventSystem::MapLevel, &api_Mission_MapLevel },
	{ EventSystem::InvenCoin, &api_Mission_InvenCoin },
	{ EventSystem::WarehouseCoin, &api_Mission_WarehouseCoin },
	{ EventSystem::JobID, &api_Mission_JobID },
	{ EventSystem::ClassID, &api_Mission_ClassID },

	// Variable per Event
	{ EventSystem::MonsterID, &api_Mission_MonsterID },
	{ EventSystem::NpcID, &api_Mission_NpcID },
	{ EventSystem::SkillID, &api_Mission_SkillID },
	{ EventSystem::SkillLevel, &api_Mission_SkillLevel },
	{ EventSystem::ItemID, &api_Mission_ItemID },
	{ EventSystem::StageClearRank, &api_Mission_StageClearRank },
	{ EventSystem::StageClearTime, &api_Mission_StageClearTime },
	{ EventSystem::MaxComboCount, &api_Mission_MaxComboCount },
	{ EventSystem::UseCoinCount, &api_Mission_UseCoinCount },
	{ EventSystem::EnchantLevel, &api_Mission_EnchantLevel },
	{ EventSystem::TotalDamageCount, &api_Mission_TotalDamageCount },
	{ EventSystem::MissionID, &api_Mission_MissionID },
	{ EventSystem::DieCount, &api_Mission_DieCount },
	{ EventSystem::GenocideCount, &api_Mission_GenocideCount },
	{ EventSystem::PartyMemberDieCount, &api_Mission_PartyMemberDieCount },
	{ EventSystem::KOClassIDinPvP, &api_Mission_KOClassIDinPvP },
	{ EventSystem::KOClassCountinPvP, &api_Mission_KOClassCountinPvP },
	{ EventSystem::KObyClassIDinPvP, &api_Mission_KObyClassIDinPvP },
	{ EventSystem::KObyClassCountinPvP, &api_Mission_KObyClassCountinPvP },
	{ EventSystem::PvPWin, &api_Mission_PvPWin },
	{ EventSystem::PvPLose, &api_Mission_PvPLose },
	{ EventSystem::AllPartsMinLevel, &api_Mission_AllPartsMinLevel },
	{ EventSystem::QuestID, &api_Mission_QuestID },
	{ EventSystem::DarkLairRoundCount, &api_Mission_DarkLairRoundCount },
	{ EventSystem::SocialActionID, &api_Mission_SocialActionID },
	{ EventSystem::KOCountinPvP, &api_Mission_KOCountinPvP },
	{ EventSystem::KObyCountinPvP, &api_Mission_KObyCountinPvP },
	{ EventSystem::StartPartyCount, &api_Mission_StartPartyCount },
	{ EventSystem::NpcReputaionFavor, &api_Mission_NpcReputaionFavor },
	{ EventSystem::NpcReputaionMalice, &api_Mission_NpcReputaionMalice },
	{ EventSystem::MasterCount, &api_Mission_MasterSystem_MasterCount },
	{ EventSystem::PupilCount, &api_Mission_MasterSystem_PupilCount },
	{ EventSystem::GraduateCount, &api_Mission_MasterSystem_GraduateCount },
	{ EventSystem::PartyMasterCount, &api_Mission_MasterSystem_PartyMasterCount },
	{ EventSystem::PartyPupilCount, &api_Mission_MasterSystem_PartyPupilCount },
	{ EventSystem::PartyClassmateCount, &api_Mission_MasterSystem_PartyClassmateCount },
	{ EventSystem::MapSubType, &api_Mission_MapSubType },
	{ EventSystem::DungeonClearCount, &api_Mission_CountingDungeonClear },
	{ EventSystem::DailyFatigue, &api_Mission_DailyFatigue },
	{ EventSystem::WeeklyFatigue, &api_Mission_WeeklyFatigue },
	{ EventSystem::PCBangFatigue, &api_Mission_PCBangFatigue },
	{ EventSystem::VIPFatigue, &api_Mission_VIPFatigue },
	{ EventSystem::ItemType, &api_Mission_ItemType },
	{ EventSystem::PCBangGrade, &api_Mission_PCBangGrade },
	{ EventSystem::PvPGameMode, &api_Mission_PvPGameMode },
	{ EventSystem::PvPTeam, &api_Mission_PvPTeam },
	{ EventSystem::PvPRoundCount, &api_Mission_PvPRoundCount },
	{ EventSystem::PvPWinCondition, &api_Mission_PvPWinCondition },
	{ EventSystem::HolyWaterUse, &api_Mission_HolyWaterUse },	
	{ EventSystem::GhoulModeWin, &api_Mission_GhoulModeWin },	
	{ EventSystem::BossKillCount, &api_Mission_BossKillCount },
	{ EventSystem::PvPModePlayCount, &api_Mission_PvPModePlayCount },	
	{ EventSystem::PvPModeWinCount, &api_Mission_PvPModeWinCount },	
	{ EventSystem::PvPModeOccupyCount, &api_Mission_PvPModeOccupyCount },	
	{ EventSystem::PvPModeStealCount, &api_Mission_PvPModeStealCount },
	{ EventSystem::PvPModeKillCount, &api_Mission_PvPModeKillCount },
	{ EventSystem::MonsterGrade, &api_Mission_MonsterGrade },
	{ EventSystem::MonsterRaceID, &api_Mission_MonsterRaceID },
	{ EventSystem::DiffMapToUserLevel, &api_Mission_DiffMapToUserLevel },
	{ EventSystem::GuildPartyCount, &api_Mission_GuildPartyCount },
	{ EventSystem::PetLevel, &api_Mission_PetLevel },	
	{ EventSystem::PartyMasterAndPupilCount, &api_Mission_MasterSystem_PartyMasterAndPupilCount },
	{ EventSystem::PartyType, &api_Mission_PartyType},	
	{ EventSystem::PvPRevengeSuccess, &api_Mission_PvPRevengeSuccess},
	{ EventSystem::PvPLevel, &api_Mission_PvPLevel },	
	{ EventSystem::PvPTournamentWin, &api_Mission_PvPTournamentWin },	
	{ EventSystem::PvPTournamentType, &api_Mission_PvPTournamentType },	
	{ EventSystem::PvPTournamentAdvance, &api_Mission_PvPTournamentAdvance },	
	{ EventSystem::ItemCount, &api_Mission_ItemCount },
	{ EventSystem::PlayerHpPercent, &api_Mission_PlayerHpPercent },
	{ EventSystem::PlayerMpPercent, &api_Mission_PlayerMpPercent },
	{ EventSystem::TotalRebirthCount, &api_Mission_TotalRebirthCount },
	{ EventSystem::SkillPoint, &api_Mission_SkillPoint },
	{ EventSystem::PvPWorldCombine, &api_Mission_PvPWorldCombine },	
	{ EventSystem::ActiveMissionID, &api_Mission_ActiveMissionID },	
	{ EventSystem::UIStringMID, &api_Mission_UIStringMID },		
};

CDNEventSystem::CDNEventSystem( CDNUserSession *pUser )
{
	m_pUserSession = pUser;
	memset( m_nEventParamValue, 0, sizeof(m_nEventParamValue) );
}

CDNEventSystem::~CDNEventSystem()
{
}

void CDNEventSystem::RegisterEventParam( EventSystem::EventValueTypeEnum Index, int nValue )
{
	m_nEventParamValue[Index] = nValue;
}

int CDNEventSystem::GetEventParam( EventSystem::EventValueTypeEnum Index )
{
	return m_nEventParamValue[Index];
}

bool CDNEventSystem::OnEvent( EventSystem::EventTypeEnum Event, int nParamCount, ... )
{
	memset( m_nEventParamValue, 0, sizeof(m_nEventParamValue) );

	va_list list;

	va_start( list, nParamCount );

	for( int i=0; i<nParamCount * 2; i+=2 ) {
		EventSystem::EventValueTypeEnum ValueType = (EventSystem::EventValueTypeEnum)va_arg( list, int );
		int nValue = va_arg( list, int );
		RegisterEventParam( ValueType, nValue );
	}
	va_end( list );
	return OnEvent( Event );
}

//Must handle recursive OnEvent calls. => m_nEventParamValue gets corrupted
bool CDNEventSystem::OnEvent( EventSystem::EventTypeEnum Event )
{
	m_pUserSession->GetQuest()->OnEvent(Event);	//Call event on remote quest	
	m_pUserSession->GetMissionSystem()->OnEvent(Event);	//OnEvent can be called recursively here.
	memset( m_nEventParamValue, 0, sizeof(m_nEventParamValue) );

	return true;
}

bool CDNEventSystem::CheckEventCondition( const TEventCondition *pEventCondition )
{
	if( !pEventCondition ) return false;

	// If CheckType is false (default), consider obtained when all conditions hold as before.
	if( pEventCondition->bCheckType == false ) {
		for( int i=0; i<5; i++ ) {
			if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc == NULL ) continue;
			switch( pEventCondition->cOperator[i] ) {
				case 0: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) == pEventCondition->nParam[i] ) ) return false; break;
				case 1: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) != pEventCondition->nParam[i] ) ) return false; break;
				case 2: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) > pEventCondition->nParam[i] ) ) return false; break;
				case 3: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) < pEventCondition->nParam[i] ) ) return false; break;
				case 4: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) >= pEventCondition->nParam[i] ) ) return false; break;
				case 5: if( !( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) <= pEventCondition->nParam[i] ) ) return false; break;
			}
		}
		return true;
	}
	else {
		for( int i=0; i<5; i++ ) {
			if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc == NULL ) continue;
			switch( pEventCondition->cOperator[i] ) {
				case 0: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) == pEventCondition->nParam[i] ) return true; break;
				case 1: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) != pEventCondition->nParam[i] ) return true; break;
				case 2: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) > pEventCondition->nParam[i] ) return true; break;
				case 3: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) < pEventCondition->nParam[i] ) return true; break;
				case 4: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) >= pEventCondition->nParam[i] ) return true; break;
				case 5: if( s_EventValueFuncList[pEventCondition->cType[i]].pFunc( m_pUserSession ) <= pEventCondition->nParam[i] ) return true; break;
			}
		}
		return false;
	}
	return false;
}
