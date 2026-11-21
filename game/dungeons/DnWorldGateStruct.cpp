
#include "stdafx.h"

#include "DnWorldGateStruct.h"
#include "DNGameDataManager.h"
#ifdef _GAMESERVER
#include "DnPartyTask.h"
#include "DNUserSession.h"
#include "DnPlayerActor.h"
#include "DNGameServerScriptAPI.h"

EWorldEnum::PermitGateEnum DungeonGateStruct::CanEnterDungeon( CMultiRoom *pRoom, std::vector<char> *cVecLessLevelActorIndex, std::vector<char> *cVecNotEnoughItemActorIndex, std::vector<char> *cVecExceedTryActorIndex , std::vector<char> *cVecNotRideVehicleIndex )
{
	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(pRoom);
	int nRealCount = pGameRoom ? (int)pGameRoom->GetRealUserCountWithoutGM() : (int)CDnPartyTask::GetInstance(pRoom).GetUserCountWithoutGM();
	int nCount = (int)CDnPartyTask::GetInstance(pRoom).GetUserCountWithoutGM();
	int nWholeCount = (int)CDnPartyTask::GetInstance(pRoom).GetUserCount();

	EWorldEnum::PermitGateEnum PermitFlag = EWorldEnum::PermitEnter;

	bool bSoloBotBypass = (nRealCount == 1 && nMaxPartyCount != 1);
	if( nMinPartyCount != -1 && !bSoloBotBypass ) {
		if( nCount < nMinPartyCount ) PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitLessPartyCount );
	}
	if( nMaxPartyCount != -1 ) {
		if( nCount > nMaxPartyCount ) PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitLessPartyCount );
	}

	int nMapIndexForClearCount = nMapIndex;
#if defined (PRE_SHARE_MAP_CLEARCOUNT)
	EWorldEnum::MapSubTypeEnum nMapSubType = MapSubType;

	if( nShareCountMapIndex > 0 )
	{
		nMapIndexForClearCount = nShareCountMapIndex;
		nMapSubType = ShareMapSubType;
	}
#endif

	for( int i=0; i<nWholeCount; i++ ) 
	{
		CDNGameRoom::PartyStruct* pPartyStruct = CDnPartyTask::GetInstance(pRoom).GetPartyData(i);
		CDNUserSession *pSession = pPartyStruct->pSession;

		if (pSession->bIsGMTrace() || pSession->bIsAIBot()) continue;

		// Attribute used to always be updated.. strangely it doesn't now.
		int nLevel = pSession->GetLevel();
		if( pSession->GetActorHandle() ) nLevel = pSession->GetActorHandle()->GetLevel();

		if( nMinLevel != -1 ) {
			if( nLevel < nMinLevel ) {
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitLessPlayerLevel );
				if( cVecLessLevelActorIndex && std::find( cVecLessLevelActorIndex->begin(), cVecLessLevelActorIndex->end(), (char)i ) == cVecLessLevelActorIndex->end() ) cVecLessLevelActorIndex->push_back( (char)i );
			}
		}
		if( nMaxLevel != -1 ) {
			if( nLevel > nMaxLevel ) {
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitLessPlayerLevel );
				if( cVecLessLevelActorIndex && std::find( cVecLessLevelActorIndex->begin(), cVecLessLevelActorIndex->end(), (char)i ) == cVecLessLevelActorIndex->end() ) cVecLessLevelActorIndex->push_back( (char)i );
			}
		}
		if( nNeedItemID > 0 && nNeedItemCount > 0 ) {
#if defined(PRE_ADD_68838)
			if(g_pDataManager->GetItemMainType(nNeedItemID) == ITEMTYPE_STAGE_COMPLETE_USEITEM)
			{
				//Check if inven + cash inven has required count of entry tickets
				int nUserTicketCount = 0;
				if(g_pDataManager->IsCashItem(nNeedItemID))
					nUserTicketCount = pSession->GetItem()->GetCashItemCountByItemID(nNeedItemID);
				else
					nUserTicketCount = pSession->GetItem()->GetInventoryItemCount(nNeedItemID);

				if(nUserTicketCount < nNeedItemCount)
				{
					PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitNotEnoughItem );
					if( cVecNotEnoughItemActorIndex && std::find( cVecNotEnoughItemActorIndex->begin(), cVecNotEnoughItemActorIndex->end(), (char)i ) == cVecNotEnoughItemActorIndex->end() ) cVecNotEnoughItemActorIndex->push_back( (char)i );
				}
			}
			else if( pSession->GetItem()->GetInventoryItemCount( nNeedItemID ) < nNeedItemCount ) 
			{
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitNotEnoughItem );
				if( cVecNotEnoughItemActorIndex && std::find( cVecNotEnoughItemActorIndex->begin(), cVecNotEnoughItemActorIndex->end(), (char)i ) == cVecNotEnoughItemActorIndex->end() ) cVecNotEnoughItemActorIndex->push_back( (char)i );
			}
#else
			if( pSession->GetItem()->GetInventoryItemCount( nNeedItemID ) < nNeedItemCount ) {
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitNotEnoughItem );
				if( cVecNotEnoughItemActorIndex && std::find( cVecNotEnoughItemActorIndex->begin(), cVecNotEnoughItemActorIndex->end(), (char)i ) == cVecNotEnoughItemActorIndex->end() ) cVecNotEnoughItemActorIndex->push_back( (char)i );
			}
#endif	// #if defined(PRE_ADD_68838)
		}

		if( bNeedVehicle ) {
			bool bExistVehicle = false;
			TVehicle *pEquipVehicle = pSession->GetItem()->GetVehicleEquip();
			if( ( pEquipVehicle && pEquipVehicle->Vehicle[Vehicle::Slot::Body].nItemID > 0 ) || pSession->GetItem()->GetVehicleInventoryCount() > 0 )
				bExistVehicle = true;

			if( !bExistVehicle )
			{
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitNotRideVehicle );
				if( cVecNotRideVehicleIndex && std::find( cVecNotRideVehicleIndex->begin(), cVecNotRideVehicleIndex->end(), (char)i ) == cVecNotRideVehicleIndex->end() ) cVecNotRideVehicleIndex->push_back( (char)i );
			}
		}

		if( this->nNeedClearMissionID > 0 && pPartyStruct->bLeader == true )
		{
			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSION );
			int nTargetArrayIndex = pSox->GetArrayIndex( this->nNeedClearMissionID );
			if( api_user_IsMissionAchieved( static_cast<CDNGameRoom*>(pRoom), pSession->GetSessionID(), nTargetArrayIndex ) != 1 )
			{
				PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitMission );
			}
		}

#if defined (PRE_SHARE_MAP_CLEARCOUNT)
		switch( nMapSubType ) {
#else
		switch( MapSubType ) {
#endif
			case EWorldEnum::MapSubTypeNest:
				{
					int nExpandNestClearCount = pSession->m_nExpandNestClearCount;
					int nPCBangExpandNestClearCount = g_pDataManager->GetPCBangNestClearCount(pSession->GetPCBangGrade(), nMapIndexForClearCount);
					if (!bExpandable){
						nExpandNestClearCount = 0;
						nPCBangExpandNestClearCount = 0;
					}
					if( pSession->GetNestClearTotalCount( nMapIndexForClearCount ) >= (nMaxTryCount + nExpandNestClearCount + nPCBangExpandNestClearCount) )
					{
						PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitExceedTryCount );
						if( cVecExceedTryActorIndex && std::find( cVecExceedTryActorIndex->begin(), cVecExceedTryActorIndex->end(), (char)i ) == cVecExceedTryActorIndex->end() ) cVecExceedTryActorIndex->push_back( (char)i );
					}
				}
				break;

#if defined(PRE_ADD_TSCLEARCOUNTEX)
			case EWorldEnum::MapSubTypeTreasureStage:
				{
					int nExpandTreasureStageClearCount = pSession->m_nExpandTreasureStageClearCount;
					if (!bExpandable){
						nExpandTreasureStageClearCount = 0;
					}
					if( pSession->GetNestClearTotalCount( nMapIndexForClearCount ) >= (nMaxTryCount + nExpandTreasureStageClearCount) ) {
						PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitExceedTryCount );
						if( cVecExceedTryActorIndex && std::find( cVecExceedTryActorIndex->begin(), cVecExceedTryActorIndex->end(), (char)i ) == cVecExceedTryActorIndex->end() ) cVecExceedTryActorIndex->push_back( (char)i );
					}
					break;
				}
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)

			case EWorldEnum::MapSubTypeEvent:
			case EWorldEnum::MapSubTypeChaosField:
			case EWorldEnum::MapSubTypeDarkLair:
#if !defined(PRE_ADD_TSCLEARCOUNTEX)
#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
			case EWorldEnum::MapSubTypeTreasureStage:
#endif
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
			case EWorldEnum::MapSubTypeFellowship:
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
				if( pSession->GetNestClearTotalCount( nMapIndexForClearCount ) >= nMaxTryCount ) {
					PermitFlag = (EWorldEnum::PermitGateEnum)( PermitFlag | EWorldEnum::PermitExceedTryCount );
					if( cVecExceedTryActorIndex && std::find( cVecExceedTryActorIndex->begin(), cVecExceedTryActorIndex->end(), (char)i ) == cVecExceedTryActorIndex->end() ) cVecExceedTryActorIndex->push_back( (char)i );
				}
				break;
		}
	}

#if defined (PRE_WORLDCOMBINE_PARTY)
	if (pGameRoom && pGameRoom->IsCloseTargetMap(nMapIndex))
		PermitFlag = EWorldEnum::PermitClose;
#endif
	return PermitFlag;
}
#endif

GateStruct * CalcMapInfo( int nMapIndex )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
	char szLabel[64];
	std::string szTemp;

	if( pSox->IsExistItem( nMapIndex ) == false ) return NULL;

	GateStruct *pStruct = NULL;
	EWorldEnum::MapTypeEnum MapType = (EWorldEnum::MapTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapType" )->GetInteger();

	switch( MapType ) {
		case EWorldEnum::MapTypeDungeon:
			pStruct = new DungeonGateStruct;
			break;
		default: 
			pStruct = new IBoostPoolGateStruct; 
			break;
	}

	pStruct->nMapIndex = nMapIndex;
	pStruct->MapType = MapType;
	pStruct->MapSubType = (EWorldEnum::MapSubTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapSubType" )->GetInteger();
	pStruct->Environment = (EWorldEnum::LandEnvironment)pSox->GetFieldFromLablePtr( nMapIndex, "_Environment" )->GetInteger();

	for( int j=0; j<10; j++ ) {
		sprintf_s( szLabel, "_ToolName%d", j + 1 );
		szTemp = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szTemp.empty() ) continue;

		pStruct->szVecToolMapName.push_back( szTemp );
	}
	static char *szFatigueStr[] = { "_FatigueEasy", "_DecFatigue", "_FatigueHard", "_FatigueMaster", "_FatigueAbyss" };
	for( int i=0; i<5; i++ ) {
		pStruct->nFatigue[i] = pSox->GetFieldFromLablePtr( nMapIndex, szFatigueStr[i] )->GetInteger();
	}

	pStruct->PermitFlag = EWorldEnum::PermitEnter;
	pStruct->nMaxTryCount = pSox->GetFieldFromLablePtr( nMapIndex, "_MaxClearCount" )->GetInteger();
	pStruct->bExpandable = pSox->GetFieldFromLablePtr( nMapIndex, "_Expandable" )->GetInteger() ? true : false;

	if( MapType == EWorldEnum::MapTypeDungeon ) {
		int nTemp = pSox->GetFieldFromLablePtr( nMapIndex, "_EnterConditionTableID" )->GetInteger();
		if( nTemp > 0 ) {
			DNTableFileFormat *pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
			if( pDungeonSox->IsExistItem( nTemp ) ) {
				DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pStruct;
				pDungeonStruct->nMinLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_LvlMin" )->GetInteger();
				pDungeonStruct->nMaxLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_LvlMax" )->GetInteger();
				pDungeonStruct->nMinPartyCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_PartyOneNumMin" )->GetInteger();
				pDungeonStruct->nMaxPartyCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_PartyOneNumMax" )->GetInteger();
				pDungeonStruct->nMaxUsableCoin = pDungeonSox->GetFieldFromLablePtr( nTemp, "_MaxUsableCoin" )->GetInteger();
				pDungeonStruct->nAbyssMinLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssLvlMin" )->GetInteger();
				pDungeonStruct->nAbyssMaxLevel = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssLvlMax" )->GetInteger();
				pDungeonStruct->nAbyssNeedQuestID = pDungeonSox->GetFieldFromLablePtr( nTemp, "_AbyssQuestID" )->GetInteger();

				static char *szDifficultStr[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare" };
				for( int i=0; i<5; i++ ) {
					sprintf_s( szLabel, "_Recommend%sLevel", szDifficultStr[i] );
					pDungeonStruct->nRecommandedLv[i] = pDungeonSox->GetFieldFromLablePtr( nTemp, szLabel )->GetInteger();
				}

				pDungeonStruct->nNeedItemID = pDungeonSox->GetFieldFromLablePtr( nTemp, "_NeedItemID" )->GetInteger();
				pDungeonStruct->nNeedItemCount = pDungeonSox->GetFieldFromLablePtr( nTemp, "_NeedItemCount" )->GetInteger();
				pDungeonStruct->bNeedVehicle = (pDungeonSox->GetFieldFromLablePtr( nTemp, "_Vehicle" )->GetInteger()) ? true : false;
				pDungeonStruct->nNeedClearMissionID = pDungeonSox->GetFieldFromLablePtr( nTemp, "_NeedMission" )->GetInteger();

#if defined (PRE_SHARE_MAP_CLEARCOUNT)
				//If clear count share option is enabled, reset that info
				pDungeonStruct->nShareCountMapIndex = pDungeonSox->GetFieldFromLablePtr( nTemp, "_ShareCountMap" )->GetInteger();
				if( pDungeonStruct->nShareCountMapIndex > 0 )
				{
					if( pSox->IsExistItem( pDungeonStruct->nShareCountMapIndex ) == false )
						ASSERT(0);	//Resource error! Should we just terminate to make it easy to spot?
					//On resource error, better to block entry than allow clear-count abuse.
					pDungeonStruct->ShareMapSubType = (EWorldEnum::MapSubTypeEnum)pSox->GetFieldFromLablePtr( pDungeonStruct->nShareCountMapIndex, "_MapSubType" )->GetInteger();
					pDungeonStruct->nMaxTryCount = pSox->GetFieldFromLablePtr( pDungeonStruct->nShareCountMapIndex, "_MaxClearCount" )->GetInteger();
					pDungeonStruct->bExpandable = pSox->GetFieldFromLablePtr( pDungeonStruct->nShareCountMapIndex, "_Expandable" )->GetInteger() ? true : false;
				}
#endif // #if defined (PRE_SHARE_MAP_CLEARCOUNT)

			}
		}
	}

	return pStruct;
}