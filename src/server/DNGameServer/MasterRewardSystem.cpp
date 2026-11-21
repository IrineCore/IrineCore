
#include "stdafx.h"
#include "MasterRewardSystem.h"
#include "DNUserSession.h"
#include "DNGameRoom.h"
#include "DnGameTask.h"
#include "DNGameDataManager.h"
#include "DNDBConnection.h"

using namespace MasterSystem;

CRewardSystem::CRewardSystem( CDNGameRoom* pGameRoom )
:m_pGameRoom(pGameRoom)
{

}

void CRewardSystem::Update( CDNUserSession* pSession, TAGetMasterSystemCountInfo* pPacket )
{
	TCountInfo Info( pPacket->iMasterCount, pPacket->iPupilCount, pPacket->iClassmateCount );

	if( Info.bIsAllZero() )
	{
		std::map<INT64,TCountInfo>::iterator itor = m_mCountRepository.find( pPacket->biCharacterDBID );
		if( itor == m_mCountRepository.end() )
			return;
		m_mCountRepository.erase( itor );
	}
	else
	{
		std::map<INT64,TCountInfo>::iterator itor = m_mCountRepository.find( pPacket->biCharacterDBID );
		if( itor == m_mCountRepository.end() )
		{
			m_mCountRepository.insert( std::make_pair(pPacket->biCharacterDBID,Info) );
		}
		else
		{
			if( (*itor).second != Info )
				(*itor).second = Info;
		}
	}

	if( pPacket->bClientSend  == true )
	{
		if( pSession )
			pSession->SendMasterSystemCountInfo( Info.iMasterCount, Info.iPupilCount, Info.iClassmateCount );
	}
}

void CRewardSystem::GetCountInfo( CDNUserSession* pSession, TCountInfo& CountInfo )
{
	CountInfo.Clear();

	if( pSession == NULL )
		return;

	std::map<INT64,TCountInfo>::iterator itor = m_mCountRepository.find( pSession->GetCharacterDBID() );
	if( itor != m_mCountRepository.end() )
		CountInfo = (*itor).second;
}

void CRewardSystem::RequestRefresh()
{
	for( UINT i=0 ;i<m_pGameRoom->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = m_pGameRoom->GetUserData(i);
		if( pSession && pSession->GetState() == SESSION_STATE_GAME_PLAY )
			pSession->QueryGetMasterSystemCountInfo( true );
	}
}

const CRewardSystem::TCountInfo* CRewardSystem::GetCountInfo( CDNUserSession* pSession )
{
	if( pSession == NULL )
		return NULL;
	if( m_mCountRepository.empty() )
		return NULL;

	std::map<INT64,TCountInfo>::iterator itor = m_mCountRepository.find( pSession->GetCharacterDBID() );
	if( itor == m_mCountRepository.end() )
		return NULL;

	return &(*itor).second;
}

bool CRewardSystem::bIsDurabilityReward( CDNUserSession* pSession )
{
	const TCountInfo* pCountInfo = GetCountInfo( pSession );
	if( pCountInfo == NULL )
		return false;

	// Durability reward if playing together with pupil.
	if( pCountInfo->iPupilCount > 0 )
		return true;

	return false;
}

bool CRewardSystem::bIsFatigueReward( CDNUserSession* pSession )
{
	// In the current design, durability reward and fatigue reward share the same target, so internally call the fatigue reward check function.
	return bIsDurabilityReward( pSession );
}

float CRewardSystem::GetExpRewardRate( CDNUserSession* pSession )
{
	const TCountInfo* pCountInfo = GetCountInfo( pSession );
	if( pCountInfo == NULL )
		return 0.f;

	// If there is a pupil
	if( pCountInfo->iPupilCount > 0 )
	{
		return MasterSystem::Reward::PupilCountExpBonusRate/100.f;
	}
	// When having a master or classmate
	else if( pCountInfo->iMasterCount > 0 || pCountInfo->iClassmateCount > 0 )
	{
		return ((pCountInfo->iMasterCount*CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_MasterCountExpBonusRate ))
			   +(pCountInfo->iClassmateCount*CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_ClassmateCountExpBonusRate )));
	}

	return 0.f;
}

float CRewardSystem::GetTreasureBoxRewardRate( CDNUserSession* pSession )
{
	const TCountInfo* pCountInfo = GetCountInfo( pSession );
	if( pCountInfo == NULL )
		return 0.f;

	// Treasure chest bonus for master and apprentice
	if( pCountInfo->iPupilCount > 0 || pCountInfo->iMasterCount )
		return CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_TreasureBoxBonusRate );
	return 0.f;
}

void CRewardSystem::RequestDungeonClear( CDNUserSession* pSession )
{
	const TCountInfo* pCountInfo = GetCountInfo( pSession );
	if( pCountInfo == NULL )
		return;

	CDNGameRoom* pGameRoom = pSession->GetGameRoom();
	if( pGameRoom == NULL || pGameRoom->GetGameTask() == NULL )
		return;

	if( pCountInfo->iPupilCount > 0 )
	{
		DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
		DNTableFileFormat *pEnterDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( pMapSox == NULL || pEnterDungeonSox == NULL )
			return;

		if( !pMapSox->IsExistItem( pGameRoom->GetGameTask()->GetEnterMapTableID() ) ) 
			return;

		int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( pGameRoom->GetGameTask()->GetEnterMapTableID(), "_EnterConditionTableID" )->GetInteger();
		if( nDungeonEnterTableID < 1 || !pEnterDungeonSox->IsExistItem( nDungeonEnterTableID ) ) 
			return;

		int nLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();

		//
		TMasterSystemGainTableData* pGainTable = g_pDataManager->GetMasterSystemGainTableData( nLevel, pGameRoom->m_StageDifficulty );
		if( pGainTable == NULL )
			return;
	
		if( pSession->CheckDBConnection() == false )
			return;

		// Apply respect value
		if( pGainTable->iRepectPoint > 0 )
			pSession->GetDBConnection()->QueryModMasterSystemRespectPoint( pSession, pGainTable->iRepectPoint );
			
		// Apply master-apprentice affinity
		if( pGainTable->iFavorPoint > 0 )
		{
			for( int i=0 ; i<pSession->GetMasterSystemData()->SimpleInfo.cCharacterDBIDCount ; ++i )
			{
				// Did that disciple clear the dungeon together (DungeonClear)?
				if( pGameRoom->GetUserSessionByCharDBID( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID) == NULL )
					continue;
				pSession->GetDBConnection()->QueryModMasterSystemFavorPoint( pSession, pSession->GetCharacterDBID(), pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID, pGainTable->iFavorPoint );

				pSession->GetDBConnection()->QueryGetMasterSystemSimpleInfo( pSession->GetDBThreadID(), pSession, true, MasterSystem::EventType::DungeonClear );
				pSession->QueryGetMasterSystemCountInfo( true );
			}
		}
	}
}

int CRewardSystem::GetMasterSystemAddExp( CDNUserSession* pSession, float fExp, bool bStageClear )
{	
	const TCountInfo* pCountInfo = GetCountInfo( pSession );
	if( pCountInfo == NULL )
		return 0;

	TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(pSession->GetLevel());
	if( !pPlayerCommonLevelTableInfo )
		return 0;

	float fReturn = 0;
	// When master
	if( pCountInfo->iPupilCount > 0 )
	{		
		if( bStageClear )
			fReturn = (pPlayerCommonLevelTableInfo->fMasterGainExp*(pSession->GetTotalMasterDecreaseFatigue()+pSession->GetDecreaseFatigue())*pCountInfo->iPupilCount);			
		else
			fReturn = fExp;

		int nAddPercent = 0;
		for(int i=0; i<pSession->GetMasterSystemData()->SimpleInfo.cCharacterDBIDCount; ++i)
		{
			// Did that disciple clear the dungeon together (DungeonClear)?
			if( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID
				&& m_pGameRoom->GetUserSessionByCharDBID( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID) == NULL )
				continue;
			nAddPercent += g_pDataManager->GetMasterSysFeelAddExp(pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].nFavorPoint );								
		}
		fReturn = (fReturn*(nAddPercent/100.f));				
	}
	else if( pCountInfo->iMasterCount > 0 )
	{
		// When pupil		
		int nAddPercent = 0;
		for(int i=0; i<pSession->GetMasterSystemData()->SimpleInfo.cCharacterDBIDCount; ++i)
		{
			// Did that master clear the dungeon together (DungeonClear)?
			if( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID
				&& m_pGameRoom->GetUserSessionByCharDBID( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID) == NULL )
				continue;
			nAddPercent += g_pDataManager->GetMasterSysFeelAddExp(pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].nFavorPoint );							
		}
		fReturn = fExp*(nAddPercent/100.f);		
	}
	return (int)fReturn;
}

void CRewardSystem::UpdateFavor( CDNUserSession* pSession, INT64 biPupilCharacterDBID, int nAddFavor)
{
	for( int i=0 ; i<pSession->GetMasterSystemData()->SimpleInfo.cCharacterDBIDCount ; ++i )
	{
		// Did that disciple clear the dungeon together (DungeonClear)?
		if( pSession->GetMasterSystemData()->SimpleInfo.OppositeInfo[i].CharacterDBID == biPupilCharacterDBID )
		{
			(const_cast<TMasterSystemData*>(pSession->GetMasterSystemData()))->SimpleInfo.OppositeInfo[i].nFavorPoint += nAddFavor;
			break;
		}
	}
}
