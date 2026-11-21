#include "stdafx.h"
#include "DnDLGameTask.h"
#include "DNGameDataManager.h"
#include "DnWorld.h"
#include "GameSendPacket.h"
#include "DnDLPartyTask.h"
#include "DnDropItem.h"
#include "DNEventSystem.h"
#include "DNDLGameRoom.h"
#include "DNLogConnection.h"
#include "ItemRespawnLogic.h"
#include "EtWorldSector.h"
#include "EtWorldEventControl.h"
#include "DNMasterConnectionManager.h"
#include "MasterRewardSystem.h"
#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
#include "DNDBConnection.h"
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)

CDnDLGameTask::CDnDLGameTask( CMultiRoom *pRoom )
: CDnGameTask( pRoom )
{
	m_nGameTaskType = GameTaskType::DarkLair;
	m_nTotalRound = 0;
	m_nCurrentRound = 0;
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	m_bChallengeDarkLair = false;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	memset( &m_CurrentRankInfo, 0, sizeof(m_CurrentRankInfo) );
	m_pItemRespawnLogic = new CDLItemRespawnLogic(static_cast<CDNGameRoom*>(pRoom));
	m_bCheckDungeonClear = false;
	m_ChallengeResponseEnum = Response_None;
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	m_bBossRound = false;
#endif // PRE_MOD_DARKLAIR_RECONNECT
}

CDnDLGameTask::~CDnDLGameTask()
{
	SAFE_DELETE( m_pItemRespawnLogic );
}


void CDnDLGameTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_pItemRespawnLogic->Process( fDelta );
	CDnGameTask::Process( LocalTime, fDelta );
}

bool CDnDLGameTask::PostInitializeStage( int nRandomSeed )
{
	bool bResult = CDnGameTask::PostInitializeStage( nRandomSeed );
	if( !bResult ) return false;

	return true;
}


void CDnDLGameTask::ResetRound( bool bMakeQueryData )
{
	const TDLMapData *pData = g_pDataManager->GetDLMapData( m_nMapTableID );
	if( pData == NULL ) return;

	m_nCurrentRound = 0;
	m_nTotalRound = pData->nTotalRound;
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	m_bChallengeDarkLair = pData->bChallengeDarkLair;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	// Prepare ranking query
	if( bMakeQueryData ) {
		((CDNDLGameRoom*)GetRoom())->MakeRankQueryData();

		m_CurrentRankInfo.nRank = -1;
		m_CurrentRankInfo.nPlayRound = 1;
		m_CurrentRankInfo.nPlaySec = 0;
		if( GetRoom()->GetStartMemberCount() == 1 && wcslen( GetRoom()->GetPartyName() ) < 1 && GetRoom()->GetUserData(0) ) {
			WCHAR wszTempName[256] = {0, };
			swprintf_s( wszTempName,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3416 ), GetRoom()->GetUserData(0)->GetStatusData()->wszCharacterName );
			_wcscpy(m_CurrentRankInfo.wszPartyName, PARTYNAMELENMAX, wszTempName, (int)wcslen(wszTempName));
		}
		else 
		{
			// Replaced with wcscpy because %s %d in party name causes invalid param error.
			_wcscpy( m_CurrentRankInfo.wszPartyName, _countof(m_CurrentRankInfo.wszPartyName), GetRoom()->GetPartyName(), (int)wcslen(GetRoom()->GetPartyName()) );
			//swprintf_s( m_CurrentRankInfo.wszPartyName, GetRoom()->GetPartyName() );
		}

		m_CurrentRankInfo.cPartyUserCount = (char)GetRoom()->GetUserCount();
		for( int i=0; i<m_CurrentRankInfo.cPartyUserCount; i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			m_CurrentRankInfo.Info[i].nLevel = pSession->GetLevel();
			m_CurrentRankInfo.Info[i].nJobIndex = pSession->GetUserJob();
			swprintf_s( m_CurrentRankInfo.Info[i].wszCharacterName, pSession->GetCharacterName() );
		}
	}
}

void CDnDLGameTask::SetStartRound( int iRound )
{
	m_nCurrentRound = iRound;
	m_nTotalRound += iRound;
}

#ifdef PRE_MOD_DARKLAIR_RECONNECT
void CDnDLGameTask::UpdateRound( int iNextTotalRound, bool bBossRound )
#else // PRE_MOD_DARKLAIR_RECONNECT
void CDnDLGameTask::UpdateRound( int iNextTotalRound )
#endif // PRE_MOD_DARKLAIR_RECONNECT
{
	m_nCurrentRound++;
	m_nTotalRound += iNextTotalRound;
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	m_bBossRound = bBossRound;
#endif // PRE_MOD_DARKLAIR_RECONNECT

	if( m_nCurrentRound > m_nTotalRound 
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		&& false == m_bChallengeDarkLair
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		)
		m_nCurrentRound = m_nTotalRound;
	else {
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			if( !pSession ) continue;
			pSession->GetEventSystem()->OnEvent( EventSystem::OnDarklairClearRound);
		}
	}
}

bool CDnDLGameTask::InitializeStage( int nCurrentMapIndex, int nGateIndex, TDUNGEONDIFFICULTY StageDifficulty, int nRandomSeed, bool bContinueStage, bool bDirectConnect, int nGateSelect )
{
	EWorldEnum::MapTypeEnum PrevMapType = CDnWorld::GetInstance(GetRoom()).GetMapType();

	bool bResult = CDnGameTask::InitializeStage( nCurrentMapIndex, nGateIndex, StageDifficulty, nRandomSeed, bContinueStage, bDirectConnect, nGateSelect );
	if( !bResult ) return false;

	EWorldEnum::MapTypeEnum CurMapType = CDnWorld::GetInstance(GetRoom()).GetMapType();

	switch( CurMapType ) {
		case EWorldEnum::MapTypeDungeon:
			switch( PrevMapType ) {
				case EWorldEnum::MapTypeDungeon: 
					break;
				case EWorldEnum::MapTypeWorldMap:
					ResetRound( true );
					break;
			}
			break;
		case EWorldEnum::MapTypeWorldMap:
		{
			ResetRound( false );
			ClearChallengeResponse();
			break;
		}
	}

	for( DWORD i=0; i<m_pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
	{
		CEtWorldSector*			pSector	 = m_pWorld->GetGrid()->GetActiveSector(i);
		CEtWorldEventControl*	pControl = pSector->GetControlFromUniqueID( ETE_PvPRespawnItemArea );
		if( !pControl ) 
			continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) 
		{
			CEtWorldEventArea* pArea = pControl->GetAreaFromIndex(j);
			if( pArea )
				m_pItemRespawnLogic->AddItemRespawnArea( pArea );
		}
	}
	m_bCheckDungeonClear = false;

	return true;
}

bool CDnDLGameTask::InitializeNextStage( const char *szGridName, int nMapTableID, TDUNGEONDIFFICULTY StageDifficulty, int nStartPositionIndex )
{
	if( m_pItemRespawnLogic )
		m_pItemRespawnLogic->Reset();

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	const TDLMapData *pData = g_pDataManager->GetDLMapData( nMapTableID );
	if( pData )
	{
		m_bChallengeDarkLair = pData->bChallengeDarkLair;
	}
#if defined( PRE_FIX_73930 ) 
	EWorldEnum::MapTypeEnum CurMapType = CDnWorld::GetInstance(GetRoom()).GetMapType();
	if( CurMapType == EWorldEnum::MapTypeWorldMap )
		m_bChallengeDarkLair = false;
#endif	// #if defined( PRE_FIX_73930 )
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	
	return CDnGameTask::InitializeNextStage( szGridName, nMapTableID, StageDifficulty, nStartPositionIndex );
}

void CDnDLGameTask::RequestDungeonClear( bool bClear, DnActorHandle hIgnoreActor, bool bIgnoreRewardItem )
{
	if( CDnWorld::GetInstance(GetRoom()).GetMapType() != EWorldEnum::MapTypeDungeon ) 
		return;

#ifdef PRE_FIX_73312
	if( m_DungeonClearState != DCS_None )
		return;
#endif

#if defined (_FINAL_BUILD)
	if( !m_bEnteredDungeon ) 
	{
		for( DWORD i=0; i<GetUserCount(); i++ ) 
		{
			CDNUserSession *pStruct = GetUserData(i);
			std::wstring szTemp = L"Cannot execute dungeon clear without starting a gate.";
			pStruct->SendChat( CHATTYPE_SYSTEM, (int)szTemp.size() * sizeof(WCHAR), L"", (WCHAR*)szTemp.c_str() );
		}
		return;
	}
#endif

	m_bIgnoreDungeonClearRewardItem = bIgnoreRewardItem;
	GetRoom()->AddDungeonPlayTime( timeGetTime() - m_dwStageCreateTime );
	DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );

	// Fetch each member's values first.
	char cStartingMemberCount = GetRoom()->m_iPartMemberCnt;
	char cCount = 0;
	TDLDungeonClearInfo Info[PARTYMAX];
	int nGuildPoint[PARTYMAX];
	CDNGameRoom::PartyStruct *pPartyStruct[PARTYMAX] = { 0, };

	memset( Info, 0, sizeof(Info) );
	memset( nGuildPoint, 0, sizeof(nGuildPoint) );

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
		if( pStruct == NULL || !pStruct->pSession || pStruct->pSession->bIsGMTrace() || pStruct->pSession->bIsAIBot() ) continue;
		DnActorHandle hActor = pStruct->pSession->GetActorHandle();
		if( !hActor ) continue;
		if( hActor == hIgnoreActor ) continue;

		Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
		pPartyStruct[cCount] = pStruct;

		cCount++;

		((CDNDLGameRoom*)GetRoom())->SetUpdateRankData( pStruct->pSession );
	}

#if defined( PRE_SKIP_REWARDBOX )
	m_nRewardBoxUserCount = cCount;
#endif

	for( char i=0; i<cCount; i++ ) {
		if( !pPartyStruct[i]->pSession || !pPartyStruct[i]->pSession->GetActorHandle() )
			continue;
#if defined( PRE_ADD_DUNGEONCLEARINFO )
		CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pPartyStruct[i]->pSession->GetCharacterDBID() );
		if( !PartyFirst )
		{
			// By design, missing case is a bug
			g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[CDnDLGameTask::RequestDungeonClear] PartyFirst error!!\r\n");
			continue;
		}
#endif
		if( pPartyStruct[i]->pSession->GetActorHandle() == hIgnoreActor ) continue;
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(pPartyStruct[i]->pSession->GetActorHandle().GetPointer());

		int nDungeonClearID = pMapSox->GetFieldFromLablePtr( m_nMapTableID, "_ClearDungeonTableID_Nightmare" )->GetInteger();
		TDLClearDataItem *pClearData = g_pDataManager->GetDLClearData( nDungeonClearID, pPlayer->GetLevel(), m_nCurrentRound );
		if( !pClearData ) continue;

		nGuildPoint[i] = pClearData->nRewardGP;
		Info[i].nExperience = pClearData->nRewardExperience;
		Info[i].nExperience += pPlayer->GetCompleteExperience();

		//Added channel-related logic
		CheckMerit(pPlayer, GlobalEnum::MERIT_BONUS_COMPLETEEXP, Info[i].nExperience, Info[i].nMeritBonusExperience);
		GetRoom()->GetEventExpWhenStageClear(pPartyStruct[i]->pSession, Info[i].nExperience, Info[i].cBonusCount, Info[i].EventClearBonus);

		Info[i].nPromotionExperience = (int)(((float)Info[i].nExperience * (float)((float)(pPartyStruct[i]->pSession->GetPromotionValue(PROMOTIONTYPE_STAGECLEAR))/100)) + 0.5f);
		Info[i].cMaxLevelCharCount = pPartyStruct[i]->pSession->GetMaxLevelCharacterCount();

		int nBonusExperience = 0;
		for( int v=0; v<Info[i].cBonusCount; v++ ) {
			nBonusExperience += Info[i].EventClearBonus[v].nClearEventBonusExperience;
		}

		// Pulled the code from below up to here.		
		if( GetRoom()->GetMasterRewardSystem() )
		{
			float fRate = GetRoom()->GetMasterRewardSystem()->GetExpRewardRate( pPlayer->GetUserSession() );
			if( fRate > 0.f )
			{				
				float fBonusExp = Info[i].nExperience*fRate;
				int iMasterAddExp = GetRoom()->GetMasterRewardSystem()->GetMasterSystemAddExp(pPlayer->GetUserSession(), static_cast<float>(Info[i].nExperience), true);
				Info[i].nExperience += iMasterAddExp; //Just add to gained EXP..				
			}
		}

		if( pPlayer->GetUserSession()->GetPeriodExpItemRate() > 0 )
		{
			int nAddItemExp = static_cast<int>(pClearData->nRewardExperience* (float)(pPlayer->GetUserSession()->GetPeriodExpItemRate()/100.0f));
			Info[i].nExperience += nAddItemExp; //Just add to gained EXP..				
		}

		pPlayer->UpdateMaxLevelGainExperience( Info[i].nExperience + Info[i].nMeritBonusExperience + nBonusExperience + Info[i].nPromotionExperience );
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if( pPlayer->GetLevel() >= nLevelLimit ) {
			Info[i].cClearRewardType = 1;
			Info[i].nRewardGold = (int)( pPlayer->GetMaxLevelGainExperience() * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MaxLevelExpTransGoldValue ) );
		}


		// Set reference values for reward items
		DNVector(TreasureBoxLevelStruct) VecTreasureList;
		int nTotalTreasureOffset = 0;
		for( int j=0; j<4; j++ ) {
			int nProb = pClearData->cTreasureBoxRatio[j];
			if( nProb <= 0 ) continue;

			nTotalTreasureOffset += nProb;
			TreasureBoxLevelStruct Struct;
			Struct.cTreasureLevel = (char)j;
			Struct.nOffset = nTotalTreasureOffset;
			VecTreasureList.push_back( Struct );
		}

		// Reward item
		int nRewardItemCount = pClearData->nShowBoxCount;
		Info[i].cShowBoxCount = pClearData->nShowBoxCount;
		Info[i].cSelectBoxCount = pClearData->nSelectBoxCount;
		/*
		int nRewardItemCount = 4;
		Info[i].cShowBoxCount = 4;
		Info[i].cSelectBoxCount = 2;
		*/

		if( !VecTreasureList.empty() ) {
			for( int k=0; k<nRewardItemCount; k++ ) {
				char cTreasureBoxType = GetTreasureBoxType( nTotalTreasureOffset, VecTreasureList );
				int nDropItemTableID = pClearData->nRewardItemID[cTreasureBoxType];
				DNVector(CDnItem::DropItemStruct) VecItemResult;
				CDnDropItem::CalcDropItemList( GetRoom(), nDropItemTableID, VecItemResult, false );
				int nResultItemID = 0;
				int nResultItemCount = 0;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				int nResultItemEnchantID = 0;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				for( DWORD j=0; j<VecItemResult.size(); j++ ) {
					if( VecItemResult[j].nItemID == 0 ) continue;
					nResultItemID = VecItemResult[j].nItemID;
					nResultItemCount = VecItemResult[j].nCount;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					nResultItemEnchantID = VecItemResult[j].nEnchantID;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					break;
				}
#if defined( PRE_ADD_DUNGEONCLEARINFO )				
				PartyFirst->ClearInfo.cRewardItemType[k] = cTreasureBoxType;
				PartyFirst->ClearInfo.RewardItem[k].nItemID = nResultItemID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				pPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, PartyFirst->ClearInfo.RewardItem[k], 0, nResultItemEnchantID );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				pPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, PartyFirst->ClearInfo.RewardItem[k] );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				PartyFirst->ClearInfo.RewardItem[k].wCount = nResultItemCount;				
#else
				// Also set in clear info.
				pPartyStruct[i]->ClearInfo.cRewardItemType[k] = cTreasureBoxType;
				pPartyStruct[i]->ClearInfo.RewardItem[k].nItemID = nResultItemID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				pPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, pPartyStruct[i]->ClearInfo.RewardItem[k], 0, nResultItemEnchantID  );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				pPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, pPartyStruct[i]->ClearInfo.RewardItem[k] );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				pPartyStruct[i]->ClearInfo.RewardItem[k].wCount = nResultItemCount;
#endif
			}
		}
#if defined( PRE_ADD_DUNGEONCLEARINFO )
		PartyFirst->ClearInfo.cSelectRewardItem = 0;
		PartyFirst->ClearInfo.cSelectRemainCount = pClearData->nSelectBoxCount;
#else
		pPartyStruct[i]->ClearInfo.cSelectRewardItem = 0;
		pPartyStruct[i]->ClearInfo.cSelectRemainCount = pClearData->nSelectBoxCount;
#endif

		pPlayer->GetUserSession()->GetQuest()->OnStageClear(m_nMapTableID);
	}

#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
	bool CheckAbuseClearTime = false;
	if( CheckDungeonClearAbuseTime(GetRoom()->GetDungeonPlayTime(), m_nMapTableID) )
		CheckAbuseClearTime = true;
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
	// Send dungeon clear window.
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if( GetUserData(i)->GetActorHandle() == hIgnoreActor ) continue;
		SendGameDLDungeonClear( GetUserData(i), m_nCurrentRound, GetRoom()->GetDungeonPlayTime(), bClear, cCount, Info );
#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
		if( CheckAbuseClearTime )
		{
			g_Log.Log(LogType::_HACK, GetUserData(i), L"AbuseCount[StageClearTime] MapID[%d] Time[%d]\r\n", m_nMapTableID, GetRoom()->GetDungeonPlayTime());
			GetUserData(i)->GetDBConnection()->QueryAddAbuseMonitor(GetUserData(i), StageClearCheckTime::AbuseCount, 0);
		}
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
	}
	// After changing the state.
	ChangeDungeonClearState( DCS_ClearResultStay );

	// Dispatch ranking query.
	((CDNDLGameRoom*)GetRoom())->RequestRankQueryData();

	// Grant EXP, items, money, etc.
	for( char i=0; i<cCount; i++ ) {
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( GetRoom(), Info[i].nSessionID );
		if( !hActor ) continue;
		if( hActor == hIgnoreActor ) continue;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());

		int iMasterExpBonus = 0;
		TExpData ExpData;		
		ExpData.set( (float)(Info[i].nExperience + Info[i].nMeritBonusExperience + Info[i].nPromotionExperience + iMasterExpBonus) );
		pPlayer->CmdAddExperience( ExpData, DBDNWorldDef::CharacterExpChangeCode::Dungeon, pPlayer->GetUserSession()->GetPartyID() );

		if( Info[i].cClearRewardType == 1 ) {
			pPlayer->CmdAddCoin( Info[i].nRewardGold, DBDNWorldDef::CoinChangeCode::MaxLevelExperienceTransGold, 0 );
		}

		if (0 < nGuildPoint[i]) {
			pPlayer->GetUserSession()->AddGuildPoint(GUILDPOINTTYPE_STAGE, nGuildPoint[i]);
		}

		CDNUserSession *pSession = pPlayer->GetUserSession();

		if( bClear ) {	
			EWorldEnum::MapSubTypeEnum MapSubType = (EWorldEnum::MapSubTypeEnum)pMapSox->GetFieldFromLablePtr( m_nMapTableID, "_MapSubType" )->GetInteger();
			EWorldEnum::MapSubTypeEnum TempSubType = CDnWorld::GetInstance(GetRoom()).GetMapSubType();

			if (MapSubType != TempSubType)
				g_Log.Log(LogType::_GUILDWAR, L"CDnDLGameTask - OnDungeonClear MapID:%d MapSubType:%d != TempSubType:%d Level:%d \n", m_nMapTableID, MapSubType, TempSubType, GetStageDifficulty() );

			pSession->GetEventSystem()->OnEvent( EventSystem::OnDungeonClear, 1, EventSystem::MapSubType, MapSubType);
			pSession->IncreaseDungeonClearCount();
			pSession->GetEventSystem()->OnEvent( EventSystem::OnCountingDungeonClear );

		}
	}

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		DnActorHandle hActor = GetUserData(i)->GetActorHandle();
		if( hActor && hActor->IsMove() && !hActor->IsDie() ) hActor->CmdStop( "Stand" );
	}

	RequestDungeonClearBase();
}

void CDnDLGameTask::MakeHistoryInfo( TDLRankHistoryPartyInfo *pInfo, SDarkLairHistory *pHistory )
{
	pInfo->cPartyUserCount = pHistory->cPartyUserCount;
	pInfo->nPlaySec = pHistory->uiPlaySec;
	pInfo->nPlayRound = pHistory->unPlayRound;
	_wcscpy( pInfo->wszPartyName, _countof(pInfo->wszPartyName), pHistory->wszPartyName, (int)wcslen(pHistory->wszPartyName) );
	for( int i=0; i<pHistory->cPartyUserCount; i++ ) {
		pInfo->Info[i].nLevel = pHistory->sUserData[i].unLevel;
		pInfo->Info[i].nJobIndex = pHistory->sUserData[i].iJobIndex;
		swprintf_s( pInfo->Info[i].wszCharacterName, pHistory->sUserData[i].wszCharacterName );
	}
}

void CDnDLGameTask::MakeHistoryInfo( TDLRankHistoryPartyInfo *pInfo, SDarkLairBestHistory *pHistory )
{
	MakeHistoryInfo( pInfo, (SDarkLairHistory*)pHistory );
	pInfo->nRank = pHistory->iRank;
}

void CDnDLGameTask::ChangeDungeonClearState( DungeonClearStateEnum State )
{
	m_DungeonClearState = State;
	switch( m_DungeonClearState ) 
	{
		case DCS_RequestDungeonClear:
			m_DungeonClearState = DCS_ClearResultStay;
			m_fDungeonClearDelta = 0.f;
			break;
		case DSC_RequestRankResult:
			{
				m_DungeonClearState = DSC_RankResultStay;
				m_fDungeonClearDelta = 0.f;

				TAUpdateDarkLairResult *pResult = ((CDNDLGameRoom*)GetRoom())->GetRankQueryResultInfo();
				if(!pResult) break;	//If it breaks here, ranking info will not be shown on the result screen and it will proceed to the next step.

				m_CurrentRankInfo.nRank = pResult->iRank;
				m_CurrentRankInfo.nPlaySec = GetRoom()->GetDungeonPlayTime() / 1000;
				m_CurrentRankInfo.nPlayRound = m_nCurrentRound;

				TDLRankHistoryPartyInfo UserTopScore;
				TDLRankHistoryPartyInfo HistoryScore[5];
				memset( &UserTopScore, 0, sizeof(UserTopScore) );
				UserTopScore.nRank = m_CurrentRankInfo.nRank;
				UserTopScore.nPlayRound = m_CurrentRankInfo.nPlayRound; 
				UserTopScore.nPlaySec = m_CurrentRankInfo.nPlaySec;
				UserTopScore.cPartyUserCount = m_CurrentRankInfo.cPartyUserCount;
				memcpy( UserTopScore.Info, m_CurrentRankInfo.Info, sizeof(TDLRankHistoryMemberInfo) * UserTopScore.cPartyUserCount );
				
				_wcscpy( UserTopScore.wszPartyName, _countof(UserTopScore.wszPartyName), m_CurrentRankInfo.wszPartyName, (int)wcslen(m_CurrentRankInfo.wszPartyName) );

				for( int i=0; i<5; i++ ) {
					HistoryScore[i].nRank = i + 1;
					MakeHistoryInfo( &HistoryScore[i], &pResult->sHistoryTop[i] );
				}

				for( DWORD i=0; i<GetUserCount(); i++ ) {
					for( DWORD j=0; j<pResult->cPartyUserCount; j++ ) {
						if( pResult->sBestUserData[j].i64CharacterDBID == GetUserData(i)->GetCharacterDBID() ) {
							MakeHistoryInfo( &UserTopScore, &pResult->sBestUserData[j] );
							break;
						}
					}
					SendGameDLRankResult( GetUserData(i), &m_CurrentRankInfo, &UserTopScore, HistoryScore );
				}

				((CDNDLGameRoom*)GetRoom())->FlushRankQueryResultInfo();
			}
			break;
		case DCS_RequestSelectRewardItem:
			m_DungeonClearState = DCS_SelectRewardItemStay;
			m_fDungeonClearDelta = 0.f;

			for( DWORD i=0; i<GetUserCount(); i++ ) {
				if( GetUserData(i)->bIsAIBot() ) continue;
				SendGameDungeonClearSelectRewardItem( GetUserData(i) );
			}
			break;
		case DCS_RequestRewardItemStay:
			{
				m_DungeonClearState = DCS_RewardItemStay;
				m_fDungeonClearDelta = 0.f;

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				TRewardBoxTypeStruct Info[PARTYMAX];
#else
				TRewardItemStruct Info[PARTYMAX];
#endif

				char cCount = 0;
#if defined( PRE_ADD_DUNGEONCLEARINFO )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( !pStruct || !pStruct->pSession || pStruct->pSession->bIsAIBot() ) continue;
					CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pStruct->pSession->GetCharacterDBID() );
					if( !PartyFirst )
					{
						// By design, missing case is a bug
						g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[CDnDLGameTask::ChangeDungeonClearState] PartyFirst error!!\r\n");
						continue;
					}

					Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
					memcpy( Info[cCount].cRewardItemType, PartyFirst->ClearInfo.cRewardItemType, sizeof(PartyFirst->ClearInfo.cRewardItemType) );
#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )
					for( int j=0; j<4; j++ ) {
						Info[cCount].nRewardItemID[j] = PartyFirst->ClearInfo.RewardItem[j].nItemID;
						Info[cCount].nRewardItemRandomSeed[j] = PartyFirst->ClearInfo.RewardItem[j].nRandomSeed;
						Info[cCount].cRewardItemOption[j] = PartyFirst->ClearInfo.RewardItem[j].cOption;
						Info[cCount].wRewardItemCount[j] = PartyFirst->ClearInfo.RewardItem[j].wCount;
						Info[cCount].cRewardItemSoulBound[j] = PartyFirst->ClearInfo.RewardItem[j].bSoulbound;
					}
#endif
					// Modify so the server notifies users who have not selected everything yet.
					if( PartyFirst->ClearInfo.cSelectRemainCount > 0 ) {
						for( int j=0; j<4; j++ ) {
							if( !PartyFirst->ClearInfo.IsFlag(j) ) {
								PartyFirst->ClearInfo.SetFlag( j, true );
								PartyFirst->ClearInfo.cSelectRemainCount--;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
								CheckSelectNamedItem( pStruct->pSession, PartyFirst->ClearInfo.RewardItem[j], j );
#endif
								if( PartyFirst->ClearInfo.cSelectRemainCount == 0 ) break;
							}
						}
						// Must also send to others if not selected.
						for( DWORD j=0; j<GetUserCount(); j++ ) {
							SendGameSelectRewardItem( GetUserData(j), pStruct->pSession->GetSessionID(), PartyFirst->ClearInfo.cSelectRewardItem );
						}
					}
					cCount++;

				}
#else
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( !pStruct ) continue;
					Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
					memcpy( Info[cCount].cRewardItemType, pStruct->ClearInfo.cRewardItemType, sizeof(pStruct->ClearInfo.cRewardItemType) );
#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )
					for( int j=0; j<4; j++ ) {
						Info[cCount].nRewardItemID[j] = pStruct->ClearInfo.RewardItem[j].nItemID;
						Info[cCount].nRewardItemRandomSeed[j] = pStruct->ClearInfo.RewardItem[j].nRandomSeed;
						Info[cCount].cRewardItemOption[j] = pStruct->ClearInfo.RewardItem[j].cOption;
						Info[cCount].wRewardItemCount[j] = pStruct->ClearInfo.RewardItem[j].wCount;
						Info[cCount].cRewardItemSoulBound[j] = pStruct->ClearInfo.RewardItem[j].bSoulbound;
					}
#endif
					// Modify so the server notifies users who have not selected everything yet.
					if( pStruct->ClearInfo.cSelectRemainCount > 0 ) {
						for( int j=0; j<4; j++ ) {
							if( !pStruct->ClearInfo.IsFlag(j) ) {
								pStruct->ClearInfo.SetFlag( j, true );
								pStruct->ClearInfo.cSelectRemainCount--;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
								CheckSelectNamedItem( pStruct->pSession, pStruct->ClearInfo.RewardItem[j], j );
#endif
								if( pStruct->ClearInfo.cSelectRemainCount == 0 ) break;
							}
						}
						// Must also send to others if not selected.
						for( DWORD j=0; j<GetUserCount(); j++ ) {
							SendGameSelectRewardItem( GetUserData(j), pStruct->pSession->GetSessionID(), pStruct->ClearInfo.cSelectRewardItem );
						}
					}
					cCount++;

				}
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardBoxType( GetUserData(i), cCount, Info );
				}
#else
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardItem( GetUserData(i), cCount, Info );
				}
#endif
			}
			break;
		case DCS_RequestRewardItemResult:
			{
				m_DungeonClearState = DCS_RewardItemResultStay;
				m_fDungeonClearDelta = 0.f;

				TRewardItemResultStruct Info[PARTYMAX];
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				TRewardItemStruct ItemInfo[PARTYMAX];				
#endif
				char cCount = 0;
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( !pStruct ) continue;
#if defined( PRE_ADD_DUNGEONCLEARINFO )
					CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pStruct->pSession->GetCharacterDBID() );
					if( !PartyFirst )
					{
						// By design, missing case is a bug
						g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[CDnDLGameTask::ChangeDungeonClearState] PartyFirst error!!\r\n");
						continue;
					}
					Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
					Info[cCount].cIndex = PartyFirst->ClearInfo.cSelectRewardItem;

					for( int j=0; j<4; j++ ) {						
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
						if( PartyFirst->ClearInfo.RewardItem[j].nItemID > 0 ) 
						{
							TItemData *pItemData = g_pDataManager->GetItemData( PartyFirst->ClearInfo.RewardItem[j].nItemID );
							if(pItemData)
							{
								if(pItemData->cRank == ITEMRANK_SSS )
								{
									if( PartyFirst->ClearInfo.bNamedGiveResult[j] == false )
									{
										SwapNamedItemToNormalItem( pStruct->pSession, PartyFirst->ClearInfo.RewardItem[j] );
									}
								}	
							}							
						}
#endif	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
						if( !PartyFirst->ClearInfo.IsFlag(j) ) continue;
						if( PartyFirst->ClearInfo.RewardItem[j].nItemID > 0 ) {

							pStruct->pSession->GetItem()->CreateInvenWholeItem( PartyFirst->ClearInfo.RewardItem[j], DBDNWorldDef::AddMaterializedItem::DungeonReward, pStruct->pSession->GetPartyID() );
						}
					}
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
					ItemInfo[cCount].nSessionID = pStruct->pSession->GetSessionID();					
					for( int j=0; j<4; j++ ) 
					{
						ItemInfo[cCount].nRewardItemID[j] = PartyFirst->ClearInfo.RewardItem[j].nItemID;
						ItemInfo[cCount].nRewardItemRandomSeed[j] = PartyFirst->ClearInfo.RewardItem[j].nRandomSeed;
						ItemInfo[cCount].cRewardItemOption[j] = PartyFirst->ClearInfo.RewardItem[j].cOption;
						ItemInfo[cCount].wRewardItemCount[j] = PartyFirst->ClearInfo.RewardItem[j].wCount;
						ItemInfo[cCount].cRewardItemSoulBound[j] = PartyFirst->ClearInfo.RewardItem[j].bSoulbound;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
						ItemInfo[cCount].cRewardItemLevel[j] = PartyFirst->ClearInfo.RewardItem[j].cLevel;
						ItemInfo[cCount].cRewardItemPotential[j] = PartyFirst->ClearInfo.RewardItem[j].cPotential;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					}
#endif	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )	
#else	//	#if defined( PRE_ADD_DUNGEONCLEARINFO )
					Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
					Info[cCount].cIndex = pStruct->ClearInfo.cSelectRewardItem;

					for( int j=0; j<4; j++ ) {
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
						if( pStruct->ClearInfo.RewardItem[j].nItemID > 0 ) 
						{
							TItemData *pItemData = g_pDataManager->GetItemData( pStruct->ClearInfo.RewardItem[j].nItemID );
							if (pItemData)
							{
								if (pItemData->cRank == ITEMRANK_SSS)
								{
									if( pStruct->ClearInfo.bNamedGiveResult[j] == false )
									{
										SwapNamedItemToNormalItem( pStruct->pSession, pStruct->ClearInfo.RewardItem[j] );
									}
								}								
							}
						}
#endif	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
						if( !pStruct->ClearInfo.IsFlag(j) ) continue;
						if( pStruct->ClearInfo.RewardItem[j].nItemID > 0 ) {
							pStruct->pSession->GetItem()->CreateInvenWholeItem( pStruct->ClearInfo.RewardItem[j], DBDNWorldDef::AddMaterializedItem::DungeonReward, pStruct->pSession->GetPartyID() );
						}
					}
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
					ItemInfo[cCount].nSessionID = pStruct->pSession->GetSessionID();					
					for( int j=0; j<4; j++ ) {
						ItemInfo[cCount].nRewardItemID[j] = pStruct->ClearInfo.RewardItem[j].nItemID;
						ItemInfo[cCount].nRewardItemRandomSeed[j] = pStruct->ClearInfo.RewardItem[j].nRandomSeed;
						ItemInfo[cCount].cRewardItemOption[j] = pStruct->ClearInfo.RewardItem[j].cOption;
						ItemInfo[cCount].wRewardItemCount[j] = pStruct->ClearInfo.RewardItem[j].wCount;
						ItemInfo[cCount].cRewardItemSoulBound[j] = pStruct->ClearInfo.RewardItem[j].bSoulbound;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
						ItemInfo[cCount].cRewardItemLevel[j] = pStruct->ClearInfo.RewardItem[j].cLevel;
						ItemInfo[cCount].cRewardItemPotential[j] = pStruct->ClearInfo.RewardItem[j].cPotential;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					}
#endif	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
#endif	//	#if defined( PRE_ADD_DUNGEONCLEARINFO )
					cCount++;
				}
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardItem( GetUserData(i), cCount, ItemInfo );
				}
#endif
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardItemResult( GetUserData(i), cCount, Info ); 
				}
			}
			break;
		case DCS_RequestWarpDungeon:
			m_DungeonClearState = DCS_WarpDungeonStay;
			m_fDungeonClearDelta = 0.f;

			SendGameWarpDungeonClearToLeader();
			break;

		case DCS_WarpStandBy:
			{
				m_fDungeonClearDelta = 0.f;

				/*
				if (m_pWorld)
					m_pWorld->OnTriggerEventCallback( "CDnGameTask::ChangeDungeonClearState", m_LocalTime, m_fDelta );
					*/

				for (DWORD i=0; i<GetUserCount(); i++)
				{
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( pStruct->pSession->bIsGMTrace() || pStruct->pSession->bIsAIBot() )
						continue;
					SendGameWarpDungeonClear(pStruct->pSession);
				}

				if (CDnPartyTask::IsActive(GetRoom()))
				{
					CDnPartyTask& partyTask = CDnPartyTask::GetInstance(GetRoom());
					partyTask.ReleaseSharingReversionItem();
				}
			}
			break;

		case DCS_DLRequestChallenge:
		{
			for( DWORD i=0; i<GetUserCount(); i++ ) 
			{
				CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
				if( pStruct->pSession ) 
					SendGameDLChallengeRequest( pStruct->pSession );
			}
			break;
		}
	}
}

bool CDnDLGameTask::ProcessDungeonClearState( LOCAL_TIME LocalTime, float fDelta )
{
	if (m_DungeonClearState == DCS_None) return false;

	if (m_DungeonClearState == DCS_WarpStandBy)
	{
		if (m_bDungeonClearQuestComplete && CDnPartyTask::GetInstance(GetRoom()).IsPartySharingReversionItem() == false)
		{
			if (m_bDungeonClearSendWarpEnable == false)
			{
				CDNUserSession* pLeaderSession = GetPartyLeaderSession();
				if (pLeaderSession)
				{
					SendGameEnableDungeonClearLeaderWarp(pLeaderSession, true);
					m_bDungeonClearSendWarpEnable = true;
				}
			}
		}
		return false;
	}

	m_fDungeonClearDelta += fDelta;
	switch( m_DungeonClearState ) {
		case DCS_ClearResultStay:
			if( m_fDungeonClearDelta >= 4.f && ((CDNDLGameRoom*)GetRoom())->IsRecvRankQueryData() ) {
				ChangeDungeonClearState( DSC_RequestRankResult );
			}
			break;
		case DSC_RankResultStay:
			if( m_fDungeonClearDelta >= 5.f ) 
			{
				if( GetStartFloor() == 0 )
					ChangeDungeonClearState( DCS_RequestSelectRewardItem );
				else
					ChangeDungeonClearState( DCS_WarpStandBy );
			}
			break;
		case DCS_SelectRewardItemStay:
#if defined( PRE_SKIP_REWARDBOX )
			if( m_fDungeonClearDelta >= 5.f || m_nRewardBoxUserCount == 0) {
				ChangeDungeonClearState( DCS_RequestRewardItemStay );
			}
#else
			if( m_fDungeonClearDelta >= 5.f ) {
				ChangeDungeonClearState( DCS_RequestRewardItemStay );
			}
#endif
			break;
		case DCS_RewardItemStay:
			if( m_fDungeonClearDelta >= 5.f ) {
				ChangeDungeonClearState( DCS_RequestRewardItemResult );
			}
			break;
		case DCS_RewardItemResultStay:
			if( m_fDungeonClearDelta >= 5.f ) {
				ChangeDungeonClearState(DCS_WarpStandBy);
			}
			break;
	}
	return true;
}

void CDnDLGameTask::CheckAndRequestDungeonClear( DnActorHandle hActor )
{
	if( m_bCheckDungeonClear ) 
		return;
	
	bool bAllDie = true;
	bool bAllNoUsableRebirthCoin = true;
	bool bAllNoCoin = true;

	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		CDNUserSession *pSession = GetUserData(i);
		if( !pSession ) 
			continue;
		DnActorHandle hLocalActor = pSession->GetActorHandle();
		if( !hLocalActor ) 
			continue;
		if( hLocalActor == hActor ) 
			continue;
		if( !hLocalActor->IsDie() ) 
		{
			bAllDie = false;
			break;
		}
		// Do not break because bAllDie is a required condition here.
		CDNGameRoom::PartyStruct* pStruct = GetRoom()->GetPartyData(pSession);
#if defined( PRE_ADD_REBIRTH_EVENT)
		if( pStruct && (pStruct->nUsableRebirthCoin < 0 || pStruct->nUsableRebirthCoin > 0) )
#else
		if( pStruct && pStruct->nUsableRebirthCoin > 0 )
#endif
		{
			if( pSession->GetTotalRebirthCoin() > 0 )
				bAllNoUsableRebirthCoin = false;
		}

		if( pSession->GetTotalRebirthCoin() > 0 )
			bAllNoCoin = false;
	}

	if( bAllDie == false ) 
		return;

	if( bAllNoCoin == false )
	{
		if( bAllNoUsableRebirthCoin == false )
			return;
	}

	RequestDungeonClear( false, hActor );
	m_bCheckDungeonClear = true;
}

void CDnDLGameTask::OnGhost( DnActorHandle hActor )
{
	CheckAndRequestDungeonClear();
}

int CDnDLGameTask::OnRecvRoomSelectRewardItem( CDNUserSession * pSession, CSSelectRewardItem *pPacket )
{
	if( m_DungeonClearState != DCS_SelectRewardItemStay ) return ERROR_NONE; // If entered in any other case it's a bad actor, but return None for now
#if defined( PRE_ADD_DUNGEONCLEARINFO )
	if(!pSession)
		return ERROR_NONE;
	CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pSession->GetCharacterDBID() );
	if( PartyFirst==NULL || PartyFirst->ClearInfo.cSelectRemainCount == 0)
	{		
		return ERROR_NONE;	
	}
	PartyFirst->ClearInfo.SetFlag( pPacket->cItemIndex, true );
	PartyFirst->ClearInfo.cSelectRemainCount--;
#if defined( PRE_SKIP_REWARDBOX )
	if( PartyFirst->ClearInfo.cSelectRemainCount == 0 )
		m_nRewardBoxUserCount--;
#endif	// #if defined( PRE_SKIP_REWARDBOX )
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	//Check with DB when a named item is selected
	CheckSelectNamedItem( pSession, PartyFirst->ClearInfo.RewardItem[pPacket->cItemIndex], pPacket->cItemIndex );
#endif

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pUserSession = GetUserData(i);
		SendGameSelectRewardItem( pUserSession, pSession->GetSessionID(), PartyFirst->ClearInfo.cSelectRewardItem );
	}
#else
	CDNGameRoom::PartyStruct *pStruct = GetPartyData(pSession);
	if( pStruct==NULL || pStruct->ClearInfo.cSelectRemainCount == 0 ) return ERROR_NONE;

	pStruct->ClearInfo.SetFlag( pPacket->cItemIndex, true );
	pStruct->ClearInfo.cSelectRemainCount--;

#if defined( PRE_SKIP_REWARDBOX )
	if( pStruct->ClearInfo.cSelectRemainCount == 0 )
		m_nRewardBoxUserCount--;
#endif	// #if defined( PRE_SKIP_REWARDBOX )

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	//Check with DB when a named item is selected
	CheckSelectNamedItem( pSession, pStruct->ClearInfo.RewardItem[pPacket->cItemIndex], pPacket->cItemIndex );
#endif

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pUserSession = GetUserData(i);
		SendGameSelectRewardItem( pUserSession, pSession->GetSessionID(), pStruct->ClearInfo.cSelectRewardItem );
	}
#endif

	return ERROR_NONE;
}

int CDnDLGameTask::OnRecvRoomDLChallengeResponse( CDNUserSession* pSession, CSDLChallengeResponse* pPacket )
{
	if( GetDungeonClearState() != DCS_DLRequestChallenge )
		return ERROR_NONE;
	if( GetPartyLeaderSession() != pSession )
		return ERROR_NONE;

	if( pPacket->bAccept )
	{
		static_cast<CDNDLGameRoom*>(GetRoom())->UpdateResultRankMapIndex();
		m_ChallengeResponseEnum = Response_Challenge;
	}
	else
		m_ChallengeResponseEnum = Response_Deny;

	ChangeDungeonClearState( CDnGameTask::DCS_None );

	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		CDNUserSession* pSend = GetUserData(i);
		if( pSend )
			SendGameDLChallengeResponse( pSend, pPacket->bAccept );
	}
	
	return ERROR_NONE;
}

int CDnDLGameTask::GetStartFloor()
{
	TQUpdateDarkLairResult* pRankData = static_cast<CDNDLGameRoom*>(GetRoom())->GetRankQueryData();

	int iMapIndex = pRankData->iMapIndex%DarkLair::TopFloorAbstractMapIndex;
	const TDLMapData* pDLMapData = g_pDataManager->GetDLMapData( iMapIndex );
	if( pDLMapData == NULL )
		return 0;

	return pDLMapData->nFloor;
}

#if defined(PRE_MOD_DARKLAIR_RECONNECT)
bool CDnDLGameTask::OnInitializeBreakIntoActor( CDNUserSession* pSession, const int iVectorIndex )
{
	if( pSession )
		SendGameDLRoundInfo( pSession, m_nCurrentRound, m_nTotalRound, m_bBossRound );
	return CDnGameTask::OnInitializeBreakIntoActor( pSession, iVectorIndex );
}
#endif // PRE_MOD_DARKLAIR_RECONNECT