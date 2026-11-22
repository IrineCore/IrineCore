#include "stdafx.h"
#include "DnGameTask.h"
#include "DnWorld.h"
#include "EtMatrixEx.h"
#include "DnActor.h"
#include "FrameSync.h"
#include "DnMonsterActor.h"
#include "DnActorClassDefine.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "EtWorldSector.h"
#include "DnProjectile.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnWorldSector.h"
#include "DnDropItem.h"
#include "DnPartyTask.h"
#include "DnPlayerActor.h"
#include "PerfCheck.h"
#include "DNGameDataManager.h"
#include "DnNPCActor.h"
#include "GameSendPacket.h"
#include "DnItemTask.h"
#include "DnWorldActProp.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNLogConnection.h"
#include "DnDungeonHistory.h"
#include "DnStateBlow.h"
#include "DnExtendedCombatBlows.h"
#include "DNDBConnectionManager.h"

#include "DnWorldBrokenProp.h"
#include "DnWorldTrapProp.h"
#include "DNIsolate.h"

#ifndef _FINAL_BUILD
#include "MAAiBase.h"
#endif
#include "DNMissionSystem.h"
#include "DNAppellation.h"
#include "DNGuildSystem.h"
#include "DNMonsterAggroSystem.h"
#include "DNDBConnection.h"
#include "MasterRewardSystem.h"
#include "DnNPCActor.h"

#include "DNChatRoom.h"
#include "DNChatRoomManager.h"
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
#endif
#if defined(PRE_FIX_69108)
#include "DNDLGameRoom.h"
#endif
#ifdef PRE_ADD_DOORS
#include "DNFriend.h"
#endif		//#ifdef PRE_ADD_DOORS
#ifdef IRINE_MOD_PLAYER_BOTS
#include "DnAIBotController.h"
#endif

#define DCS_ClearResultStay_Delay 8.f
#define DCS_SelectRewardItemStay_Delay 5.f
#define DCS_RewardItemStay_Delay 5.f
#define DCS_RewardItemResultStay_Delay 5.f

CDnGameTask::CDnGameTask(CMultiRoom *pRoom)
: CTask((CDNGameRoom*)pRoom)
{
	m_nGameTaskType = GameTaskType::Normal;
	m_pWorld = NULL;
	m_StageDifficulty = Dungeon::Difficulty::Max;
	m_nStartPositionIndex = 1;

	m_nMapTableID = 0;
#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	m_nRootMapTableID = 0;
#endif

	m_bReleaseWorld = true;

	m_bEnteredDungeon = false;
	m_nEnterDungeonTableID = -1;
	m_nEnterMapTableID = -1;
	m_nReturnDungeonClearWorldID = -1;
	m_nReturnDungeonClearStartPositionIndex = -1;

	m_fGameProcessDelayDelta = 0.f;
	m_bGameProcessDelay = false;

	m_bWaitPlayCutScene = false;
	m_bCutSceneAutoFadeIn = true;
	m_bDungeonFailed = false;

	m_fAIDelta = 0.f;
	m_fProjectileDelta = 0.f;
	m_dwStageCreateTime = 0;

	m_DungeonClearState = DCS_None;
	m_fDungeonClearDelta = 0.f;
	m_bIgnoreDungeonClearRewardItem = false;

	m_pMeritInfo = NULL;

	m_bDungeonClearQuestComplete = false;
	m_bDungeonClearSendWarpEnable = false;

	m_vPermitPassClassIds.clear();
	m_nPermitMapLevel = 0;

	memset(&m_sWarpDungeonStruct, 0, sizeof(CDnGameTask::sWarpStruct));

#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	m_nShareCountMapIndex = 0;
#endif

	m_bIsTimeAttack		= false;
	m_fTimeAttackDelta	= 0.f;
	m_fOriginTimeAttackDelta = 0.f;

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	memset(&m_sStageDamageLimitStruct,0,sizeof(StageDamageLimitStruct));
#endif

#if defined( PRE_ADD_HEAL_TABLE_REGULATION )
	memset( &m_sStageHealLimitStruct, 0, sizeof(m_sStageHealLimitStruct) );
#endif

#if defined( PRE_ADD_STAGE_WEIGHT )
	memset(&m_StageWeightData, 0, sizeof(m_StageWeightData));
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )
#if defined( PRE_SKIP_REWARDBOX )
	m_nRewardBoxUserCount = 0;
#endif
	ResetHeroAllyRunState();
}

CDnGameTask::~CDnGameTask()
{
	if( m_bReleaseWorld ) FinalizeStage();
	CDnWeapon::ReleaseClass( GetRoom() );
	CDnParts::DeleteAllObject( GetRoom() );
	CEtObject::DeleteAllObject( GetRoom() );

	CDnBlow::DeleteAllObject( GetRoom() );
	CDnSkill::DeleteAllObject( GetRoom() );
	CDnWorldProp::DeleteAllObject( GetRoom() );
}

bool CDnGameTask::Initialize()
{
	return true;
}

void CDnGameTask::SetSyncComplete( bool bFlag )
{
	CGameServerSession::SetSyncComplete( bFlag );
	if( bFlag == true ) {
		ResetTimer();
		CDnStateBlow::ResumeProcess( GetRoom() );

		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			if( !pSession ) continue;
			if( !pSession->bIsAIBot() )
			{
				pSession->GetEventSystem()->OnEvent( EventSystem::OnDungeonEnter, 1, 
					EventSystem::MapLevel, m_StageDifficulty );
			}
			DnActorHandle hActor = pSession->GetActorHandle();
			if( !hActor ) continue;

			hActor->ResetActor();
		}
		for( DWORD i=0; i<m_hVecMonsterList.size(); i++ ) {
			if( m_hVecMonsterList[i] ) 
				m_hVecMonsterList[i]->ResetActor();
		}

		CDnWorld::GetInstance( GetRoom() ).EnableTriggerEventCallback( true );
		m_dwStageCreateTime = timeGetTime();
	}
}

#include "DNGameServerScriptAPI.h"
void CDnGameTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bDestroyTask ) return;
	if( CDnPartyTask::IsActive( GetRoom() ) && !CDnPartyTask::GetInstance( GetRoom() ).IsSyncComplete() ) {
		CDnWorld::GetInstance( GetRoom() ).EnableTriggerEventCallback( false );
		return;
	}
	
	if (m_sWarpDungeonStruct.bActivated && GetRoom()->GetOutMemberExist() == false)
	{
		_DANGER_POINT_MSG(L"m_sWarpDungeonStruct.bActivated");

		UINT nLeaderSessionID = 0;
		GetRoom()->GetLeaderSessionID(nLeaderSessionID);
		CDNUserSession * pLeaderSession = GetRoom()->GetUserSession(nLeaderSessionID);
		if (pLeaderSession)
		{
			OnRecvRoomWarpDungeon(pLeaderSession, &m_sWarpDungeonStruct.sWarp);
			memset(&m_sWarpDungeonStruct, 0, sizeof(m_sWarpDungeonStruct));
			return;
		}
	}

	ProcessTimeAttack( fDelta );

	if( m_bGameProcessDelay ) {
		m_fGameProcessDelayDelta -= fDelta;
		if( m_fGameProcessDelayDelta <= 0.f ) {
			m_fGameProcessDelayDelta = 0.f;
			m_bGameProcessDelay = false;
		}
		else return;
	}
	if( m_bWaitPlayCutScene ) {
		if( !CheckAndSyncCutScenePlay() ) return;
	}

	if( ProcessDungeonClearState( LocalTime, fDelta ) ) return;

	// Process Class
	// Actor Process
	UINT uiTick = timeGetTime();
	PROFILE_TIME_TEST( CDnActor::ProcessClass( GetRoom(), LocalTime, fDelta ) );
	if( timeGetTime()-uiTick >= 80 )
	{
		// #51580 Added delay log related to CDnActor::ProcessClass
		if( GetRoom()->GetGameServer()->bIsFrameAlert() )
		{
			g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] CDnActor::ProcessClass() Delay RoomID:%d MapIndex=%d Delay=%d ActorCount=%d\n", g_Config.nManagedID, GetRoom()->GetRoomID(), GetRoom()->m_iMapIdx, timeGetTime()-uiTick, CDnActor::GetItemCount( GetRoom() ) );

			// Write user log
			for( UINT i=0 ; i<GetRoom()->GetUserCount() ; ++i )
			{
				CDNUserSession* pSession = GetRoom()->GetUserData(i);
				if( pSession )
				{
					g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] CDnActor::ProcessClass() Delay RoomID:%d MapIndex=%d CharName=%s JobID=%d\n", g_Config.nManagedID, GetRoom()->GetRoomID(), GetRoom()->m_iMapIdx, pSession->GetCharacterName(), (int)pSession->GetUserJob() );
				}
			}

			// User actor was logged above, so only log monster actor

			map<int, int> mapMonsterActor;
			DnActorHandle hActor;
			CDnMonsterActor *pMonster;
			for( DWORD i=0; i<STATIC_INSTANCE(CDnActor::s_pVecProcessList).size(); i++ ) 
			{
				hActor = STATIC_INSTANCE(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
				
				if( !hActor->IsMonsterActor() ) continue;
				pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
				if( !pMonster ) continue;
				if( pMonster->IsDie() ) continue;

				int ActorID = pMonster->GetMonsterClassID();
				if( 0 == mapMonsterActor.count( ActorID ) )
					mapMonsterActor[ ActorID ] = 1;
				else
					mapMonsterActor[ ActorID ] += 1;
			}

			for( map<int, int>::iterator iter = mapMonsterActor.begin(); iter != mapMonsterActor.end(); ++iter )
			{
				g_Log.Log( LogType::_GAMESERVERDELAY, L"[%d] CDnActor::ProcessClass() Delay RoomID:%d Monster ActorID=%d Count=%d\n",
					g_Config.nManagedID, GetRoom()->GetRoomID(), iter->first, iter->second );
			}			
		}
	}

	if( m_FrameSkipProjectile.Update( fDelta ) ) {
		m_fProjectileDelta += fDelta;
		UINT uiTick = timeGetTime();
		PROFILE_TIME_TEST( CDnWeapon::ProcessClass( GetRoom(), LocalTime, m_fProjectileDelta ) );
		if( timeGetTime()-uiTick >= 80 )
		{
			if( GetRoom()->GetGameServer()->bIsFrameAlert() )
			{
				g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] CDnWeapon::ProcessClass() Delay RoomID:%d MapIndex=%d Delay=%d ItemCount=%d\n", g_Config.nManagedID, GetRoom()->GetRoomID(), GetRoom()->m_iMapIdx, timeGetTime()-uiTick, CDnWeapon::GetItemCount( GetRoom() ) );

				// Write user log
				for( UINT i=0 ; i<GetRoom()->GetUserCount() ; ++i )
				{
					CDNUserSession* pSession = GetRoom()->GetUserData(i);
					if( pSession )
					{
						g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] CDnWeapon::ProcessClass() Delay RoomID:%d MapIndex=%d CharName=%s JobID=%d\n", g_Config.nManagedID, GetRoom()->GetRoomID(), GetRoom()->m_iMapIdx, pSession->GetCharacterName(), (int)pSession->GetUserJob() );
					}
				}

				// Also record the weapon ID of the currently active projectile.
				map<int, int> mapProjectileCount;
				for( int i = 0; i < CDnWeapon::GetItemCount( GetRoom() ); ++i )
				{
					DnWeaponHandle hWeapon = CDnWeapon::GetItem( GetRoom(), i );
					if( hWeapon && CDnWeapon::Projectile == hWeapon->GetWeaponType() )
					{
						int iWeaponID = hWeapon->GetClassID();
						if( 0 == mapProjectileCount.count( iWeaponID ) )
							mapProjectileCount[ iWeaponID ] = 1;
						else
							mapProjectileCount[ iWeaponID ] += 1;
					}
				}

				for( map<int, int>::iterator iter = mapProjectileCount.begin(); iter != mapProjectileCount.end(); ++iter )
				{
					g_Log.Log( LogType::_GAMESERVERDELAY, L"[%d] CDnWeapon::ProcessClass() Delay RoomID:%d Projectile WeaponTableID=%d Count=%d\n",
								g_Config.nManagedID, GetRoom()->GetRoomID(), iter->first, iter->second );
				}

				// Remove projectile
				CDnWeapon::ReleaseClass( GetRoom(), CDnWeapon::Projectile );
			}
		}
		m_fProjectileDelta = 0.f;
	}
	else m_fProjectileDelta += fDelta;

	PROFILE_TIME_TEST( CDnDropItem::ProcessClass( GetRoom(), LocalTime, fDelta ) );

	if( m_FrameSkip.Update( fDelta ) ) 
	{
		m_fAIDelta += fDelta;
		PROFILE_TIME_TEST( CDnMonsterActor::ProcessAIClass( GetRoom(), LocalTime, m_fAIDelta ) );
		m_fAIDelta = 0.f;
	}
	else
	{
		m_fAIDelta += fDelta;
	}

	// Process World
	if( m_pWorld ) {
		PROFILE_TIME_TEST( m_pWorld->Process( LocalTime, fDelta ) );
	}

	ProcessHeroAlly( LocalTime, fDelta );
	
	for( DWORD i=0; i<CDnPartyTask::GetInstance( GetRoom() ).GetUserCount(); i++ ) {
		CDnPartyTask::GetInstance( GetRoom() ).GetUserData(i)->FlushPacketQueue();
	}
}

bool CDnGameTask::InitializeStage( const char *szGridName, int nMapTableID, TDUNGEONDIFFICULTY StageDifficulty, int nStartPositionIndex )
{
	ResetHeroAllyAreaState();
	m_FrameSkip.SetFramePerSec(CDNMonsterAggroSystem::AIProcessFrame);
	m_FrameSkipProjectile.SetFramePerSec( 10.f );

	m_pWorld = GetRoom()->GetWorld();
	m_nMapTableID = nMapTableID;

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	CalcStageDamageLimit( nMapTableID );
#endif

#if defined( PRE_ADD_HEAL_TABLE_REGULATION )
	CalcStageHealLimit( nMapTableID );
#endif

	m_StageDifficulty = StageDifficulty;
	m_nStartPositionIndex = nStartPositionIndex;

	// World Initialize
	if( InitializeWorld( szGridName ) == false ) return false;

	// Initialize Actor
	CDnActor::InitializeClass( GetRoom() );
	// Initialize Weapon
	CDnWeapon::InitializeClass( GetRoom() );
	// Initialize Drop Item
	CDnDropItem::InitializeClass( GetRoom() );

	if( m_pWorld->ForceInitialize() == false ) return false;
	return true;
}

bool CDnGameTask::InitializeNextStage( const char *szGridName, int nMapTableID, TDUNGEONDIFFICULTY StageDifficulty, int nStartPositionIndex )
{
	DespawnHeroAlly( false );
	ResetHeroAllyAreaState();

	if( m_pWorld ) m_pWorld->Destroy();

	if( CDnPartyTask::IsActive( GetRoom() ) )
		CDnPartyTask::GetInstance( GetRoom() ).ClearReversionItemList();

	CDnDropItem::ReleaseClass( GetRoom() );
	CDnWeapon::ReleaseClass( GetRoom(), CDnWeapon::Projectile );

#if defined(PRE_FIX_63375)
	bool isPrevUseRegulation = false;
	bool newMapIsWoldMap = false;

	//Check if previous map had correction, and if new map is world map, set character HP/MP to 100%..
	const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( m_nMapTableID );
#if defined(PRE_FIX_73930) && defined(PRE_ADD_CHALLENGE_DARKLAIR)
	if (pMapInfo && ((pMapInfo->cRevisionNum != 0) || (pMapInfo->MapSubType == GlobalEnum::eMapSubTypeEnum::MAPSUB_DARKLAIR)))
#else	// #if defined(PRE_FIX_73930) && defined(PRE_ADD_CHALLENGE_DARKLAIR)
	if (pMapInfo && pMapInfo->cRevisionNum != 0)
#endif	// #if defined(PRE_FIX_73930) && defined(PRE_ADD_CHALLENGE_DARKLAIR)
		isPrevUseRegulation = true;	

	newMapIsWoldMap = g_pDataManager->GetMapType( nMapTableID ) == GlobalEnum::eMapTypeEnum::MAP_WORLDMAP;

	bool isRefreshHPMP = isPrevUseRegulation && newMapIsWoldMap;
#endif // PRE_FIX_63375

	m_nMapTableID = nMapTableID;

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	CalcStageDamageLimit( nMapTableID );
#endif

#if defined( PRE_ADD_HEAL_TABLE_REGULATION )
	CalcStageHealLimit( nMapTableID );
#endif

	m_StageDifficulty = StageDifficulty;
	m_nStartPositionIndex = nStartPositionIndex;

	m_nGateIndex = -1;

	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		GetUserData(i)->VerifyValidMap(m_nMapTableID);
		DnActorHandle hActor = GetUserData(i)->GetActorHandle();
		if( hActor )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
			if(pPlayer && pPlayer->IsVehicleMode())
				pPlayer->UnRideVehicle();

			if( pPlayer )
				pPlayer->OnBeforeDestroyStageMonsters();

			hActor->RemoveOctreeNode();
			hActor->ResetActor();
		}
	}

	SAFE_RELEASE_SPTRVEC( m_hVecMonsterList );
	SAFE_RELEASE_SPTRVEC( m_hVecNpcList );
	SAFE_DELETE_PVEC( m_pVecActorBirthList );

	DNVector(DnActorHandle) hVecList;
	CDnActor::GetActorList( GetRoom(), hVecList );
	if( hVecList.size() != GetUserCount() )
	{
		//g_Log.Log( LogType::_OCTREELOG, L"[%d] RoomID:%d ActorSize Invalid %d:%d \r\n", g_Config.nManagedID, GetRoom()->GetRoomID(), hVecList.size(), GetUserCount() );
		CDnActor::InitializeNextStage( GetRoom() );
	}

	// World Initialize
	if( InitializeWorld( szGridName ) == false ) return false;

	// Initialize Actor
	CDnActor::InitializeClass( GetRoom() );
	CDnStateBlow::StopProcess( GetRoom() );
	// Initialize Drop Item
	CDnDropItem::InitializeClass( GetRoom() );

	if( m_pWorld->ForceInitialize() == false ) return false;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		DnActorHandle hActor = GetUserData(i)->GetActorHandle();
		if( !hActor && GetUserData(i)->bIsAIBot() )
		{
			CDnItemTask::GetInstance(GetRoom()).InitializePlayerItem( GetUserData(i) );
			OnInitializePlayerActor( GetUserData(i), i );
			hActor = GetUserData(i)->GetActorHandle();
		}
		if( !hActor )
			continue;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( GetUserData(i)->bIsAIBot() )
		{
			if( hActor->IsDie() )
			{
				if( pPlayer->IsGhost() )
					pPlayer->ToggleGhostMode( false );
				hActor->SetHP( hActor->GetMaxHP() );
				hActor->SetSP( hActor->GetMaxSP() );
				pPlayer->CmdAction( "Stand" );
				pPlayer->CmdRefreshHPSP( hActor->GetHP(), hActor->GetSP() );
			}
			pPlayer->OnAIBotStageStart();
		}

		SetStartPosition( GetUserData(i), i );
		hActor->InsertOctreeNode();
		if( hActor->IsDie() && !pPlayer->IsGhost() ) {
			pPlayer->ToggleGhostMode( true );
		}

		pPlayer->RideVehicle(GetUserData(i)->GetItem()->GetVehicleEquip());

		const TVehicle* pEquipPet = GetUserData(i)->GetItem()->GetPetEquip();
		if( pEquipPet && pEquipPet->Vehicle[Pet::Slot::Body].nItemID > 0 )
		{
			if( pPlayer->IsCanPetMode() )
			{
				if( pEquipPet->nSkillID1 > 0 )
					pPlayer->AddSkill(pEquipPet->nSkillID1);
				if( pEquipPet->nSkillID2 > 0 )
					pPlayer->AddSkill(pEquipPet->nSkillID2);
			}
			else
			{
				if( pEquipPet->nSkillID1 > 0 )
					pPlayer->RemoveSkill(pEquipPet->nSkillID1);
				if( pEquipPet->nSkillID2 > 0 )
					pPlayer->RemoveSkill(pEquipPet->nSkillID2);
			}
		}

#if defined(PRE_FIX_63375)
		if (isRefreshHPMP == true)
			pPlayer->CmdRefreshHPSP(pPlayer->GetMaxHP(), pPlayer->GetMaxSP());
#endif // PRE_FIX_63375
	}
	return true;
}

bool CDnGameTask::InitializeStage( int nCurrentMapIndex, int nGateIndex, TDUNGEONDIFFICULTY StageDifficulty, int nRandomSeed, bool bContinueStage, bool bDirectConnect, int nGateSelect )
{
	if( CDnPartyTask::IsActive( GetRoom() ) ) {
		CDnPartyTask::GetInstance( GetRoom() ).ResetGateIndex();
		m_bSyncComplete = false;
	}

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );

	char szLabel[64];
	EWorldEnum::MapTypeEnum MapType;
	EWorldEnum::MapSubTypeEnum MapSubType;
	int nMapIndex;
	int nStartPositionIndex;
	int nDungeonEnterTableID;
	int nUsableRevbirthCoinCount = -1;
	UINT nDungeonEnterNeedItem = 0;
	int nDungeonEnterNeedItemCount = 0;
	EWorldEnum::LandEnvironment Environment;
	std::string szGridName;
	int nGridIndex = 0;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	int nShareCountMapIndex = 0;
#endif
#if defined(PRE_ADD_ACTIVEMISSION)
	int nMaxPartyCount = 0;
#endif

	if( bDirectConnect ) { // For direct connect (e.g., dungeon Exit, starting after selecting a dungeon), MapIndex and GateIndex are passed as the destination indices, so just assign them directly.
		nMapIndex = nCurrentMapIndex;
		nStartPositionIndex = nGateIndex;
	}
	else {
		// Originally obtained via InitializeGateInfo..
		// Unpack it and change it to get only what is needed.
		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (GetRoom()->GetGameType() == REQINFO_TYPE_FARM)
		{
			//For Farm, only initial loading exists and MapIndex is passed as the destination
			nMapIndex = nCurrentMapIndex;
			//Ignore gate for now.
		}
		else
		{
			if( nCurrentMapIndex <= 0 ) return false;
			if( nGateIndex < 1 || nGateIndex > DNWORLD_GATE_COUNT ) return false;
			sprintf_s( szLabel, "_Gate%d_MapIndex_txt", nGateIndex );
			char* pszGateMapIndexs = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetString();
			std::vector<string> vGateMapIndexs;
			TokenizeA(pszGateMapIndexs, vGateMapIndexs, ";");
			if (nGateSelect < vGateMapIndexs.size())
				nMapIndex = atoi(vGateMapIndexs[nGateSelect].c_str());
			else
				return false;

			sprintf_s( szLabel, "_Gate%d_StartGate_txt", nGateIndex );
			char* pszGateStartIndexs = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetString();
			std::vector<string> vGateStartIndexs;
			TokenizeA(pszGateStartIndexs, vGateStartIndexs, ";");

			if (nGateSelect < vGateStartIndexs.size())
				nStartPositionIndex = atoi(vGateStartIndexs[nGateSelect].c_str());
			else
				return false;
		}

	}

	if( !pSox->IsExistItem( nMapIndex ) ) return false;

#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	std::vector<std::pair<int,std::string>> vecToolMapInfo;
	vecToolMapInfo.reserve( 10 );
	std::string szToolMapName;
	int nToolMapIndex;

	m_nRootMapTableID = nMapIndex;
	
	for( int i=0; i<10; i++ ) 
	{
		sprintf_s( szLabel, "_ToolName%d", i + 1 );
		szToolMapName = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szToolMapName.empty() ) continue;

		sprintf_s( szLabel, "_ToolNameIndex%d", i + 1 );
		nToolMapIndex =  pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();

		vecToolMapInfo.push_back( std::make_pair( nToolMapIndex , szToolMapName ) );
	}

	// Sync RandomSeed
	_srand( GetRoom(), nRandomSeed );
	OutputDebug( "RandomSeed : %d\n", nRandomSeed );

	if( vecToolMapInfo.empty() ) return false;
	int nArrayIndex = _rand(GetRoom())%(int)vecToolMapInfo.size();
	OutputDebug( "MapArrayIndex : %d\n", nArrayIndex );

	szGridName = vecToolMapInfo[nArrayIndex].second;
	nGridIndex = nArrayIndex;

	if( vecToolMapInfo[nArrayIndex].first > 0 )
	{
		nMapIndex = vecToolMapInfo[nArrayIndex].first;
		nGridIndex = 0;
		GetRoom()->OnSelectRandomMap( m_nRootMapTableID, nMapIndex );
		if( !pSox->IsExistItem( nMapIndex ) ) return false;
	}
#endif
	
	int nWorldID = 0;
	DNVector(std::wstring) vAllUserName;
	vAllUserName.reserve( GetUserCount() );
	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		nWorldID = GetPartyData(i)->pSession->GetWorldSetID();
		vAllUserName.push_back(GetPartyData(i)->pSession->GetCharacterName());
		// Initialize on stage initialization
		GetPartyData(i)->pSession->SetNoneWindowState();
	}

#if defined( PRE_WORLDCOMBINE_PARTY )
	if( g_pMasterConnectionManager )
	{
		if( GetRoom()->bIsWorldCombineParty() )
		{
			for( DWORD i=0; i<GetUserCount(); i++ )
			{
				nWorldID = GetPartyData(i)->pSession->GetWorldSetID();
				g_pMasterConnectionManager->SendWorldUserState(nWorldID, GetPartyData(i)->pSession->GetCharacterName(), nMapIndex);
			}
			
		}
		else
			g_pMasterConnectionManager->SendWorldUserState(nWorldID, vAllUserName, nMapIndex);
	}
#else
	//Update user location on master
	if( g_pMasterConnectionManager )
		g_pMasterConnectionManager->SendWorldUserState(nWorldID, vAllUserName, nMapIndex);
#endif
	// Moved UpdateRoomCountInfo to GameThread.
	//GetRoom()->GetGameServer()->GetServerManager()->UpdateRoomCountInfo(GetRoom()->GetRoomID(), nMapIndex);

#if defined( PRE_ADD_STAGE_WEIGHT )
	static char *szStageWeightStr[] = { "_Weight_Easy", "_Weight_Normal", "_Weight_Hard", "_Weight_VeryHard", "_Weight_Nightmare", "_Weight_Chaos", "_Weight_Hell" };
	int nStageWeightID = 0;
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )

	MapType = (EWorldEnum::MapTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapType" )->GetInteger();
	MapSubType = (EWorldEnum::MapSubTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapSubType" )->GetInteger();
	Environment = (EWorldEnum::LandEnvironment)pSox->GetFieldFromLablePtr( nMapIndex, "_Environment" )->GetInteger();

	nDungeonEnterTableID = pSox->GetFieldFromLablePtr( nMapIndex, "_EnterConditionTableID" )->GetInteger();
	if( nDungeonEnterTableID > 0 ) {
		DNTableFileFormat *pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( pDungeonSOX->IsExistItem( nDungeonEnterTableID ) ) {
			nUsableRevbirthCoinCount = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_MaxUsableCoin" )->GetInteger();
			nDungeonEnterNeedItem = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_NeedItemID" )->GetInteger();
			nDungeonEnterNeedItemCount = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_NeedItemCount" )->GetInteger();		

#if defined(PRE_SHARE_MAP_CLEARCOUNT)
			nShareCountMapIndex = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_ShareCountMap" )->GetInteger();
#endif
#if defined(PRE_ADD_ACTIVEMISSION)
			nMaxPartyCount = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_PartyOneNumMax" )->GetInteger();
#endif
#if defined( PRE_ADD_STAGE_WEIGHT )
			nStageWeightID = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, szStageWeightStr[StageDifficulty] )->GetInteger();			
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )
		}
	}

#if defined( PRE_ADD_STAGE_WEIGHT )
	SetStageWeightData( nStageWeightID );
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )

	int nFatigue = 0;
	static char *szFatigueStr[] = { "_FatigueEasy", "_DecFatigue", "_FatigueHard", "_FatigueMaster", "_FatigueAbyss", "_FatigueChaos", "_FatigueHell" };
	nFatigue = pSox->GetFieldFromLablePtr( nMapIndex, szFatigueStr[StageDifficulty] )->GetInteger();

	int nFatigueUserCount = 0;
	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		if( GetPartyData(i) && GetPartyData(i)->pSession && !GetPartyData(i)->pSession->bIsAIBot() )
			++nFatigueUserCount;
	}
	if( nFatigueUserCount < 1 ) nFatigueUserCount = 1;
 	nFatigue = (int)( nFatigue * CFatigueAdjustmentTable::GetInstance().GetValue( nFatigueUserCount, StageDifficulty ) );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// If the map to enter is a dungeon, save it to the dungeon history.
	// Set usable coins
	EWorldEnum::MapTypeEnum ePrevWorld = CDnWorld::GetInstance(GetRoom()).GetMapType();

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	if( GetRoom()->bIsAlteiaWorldmap() )
	{
		GetRoom()->ResetAlteiaWorldmap();
		ePrevWorld = EWorldEnum::MapTypeWorldMap;
	}
#endif
	// Reset hero ally run state whenever entering a fresh dungeon run, retrying after clear, or leaving a dungeon.
	// Preserve m_bHeroAllyRunRolled & m_eRunHeroAllyType across multi-area transitions (Area 1 -> Area 2 -> Boss).
	if( MapType != EWorldEnum::MapTypeDungeon || ePrevWorld != EWorldEnum::MapTypeDungeon || m_DungeonClearState != DCS_None )
	{
		ResetHeroAllyRunState();
	}

	switch( MapType ) {
		case EWorldEnum::MapTypeDungeon:
			switch( ePrevWorld ) {
				case EWorldEnum::MapTypeDungeon: 

					for( DWORD i=0; i<GetUserCount(); i++ ) 
					{
						if( GetPartyData(i)->pSession->bIsAIBot() ) continue;
						// Deduct previous Map fatigue
						GetPartyData(i)->pSession->DecreaseFatigue();
						// Set current Map fatigue
						GetPartyData(i)->pSession->SetDecreaseFatigue( nFatigue );
					}

					SyncTimeAttack();
					BackupDungeonHistory();
					// Save dungeon play time
					GetRoom()->AddDungeonPlayTime( timeGetTime() - m_dwStageCreateTime );

					// Dungeon -> Dungeon : Inherit buff/debuff state effects as-is.
#if defined(PRE_ADD_ACTIVEMISSION)
					GetRoom()->SetIsFirstInitializeDungeon(false);
#endif
					break;
				case EWorldEnum::MapTypeWorldMap:
				case EWorldEnum::MapTypeVillage:
					// Reset dungeon clear values
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
					ResetDungeonClear( true, m_nMapTableID, m_nGateIndex, nMapIndex, nDungeonEnterTableID, nShareCountMapIndex );
#else
					ResetDungeonClear( true, m_nMapTableID, m_nGateIndex, nMapIndex, nDungeonEnterTableID );
#endif
#if defined(PRE_ADD_WORLD_EVENT)
#else
#if defined(PRE_ADD_REBIRTH_EVENT)
					// Check if resurrection event is active
					if( g_pEvent->IsRebirthEvent( nWorldID, nMapIndex) )
					{
						nUsableRevbirthCoinCount = -1; //Remove resurrection limit
						for( DWORD i=0; i<GetUserCount(); i++ ) 
						{
							GetPartyData(i)->pSession->SendRebirthMaxCoin(nUsableRevbirthCoinCount);
						}
					}
#endif //#if defined(PRE_ADD_REBIRTH_EVENT)
#endif //#if defined(PRE_ADD_WORLD_EVENT)

					// Set usable resurrection coins
					for( DWORD i=0; i<GetUserCount(); i++ ) {
						GetPartyData(i)->nUsableRebirthCoin = nUsableRevbirthCoinCount;
						GetPartyData(i)->ReverseItemList.clear();						
						GetPartyData(i)->nUsableRebirthItemCoin = 0;
						GetPartyData(i)->pSession->SetDecreaseFatigue( nFatigue );	// Set current Map fatigue
						GetPartyData(i)->pSession->SetPeriodExpItemRate();	// Set EXP boost item
#if defined( PRE_FIX_49129 )
						// Save as initial party member when moving from world map to dungeon
						if( !GetPartyData(i)->pSession->bIsAIBot() )
							GetRoom()->AddFirstPartyStruct( GetPartyData(i)->pSession );
#endif
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
						GetPartyData(i)->UseLimitItem.clear();
#endif
					}
#if defined(PRE_ADD_ACTIVEMISSION)
					GetRoom()->SetIsFirstInitializeDungeon(true);
#endif
					if( ePrevWorld == EWorldEnum::MapTypeWorldMap )
					{
						for( DWORD i = 0; i < GetUserCount(); ++i )
						{
							// World zone -> Dungeon: remove debuff skill state effects.
							if( GetPartyData(i)->pSession->GetActorHandle() )
								GetPartyData(i)->pSession->GetActorHandle()->ResetStateEffect( (int)CDnSkill::DurationTypeEnum::Debuff );
							if( GetPartyData(i)->pSession->bIsAIBot() )
								continue;
							// Reset clear info
							GetPartyData(i)->pSession->SetLastStageClearRank( -1 );
							GetPartyData(i)->pSession->ClearTotalMasterDecreaseFatigue();
							// Use required item if present when entering world zone -> dungeon
							if (nDungeonEnterNeedItem > 0 && nDungeonEnterNeedItemCount > 0 && !GetPartyData(i)->pSession->bIsAIBot())
							{
#if defined(PRE_ADD_68838)
								// Handle when entry ticket type is [Delete on stage clear]
								if(g_pDataManager->GetItemMainType(nDungeonEnterNeedItem) == ITEMTYPE_STAGE_COMPLETE_USEITEM)
								{
									//If the item is a cash item
									if(g_pDataManager->IsCashItem(nDungeonEnterNeedItem))
									{
										if( GetPartyData(i)->pSession->GetItem()->GetCashItemCountByItemID(nDungeonEnterNeedItem) < nDungeonEnterNeedItemCount)
										{
											//Leave a log if entered without the item, just in case
											g_Log.Log(LogType::_ERROR, GetPartyData(i)->pSession, L"Initialize Stage Failed : Map Index - %d, StageClear After UseItem(Cash)(NeedItemID : %d, NeedItemCount : %d, UserItemCount : %d)\n", 
												nMapIndex, nDungeonEnterNeedItem, nDungeonEnterNeedItemCount,  GetPartyData(i)->pSession->GetItem()->GetCashItemCountByItemID(nDungeonEnterNeedItem));
											_DANGER_POINT();
										}
									}
									else
									{
										if( GetPartyData(i)->pSession->GetItem()->GetInventoryItemCount(nDungeonEnterNeedItem) < nDungeonEnterNeedItemCount)
										{
											//Leave a log if entered without the item, just in case
											g_Log.Log(LogType::_ERROR, GetPartyData(i)->pSession, L"Initialize Stage Failed : Map Index - %d, StageClear After UseItem(Normal)(NeedItemID : %d, NeedItemCount : %d, UserItemCount : %d)\n", 
												nMapIndex, nDungeonEnterNeedItem, nDungeonEnterNeedItemCount,  GetPartyData(i)->pSession->GetItem()->GetInventoryItemCount(nDungeonEnterNeedItem));
											_DANGER_POINT();
										}
									}
									continue;
								}
#endif	// #if defined(PRE_ADD_68838)
								//They say only one unconditionally for now; adjust the count used if more are added
								if (GetPartyData(i)->pSession->GetItem()->DeleteInventoryByItemID(nDungeonEnterNeedItem, nDungeonEnterNeedItemCount, DBDNWorldDef::UseItem::Use) == false)
								{
									_DANGER_POINT();		//Check once right before initialization when start button is pressed; reaching here likely means an internal issue, so log and proceed
								}
								else
								{
									GetPartyData(i)->pSession->GetEventSystem()->OnEvent( EventSystem::OnItemUse, 1, EventSystem::ItemID, nDungeonEnterNeedItem);
								}
							}
						}
#if defined( PRE_PARTY_DB )
						// Delete world map -> dungeon party info from DB
						GetRoom()->DelPartyDB();
#endif
#if defined( PRE_ADD_DIRECTNBUFF )
						if(GetRoom()->bIsDirectPartyBuff())
						{
							GetRoom()->ApplyDirectPartyBuff();
							GetRoom()->SendDirectPartyBuffMsg();
						}
#endif
#if defined( PRE_FIX_BUFFITEM )
						GetRoom()->CheckEffectSkillItemData();
#endif
					}
					break;
				}
				break;
		case EWorldEnum::MapTypeWorldMap:
			for( DWORD i=0; i<GetUserCount(); i++ ) {
				GetPartyData(i)->nUsableRebirthCoin = -1;
			}
			ResetDungeonClear( false );		
			ClearDungeonHistory();

			switch( ePrevWorld )
			{
				// World zone -> World zone: inherit both buffs and debuffs as is
				case EWorldEnum::MapTypeWorldMap:
				{
#if defined( PRE_PARTY_DB )
					if (GetRoom()->GetPartyIndex() > 0)
					{
						CDNUserSession* pSession = GetRoom()->GetUserData(0);
						if( pSession )
						{
							Party::Data PartyData = GetRoom()->GetPartyStructData();
							PartyData.iLocationID = nMapIndex;
							GetRoom()->GetDBConnection()->QueryModParty( pSession, PartyData, PartyData.iBitFlag );
						}
					}
#endif // #if defined( PRE_PARTY_DB )
					break;
				}

				// Dungeon -> World Zone : Remove debuff state effects.
				case EWorldEnum::MapTypeDungeon:
					{
						StopTimeAttack();
						
						for( DWORD i=0; i<GetUserCount(); i++ ) 
						{
							if( GetPartyData(i)->pSession->bIsAIBot() ) continue;
							// Deduct previous Map fatigue
							GetPartyData(i)->pSession->DecreaseFatigue();
							// StageClear log
							GetPartyData(i)->pSession->CheckAndSendStageClearLog();
#if defined(PRE_ADD_ACTIVEMISSION)
							// Reset ActiveMission
							GetPartyData(i)->pSession->GetMissionSystem()->ResetActiveMission();
#endif
						}

						GetRoom()->SetStageStartLogFlag( false );

						for( DWORD i = 0; i < GetUserCount(); ++i )
						{
							if( GetPartyData(i)->pSession->GetActorHandle() )
								GetPartyData(i)->pSession->GetActorHandle()->ResetStateEffect( (int)CDnSkill::DurationTypeEnum::Debuff );
						}

						GetRoom()->SendRefreshParty(0, NULL);
#if defined( PRE_PARTY_DB )
						// Dungeon -> World Map: save party info in DB
						GetRoom()->AddPartyDB( NULL );
						
#endif
#if defined( PRE_ADD_DIRECTNBUFF )
						if(GetRoom()->bIsDirectPartyBuff())
						{
							GetRoom()->RemoveDirectPartyBuff(0, true);
						}
#endif
#if defined( PRE_FIX_BUFFITEM )
						GetRoom()->CheckEffectSkillItemData();
#endif
					}
					break;
			}
		break;
	}


	CDnWorld::GetInstance( GetRoom() ).SetMapType( MapType );
	CDnWorld::GetInstance( GetRoom() ).SetMapSubType( MapSubType );
	CDnWorld::GetInstance( GetRoom() ).SetLandEnvironment( Environment );
#if defined(PRE_ADD_ACTIVEMISSION)
	CDnWorld::GetInstance( GetRoom() ).SetMaxPartyCount( nMaxPartyCount );	
#endif

#if !defined(PRE_ADD_RENEW_RANDOM_MAP)
	std::string szTemp;
	DNVector(std::string) szVecMapName;
	szVecMapName.reserve( 10 );
	for( int i=0; i<10; i++ ) {
		sprintf_s( szLabel, "_ToolName%d", i + 1 );
		szTemp = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szTemp.empty() ) continue;

		szVecMapName.push_back( szTemp );
	}

	// Sync RandomSeed
	_srand( GetRoom(), nRandomSeed );
	OutputDebug( "RandomSeed : %d\n", nRandomSeed );

	if( szVecMapName.empty() ) return false;
	int nArrayIndex = _rand(GetRoom())%(int)szVecMapName.size();
	OutputDebug( "MapArrayIndex : %d\n", nArrayIndex );
	szGridName = szVecMapName[nArrayIndex];
	nGridIndex = nArrayIndex;
#endif

	EWorldEnum::WeatherEnum Weather = CDnWorld::GetInstance( GetRoom() ).GetNextWeather( nMapIndex ); 
	CDnWorld::GetInstance( GetRoom() ).SetCurrentWeather( Weather );

	// Sync RandomSeed
	_srand( GetRoom(), nRandomSeed );
	if( bContinueStage ) {
		// Game -> Game Out every time
		int nLogMapIndex = nCurrentMapIndex;
		// On dungeon entry, if DirectConnect, nCurrentMapIndex is passed as the destination map index
		// Leave an Out log with the current map index before changing.
		if( bDirectConnect ) nLogMapIndex = m_nMapTableID;  
		
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			pSession->m_bCharOutLog = false;
		}

		if( InitializeNextStage( szGridName.c_str(), nMapIndex, StageDifficulty, nStartPositionIndex ) == false ) {
			g_Log.Log(LogType::_ERROR, L"Initialize Stage Failed : Map Index - %d, ArrayIndex - %d, Grid Name : %S\n", nMapIndex, nArrayIndex, szGridName.c_str() );
			return false;
		}
		PostInitializeStage( nRandomSeed );

	}
	else {
		if( InitializeStage( szGridName.c_str(), nMapIndex, StageDifficulty, nStartPositionIndex ) == false ) {
			g_Log.Log(LogType::_ERROR, L"Initialize Stage Failed : Map Index - %d, ArrayIndex - %d, Grid Name : %S\n", nMapIndex, nArrayIndex, szGridName.c_str() );
			return false;
		}
	}

	CDnWorld::GetInstance( GetRoom() ).InitializeGateInfo( m_nMapTableID, nGridIndex );
	CDnWorld::GetInstance( GetRoom() ).RefreshGate( m_nMapTableID );

	m_vPermitPassClassIds.clear();
	m_nPermitMapLevel = 0;

	if (CDnWorld::GetInstance( GetRoom() ).GetMapType() == GlobalEnum::MAP_WORLDMAP)
		CDnWorld::GetInstance( GetRoom() ).GetPermitMapCondition(m_nMapTableID, m_vPermitPassClassIds, m_nPermitMapLevel);

//	UpdatePlayerWorldLevel();
	InitializeCP();

	m_nStageRandomSeed = nRandomSeed;
	m_bDungeonFailed = false;
	m_DungeonClearState = DCS_None;

	RecoveryDungeonHistory( m_nMapTableID );

	InitializeMerit();

	m_bDungeonClearQuestComplete = false;
	m_bDungeonClearSendWarpEnable = false;

	return true;
}

void CDnGameTask::InitializeCP()
{
	// Set up CP table.
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );

	char szLabel[64] = {0,};
	static char *szStaticLabel[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare", "Chaos", "Hell" };

	sprintf_s( szLabel, "_CPTableID_%s", szStaticLabel[m_StageDifficulty] );
	int nCPTableID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();

	sprintf_s( szLabel, "_ClearDungeonTableID_%s", szStaticLabel[m_StageDifficulty] );
	int nDungeonClearID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();

	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
		if( !pStruct ) continue;
		DnActorHandle hActor = pStruct->pSession->GetActorHandle();
		if( !hActor ) continue;
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());

		if( nCPTableID > 0 ) 
			pPlayer->InitializeCPScore( nCPTableID, pPlayer->GetClassID(), nDungeonClearID );
	}
}

void CDnGameTask::InitializeBreakIntoPlayerCP( CDNUserSession * pSession )
{
	if( NULL == pSession )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );
	if( NULL == pSox )
		return;

	char szLabel[64] = {0,};
	static char *szStaticLabel[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare", "Chaos", "Hell" };

	sprintf_s( szLabel, "_CPTableID_%s", szStaticLabel[m_StageDifficulty] );
	int nCPTableID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();
	if( 0 == nCPTableID )
		return;

	sprintf_s( szLabel, "_ClearDungeonTableID_%s", szStaticLabel[m_StageDifficulty] );
	int nDungeonClearID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();

	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor )
		return;

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( NULL == pPlayer )
		return;
	
	pPlayer->InitializeCPScore( nCPTableID, pPlayer->GetClassID(), nDungeonClearID );
}

bool CDnGameTask::InitializeWorld( const char *szGridName )
{
	// Map Load
	bool bResult = m_pWorld->Initialize( CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szGridName );
	if( bResult == false ) return false;

	return true;
}

bool CDnGameTask::PostInitializeStage( int nRandomSeed )
{
	// Start with Weapon Unique ID set above a certain value, because the player uses the initial indices.
	STATIC_INSTANCE(CDnWeapon::s_dwUniqueCount) = 100000;

	GenerationMonster( nRandomSeed );
	ResetSetMonsterReference();
	GenerationSetMonster( nRandomSeed );
	GenerationNpc( nRandomSeed );

	// Village -> Game In once
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		GetUserData(i)->SetMapIndex( m_nMapTableID );	// Save the map and

		CDNUserSession *pSession = GetUserData(i);
		pSession->m_bCharOutLog = true;
	}

	return true;
}

void CDnGameTask::FinalizeStage()
{
	ResetHeroAllyRunState();
	CDnDropItem::ReleaseClass( GetRoom() );
	CDnActor::ReleaseClass( GetRoom() );
	CDnWeapon::ReleaseClass( GetRoom(), CDnWeapon::Projectile );
	SAFE_DELETE_VEC( m_hVecMonsterList );
	SAFE_DELETE_PVEC( m_pVecActorBirthList );

	ResetDungeonClear( false );
	ClearDungeonHistory();

	if( m_pWorld ) m_pWorld->Destroy();
}

// ##################################### MUST READ #####################################
//
// If OnInitialzePlayerActor() changes, the protocol synced on join-in-progress must also be updated.
// Therefore, if this changes, be sure to let me (Kimbap) know!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// ##################################### MUST READ #####################################

bool CDnGameTask::OnInitializePlayerActor( CDNUserSession* pSession, const int iVectorIndex )
{
	DnActorHandle hActor = CreateActor( GetRoom(), pSession->GetClassID() );
	if( !hActor ) return false;
	CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();

	// Job must be set first. Definitely!!
	std::vector<int> nVecJobList;
	nVecJobList.reserve( JOBMAX );
	for( int j=0; j<JOBMAX; j++ ) 
	{
		if( pSession->GetStatusData()->cJobArray[j] == 0 ) break;
		nVecJobList.push_back( pSession->GetStatusData()->cJobArray[j] );
	}
	pActor->SetJobHistory( nVecJobList );

	pActor->InitializeRoom( GetRoom() );
	pActor->SetUserSession( pSession );
	hActor->Initialize();
	hActor->SetUniqueID( pSession->GetSessionID() );
	hActor->SetLevel( pSession->GetLevel() );
	pActor->SetExperience( pSession->GetExp() );
	hActor->SetName( pSession->GetCharacterName() );
	pSession->SetActorHandle( hActor );
	SetStartPosition( pSession, iVectorIndex );

	hActor->ResetToggleSkill();
	hActor->ResetAuraSkill();		

	pActor->SetWeaponViewOrder( 0, pSession->IsExistViewCashEquipBitmap(CASHEQUIP_WEAPON1));
	pActor->SetWeaponViewOrder( 1, pSession->IsExistViewCashEquipBitmap(CASHEQUIP_WEAPON2));

	for (int i = 0; i < CDnParts::SwapParts_Amount; ++i){
		pActor->SetPartsViewOrder(i, pSession->IsExistViewCashEquipBitmap(i));
	}

	// Normal item
	for( int j=EQUIP_FACE; j<=EQUIP_RING2; j++ ) 
	{
		CDnItem* pItem = CDnItemTask::GetInstance( GetRoom() ).GetEquipItem( pSession, j );
		if( !pItem ) continue;
		if( pItem->GetItemType() != ITEMTYPE_PARTS ) continue;

		DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
		pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)j );
	}

	for( int j=0; j<2; j++ ) 
	{
		CDnItem* pItem = CDnItemTask::GetInstance( GetRoom() ).GetEquipItem( pSession, j + EQUIP_WEAPON1 );
		if( !pItem ) continue;
		if( pItem->GetItemType() != ITEMTYPE_WEAPON ) continue;

		DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
		pActor->AttachWeapon( hWeapon, j );
	}
	//Cash item
	for( int j=CASHEQUIP_HELMET; j<=CASHEQUIP_FAIRY; j++ ) 
	{
		CDnItem* pItem = CDnItemTask::GetInstance( GetRoom() ).GetCashEquipItem( pSession, j );
		if( !pItem ) continue;
		if( pItem->GetItemType() != ITEMTYPE_PARTS ) continue;

		DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
		pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)j );
	}

	for( int j=0; j<2; j++ ) 
	{
		CDnItem* pItem = CDnItemTask::GetInstance( GetRoom() ).GetCashEquipItem( pSession, j + CASHEQUIP_WEAPON1 );
		if( !pItem ) continue;
		if( pItem->GetItemType() != ITEMTYPE_WEAPON ) continue;

		DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
		pActor->AttachCashWeapon( hWeapon, j );
	}

	// When creating a character on game server
	CDnItem* pVehicleEffect = CDnItemTask::GetInstance( GetRoom() ).GetCashEquipItem( pSession, CASHEQUIP_EFFECT );
	if(pVehicleEffect && pVehicleEffect->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
	{
		pActor->SetVehicleEffectIndex(pVehicleEffect->GetClassID());
	}

	for( int j=0; j<SKILLMAX; j++ ) 
	{
		TSkill* pSkillStruct = &(pSession->GetSkillData()->SkillList[j]);
		if( pSkillStruct->nSkillID == 0 ) 
			continue;
		if( pActor->CanAddSkill( pSkillStruct->nSkillID, pSkillStruct->cSkillLevel ) == false )
			continue;
		if( pActor->AddSkill( pSkillStruct->nSkillID, pSkillStruct->cSkillLevel ) == false ) 
			continue;
		DnSkillHandle hSkill = pActor->FindSkill( pSkillStruct->nSkillID );

		// Level 0 skills that are only unlocked are not returned by MASkillUser::FindSkill.
		if( hSkill )
			hSkill->SetElapsedDelayTime( pSkillStruct->nCoolTime / 1000.f );
	}

	pActor->InitializeEnchantPassiveSkills();

	const TVehicle* pEquipPet = pSession->GetItem()->GetPetEquip();
	if( pEquipPet && pEquipPet->Vehicle[Pet::Slot::Body].nItemID > 0 )
	{
		if( pActor->IsCanPetMode() )
		{
			if( pEquipPet->nSkillID1 > 0 )
				pActor->AddSkill(pEquipPet->nSkillID1);
			if( pEquipPet->nSkillID2 > 0 )
				pActor->AddSkill(pEquipPet->nSkillID2);
		}
		else
		{
			if( pEquipPet->nSkillID1 > 0 )
				pActor->RemoveSkill(pEquipPet->nSkillID1);
			if( pEquipPet->nSkillID2 > 0 )
				pActor->RemoveSkill(pEquipPet->nSkillID2);
		}
	}

	hActor->GetStateBlow()->Process( 0, 0.f, true );

	// Glyph Item
	for( int j=0; j<GLYPHMAX; j++ ) {
		CDnItem *pItem = CDnItemTask::GetInstance(GetRoom()).GetGlyphItem( pSession, j );
		if( !pItem ) continue;
		if(  pItem->GetItemType() == ITEMTYPE_GLYPH ) {
			//For cash crest slot, check if the slot is active
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
			if(j >= GLYPH_CASH1 && j <= GLYPH_CASH3)
#else
			if(j >= GLYPH_CASH1)
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
			{
				TCashGlyphData* GlyphDate = pSession->GetItem()->GetGlyphExpireDate();
				int nIndex = j - GLYPH_CASH1;
				if( !(GlyphDate[nIndex].cActiveGlyph) )
					continue;
			}
			DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
			pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)j );
		}
	}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	// Talisman Item
	{
		__time64_t tCurTime = 0;
		_time64( &tCurTime );
		for( int j=0; j<TALISMAN_MAX; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance(GetRoom()).GetTalismanItem( pSession, j );
			if( !pItem ) continue;
			if( pItem->GetItemType() == ITEMTYPE_TALISMAN ) {
				if( !pSession->GetItem()->IsTalismanSlotOpen(j) )
					continue;
				if( !g_pDataManager->IsTalismanSlotLevel(j, pSession->GetLevel()) )
					continue;
				const TItem *pTalismanData = pSession->GetItem()->GetTalisman( j );
				if( pTalismanData && !pTalismanData->bEternity && pTalismanData->tExpireDate <= tCurTime )
					continue;
				DnTalismanHandle hTalisman = ((CDnTalisman*)pItem)->GetMySmartPtr();
				pActor->AttachTalisman( hTalisman, j, g_pDataManager->GetTalismanSlotEfficiency(j) );
			}
		}
	}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)


	pActor->SetAppellationIndex( pSession->GetAppellationData()->nCoverAppellation, pSession->GetAppellationData()->nSelectAppellation );
	hActor->RefreshState();
	hActor->SetHP( hActor->GetMaxHP() );
	hActor->SetSP( hActor->GetMaxSP() );

	pActor->SetBattleMode( false );
	pActor->RideVehicle(pSession->GetItem()->GetVehicleEquip());

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	if( !pSession->bIsBreakIntoSession() )
	{
		pSession->GetItem()->BroadcastEffectSkillItemData(false);
		std::vector<TEffectSkillData> vEffectSkill;
		vEffectSkill.clear();
		pSession->GetItem()->GetEffectSkillItem( vEffectSkill );
		CDNUserSession::ApplyEffectSkill(pActor, vEffectSkill , true , pSession->GetGameRoom()->bIsPvPRoom() );
	}
#else	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	pSession->GetItem()->BroadcastSourceData(false);
	CDNUserSession::ApplySourceEffect(pActor, pSession->GetItem()->GetSource(), true);
#endif	//	#if defined( PRE_ADD_NAMEDITEM_SYSTEM )

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	if( !pSession->bIsAIBot() )
	{
		int* nTotalLevelSkillData = pSession->GetTotalLevelSkillData();
		pSession->SendTotalLevel(pSession->GetSessionID(), pSession->GetTotalLevelSkillLevel());
		pActor->UpdateTotalLevel( pSession->GetTotalLevelSkillLevel() );
		
		for( int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++ )
		{		
			if(nTotalLevelSkillData[i] > 0)
			{
				pActor->AddTotalLevelSkill(i, nTotalLevelSkillData[i], true);
			}
		}
		for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
		{
			if(g_pDataManager->bIsTotalLevelSkillCashSlot(i))
				pSession->SendTotalLevelSkillCashSlot(pSession->GetSessionID(), i, pSession->bIsTotalLevelSkillCashSlot(i), pSession->GetTotalLevelSkillCashSlot(i));
		}
		pSession->SendTotalLevelSkillList(pSession->GetSessionID(), nTotalLevelSkillData);	
	}
#endif

	if( !pSession->bIsAIBot() )
	{
#if defined(PRE_ADD_DOORS_PROJECT)
		pSession->GetDBConnection()->QuerySaveCharacterAbility(pSession);
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
		if(pSession->GetPrivateChannelID())
		{
			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pSession->GetWorldSetID(), pSession->GetPrivateChannelID());
			if(pPrivateChatChannel && !pPrivateChatChannel->bIsLoaded())
			{
				pSession->GetDBConnection()->QueryGetPrivateChatChannelMember(pSession, pSession->GetPrivateChannelID() );
			}
			else if(pPrivateChatChannel)
			{
				if( pPrivateChatChannel->CheckPrivateChannelMember(pSession->GetCharacterDBID()) )
				{
					pPrivateChatChannel->SetPrivateChannelMemberAccountDBID(pSession->GetAccountDBID(), pSession->GetCharacterDBID());
					pSession->SendPrivateChatChannel(pPrivateChatChannel);
					pSession->GetDBConnection()->QueryModPrivateMemberServerID( pSession );
				}
			}
		}
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
		pSession->GetAppellation()->CheckComebackAppellation( pSession->GetAppellationData()->nSelectAppellation, pSession->GetAppellationData()->nCoverAppellation );
#endif

#if defined( PRE_ADD_GUILDREWARDITEM )	
		if( !pSession->bIsBreakIntoSession() )
		{
			pSession->SetGuildRewardItem();
			pSession->ApplyGuildRewardSkill();	
		}
#endif	// #if defined( PRE_ADD_GUILDREWARDITEM )
	}

	//#68831
	//For normal maps, when adding integrated level skills above, the third argument is set to true so HP is set to MaxHp.
	//In the Colosseum, scaling/correction is applied so it does not work properly
	//Added code below.
	hActor->GetStateBlow()->Process( 0, 0.f, true );
	hActor->RefreshState();
	pActor->CmdRefreshHPSP(pActor->GetMaxHP(), pActor->GetMaxSP());

	return true;
}

void CDnGameTask::InitializeMerit()
{
	m_pMeritInfo = g_pDataManager->GetMeritInfo(GetRoom()->m_nMeritBonusID);
	if (m_pMeritInfo)
	{
		DungeonGateStruct * pDungeonStruct = \
			(DungeonGateStruct *)m_pWorld->GetGateStruct(CDnPartyTask::GetInstance(GetRoom()).GetEnteredGate());
		if (pDungeonStruct)
		{
			if (m_pMeritInfo->nMinLevel > pDungeonStruct->nRecommandedLv[GetStageDifficulty()] && 
				m_pMeritInfo->nMaxLevel < pDungeonStruct->nRecommandedLv[GetStageDifficulty()])
				m_pMeritInfo = NULL;
		}
	}
}

bool CDnGameTask::InitializePlayerActor()
{
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		CDNUserSession *pSession = GetUserData(i);
		if( pSession == NULL ) 
			continue;
		
		if (OnInitializePlayerActor( pSession, i ) == false)
		{
			g_Log.Log(LogType::_INVALIDACTOR, L"OnInitializePlayerActor Failed!!! ClassID[%d]\n", pSession->GetClassID());
			return false;
		}

		if(pSession->GetTeam() != PvPCommon::Team::Observer)
		{
			DnActorHandle hActor = pSession->GetActorHandle();
			if( !hActor )
			{
				g_Log.Log(LogType::_INVALIDACTOR, L"OnInitializePlayerActor if( !hActor ) Failed!!! ClassID[%d]\n", pSession->GetClassID());
				return false;
			}
		}
	}

	return true;
}

bool CDnGameTask::OnInitializeBreakIntoActor( CDNUserSession* pSession, const int iVectorIndex )
{
	_srand( GetRoom(), m_nStageRandomSeed );
	OnInitializePlayerActor( pSession, iVectorIndex );

	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor ) 
		return false;
	hActor->RemoveOctreeNode();
	hActor->SetOctreeUpdate( false );

#if defined( PRE_FIX_82502 )
	InitializeBreakIntoPlayerCP( pSession );
#endif	// #if defined( PRE_FIX_82502 )

	return true;
}

void CDnGameTask::OnInitializeStartPosition( CDNUserSession* pSession, const int iPartyIndex )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	EtVector2	vStartPos;
	float		fRotate;

	CEtWorldEventControl*	pControl	= NULL;
	CEtWorldSector*			pSector		= NULL;
	CEtWorldEventArea*		pArea		= NULL;

	char szStartName[32];
	sprintf_s( szStartName, "StartPosition %d", m_nStartPositionIndex );
	for( DWORD i=0 ; i<m_pWorld->GetGrid()->GetActiveSectorCount() ; i++ ) 
	{
		pSector		= m_pWorld->GetGrid()->GetActiveSector(i);
		pControl	= pSector->GetControlFromUniqueID( ETE_EventArea );
		if( !pControl ) 
			continue;

		pArea = pControl->GetAreaFromName( szStartName );
		if( pArea ) 
		{
			SOBB *pBox = pArea->GetOBB();
			switch( iPartyIndex ) 
			{
				case 0: vStartPos = EtVec3toVec2( pBox->Vertices[4] );									break;
				case 1: vStartPos = EtVec3toVec2( pBox->Vertices[5] );									break;
				case 2: vStartPos = EtVec3toVec2( pBox->Vertices[0] );									break;
				case 3: vStartPos = EtVec3toVec2( pBox->Vertices[1] );									break;
				case 4: vStartPos = EtVec3toVec2( pBox->Center + ( pBox->Axis[2] * pBox->Extent[2] ) ); break;
				case 5: vStartPos = EtVec3toVec2( pBox->Center - ( pBox->Axis[2] * pBox->Extent[2] ) ); break;
				case 6: vStartPos = EtVec3toVec2( pBox->Center - ( pBox->Axis[0] * pBox->Extent[0] ) ); break;
				case 7: vStartPos = EtVec3toVec2( pBox->Center + ( pBox->Axis[0] * pBox->Extent[0] ) ); break;
			}

			fRotate = pArea->GetRotate();

			if( hActor ) 
			{
				float fHeight = m_pWorld->GetHeight( vStartPos.x, vStartPos.y ) + pArea->GetMin()->y;
				hActor->SetPosition( EtVector3( vStartPos.x, fHeight, vStartPos.y ) );

				EtVector2 vDir;
				vDir.x = sin( EtToRadian( fRotate ) );
				vDir.y = cos( EtToRadian( fRotate ) );
				hActor->Look( vDir );

				if( hActor->GetObjectHandle() )
					hActor->GetObjectHandle()->Update( *hActor->GetMatEx() );
			}
		}
	}
}

void CDnGameTask::SetStartPosition( CDNUserSession* pSession, int nPartyIndex )
{
	OnInitializeStartPosition( pSession, nPartyIndex );
}

EtVector3 CDnGameTask::GetGenerationRandomPosition( SOBB *pOBB )
{
	EtVector3 vPos = pOBB->Center;
	vPos -= pOBB->Axis[0] * pOBB->Extent[0];
	vPos -= pOBB->Axis[2] * pOBB->Extent[2];
	int nX = (int)( pOBB->Extent[0] * 2.f );
	int nZ = (int)( pOBB->Extent[2] * 2.f );
	if( nX < 50 ) nX = 50;
	if( nZ < 50 ) nZ = 50;
	nX /= 50;
	nZ /= 50;

	vPos += pOBB->Axis[0] * (float)( (int)( _roomrand(GetRoom())%nX ) * 50.f );
	vPos += pOBB->Axis[2] * (float)( (int)( _roomrand(GetRoom())%nZ ) * 50.f );
	vPos.y = m_pWorld->GetHeight( vPos );
	if( pOBB->Extent[1] != 100000.f )
		vPos += pOBB->Axis[1] * pOBB->Extent[1];

	return vPos;
}

bool CDnGameTask::GenerationMonster( int nRandomSeed )
{
	_srand( GetRoom(), nRandomSeed );
	char szTemp[256];

	DNVector(GenerationMonsterStruct) VecGenerationResult;
	DNVector(MonsterTableStruct) VecMonsterList;

	// Search monster spawn position
	DNTableFileFormat *pSox = NULL;
	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	DNVector(AreaStruct) VecCommonArea;
	char szAreaName[256];
	char szMonsterName[256];

	bool bMonsterNameLoad = false;

	for( DWORD i=0; i<dwCount; i++ ) {
		pSector = m_pWorld->GetGrid()->GetActiveSector(i);
		// Code related to spawning monsters, NPCs, and MonsterGroups in UnitArea.
		pControl = pSector->GetControlFromUniqueID( ETE_UnitArea );
		if( !pControl ) continue;
		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			pArea = pControl->GetAreaFromIndex(j);
			vPos.x = pArea->GetMin()->x + ( _rand(GetRoom())%(int)( pArea->GetMax()->x - pArea->GetMin()->x ) );
			vPos.z = pArea->GetMin()->z + ( _rand(GetRoom())%(int)( pArea->GetMax()->z - pArea->GetMin()->z ) );
			vPos.y = m_pWorld->GetHeight( vPos );

			sprintf_s( szAreaName, pArea->GetName() );
			_strlwr_s( szAreaName );

			memset( szMonsterName, 0, sizeof(szMonsterName) );
			sscanf_s( szAreaName, "%s", szMonsterName, 256 );

			if( stricmp( szMonsterName, "npc" ) == NULL ) continue;
			if( stricmp( szMonsterName, "monster" ) == NULL ) {
				int nMonsterTableID = -1;
				sscanf_s( szAreaName, "%s %d", szMonsterName, 256, &nMonsterTableID );
				if( nMonsterTableID == -1 ) continue;

				GenerationMonsterStruct Struct;
				Struct.nActorTableID = -1;
				Struct.pArea = pArea;

				UnitAreaStruct *pStruct = (UnitAreaStruct *)pArea->GetData();
				int nCount = 1;
				if( pStruct->vCount ) {
					nCount = (int)pStruct->vCount->x + _rand(GetRoom())%( ( (int)pStruct->vCount->y + 1 ) - (int)pStruct->vCount->x );
					if( 0 < nCount) VecGenerationResult.reserve( VecGenerationResult.capacity()+nCount ); // 20080916 performance improvement
				}

				for( int m=0; m<nCount; m++ ) 
				{
					Struct.nMonsterTableID = nMonsterTableID;
					Struct.nMonsterSpawn = pStruct->MonsterSpawn;

					VecGenerationResult.push_back( Struct );
				}
			}
			else if( strcmp( szMonsterName, "monstergroup" ) == NULL ) {
				int nCompoundIndex = -1;
				sscanf_s( szAreaName, "%s %d", szMonsterName, 256, &nCompoundIndex );
				if( nCompoundIndex == -1 ) continue;

				AreaStruct Struct;
				Struct.nCompoundIndex = nCompoundIndex;

				UnitAreaStruct *pStruct = (UnitAreaStruct *)pArea->GetData();
				if( pStruct->vCount == NULL ) {
					Struct.nMin = Struct.nMax = 0;
				}
				else {
					Struct.nMin = (int)pStruct->vCount->x;
					Struct.nMax = (int)pStruct->vCount->y;
				}

				Struct.nMonsterSpawn = pStruct->MonsterSpawn;
				Struct.pArea = pArea;
				VecCommonArea.push_back( Struct );
			}
			else			
			{
				if( !bMonsterNameLoad )
				{
					// Search monster name
					pSox = GetDNTable( CDnTableDB::TACTOR );

					if( 2 < pSox->GetItemCount() )VecMonsterList.reserve( pSox->GetItemCount() ); // 20080916 performance improvement
					for( int i=10; i<pSox->GetItemCount(); i++ ) {
						MonsterTableStruct Struct;
						int nTableID = pSox->GetItemID(i);
						int nClassType = pSox->GetFieldFromLablePtr( nTableID, "_Class" )->GetInteger();
						if( nClassType < CDnActorState::SimpleRush || nClassType >= CDnActorState::Npc ) continue;

						sprintf_s( szTemp, pSox->GetFieldFromLablePtr( nTableID, "_StaticName" )->GetString() );
						if( strlen( szTemp ) == 0 ) continue;
						_strlwr_s( szTemp );

						Struct.szName = szTemp;
						Struct.nActorTableID = nTableID;
						VecMonsterList.push_back( Struct );
					}
					bMonsterNameLoad = true;
				}
				// Create a character for an Area that has a character name specified.
				for( DWORD k=0; k<VecMonsterList.size(); k++ ) 
				{
					if( strstr( szMonsterName, VecMonsterList[k].szName.c_str() ) ) 
					{
						int nMonsterTableID = -1;
						sscanf_s( szAreaName, "%s %d", szMonsterName, 256, &nMonsterTableID );
						if( nMonsterTableID == -1 ) continue;

						GenerationMonsterStruct Struct;
						Struct.nActorTableID = VecMonsterList[k].nActorTableID;
						Struct.pArea = pArea;

						UnitAreaStruct *pStruct = (UnitAreaStruct *)pArea->GetData();
						int nCount = 1;
						if( pStruct->vCount ) {
							nCount = (int)pStruct->vCount->x + _rand(GetRoom())%( ( (int)pStruct->vCount->y + 1 ) - (int)pStruct->vCount->x );
							if( 0 < nCount) VecGenerationResult.reserve( VecGenerationResult.capacity()+nCount ); // 20080916 performance improvement
						}

						for( int m=0; m<nCount; m++ ) 
						{
							Struct.nMonsterTableID = nMonsterTableID;
							Struct.nMonsterSpawn = pStruct->MonsterSpawn;

							VecGenerationResult.push_back( Struct );
						}
						break;
					}
				}
			}
		}
	}

	if( VecCommonArea.empty() && VecGenerationResult.empty()) 
		return false;

	// Get combination table
	pSox = GetDNTable( CDnTableDB::TMONSTERCOMPOUND );

	for( DWORD i=0; i<VecCommonArea.size(); i++ ) {
		int nTableID = VecCommonArea[i].nCompoundIndex;
		if( !pSox->IsExistItem( nTableID ) ) continue;
		DNVector(MonsterCompoundStruct) VecCompoundList;
		VecCompoundList.reserve(15); // 20080916 performance improvement, based on the for loop below
		std::vector<int> nVecOffset;
		nVecOffset.reserve(15); // 20080916 performance improvement, based on the for loop below
		MonsterCompoundStruct Struct;
		char szLabel[32];
		int nRandomMax = 0;
		bool bFavoritism = true;

		for( int j=0; j<15; j++ ) {
			sprintf_s( szLabel, "_MonsterTableID%d_%d", m_StageDifficulty + 1, j + 1 );
			Struct.nMonsterTableID = pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetInteger();
			if( Struct.nMonsterTableID < 1 ) continue;

			_fpreset();
			sprintf_s( szLabel, "_Ratio%d_%d", m_StageDifficulty + 1, j + 1 );
			Struct.nPossessionProb = (int)( ( pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetFloat() + 0.0001f ) * 100 );
			if( Struct.nPossessionProb <= 0 ) continue;

			sprintf_s( szLabel, "_Value%d_%d", m_StageDifficulty + 1, j + 1 );
			Struct.nValue = (int)( pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetInteger() );
			if( Struct.nValue < 1 ) Struct.nValue = 1;

			nRandomMax += Struct.nPossessionProb;
			if( Struct.nPossessionProb != 100 ) bFavoritism = false;

			nVecOffset.push_back( nRandomMax );
			VecCompoundList.push_back( Struct );
		}
		if( VecCompoundList.empty() ) continue;

		int nMaxValue = VecCommonArea[i].nMin + _rand(GetRoom())% ( ( VecCommonArea[i].nMax + 1 ) - VecCommonArea[i].nMin );
		int nCurValue = 0;

		GenerationMonsterStruct GenStruct;

		GenStruct.pArea = VecCommonArea[i].pArea;
		GenStruct.nMonsterSpawn = VecCommonArea[i].nMonsterSpawn;

		if( bFavoritism == true ) {
			MonsterCompoundStruct LocalStruct;
			int nIndex = _rand(GetRoom())%(int)VecCompoundList.size();
			LocalStruct = VecCompoundList[nIndex];
			VecCompoundList.clear();
			nVecOffset.clear();

			VecCompoundList.push_back( LocalStruct );
			nVecOffset.push_back( 100 );
			nRandomMax = 100;
		}
		while(1) {
			int nResult = _rand(GetRoom())%nRandomMax;
			for( DWORD k=0; k<nVecOffset.size(); k++ ) {
				if( nResult < nVecOffset[k] ) {
					if( nCurValue + VecCompoundList[k].nValue > nMaxValue ) {
						// Recalculate random offset.
						int nTemp = VecCompoundList[k].nPossessionProb;
						nVecOffset.erase( nVecOffset.begin() + k );
						VecCompoundList.erase( VecCompoundList.begin() + k );
						for( DWORD h=k; h<nVecOffset.size(); h++ ) nVecOffset[h] -= nTemp;
						nRandomMax -= nTemp;
					}
					else {
						GenStruct.nActorTableID = -1;
						GenStruct.nMonsterTableID = VecCompoundList[k].nMonsterTableID;
						VecGenerationResult.push_back( GenStruct );

						nCurValue += VecCompoundList[k].nValue;
					}
					break;
				}
			}
			if( VecCompoundList.empty() ) break;
			if( nCurValue == nMaxValue ) break;
		}
	}

	// Spawn monster
	pSox = GetDNTable( CDnTableDB::TMONSTER );
	DnActorHandle hMonster;
	for( DWORD i=0; i<VecGenerationResult.size(); i++ ) 
	{
		int nMonsterID = VecGenerationResult[i].nMonsterTableID;
		if( VecGenerationResult[i].nActorTableID == -1 ) {
			if( !pSox->IsExistItem( nMonsterID ) ) continue;
			VecGenerationResult[i].nActorTableID = pSox->GetFieldFromLablePtr( nMonsterID, "_ActorTableID" )->GetInteger();
		}
		if( VecGenerationResult[i].nActorTableID < 1 ) continue;
		if( !pSox->IsExistItem( nMonsterID ) ) continue;
		
		GenerationMonsterStruct& genMonStructInfo = VecGenerationResult[i];
		SOBB *pOBB = genMonStructInfo.pArea->GetOBB();
		if (genMonStructInfo.nMonsterSpawn == 0)
			vPos = GetGenerationRandomPosition( pOBB );
		else
			vPos = pOBB->Center;

		hMonster = CreateActor( GetRoom(), VecGenerationResult[i].nActorTableID );
		if( !hMonster )
		{
			g_Log.Log(LogType::_ERROR, L"CDnGameTask::GenerationMonster() CreateActor() Failed! ActorID:%d\r\n", VecGenerationResult[i].nActorTableID );
			continue;
		}

		hMonster->SetPosition( vPos );

		InitializeMonster( hMonster, STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++, nMonsterID, *pOBB, VecGenerationResult[i].pArea );

		m_hVecMonsterList.push_back( hMonster );		
	}
	return true;
}

bool CDnGameTask::Sort_SetMonster_SetID( MonsterSetStruct &a, MonsterSetStruct &b )
{
	return ( a.nSetID < b.nSetID ) ? true : false;
}
bool CDnGameTask::Sort_SetMonster_PosID( MonsterSetStruct &a, MonsterSetStruct &b )
{
	if( a.nSetID != b.nSetID ) return false;
	return ( a.nPosID < b.nPosID ) ? true : false;
}

bool CDnGameTask::GenerationSetMonster( int nRandomSeed )
{
	_srand( GetRoom(), nRandomSeed );

	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	DnActorHandle hMonster;

	DNVector(MonsterSetStruct) VecSetMonsterList;

	for( DWORD i=0; i<dwCount; i++ ) {
		pSector = m_pWorld->GetGrid()->GetActiveSector(i);
		// Part that spawns using MonsterSet
		pControl = pSector->GetControlFromUniqueID( ETE_MonsterSetArea );
		if( !pControl ) continue;
		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			pArea = pControl->GetAreaFromIndex(j);
			MonsterSetAreaStruct *pStruct = (MonsterSetAreaStruct *)pArea->GetData();
			MonsterSetStruct SetStruct;
			SetStruct.nSetID = pStruct->nSetID;
			SetStruct.nPosID = pStruct->nPosID;
			SetStruct.pArea = pArea;
			SetStruct.nMonsterSpawn = pStruct->MonsterSpawn;

			VecSetMonsterList.push_back( SetStruct );
		}
	}
	
	return GenerationMonster(VecSetMonsterList, 0, 0, &m_hVecMonsterList);
}

bool CDnGameTask::GenerationMonster(DNVector(MonsterSetStruct) &VecSetMonsterList, int nInitalizeType, int nTeamSetting, DNVector(DnActorHandle) * pVecResult)
{
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	DnActorHandle hActor;

	if( VecSetMonsterList.empty() ) return false;

	std::sort( VecSetMonsterList.begin(), VecSetMonsterList.end(), Sort_SetMonster_SetID );
	std::sort( VecSetMonsterList.begin(), VecSetMonsterList.end(), Sort_SetMonster_PosID );

	struct MonsterSpawnAreaInfo
	{
		CEtWorldEventArea* pArea;
		int nMonsterSpawnPos;
	};

	std::map<int, std::map<int, std::vector<MonsterSpawnAreaInfo>>> nMapUseSetList;

	std::map<int, std::map<int, std::vector<MonsterSpawnAreaInfo>>>::iterator iterSet;
	std::map<int, std::vector<MonsterSpawnAreaInfo>>::iterator iterPos;

	// Categorize monster sets by SetID and PosID and store them in a map.
	for( DWORD i=0; i<VecSetMonsterList.size(); i++ ) {
		iterSet = nMapUseSetList.find( VecSetMonsterList[i].nSetID );
		if( iterSet == nMapUseSetList.end() ) {

			std::map<int, std::vector<MonsterSpawnAreaInfo>> nMapUsePosList;
			std::vector<MonsterSpawnAreaInfo> pVecAreaList;

			MonsterSpawnAreaInfo areaInfo;
			areaInfo.pArea = VecSetMonsterList[i].pArea;
			areaInfo.nMonsterSpawnPos = VecSetMonsterList[i].nMonsterSpawn;

			pVecAreaList.push_back( areaInfo );
			nMapUsePosList.insert( make_pair( VecSetMonsterList[i].nPosID, pVecAreaList ) );

			nMapUseSetList.insert( make_pair( VecSetMonsterList[i].nSetID, nMapUsePosList ) );
		}
		else {
			iterPos = iterSet->second.find( VecSetMonsterList[i].nPosID );
			if( iterPos == iterSet->second.end() ) {

				std::vector<MonsterSpawnAreaInfo> pVecAreaList;
				MonsterSpawnAreaInfo areaInfo;
				areaInfo.pArea = VecSetMonsterList[i].pArea;
				areaInfo.nMonsterSpawnPos = VecSetMonsterList[i].nMonsterSpawn;

				pVecAreaList.push_back( areaInfo );

				iterSet->second.insert( make_pair( VecSetMonsterList[i].nPosID, pVecAreaList ) );
			}
			else {

				MonsterSpawnAreaInfo areaInfo;
				areaInfo.pArea = VecSetMonsterList[i].pArea;
				areaInfo.nMonsterSpawnPos = VecSetMonsterList[i].nMonsterSpawn;

				iterPos->second.push_back( areaInfo );
			}
		}
	}

	DNTableFileFormat *pMonsterSox = GetDNTable( CDnTableDB::TMONSTER );

	// Select by referencing actual group ID from organized list
	for( iterSet = nMapUseSetList.begin(); iterSet != nMapUseSetList.end(); iterSet++ ) {
		int nSetID = iterSet->first;
		int nResultSetItemID = CalcSetMonsterReference( nSetID );
		if( nResultSetItemID == -1 ) continue;

		// Organize monster group list using selected set item ID
		std::map<int, DNVector(MonsterGroupStruct)> nMapGroupList;
		std::map<int, DNVector(MonsterGroupStruct)>::iterator iterGroup;

		CalcSetMonsterGroupList( nResultSetItemID, nMapGroupList );

		for( iterPos = iterSet->second.begin(); iterPos != iterSet->second.end(); iterPos++ ) {
			// Get the item list to roll using the actual SetID and PosID.
			std::vector<int> nVecResultMonsterList;
			iterGroup = nMapGroupList.find( iterPos->first );
			if( iterGroup == nMapGroupList.end() ) continue;

			// Once the final monster ID to select is obtained
			for( DWORD i=0; i<iterGroup->second.size(); i++ ) {
				int nMonsterID = CalcMonsterIDFromMonsterGroupID( iterGroup->second[i].nGroupID );
				if( nMonsterID < 1 ) continue;
				for( int j=0; j<iterGroup->second[i].nCount; j++ ) 
					nVecResultMonsterList.push_back( nMonsterID );
			}
			if( nVecResultMonsterList.empty() ) continue;

			// Iterate through event areas and spawn monsters at each position.
			int nAreaCount = 0;
			int nTotalAreaCount = (int)iterPos->second.size();
			while( !nVecResultMonsterList.empty() ) {
				int nOffset = _rand(GetRoom()) % (int)nVecResultMonsterList.size();
				int nMonsterID = nVecResultMonsterList[nOffset];
				nVecResultMonsterList.erase( nVecResultMonsterList.begin() + nOffset );

				if( !pMonsterSox->IsExistItem( nMonsterID ) ) continue;
				int nActorTableID = pMonsterSox->GetFieldFromLablePtr( nMonsterID, "_ActorTableID" )->GetInteger();

				MonsterSpawnAreaInfo &areaInfo = iterPos->second[nAreaCount%nTotalAreaCount];
				pArea = areaInfo.pArea;
				SOBB *pOBB = pArea->GetOBB();

				if (areaInfo.nMonsterSpawnPos == 0)
					vPos = GetGenerationRandomPosition( pOBB );
				else
					vPos = pOBB->Center;

				switch(nInitalizeType)
				{
				case 0:
					{
						hActor = CreateActor( GetRoom(), nActorTableID );
						hActor->SetPosition( vPos );

						InitializeMonster( hActor, STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++, nMonsterID, *pOBB, pArea );
					}
					break;
				case 1:
					{
						hActor = RequestGenerationMonsterFromActorID( nMonsterID, -1, vPos, EtVector3( 0.f, 0.f, 0.f ), EtVector3( 0.f, 0.f, 0.f ), pArea->GetOBB(), pArea->GetCreateUniqueID(), nTeamSetting );
					}
					break;
				}

				if( pVecResult && hActor ) pVecResult->push_back( hActor );

				nAreaCount++;
			}
		}
	}

	return true;
}

int CDnGameTask::CalcMonsterIDFromMonsterGroupID( int nItemID )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMONSTERGROUP );

	int nRandomOffset = _rand(GetRoom())%1000000000;
	char szLabel[32];

	int nTotalProb = 0;

	for( DWORD i=0; i<20; i++ ) {
		sprintf_s( szLabel, "_MonsterID%d", i + 1 );

		int nMonsterID = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetInteger();
		if( nMonsterID < 1 ) continue;

		sprintf_s( szLabel, "_Prob%d", i + 1 );
		int nProb = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetInteger();
		if( nProb < 1 ) continue;

		if( nRandomOffset < nProb + nTotalProb ) return nMonsterID;
		nTotalProb += nProb;
	}
	return 0;
}

bool CDnGameTask::GenerationNpc( int nRandomSeed )
{
	_srand( GetRoom(), nRandomSeed );

	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	char szAreaName[256];
	char szUnitName[256];
	char szNpcName[256];

	for( DWORD i=0; i<dwCount; i++ ) {
		pSector = m_pWorld->GetGrid()->GetActiveSector(i);
		pControl = pSector->GetControlFromUniqueID( ETE_UnitArea );
		if( !pControl ) continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			pArea = pControl->GetAreaFromIndex(j);
			vPos.x = pArea->GetMin()->x + ( _rand(GetRoom())%(int)( pArea->GetMax()->x - pArea->GetMin()->x ) );
			vPos.z = pArea->GetMin()->z + ( _rand(GetRoom())%(int)( pArea->GetMax()->z - pArea->GetMin()->z ) );
			vPos.y = m_pWorld->GetHeight( vPos );

			sprintf_s( szAreaName, pArea->GetName() );
			_strlwr_s( szAreaName );

			memset( szUnitName, 0, sizeof(szUnitName) );
			sscanf_s( szAreaName, "%s", szUnitName, 256 );

			if( strcmp( szUnitName, "npc" ) == NULL ) 
			{
				int nNpcIndex = -1;
				sscanf_s( szAreaName, "%s %d", szNpcName, 256, &nNpcIndex );

				TNpcData* pNpcData = NULL;
				pNpcData = g_pDataManager->GetNpcData(nNpcIndex);
				if (!pNpcData)
				{
					g_Log.Log(LogType::_REMAINEDACTOR, _T("Find npc data Failed: [%d] \n"),nNpcIndex);
					continue;
				}

				EtVector3 vLocalPos;
				vLocalPos.x = pArea->GetMin()->x + ((int)(pArea->GetMax()->x - pArea->GetMin()->x) / 2);
				vLocalPos.z = pArea->GetMin()->z + ((int)(pArea->GetMax()->z - pArea->GetMin()->z) / 2);
				vLocalPos.y = m_pWorld->GetHeight( vLocalPos ) + pArea->GetMin()->y;

				DnActorHandle hNpc = CreateNpc(nNpcIndex, pNpcData, vLocalPos, pArea->GetRotate() );
				if( !hNpc )
				{
					g_Log.Log(LogType::_REMAINEDACTOR, _T("CreateNpc failed : [%d] \n"),nNpcIndex);
					continue;
				}
				InsertBirthAreaList( hNpc, pArea->GetCreateUniqueID() );
			}
		}
	}
	return true;
}

DnActorHandle CDnGameTask::CreateNpc(int nNpcIndex,TNpcData* pNpcData, EtVector3 vPos, float fRot)
{
	DnActorHandle hNPC = CreateNpcActor( GetRoom(), nNpcIndex, pNpcData, vPos, fRot);
	if( !hNPC ) return CDnActor::Identity();
	hNPC->InitializeRoom(GetRoom());
	m_hVecNpcList.push_back(hNPC);
	return hNPC;
}

void CDnGameTask::DestroyNpc(UINT nUniqueID)
{
	DNVector(DnActorHandle)::iterator it = m_hVecNpcList.begin();
	for ( it ; it != m_hVecNpcList.end() ; it++ )
	{
		DnActorHandle hActor = *it;
		if ( hActor->GetUniqueID() == nUniqueID )
		{
			m_hVecNpcList.erase(it);
			return;
		}
	}
}

void CDnGameTask::InitializeMonster( DnActorHandle hMonster, DWORD dwUniqueID, int nMonsterID, SOBB &GenerationArea, CEtWorldEventArea *pBirthArea, int nForceSkillLevel /*= -1*/ )
{
	if( !hMonster )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMONSTER );
	CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(hMonster.GetPointer());
	pMonster->InitializeRoom(GetRoom());

	hMonster->SetUniqueID( dwUniqueID );
	pMonster->SetMonsterClassID( nMonsterID );
	pMonster->SetAIDifficult( m_StageDifficulty );
	hMonster->Initialize();

	char szWeaponLable[32];
	for( int j=0; j<2; j++ ) {
		sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );
		int nWeapon = pSox->GetFieldFromLablePtr( nMonsterID, szWeaponLable )->GetInteger();
		if( nWeapon < 1 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( GetRoom(), nWeapon, _rand(GetRoom()) );
		hMonster->AttachWeapon( hWeapon, j, true );
	}
	// Random size
	_fpreset();
	float fScale = 1.f;
	int nMin = (int)( pSox->GetFieldFromLablePtr( nMonsterID, "_SizeMin" )->GetFloat() * 100.f );
	int nMax = (int)( pSox->GetFieldFromLablePtr( nMonsterID, "_SizeMax" )->GetFloat() * 100.f );
	if( nMin <= nMax ) {
		fScale = ( nMin + ( _rand(GetRoom())%( ( nMax + 1 ) - nMin ) ) ) / 100.f;
	}
	pMonster->SetScale( fScale );

	pMonster->SetGenerationArea( GenerationArea );
	//pMonster->InitializeRoom( m_pSession );

	int nNameID = pSox->GetFieldFromLablePtr( nMonsterID, "_NameID" )->GetInteger();
	if( CEtUIXML::IsActive() )
	{
		hMonster->SetName( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nNameID ) );
	}
	hMonster->SetLevel( pSox->GetFieldFromLablePtr( nMonsterID, "_Level" )->GetInteger() );
	hMonster->RefreshState();
	hMonster->SetHP( hMonster->GetMaxHP() );
	hMonster->SetSP( hMonster->GetMaxSP() );
	hMonster->Look( EtVec3toVec2( GenerationArea.Axis[2] ) );

	// StaticMovement Type has no Look, so force-set it.
	if( hMonster->GetMovement() && hMonster->GetMovement()->bIsStaticMovement() == true )
		hMonster->GetMovement()->ForceLook( EtVec3toVec2( GenerationArea.Axis[2] ) );

	if( pBirthArea ) {
		InsertBirthAreaList( hMonster, pBirthArea->GetCreateUniqueID() );
		pMonster->SetBirthAreaHandle( pBirthArea->GetCreateUniqueID() );
	}

	int nSkillTableId = pSox->GetFieldFromLablePtr( nMonsterID, "_SkillTable")->GetInteger();
	DNTableFileFormat*pSkillSox = GetDNTable( CDnTableDB::TMONSTERSKILL );

	if ( !pSkillSox )
	{
		_ASSERT(0);
		g_Log.Log(LogType::_ERROR, L"TMONSTERSKILL Table is null\n");
		return;
	}

//PROFILE_TIME_TEST_BLOCK_START( "CDnGameTask::InitializeMonster() -> MonsterAddSkill Loop" );
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
#if defined( PRE_ADD_ACADEMIC )
	for( int j=0; j<120; j++ )
#else
	for( int j=0; j<30; j++ )
#endif // #if defined( PRE_ADD_ACADEMIC )
	{
		char szLabel1[128]={0,};
		char szLabel2[128]={0,};
		sprintf_s( szLabel1, "_SkillIndex%d", j + 1);
		sprintf_s( szLabel2, "_SkillLevel%d", j + 1);
		DNTableCell* pField = pSkillSox->GetFieldFromLablePtr( nSkillTableId, szLabel1 );
		if ( !pField )
			continue;

		int nSkillId = pField->GetInteger();

		pField = pSkillSox->GetFieldFromLablePtr( nSkillTableId, szLabel2 );
		if ( !pField )
			continue;

		int nSkillLevel = pField->GetInteger();

		if ( nSkillId < 0 || nSkillLevel < 0 )
			break;

		// Handle state effect #167: force summoned monster skill level.
		if( 0 < nForceSkillLevel )
			nSkillLevel = nForceSkillLevel;
		//////////////////////////////////////////////////////////////////////////

		//PROFILE_TIME_TEST_BLOCK_START( "CDnGameTask::InitializeMonster() -> AddSkill()" );
		bool bResult = hMonster->AddSkill( nSkillId, nSkillLevel );
		if( false == bResult )
		{
			_ASSERT(0);
			// Removed because too many logs were being written.
			//g_Log.Log(LogType::_ERROR, L"Error Exist Skill!, SkillID : %d, SkillLevel : %d name : %s\n", nSkillId, nSkillLevel, hMonster->GetName() );				
		}
		//PROFILE_TIME_TEST_BLOCK_END();
	}
#else
	std::set<int> SkillSet;

#if defined( PRE_ADD_ACADEMIC )
	for( int j=0; j<120; j++ )
#else
	for( int j=0; j<30; j++ )
#endif // #if defined( PRE_ADD_ACADEMIC )
	{
		char szLabel1[128]={0,};
		char szLabel2[128]={0,};
		sprintf_s( szLabel1, "_SkillIndex%d", j + 1);
		sprintf_s( szLabel2, "_SkillLevel%d", j + 1);
		DNTableCell* pField = pSkillSox->GetFieldFromLablePtr( nSkillTableId, szLabel1 );
		if ( !pField )
			continue;

		int nSkillId = pField->GetInteger();

		pField = pSkillSox->GetFieldFromLablePtr( nSkillTableId, szLabel2 );
		if ( !pField )
			continue;

		int nSkillLevel = pField->GetInteger();

		// Handle state effect #167: force summoned monster skill level.
		if( 0 < nForceSkillLevel )
			nSkillLevel = nForceSkillLevel;
		//////////////////////////////////////////////////////////////////////////

		if ( nSkillId < 0 || nSkillLevel < 0 )
			continue;

		std::set<int>::iterator it = SkillSet.find(nSkillLevel);
		if ( it != SkillSet.end() )
		{
			_ASSERT(0);
			g_Log.Log(LogType::_ERROR, L"Error Exist Skill!, SkillID : %d, SkillLevel : %d name : %s\n", nSkillId, nSkillLevel, hMonster->GetName() );				
		}

		//PROFILE_TIME_TEST_BLOCK_START( "CDnGameTask::InitializeMonster() -> AddSkill()" );
		bool bResult = hMonster->AddSkill(nSkillId, nSkillLevel);

		if ( bResult )
		{
			SkillSet.insert(nSkillId);
		}
		//PROFILE_TIME_TEST_BLOCK_END();
	}
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE
//PROFILE_TIME_TEST_BLOCK_END();

	//PROFILE_TIME_TEST_BLOCK_START( "CDnGameTask::InitializeMonster() -> Immune Blow Add" );
	// Set immune state effect index last if present
	const char* pImmunedStateEffects = pSox->GetFieldFromLablePtr( nMonsterID, "_ImmunedStateBlow" )->GetString();
	const char* pImmunePercent = pSox->GetFieldFromLablePtr( nMonsterID, "_ImmunePercent" )->GetString();
	const char* pImmuneReduceTime = pSox->GetFieldFromLablePtr( nMonsterID, "_ImmuneReduceTime")->GetString();
	if( pImmunedStateEffects && 0 < strlen(pImmunedStateEffects) )
	{
		int nISFoundPos = -1;
		int nIPFoundPos = -1;
		int nIRFoundPos = -1;
		string strImmunedStateEffects( pImmunedStateEffects );
		string strImmunePercent( pImmunePercent );
		string strImmuneReduceTime(pImmuneReduceTime);

		if( strImmunedStateEffects.at(strImmunedStateEffects.length()-1) != ';' )
			strImmunedStateEffects.push_back( ';' );
		if( strImmunePercent.at(strImmunePercent.length()-1) != ';' )
			strImmunePercent.push_back( ';' );
		if( strImmuneReduceTime.at(strImmuneReduceTime.length()-1) != ';' )
			strImmuneReduceTime.push_back( ';' );

		while( true )
		{
			int nISStartPos = nISFoundPos+1;
			int nIPStartPos = nIPFoundPos+1;
			int nIRStartPos = nIRFoundPos+1;
		
			nISFoundPos = (int)strImmunedStateEffects.find_first_of( ';', nISStartPos );
			nIPFoundPos = (int)strImmunePercent.find_first_of( ';', nIPStartPos );
			nIRFoundPos = (int)strImmuneReduceTime.find_first_of(';', nIRStartPos );
			if( nISFoundPos != (int)string::npos &&
				nIPFoundPos != (int)string::npos && 
				nIRFoundPos != (int)string::npos  )
			{
				// Set the immunity state effect (77) to the state effect index specified in the table.
				// Client and game server each configure from their own tables, so do not send a packet.
				string strArg( strImmunedStateEffects.substr(nISStartPos, nISFoundPos-nISStartPos) );
				strArg.append(";");
				strArg.append( strImmunePercent.substr(nIPStartPos, nIPFoundPos-nIPStartPos) );
				strArg.append(";");
				strArg.append( strImmuneReduceTime.substr(nIRStartPos, nIRFoundPos-nIRStartPos) );

				hMonster->CDnActor::AddStateBlow( STATE_BLOW::BLOW_077, NULL, -1, strArg.c_str(), false, false );
			}
			else
			{
				_ASSERT( nISFoundPos == (int)string::npos &&
					nIPFoundPos == (int)string::npos && "Monster state effect immunity column count does not match probability count." );
				if( !(nISFoundPos == (int)string::npos && nIPFoundPos == (int)string::npos) )
					OutputDebug( "[MonsterTable Error] id: %d, immunity column count does not match probability count\n", nMonsterID );
				break;
			}
		}
	}
	//PROFILE_TIME_TEST_BLOCK_END();

#ifdef STRESS_TEST
	static int nTeamTemp = 0;
	pMonster->SetTeam( nTeamTemp );
	nTeamTemp++;
#endif
	OutputDebug("[%s] RETURN OK!", __FUNCTION__);
}

void CDnGameTask::RequestGenerationNpc( int nNpcID, float x, float y, float z, float fRotate, int nAreaUniqueID /* = -1 */)
{
	TNpcData* pNpcData = NULL;
	pNpcData = g_pDataManager->GetNpcData(nNpcID);
	if (!pNpcData) 
		return ;

	DnActorHandle hNpc = CreateNpc(nNpcID, pNpcData, EtVector3(x,y,z), fRotate );
	UINT nUniqueID = hNpc->GetUniqueID();

	InsertBirthAreaList(hNpc, nAreaUniqueID);
	for( DWORD i=0; i<GetUserCount(); i++ )
	{
		CDNUserSession *pStruct = GetUserData(i);
		pStruct->SendEnterNpc(nUniqueID, nNpcID, x, y, z, fRotate);
	}

	CDnNPCActor* pNpc = static_cast<CDnNPCActor*>(hNpc.GetPointer());
	pNpc->SetTriggerNpc();

	OutputDebug( "Create Npc : %d\n", nUniqueID );
}

void CDnGameTask::RequestDestroyNpc(int nAreaUniqueID)
{
	DNVector(DnActorHandle) Actors;

	// Get NPCs inside the event area.
	GetBirthAreaLiveActors(nAreaUniqueID, Actors);

	if ( Actors.empty() )
		return ;

	for ( int j = 0 ; j < (int)Actors.size() ; j++ )
	{
		DnActorHandle actor = Actors[j];
		for( DWORD i=0; i<GetUserCount(); i++ ) 
		{
			CDNUserSession *pStruct = GetUserData(i);
			pStruct->SendLeaveNpc(actor->GetUniqueID());
		}
		DestroyNpc(actor->GetUniqueID());
	}

	// Destroy the corresponding NPC actors on game server.
	RemoveBirthAreaList(nAreaUniqueID);
}

DnActorHandle CDnGameTask::RequestGenerationMonsterFromMonsterID( int nMonsterID, EtVector3 &vPosition, EtVector3& vVel, EtVector3& vResistance,
																 SOBB *pGenerationArea, int nAreaUniqueID, int nTeamSetting )
{
	return RequestGenerationMonsterFromActorID( nMonsterID, -1, vPosition, vVel, vResistance, pGenerationArea, nAreaUniqueID, nTeamSetting );
}

DnActorHandle CDnGameTask::RequestGenerationMonsterFromActorID( int nMonsterID, int nActorID, EtVector3 &vPosition, EtVector3& vVel, EtVector3& vResistance,
																SOBB *pGenerationArea, int nAreaUniqueID, int nTeamSetting, bool bRandomFrameSummon, EtQuat* pqRotation, int nForceSkillLevel /*= -1*/ )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	int nActorTableID = -1;
	if( !pSox->IsExistItem( nMonsterID ) ) return CDnActor::Identity();
	if( nActorID > 0 ) {
		nActorTableID = nActorID;
	}
	else {
		nActorTableID = pSox->GetFieldFromLablePtr( nMonsterID, "_ActorTableID" )->GetInteger();
	}

	int nRandomSeed = CRandom::Seed( GetRoom() );
	_srand( GetRoom(), nRandomSeed );

#ifdef PRE_TESTLOG_58816
	// note by kalliste : Temporary log code to resolve #58816 Room Crash issue
	if (nMonsterID == 23042 || nMonsterID == 23043)
		g_Log.Log(LogType::_NORMAL, L"[0526RCTEST] RequestGenerationMonsterFromActorID - MonsterID:%d ActorTableID:%d(%d) nTeamSetting:%d\r\n", nMonsterID, nActorTableID, nActorID, nTeamSetting);
#endif

	DnActorHandle hMonster = CreateActor( GetRoom(), nActorTableID, nTeamSetting );
	if( !hMonster || !hMonster->IsMonsterActor() ) 
		return CDnActor::Identity();

	CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hMonster.GetPointer());
	
	pMonster->SetTriggerMonster( nRandomSeed );
	hMonster->SetPosition( vPosition );
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	pMonster->SetEventAreaUniqueID( nAreaUniqueID );
#endif // PRE_MOD_DARKLAIR_RECONNECT

	SOBB Box;
	if( pGenerationArea ) 
	{
		Box = *pGenerationArea;
	}
	else {
		Box.Center = vPosition;
		Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
		Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
		Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
		Box.Extent[0] = 100.f;
		Box.Extent[1] = 0.f;
		Box.Extent[2] = 100.f;
	}
	std::vector<CEtWorldEventArea *> pVecArea;
	CEtWorldEventArea *pEventArea = NULL;
	if( m_pWorld->FindEventAreaFromCreateUniqueID( nAreaUniqueID, &pVecArea ) > 0 ) {
		pEventArea = pVecArea[0];
	}

#if defined(PRE_FIX_51048)
	//Added these functions due to packet send order issues in GameTask::RequestGenerationMonsterFromActorID
	//ApplyPassiveSkill is called inside InitializeMonster, at which point the client has not created the monster yet.
	//Passive skill state effects are applied only on the server side, not on the client.
	//Set the flag here and build the list of state effects to apply in ApplyPassiveSkill.
	pMonster->SetEnablePassiveStateEffectList(true);
#endif // PRE_FIX_51048

	InitializeMonster( hMonster, STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++, nMonsterID, Box, pEventArea, nForceSkillLevel );
	m_hVecMonsterList.push_back( hMonster );
	if( bRandomFrameSummon ) {
		bRandomFrameSummon = pMonster->IsSummonRandomFrame();
	}

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pStruct = GetUserData(i);

		if( pStruct->bIsBreakIntoSession() == true )
		{
			if( pStruct->GetState() != SESSION_STATE_GAME_PLAY )
				continue;
		}	
		SendGameGenerationMonsterMsg( pStruct, nRandomSeed, nMonsterID, hMonster->GetUniqueID(), *(hMonster->GetPosition()), vVel, vResistance, ( pEventArea ) ? pEventArea->GetCreateUniqueID() : -1 , nTeamSetting, bRandomFrameSummon, pqRotation );
	}

#if defined(PRE_FIX_51048)
	//Actually apply the passive state effects in the list after calling SendGameGenerationMonsterMsg.
	pMonster->SetEnablePassiveStateEffectList(false);
	pMonster->ApplyPassiveStateEffect();
	pMonster->InitPassiveStateEffectInfo();
#endif // PRE_FIX_51048

#if defined(PRE_ADD_WEEKLYEVENT)
	if (CDnWorld::GetInstance(GetRoom()).GetMapSubType() != EWorldEnum::MapSubTypeNest){
		int nThreadID = GetRoom()->GetServerID();

		float fEventHp = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Monster, 0, WeeklyEvent::Event_1, nThreadID);
		if (fEventHp != 0.f){
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventHp));
			int nBlowIndex = hMonster->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_058, -1, strValue.c_str(), false, true );
			DnBlowHandle hBlow = hMonster->GetStateBlowFromID( nBlowIndex );
			if( hBlow ) 
				hBlow->SetHPMPFullWhenBegin();
		}
		float fEventAttack = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Monster, 0, WeeklyEvent::Event_2, nThreadID);
		if (fEventAttack != 0.f){
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventAttack));
			hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_002, -1, strValue.c_str(), false, true);
			hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_029, -1, strValue.c_str(), false, true);
		}

		float fEventDefense = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Monster, 0, WeeklyEvent::Event_3, nThreadID);
		if (fEventDefense != 0.f){
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventDefense));
			hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_004, -1, strValue.c_str(), false, true);
			hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_094, -1, strValue.c_str(), false, true);
		}
	}
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)

#if defined( PRE_ADD_STAGE_WEIGHT )
	if( m_StageWeightData.fHPRate > 0 )
	{
		std::string strValue;
		strValue.append(boost::lexical_cast<std::string>( m_StageWeightData.fHPRate ));
		int nBlowIndex = hMonster->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_058, -1, strValue.c_str(), false, true );
		DnBlowHandle hBlow = hMonster->GetStateBlowFromID( nBlowIndex );
		if( hBlow ) 
			hBlow->SetHPMPFullWhenBegin();
	}

	if( m_StageWeightData.fAttackRate > 0 )
	{
		std::string strValue;
		strValue.append(boost::lexical_cast<std::string>( m_StageWeightData.fAttackRate ));
		hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_002, -1, strValue.c_str(), false, true);
		hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_029, -1, strValue.c_str(), false, true);
	}

	if( m_StageWeightData.fSuperArmorRate > 0 ) 
	{
		std::string strValue;
		strValue.append(boost::lexical_cast<std::string>( m_StageWeightData.fSuperArmorRate ));
		hMonster->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_064, -1, strValue.c_str(), false, true);
	}
#endif

	hMonster->SyncClassTime( 0 );
	
	// Sends Stand action to client right after summoning in MAAiScript::Process() before action queue is processed
	// In some cases, set action directly here instead of ActionQueue. <- Changed to SetAction, but an error occurred.

	// If SetAction is called right when m_LocalTime is set to 0 by SyncClassTime, the animation start frame value gets messed up.
	// Consequently, Summon_On action may fail to execute properly. Therefore, use SetActionQueue here. 
	
	hMonster->SetActionQueue( "Summon_On", 0, 0.f, 0.f, false );
	//hMonster->SetAction( "Summon_On", 0.f, 0.f, false );

	if( bRandomFrameSummon )
	{
		// SetPlaySpeed(..., 0.f) halts ProcessSignal during the random delay, so apply frame-0
		// STE_CanHit=false immediately to prevent attacks killing monsters before Summon_On runs.
		CDnActionBase::ActionElementStruct* pSummonElem = hMonster->GetElement( "Summon_On" );
		if( pSummonElem )
		{
			for( DWORD s = 0; s < pSummonElem->pVecSignalList.size(); ++s )
			{
				CEtActionSignal* pSig = pSummonElem->pVecSignalList[s];
				if( pSig && pSig->GetSignalIndex() == STE_CanHit && pSig->GetStartFrame() <= 0 )
				{
					CanHitStruct* pCanHit = (CanHitStruct*)pSig->GetData();
					if( pCanHit && pCanHit->bHittable == FALSE )
					{
						hMonster->SetHittable( false );
						break;
					}
				}
			}
		}
		hMonster->SetPlaySpeed( ( _rand(GetRoom())%20 ) * 100, 0.f );
	}

#ifdef PRE_MOD_PRELOAD_MONSTER
	if( nTeamSetting == 3 )
		hMonster->SetDestroy();
#endif

	return hMonster;
}

void CDnGameTask::RequestGenerationMonsterFromMonsterGroupID( int nMonsterGroupID, int nCount, SOBB &GenerationArea, int nAreaUniqueID, DNVector(DnActorHandle) *pVecResult, int nTeamSetting )
{
	DNTableFileFormat*pSox = GetDNTable( CDnTableDB::TMONSTERCOMPOUND );
	DNVector(MonsterCompoundStruct) VecCompoundList;
	std::vector<int> nVecOffset;
	MonsterCompoundStruct Struct;
	int nRandomMax = 0;
	bool bFavoritism = true;

	nVecOffset.reserve( 15 );
	VecCompoundList.reserve( 15 );

	char szLabel[32];
	for( int j=0; j<15; j++ ) {
		sprintf_s( szLabel, "_MonsterTableID%d_%d", m_StageDifficulty + 1, j + 1 );
		if( !pSox->IsExistItem( nMonsterGroupID ) )
		{
			_DANGER_POINT();
			continue;
		}
		Struct.nMonsterTableID = pSox->GetFieldFromLablePtr( nMonsterGroupID, szLabel )->GetInteger();
		if( Struct.nMonsterTableID < 1 ) continue;

		_fpreset();
		sprintf_s( szLabel, "_Ratio%d_%d", m_StageDifficulty + 1, j + 1 );
		Struct.nPossessionProb = (int)( ( pSox->GetFieldFromLablePtr( nMonsterGroupID, szLabel )->GetFloat() + 0.0001f ) * 100 );
		if( Struct.nPossessionProb <= 0 ) continue;

		sprintf_s( szLabel, "_Value%d_%d", m_StageDifficulty + 1, j + 1 );
		Struct.nValue = (int)( pSox->GetFieldFromLablePtr( nMonsterGroupID, szLabel )->GetInteger() );
		if( Struct.nValue < 1 ) Struct.nValue = 1;

		nRandomMax += Struct.nPossessionProb;
		if( Struct.nPossessionProb != 100 ) bFavoritism = false;

		nVecOffset.push_back( nRandomMax );

		VecCompoundList.push_back( Struct );
	}
	if( VecCompoundList.empty() ) return;

	int nMaxValue = nCount;
	int nCurValue = 0;

	if( bFavoritism == true ) {
		MonsterCompoundStruct LocalStruct;
		int nIndex = _rand(GetRoom())%(int)VecCompoundList.size();
		LocalStruct = VecCompoundList[nIndex];
		VecCompoundList.clear();
		nVecOffset.clear();

		VecCompoundList.push_back( LocalStruct );
		nVecOffset.push_back( 100 );
		nRandomMax = 100;
	}

	EtVector3 vPos;
	DnActorHandle hMonster;
	while(1) {
		int nResult = _rand(GetRoom())%nRandomMax;
		for( DWORD k=0; k<nVecOffset.size(); k++ ) {
			if( nResult < nVecOffset[k] ) {
				if( nCurValue + VecCompoundList[k].nValue > nMaxValue ) {
					// Recalculate random offset.
					int nTemp = VecCompoundList[k].nPossessionProb;
					nVecOffset.erase( nVecOffset.begin() + k );
					VecCompoundList.erase( VecCompoundList.begin() + k );
					for( DWORD h=k; h<nVecOffset.size(); h++ ) nVecOffset[h] -= nTemp;
					nRandomMax -= nTemp;
				}
				else {
					/*
					vPos.x = GenerationArea.Min.x + ( _rand(GetRoom())%(int)( GenerationArea.Max.x - GenerationArea.Min.x ) );
					vPos.z = GenerationArea.Min.z + ( _rand(GetRoom())%(int)( GenerationArea.Max.z - GenerationArea.Min.z ) );
					*/

					int nMonsterSpawn = GetMonsterSpawnType(nAreaUniqueID);
					
					if (nMonsterSpawn == 0)
						vPos = GetGenerationRandomPosition( &GenerationArea );
					else
						vPos = GenerationArea.Center;

#ifdef PRE_TESTLOG_58816
					// note by kalliste : Temporary log code to resolve #58816 Room Crash issue
					if (nMonsterGroupID == 11110 || nMonsterGroupID == 11111)
					{
						DWORD userCount = GetUserCount();
						if (userCount > 0)
						{
							CDNUserSession *pStruct = GetUserData(0);
							if (pStruct)
								g_Log.Log(LogType::_NORMAL, L"[0526RCTEST] RequestGenerationMonsterFromMonsterGroupID - MonsterGroupId:%d, MonsterCount:%d, MonsterTableID:%d, MapIndex:%d, AreaUniqueID:%d partyId:%u\r\n", nMonsterGroupID, nCount, VecCompoundList[k].nMonsterTableID, pStruct->GetMapIndex(), nAreaUniqueID, pStruct->GetPartyID());
						}
					}
#endif

					hMonster = RequestGenerationMonsterFromMonsterID( VecCompoundList[k].nMonsterTableID, vPos, EtVector3( 0.f, 0.f, 0.f ), EtVector3( 0.f, 0.f, 0.f ), &GenerationArea, nAreaUniqueID, nTeamSetting );
					if( !hMonster ) {
						nCurValue += VecCompoundList[k].nValue;
						break;
					}
					if( pVecResult ) pVecResult->push_back( hMonster );

					nCurValue += VecCompoundList[k].nValue;
				}
				break;
			}
		}
		if( VecCompoundList.empty() ) break;
		if( nCurValue == nMaxValue ) break;
	}
}

int CDnGameTask::GetMonsterSpawnType(int nAreaUniqueID)
{
	int nMonsterSpawn = 0;

	std::vector<CEtWorldEventArea *> pVecArea;
	CEtWorldEventArea *pEventArea = NULL;
	if( m_pWorld->FindEventAreaFromCreateUniqueID( nAreaUniqueID, &pVecArea ) > 0 ) {
		pEventArea = pVecArea[0];
	}				

	if (pEventArea)
	{
		switch(pEventArea->GetControl()->GetUniqueID())
		{
		case ETE_MonsterSetArea:
			{
				MonsterSetAreaStruct *pStruct = (MonsterSetAreaStruct *)pEventArea->GetData();
				nMonsterSpawn = pStruct->MonsterSpawn;
			}
			break;
		case ETE_UnitArea:
			{
				UnitAreaStruct *pStruct = (UnitAreaStruct *)pEventArea->GetData();
				nMonsterSpawn = pStruct->MonsterSpawn;
			}
			break;
		case ETE_EventArea:
			{
				EventAreaStruct *pStruct = (EventAreaStruct *)pEventArea->GetData();
				nMonsterSpawn = pStruct->MonsterSpawn;
			}
			break;
		}
	}

	return nMonsterSpawn;
}

void CDnGameTask::RequestGenerationMonsterFromSetMonsterID( int nSetMonsterID, int nPositionID, SOBB &GenerationArea, int nAreaUniqueID, DNVector(DnActorHandle) *pVecResult, int nTeamSetting, bool bResetReference )
{
	int nResultSetItemID = CalcSetMonsterReference( nSetMonsterID, bResetReference );

	// Organize monster group list using selected set item ID
	std::map<int, DNVector(MonsterGroupStruct)> nMapGroupList;
	std::map<int, DNVector(MonsterGroupStruct)>::iterator iterGroup;

	CalcSetMonsterGroupList( nResultSetItemID, nMapGroupList );

	// Get the item list to roll using the actual SetID and PosID.
	std::vector<int> nVecResultMonsterList;
	iterGroup = nMapGroupList.find( nPositionID );
	if( iterGroup == nMapGroupList.end() ) return;

	// Once the final monster ID to select is obtained
	for( DWORD i=0; i<iterGroup->second.size(); i++ ) {
		int nMonsterID = CalcMonsterIDFromMonsterGroupID( iterGroup->second[i].nGroupID );
		if( nMonsterID < 1 ) continue;
		for( int j=0; j<iterGroup->second[i].nCount; j++ ) 
			nVecResultMonsterList.push_back( nMonsterID );
	}
	if( nVecResultMonsterList.empty() ) return;

	EtVector3 vPos;
	DnActorHandle hActor;
	for( DWORD i=0; i<nVecResultMonsterList.size(); i++ ) {
		int nMonsterSpawn = GetMonsterSpawnType(nAreaUniqueID);

		if (nMonsterSpawn == 0)
			vPos = GetGenerationRandomPosition( &GenerationArea );
		else
			vPos = GenerationArea.Center;

		hActor = RequestGenerationMonsterFromActorID( nVecResultMonsterList[i], -1, vPos, EtVector3( 0.f, 0.f, 0.f ), EtVector3( 0.f, 0.f, 0.f ), &GenerationArea, nAreaUniqueID, nTeamSetting );
		if( pVecResult && hActor ) pVecResult->push_back( hActor );
	}
}

void CDnGameTask::RequestGenerationMonsterFromSetMonsterID( int nSetMonsterID, int nPositionID, std::vector<CEtWorldEventArea *> &pVecAreaUniqueList, DNVector(DnActorHandle) *pVecResult, int nTeamSetting, bool bResetReference )
{
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	DnActorHandle hActor;

	DNVector(MonsterSetStruct) VecSetMonsterList;

	for( DWORD j=0; j<pVecAreaUniqueList.size(); j++ ) {
		int nSetID = nSetMonsterID;
		int nPosID = nPositionID;
		int nMonsterSpawn = 0;
		pArea = pVecAreaUniqueList[j];
		if( nSetMonsterID == 0 || nPositionID == 0 ) {
			if( pArea->GetControl()->GetUniqueID() == ETE_MonsterSetArea ) {
				MonsterSetAreaStruct *pStruct = (MonsterSetAreaStruct *)pArea->GetData();
				if( nSetID == 0 ) nSetID = pStruct->nSetID;
				if( nPosID == 0 ) nPosID = pStruct->nPosID;
				//Monster spawn method
				nMonsterSpawn = pStruct->MonsterSpawn;
			}
			else continue;
		}
		if( nSetID > 0 && nPosID > 0 ) {
			MonsterSetStruct SetStruct;
			SetStruct.nSetID = nSetID;
			SetStruct.nPosID = nPosID;
			SetStruct.nMonsterSpawn = nMonsterSpawn;
			SetStruct.pArea = pArea;
			VecSetMonsterList.push_back( SetStruct );
		}
	}
	
	GenerationMonster(VecSetMonsterList, 1, nTeamSetting, pVecResult);
}

int CDnGameTask::CalcSetMonsterReference( int nSetID, bool bNewReference )
{
	std::map<int, int>::iterator it = m_nMapSetMonsterReference.find( nSetID );
	if( it != m_nMapSetMonsterReference.end() ) {
		if( bNewReference ) m_nMapSetMonsterReference.erase( it );
		else return it->second;
	}
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERSET );

	std::vector<int> nVecItemID;
	DNVector(MonsterSetProbStruct) VecSetProb;
	pSox->GetItemIDListFromField( "_SetNo", nSetID, nVecItemID );
	int nTotalProb = 0;

	for( DWORD i=0; i<nVecItemID.size(); i++ ) {
		if( pSox->GetFieldFromLablePtr( nVecItemID[i], "_Difficulty" )->GetInteger() == GetStageDifficulty() ) {
			MonsterSetProbStruct ProbStruct;
			ProbStruct.nItemID = nVecItemID[i];
			int nProb = (int)( ( pSox->GetFieldFromLablePtr( nVecItemID[i], "_SetWeight" )->GetFloat() + 0.0001f ) * 100.f );
			ProbStruct.nProb = nTotalProb + nProb;
			nTotalProb += nProb;
			VecSetProb.push_back( ProbStruct );
		}
	}
	if( nTotalProb == 0 ) return -1;

	int nRandomOffset = _rand(GetRoom())%nTotalProb;
	int nResultSetItemID = -1;
	for( DWORD i=0; i<VecSetProb.size(); i++ ) {
		if( nRandomOffset < VecSetProb[i].nProb ) {
			m_nMapSetMonsterReference.insert( make_pair( nSetID, VecSetProb[i].nItemID ) );
			return VecSetProb[i].nItemID;
		}
	}
	return -1;
}

void CDnGameTask::CalcSetMonsterGroupList( int nSetTableID, std::map<int, DNVector(MonsterGroupStruct)> &nMapResult )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERSET );

	char szLabel[32];
	std::map<int, DNVector(MonsterGroupStruct)>::iterator iterGroup;

	for( int i=0; i<20; i++ ) {
		sprintf_s( szLabel, "_PosID%d", i + 1 );
		if( !pSox->IsExistItem( nSetTableID ) )
		{
			_DANGER_POINT();
			continue;
		}
		int nPosID = pSox->GetFieldFromLablePtr( nSetTableID, szLabel )->GetInteger();
		if( nPosID < 1 ) continue;
		sprintf_s( szLabel, "_GroupID%d", i + 1 );
		int nGroupID = pSox->GetFieldFromLablePtr( nSetTableID, szLabel )->GetInteger();
		if( nGroupID < 1 ) continue;
		sprintf_s( szLabel, "_Num%d", i + 1 );
		int nGroupMonsterCount = pSox->GetFieldFromLablePtr( nSetTableID, szLabel )->GetInteger();
		if( nGroupMonsterCount < 1 ) continue;

		MonsterGroupStruct GroupStruct;
		GroupStruct.nGroupID = nGroupID;
		GroupStruct.nCount = nGroupMonsterCount;

		iterGroup = nMapResult.find( nPosID );
		if( iterGroup == nMapResult.end() ) {
			DNVector(MonsterGroupStruct) VecGroupList;
			VecGroupList.push_back( GroupStruct );

			nMapResult.insert( make_pair( nPosID, VecGroupList ) );
		}
		else {
			iterGroup->second.push_back( GroupStruct );
		}
	}
}

void CDnGameTask::ResetSetMonsterReference()
{
	SAFE_DELETE_MAP( m_nMapSetMonsterReference );
}

bool CDnGameTask::CheckMonsterIsInsideArea( int nAreaUniqueID, int nTargetUniqueID )
{
	std::vector<CEtWorldEventArea*> VecArea;
	m_pWorld->FindEventAreaFromCreateUniqueID( nTargetUniqueID, &VecArea );
	if( VecArea.empty() )
	{
		//_DANGER_POINT();
		return false;
	}

	int nCount = 0;

	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) 
	{
		if( m_pVecActorBirthList[i]->nAreaUniqueID != nAreaUniqueID )
			continue;

		nCount = 0;
		for( DWORD j=0; j<m_pVecActorBirthList[i]->hVecList.size(); j++ ) 
		{
			DnActorHandle hActor = m_pVecActorBirthList[i]->hVecList[j];

			if( !hActor ) continue;
			if( hActor->IsDie() ) continue;

			SOBB* pOBB = VecArea[0]->GetOBB();
			if( !pOBB )
				return false;

			if( !pOBB->IsInside( *hActor->GetPosition() ) )
				return false;

			++nCount;
		}
		break;
	}

	if( nCount == 0 ) 
		return false;

	return true;
}

bool CDnGameTask::CheckMonsterLessHP( int iMonsterID, int iHP )
{
	for( UINT i=0; i<STATIC_INSTANCE(CDnActor::s_pVecProcessList).size() ; ++i )
	{
		DnActorHandle hActor = STATIC_INSTANCE(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
		if( hActor && hActor->IsMonsterActor() && !hActor->IsDie() )
		{
			CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
			if( pMonster->GetMonsterClassID() == iMonsterID )
			{
				int iHPPercent = hActor->GetHPPercent();
				if( iHPPercent <= iHP )
					return true;
			}
		}
	}

	return false;
}

bool CDnGameTask::CheckInsideAreaMonsterLessHP( int iAreaUniqueID, int iHP )
{
	DNVector(DnActorHandle) vActor;
	GetBirthAreaLiveActors( iAreaUniqueID, vActor );
	if( vActor.empty() )
		return false;

	for( UINT i=0 ; i<vActor.size() ; ++i )
	{
		DnActorHandle hActor = vActor[i];
		if( hActor && hActor->IsMonsterActor() )
		{
			int iHPPercent = hActor->GetHPPercent();
			if( iHPPercent <= iHP )
				return true;
		}
	}

	return false;
}

int CDnGameTask::GetInsideAreaMonsterHPPercent( int iAreaUniqueID )
{
	DNVector(DnActorHandle) vActor;
	GetBirthAreaLiveActors( iAreaUniqueID, vActor );
	if( vActor.empty() )
		return -1;

	for( UINT i=0 ; i<vActor.size() ; ++i )
	{
		DnActorHandle hActor = vActor[i];
		if( hActor && hActor->IsMonsterActor() )
		{
			return hActor->GetHPPercent();
		}
	}

	return -1;
}

void CDnGameTask::RequestDestroyAllMonster( bool bDropItem, int nTeam )
{
	DnActorHandle hActor;
	CDnMonsterActor *pMonster;
	for( DWORD i=0; i<STATIC_INSTANCE(CDnActor::s_pVecProcessList).size(); i++ ) {
		hActor = STATIC_INSTANCE(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
		if( !hActor ) continue;
		if( !hActor->IsMonsterActor() ) continue;
		pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if( !pMonster ) continue;
		if( pMonster->IsDie() ) continue;
		if( nTeam != -1 && pMonster->GetTeam() != nTeam ) continue;

		// Exception for cannon monsters (#18028)
		if( pMonster->GetActorType() == CDnActorState::Cannon ) continue;

		// Exception for monsters summoned by a player (#42685)
		if( pMonster->GetSummonerPlayerActor() && pMonster->GetSummonerPlayerActor()->IsPlayerActor() ) 
			continue;

		pMonster->CmdSuicide( bDropItem, false );
	}
}

void CDnGameTask::InsertBirthAreaList( DnActorHandle hActor, int nAreaUniqueID )
{
	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) {
		if( m_pVecActorBirthList[i]->nAreaUniqueID == nAreaUniqueID ) {
			m_pVecActorBirthList[i]->hVecList.push_back( hActor );
			return;
		}
	}
	ActorBirthStruct *pStruct = new ActorBirthStruct;
	pStruct->nAreaUniqueID = nAreaUniqueID;
	pStruct->hVecList.push_back( hActor );
	m_pVecActorBirthList.push_back( pStruct );
}

// 
void CDnGameTask::RemoveBirthAreaList( int nAreaUniqueID )
{
	std::vector<ActorBirthStruct *>::iterator it = m_pVecActorBirthList.begin();
	while( it != m_pVecActorBirthList.end() )
	{
		ActorBirthStruct* pStruct = *it;

		if ( pStruct && pStruct->nAreaUniqueID == nAreaUniqueID  )
		{
			for ( DWORD j = 0 ; j < (DWORD)pStruct->hVecList.size() ; j++ )
			{
				pStruct->hVecList[j]->SetDestroy();
			}


			it = m_pVecActorBirthList.erase( it );
			SAFE_DELETE(pStruct);
		}
		else
		{
			it++;
		}
	}

}

void CDnGameTask::RemoveBirthAreaList( DnActorHandle hActor, int nAreaUniqueID )
{
	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) {
		if( m_pVecActorBirthList[i]->nAreaUniqueID == nAreaUniqueID ) {
			for( DWORD j=0; j<m_pVecActorBirthList[i]->hVecList.size(); j++ ) {
				if( m_pVecActorBirthList[i]->hVecList[j] == hActor ) {
					m_pVecActorBirthList[i]->hVecList.erase( m_pVecActorBirthList[i]->hVecList.begin() + j );
					break;
				}
			}
			break;
		}
	}
}

DWORD CDnGameTask::GetBirthAreaLiveCount( int nAreaUniqueID )
{
	DWORD dwResult = 0;
	DnActorHandle hActor;

	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) 
	{
		if( m_pVecActorBirthList[i]->nAreaUniqueID != nAreaUniqueID )
			continue;

		for( DWORD j=0; j<m_pVecActorBirthList[i]->hVecList.size(); j++ ) 
		{
			hActor = m_pVecActorBirthList[i]->hVecList[j];

			if( !hActor ) continue;
			if( hActor->IsDie() ) continue;
			dwResult++;
		}

		break;
	}

	return dwResult;
}

DWORD CDnGameTask::GetBirthAreaTotalCount( int nAreaUniqueID )
{
	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) 
	{
		if( m_pVecActorBirthList[i]->nAreaUniqueID == nAreaUniqueID ) 
			return static_cast<DWORD>(m_pVecActorBirthList[i]->hVecList.size());
	}

	return 0;
}

bool CDnGameTask::GetBirthAreaLiveActors( int nAreaUniqueID, DNVector(DnActorHandle)& out )
{
	bool bResult = false;
	DnActorHandle hActor;

	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) 
	{
		if( m_pVecActorBirthList[i]->nAreaUniqueID != nAreaUniqueID )
			continue;

		for( DWORD j=0; j<m_pVecActorBirthList[i]->hVecList.size(); j++ ) 
		{
			hActor = m_pVecActorBirthList[i]->hVecList[j];

			if( !hActor ) continue;
			if( hActor->IsDie() ) continue;
			out.push_back(hActor);
			bResult = true;
		}

		break;
	}

	return bResult;

}

void CDnGameTask::RequestGateInfo( CDNUserSession* pBreakIntoGameSession/*=NULL*/ )
{
	char cGateIndex[DNWORLD_GATE_COUNT] = { 0, };
	char cPermitFlag[DNWORLD_GATE_COUNT] = { 0, };
	for( DWORD i=0; i<CDnWorld::GetInstance(GetRoom()).GetGateCount(); i++ ) {
		cGateIndex[i] = CDnWorld::GetInstance(GetRoom()).GetGateStructFromIndex(i)->cGateIndex;
		cPermitFlag[i] = CDnWorld::GetInstance(GetRoom()).GetGateStructFromIndex(i)->PermitFlag;
	}

	if( pBreakIntoGameSession )
	{
		SendGameGateInfo( pBreakIntoGameSession, (char)CDnWorld::GetInstance(GetRoom()).GetGateCount(), cGateIndex, cPermitFlag );
	}
	else
	{
		for( DWORD i=0; i<GetUserCount(); i++ ) 
		{
			CDNUserSession *pStruct = GetUserData(i);

			SendGameGateInfo( pStruct, (char)CDnWorld::GetInstance(GetRoom()).GetGateCount(), cGateIndex, cPermitFlag );
		}
	}

	for( DWORD i=0; i<CDnWorld::GetInstance(GetRoom()).GetGateCount(); i++ ) 
	{
		GateStruct *pStruct = CDnWorld::GetInstance(GetRoom()).GetGateStructFromIndex(i);
		if( pStruct->PermitFlag == EWorldEnum::PermitEnter || pStruct->PermitFlag == EWorldEnum::PermitClose ) continue;

		char cActorIndex[PARTYMAX*3] = { 0, };

		int nOffset = 0;
		for( DWORD j=0; j<pStruct->cVecLessLevelActorIndex.size(); j++ ) {
			cActorIndex[j+nOffset] = pStruct->cVecLessLevelActorIndex[j];
		}
		nOffset += (int)pStruct->cVecLessLevelActorIndex.size();

		for( DWORD j=0; j<pStruct->cVecNotEnoughItemActorIndex.size(); j++ ) {
			cActorIndex[j+nOffset] = pStruct->cVecNotEnoughItemActorIndex[j];
		}
		nOffset += (int)pStruct->cVecNotEnoughItemActorIndex.size();

		for( DWORD j=0; j<pStruct->cVecExceedTryActorIndex.size(); j++ ) {
			cActorIndex[j+nOffset] = pStruct->cVecExceedTryActorIndex[j];
		}
		nOffset += (int)pStruct->cVecExceedTryActorIndex.size();

		for( DWORD j=0; j<pStruct->cVecNotRideVehicleActorIndex.size(); j++ ) {
			cActorIndex[j+nOffset] = pStruct->cVecNotRideVehicleActorIndex[j];
		}
		nOffset += (int)pStruct->cVecNotRideVehicleActorIndex.size();

		if( pBreakIntoGameSession )
		{
			SendGameGateEjectInfo( pBreakIntoGameSession, pStruct->cGateIndex, (char)pStruct->cVecLessLevelActorIndex.size(), (char)pStruct->cVecNotEnoughItemActorIndex.size(), (char)pStruct->cVecExceedTryActorIndex.size(), (char)pStruct->cVecNotRideVehicleActorIndex.size(), cActorIndex );
		}
		else
		{
			for( DWORD j=0; j<GetUserCount(); j++ ) 
			{
				CDNUserSession *pSession = GetUserData(j);
				SendGameGateEjectInfo( pSession, pStruct->cGateIndex, (char)pStruct->cVecLessLevelActorIndex.size(), (char)pStruct->cVecNotEnoughItemActorIndex.size(), (char)pStruct->cVecExceedTryActorIndex.size(), (char)pStruct->cVecNotRideVehicleActorIndex.size(), cActorIndex );
			}
		}
	}
}

void CDnGameTask::RequestChangeGateInfo( char cGateIndex, EWorldEnum::PermitGateEnum PermitFlag )
{
	CDnWorld::GetInstance(GetRoom()).SetPermitGate( cGateIndex, PermitFlag );

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pStruct = GetUserData(i);
		//for( DWORD i=0; i<m_pSession->GetUserCount(); i++ ) {
		//	CDNUserConnection *pStruct = m_pSession->GetUserData(i);
		SendGameChangeGateState( pStruct, cGateIndex, PermitFlag );
	}
}


char CDnGameTask::GetTreasureBoxType( int nOffset, DNVector(TreasureBoxLevelStruct) &VecList )
{
	int nPrevOffset = 0;
	int nSeed = rand()%nOffset;
	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( nSeed >= nPrevOffset && nSeed < VecList[i].nOffset ) return VecList[i].cTreasureLevel;
	}
	return 0;
}

void CDnGameTask::ChangeDungeonClearState( DungeonClearStateEnum State )
{
	m_DungeonClearState = State;
	switch( m_DungeonClearState ) {
		case DCS_RequestDungeonClear:
			m_DungeonClearState = DCS_ClearResultStay;
			m_fDungeonClearDelta = 0.f;
			break;
		case DCS_RequestSelectRewardItem:
			m_DungeonClearState = DCS_SelectRewardItemStay;
			m_fDungeonClearDelta = 0.f;

			for( DWORD i=0; i<GetUserCount(); i++ ) 
			{
				CDNUserSession* pSession = GetUserData(i);
				if( pSession && (pSession->bIsGMTrace() || pSession->bIsAIBot()) )
					continue;
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
#endif	//	#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )

				char cCount = 0;

#if defined( PRE_ADD_DUNGEONCLEARINFO )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( !pStruct || !pStruct->pSession || pStruct->pSession->bIsAIBot() ) continue;

					CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pStruct->pSession->GetCharacterDBID() );
					if( !PartyFirst )
					{
						// By design, missing case is a bug
						g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[GameTask::ChangeDungeonClearState] PartyFirst error!!\r\n");
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
#endif	//	#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )
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
#else	//	#if defined( PRE_ADD_DUNGEONCLEARINFO )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
					if( !pStruct || !pStruct->pSession || pStruct->pSession->bIsAIBot() ) continue;


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
//					memcpy( Info[cCount].nRewardItemID, pStruct->ClearInfo.nRewardItemID, sizeof(pStruct->ClearInfo.nRewardItemID) );
#endif	//	#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )

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
#endif	//	#if defined( PRE_ADD_DUNGEONCLEARINFO )
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardBoxType( GetUserData(i), cCount, Info );
				}
#else
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					SendGameDungeonClearRewardItem( GetUserData(i), cCount, Info );
				}
#endif	//	#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )
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
					if( !pStruct || !pStruct->pSession || pStruct->pSession->bIsAIBot() ) continue;

#if defined( PRE_ADD_DUNGEONCLEARINFO )
					CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pStruct->pSession->GetCharacterDBID() );
					if( !PartyFirst )
					{
						// By design, missing case is a bug
						g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[GameTask::ChangeDungeonClearState] PartyFirst error!!\r\n");
						continue;
					}
					Info[cCount].nSessionID = pStruct->pSession->GetSessionID();
					Info[cCount].cIndex = PartyFirst->ClearInfo.cSelectRewardItem;

					int iCreateInvenItemRet = ERROR_ITEM_NOTFOUND;

					BYTE cThreadID;
					CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
				
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
#endif
						if( !PartyFirst->ClearInfo.IsFlag(j) ) continue;
						if( PartyFirst->ClearInfo.RewardItem[j].nItemID > 0 ) 
						{				
							iCreateInvenItemRet = pStruct->pSession->GetItem()->CreateInvenWholeItem( PartyFirst->ClearInfo.RewardItem[j], DBDNWorldDef::AddMaterializedItem::DungeonReward, pStruct->pSession->GetPartyID() );

							// Stage treasure chest log
							pDBCon->QueryAddStageRewardLog( cThreadID, GetRoom()->GetWorldSetID(), pStruct->pSession->GetAccountDBID(), GetRoom()->GetRoomLogIndex(), pStruct->pSession->GetCharacterDBID(), 
								static_cast<DBDNWorldDef::RewardBoxCode::eCode>(PartyFirst->ClearInfo.cRewardItemType[j]+1), PartyFirst->ClearInfo.RewardItem[j].nSerial, PartyFirst->ClearInfo.RewardItem[j].nItemID, iCreateInvenItemRet );

							if( PartyFirst->ClearInfo.RewardItem[j].nItemID == 255 )
								g_Log.Log(LogType::_DANGERPOINT, L"[Reward Stage Item] Gain Stage Reward Item : %d!!\r\n", PartyFirst->ClearInfo.RewardItem[j].nItemID);
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

					int iCreateInvenItemRet = ERROR_ITEM_NOTFOUND;

					BYTE cThreadID;
					CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );

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
#endif
						if( !pStruct->ClearInfo.IsFlag(j) ) continue;
						if( pStruct->ClearInfo.RewardItem[j].nItemID > 0 ) 
						{
							iCreateInvenItemRet = pStruct->pSession->GetItem()->CreateInvenWholeItem( pStruct->ClearInfo.RewardItem[j], DBDNWorldDef::AddMaterializedItem::DungeonReward, pStruct->pSession->GetPartyID() );

							// Stage treasure chest log
							pDBCon->QueryAddStageRewardLog( cThreadID, GetRoom()->GetWorldSetID(), pStruct->pSession->GetAccountDBID(), GetRoom()->GetRoomLogIndex(), pStruct->pSession->GetCharacterDBID(), 
								static_cast<DBDNWorldDef::RewardBoxCode::eCode>(pStruct->ClearInfo.cRewardItemType[j]+1), pStruct->ClearInfo.RewardItem[j].nSerial, pStruct->ClearInfo.RewardItem[j].nItemID, iCreateInvenItemRet );

							if( pStruct->ClearInfo.RewardItem[j].nItemID == 255 )
								g_Log.Log(LogType::_DANGERPOINT, L"[Reward Stage Item] Gain Stage Reward Item : %d!!\r\n", pStruct->ClearInfo.RewardItem[j].nItemID);

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

				if (m_pWorld)
					m_pWorld->OnTriggerEventCallback( "CDnGameTask::ChangeDungeonClearState", m_LocalTime, m_fDelta );
			
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
	}
}

bool CDnGameTask::ProcessDungeonClearState( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_DungeonClearState == DCS_None) return false;

	if (m_DungeonClearState == DCS_WarpStandBy)
	{
		if (m_bDungeonClearQuestComplete && CDnPartyTask::GetInstance(GetRoom()).IsPartySharingReversionItem() == false && m_bIgnoreDungeonClearRewardItem == false )
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
			if( m_fDungeonClearDelta >= DCS_ClearResultStay_Delay ) {
				if( m_bIgnoreDungeonClearRewardItem )
					ChangeDungeonClearState( DCS_WarpStandBy );
				else ChangeDungeonClearState( DCS_RequestSelectRewardItem );
			}
			break;
		case DCS_SelectRewardItemStay:
#if defined( PRE_SKIP_REWARDBOX )
			if( m_fDungeonClearDelta >= DCS_SelectRewardItemStay_Delay || m_nRewardBoxUserCount == 0) {
				ChangeDungeonClearState( DCS_RequestRewardItemStay );
			}
#else
			if( m_fDungeonClearDelta >= DCS_SelectRewardItemStay_Delay ) {
				ChangeDungeonClearState( DCS_RequestRewardItemStay );
			}
#endif
			break;
		case DCS_RewardItemStay:
			if( m_fDungeonClearDelta >= DCS_RewardItemStay_Delay ) {
				ChangeDungeonClearState( DCS_RequestRewardItemResult );
			}
			break;
		case DCS_RewardItemResultStay:
			if( m_fDungeonClearDelta >= DCS_RewardItemResultStay_Delay ) {
				ChangeDungeonClearState(DCS_WarpStandBy);
			}
			break;
	}
	return true;
}

void CDnGameTask::RequestDungeonClearBase()
{
	if( GetRoom() && GetRoom()->GetMasterRewardSystem() )
	{
		for( UINT i=0 ; i<GetRoom()->GetUserCount() ; ++i )
		{
			CDNUserSession* pSession = GetRoom()->GetUserData(i);
			if( pSession && !pSession->bIsAIBot() )
				GetRoom()->GetMasterRewardSystem()->RequestDungeonClear( pSession );
		}
	}
	GetRoom()->DelBackupPartyInfo();
}

void CDnGameTask::AddDungeonClearUserInfo( std::vector<TDungeonClearInfo> & vInfo, std::vector<CDNGameRoom::PartyStruct *> & vPartyStruct )
{
	DWORD dwAllUserCount = GetUserCount();
	for( DWORD itr = 0; itr < dwAllUserCount; ++itr ) 
	{
		CDNGameRoom::PartyStruct *pStruct = GetPartyData(itr);
		if( pStruct == NULL )		
			continue;
		if( !pStruct->pSession )	
			continue;
		if( pStruct->pSession->bIsGMTrace() || pStruct->pSession->bIsAIBot() )
			continue;
		DnActorHandle hActor = pStruct->pSession->GetActorHandle();
		if( !hActor || hActor->bIsObserver() ) 
			continue;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());

		std::vector<TDungeonClearInfo>::iterator Itor = vInfo.insert( vInfo.end(), TDungeonClearInfo() );
		Itor->nSessionID = pStruct->pSession->GetSessionID();
		Itor->nMaxComboCount = pPlayer->GetMaxComboCount();
		Itor->nKillBossCount = pPlayer->GetKillBossCount();

		vPartyStruct.push_back( pStruct );

		if( hActor->IsDie() )
		{
			hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_057, 5000, "StageClear" );

			// Note Hanki: When using a skill right after pressing resurrect, when the skill request packet from client arrives at server,
			// Since the server may still be running the "Die" action and fail the skill use while the client uses the skill 
			// Execute "Stand" action. (Moving after becoming a ghost triggers "Stand" on the server, before which it is in "Die" state)
			hActor->SetActionQueue( "Stand" );
		}

		hActor->RemovedActivatedStateBlow( true ); // #47289 Remove all other active state effects.
		hActor->CmdAddStateEffect(NULL, STATE_BLOW::BLOW_099, -1, "-1");
	}
}

int CDnGameTask::CalcBonusCP( const int nDungeonClearID, const int nMaxComboCount, const int nKillBossCount, CDnPlayerActor * pPlayerActor )
{
	int nRetValue = 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONCLEAR );
	if( NULL == pSox )
		return 0;

	int nStartingMemberCount = GetRoom()->m_iPartMemberCnt;
	if( nStartingMemberCount > 1 )
	{
		char szLabel[64] = {0,};
		sprintf_s( szLabel, "_RewordCP%d", nStartingMemberCount );
		nRetValue += pSox->GetFieldFromLablePtr( nDungeonClearID, szLabel )->GetInteger();
	}

#if defined( PRE_ADD_CP_RENEWAL )
	const int nStandardCP = pPlayerActor->GetStandardCP();

	int nScore = nMaxComboCount * pSox->GetFieldFromLablePtr( nDungeonClearID, "_MaxComboScore" )->GetInteger();
	int nMaxScore = nStandardCP * MACP_Renewal::eBonusCP_Revision_MaxCombo;
	if( nMaxScore < nScore )
		nScore = nMaxScore;
	nRetValue += nScore;

	nScore = nKillBossCount * pSox->GetFieldFromLablePtr( nDungeonClearID, "_KillBossNumScore" )->GetInteger();
	nRetValue += nScore;

	int nTimeLimitScore = pSox->GetFieldFromLablePtr( nDungeonClearID, "_TimeLimitScore" )->GetInteger();
	if( 0 == nTimeLimitScore )
	{
		nTimeLimitScore = MACP_Renewal::eBonusCP_Revision_ClearTime;
		_ASSERT( 0 );
	}

	int nTimeLimit = (nStandardCP * MACP_Renewal::eBonusCP_Revision_ClearTime / nTimeLimitScore) + pSox->GetFieldFromLablePtr( nDungeonClearID, "_MaxComboScore" )->GetInteger();
	int nRevisionClearTime = MACP_Renewal::GetClearTime_Revision( m_StageDifficulty );
	nScore = ( nTimeLimit - ( GetRoom()->GetDungeonPlayTime() / 1000 ) + nRevisionClearTime ) * pSox->GetFieldFromLablePtr( nDungeonClearID, "_TimeLimitScore" )->GetInteger();
	nMaxScore = nStandardCP * MACP_Renewal::eBonusCP_Revision_ClearTime;
	if( nScore < 0 )
		nScore = 0;
	else if( nMaxScore < nScore )
		nScore = nMaxScore;
	nRetValue += nScore;
#else	// #if defined( PRE_ADD_CP_RENEWAL )
	nRetValue += nMaxComboCount * pSox->GetFieldFromLablePtr( nDungeonClearID, "_MaxComboScore" )->GetInteger();

	nRetValue += nKillBossCount * pSox->GetFieldFromLablePtr( nDungeonClearID, "_KillBossNumScore" )->GetInteger();

	int nTemp = ( pSox->GetFieldFromLablePtr( nDungeonClearID, "_TimeLimit" )->GetInteger() - ( GetRoom()->GetDungeonPlayTime() / 1000 ) ) * pSox->GetFieldFromLablePtr( nDungeonClearID, "_TimeLimitScore" )->GetInteger();
	if( nTemp < 0 )
		nTemp = 0;
	nRetValue += nTemp;
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

	return nRetValue;
}

int CDnGameTask::CalcCP( std::vector<TDungeonClearInfo> & vInfo, std::vector<CDNGameRoom::PartyStruct *> & vPartyStruct, const int nDungeonClearID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONCLEAR );
	if( NULL == pSox )
		return 0;

	int nRetValue = 0;
	char cClearUserCount = (char)vInfo.size();
	for( char itr = 0; itr < cClearUserCount; ++itr )
	{
		if( NULL == vPartyStruct[itr]->pSession || !vPartyStruct[itr]->pSession->GetActorHandle() )
			continue;
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(vPartyStruct[itr]->pSession->GetActorHandle().GetPointer());

		// Default CP
		int nDefaultCP = pPlayer->GetAccumulationCP();
		if( nDefaultCP < 0 )
			nDefaultCP = 0;
		vInfo[itr].nDefaultCP = nDefaultCP;

		nDefaultCP += CalcBonusCP( nDungeonClearID, vInfo[itr].nMaxComboCount, vInfo[itr].nKillBossCount, pPlayer );
		vInfo[itr].nBonusCP = nDefaultCP - vInfo[itr].nDefaultCP;

#if defined( PRE_ADD_CP_RENEWAL )
		if( nRetValue < nDefaultCP )
			nRetValue = nDefaultCP;
#else	// #if defined( PRE_ADD_CP_RENEWAL )
		nRetValue += nDefaultCP;
#endif	// #if defined( PRE_ADD_CP_RENEWAL )
	}

#if defined( PRE_ADD_CP_RENEWAL )
#else	// #if defined( PRE_ADD_CP_RENEWAL )
	int nOrgAveCP = nRetValue;
	nRetValue = static_cast<int>( nRetValue*(1.f-GetRoom()->GetHackRate()));
	if( GetRoom()->GetHackRate() >= 0.01f )
		g_Log.Log(LogType::_CLEARGRADEPENALTY, L"[%d] RoomID=%d HackPenalty=%d!!! CP: %d->%d\n", g_Config.nManagedID, GetRoom()->GetRoomID(), GetRoom()->GetHackPenalty(), nOrgAveCP, nRetValue );

	nRetValue /= cClearUserCount;
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

	return nRetValue;
}

BYTE CDnGameTask::CalcRank( std::vector<float> & vecRankRatio, const int nStandardCP, const int nCP )
{
	BYTE cRank = 255;
	for( int jtr = 0; jtr < (int)vecRankRatio.size(); ++jtr )
	{
		if( nCP >= (int)( nStandardCP * vecRankRatio[jtr] ) )
		{
			cRank = jtr;
			break;
		}
	}

	return cRank;
}

void CDnGameTask::RequestDungeonClear( bool bClear, DnActorHandle hIgnoreActor, bool bIgnoreRewardItem )
{
	if( CDnWorld::GetInstance(GetRoom()).GetMapType() != EWorldEnum::MapTypeDungeon ) return;

#if defined (_FINAL_BUILD)
	if( !m_bEnteredDungeon ) {
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pStruct = GetUserData(i);
			std::wstring szTemp = L"Cannot execute dungeon clear without starting a gate.";
			pStruct->SendChat( CHATTYPE_SYSTEM, (int)szTemp.size() * sizeof(WCHAR), L"", (WCHAR*)szTemp.c_str() );
		}
		return;
	}
#endif // #if defined (_FINAL_BUILD)

	if( m_DungeonClearState != DCS_None )
	{
		// Duplicate stage clear received; log and return
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[RequestDungeonClear] m_DungeonClearState error MapID:%d!!\r\n", m_nMapTableID);
		return;
	}

	m_bIgnoreDungeonClearRewardItem = bIgnoreRewardItem;
	GetRoom()->AddDungeonPlayTime( timeGetTime() - m_dwStageCreateTime );
	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONCLEAR );
	DNTableFileFormat* pStandardSOX = GetDNTable( CDnTableDB::TCLEARSTANDARD );

	char szLabel[64] = {0,};
	static char *szStaticLabel[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare", "Chaos", "Hell" };
	sprintf_s( szLabel, "_ClearDungeonTableID_%s", szStaticLabel[m_StageDifficulty] );
	int nDungeonClearID = pMapSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();
	if( nDungeonClearID == 0 ) return;

	int nStandardID = pSox->GetFieldFromLablePtr( nDungeonClearID, "_StandardID" )->GetInteger();
	int nReturnWorldID = pSox->GetFieldFromLablePtr( nDungeonClearID, "_ReturnWorldID" )->GetInteger();
	int nReturnWorldStartPos = pSox->GetFieldFromLablePtr( nDungeonClearID, "_ReturnWorldStartPos" )->GetInteger();
	int iRewardGP = pSox->GetFieldFromLablePtr( nDungeonClearID, "_RewardGP" )->GetInteger();

	if( nReturnWorldID > 0 )
	{
		m_nReturnDungeonClearWorldID = nReturnWorldID;
		m_nReturnDungeonClearStartPositionIndex = nReturnWorldStartPos;
	}

	std::vector<TDungeonClearInfo> vInfo;
	std::vector<CDNGameRoom::PartyStruct *> vPartyStruct;

	AddDungeonClearUserInfo( vInfo, vPartyStruct );
	char cClearUserCount = (char)vInfo.size();				// Number of users to actually set dungeon clear info for

#if defined( PRE_SKIP_REWARDBOX )
	m_nRewardBoxUserCount = cClearUserCount;
#endif

	// Read base values for CP calculation.
#define MAX_CLEAR_RANK_COUNT 7
	static char *szStaticRank[MAX_CLEAR_RANK_COUNT] = { "SSS", "SS", "S", "A", "B", "C", "D" };
	std::vector<float> vecRankRatio;
	std::vector<float> vecExpRatio;

	for( int i=0; i<MAX_CLEAR_RANK_COUNT; i++ )
	{
		sprintf_s( szLabel, "_%sRankExpProb", szStaticRank[i] );
		vecExpRatio.push_back( pStandardSOX->GetFieldFromLablePtr( nStandardID, szLabel )->GetFloat() );

		if( i == MAX_CLEAR_RANK_COUNT - 1 ) continue;
		sprintf_s( szLabel, "_%sRankCPProb", szStaticRank[i] );
		vecRankRatio.push_back( pStandardSOX->GetFieldFromLablePtr( nStandardID, szLabel )->GetFloat() );
	}

	const int nCP = CalcCP( vInfo, vPartyStruct, nDungeonClearID );

#if defined(PRE_ADD_CP_RANK)
	int nAbyssLvlMin = 0;
	if( pMapSox->IsExistItem( m_nEnterMapTableID ) ) 
	{
		int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( m_nEnterMapTableID, "_EnterConditionTableID" )->GetInteger();
		if( nDungeonEnterTableID > 0 ) 
		{
			DNTableFileFormat *pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
			if( pDungeonSOX->IsExistItem( nDungeonEnterTableID ) )			
				nAbyssLvlMin = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_RecommendNightmareLevel" )->GetInteger();
		}
	}
#endif //#if defined(PRE_ADD_CP_RANK)

	for( char itr = 0; itr < cClearUserCount; ++itr )
	{
		if( !vPartyStruct[itr]->pSession || !vPartyStruct[itr]->pSession->GetActorHandle() )
			continue;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(vPartyStruct[itr]->pSession->GetActorHandle().GetPointer());
		if( NULL == pPlayer )
			continue;

#if defined( PRE_ADD_DUNGEONCLEARINFO )
		CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( vPartyStruct[itr]->pSession->GetCharacterDBID() );
		if( !PartyFirst )
		{
			// By design, missing case is a bug
			g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[GameTask::RequestDungeonClear] PartyFirst error!!\r\n");
			continue;
		}
#endif

#if defined( PRE_ADD_CP_RENEWAL )
		const int nStandardCP = pPlayer->GetStandardCP() * MACP_Renewal::eBonusCP_Revision_ClearTime;
#else	// #if defined( PRE_ADD_CP_RENEWAL )
		const int nStandardCP = pSox->GetFieldFromLablePtr( nDungeonClearID, "_StandardCP" )->GetInteger();
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

		vInfo[itr].cRank = CalcRank( vecRankRatio, nStandardCP, nCP );
		
		if( vInfo[itr].cRank == 255 )
			vInfo[itr].cRank = MAX_CLEAR_RANK_COUNT - 1;

		vInfo[itr].nCompleteExperience = pPlayer->GetCompleteExperience();

		vInfo[itr].nRewardExperience = (int)( vInfo[itr].nCompleteExperience * vecExpRatio[vInfo[itr].cRank] );

		CheckMerit(pPlayer, GlobalEnum::MERIT_BONUS_COMPLETEEXP, vInfo[itr].nRewardExperience, vInfo[itr].nMeritBonusExperience);
		GetRoom()->GetEventExpWhenStageClear(vPartyStruct[itr]->pSession, vInfo[itr].nRewardExperience, vInfo[itr].cBonusCount, vInfo[itr].EventClearBonus);

		vInfo[itr].nPromotionExperience = (int)(((float)vInfo[itr].nRewardExperience * (float)((float)(vPartyStruct[itr]->pSession->GetPromotionValue(PROMOTIONTYPE_STAGECLEAR))/100)) + 0.5f);
		vInfo[itr].cMaxLevelCharCount = vPartyStruct[itr]->pSession->GetMaxLevelCharacterCount();

		int nBonusExperience = 0;
		for( int vtr = 0; vtr < vInfo[itr].cBonusCount; ++vtr )
			nBonusExperience += vInfo[itr].EventClearBonus[vtr].nClearEventBonusExperience;

		if( GetRoom()->GetMasterRewardSystem() )
		{
			float fRate = GetRoom()->GetMasterRewardSystem()->GetExpRewardRate( pPlayer->GetUserSession() );
			if( fRate > 0.f )
			{				
				int iMasterAddExp = GetRoom()->GetMasterRewardSystem()->GetMasterSystemAddExp(pPlayer->GetUserSession(), static_cast<float>(vInfo[itr].nRewardExperience), true);
				
				const MasterSystem::CRewardSystem::TCountInfo* pCountInfo = GetRoom()->GetMasterRewardSystem()->GetCountInfo( pPlayer->GetUserSession() );
				if( pCountInfo && pCountInfo->iPupilCount > 0 )
				{
					// Nests also grant clear EXP.
					if( CDnWorld::GetInstance(GetRoom()).GetMapSubType() == EWorldEnum::MapSubTypeNest)				
						vInfo[itr].nRewardExperience += iMasterAddExp; //Add to reward EXP
					else				
					{
						vInfo[itr].nRewardExperience = iMasterAddExp;	//Replace reward EXP
						vInfo[itr].nCompleteExperience = iMasterAddExp;
					}
				}
				else if( pCountInfo && pCountInfo->iMasterCount > 0 )
				{
					vInfo[itr].nRewardExperience += iMasterAddExp;		//Add to reward EXP
				}
			}
		}
		if( pPlayer->GetUserSession()->GetPeriodExpItemRate() > 0 )
		{
			int nAddItemExp = static_cast<int>(vInfo[itr].nRewardExperience* (float)(pPlayer->GetUserSession()->GetPeriodExpItemRate()/100.0f));
			vInfo[itr].nRewardExperience += nAddItemExp; //Add to reward EXP				
		}

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)		
		if (pPlayer && pPlayer->IsAppliedThisStateBlow(STATE_BLOW::BLOW_255))
		{
			float fIncExpRate = 0.0f;
			DNVector(DnBlowHandle) vlBlows;
			pPlayer->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_255, vlBlows);
			{
				int nCount = (int)vlBlows.size();
				for (int j = 0; j < nCount; ++j)
				{
					DnBlowHandle hBlow = vlBlows[j];
					if (hBlow && hBlow->IsEnd() == false)
					{
						fIncExpRate += hBlow->GetFloatValue();
					}
				}
				int nAddTotalLevelSkillExp = static_cast<int>(vInfo[itr].nRewardExperience * fIncExpRate);
				vInfo[itr].nRewardExperience += nAddTotalLevelSkillExp;
			}
		}		
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

		pPlayer->UpdateMaxLevelGainExperience( vInfo[itr].nRewardExperience + vInfo[itr].nMeritBonusExperience + nBonusExperience + vInfo[itr].nPromotionExperience );
		// Apply max-level bonus EXP only in normal dungeons.
		switch( CDnWorld::GetInstance(GetRoom()).GetMapSubType() ) {
			case EWorldEnum::MapSubTypeNone:
				{
					int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
					if( pPlayer->GetLevel() >= nLevelLimit ) {
						vInfo[itr].cClearRewardType = 1;
						vInfo[itr].nRewardGold = (int)( pPlayer->GetMaxLevelGainExperience() * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MaxLevelExpTransGoldValue ) );
					}
				}
				break;
		}
		
		// Set reference values for reward items		
		static char *szBoxStr[] = { "_Bronze", "_Silver", "_Gold", "_Platinum" };		
		//Change grade group 
		static int nTreasureRatioRank[] = { 4, 3, 2, 1, 1, 1, 1 };
#if defined(PRE_DETAILCLEARRANK)
		static int nTreasureRatioRank2[] = { 7, 6, 5, 4, 3, 2, 1 };
#endif	// #if defined(PRE_DETAILCLEARRANK)
		// Reward item
		sprintf_s( szLabel, "_ShowBoxCount%d", nTreasureRatioRank[vInfo[itr].cRank] );
		int nRewardItemCount = pSox->GetFieldFromLablePtr( nDungeonClearID, szLabel )->GetInteger();
		sprintf_s( szLabel, "_SelectBoxCount%d", nTreasureRatioRank[vInfo[itr].cRank] );
		int nSelectItemCount = pSox->GetFieldFromLablePtr( nDungeonClearID, szLabel )->GetInteger();

		if( vPartyStruct[itr] && vPartyStruct[itr]->pSession )
		{
			bool bCheckTable = false;
#if defined( PRE_ADD_VIP_CLEAR_BOX )
			if( vPartyStruct[itr]->pSession->IsVIP() )
				bCheckTable = true;
#endif // #if defined( PRE_ADD_VIP_CLEAR_BOX )
			if (g_pDataManager->GetPCBangClearBoxFlag(vPartyStruct[itr]->pSession->GetPCBangGrade()))
				bCheckTable = true;
#if defined( PRE_ADD_NEWCOMEBACK )
			if( vPartyStruct[itr]->pSession->GetComebackAppellation() )
				bCheckTable = true;			
#endif
			
			if( bCheckTable == true )
			{
				int nVipBonusItemCount = pMapSox->GetFieldFromLablePtr(m_nMapTableID, "_VipClear")->GetInteger();
				if (nVipBonusItemCount > 0)
					nSelectItemCount = nVipBonusItemCount;
			}
		}

		vInfo[itr].cShowBoxCount = (char)nRewardItemCount;
		vInfo[itr].cSelectBoxCount = (char)nSelectItemCount;

		// Get count of each box by grade.. starting from gold box backwards..
		DNVector(char) VecTreasureBoxType;
		for( char j=3; j>=0; --j ) 
		{
#if defined(PRE_DETAILCLEARRANK)
			sprintf_s( szLabel, "%sTreasureBoxCount%d", szBoxStr[j], nTreasureRatioRank2[vInfo[itr].cRank] );
#else	// #if defined(PRE_DETAILCLEARRANK)
			sprintf_s( szLabel, "%sTreasureBoxCount%d", szBoxStr[j], nTreasureRatioRank[vInfo[itr].cRank] );
#endif	// #if defined(PRE_DETAILCLEARRANK)
			int nItemCount = pSox->GetFieldFromLablePtr( nDungeonClearID, szLabel )->GetInteger();
			if( nItemCount == 0 )
				continue;
			if( static_cast<int>(VecTreasureBoxType.size()) + nItemCount > nRewardItemCount )				
				nItemCount = nRewardItemCount - static_cast<int>(VecTreasureBoxType.size());
			for( int k=0; k<nItemCount; ++k)
			{
				VecTreasureBoxType.push_back(j);
			}
			if( VecTreasureBoxType.size() == nRewardItemCount )
				break;							
		}			
		if( VecTreasureBoxType.size() != nRewardItemCount )		
		{
			//We are screwed if this happens.. (call design team)
			g_Log.Log(LogType::_ERROR, L"[%d] RequestDungeonClear() Calc TreasureBox Error ClearID:%d, RewardCount:%d, TreasureBoxCount:%d\n",
				g_Config.nManagedID, nDungeonClearID, nRewardItemCount,VecTreasureBoxType.size() );
		}
		
		// Shuffle once here... just a simple brute-force shuffle.
		int nFirstIndex = 0;
		int nSecondIndex = 0;
		char cTemp = 0;			
		for( UINT k=0; k<VecTreasureBoxType.size(); k++)
		{
			// Even if the same index comes up, well, whatever..
			nFirstIndex = _roomrand(GetRoom())%nRewardItemCount;
			nSecondIndex = _roomrand(GetRoom())%nRewardItemCount;
			cTemp = VecTreasureBoxType[nFirstIndex];
			VecTreasureBoxType[nFirstIndex] = VecTreasureBoxType[nSecondIndex];
			VecTreasureBoxType[nSecondIndex] = cTemp;
		}

		for( UINT k=0; k<VecTreasureBoxType.size(); k++ )
		{
			char cTreasureBoxType = VecTreasureBoxType[k];
			sprintf_s( szLabel, "%sItemTableID", szBoxStr[cTreasureBoxType] );
			int nDropItemTableID = pSox->GetFieldFromLablePtr( nDungeonClearID, szLabel )->GetInteger();
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

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
			if( cTreasureBoxType == 3 && vPartyStruct[itr]->pSession->GetGoldBoxItemID() > 0 )
			{
				nResultItemID = vPartyStruct[itr]->pSession->GetGoldBoxItemID();
				nResultItemCount = 1;
				g_Log.Log(LogType::_NORMAL, vPartyStruct[itr]->pSession, L"Change GoldBoxItem : %d ", nResultItemID);
			}
#endif
			// Also set in clear info.
#if defined( PRE_ADD_DUNGEONCLEARINFO )				
			PartyFirst->ClearInfo.cRewardItemType[k] = cTreasureBoxType;
			PartyFirst->ClearInfo.RewardItem[k].nItemID = nResultItemID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			vPartyStruct[itr]->pSession->GetItem()->MakeItemStruct( nResultItemID, PartyFirst->ClearInfo.RewardItem[k] ,0, nResultItemEnchantID);
#else
			vPartyStruct[itr]->pSession->GetItem()->MakeItemStruct( nResultItemID, PartyFirst->ClearInfo.RewardItem[k] );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			PartyFirst->ClearInfo.RewardItem[k].wCount = nResultItemCount;
			TItemData *pItemData = g_pDataManager->GetItemData(nResultItemID);
			if (pItemData)
			{
				if (pItemData->cReversion == ITEMREVERSION_BELONG)
					PartyFirst->ClearInfo.RewardItem[k].bSoulbound = pItemData->IsSealed ? false : true;
			}
			else
			{
				_ASSERT(0);
			}
#else
			vPartyStruct[i]->ClearInfo.cRewardItemType[k] = cTreasureBoxType;
			vPartyStruct[i]->ClearInfo.RewardItem[k].nItemID = nResultItemID;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			vPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, vPartyStruct[i]->ClearInfo.RewardItem[k], 0, nResultItemEnchantID );
#else
			vPartyStruct[i]->pSession->GetItem()->MakeItemStruct( nResultItemID, v[i]->ClearInfo.RewardItem[k] );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			vPartyStruct[i]->ClearInfo.RewardItem[k].wCount = nResultItemCount;
			TItemData *pItemData = g_pDataManager->GetItemData(nResultItemID);
			if (pItemData)
			{
				if (pItemData->cReversion == ITEMREVERSION_BELONG)
					vPartyStruct[i]->ClearInfo.RewardItem[k].bSoulbound = pItemData->IsSealed ? false : true;
			}
			else
			{
				_ASSERT(0);
			}
#endif
		}
#if defined( PRE_ADD_DUNGEONCLEARINFO )
		PartyFirst->ClearInfo.cSelectRewardItem = 0;
		PartyFirst->ClearInfo.cSelectRemainCount = nSelectItemCount;
#else
		vPartyStruct[itr]->ClearInfo.cSelectRewardItem = 0;
		vPartyStruct[itr]->ClearInfo.cSelectRemainCount = nSelectItemCount;
#endif

		pPlayer->GetUserSession()->SetLastStageClearRank(vInfo[itr].cRank);
		pPlayer->GetUserSession()->GetQuest()->OnStageClear(m_nMapTableID);
#if defined(PRE_ADD_CP_RANK)
		if( GetStageDifficulty() == Dungeon::Difficulty::Abyss )
		{
			pPlayer->GetUserSession()->GetDBConnection()->QueryAddStageClearBest(pPlayer->GetUserSession(), m_nEnterMapTableID, 
				static_cast<DBDNWorldDef::ClearGradeCode::eCode>(vInfo[itr].cRank+DBDNWorldDef::ClearGradeCode::eCode::SSS), vInfo[itr].nDefaultCP+vInfo[itr].nBonusCP, nAbyssLvlMin);		
		}
#endif //#if defined(PRE_ADD_CP_RANK)
	}

	if (CDnPartyTask::IsActive(GetRoom()))
	{
		CDnPartyTask& partyTask = CDnPartyTask::GetInstance(GetRoom());
		partyTask.HoldSharingReversionItem();
	}

	ChangeDungeonClearState( DCS_ClearResultStay );
#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
	bool CheckAbuseClearTime = false;
	if( CheckDungeonClearAbuseTime(GetRoom()->GetDungeonPlayTime(), m_nMapTableID) )
		CheckAbuseClearTime = true;
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
	// Send dungeon clear window.
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		CDNUserSession* pSession = GetUserData(i);
		if (!pSession)
			continue;
		if( pSession->bIsGMTrace() || pSession->bIsAIBot() )
			continue;
#if defined(PRE_ADD_CP_RANK)
		if( GetStageDifficulty() == Dungeon::Difficulty::Abyss )
			SendAbyssStageClearBest(pSession, GetRoom()->GetLegendClearBest(), GetRoom()->GetMonthlyClearBest());
#endif //#if defined(PRE_ADD_CP_RANK)
#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
		if( CheckAbuseClearTime )
		{
			g_Log.Log(LogType::_HACK, pSession, L"AbuseCount[StageClearTime] MapID[%d] Time[%d] Difficulty[%d]\r\n", m_nMapTableID, GetRoom()->GetDungeonPlayTime(), GetStageDifficulty());
			pSession->GetDBConnection()->QueryAddAbuseMonitor(pSession, StageClearCheckTime::AbuseCount, 0);
		}
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
		SendGameDungeonClear( pSession, nDungeonClearID, GetRoom()->GetDungeonPlayTime(), pSession->GetLevel(), pSession->GetExp(), m_bIgnoreDungeonClearRewardItem, cClearUserCount, vInfo );
	}

#if defined(PRE_ADD_68838)
	int nNeedClearItemID = 0;
	int nNeedClearItemCount = 0;
	if( pMapSox->IsExistItem( m_nEnterMapTableID ) ) 
	{
		int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( m_nEnterMapTableID, "_EnterConditionTableID" )->GetInteger();
		if( nDungeonEnterTableID > 0 ) 
		{
			DNTableFileFormat *pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
			if( pDungeonSOX->IsExistItem( nDungeonEnterTableID ) )
			{
				nNeedClearItemID = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_NeedItemID" )->GetInteger();
				//If the item type deletes the ticket after stage clear, set the values
				if(g_pDataManager->GetItemMainType(nNeedClearItemID) == ITEMTYPE_STAGE_COMPLETE_USEITEM)
					nNeedClearItemCount = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_NeedItemCount" )->GetInteger();
			}
		}
	}
#endif	// #if defined(PRE_ADD_68838)
	// Grant EXP, items, money, etc.
	for( char itr = 0; itr < cClearUserCount; ++itr )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( GetRoom(), vInfo[itr].nSessionID );
		if( !hActor ) continue;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
#if defined(PRE_ADD_68838)
		if(nNeedClearItemCount > 0)
		{	
			if( g_pDataManager->IsCashItem(nNeedClearItemID))
			{
				if(!pPlayer->GetUserSession()->GetItem()->DeleteCashInventoryByItemID(nNeedClearItemID, nNeedClearItemCount, DBDNWorldDef::UseItem::Use))
				{
					g_Log.Log(LogType::_ERROR, pPlayer->GetUserSession(), L"[%d] RequestDungeonClear() StageClear After UseItem(Cash) Error(ClearID:%d, ItemID : %d, ItemCount : %d) \n", g_Config.nManagedID, nDungeonClearID, nNeedClearItemID, nNeedClearItemCount);
					continue;
				}
			}
			else
			{
				if(!pPlayer->GetUserSession()->GetItem()->DeleteInventoryByItemID(nNeedClearItemID, nNeedClearItemCount, DBDNWorldDef::UseItem::Use))
				{
					g_Log.Log(LogType::_ERROR, pPlayer->GetUserSession(), L"[%d] RequestDungeonClear() StageClear After UseItem(Normal) Error(ClearID:%d, ItemID : %d, ItemCount : %d) \n", g_Config.nManagedID, nDungeonClearID, nNeedClearItemID, nNeedClearItemCount);
					continue;
				}
			}
			pPlayer->GetUserSession()->GetEventSystem()->OnEvent( EventSystem::OnItemUse, 1, EventSystem::ItemID, nNeedClearItemID);
		}
#endif	// #if defined(PRE_ADD_68838)
		UINT nClearEventBonus = 0;
		for (int vtr = 0; vtr < vInfo[itr].cBonusCount; ++vtr )
			nClearEventBonus += vInfo[itr].EventClearBonus[vtr].nClearEventBonusExperience;

		int iMasterExpBonus = 0;
		int nTotalExp = vInfo[itr].nRewardExperience + vInfo[itr].nMeritBonusExperience + nClearEventBonus + vInfo[itr].nPromotionExperience + iMasterExpBonus;
		TExpData ExpData;		
		ExpData.set( (float)nTotalExp );
		pPlayer->CmdAddExperience( ExpData, DBDNWorldDef::CharacterExpChangeCode::Dungeon, pPlayer->GetUserSession()->GetPartyID() );

		if( vInfo[itr].cClearRewardType == 1 ) {
			pPlayer->CmdAddCoin( vInfo[itr].nRewardGold, DBDNWorldDef::CoinChangeCode::MaxLevelExperienceTransGold, 0 );
		}

		if (0 < iRewardGP) {
			pPlayer->GetUserSession()->AddGuildPoint(GUILDPOINTTYPE_STAGE, iRewardGP);
		}

		CDNUserSession *pSession = pPlayer->GetUserSession();
		int nPetExp = static_cast<int>(nTotalExp * Pet::PetExpRate);
		if( pSession->GetItem()->IsSatietyPet() )		
			nPetExp = static_cast<int>(nPetExp*pSession->GetItem()->GetSatietyApplyExpRatio());		

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		//Increase pet EXP..
		float incExpRate = 0.0f;
		if (pPlayer && pPlayer->IsAppliedThisStateBlow(STATE_BLOW::BLOW_256))
		{
			DNVector(DnBlowHandle) vlBlows;
			pPlayer->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_256, vlBlows);
			{
				int nCount = (int)vlBlows.size();
				for( int itr = 0; itr < nCount; ++itr )
				{
					DnBlowHandle hBlow = vlBlows[itr];
					if (hBlow && hBlow->IsEnd() == false)
					{
						incExpRate += hBlow->GetFloatValue();
					}
				}
			}
		}	

		nPetExp += (int)((float)nPetExp * incExpRate);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

		pSession->GetItem()->AddPetExp(nPetExp);
		pSession->ChangeStageUserData(nPetExp);

		// Stage clear log
		BYTE cThreadID;
		CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
		if( pDBCon )
		{
#if defined( PRE_ADD_36870 )
			pDBCon->QueryAddStageClearLog( cThreadID, GetRoom()->GetWorldSetID(), pSession->GetAccountDBID(), GetRoom()->GetRoomLogIndex(), pSession->GetCharacterDBID(), pSession->GetUserJob(), pSession->GetLevel(),
				true, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(vInfo[itr].cRank+DBDNWorldDef::ClearGradeCode::eCode::SSS), vInfo[itr].nMaxComboCount, static_cast<BYTE>(vInfo[itr].nKillBossCount), 
				vInfo[itr].nRewardExperience, vInfo[itr].nMeritBonusExperience, vInfo[itr].nDefaultCP, vInfo[itr].nBonusCP, GetRoom()->GetDungeonPlayTime()/1000, GetRoom() ? GetRoom()->GetDungeonClearRound() : 0 );
#else
			pDBCon->QueryAddStageClearLog( cThreadID, GetRoom()->GetWorldSetID(), pSession->GetAccountDBID(), GetRoom()->GetRoomLogIndex(), pSession->GetCharacterDBID(), pSession->GetUserJob(), pSession->GetLevel(),
				true, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(vInfo[itr].cRank+DBDNWorldDef::ClearGradeCode::eCode::SSS), Info[itr].nMaxComboCount, static_cast<BYTE>(Info[itr].nKillBossCount), 
				Info[itr].nRewardExperience, Info[itr].nMeritBonusExperience, Info[itr].nDefaultCP, Info[itr].nBonusCP, GetRoom()->GetDungeonPlayTime()/1000 );
#endif // #if defined( PRE_ADD_36870 )
		}

#if !defined(PRE_DELETE_DUNGEONCLEAR)
		// Update dungeon difficulty
		if( GetStageDifficulty() > 0 && GetStageDifficulty() < 3 ) {
			pSession->UpdateDungenEnterLevel( GetStageDifficulty(), vInfo[itr].cRank );
		}
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

		EWorldEnum::MapSubTypeEnum MapSubType = (EWorldEnum::MapSubTypeEnum)pMapSox->GetFieldFromLablePtr( m_nMapTableID, "_MapSubType" )->GetInteger();
		EWorldEnum::MapSubTypeEnum TempSubType = CDnWorld::GetInstance(GetRoom()).GetMapSubType();

		if (MapSubType != TempSubType)
			g_Log.Log(LogType::_GUILDWAR, pSession, L"CDnGameTask - OnDungeonClear MapID:%d MapSubType:%d != TempSubType:%d Level:%d MapType:%d \n", 
						m_nMapTableID, MapSubType, TempSubType, GetStageDifficulty(), CDnWorld::GetInstance(GetRoom()).GetMapType());

		pSession->GetEventSystem()->OnEvent( EventSystem::OnDungeonClear, 2, 
			EventSystem::StageClearRank, vInfo[itr].cRank,
			EventSystem::MapSubType, MapSubType);

		pSession->IncreaseDungeonClearCount();
		pSession->GetEventSystem()->OnEvent( EventSystem::OnCountingDungeonClear );
	}

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		DnActorHandle hActor = GetUserData(i)->GetActorHandle();
		//CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( hActor && hActor->IsMove() && !hActor->IsDie() ) hActor->CmdStop( "Stand" );
	}

	//Increment keep count if normal clear
	if (m_bDungeonFailed == false)
	{	//If it didn't fail, it succeeded
		if (GetRoom()->GetRealUserCountWithoutGM() > 1 && GetRoom()->GetPartyUpkeepCount() < 250)		//Record up to 250 times only.
			GetRoom()->IncreasePartyUpkeepCount();
		
		GetRoom()->UpdateAppliedEventValue();
	}

	RequestDungeonClearBase();
}

int CDnGameTask::OnRecvRoomSelectRewardItem( CDNUserSession * pSession, CSSelectRewardItem *pPacket )
{
	if( m_DungeonClearState != DCS_SelectRewardItemStay ) return ERROR_NONE; // If entered in any other case it's a bad actor, but return None for now
#if defined( PRE_ADD_DUNGEONCLEARINFO )
	if( pSession==NULL )
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
	if( pStruct->ClearInfo.cSelectRemainCount == 0 ) return ERROR_NONE;

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

int CDnGameTask::OnRecvRoomIdentifyRewardItem( CDNUserSession * pSession, CSIdentifyRewardItem *pPacket )
{
	// Check if at least one exists
	if (pSession->GetItem()->GetCashItemByType(ITEMTYPE_REWARDITEM_IDENTIFY) == NULL && pSession->GetItem()->GetInventoryItemCountByType(ITEMTYPE_REWARDITEM_IDENTIFY) == 0) return ERROR_NONE;
	if (!pSession->GetItem()->UseItemByType(ITEMTYPE_REWARDITEM_IDENTIFY, 1, true)) return ERROR_ITEM_FAIL;

#if defined( PRE_ADD_DUNGEONCLEARINFO )
	CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pSession->GetCharacterDBID() );
	if( !PartyFirst )
	{
		// By design, missing case is a bug
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[GameTask::OnRecvRoomIdentifyRewardItem] PartyFirst error!!\r\n");
		return ERROR_NONE;	
	}
	SendGameIdentifyRewardItem( pSession, pPacket->cItemIndex, PartyFirst->ClearInfo.cRewardItemType[pPacket->cItemIndex] );
#else
	CDNGameRoom::PartyStruct *pStruct = GetPartyData(pSession);
	SendGameIdentifyRewardItem( pSession, pPacket->cItemIndex, pStruct->ClearInfo.cRewardItemType[pPacket->cItemIndex] );
#endif

	return ERROR_NONE;
}

void CDnGameTask::RequestDungeonFailed( bool bForce/*=false*/, bool bTimeOut/*=false*/ )
{
	if( bForce == false )
	{
		bool bAllDie = true;
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			if( !pSession || !pSession->GetActorHandle() ) continue;
			if( !pSession->GetActorHandle()->IsDie() || pSession->GetActorHandle()->GetStateBlow()->IsExistStateBlowFromBlowIndex( STATE_BLOW::BLOW_057 ) ) {
				bAllDie = false;
				break;
			}
		}
		if( !bAllDie ) 
			return;
	}

	if( m_bDungeonFailed == true )
		return;

	m_bDungeonFailed = true;
#if defined(PRE_FIX_69108)
	//Process ranking update
	if( GetGameTaskType() == GameTaskType::DarkLair && CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeDungeon )
	{
		GetRoom()->AddDungeonPlayTime( timeGetTime() - GetStageCreateTime() );
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
			if( pStruct == NULL ) continue;
			((CDNDLGameRoom*)GetRoom())->SetUpdateRankData( pStruct->pSession );
		}
		((CDNDLGameRoom*)GetRoom())->RequestRankQueryData();
	}
#endif
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pSession = GetUserData(i);
		if( !pSession ) continue; 
		SendGameDungeonFailed( pSession, ( m_nReturnDungeonClearWorldID == -1 ) ? false : true, bTimeOut );
		pSession->GetEventSystem()->OnEvent( EventSystem::OnDungeonGiveup );
	}
}

int CDnGameTask::OnRecvNpcTalkMsg( CDNUserSession * pSession, CSNpcTalk *pPacket, int nLen )
{

	DnActorHandle hNPC = CDnActor::FindActorFromUniqueID( GetRoom(), pPacket->nNpcUniqueID );

	if ( !hNPC )
	{
		std::wstring wszLog ;
		wszLog = FormatW( L"NPC not found RoomID:%d nNpcUniqueID:%d \n", GetRoom()->GetRoomID(), pPacket->nNpcUniqueID);
		g_Log.Log(LogType::_ERROR, pSession, L"%s", wszLog.c_str() );
		pSession->SendDebugChat(wszLog.c_str());
		pSession->SendNextTalkError();
		return ERROR_NONE;
	}

	std::wstring wszIndex;
	std::wstring wszTarget;

	bool bResult = g_pDataManager->GetTalkIndexName(pPacket->dwIndexHashCode, wszIndex);
	if ( !bResult )
	{
		g_Log.Log(LogType::_ERROR, pSession, L"GetTalkIndexName Failed HashCode : %u \n", pPacket->dwIndexHashCode);
		pSession->SendNextTalkError();
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	bResult =  g_pDataManager->GetTalkFileName(pPacket->dwTargetHashCode, wszTarget);
	if ( !bResult )
	{
		g_Log.Log(LogType::_ERROR, pSession, L"GetTalkFileName Failed HashCode : %u \n", pPacket->dwTargetHashCode);
		pSession->SendNextTalkError();
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if ( hNPC->IsNpcActor() )
	{
		CDnNPCActor* pNpc = static_cast<CDnNPCActor*>(hNPC.GetPointer());
		_ASSERT( pSession->GetSessionID() == (pSession->GetActorHandle() ? pSession->GetActorHandle()->GetUniqueID() : -1 ));

		if (0 != pSession->m_dwNpcTalkLastIndexHashCode && ::wcscmp(wszIndex.c_str(), L"start") && pPacket->dwIndexHashCode == pSession->m_dwNpcTalkLastIndexHashCode && pPacket->dwTargetHashCode == pSession->m_dwNpcTalkLastTargetHashCode)
		{
			pSession->SendNextTalkError();
			return ERROR_GENERIC_INVALIDREQUEST;
		}
		pSession->m_dwNpcTalkLastIndexHashCode = pPacket->dwIndexHashCode;
		pSession->m_dwNpcTalkLastTargetHashCode = pPacket->dwTargetHashCode;

		TALK_PARAGRAPH& para = pSession->GetLastTalkParagraph();

		// When starting a conversation with an NPC, reset remembered previous dialogue.
		if( ::wcscmp(wszIndex.c_str(), NpcTalkStringGroup[NPCTALK_START].c_str()) == 0 )
#if defined(PRE_FIX_QUEST_GUILD_BOARD)
		{
			para = g_pDataManager->GetEmptyTalkParagraph();
			pSession->GetQuest()->ResetRewardFlag();
		}
#else
			para = g_pDataManager->GetEmptyTalkParagraph();
#endif
		bool bValidAnswer = false;

		// Check for data that bypasses validation
		for (int i = NPCTALK_START; i<NPCTALK_END; i++)
		{
			if( ::wcscmp(wszIndex.c_str(), NpcTalkStringGroup[i].c_str()) == 0)
				bValidAnswer = true;
		}

		if (bValidAnswer == false)
		{
			if( para.Answers.size() )
			{
				for( std::vector<TALK_ANSWER>::iterator i = para.Answers.begin(); i != para.Answers.end(); i++ )
				{
					if( (*i).szLinkIndex == wszIndex )
					{
						bValidAnswer = true;
						break;
					}
				}
			}
#if !defined(PRE_FIX_QUEST_GUILD_BOARD)	//Due to the statement below, a dead-end dialogue with no choices passes to another context without validation.
			else
				bValidAnswer = true;
#endif
		}

		if( bValidAnswer == false )
		{
#if !defined (_FINAL_BUILD)
			g_Log.Log(LogType::_ERROR, pSession, L"[CS_NPCTALK] PAK:%d/%d/%d INDEX:%s\r\n", pPacket->nNpcUniqueID, pPacket->dwIndexHashCode, pPacket->dwTargetHashCode, wszIndex);
#endif 
			pSession->SendNextTalkError();
			return ERROR_GENERIC_INVALIDREQUEST;
		}

		pSession->m_nClickedNpcID = pNpc->GetNpcData()->nNpcID;	// Update on every click
		pSession->m_nClickedNpcObjectID = pPacket->nNpcUniqueID;

		if (!pSession->IsValidPacketByNpcClick()){	// Check NPC distance
			pSession->SendNextTalkError();
			return ERROR_GENERIC_INVALIDREQUEST;
		}

		bool bLocalResult = pNpc->Talk(pSession, hNPC->GetUniqueID(), wszIndex, wszTarget);
		if ( !bLocalResult )
		{
			pSession->SendNextTalkError();
			return ERROR_GENERIC_INVALIDREQUEST;
		}

		if( ::wcscmp(wszIndex.c_str(), NpcTalkStringGroup[NPCTALK_START].c_str()) == 0 )
		{
			// Trigger Event
			if( m_pWorld ) 
			{
				m_pWorld->InsertTriggerEventStore( "LastTalkNpc", ( hNPC ) ? hNPC->GetUniqueID() : -1 );
				m_pWorld->InsertTriggerEventStore( "LastTalkActor", ( pSession->GetActorHandle() ) ? pSession->GetActorHandle()->GetUniqueID() : -1 );
				m_pWorld->OnTriggerEventCallback( "CDnGameTask::OnRecvNpcTalkMsg", m_LocalTime, m_fDelta );
			}

			pSession->GetEventSystem()->OnEvent( EventSystem::OnNpcTalk, 1, EventSystem::NpcID, pSession->m_nClickedNpcID );
		}

		if (!pSession->IsNpcTalk()) {
			pSession->SendTalkEnd(hNPC->GetUniqueID());			// When the client receives this packet, the existing NPC dialogue window closes
		}

		return ERROR_NONE;
	}	
	else
	{
		pSession->SendNextTalkError();
		_DANGER_POINT();
	}
	return ERROR_NONE;
}

int CDnGameTask::OnRecvNpcTalkEndMsg( CDNUserSession * pSession, CSNpcTalkEnd *pPacket, int nLen )
{
#if defined(PRE_FIX_QUEST_GUILD_BOARD)
	pSession->GetQuest()->ResetRewardFlag();
#endif
	DnActorHandle hNPC = CDnActor::FindActorFromUniqueID( GetRoom(), pPacket->nNpcUniqueID );

	if( !hNPC ) {
		return ERROR_NONE;
	}

	// Trigger Event
	if( m_pWorld ) {
		m_pWorld->InsertTriggerEventStore( "LastTalkNpc", ( hNPC ) ? hNPC->GetUniqueID() : -1 );
		m_pWorld->InsertTriggerEventStore( "LastTalkActor", ( pSession->GetActorHandle() ) ? pSession->GetActorHandle()->GetUniqueID() : -1 );
		m_pWorld->OnTriggerEventCallback( "CDnGameTask::OnRecvNpcTalkEndMsg", m_LocalTime, m_fDelta );
	}
	return ERROR_NONE;
}

int CDnGameTask::OnRecvChatChatMsg( CDNUserSession * pSession, CSChat *pPacket, int nLen )
{
	int nResult = -2;

	if (sizeof(CSChat) - sizeof(pPacket->wszChatMsg) + (pPacket->nLen * sizeof(WCHAR)) != nLen )
		return ERROR_INVALIDPACKET;

	if (pPacket->nLen > CHATLENMAX-1)	return ERROR_NONE;
	WCHAR wszChatMsg[CHATLENMAX] = { 0, };
	_wcscpy(wszChatMsg, CHATLENMAX, pPacket->wszChatMsg, pPacket->nLen);

//	std::wstring WChatMsg = wszChatMsg;
	if (pSession->GetAccountLevel() >= AccountLevel_New && pSession->GetAccountLevel() <= AccountLevel_Developer)
	{
		wstring WChatMsg = wszChatMsg;
		if (WChatMsg.find(L"/") == 0){
			if (pSession->GetAccountLevel() == AccountLevel_QA){	// Allow cheats for QA
				if (pSession->GetCheatCommand()){
					if (pSession->GetCheatCommand()->Command(WChatMsg) > 0) return ERROR_NONE;
				}
			}
			if (pSession->GetGMCommand()){ 
				if (pSession->GetGMCommand()->GMCommand(WChatMsg)) return ERROR_NONE;
			}

			return ERROR_NONE;
		}
	}
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	if( pSession->GetSpammer() ) // Skip everything for spammers...
		return ERROR_NONE;	
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
	if( pSession->IsMuteChatting()) //Skip if user is chat-banned
		return ERROR_NONE;
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
	switch (pPacket->eType)
	{
		case CHATTYPE_NORMAL:
		{
			// Handled on client side by default, so server finishes without sending a response.
			if( pSession->CheckPapering(CDNUserBase::eChatPapering) )
				return ERROR_NONE;	
		}
		case CHATTYPE_PARTY:
		case CHATTYPE_RAIDNOTICE:
		{
			for( DWORD i=0; i<GetUserCount(); i++ ) 
			{
				CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
				if (!pStruct) continue;

				if ((pSession->GetAccountLevel() >= AccountLevel_New) && (pSession->GetAccountLevel() <= AccountLevel_QA))
				{					
					if (pPacket->eType == CHATTYPE_NORMAL) pPacket->eType = CHATTYPE_GM;
					else if (pPacket->eType == CHATTYPE_PARTY) pPacket->eType = CHATTYPE_PARTY_GM;				
				}				

				pStruct->pSession->SendChat(pPacket->eType, pPacket->nLen, pSession->GetCharacterName(), wszChatMsg);
			}
#ifdef IRINE_MOD_PLAYER_BOTS
			if( !pSession->bIsAIBot() && GetRoom() && GetRoom()->HasAIBots() &&
				( pPacket->eType == CHATTYPE_PARTY || pPacket->eType == CHATTYPE_PARTY_GM ||
				  pPacket->eType == CHATTYPE_NORMAL || pPacket->eType == CHATTYPE_GM ) )
			{
				CDnAIBotController::HandlePartyChatCommand( GetRoom(), pSession, wszChatMsg );
			}
#endif
			break;
		}

		case CHATTYPE_GUILD:
		{
			const TGuildUID GuildUID(pSession->GetGuildUID());
			if (!GuildUID.IsSet()) {
				break;
			}
			DN_ASSERT(pSession->GetWorldSetID() == GuildUID.nWorldID,	"Check!");

			CDNGuildBase* pGuild = g_pGuildManager->At (GuildUID);

			if (!pGuild) 
				break;

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
			if (FALSE == pGuild->IsEnable()) break;
#endif

			g_pMasterConnectionManager->SendGuildChat(pSession->GetWorldSetID(), GuildUID, pSession->GetAccountDBID(), pSession->GetCharacterDBID(), pPacket->wszChatMsg, pPacket->nLen);

			break;
		}

		// Team chat
		case CHATTYPE_TEAMCHAT:
		{
			int iTeam = pSession->GetTeam();
			
			for( UINT i=0 ; i<GetUserCount() ; ++i )
			{
				CDNGameRoom::PartyStruct* pStruct = GetPartyData(i);
				if( pStruct->pSession && pStruct->pSession->GetTeam() == iTeam )
					pStruct->pSession->SendChat( pPacket->eType, pPacket->nLen, pSession->GetCharacterName(), wszChatMsg );
			}
			break;
		}

		case CHATTYPE_WORLD:
#if defined(PRE_ADD_WORLD_MSG_RED)
		case CHATTYPE_WORLD_POPMSG:
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)
		{
			//Supposedly enables chatting via an item. When actually applied, proceed with the flow after checking item ownership or usage
			//Just check if item exists or can be used/fired and immediately send to Master; Master broadcasts
			if (g_pMasterConnectionManager)
			{
				int nItemID = 0;
#if defined(PRE_ADD_WORLD_MSG_RED)
				if(!pSession->GetItem()->IsVaildWorldChatItem(pPacket->eType, pPacket->biItemSerial, nItemID))
				{
					g_Log.Log(LogType::_ERROR, pSession, L"[OnRecvChatChatMsg] WorldChat Item Find Failed(ChatType : %d, ItemID : %d, Serial: %I64d\r\n", pPacket->eType, pPacket->biItemSerial);
					break;
				}
#else	// #if defined(PRE_ADD_WORLD_MSG_RED)
				const TItem *pItem = pSession->GetItem()->GetCashInventory(pPacket->biItemSerial);
				if (pItem) nItemID = pItem->nItemID;
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)

				if (pSession->GetItem()->DeleteItemByUse(ITEMPOSITION_CASHINVEN, -1, pPacket->biItemSerial)){
					g_pMasterConnectionManager->SendChat(pSession->GetWorldSetID(), pPacket->eType, pSession->GetAccountDBID(), wszChatMsg, pPacket->nLen);
					pSession->GetEventSystem()->OnEvent( EventSystem::OnItemUse, 1, EventSystem::ItemID, nItemID );
				}
			}
			break;
		}
#if defined( PRE_PRIVATECHAT_CHANNEL )
		case CHATTYPE_PRIVATECHANNEL:
			{					
				if (g_pMasterConnectionManager) 
				{
					if(pSession->GetPrivateChannelID())
						g_pMasterConnectionManager->SendPrivateChannelChat(pSession->GetWorldSetID(), pPacket->eType, pSession->GetAccountDBID(), wszChatMsg, pPacket->nLen, pSession->GetPrivateChannelID());
				}
			}
			break;
#endif
		default:
		{
			return ERROR_GENERIC_INVALIDREQUEST;
		}
	}

	return ERROR_NONE;;
}

int CDnGameTask::OnRecvChatChatRoomMsg( CDNUserSession * pSession, CSChatChatRoom *pChatChatRoom, int nLen )

{
	if (sizeof(CSChatChatRoom) - sizeof(pChatChatRoom->wszChatMsg) + (pChatChatRoom->nLen * sizeof(WCHAR)) != nLen)
		return ERROR_INVALIDPACKET;

	if (pChatChatRoom->nLen > CHATLENMAX-1)	return ERROR_NONE;
	WCHAR wszChatMsg[CHATLENMAX] = { 0, };
	_wcscpy(wszChatMsg, CHATLENMAX, pChatChatRoom->wszChatMsg, pChatChatRoom->nLen);

	if( pSession->m_nChatRoomID  <= 0 )	// User is not in the chat room.
	{
		return ERROR_CHATROOM_UNKNOWNUSER;
	}

	CDNChatRoom * pChatRoom = pSession->GetGameRoom()->m_ChatRoomManager.GetChatRoom( pSession->m_nChatRoomID );
	if( pChatRoom )
	{
		pChatRoom->SendChatMessageToAll( pChatChatRoom->nLen, pSession->GetCharacterName(), wszChatMsg );
	}

	return ERROR_NONE;
}
#if defined (PRE_ADD_ANTI_CHAT_SPAM)
int CDnGameTask::OnRecvChatSpammer( CDNUserSession * pSession, bool* bSpammer )
{
	pSession->SetSpammer(*bSpammer);
	if( *bSpammer )
	{
		int iInvalidCount = 1;
		g_Log.Log( LogType::_HACK, pSession, L"HackChecker(Spammer) : CharName=%s Counter=%d\n", pSession->GetCharacterName(), iInvalidCount );
		pSession->GetDBConnection()->QueryAddAbuseMonitor( pSession, 0, iInvalidCount );
	}
	return ERROR_NONE;
}
#endif
int CDnGameTask::OnRecvChatPrivateMsg( CDNUserSession * pSession, CSChatPrivate *pPacket, int nLen )
{
	if (pPacket->nLen > CHATLENMAX-1)	return ERROR_NONE;

	if (sizeof(CSChatPrivate) - sizeof(pPacket->wszChatMsg) + (pPacket->nLen * sizeof(WCHAR)) != nLen)
		return ERROR_INVALIDPACKET;

	WCHAR wszChatMsg[CHATLENMAX] = { 0, };
	_wcscpy(wszChatMsg, CHATLENMAX, pPacket->wszChatMsg, pPacket->nLen);

	if (wcscmp(pPacket->wszToCharacterName, pSession->GetCharacterName()) == 0){		// Compare names and return if sent to self
		pSession->SendChat(CHATTYPE_PRIVATE, pPacket->nLen, pPacket->wszToCharacterName, wszChatMsg, NULL, ERROR_CHAT_DONTSAYYOURSELF);		// Self
		return ERROR_NONE;
	}

	if ( pSession->GetIsolate() && pSession->GetIsolate()->IsIsolateItem(pPacket->wszToCharacterName))
	{
		pSession->SendIsolateResult(ERROR_ISOLATE_REQUESTFAIL);
		return ERROR_NONE;
	}

	int nChatType = CHATTYPE_PRIVATE;
	if ((pSession->GetAccountLevel() >= AccountLevel_New) && (pSession->GetAccountLevel() <= AccountLevel_QA)) nChatType = CHATTYPE_PRIVATE_GM;

	CDNGameRoom::PartyStruct *pStruct = GetPartyData(pPacket->wszToCharacterName);
	if( pStruct ) {	// If in the same party
		// If GM, treat as if not present
		if ((pStruct->pSession->GetAccountLevel() >= AccountLevel_New) && (pStruct->pSession->GetAccountLevel() <= AccountLevel_QA)){
			if (!pStruct->pSession->GetGMCommand()->IsWhisperName(pSession->GetCharacterName())){
				pStruct->pSession->SendChat(CHATTYPE_PRIVATE, pPacket->nLen, pSession->GetCharacterName(), wszChatMsg, NULL, ERROR_CHAT_USERNOTFOUND);
				return ERROR_NONE;
			}
		}

		// Handled on client side by default, so server finishes without sending a response.
		if( pSession->CheckPapering(CDNUserBase::eChatPapering) )
			return ERROR_NONE;

		pStruct->pSession->SendChat(nChatType, pPacket->nLen, pSession->GetCharacterName(), wszChatMsg);
	}	
	else {	// Otherwise

		if( pSession->CheckPapering(CDNUserBase::eChatPapering) )
			return ERROR_NONE;
#ifdef PRE_ADD_DOORS
		INT64 biDoorsDestChracterDBID = 0;
		if (pSession->GetFriend())
		{
			TFriend * pFriend = pSession->GetFriend()->GetFriend(pPacket->wszToCharacterName);
			if (pFriend)
				//if (pFriend && pFriend->bMobileAuthentication)
				biDoorsDestChracterDBID = pFriend->biFriendCharacterDBID;
		}
		if (g_pMasterConnectionManager->SendPrivateChat(pSession->GetWorldSetID(), pSession->GetAccountDBID(), pPacket->wszToCharacterName, nChatType, wszChatMsg, pPacket->nLen, biDoorsDestChracterDBID) == false)
			pSession->SendChat(CHATTYPE_PRIVATE, pPacket->nLen, pPacket->wszToCharacterName, wszChatMsg, NULL, ERROR_CHAT_USERNOTFOUND);
#else		//#ifdef PRE_ADD_DOORS
		if (g_pMasterConnectionManager->SendPrivateChat(pSession->GetWorldSetID(), pSession->GetAccountDBID(), pPacket->wszToCharacterName, nChatType, wszChatMsg, pPacket->nLen) == false)
			pSession->SendChat(CHATTYPE_PRIVATE, pPacket->nLen, pPacket->wszToCharacterName, wszChatMsg, NULL, ERROR_CHAT_USERNOTFOUND);
#endif		//#ifdef PRE_ADD_DOORS
	}

	return ERROR_NONE;
}

int CDnGameTask::OnRecvDiceMsg( CDNUserSession * pSession, CSDice *pPacket, int nLen )
{
	return ERROR_NONE;
}

void CDnGameTask::OnRecvVillageState( CDNUserSession *pSession, MAGAVillageState *pPacket )
{
	switch( pPacket->nRet ) {
		case ERROR_NONE:
			{
				ResetDungeonClear( false );

				int nLastVillageIndex = pSession->GetLastVillageMapIndex();
				if (pSession->GetLastSubVillageMapIndex() > 0) 
					nLastVillageIndex = pSession->GetLastSubVillageMapIndex();

#if defined( PRE_PARTY_DB )
				nLastVillageIndex = pPacket->nMapIndex;
#endif

				g_pMasterConnectionManager->SendRequestNextVillageInfo( pSession->GetWorldSetID(), nLastVillageIndex, -1, true, pSession->GetGameRoom() );

				// Send party leave message to the GM intrusion Session.
				if( GetRoom() && GetRoom()->GetTaskMng() )
				{
					for( UINT i=0 ; i<GetUserCount() ; ++i )
					{
						CDNUserSession* pSession = GetUserData(i);
						if( pSession && pSession->bIsGMTrace() == false )
						{
							for( UINT j=0 ; j<GetUserCount() ; ++j )
							{
								CDNUserSession* pGMSession = GetUserData(j);
								if( pGMSession && pGMSession->bIsGMTrace() == true )
								{
									if( GetRoom()  && GetRoom()->GetTaskMng() )
									{
										CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetRoom()->GetTaskMng()->GetTask("PartyTask"));
										if( pPartyTask )
											pPartyTask->OutPartyMember( pSession->GetSessionID(), -1, -1);
									}
								}
							}
						}
					}
				}
			}
			break;
		default:
			{
				// Restore HP and SP. Might be good to handle reward state effect logic here later too.
				for( DWORD i=0; i<GetUserCount(); i++ ) {
					CDNUserSession *pLocalSession = GetUserData(i);
					if( !pLocalSession ) continue;
					DnActorHandle hActor = pLocalSession->GetActorHandle();
					if( !hActor ) continue;
					CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
					pPlayer->CmdRefreshHPSP( hActor->GetMaxHP(), hActor->GetMaxSP() );
					pPlayer->CmdRemoveStateEffect(STATE_BLOW::BLOW_099);
					pLocalSession->SendVillageInfo( "", 0, ERROR_GENERIC_VILLAGECON_NOT_FOUND, 0, 0);
				}

				// If mission Failed and not a dungeon, it is -1. Check and set it.
				if( m_nReturnDungeonClearWorldID == -1 ) {
					m_nReturnDungeonClearWorldID = m_nMapTableID;
					m_nReturnDungeonClearStartPositionIndex = m_nStartPositionIndex;
				}
#if defined(PRE_FIX_INITSTATEANDSYNC)
				GetRoom()->ReserveInitStateAndSync( m_nReturnDungeonClearWorldID, m_nReturnDungeonClearStartPositionIndex, CRandom::Seed(GetRoom()), Dungeon::Difficulty::Easy, true );
#else
				GetRoom()->InitStateAndSync( m_nReturnDungeonClearWorldID, m_nReturnDungeonClearStartPositionIndex, CRandom::Seed(GetRoom()), Dungeon::Difficulty::Easy, true );
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
			}
			break;
	}
}

CDNUserSession* CDnGameTask::GetPartyLeaderSession()
{
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pSession = GetUserData(i);
		if( GetPartyData(i)->bLeader ) 
			return pSession;
	}

	return NULL;
}

void CDnGameTask::EnableDungeonClearWarpQuestFlag()
{
	m_bDungeonClearQuestComplete = true;
}

int CDnGameTask::OnRecvRoomWarpDungeon( CDNUserSession * pSession, CSWarpDungeon *pPacket )
{
#if defined( PRE_WORLDCOMBINE_PARTY )
	if( GetRoom()->bIsWorldCombineParty() && pPacket->bWarpVillage == false )
		return ERROR_NONE;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

#if defined( PRE_PARTY_DB )
	int nLevelCheckLastVillageIndex = 0;
	if( pPacket->bWarpVillage )
	{
		CDNUserSession* LeaderSession = GetPartyLeaderSession();
		if( LeaderSession )
		{
			nLevelCheckLastVillageIndex = LeaderSession->GetLastVillageMapIndex();
			int nPerminLevel = g_pDataManager->GetMapPermitLevel(nLevelCheckLastVillageIndex);		
			for( DWORD i=0 ; i<GetUserCount() ; ++i )
			{			
				CDNUserSession *pCheckSession = GetUserData(i);
				if( !pCheckSession ) 
					continue;
				if(pCheckSession->GetLevel() < nPerminLevel)
				{
					nLevelCheckLastVillageIndex = pCheckSession->GetLastVillageMapIndex();
					nPerminLevel = g_pDataManager->GetMapPermitLevel(nLevelCheckLastVillageIndex);
				}
			}
		}		
	}
#endif

	if (GetRoom()->GetOutMemberExist())
	{
		_DANGER_POINT_MSG(L"if (GetRoom()->GetOutMemberExist())");
		m_sWarpDungeonStruct.bActivated = true;
		m_sWarpDungeonStruct.sWarp = *pPacket;
		return ERROR_NONE;
	}

	// Bad actor if returning here.
	if( !pPacket->bWarpVillage ) {
		if( !m_bEnteredDungeon ) return ERROR_GENERIC_INVALIDREQUEST;
		if( m_nReturnDungeonClearWorldID == -1 || m_nReturnDungeonClearStartPositionIndex < 1 ) {
			DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDUNGEONCLEAR );

			char szLabel[64];
			static char *szStaticLabel[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare", "Chaos", "Hell" };
			sprintf_s( szLabel, "_ClearDungeonTableID_%s", szStaticLabel[m_StageDifficulty] );
			int nDungeonClearID = pMapSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();
			if( nDungeonClearID ) {
				int nReturnWorldID = pSox->GetFieldFromLablePtr( nDungeonClearID, "_ReturnWorldID" )->GetInteger();
				int nReturnWorldStartPos = pSox->GetFieldFromLablePtr( nDungeonClearID, "_ReturnWorldStartPos" )->GetInteger();

				if( nReturnWorldID > 0 ) {
					m_nReturnDungeonClearWorldID = nReturnWorldID;
					m_nReturnDungeonClearStartPositionIndex = nReturnWorldStartPos;
				}
			}
			if( m_nReturnDungeonClearWorldID == -1 || m_nReturnDungeonClearStartPositionIndex < 1 )
				return ERROR_GENERIC_INVALIDREQUEST;
		}
	}

	bool bWarpVillage = pPacket->bWarpVillage;
	if (GetRoom()->GetGameServer() && GetRoom()->GetGameServer()->m_bZeroPopulation)
		bWarpVillage = true;

	if (GetDungeonClearState() == DCS_WarpStandBy)
	{
		CDNUserSession* pLeaderSession = GetPartyLeaderSession();
		if (pLeaderSession && pLeaderSession == pSession)
		{
			if (CDnPartyTask::IsActive(GetRoom()))
			{
				int errCode = ERROR_NONE;
				if (CDnPartyTask::GetInstance(GetRoom()).IsPartySharingReversionItem())
					errCode = ERROR_PARTY_DUNGEONWARPOUT_FAIL_ITEMSHARING;
				else if (m_bDungeonClearQuestComplete == false)
					errCode = ERROR_PARTY_DUNGEONWARPOUT_FAIL_QUEST;

				if (errCode != ERROR_NONE)
				{
					SendGameDungeonWarpOutFailed(pLeaderSession, errCode);
					return errCode;
				}
			}
		}
	}

	// If stage start log exists...
	if( GetRoom()->GetStageStartLogFlag() )
	{
		// Stage end log
		BYTE cThreadID;
		CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
		if( pDBCon )
		{
			pDBCon->QueryAddStageEndLog( cThreadID, GetRoom()->GetWorldSetID(), 0, GetRoom()->GetRoomLogIndex(), pPacket->bWarpVillage ? DBDNWorldDef::WhereToGoCode::Village : DBDNWorldDef::WhereToGoCode::StageEntrance );
			GetRoom()->SetStageEndLogFlag( true );			
		}
	}

#if defined( PRE_ADD_58761 )
	if( GetRoom()->GetDungeonGateID() > 0 )
		GetRoom()->EndDungeonGateTime( GetRoom()->GetDungeonGateID() );
#endif

	if( bWarpVillage ) 
	{
#if defined( PRE_WORLDCOMBINE_PARTY )
		if( GetRoom()->bIsWorldCombineParty() )
		{
			for( DWORD i=0 ; i<GetUserCount() ; ++i )
			{
				CDNUserSession *pLocalSession = GetUserData(i);
				if( !pLocalSession ) 
					continue;
				pLocalSession->CmdAbandonStage( false, true, false );
			}
		}
		else
		{
			int nLastVillageIndex = pSession->GetLastVillageMapIndex();
			nLastVillageIndex = nLevelCheckLastVillageIndex;
			g_pMasterConnectionManager->SendVillageState(pSession->GetWorldSetID(), pSession->GetAccountDBID(), nLastVillageIndex, GetRoom()->GetRoomID());
		}
#else
		int nLastVillageIndex = pSession->GetLastVillageMapIndex();
#if defined( PRE_PARTY_DB )		
		nLastVillageIndex = nLevelCheckLastVillageIndex;
#endif
		g_pMasterConnectionManager->SendVillageState(pSession->GetWorldSetID(), pSession->GetAccountDBID(), nLastVillageIndex, GetRoom()->GetRoomID());
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	}
	else 
	{
		if( GetRoom() )
			GetRoom()->DelBackupPartyInfo();

		// Restore HP and SP. Might be good to handle reward state effect logic here later too.
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pLocalSession = GetUserData(i);
			if( !pLocalSession ) continue;

			DnActorHandle hActor = pLocalSession->GetActorHandle();
			if( !hActor ) continue;
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
			pPlayer->CmdRefreshHPSP( hActor->GetMaxHP(), hActor->GetMaxSP() );
			pPlayer->CmdRemoveStateEffect(STATE_BLOW::BLOW_099);
#if defined(PRE_FIX_71954)
			//Unequip temporary items here.			
			pLocalSession->GetItem()->RemoveInstantEquipVehicleData(true);
			pLocalSession->GetItem()->RemoveInstantVehicleItemData(true);
#endif // #if defined(PRE_FIX_71954)
		}
#if defined(PRE_FIX_INITSTATEANDSYNC)
		GetRoom()->ReserveInitStateAndSync( m_nReturnDungeonClearWorldID, m_nReturnDungeonClearStartPositionIndex, CRandom::Seed(GetRoom()), Dungeon::Difficulty::Easy, true );
#else
		GetRoom()->InitStateAndSync( m_nReturnDungeonClearWorldID, m_nReturnDungeonClearStartPositionIndex, CRandom::Seed(GetRoom()), Dungeon::Difficulty::Easy, true );
#endif //#if defined(PRE_FIX_INITSTATEANDSYNC)
	}

	return ERROR_NONE;
}

int CDnGameTask::OnRecvRoomRequestDungeonFailed( CDNUserSession *pSession, char *pPacket )
{
	RequestDungeonFailed();
	return ERROR_NONE;
}

int CDnGameTask::OnRecvQuestCompleteCutScene( CDNUserSession * pSession, char *pPacket )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor ) return ERROR_NONE;

	CDnPlayerActor *pPlayer = (CDnPlayerActor*)hActor.GetPointer();

	if( pPlayer ) pPlayer->SetCompleteCutScene( true );
	CheckAndSyncCutScenePlay();

	return ERROR_NONE;
}

int CDnGameTask::OnRecvQuestSkipCutScene( CDNUserSession * pSession, char *pPacket )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor ) return ERROR_NONE;

	CDnPlayerActor *pPlayer = (CDnPlayerActor*)hActor.GetPointer();

	if( pPlayer ) pPlayer->SetSkipCutScene( true );
	CheckAndSyncCutSceneSkip( pSession );

	return ERROR_NONE;
}

int CDnGameTask::OnRecvSelectAppellation( CDNUserSession *pSession, CSSelectAppellation *pPacket )
{
	if( pSession->GetAppellation()->OnRecvSelectAppellation( pPacket->nArrayIndex, pPacket->nCoverArrayIndex ) == true ) {
		DnActorHandle hActor = pSession->GetActorHandle();
		if( !hActor ) return ERROR_NONE;
		CDnPlayerActor *pPlayer = (CDnPlayerActor*)hActor.GetPointer();
		pPlayer->ChangeAppellationState( pPacket->nCoverArrayIndex, pPacket->nArrayIndex );
		return ERROR_NONE;
	}
	return ERROR_GENERIC_UNKNOWNERROR;
}

int CDnGameTask::OnRecvCollectionBook( CDNUserSession *pSession, CSCollectionBook *pPacket )
{
	if (pSession->GetAppellation()->OnRecvCollectionBook(pPacket->nCollectionID) == true)
		return ERROR_NONE;

	return ERROR_GENERIC_UNKNOWNERROR;
}

void CDnGameTask::RefreshDungeonClearState()
{
	if( m_DungeonClearState == DCS_None ) return;

	switch( m_DungeonClearState ) {
		case DCS_WarpDungeonStay:
			{
				SendGameWarpDungeonClearToLeader();
			}
			break;
	}
}

bool CDnGameTask::CheckAndSyncCutScenePlay()
{
#ifdef STRESS_TEST
	return true;
#endif
	DnActorHandle hActor;
	CDnPlayerActor *pPlayer;
	bool bAllComplete = true;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if( GetUserData(i)->bIsGMTrace() || GetUserData(i)->bIsAIBot() ) continue;
		hActor = GetUserData(i)->GetActorHandle();
		if( !hActor ) continue;

		pPlayer = (CDnPlayerActor*)hActor.GetPointer();
		if( pPlayer->IsCheckCompleteCutScene() == false )
			continue;
		if( !pPlayer->IsCompleteCutScene() ) {
			bAllComplete = false;
			break;
		}
	}
	if( bAllComplete ) {
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			hActor = GetUserData(i)->GetActorHandle();
			if( !hActor ) continue;

			pPlayer = (CDnPlayerActor*)hActor.GetPointer();
			pPlayer->SetCompleteCutScene( false );
			pPlayer->SetSkipCutScene( false );

			GetUserData(i)->SendCompleteCutScene( m_bCutSceneAutoFadeIn );
		}
		m_bWaitPlayCutScene = false;

		if (CDnPartyTask::IsActive(GetRoom()))
		{
			CDnPartyTask& partyTask = CDnPartyTask::GetInstance(GetRoom());
			partyTask.ReleaseSharingReversionItem();
		}
	}

	return bAllComplete;
}

void CDnGameTask::CheckAndSyncCutSceneSkip( CDNUserSession *pSession )
{
	DnActorHandle hActor;
	CDnPlayerActor *pPlayer;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		GetUserData(i)->SendSkipCutScene( pSession->GetSessionID() );
	}

	bool bAllSkip = true;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		if( GetUserData(i)->bIsGMTrace() || GetUserData(i)->bIsAIBot() ) continue;
		hActor = GetUserData(i)->GetActorHandle();
		if( !hActor ) continue;
		if( GetUserData(i)->bIsGMTrace() || GetUserData(i)->bIsAIBot() ) continue;

		pPlayer = (CDnPlayerActor*)hActor.GetPointer();
		if( !pPlayer->IsSkipCutScene() ) {
			bAllSkip = false;
			break;
		}
	}

	if( bAllSkip ) {
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			hActor = GetUserData(i)->GetActorHandle();
			if( !hActor ) continue;

			pPlayer = (CDnPlayerActor*)hActor.GetPointer();
			pPlayer->SetSkipCutScene( false );

			GetUserData(i)->SendSkipAllCutScene();
		}

		if (CDnPartyTask::IsActive(GetRoom()))
		{
			CDnPartyTask& partyTask = CDnPartyTask::GetInstance(GetRoom());
			partyTask.ReleaseSharingReversionItem();
		}
	}
}

void CDnGameTask::BackupDungeonHistory()
{
}

void CDnGameTask::RecoveryDungeonHistory( int nMapIndex )
{
}

void CDnGameTask::ClearDungeonHistory()
{

}

void CDnGameTask::GetPermitPassClassIDs(int * pClassArr, int nArrMaxSize)
{
	if (pClassArr == NULL) return;

	int nCount = 0;
	std::vector <int>::iterator ii;
	for (ii = m_vPermitPassClassIds.begin(); ii != m_vPermitPassClassIds.end(); ii++)
	{
		pClassArr[nCount] = (*ii);
		nCount++;
		if (nCount >= nArrMaxSize)
			return;
	}
}

int CDnGameTask::CheckRecoveryDungeonHistorySeed( int nMapIndex, int nRandomSeed )
{
	return nRandomSeed;
}

void CDnGameTask::CheckMerit(CDnPlayerActor * pActor, int nCheckType, int nCalcVal, UINT &nOutVal)
{
	//Added channel-related logic
	if (m_pMeritInfo == NULL) return;
	if (m_pMeritInfo->nMeritType == GlobalEnum::MERIT_BONUS_COMPLETEEXP)
	{
		if (m_pMeritInfo->nMaxLevel >= pActor->GetLevel() && m_pMeritInfo->nMinLevel <= pActor->GetLevel())
			nOutVal = (int)((float)nCalcVal * (float(m_pMeritInfo->nExtendValue)/100.f));
	}
}

DnPropHandle CDnGameTask::RequestGenerationProp( int nPropTableID, int nReqClassID, EtVector3 &vPosition, EtVector3 &vRotation, EtVector3 &vScale, int nLifeTime, void* pAdditionalPropInfo )
{
	DNTableFileFormat* pPropSox = GetDNTable( CDnTableDB::TPROP );
	if( !pPropSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"PropTable.ext failed\r\n");
		return CDnWorldProp::Identity();
	}
	if( !pPropSox->IsExistItem( nPropTableID ) )
	{
		_ASSERT( !"PropTable ID does not exist" );
		return CDnWorldProp::Identity();
	}

	int nClassID = pPropSox->GetFieldFromLablePtr( nPropTableID, "_ClassID" )->GetInteger();
	std::string szPropName = pPropSox->GetFieldFromLablePtr( nPropTableID, "_Name" )->GetString();

	_ASSERT( nReqClassID == nClassID && "Requested summon prop type does not match PropTable ID type." );

	int nSeed = CRandom::Seed(GetRoom());
	_srand( GetRoom(), nSeed );

	CEtWorldProp *pProp = CDnWorld::GetInstance(GetRoom()).AddProp( szPropName.c_str(), vPosition, vRotation, vScale, pAdditionalPropInfo );
	if( pProp == NULL ) return CDnWorldProp::Identity();
	DnPropHandle hProp = ((CDnWorldProp*)pProp)->GetMySmartPtr();

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pStruct = GetUserData(i);

		BYTE pBuffer[ 256 ];
		CPacketCompressStream PacketStream( pBuffer, 256 );
		DWORD dwPropUniqueID = hProp->GetUniqueID();
		PacketStream.Write( &dwPropUniqueID, sizeof(DWORD) );
		PacketStream.Write( &nPropTableID, sizeof(int) );
		PacketStream.Write( &nReqClassID, sizeof(int) );
		PacketStream.Write( &nSeed, sizeof(int) );
		PacketStream.Write( &vPosition, sizeof(EtVector3) );
		PacketStream.Write( &vRotation, sizeof(EtVector3) );
		PacketStream.Write( &vScale, sizeof(EtVector3) );
		PacketStream.Write( &nLifeTime, sizeof(int) );

		int iPropArgInfoSize = 0;
		switch( nReqClassID )
		{
			case PTE_Chest:
				{
					ChestStruct* pLocalStruct = static_cast<ChestStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nNeedKeyID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nItemDropGroupTableID, sizeof(int) );
				}
				break;

			case PTE_Broken:
				{
					BrokenStruct* pLocalStruct = static_cast<BrokenStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nDurability, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nItemDropGroupTableID, sizeof(int) );
				}
				break;

			case PTE_BrokenDamage:
				{
					BrokenDamageStruct* pLocalStruct = static_cast<BrokenDamageStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nMonsterTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nItemDropGroupTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nDurability, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nSkillTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nSkillLevel, sizeof(int) );
					PacketStream.Write( &pLocalStruct->SkillApplyType, sizeof(int) );
				}
				break;

			case PTE_HitMoveDamageBroken:
				{
					HitMoveDamageBrokenStruct* pLocalStruct = static_cast<HitMoveDamageBrokenStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nMonsterTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nItemDropGroupTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nDurability, sizeof(int) );
					PacketStream.Write( &pLocalStruct->MovingAxis, sizeof(int) );
					PacketStream.Write( &pLocalStruct->fDefaultAxisMovingSpeed, sizeof(float) );
					PacketStream.Write( &pLocalStruct->fMaxAxisMoveDistance, sizeof(float) );
				}
				break;

			case PTE_Buff:
				{
					BuffStruct* pLocalStruct = static_cast<BuffStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nMonsterTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nSkillLevel, sizeof(int) );
					PacketStream.Write( &pLocalStruct->fCheckRange, sizeof(float) );
					PacketStream.Write( &pLocalStruct->nTeam, sizeof(int) );
				}
				break;

			case PTE_BuffBroken:
				{
					BuffBrokenStruct* pLocalStruct = static_cast<BuffBrokenStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->nMonsterTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nDurability, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nItemDropGroupTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nSkillTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->nSkillLevel, sizeof(int) );
					PacketStream.Write( &pLocalStruct->fCheckRange, sizeof(float) );
					PacketStream.Write( &pLocalStruct->nTeam, sizeof(int) );
				}
				break;

			// #60784
			case PTE_HitStateEffect:
				{
					HitStateEffectStruct * pLocalStruct = static_cast<HitStateEffectStruct*>(pAdditionalPropInfo);
					PacketStream.Write( &pLocalStruct->MonsterTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->SkillTableID, sizeof(int) );
					PacketStream.Write( &pLocalStruct->SkillLevel, sizeof(int) );
				}
				break;


		}

		pStruct->SendPacket( SC_ROOM, eRoom::SC_GENERATION_PROP_MSG, PacketStream.GetBuffer(), PacketStream.Tell(), _RELIABLE );
	}

	// If it is an action prop, execute the Summon_On action
	if( hProp )
	{
		CDnWorldActProp* pWorldActProp = dynamic_cast<CDnWorldActProp*>(hProp.GetPointer());
		if( pWorldActProp && pWorldActProp->IsExistAction( "Summon_On" ) )
			pWorldActProp->SetActionQueue( "Summon_On" );
	}

	return hProp;
}

void CDnGameTask::RequestChangeGameSpeed( float fSpeed, DWORD dwDelay )
{
	SetLocalTimeSpeed( fSpeed, dwDelay );
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pStruct = GetUserData(i);
		SendGameChangeGameSpeed( pStruct, fSpeed, dwDelay );
	}
}

void CDnGameTask::CalcDungeonClear()
{
}
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
void CDnGameTask::ResetDungeonClear( bool bCreate, int nMapIndex, int nGateIndex, int nEnterMapTabeID, int nDungeonClearTableID, int nShareCountMapIndex )
#else
void CDnGameTask::ResetDungeonClear( bool bCreate, int nMapIndex, int nGateIndex, int nEnterMapTabeID, int nDungeonClearTableID )
#endif
{
	m_nReturnDungeonClearWorldID = -1;
	m_nReturnDungeonClearStartPositionIndex = -1;
	m_nEnterMapTableID = -1;
	m_nEnterDungeonTableID = -1;
	m_DungeonClearState = DCS_None;
	m_fDungeonClearDelta = 0.f;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	m_nShareCountMapIndex = 0;
#endif

	GetRoom()->ResetCompleteExperience();
	GetRoom()->ResetDungeonPlayTime();
	if( bCreate ) {
		m_bEnteredDungeon = true;
		m_nReturnDungeonClearWorldID = nMapIndex;
		m_nReturnDungeonClearStartPositionIndex = nGateIndex;
		m_nEnterDungeonTableID = nDungeonClearTableID;
		m_nEnterMapTableID = nEnterMapTabeID;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
		m_nShareCountMapIndex = nShareCountMapIndex;
#endif
	}
	else m_bEnteredDungeon = false;

	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNGameRoom::PartyStruct *pStruct = GetPartyData(i);
		if( pStruct == NULL ) continue;

#if !defined( PRE_ADD_DUNGEONCLEARINFO )
		pStruct->ClearInfo = CDNGameRoom::DungeonClearInfo();
#endif
		CDNUserSession *pSession = GetUserData(i);
		if( !pSession ) continue;
		DnActorHandle hActor = pSession->GetActorHandle();
		if( !hActor ) continue;
		CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(pSession->GetActorHandle().GetPointer());
		if( pActor ) {
			pActor->ResetCP();
			pActor->ResetTotalComboCount();
			pActor->UpdateStartPartyCount( bCreate ? GetUserCount() : 0 );
		}
	}
#if defined( PRE_ADD_DUNGEONCLEARINFO )	
	GetRoom()->ResetFirstPartyMember();
#endif

	// When changing from dungeon -> world/village [2011/01/12 semozz]
	// Initialize Sea Dragon Nest config values..
	CDnWorld::GetInstance(GetRoom()).SetDragonNestType(eDragonNestType::None);
}

void CDnGameTask::RequestPlayCutScene( int nCutSceneTableID, bool bFadeIn, int nQuestIndex, int nQuestStep)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCUTSCENE );
	if( !pSox->IsExistItem( nCutSceneTableID ) ) return;

	// For a quest cutscene, distinguish who the quest performer is and notify each client.
	DWORD dwQuestPlayingUserUniqueID = UINT_MAX;
	if( 0 < nQuestIndex && 0 < nQuestStep )
	{
		DWORD dwNumUser = GetUserCount();
		for( DWORD i = 0; i < dwNumUser; ++i )
		{
			CDNUserSession* pSession = GetUserData( i );
			bool bQuestPlayer = (0 <= pSession->GetQuest()->FindPlayingQuest( nQuestIndex ));
			if( bQuestPlayer )
			{
				dwQuestPlayingUserUniqueID = pSession->GetSessionID();
				break;
			}
		}
	}

	m_bWaitPlayCutScene = true;
	m_bCutSceneAutoFadeIn = bFadeIn;
	for( DWORD i=0; i<GetUserCount(); i++ ) {
		CDNUserSession *pSession = GetUserData(i);
		if( pSession->GetActorHandle() ) {
			CDnPlayerActor *pPlayer = (CDnPlayerActor*)pSession->GetActorHandle().GetPointer();
			pPlayer->SetCompleteCutScene( false );
			pPlayer->SetSkipCutScene( false );
			pPlayer->SetCheckCompleteCutScene( true );
		}
		pSession->SendPlayCutScene( nCutSceneTableID, nQuestIndex ,nQuestStep, -1, true, dwQuestPlayingUserUniqueID );
	}

	if (CDnPartyTask::IsActive(GetRoom()))
	{
		CDnPartyTask& partyTask = CDnPartyTask::GetInstance(GetRoom());
		partyTask.HoldSharingReversionItem();
	}
}

void CDnGameTask::SetGameProcessDelay( int nDelay )
{
	m_bGameProcessDelay = true;
	m_fGameProcessDelayDelta = nDelay * 0.001f;
	if( nDelay == 0 ) {
		m_bGameProcessDelay = false;
	}
}


void CDnGameTask::RequestChangeMap( int nMapIndex, char cGateNo )
{
	if( nMapIndex == -1 ) {
		CDNUserSession *pLeaderSession = NULL;
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			pSession->SetNextVillageData( NULL, 0, GetMapTableID(), nMapIndex, cGateNo );

			if( GetPartyData(i)->bLeader ) pLeaderSession = pSession;
		}

		if( pLeaderSession ) {
			nMapIndex = pLeaderSession->GetLastVillageMapIndex();
			if( cGateNo == -1 ) cGateNo = pLeaderSession->GetLastVillageGateNo();
		}
	}
	if( nMapIndex == -1 ) return;

	if( g_pDataManager->GetMapType( nMapIndex ) == GlobalEnum::eMapTypeEnum::MAP_VILLAGE ) {

		CDNUserSession *pLeaderSession = NULL;
		for( DWORD i=0; i<GetUserCount(); i++ ) {
			CDNUserSession *pSession = GetUserData(i);
			pSession->SetNextVillageData( NULL, 0, GetMapTableID(), nMapIndex, cGateNo );
			
			if( GetPartyData(i)->bLeader ) pLeaderSession = pSession;
		}

		if( pLeaderSession ) {
			g_pMasterConnectionManager->SendRequestNextVillageInfo(pLeaderSession->GetWorldSetID(), nMapIndex, cGateNo, true, pLeaderSession->GetGameRoom());
		}
	}
	else {

#if defined(PRE_FIX_63305)
		//#63305 Refill HP/MP to full when moving to world map.
		bool bRefreshHPMP = g_pDataManager->GetMapType( nMapIndex ) == GlobalEnum::eMapTypeEnum::MAP_WORLDMAP;
#endif // PRE_FIX_63305

		for( DWORD i=0; i<GetUserCount(); i++ )
		{
			CDNUserSession *pSession = GetUserData(i);
			if( !pSession ) continue;
			DnActorHandle hActor = pSession->GetActorHandle();
			if( !hActor ) continue;
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
			pPlayer->CmdRemoveStateEffect(STATE_BLOW::BLOW_099);

#if defined(PRE_FIX_63305)
			if (bRefreshHPMP == true)
				pPlayer->CmdRefreshHPSP( pPlayer->GetMaxHP(), pPlayer->GetMaxSP() );
#endif // PRE_FIX_63305
		}
#if defined(PRE_FIX_INITSTATEANDSYNC)
		GetRoom()->ReserveInitStateAndSync( nMapIndex, cGateNo, CRandom::Seed(GetRoom()), GetStageDifficulty(), true );
#else
		GetRoom()->InitStateAndSync( nMapIndex, cGateNo, CRandom::Seed(GetRoom()), GetStageDifficulty(), true );
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
	}

}

void CDnGameTask::RequestSummonMonster( DnActorHandle hOwner, SummonMonsterStruct* pStruct, bool bReCreateFollowStageMonster/* = false*/, int nSkillID/* = -1*/ )
{
	if( !hOwner )
		return;

	if( !hOwner->bIsCanSummonMonster( pStruct ) )
	{
		//std::cout << "Hit maximum monster count limit!!!!" << std::endl;
		
		//If the max monster count limit is reached, remove the summoned monster with the oldest summon time.
		DnMonsterActorHandle hOldSummon = hOwner->FindOldSummonMonster(pStruct);
		if (hOldSummon)
			hOldSummon->CmdSuicide(false, false);
	}

	// Determine whether to summon with a certain probability (100% if 0)
	if( pStruct->nSummonRate > 0 && pStruct->nSummonRate < _rand(GetRoom())%100 )
		return;

	MatrixEx Cross = *hOwner->GetMatEx();
	Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
	Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
	Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;
	INSTANCE(CDnWorld).RevisionPosByNaviMesh( Cross, hOwner->GetMatEx()->m_vPosition );

	// #28351 Added rotation offset value.
	EtMatrix matNowRot = Cross;
	matNowRot._41 = matNowRot._42 = matNowRot._43 = 0.0f;
	EtMatrix matRotY;
	EtMatrixRotationY( &matRotY, EtToRadian(pStruct->fAngleOffset) );
	matNowRot = matNowRot * matRotY;
	Cross.CopyRotationFromThis( (MatrixEx*)&matNowRot );

	// Extract rotation as a quaternion for sending in packet.
	EtQuat qRotation;
	EtQuaternionRotationMatrix( &qRotation, &matNowRot );

	EtVector3 v;
	v.x = v.y = v.z = 0.0f;

	int nTeam = hOwner->GetTeam();
	if( pStruct->nTeam == 1 ) {
		nTeam = abs( hOwner->GetTeam() - 1 );
	}

	DnMonsterActorHandle hSummonedMonster = RequestGenerationMonsterFromActorID( pStruct->MonsterID, -1, Cross.GetPosition(), v, v, NULL, -1, nTeam, false, &qRotation, pStruct->nForceSkillLevel );
	if( !hSummonedMonster )
	{
		// Monster creation failed!! Creation fails if there is no data for that monster ID in the monster table.
		g_Log.Log( LogType::_SKILL_ACTION, "[GameServer CDnGameTask::RequestSummonMonster] %d Monster Id Generation Fail!!\r\n", pStruct->MonsterID );
		return;
	}

	if( pStruct->nLifeTime > 0 )
		hSummonedMonster->SetTimeMonster( true, pStruct->nLifeTime );
	else
		hSummonedMonster->SetTimeMonster( false, -1 );

	// #32426 Related to summon control feature. Automatically dies if separated beyond a certain distance.
	if( 0.0f < pStruct->fLimitSummonerDistance )
		hSummonedMonster->SetLimitSummonerDistance( pStruct->fLimitSummonerDistance );

	*hSummonedMonster->GetMatEx() = Cross;

	_ASSERT( hSummonedMonster && "SummonMonster signal: failed to summon monster!" );

	// If the monster needs to copy the caster's stats, copy them.
	if( pStruct->bCopySummonerState )
	{
		if( hSummonedMonster )
		{
			int nMoveSpeed = hSummonedMonster->GetMoveSpeed();
			int nBaseMoveSpeed = const_cast<CDnState*>(hSummonedMonster->GetBaseState())->GetMoveSpeed(); // Original base movement speed value, usually 0.
			CDnActorState::ActorTypeEnum eActorType = hSummonedMonster->GetActorType();
			int nPressLevel = hSummonedMonster->GetPressLevel();
			float fRevisionWeight = hSummonedMonster->GetWeight();
			float fRotateAngleSpeed = hSummonedMonster->GetRotateAngleSpeed();
			hSummonedMonster->CopyStateFromThis( hOwner, (TRUE == pStruct->bDontCopySkillStateEffect) );	// Issue #37841. Copy stats excluding state effect stats.

			const_cast<CDnState*>(hSummonedMonster->GetBaseState())->SetMaxHP( (INT64)( hOwner->GetMaxHP() * pStruct->fHPAdjustValue ) );
			const_cast<CDnState*>(hSummonedMonster->GetBaseState())->SetMoveSpeed( nBaseMoveSpeed );
			hSummonedMonster->RefreshState();
			hSummonedMonster->SetHP( hSummonedMonster->GetMaxHP() );

			hSummonedMonster->SetRotateAngleSpeed( fRotateAngleSpeed );		// Do not copy rotation speed either.
			hSummonedMonster->SetActorType( eActorType );	// Keep the actor type as is. Summoned actors are always monsters.
			hSummonedMonster->SetPressLevel( nPressLevel );	// Weight and overlap level must also be preserved as is.
			hSummonedMonster->SetMovable( true );			// User may be summoned while under an immovable state effect, so clear it to allow movement. #23131
			hSummonedMonster->SetStateEffect( hSummonedMonster->GetStateEffect() & ~CDnActorState::Cant_Move );
			static_cast<CDnMonsterActor*>(hSummonedMonster.GetPointer())->SetRevisionWeight( fRevisionWeight );

			if( hOwner->IsPlayerActor() )
			{
				CDnState::ElementEnum eType = ( hOwner->CDnActor::GetWeapon() ) ? hOwner->CDnActor::GetWeapon()->GetElementType() : CDnState::ElementEnum_Amount;
				hSummonedMonster->SetMonsterElementType( eType );
			}
		}
	}

	hSummonedMonster->SetAutoRecallRange( pStruct->AutoRecallRange );

	hOwner->PushSummonMonster( hSummonedMonster, pStruct, bReCreateFollowStageMonster );

	// Notify the client that this is a monster summoned by a player.
	hSummonedMonster->SlaveOf( hOwner->GetUniqueID(), 
							   true, 
							   (pStruct->bSuicideWhenSummonerDie == TRUE), 
							   (pStruct->bFollowSummonerStage == TRUE), 
							   bReCreateFollowStageMonster );

	if( hOwner->IsPlayerActor() )
	{
		if( hOwner->IsProcessSkill() )
		{
			DnSkillHandle hProcessSkill = hOwner->GetProcessSkill();

			//#52874 When summoning a monster via projectile, the currently active skill may differ from the monster-summoning skill.
			if (hProcessSkill)
			{
				if (nSkillID != -1 && hProcessSkill->GetClassID() != nSkillID)
					hProcessSkill = hOwner->FindSkill(nSkillID);
				
				if (hProcessSkill)
				{
					CDnState::ElementEnum eSkillElement = hProcessSkill->GetElement();
					if( CDnState::ElementEnum::ElementEnum_Amount != eSkillElement )
						hSummonedMonster->PushForceHitElement( eSkillElement );
				}
			}
		}
	}

	//Set the creation timestamp.
	hSummonedMonster->SetRequestSummonTime(hOwner->GetLocalTime());

	// BLOW_290: AddStateOnSummonMonsterBlow (e.g. Gear Master Lv 65 passive)
	if( hOwner && hOwner->IsAppliedThisStateBlow( STATE_BLOW::BLOW_290 ) )
	{
		DNVector(DnBlowHandle) vlBlows290;
		hOwner->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_290, vlBlows290 );
		for( DWORD bi = 0; bi < vlBlows290.size(); ++bi )
		{
			if( vlBlows290[bi] )
				static_cast<CDnAddStateOnSummonMonsterBlow*>( vlBlows290[bi].GetPointer() )->OnSummonMonster( hSummonedMonster, pStruct->MonsterID );
		}
	}

#ifdef PRE_ADD_39644 // Grant invincibility to the summoned monster.
	if( pStruct->AddImmuneBlow )
		hSummonedMonster->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_099, -1 , "-1" );
#endif

#if defined(PRE_ADD_65808)
	std::list<int> glyphIDs;
	if (hOwner->GetSummonMonsterGlyphInfo(pStruct->MonsterID, glyphIDs))
	{
		std::list<int>::iterator iter = glyphIDs.begin();
		std::list<int>::iterator endIter = glyphIDs.end();

		for (; iter != endIter; ++iter)
		{
			int nGlyphID = (*iter);

			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hSummonedMonster.GetPointer());
			ApplySummonMonsterGlyph(hOwner, pMonsterActor, nGlyphID);
		}
	}
#endif // PRE_ADD_65808
}

#ifdef _TEST_CODE_KAL
void CDnGameTask::RequestDungeonClear_ForCheat(int mapTableID, DWORD clearTime, int meritBonusID, int enterGateIdx, char partyMemeberCount, int completeExp, int rankIdx, int difficulty)
{
	RequestDungeonClear(true, CDnActor::Identity(), false);
}
#endif // _TEST_CODE_KAL

void CDnGameTask::SyncMonster( CDNUserSession* pBreakIntoGameSession )
{
	DN_ASSERT( pBreakIntoGameSession != NULL, "CDnGameTask::SyncMonster() pBreakIntoGameSession != NULL" );

	for( UINT i=0 ; i<m_hVecMonsterList.size() ; ++i )
	{
		DnActorHandle hMonster = m_hVecMonsterList[i];
		if( !hMonster || !hMonster->IsMonsterActor() )
			continue;

		if( hMonster->IsDie() || hMonster->IsDestroy() )
			continue;

		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hMonster.GetPointer());

		// Monster spawned by Trigger
		if( pMonster->bIsTriggerMonster() )
		{
			EtQuat qRotation;
			EtMatrix matNowRot = *hMonster->GetMovement()->GetMatEx();
			EtQuaternionRotationMatrix( &qRotation, &matNowRot );

#ifdef PRE_MOD_DARKLAIR_RECONNECT
			SendGameGenerationMonsterMsg( pBreakIntoGameSession, pMonster->GetTriggerRandomSeed(), pMonster->GetMonsterClassID(), pMonster->GetUniqueID(), 
										*(hMonster->GetPosition()), EtVector3(0.f,0.f,0.f), EtVector3(0.f,0.f,0.f), pMonster->GetEventAreaUniqueID(), pMonster->GetTeam(), true, &qRotation );
#else // PRE_MOD_DARKLAIR_RECONNECT
			SendGameGenerationMonsterMsg( pBreakIntoGameSession, pMonster->GetTriggerRandomSeed(), pMonster->GetMonsterClassID(), 
										  pMonster->GetUniqueID(), *(hMonster->GetPosition()), EtVector3(0.f,0.f,0.f), EtVector3(0.f,0.f,0.f), -1, pMonster->GetTeam(), true, &qRotation );
#endif // PRE_MOD_DARKLAIR_RECONNECT

			if( pMonster->IsSummonedMonster() ) // If summoned monster, resend summon info.
				pMonster->ReTransmitSlaveMsg(pBreakIntoGameSession);
		}
		// Default monster
		else
		{
			_SendActorShow( pBreakIntoGameSession, hMonster );
			_SendActorTeam( pBreakIntoGameSession, hMonster );
		}

		// Sync HP/SP
		hMonster->CmdRefreshHPSP( hMonster->GetHP(), hMonster->GetSP() );
		// Synchronize state effects
		hMonster->CmdSyncBlow( pBreakIntoGameSession );
	}
}

void CDnGameTask::SyncNpc( CDNUserSession* pBreakIntoGameSession )
{
	DN_ASSERT( pBreakIntoGameSession != NULL, "CDnGameTask::SyncNpc() pBreakIntoGameSession != NULL" );

	for( UINT i=0 ; i<m_hVecNpcList.size() ; ++i )
	{
		DnActorHandle hNpc = m_hVecNpcList[i];
		if( !hNpc )
			continue;

		DN_ASSERT( hNpc->IsNpcActor(), "CDnGameTask::SyncNpc() hMonster->IsNpcActor()" );

		CDnNPCActor* pNpc = static_cast<CDnNPCActor*>(hNpc.GetPointer());

		// NPC spawned by Trigger
		if( pNpc->bIsTriggerNpc() )
		{
			pBreakIntoGameSession->SendEnterNpc( pNpc->GetUniqueID(), pNpc->GetNpcClassID(), pNpc->GetPosition()->x, pNpc->GetPosition()->y, pNpc->GetPosition()->z, 0.f );
		}
		// Default monster
		else
		{
			if (hNpc->IsShow())
				_SendActorShow( pBreakIntoGameSession, hNpc );
		}
	}
}

void CDnGameTask::_SendActorShow( CDNUserSession* pBreakIntoGameSession, DnActorHandle hActor )
{
	DnActorHandle hSessionActor = pBreakIntoGameSession->GetActorHandle();
	if( !hSessionActor )
		return;

	hSessionActor->Send( pBreakIntoGameSession, eActor::SC_CMDSHOW, hActor->GetUniqueID(), NULL );
}

void CDnGameTask::_SendActorHide( CDNUserSession* pBreakIntoGameSession, DnActorHandle hActor )
{
	DnActorHandle hSessionActor = pBreakIntoGameSession->GetActorHandle();

	hSessionActor->Send( pBreakIntoGameSession, eActor::SC_CMDHIDE, hActor->GetUniqueID(), NULL );
}

void CDnGameTask::_SendActorTeam( CDNUserSession* pBreakIntoGameSession, DnActorHandle hActor )
{
	DnActorHandle hSessionActor = pBreakIntoGameSession->GetActorHandle();
	if( !hSessionActor )
		return;

	int iTeam = hActor->GetTeam();
	char pBuffer[32];
	CPacketCompressStream Stream( pBuffer, sizeof(pBuffer) );
	Stream.Write( &iTeam, sizeof(int) );

	hSessionActor->Send( pBreakIntoGameSession, eActor::SC_CMDCHANGETEAM, hActor->GetUniqueID(), &Stream );
}

bool CDnGameTask::_CheckRequestNestDungeonClear( int iMapIndex )
{
	const TMapInfo *pInfo = g_pDataManager->GetMapInfo( iMapIndex );

	if( !pInfo ) 
		return false;
	if( pInfo->MapType != EWorldEnum::MapTypeDungeon ) 
		return false;
	switch( pInfo->MapSubType ) 
	{
		case EWorldEnum::MapSubTypeNest:
		case EWorldEnum::MapSubTypeEvent:
		case EWorldEnum::MapSubTypeChaosField:
		case EWorldEnum::MapSubTypeDarkLair:
#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
		case EWorldEnum::MapSubTypeTreasureStage:
#endif	// #if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
#if defined( PRE_ADD_DRAGON_FELLOWSHIP )
		case EWorldEnum::MapSubTypeFellowship:
#endif	// #if defined( PRE_ADD_DRAGON_FELLOWSHIP )
			break;
		default: 
			return false;
	}

	return true;
}

void CDnGameTask::RequestNestDungeonClear( DnActorHandle hActor )
{
	int nMapTableID = GetEnterMapTableID();
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	if( GetShareCountMapIndex() > 0 )
		nMapTableID = GetShareCountMapIndex();
#endif
	if( _CheckRequestNestDungeonClear( nMapTableID ) == false )
		return;

	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		CDNUserSession *pSession = GetUserData(i);
		if( !pSession || pSession->bIsAIBot() ) 
			continue;

		if( pSession->GetActorHandle() != hActor )
			continue;

		pSession->AddNestClear( nMapTableID );
		pSession->SendUpdateNestClear( pSession->GetNestClearCount(nMapTableID) );

	}
}

void CDnGameTask::RequestNestDungeonClear( int iForceMapIndex/*=0*/ )
{
	int nMapTableID = GetEnterMapTableID();
	if( iForceMapIndex > 0 )
		nMapTableID = iForceMapIndex;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	else if( GetShareCountMapIndex() > 0 )
		nMapTableID = GetShareCountMapIndex();
#endif

	if( _CheckRequestNestDungeonClear(nMapTableID) == false )
		return;

	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		CDNUserSession *pSession = GetUserData(i);
		if( !pSession || pSession->bIsAIBot() ) continue;

		pSession->AddNestClear( nMapTableID );
		pSession->SendUpdateNestClear( pSession->GetNestClearCount(nMapTableID) );
	}
}

void CDnGameTask::RequestEnableDungeonClearWarpAlarm( bool bEnable )
{
	CDNUserSession* pLeaderSession = GetPartyLeaderSession();

	if( pLeaderSession )
		SendGameEnableDungeonClearLeaderWarp(pLeaderSession, bEnable);
}

void CDnGameTask::ProcessTimeAttack( float fDelta )
{
	if( IsTimeAttack() == false )
		return;

	m_fTimeAttackDelta -= fDelta;

	if( m_fTimeAttackDelta <= 0.f )
		m_fTimeAttackDelta = 0.f;
}

void CDnGameTask::StartTimeAttack( int iMin, int iSec )
{
	m_bIsTimeAttack		= true;
	m_fTimeAttackDelta	= ((iMin*60)+iSec)*1.f;
	m_fOriginTimeAttackDelta = m_fTimeAttackDelta;
}

void CDnGameTask::StopTimeAttack()
{
	if( IsTimeAttack() == false )
		return;
	m_bIsTimeAttack = false;

	for( UINT i=0 ; i<GetUserCount() ; ++i )
		SendGameStopDungeonTimeAttack( GetUserData(i) );
}

int CDnGameTask::GetRemainTimeAttackSec() const
{
	if( IsTimeAttack() == false )
		return -1;

	return static_cast<int>(m_fTimeAttackDelta);
}

int CDnGameTask::GetOriginTimeAttackSec() const
{
	if( IsTimeAttack() == false )
		return -1;

	return static_cast<int>(m_fOriginTimeAttackDelta);
}

void CDnGameTask::SyncTimeAttack( CDNUserSession* pSession/*=NULL*/ )
{
	if( IsTimeAttack() == false )
		return;

	int iOrgSec = GetOriginTimeAttackSec();
	int iSec = GetRemainTimeAttackSec();

	// Synchronize to all users
	if( pSession == NULL )
	{
		for( UINT i=0 ; i<GetUserCount() ; ++i )
			SendGameSyncDungeonTimeAttack( GetUserData(i), iSec, iOrgSec );
	}
	// Sync only to that session
	else
	{
		SendGameSyncDungeonTimeAttack( pSession, iSec, iOrgSec );
	}
}

void CDnGameTask::EnableGameWarpDungeonClearToLeader()
{
	CDNUserSession* pLeader = GetPartyLeaderSession();
	if (!pLeader)
		return;

	SendGameEnableDungeonClearLeaderWarp(pLeader, true);
	m_bDungeonClearSendWarpEnable = true;
}

void CDnGameTask::SendGameWarpDungeonClearToLeader()
{
	CDNUserSession* pLeader = GetPartyLeaderSession();
	if (!pLeader)
		return;

	SendGameWarpDungeonClear(pLeader);
}

#ifdef PRE_FIX_PARTY_STAGECLEAR_CHANGEMASTER
void CDnGameTask::SetWarpDungeonClearToLeaderCheck(bool bSet)
{
	m_bDungeonClearSendWarpEnable = !bSet;
}
#endif

void CDnGameTask::SendActorShow( CDNUserSession* pBreakIntoGameSession, DnActorHandle hActor )
{
	_SendActorShow(pBreakIntoGameSession, hActor);
}

void CDnGameTask::SendActorHide( CDNUserSession* pBreakIntoGameSession, DnActorHandle hActor )
{
	_SendActorHide(pBreakIntoGameSession, hActor);
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnGameTask::CheckSelectNamedItem( CDNUserSession * pSession, TItem& RewardItem, BYTE cIndex )
{
	TItemData *pItemData = g_pDataManager->GetItemData( RewardItem.nItemID );
	if(pItemData)
	{
		if(pItemData->cRank == ITEMRANK_SSS)
		{
			pSession->GetDBConnection()->QueryCheckNamedItemCount( pSession, pItemData->nItemID, cIndex );
		}		
	}
}

void CDnGameTask::SwapNamedItemToNormalItem( CDNUserSession * pSession, TItem& RewardItem )
{	
	int nSwapItemID = g_pDataManager->GetNamedItemSwapID( RewardItem.nItemID );
	if( pSession && pSession->GetItem() )
	{
		pSession->GetItem()->MakeItemStruct(nSwapItemID, RewardItem);
	}
}

void CDnGameTask::CheckSelectNamedItemResult( CDNUserSession * pSession, TACheckNamedItemCount* pA )
{
	CDNGameRoom::PartyStruct *pPartyStruct = GetPartyData( pSession );
	if( !pPartyStruct ) return;

	int nMaxCount = g_pDataManager->GetNamedItemMaxCount( pA->nItemID );

#if defined( PRE_ADD_DUNGEONCLEARINFO )
	CDNGameRoom::PartyFirstStruct* PartyFirst = GetRoom()->GetFirstPartyData( pPartyStruct->pSession->GetCharacterDBID() );

	if( !PartyFirst )
	{
		// By design, missing case is a bug
		g_Log.Log(LogType::_ERROR, 0, 0, 0, 0, L"[GameTask::CheckSelectNamedItemResult] PartyFirst error!!\r\n");
		return;
	}
	
	if(pA->cIndex >= 0 && pA->cIndex < 4)
	{
		if( PartyFirst->ClearInfo.RewardItem[pA->cIndex].nItemID == pA->nItemID )
		{
			if( pA->nCount < nMaxCount )
			{
				//Legend item can be granted
				PartyFirst->ClearInfo.bNamedGiveResult[pA->cIndex] = true;
			}
		}
	}	
#else
	if(pA->cIndex >= 0 && pA->cIndex < 4)
	{
		if( pPartyStruct->ClearInfo.RewardItem[pA->cIndex].nItemID == pA->nItemID )
		{
			if( pA->nCount < nMaxCount )
			{
				//Legend item can be granted
				pPartyStruct->ClearInfo.bNamedGiveResult[pA->cIndex] = true;
			}
		}
	}	
#endif
}
#endif


DnActorHandle CDnGameTask::RequestSummonMonsterBySkill( DnActorHandle hOwner, DnActorHandle hSkillUser, SummonMonsterStruct* pStruct, bool bReCreateFollowStageMonster/* = false*/, int nSkillID/* = -1*/ )
{
	if( !hOwner || !hSkillUser)
		return CDnActor::Identity();

	if( !hOwner->bIsCanSummonMonster( pStruct ) )
	{
		//If the max monster count limit is reached, remove the summoned monster with the oldest summon time.
		DnMonsterActorHandle hOldSummon = hOwner->FindOldSummonMonster(pStruct);
		if (hOldSummon)
			hOldSummon->CmdSuicide(false, false);
	}

	// Determine whether to summon with a certain probability (100% if 0)
	if( pStruct->nSummonRate > 0 && pStruct->nSummonRate < _rand(GetRoom())%100 )
		return CDnActor::Identity();

	MatrixEx Cross = *hSkillUser->GetMatEx();
	Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
	Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
	Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;
	INSTANCE(CDnWorld).RevisionPosByNaviMesh( Cross, hSkillUser->GetMatEx()->m_vPosition );

	// #28351 Added rotation offset value.
	EtMatrix matNowRot = Cross;
	matNowRot._41 = matNowRot._42 = matNowRot._43 = 0.0f;
	EtMatrix matRotY;
	EtMatrixRotationY( &matRotY, EtToRadian(pStruct->fAngleOffset) );
	matNowRot = matNowRot * matRotY;
	Cross.CopyRotationFromThis( (MatrixEx*)&matNowRot );

	// Extract rotation as a quaternion for sending in packet.
	EtQuat qRotation;
	EtQuaternionRotationMatrix( &qRotation, &matNowRot );

	EtVector3 v;
	v.x = v.y = v.z = 0.0f;

	int nTeam = hOwner->GetTeam();
	if( pStruct->nTeam == 1 ) {
		nTeam = abs( hOwner->GetTeam() - 1 );
	}
	DnMonsterActorHandle hSummonedMonster = RequestGenerationMonsterFromActorID( pStruct->MonsterID, -1, Cross.GetPosition(), v, v, NULL, -1, nTeam, false, &qRotation, pStruct->nForceSkillLevel );
	if( !hSummonedMonster )
	{
		// Monster creation failed!! Creation fails if there is no data for that monster ID in the monster table.
		g_Log.Log( LogType::_SKILL_ACTION, "[GameServer CDnGameTask::RequestSummonMonster] %d Monster Id Generation Fail!!\r\n", pStruct->MonsterID );
		return CDnActor::Identity();
	}

	if( pStruct->nLifeTime > 0 )
		hSummonedMonster->SetTimeMonster( true, pStruct->nLifeTime );
	else
		hSummonedMonster->SetTimeMonster( false, -1 );

	// #32426 Related to summon control feature. Automatically dies if separated beyond a certain distance.
	if( 0.0f < pStruct->fLimitSummonerDistance )
		hSummonedMonster->SetLimitSummonerDistance( pStruct->fLimitSummonerDistance );

	*hSummonedMonster->GetMatEx() = Cross;

	_ASSERT( hSummonedMonster && "SummonMonster signal: failed to summon monster!" );

	// If the monster needs to copy the caster's stats, copy them.
	if( pStruct->bCopySummonerState )
	{
		if( hSummonedMonster )
		{
			int nMoveSpeed = hSummonedMonster->GetMoveSpeed();
			CDnActorState::ActorTypeEnum eActorType = hSummonedMonster->GetActorType();
			int nPressLevel = hSummonedMonster->GetPressLevel();
			float fRevisionWeight = hSummonedMonster->GetWeight();
			float fRotateAngleSpeed = hSummonedMonster->GetRotateAngleSpeed();

			//////////////////////////////////////////////////////////////////////////
			//Press info must not be copied for puppets...
			//When summoning a puppet in Green Dragon, as Press info copies ActorState values, the actual puppet's info gets overwritten with Green Dragon's values
			//Server/client synchronization issue occurred.
			CDnActorState::ActorPressEnum pressEnum = hSummonedMonster->GetPress();
			int nUnitSizeParam1 = hSummonedMonster->GetUnitSizeParam1();
			int nUnitSizeParam2 = hSummonedMonster->GetUnitSizeParam2();
			int nUnitSize = hSummonedMonster->GetUnitSize();
			//float fWeight = hSummonedMonster->GetWeight();
			int nHeight = hSummonedMonster->GetHeight();
			CDnActorState::ActorMaterialEnum materialEnum = hSummonedMonster->GetMaterial();
			CDnActorState::ActorTypeEnum actorType = hSummonedMonster->GetActorType();
			int nActorTableID = hSummonedMonster->GetActorTableID();
			//float fRotateAngleSpeed = hSummonedMonster->GetRotateAngleSpeed();
			//////////////////////////////////////////////////////////////////////////

			hSummonedMonster->CopyStateFromThis( hOwner, (TRUE == pStruct->bDontCopySkillStateEffect) );	// Issue #37841. Copy stats excluding state effect stats.
			// #25637 Summon based on summoner's max HP rather than current HP. Below a certain HP, it could spawn with 0 HP and die immediately.
			// MaxHP is updated again when State refreshes, so the player's max HP is passed directly into CopyStateFromThis()
			// #35590 Must readjust here so it does not become equal to player's max HP when the state effect is applied.
			// Values adjusted in CDnBasicBlow must also restore StateStep(0) values to the original monster values so related state effects change in sync with client.
			// Among values modified here, those also used on the client and in CDnBasicBlow must be restored. Otherwise related state effects 
			// When added, values synced with client differ due to StateStep(0) values.
			hSummonedMonster->SetMaxHP( (INT64)( hOwner->GetMaxHP() * pStruct->fHPAdjustValue ) );
			const_cast<CDnState*>(hSummonedMonster->GetBaseState())->SetMaxHP( (INT64)( hOwner->GetMaxHP() * pStruct->fHPAdjustValue ) );

			hSummonedMonster->SetHP( (INT64)( hOwner->GetMaxHP() * pStruct->fHPAdjustValue ) );

			//////////////////////////////////////////////////////////////////////////
			//If monster was summoned by puppet skill, set defense value to summoner's defense.
			hSummonedMonster->SetDefenseM( (int)(hOwner->GetDefenseM() * pStruct->fHPAdjustValue ));
			hSummonedMonster->SetDefenseMRatio( (hOwner->GetDefenseMRatio() * pStruct->fHPAdjustValue ));
			hSummonedMonster->SetDefenseP( (int)(hOwner->GetDefenseP() * pStruct->fHPAdjustValue ));
			hSummonedMonster->SetDefensePRatio( (hOwner->GetDefensePRatio() * pStruct->fHPAdjustValue ));
			
			//Elemental defense values.
			for (int i = 0; i < CDnState::ElementEnum_Amount; ++i)
			{
				hSummonedMonster->SetElementDefense( (CDnState::ElementEnum)i, (hOwner->GetElementDefense((CDnState::ElementEnum)i) * pStruct->fHPAdjustValue ));
			}
			//////////////////////////////////////////////////////////////////////////		

			hSummonedMonster->SetMoveSpeed( nMoveSpeed );	// Do not copy move speed stat.
			hSummonedMonster->GetStateStep( 0 )->SetMoveSpeed( nMoveSpeed );

			hSummonedMonster->SetRotateAngleSpeed( fRotateAngleSpeed );		// Do not copy rotation speed either.

			hSummonedMonster->SetActorType( eActorType );	// Keep the actor type as is. Summoned actors are always monsters.
			hSummonedMonster->SetPressLevel( nPressLevel );	// Weight and overlap level must also be preserved as is.
			hSummonedMonster->SetMovable( true );			// User may be summoned while under an immovable state effect, so clear it to allow movement. #23131
			hSummonedMonster->SetStateEffect( hSummonedMonster->GetStateEffect() & ~CDnActorState::Cant_Move );
			static_cast<CDnMonsterActor*>(hSummonedMonster.GetPointer())->SetRevisionWeight( fRevisionWeight );

			//Restore Press info values saved above.
			hSummonedMonster->SetPress(pressEnum);
			hSummonedMonster->SetUnitSizeParam1(nUnitSizeParam1);
			hSummonedMonster->SetUnitSizeParam2(nUnitSizeParam2);
			hSummonedMonster->SetUnitSize(nUnitSize);
			hSummonedMonster->SetHeight(nHeight);
			hSummonedMonster->SetMaterial(materialEnum);

			hSummonedMonster->SetActorType(actorType);
			hSummonedMonster->SetActorTableID(nActorTableID);

			if( hOwner->IsPlayerActor() )
			{
				CDnState::ElementEnum eType = ( hOwner->CDnActor::GetWeapon() ) ? hOwner->CDnActor::GetWeapon()->GetElementType() : CDnState::ElementEnum_Amount;
				hSummonedMonster->SetMonsterElementType( eType );
			}
		}
	}

#if defined(PRE_FIX_61382)
	//Also apply the owner actor's immunity settings to the summoned monster.
	DNVector(DnBlowHandle) vlStateBlows;
	hOwner->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_077, vlStateBlows);
	int nBlowCount = (int)vlStateBlows.size();
	for (int i = 0; i < nBlowCount; ++i)
	{
		DnBlowHandle hBlow = vlStateBlows[i];

		hSummonedMonster->CDnActor::AddStateBlow(STATE_BLOW::BLOW_077, NULL, -1, hBlow->GetValue(), false, false);
	}
#endif // PRE_FIX_61382

	hSummonedMonster->SetAutoRecallRange( pStruct->AutoRecallRange );

	hOwner->PushSummonMonster( hSummonedMonster, pStruct, bReCreateFollowStageMonster );

	//Set up Puppet before calling SlaveOf below..
	hSummonedMonster->SetPuppetSummonMonster(true);

	// Notify the client that this is a monster summoned by a player.
	hSummonedMonster->SlaveOf( hOwner->GetUniqueID(), 
		true, 
		(pStruct->bSuicideWhenSummonerDie == TRUE), 
		(pStruct->bFollowSummonerStage == TRUE), 
		bReCreateFollowStageMonster );

	// If the summoner is a player and summoned via a skill, force-grant the skill's element to the summon.
	if( hOwner->IsPlayerActor() )
	{
		if( hOwner->IsProcessSkill() )
		{
			DnSkillHandle hProcessSkill = hOwner->GetProcessSkill();
			//#52874
			//When summoning a monster via projectile, the currently used skill may differ from the monster summon skill.
			//If different from the monster summon skill, find and apply the skill using the input skill.
			if (hProcessSkill)
			{
				if (nSkillID != -1 && hProcessSkill->GetClassID() != nSkillID)
					hProcessSkill = hOwner->FindSkill(nSkillID);

				if (hProcessSkill)
				{
					CDnState::ElementEnum eSkillElement = hProcessSkill->GetElement();
					if( CDnState::ElementEnum::ElementEnum_Amount != eSkillElement )
						hSummonedMonster->PushForceHitElement( eSkillElement );
				}
			}
		}
	}

	//Set the creation timestamp.
	hSummonedMonster->SetRequestSummonTime(hOwner->GetLocalTime());

	// BLOW_290: AddStateOnSummonMonsterBlow (e.g. Gear Master Lv 65 passive)
	if( hOwner && hOwner->IsAppliedThisStateBlow( STATE_BLOW::BLOW_290 ) )
	{
		DNVector(DnBlowHandle) vlBlows290;
		hOwner->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_290, vlBlows290 );
		for( DWORD bi = 0; bi < vlBlows290.size(); ++bi )
		{
			if( vlBlows290[bi] )
				static_cast<CDnAddStateOnSummonMonsterBlow*>( vlBlows290[bi].GetPointer() )->OnSummonMonster( hSummonedMonster, pStruct->MonsterID );
		}
	}

#ifdef PRE_ADD_39644 // Grant invincibility to the summoned monster.
	if( pStruct->AddImmuneBlow )
		hSummonedMonster->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_099, -1 , "-1" );
#endif

	return hSummonedMonster;
}

#if defined(PRE_ADD_65808)
void CDnGameTask::ApplySummonMonsterGlyph(DnActorHandle hOwner, CDnMonsterActor* pMonsterActor, int nGlyphID)
{
	DNTableFileFormat* pGlyphTable = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pGlyphTable ) return;

	if (pMonsterActor == NULL)
		return;

	int nMonsterID = pMonsterActor ? pMonsterActor->GetMonsterClassID() : -1;

	char caLable[64];
	// Add state effect
	CDnSkill::StateEffectStruct StateEffect;
	StateEffect.nGlyphID = nGlyphID;
	for( int i = 0; i < CDnSkill::MAX_GLYPH_STATE_EFFECT_COUNT; ++i )
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		int iApplyType = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetInteger();
		bool bApplyAll = (CDnSkill::StateEffectApplyType::ApplyAll == iApplyType);		// Apply-to-all. Add 2 identical state effects with different targets.

		if( bApplyAll )
		{
			StateEffect.ApplyType = CDnSkill::StateEffectApplyType::ApplySelf;
			StateEffect.bApplyAllPair = true;
		}
		else
			StateEffect.ApplyType = (CDnSkill::StateEffectApplyType)iApplyType;

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pGlyphTable->GetFieldFromLablePtr( nGlyphID, caLable )->GetString();

		int nBlowID = -1;
		if (pMonsterActor)
			nBlowID = pMonsterActor->CmdAddStateEffect(NULL, (STATE_BLOW::emBLOW_INDEX)StateEffect.nID, -1, StateEffect.szValue.c_str() );

		if (nBlowID != -1)
			hOwner->AddSummonMonsterGlyphStateEffectID(nMonsterID, nGlyphID, pMonsterActor->GetUniqueID(), nBlowID);
	}
}
#endif // PRE_ADD_65808

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
void CDnGameTask::CalcStageDamageLimit( int nMapIndex )
{
	memset(&m_sStageDamageLimitStruct,0,sizeof(StageDamageLimitStruct));

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSTAGEDAMAGELIMIT );
	if( pSox )
	{
		for( int i=0; i<pSox->GetItemCount(); i++ ) 
		{
			int iItemID = pSox->GetItemID( i );
			int nFieldMapIndxe = pSox->GetFieldFromLablePtr( iItemID, "MapID" )->GetInteger();
			if( nFieldMapIndxe == nMapIndex )
			{
				m_sStageDamageLimitStruct.fDamageLimit = (float)pSox->GetFieldFromLablePtr( iItemID, "DamageLimit" )->GetInteger();
				m_sStageDamageLimitStruct.fStateLimit = (float)pSox->GetFieldFromLablePtr( iItemID, "AttackPowerLimit" )->GetInteger();
				return;
			}
		}
	}
}
#endif

#if defined( PRE_ADD_HEAL_TABLE_REGULATION )
void CDnGameTask::CalcStageHealLimit( int nMapIndex )
{
	memset( &m_sStageHealLimitStruct, 0, sizeof(m_sStageHealLimitStruct) );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSTAGEDAMAGELIMIT );
	if( pSox )
	{
		for( int i=0; i<pSox->GetItemCount(); i++ ) 
		{
			int iItemID = pSox->GetItemID( i );
			int nFieldMapIndxe = pSox->GetFieldFromLablePtr( iItemID, "MapID" )->GetInteger();
			if( nFieldMapIndxe == nMapIndex )
			{
				m_sStageHealLimitStruct.fHealLimit_Type1 = pSox->GetFieldFromLablePtr( iItemID, "HealingLimit_1" )->GetFloat();
				m_sStageHealLimitStruct.fHealLimit_Type2 = pSox->GetFieldFromLablePtr( iItemID, "HealingLimit_2" )->GetFloat();
				return;
			}
		}
	}
}
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
void CDnGameTask::ResetAlteiaWorldmap()
{	
	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		GetPartyData(i)->nUsableRebirthCoin = -1;
	}
	ResetDungeonClear( false );		
	ClearDungeonHistory();

	StopTimeAttack();

	for( DWORD i=0; i<GetUserCount(); i++ ) 
	{
		if( GetPartyData(i)->pSession->bIsAIBot() ) continue;
		// StageClear log needs verification
		GetPartyData(i)->pSession->CheckAndSendStageClearLog();
#if defined(PRE_ADD_ACTIVEMISSION)
		GetPartyData(i)->pSession->GetMissionSystem()->ResetActiveMission();
#endif
	}

	GetRoom()->SetStageStartLogFlag( false );

	for( DWORD i = 0; i < GetUserCount(); ++i )
	{
		if( GetPartyData(i)->pSession->GetActorHandle() )
			GetPartyData(i)->pSession->GetActorHandle()->ResetStateEffect( (int)CDnSkill::DurationTypeEnum::Debuff );
	}

	GetRoom()->SendRefreshParty(0, NULL);
}
#endif

#if defined( PRE_ADD_STAGE_WEIGHT )
const TStageWeightData * CDnGameTask::GetStageWeightData()
{
	return & m_StageWeightData;
}

void CDnGameTask::SetStageWeightData( int nStageWeightID )
{
	memset(&m_StageWeightData, 0, sizeof(m_StageWeightData));

	const TStageWeightData * pStageWeightData = g_pDataManager->GetStageWeigthData( nStageWeightID, GetUserCount() );
	if(pStageWeightData)
		m_StageWeightData = *pStageWeightData;
}
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )

#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
bool CDnGameTask::CheckDungeonClearAbuseTime(DWORD dwTime, int nMapID)
{
	if( dwTime > StageClearCheckTime::CheckMinTime ) return false;

	if( (nMapID == StageClearCheckTime::IgnoreMapIndex::MAP_CHAOSE_FIELD_24Lv_BossA) ||
		(nMapID == StageClearCheckTime::IgnoreMapIndex::MAP_CHAOSE_FIELD_32Lv_BossA) ||
		(nMapID == StageClearCheckTime::IgnoreMapIndex::MAP_CHAOSE_FIELD_BossA) )
		return false;

	return true;
}
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
