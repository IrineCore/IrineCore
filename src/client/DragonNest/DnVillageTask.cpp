#include "StdAfx.h"
#include "DnVillageTask.h"
#include "DnDebugConsole.h"
#include "DnWorld.h"
#include "DnFreeCamera.h"
#include "DnPlayerCamera.h"
#include "DnActor.h"
#include "DnActorClassDefine.h"
#include "DnLocalPlayerActor.h"
#include "MATransAction.h"
#include "DnGameTask.h"
#include "DnBridgeTask.h"
#include "TaskManager.h"
#include "DnMainFrame.h"
#include "EtSoundEngine.h"
#include "DnProjectile.h"
#include "DnMinimap.h"
#include "DnInterface.h"
#include "FrameSync.h"
#include "DnTableDB.h"
#include "DnLoadingTask.h"
#include "VillageSendPacket.h"
#include "PartySendPacket.h"
#include "SystemSendPacket.h"
#include "DnMessageBox.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "EtWorldSector.h"
#include "DnGateQuestionDlg.h"
#include "PerfCheck.h"
#include "DnPartyTask.h"
#include "DnItemTask.h"
#include "DnSkillTask.h"
#include "DnCharStatusDlg.h"
#include "EtUIMan.h"
#include "DnInterfaceDlgID.h"
#include "DnNPCActor.h"
#include "DnDataManager.h"
#include "DnNpcDlg.h"
#include "NpcSendPacket.h"
#include "DNCommonDef.h"
#include "EtSoundChannelGroup.h"
#include "FriendSendPacket.h"
#include "GuildSendPacket.h"
#include "DnCommonTask.h"
#include "DnInCodeResource.h"
#include "DnQuestTask.h"
#if defined(PRE_ADD_TALISMAN_SYSTEM)
#include "DnTalisman.h"
#endif
#include <mmsystem.h>
//blondymarry start
#include "GlobalValue.h"
//blondymarry end
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnItemTask.h"
#include "DnInspectPlayerDlg.h"
#include "DnIsolate.h"
#include "DnAppellationTask.h"
#include "DnPartsHair.h"
#include "DnMainMenuDlg.h"
#include "DnInterfaceCamera.h"
#include "DnServiceSetup.h"
#include "DnDropItem.h"
#include "DnGameTipTask.h"
#include "DNGestureTask.h"
#include "EtActionCoreMng.h"
#include "BugReporter.h"
#ifdef _USE_VOICECHAT
#include "GameOption.h"
#include "DNVoiceChatTask.h"
#endif
#ifdef PRE_ADD_VIP
#include "DnVIPDataMgr.h"
#include "DnInterfaceString.h"
#endif
#include "DnChatRoomTask.h"

#include "DnPartsVehicleEffect.h"
#include "DnVehicleTask.h"

#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillFishingTask.h"
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL

#include "DnGuildTask.h"

#include "DnPetTask.h"
#include "DnPetActor.h"
#include "DnRevengeTask.h"
#include "DnSecretCashShopDlg.h"

#ifdef _FINAL_BUILD
#else
#include "psapi.h"
#endif

#ifdef PRE_ADD_RELOAD_ACTFILE
#include "DnInCodeResource.h"
#endif 


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

bool CDnVillageTask::s_bCompleteFirstRender = true;
CDnVillageTask::CDnVillageTask()
: CInputReceiver( true )
{
	m_bFinalizeStage = true;
	m_pWorld = NULL;
	m_bEnterSuccess = false;
	m_bEnterLocal = false;
	m_bFirstConnect = true;
	m_bChannelMove = false;

	m_pMinimap = NULL;

	m_VillageType = WorldVillage;

	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );

	m_hDummyCamera = (new CDnInterfaceCamera)->GetMySmartPtr();
	CDnCamera::SetActiveCamera( m_hDummyCamera );

	m_bRequestCancelStageSelf = false;
	s_bCompleteFirstRender = false;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	m_bPreloadComplete = false;
#endif

	m_bIsCreateLocalActor = false;
#ifdef _FINAL_BUILD
#else
	m_fTimerSec = 60.f;
#endif
}

CDnVillageTask::~CDnVillageTask()
{
	FinalizeStage();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	ReleasePreloadPlayer();
#else
	CDnMainFrame::GetInstance().FlushPreloadPlayer();
#endif
	s_bCompleteFirstRender = true;
}

bool CDnVillageTask::Initialize( bool bFirstConnect, bool bChannelMove )
{
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false, bChannelMove );
	CDnPlayerActor::EnableAnotherPlayers( true );

	m_bFirstConnect = bFirstConnect;
	m_bChannelMove = bChannelMove;

	if( !m_bChannelMove ) CDnMouseCursor::GetInstance().ShowCursor( false, true );
	m_VillageType = WorldVillage;
	CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	if( pTask->GetChannelAttribute() & eChannelAttribute::CHANNEL_ATT_PVP ) m_VillageType = PvPVillage;
	else if( pTask->GetChannelAttribute() & eChannelAttribute::CHANNEL_ATT_FARMTOWN ) m_VillageType = FarmVillage;

//	SetPVPType(CGlobalInfo::GetInstance().GetIsPVPArea()); // Check GlobalInfo for PVP availability and set village accordingly
	SendVillageReady( bFirstConnect, bChannelMove );
	GetInterface().CloseSlideCaptionMessage();
	GetGameTipTask().ResetGameTip();

	if( m_bChannelMove ) {
		m_bEnterSuccess = false;
		m_bEnterLocal = false;
	}

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	// If not the first connection, clear active NPC reaction info whenever the map changes.
	if( false == bFirstConnect )
		GetQuestTask().ClearNpcReaction();
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	return true;
}

bool CDnVillageTask::InitializeStage( const char *szGridName, const char *szEnviName, int nMapTableID )
{
	DnDebugConsole::LogA( "INSTANCE", "Enter Village MapID=%d Grid=%s", nMapTableID, szGridName ? szGridName : "" );
	s_bCompleteFirstRender = false;
	GetInterface().CloseNpcDialog();
	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, 0.f, true );

	CDnPartyTask::GetInstance().ResetGateIndex();
	CDnPartyTask::GetInstance().OnInitializeVillageStage();
	m_pWorld = &CDnWorld::GetInstance();
	
	CGlobalInfo::GetInstance().m_nCurrentMapIndex = nMapTableID;
	CDnBridgeTask *pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	if (pBridgeTask)
		pBridgeTask->SetPreviousMapIndexForPvP(nMapTableID);

#ifdef PRE_ADD_RELOAD_ACTFILE
	CDnInCodeResource::GetInstance().Initialize();
#endif

	bool bResult = m_pWorld->Initialize( CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szGridName, this );
	if( bResult == false ) {
		DestroyTask( true );
		return false;
	}

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	g_DataManager.InitNpcTalkData( nMapTableID );
	GetQuestTask().LoadQuestTalk();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

	CDnActor::InitializeClass();
	CDnWeapon::InitializeClass();
	CDnEtcObject::InitializeClass();

	if( m_pWorld->ForceInitialize() == false ) {
		DestroyTask( true );
		return false;
	}

	SAFE_RELEASE_SPTR( m_hDummyCamera );
	m_hDebugCamera = (new CDnFreeCamera)->GetMySmartPtr();
	m_hPlayerCamera = (new CDnPlayerCamera)->GetMySmartPtr();

	CDnCamera::SetActiveCamera( m_hPlayerCamera );

	// Load environment map
	InitializeEnvi( nMapTableID, szGridName, szEnviName );

	GetInterface().UpdateMyPortrait();
	GetInterface().RefreshQuestSummaryInfo();
	
	m_pWorld->SetDragonNestType(eDragonNestType::None);

	if(CDnVehicleTask::IsActive())
		GetVehicleTask().RemoveInstantVehicleItem(); // Delete vehicle task's instant items when entering village.

	m_bFinalizeStage = false;	

//	if( m_pWorld ) m_pWorld->PlayBGM();
	return true;
}

void CDnVillageTask::FinalizeStage()
{
	m_bIsCreateLocalActor = false;

	if( m_bFinalizeStage == true ) return;
	DnDebugConsole::LogW( L"INSTANCE", L"Finalize Village Stage" );

	s_bCompleteFirstRender = false;
	GetCurRenderStack()->EmptyRenderElement();
	if( GetInterface().GetSecretCashShopAlarmBtnDlg() )
		GetInterface().GetSecretCashShopAlarmBtnDlg()->Show( false );
	if( GetInterface().GetSecretCashShopBuyDlg() )
		GetInterface().GetSecretCashShopBuyDlg()->Show( false );
	if( GetInterface().GetSecretCashShopMsgDlg() )
		GetInterface().GetSecretCashShopMsgDlg()->Show( false );
	GetInterface().Finalize( CDnInterface::Village );

	// Also reset when leaving village.
	if( CDnItemTask::IsActive() ) CDnItemTask::GetInstance().SetGuildWareReceivedState( false );

	SAFE_DELETE( m_pMinimap );
	SAFE_RELEASE_SPTR( m_hDebugCamera );
	SAFE_RELEASE_SPTR( m_hPlayerCamera );
	SAFE_RELEASE_SPTR( m_hDummyCamera );
	CDnActor::ReleaseClass();
	CDnWeapon::ReleaseClass( CDnWeapon::Projectile );
	CDnEtcObject::ReleaseClass();
	CDnCamera::ReleaseClass();

	if( m_pWorld ) m_pWorld->Destroy();
	m_bFinalizeStage = true;
	m_bEnterSuccess = false;
	m_bEnterLocal = false;
	m_bChannelMove = false;

#ifdef PRE_ADD_RELOAD_ACTFILE
	CDnInCodeResource::GetInstance().Finalize();
#endif

	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif
	if( CEtFontMng::IsActive() )
		CEtFontMng::GetInstance().FlushFontCache();

	CDnCommonTask* pCommonTask = static_cast<CDnCommonTask*>( CTaskManager::GetInstance().GetTask( "CommonTask" ) );
	if( pCommonTask )
		pCommonTask->SetCheckAttendanceFirst( false );

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	g_DataManager.ClearTalkData();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
}

bool CDnVillageTask::InitializeEnvi( int nMapIndex, const char *szGridName, const char *szEnviName )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	int nCameraFar = -1;
	if( pSox->IsExistItem( nMapIndex ) )
		nCameraFar = pSox->GetFieldFromLablePtr( nMapIndex, "_CameraFar" )->GetInteger();

	m_pWorld->InitializeEnviroment( CEtResourceMng::GetInstance().GetFullName( szEnviName ).c_str(), m_hPlayerCamera, nCameraFar );

	m_pMinimap = new CDnMinimap();
	m_pMinimap->Initialize();

	char szTemp[_MAX_PATH];
	sprintf_s( szTemp, "%s\\Grid\\%s\\%s.dds", CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szGridName, szGridName );
	m_pMinimap->SetTextures( szTemp );

	return true;
}

void CDnVillageTask::OnConnectTcp()
{
	CVillageClientSession::OnConnectTcp();
	OutputDebug( "ConnectTcp\n" );
}

void CDnVillageTask::OnDisconnectTcp( bool bValidDisconnect )
{
	CVillageClientSession::OnDisconnectTcp( bValidDisconnect );
	if( !bValidDisconnect ) {
		WriteLog( 0, ", Error, Disconnect Village Server\n" );
#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
		g_bInvalidDisconnect = true;
#endif
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, MESSAGEBOX_10, this );
#ifdef PRE_ADD_CL_DISCONNECT_LOG
		if(CDnActor::s_hLocalActor)
		{
			gs_BugReporter.AddLogW( L"CDnVillageTask::OnDisconnectTcp() - UserName[%s]", CDnActor::s_hLocalActor->GetName() );
			gs_BugReporter.ForceReporting();
		}
#endif // PRE_ADD_CL_DICONNECT_LOG

	}
}

void CDnVillageTask::OnRecvCharMapInfo( SCMapInfo *pPacket )
{
	FUNC_LOG();
	CVillageClientSession::OnRecvCharMapInfo( pPacket );

	if( !m_bChannelMove ) {
#ifdef PRE_ADD_RELOAD_ACTFILE
		CDnMainFrame::GetInstance().ReLoadActionFilePath();
#endif 
		// Set loading screen
		if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().Initialize( this, m_nVillageMapIndex );

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
		InitializePreloadPlayer();
#endif

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
		DNTableFileFormat* pWeatherSox = GetDNTable( CDnTableDB::TWEATHER );

		char szMapLable[64];
		char szEnviLable[64];
		sprintf_s( szMapLable, "_ToolName%d", m_nVillageMapArrayIndex + 1 );
		sprintf_s( szEnviLable, "_Envi%d_%d", m_nVillageMapEnviIndex + 1, m_nVillageMapEnviArrayIndex + 1 );

		int nWeatherID = pSox->GetFieldFromLablePtr( m_nVillageMapIndex, "_WeatherID" )->GetInteger();
		std::string szGridName = pSox->GetFieldFromLablePtr( m_nVillageMapIndex, szMapLable )->GetString();
#ifdef PRE_FIX_MEMOPT_EXT
		std::string szEnviName;
		CommonUtil::GetFileNameFromFileEXT(szEnviName, pWeatherSox, nWeatherID, szEnviLable);
#else
		std::string szEnviName = pWeatherSox->GetFieldFromLablePtr( nWeatherID, szEnviLable )->GetString();
#endif

		if( InitializeStage( szGridName.c_str(), szEnviName.c_str(), m_nVillageMapIndex ) == false ) {
			MessageBox( CDnMainFrame::GetInstance().GetHWnd(), _T("Initialize Stage Failed"), _T("Error"), MB_OK );
			CTaskManager::GetInstance().RemoveAllTask( false );
			return;
		}
		CDnWorld::GetInstance().SetMapType( (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr( m_nVillageMapIndex, "_MapType" )->GetInteger() );
		CDnWorld::GetInstance().SetMapSubType( (CDnWorld::MapSubTypeEnum)pSox->GetFieldFromLablePtr( m_nVillageMapIndex, "_MapSubType" )->GetInteger() );
		CDnWorld::GetInstance().SetLandEnvironment( (CDnWorld::LandEnvironment)pSox->GetFieldFromLablePtr( m_nVillageMapIndex, "_Environment" )->GetInteger() );
		CDnWorld::GetInstance().SetCurrentWeather( (CDnWorld::WeatherEnum)m_nVillageMapEnviIndex );
		CDnWorld::GetInstance().InitializeGateInfo( m_nVillageMapIndex, m_nVillageMapEnviArrayIndex );
		CDnWorld::GetInstance().SetAllowMapType( pSox->GetFieldFromLablePtr( m_nVillageMapIndex, "_AllowMapType" )->GetInteger() );

		CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pCommonTask ) pCommonTask->ResetPlayedCutScene();

		GetInterface().RefreshQuestSummaryInfo();
	}
	else {
		for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ ) {
			if( CDnActor::s_pVecProcessList[i] == CDnActor::s_hLocalActor.GetPointer() ) continue;

			// Rotha: Vehicle system does not delete one's own vehicle.
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if(pPlayer && pPlayer->IsPlayerActor() && pPlayer->IsVehicleMode() && CDnActor::s_pVecProcessList[i] == pPlayer->GetMyVehicleActor()) continue;

			if( CDnActor::s_pVecProcessList[i]->IsNpcActor() ) {
				CDnNPCActor *pNpc = dynamic_cast<CDnNPCActor*>(CDnActor::s_pVecProcessList[i]);
				if( pNpc && pNpc->IsNpcProp() ) continue;
			}

			if( pPlayer && pPlayer->IsSummonPet() && pPlayer->GetMyPetActor() == CDnActor::s_pVecProcessList[i] ) continue;

			CDnActor::s_pVecProcessList[i]->Release();
			i--;
		}
	}

	m_bEnterSuccess = true;
	SendEnter();
	OutputDebug( "VillageMapInfo\n" );
}

void CDnVillageTask::OnRecvCharEntered( SCEnter *pPacket )
{
	FUNC_LOG();
	OutputDebug( "UserEnter - 1\n" );
	CVillageClientSession::OnRecvCharEntered( pPacket );

	if( !m_bEnterSuccess ) {
		assert(0&&"OnVillageEntered");
	}

	if( m_bChannelMove ) {
		m_bEnterLocal = true;
		if( CDnLoadingTask::IsActive() ) {
			CDnLoadingTask::GetInstance().EnableBackgroundLoading( true );
			/*CDnLoadingTask::GetInstance().WaitForComplete();*/
		}
		GetInterface().SetChannelID( GetChannelIdx() );
		GetInterface().RefreshStageInfoDlg();
		SendCompleteLoading();
		return;
	}
	CInputDevice::ResetInverseMode();

	// Player Load
	DnActorHandle hPlayer = CreateActor( pPacket->cClassID, true );
	CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor *)hPlayer.GetPointer();
	CDnActor::s_hLocalActor = hPlayer;
	m_bIsCreateLocalActor = true;

	hPlayer->SetUniqueID( CDnBridgeTask::GetInstance().GetSessionID() );
	hPlayer->Initialize();

	// Rotha :: Vehicle UniqueID
	pActor->SetVehicleSessionID(pPacket->nVehicleObjectID);

	// Job must be set first. Definitely (since EXP and other lookups were changed to use the job..)
	std::vector<int> nVecJobList;
	for( int i=0; i<JOBMAX; i++ ) {
		if( pPacket->cJobArray[i] == 0 ) break;
		nVecJobList.push_back( pPacket->cJobArray[i] );
	}
	pActor->SetJobHistory( nVecJobList );

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	pActor->RecomposeAction();
#endif

	// Update skill point usage limit per job
	static_cast<CDnLocalPlayerActor*>(pActor)->UpdateSPUsingLimitByJob();

	pActor->InitializeIdleMotion();

	CGlobalInfo::GetInstance().m_cLocalAccountLevel = pPacket->cAccountLevel;
	pActor->SetAccountLevel( pPacket->cAccountLevel );
	pActor->SetAccountHide( true );

	hPlayer->SetLevel( pPacket->cLevel );
	pActor->SetExperience( pPacket->nExp );
	pActor->SetPvPLevel( pPacket->sPvPInfo.cLevel );

	if( pPacket->Position.y == 0.f ) {
		pPacket->Position.y = CDnWorld::GetInstance().GetHeight( pPacket->Position );
	}
	else {
		float fHeight = CDnWorld::GetInstance().GetHeight( pPacket->Position );
		if( fabs( pPacket->Position.y - fHeight ) < 3.f ) pPacket->Position.y = fHeight;
	}

	EtVector2 vDir( sin( EtToRadian( pPacket->fRotate ) ), cos( EtToRadian( pPacket->fRotate ) ) );
	pActor->Look( vDir );

	pActor->InitializeSession( this );

	pActor->SetName( pPacket->wszCharacterName );
	pActor->SetPartsColor( MAPartsBody::HairColor, pPacket->dwHairColor );
	pActor->SetPartsColor( MAPartsBody::EyeColor, pPacket->dwEyeColor );
	pActor->SetPartsColor( MAPartsBody::SkinColor, pPacket->dwSkinColor );

	((CDnPlayerCamera*)m_hPlayerCamera.GetPointer())->AttachActor( hPlayer );
	m_hLocalPlayerActor = hPlayer;

	((CDnPlayerCamera*)m_hPlayerCamera.GetPointer())->ResetCamera();

	if( CDnPartyTask::IsActive() ) {
		CDnPartyTask::GetInstance().SetLocalData( hPlayer );
		CDnPartyTask::GetInstance().SetPvPInfo( &pPacket->sPvPInfo );
	}

	// Set guild info
	// Set before guild self view.
	GetGuildTask().SetCurrentGuildWarEventStep( pPacket->cCurrentGuildWarEventStep );
	GetGuildTask().SetCurrentGuldWarScheduleID( pPacket->wCurrentGuldWarScheduleID );
	pActor->SetGuildSelfView( pPacket->GuildSelfView );

	// Interface Initialize
	GetInterface().Initialize( CDnInterface::Village, this );

	// Note: Request friend and guild lists
	//
	SendReqFriendList();
	GetIsolateTask().ReqIsolateList(true);

	CDnItemTask::GetInstance().RemoveAllEquipItem();

	pActor->SetWeaponViewOrder( 0, GetBitFlag(pPacket->cViewCashEquipBitmap, CASHEQUIP_WEAPON1) );
	pActor->SetWeaponViewOrder( 1, GetBitFlag(pPacket->cViewCashEquipBitmap, CASHEQUIP_WEAPON2) );

	for( int i = 0; i < CDnParts::SwapParts_Amount; ++i )
		pActor->SetPartsViewOrder( i, GetBitFlag(pPacket->cViewCashEquipBitmap, i) );

	pActor->SetHideHelmet( GetBitFlag(pPacket->cViewCashEquipBitmap, HIDEHELMET_BITINDEX) );

	pActor->SetBattleMode( true );
	int nDefaultPartsInfo[4] = { pPacket->nDefaultBody, pPacket->nDefaultLeg, pPacket->nDefaultHand, pPacket->nDefaultFoot };
	pActor->SetDefaultPartsInfo( nDefaultPartsInfo );
	pActor->SetBattleMode( false );
	hPlayer->ResetActor();

	pActor->SetPosition( pPacket->Position );
	pActor->SetVelocityY( -1000.f );
	pActor->SetResistanceY( -1000.f );
//	pActor->Process( 0, 0.f );

	//hPlayer->ResetToggleSkill();
	//hPlayer->ResetAuraSkill();

	if( !m_bFirstConnect && CDnSkillTask::IsActive() ) {
		CDnSkillTask::GetInstance().InitializeLocalPlayerSkills();
	}

#ifdef PRE_ADD_BESTFRIEND
	GetItemTask().InitBestfriendInfo();
#endif

	CDnAppellationTask::GetInstance().SelectAppellation( pPacket->nCoverAppellation, pPacket->nAppellation );

	pActor->RefreshState();
	pActor->SetHP( pActor->GetMaxHP() );
	pActor->SetSP( pActor->GetMaxSP() );
	pActor->Process( 0, 0.f );
	ProcessSound( 0, 0.f ); // Must do this once to prevent sound from momentarily playing at (0, 0, 0).
	if( m_pWorld ) m_pWorld->PlayBGM();

	for(int n=0;n<DualSkill::Type::MAX;n++)
		GetSkillTask().SetSkillPoint( pPacket->wSkillPoint[n] , n );

	GetSkillTask().SetSkillPage((int)pPacket->cSkillPage);
	

	m_bEnterLocal = true;
	ResetTimer();
	CInputDevice::GetInstance().ResetAllInput();
	CDnLocalPlayerActor::LockInput( false );
	OutputDebug( "Local Entered\n" );

#ifdef PRE_ADD_BEGINNERGUILD
	CDnPartyTask::GetInstance().RefreshKeepPartyInfo( 0, 0, false, 0 );
#else
	CDnPartyTask::GetInstance().RefreshKeepPartyInfo( 0, 0, false );
#endif
	CDnItemTask::GetInstance().GetCharInventory().RefreshInventory();
	CDnItemTask::GetInstance().RefreshEquip();
	CDnItemTask::GetInstance().SetCoin(pPacket->nCoin);
	CDnItemTask::GetInstance().SetMaxUsableRebirthCoin( -1 );
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eDAILYCOIN, pPacket->cRebirthCoin);
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eCASHCOIN, pPacket->wRebirthCashCoin);
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::ePCBANGCOIN, pPacket->cPCBangRebirthCoin);
#ifdef PRE_ADD_VIP
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eVIPCOIN, pPacket->cVIPRebirthCoin);
#endif // PRE_ADD_VIP

#ifdef PRE_FIX_NESTREBIRTH
	CDnItemTask::GetInstance().SetUpdateRebirthCoinOrItem(true);
#endif

	#ifdef PRE_MONITOR_SUPER_NOTE
		CDnItemTask::GetInstance().AddIllegalActivityMonitor(eIAMA_MONEY, 60.f);
	#endif // PRE_MONITOR_SUPER_NOTE

	CDnItemTask::GetInstance().SetFatigue( pPacket->wFatigue, pPacket->wWeekFatigue, pPacket->wPCBangFatigue, pPacket->wEventFatigue );

	if( m_bFirstConnect )
	{
		CDnItemTask::GetInstance().SetMaxFatigue( CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::Fatigue ),
			CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::WeekFatigue ),
			0,
			pPacket->wMaxEventFatigue );
	}

#ifdef PRE_ADD_VIP
	CDnItemTask::GetInstance().SetFatigue(CDnItemTask::eVIPFTG, pPacket->wVIPFatigue);
	CDnItemTask::GetInstance().SetMaxFatigue(CDnItemTask::eVIPFTG, CPlayerLevelTable::GetInstance().GetValue(pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::VIPFatigue));
#endif

	if( m_bFirstConnect && CDnGestureTask::IsActive() )
		GetGestureTask().RefreshGestureList( pPacket->cLevel );

	// Reset every time on entering village.
	CDnItemTask::GetInstance().SetGuildWareReceivedState( false );

	GetInterface().ChangeSkillPage((int)pPacket->cSkillPage);
	GetInterface().RefreshSkillDialog();
	GetInterface().UpdateMyPortrait();
	// The value used to display the current channel (Mana Ridge 1, 2) in the channel move dialog is ChannelIdx.
	GetInterface().SetChannelID( GetChannelIdx() );
	GetInterface().RefreshStageInfoDlg();

	OutputDebug( "UserEnter - 2\n" );

	if( CDnLoadingTask::IsActive() ) {		
		CDnLoadingTask::GetInstance().EnableBackgroundLoading( true );
		CDnLoadingTask::GetInstance().WaitForComplete();
	}
	SendCompleteLoading();

	CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, 0.f, false );
	GetInterface().FadeDialog( 0xff000000, 0x00000000, 3.f, this );

	GetPartyTask().JoinReservedParty();

	GetQuestTask().UpdateQuestNotifierInfo(true);

	m_pMinimap->InitializeNPC( m_nVillageMapIndex );

#if defined(_WORK)
	if( m_bFirstConnect ) {
		CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pCommonTask ) pCommonTask->ResetPlayTimeNotice();
	}
#endif


#ifdef _USE_VOICECHAT
	if( CGameOption::GetInstance().m_bVoiceChat ) {
		if( CDnVoiceChatTask::IsActive() ) {
			GetVoiceChatTask().ReqVoiceAvailable( true );
		}
	}
#endif

	if ( 1 == (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::EventToday) )
#ifdef PRE_ADD_START_POPUP_QUEUE
	{
		CDnStartPopupMgr& mgr = GetInterface().GetStartPopupMgr();
		mgr.RegisterTop(CDnStartPopupMgr::eEventToday, NULL);

		mgr.StartShow();
	}
#else
		GetInterface().ShowEventToday();
#endif

#ifdef PRE_ADD_VIP
	if (CDnLocalPlayerActor::s_hLocalActor)
	{
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if (pLocalActor != NULL)
		{
			CDnVIPDataMgr& dataMgr = pLocalActor->GetVIPDataMgr();
			dataMgr.SetData(pPacket->tVIPExpirationDate, pPacket->bVIPAutomaticPay, pPacket->nVIPPoint, pPacket->bVIP);

			const __time64_t* pDate = dataMgr.GetVIPExpireDate();
			if (dataMgr.IsVIP() && pDate)
			{
				std::wstring temp, temp1, dayText;
				DN_INTERFACE::STRING::GetDayText(dayText, *pDate);
				temp1 = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020025), dayText.c_str());			// UISTRING : VIP service expires on %s.
				temp = FormatW(L"%s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020024), temp1.c_str());	// UISTRING : VIP service is currently active.

				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", temp.c_str(), false);
			}
		}
	}
#endif // PRE_ADD_VIP

#ifdef PRE_ADD_NAMEDITEM_SYSTEM
	if( pPacket->nNamedItemID > 0 )
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNAMEDITEM );
		if( pSox )
		{
			for( int i=0; i<pSox->GetItemCount(); ++i )
			{
				int nIndex = pSox->GetItemID( i );
				int nItemIndex = pSox->GetFieldFromLablePtr( nIndex, "_NamedItem_ID" )->GetInteger();
				if( nItemIndex == pPacket->nNamedItemID )
				{
					std::string strLoopEffectName = pSox->GetFieldFromLablePtr( nIndex , "_Loop_Effect" )->GetString();
					pActor->ToggleLinkedPlayerEffect( true , strLoopEffectName.c_str() );
					break;
				}
			}
		}
	}
#endif

	if( CDnMainFrame::IsActive() )
		CDnMainFrame::GetInstance().FlushWndMessage();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif


#ifdef PRE_MOD_PVPOBSERVER
	// #64903 : Ignore exit message when re-entering Colosseum room after being moved to town due to AFK in event room.
	CGlobalInfo::GetInstance().SetGlobalMessageCode( 0 );
#endif // PRE_MOD_PVPOBSERVER

#if defined( PRE_ADD_PRESET_SKILLTREE )
	GetSkillTask().SendPresetSkillTreeList();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

#ifdef PRE_ADD_BEGINNERGUILD
	if( GetGuildTask().CheckMessageBoxForGraduateBeginnerGuild() )
		GetGuildTask().MessageBoxForGraduateBeginnerGuild();
#endif


#ifdef PRE_ADD_STAMPSYSTEM
	//// Display the stamp window if max level.
	//CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	//if( pTask )
	//{		
	//	if( CDnActor::s_hLocalActor && pTask->IsShowStampDlg() )
	//	{
	//		int nLimitLevel = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
	//		if( CDnActor::s_hLocalActor->GetLevel() == nLimitLevel )
	//			GetInterface().ShowStampDlg();
	//	} 
	//	
	//	pTask->SetStampDlg( false );
	//}
#endif // PRE_ADD_STAMPSYSTEM

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	GetInterface().SetAlteaDiceIcon();
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

	if( GetInterface().GetSecretCashShopBuyDlg() && GetInterface().GetSecretCashShopBuyDlg()->GetSecretItemCount() > 0 )
	{
		if( GetInterface().GetSecretCashShopAlarmBtnDlg() )
			GetInterface().GetSecretCashShopAlarmBtnDlg()->Show( true );
	}
}

void CDnVillageTask::OnRecvCharUserEntered( SCEnterUser *pPacket, int nSize )
{
	CVillageClientSession::OnRecvCharUserEntered( pPacket, nSize );

	if( !m_bEnterSuccess ) {
		assert(0&&"OnVillageUserEntered");
	}
	OutputDebug( "Enter User : %d\n", pPacket->nSessionID );

	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().InsertLoadObject( 
			OnLoadRecvCharUserEnteredCallback, 
			OnLoadRecvCharUserEnteredUniqueID, 
			OnCheckLoadingPacket, 
			OnLoadRecvCharUserEnteredCheckSkip,
			(void*)this, (void*)pPacket, nSize, m_LocalTime, CDnLoadingTask::CreateActor );
	}
}

int __stdcall CDnVillageTask::OnLoadRecvCharUserEnteredUniqueID( void *pParam, int nSize )
{
	return ((SCEnterUser *)pParam)->nSessionID;
}

bool __stdcall CDnVillageTask::OnLoadRecvCharUserEnteredCheckSkip( int nMainCmd, int nSubCmd )
{
	if( nMainCmd == SC_CHAR && nSubCmd == eChar::SC_LEAVEUSER ) return true;
	return false;
}

bool __stdcall CDnVillageTask::OnLoadRecvCharUserEnteredCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	SCEnterUser *pPacket = (SCEnterUser *)pParam;
	CDnVillageTask *pTask = (CDnVillageTask*)pThis;
	// Player Load
	DnActorHandle hPlayer = CreateActor( pPacket->cClassID, false, false, false );
	if( !hPlayer ) return false;
	hPlayer->Show( false );
	hPlayer->SetSoftAppear( true );

	hPlayer->Initialize();
	hPlayer->SetUniqueID( pPacket->nSessionID );
	CDnPlayerActor *pActor = (CDnPlayerActor *)hPlayer.GetPointer();

	pActor->SetAccountLevel( pPacket->cAccountLevel );
	pActor->SetLevel( pPacket->cLevel );

	// Note: When someone enters view in town, only their latest job is set here, not their job history.
	// If needed later, history must be configured..
	pActor->SetJobHistory( (int)pPacket->cJob ); 
	pActor->SetPvPLevel( pPacket->cPvPLevel );

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	pActor->RecomposeAction();
#endif

	pActor->SetWeaponViewOrder( 0, GetBitFlag(pPacket->cViewCashEquipBitmap, CASHEQUIP_WEAPON1) );
	pActor->SetWeaponViewOrder( 1, GetBitFlag(pPacket->cViewCashEquipBitmap, CASHEQUIP_WEAPON2) );

	for( int i = 0; i < CDnParts::SwapParts_Amount; ++i )
		pActor->SetPartsViewOrder( i, GetBitFlag(pPacket->cViewCashEquipBitmap, i) );

	pActor->SetHideHelmet( GetBitFlag(pPacket->cViewCashEquipBitmap, HIDEHELMET_BITINDEX) );

	int nDefaultPartsInfo[4] = { pPacket->nDefaultBody, pPacket->nDefaultLeg, pPacket->nDefaultHand, pPacket->nDefaultFoot };
	pActor->SetDefaultPartsInfo( nDefaultPartsInfo );
	pActor->SetPartsColor( MAPartsBody::HairColor, pPacket->dwHairColor );
	pActor->SetPartsColor( MAPartsBody::EyeColor, pPacket->dwEyeColor );
	pActor->SetPartsColor( MAPartsBody::SkinColor, pPacket->dwSkinColor );

	TItemInfo *pInfo = &pPacket->EquipArray[0];

#ifdef PRE_ADD_BESTFRIEND
	pActor->SetBFserial( pPacket->biBestFriendItemSerial );
	pActor->SetBestfriendName( pPacket->wszBestFriendName );
	//pActor->SetAppellationName( pPacket->wszBestFriendName );
#endif

	pActor->SetIgnoreCombine( true );
	for ( int i = 0; i < pPacket->cCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}
		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( !pItem ) {
			pInfo++;
			continue;
		}

		if( pInfo->cSlotIndex < EQUIP_WEAPON1) {
			if( pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex, true );
			}
		}
		else {
			if( pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				pActor->AttachWeapon( hWeapon, pInfo->cSlotIndex - EQUIP_WEAPON1, true );
			}
		}
		pInfo++;
	}

	for( int i=0; i<pPacket->cCashCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}
		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( !pItem ) {
			pInfo++;
			continue;
		}

		if( pInfo->cSlotIndex < CASHEQUIP_WEAPON1) {
			if( pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex, true );
			}
		}
		else {
			if( pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				pActor->AttachCashWeapon( hWeapon, pInfo->cSlotIndex - CASHEQUIP_WEAPON1, true, false );
			}
			else if(pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
			{
				DnPartsVehicleEffectHandle hVehicleEffect= ((CDnPartsVehicleEffect*)pItem)->GetMySmartPtr();
				pActor->SetPartsVehicleEffect(hVehicleEffect, true);
			}

		}
		pInfo++;
	}
	pActor->SetIgnoreCombine( false );

	pActor->RefreshHideHelmet();

	for( int i=0; i<pPacket->cGlyphCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}
		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( !pItem ) {
			pInfo++;
			continue;
		}
		if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
			DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
			if( pActor ) pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)pInfo->cSlotIndex, true );
		}
		pInfo++;
	}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	for( int i=0; i<pPacket->cTalismanCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}
		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( !pItem ) {
			pInfo++;
			continue;
		}
		if( dynamic_cast<CDnTalisman*>(pItem) && pItem->GetItemType() == ITEMTYPE_TALISMAN ) {
			DnTalismanHandle hTalisman = ((CDnTalisman*)pItem)->GetMySmartPtr();
			if( pActor ) pActor->AttachTalisman( hTalisman, pInfo->cSlotIndex, CDnItemTask::GetInstance().GetTalismanEfficiency(pInfo->cSlotIndex), true );
		}
		pInfo++;
	}
#endif


	pActor->SetPosition( pPacket->Position );

	EtVector2 vDir( sin( EtToRadian( pPacket->fRotate ) ), cos( EtToRadian( pPacket->fRotate ) ) );
	pActor->Look( vDir );

	pActor->InitializeSession( pTask );
	// note by kalliste : Moving forward because equip item error checking uses pActor name. Let me know if issues arise.
	pActor->SetName( pPacket->wszCharacterName );

	pActor->SetBattleMode( pPacket->bBattleMode );
	pActor->SetActionQueue( "Stand" );
	pActor->SetAppellationIndex( pPacket->nCoverAppellation, pPacket->nAppellation );
	pActor->RefreshState();
	pActor->SetHP( pActor->GetMaxHP() );
	pActor->SetSP( pActor->GetMaxSP() );

	pActor->CombineParts();

	bool bShow = true;
	switch( pPacket->cAccountLevel ) {
		case AccountLevel_New:
		case AccountLevel_Monitoring:
		case AccountLevel_Master:
		case AccountLevel_QA:
			if( pPacket->bHide ) bShow = false;
			break;
	}

#ifdef PRE_ADD_VIP
	pActor->SetVIP(pPacket->bVIP);
#endif

	pActor->SetGuildSelfView( pPacket->GuildSelfView );


	GetChatRoomTask().OnCharUserEntered( hPlayer );
	
	pActor->SetVehicleSessionID(pPacket->nVehicleObjectID); // Character holds a unique ID for the mount.
	if( pPacket->VehicleEquip.Vehicle[Vehicle::Slot::Body].nItemID > 0 )
	{
		TVehicleCompact VehicleInfo;
		VehicleInfo.dwPartsColor1 = pPacket->VehicleEquip.dwPartsColor1;
		for(int i=0; i<Vehicle::Slot::Max; i++)
		{
			VehicleInfo.Vehicle[i].SetItem( pPacket->VehicleEquip.Vehicle[i] );
		}
		pActor->SetVehicleInfo(VehicleInfo);
		pActor->RideVehicle(VehicleInfo,true); // ! Run immediately in this case !

		// When a GM enters town mounted, do not show them and do not run Process
		if( pActor->GetMyVehicleActor() ) pActor->GetMyVehicleActor()->Show( bShow );
		if( pActor->GetMyVehicleActor() ) pActor->GetMyVehicleActor()->SetProcess( bShow );
	}

	if( pPacket->PetEquip.Vehicle[Pet::Slot::Body].nItemID > 0 )
	{
		TVehicleCompact stPetInfo;
		stPetInfo.dwPartsColor1 = pPacket->PetEquip.dwPartsColor1;
		stPetInfo.dwPartsColor2 = pPacket->PetEquip.dwPartsColor2;
		for( int i=0; i<Pet::Slot::Max; i++ )
		{
			stPetInfo.Vehicle[i].SetItem( pPacket->PetEquip.Vehicle[i] );
		}
		
		swprintf_s( stPetInfo.wszNickName, NAMELENMAX, pPacket->PetEquip.wszNickName );
		GetPetTask().OnRecvPetEquipList( pActor, stPetInfo );

		// When a GM enters town, do not show them and do not run Process
		if( pActor->GetMyPetActor() ) pActor->GetMyPetActor()->Show( bShow );
		if( pActor->GetMyPetActor() ) pActor->GetMyPetActor()->SetProcess( bShow );
	}

	if( 0 < pPacket->nSourceItemID ) 	// Definitely not a game server here, so set to false.
		pActor->AddEffectSkill( pPacket->nSourceItemID, 0, 0,  false , true );

#ifdef PRE_ADD_NAMEDITEM_SYSTEM
	if( pPacket->nNamedItemID > 0 )
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNAMEDITEM );
		if( pSox )
		{
			for( int i=0; i<pSox->GetItemCount(); ++i )
			{
				int nIndex = pSox->GetItemID( i );
				int nItemIndex = pSox->GetFieldFromLablePtr( nIndex, "_NamedItem_ID" )->GetInteger();
				if( nItemIndex == pPacket->nNamedItemID )
				{
					std::string strLoopEffectName = pSox->GetFieldFromLablePtr( nIndex , "_Loop_Effect" )->GetString();
					pActor->ToggleLinkedPlayerEffect( true , strLoopEffectName.c_str() );
					break;
				}
			}
		}
	}
#endif

#ifdef PRE_ADD_TRANSFORM_POTION
	if( pPacket->nTransformID > 0 )
		pActor->SetRefrehVillageTransformMode( pPacket->nTransformID );
#endif

	pActor->SetPartyLeader(pPacket->bIsPartyLeader);

	hPlayer->Show( bShow );
	pActor->SetProcess( bShow );

	return true;
}


void CDnVillageTask::OnRecvCharUserLeave( SCLeaveUser *pPacket )
{
	OutputDebug( "Leave User : %d\n", pPacket->nSessionID );
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) {
		// If an admin and not inside SetProcess, hActor can be invalid. Guard against this case.
		// Search all SmartPtrHandles to find it.		
		//if( pPacket->cAccountLevel >= AccountLevel_New ) {
		if( pPacket->cAccountLevel >= AccountLevel_New && pPacket->cAccountLevel < AccountLevel_DWC ) {
			hActor = CDnActor::FindActorFromUniqueIDBySmartPtrList( pPacket->nSessionID );
			if( hActor ) {
				// Rotha: Case where GM exits while riding a horse
				CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
				if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
				{
					pPlayer->GetMyVehicleActor()->SetProcess(true);
					pPlayer->GetMyVehicleActor()->SetDestroy();
				}

				hActor->SetDestroy();
				hActor->SetProcess(true); // GMs do not run Process in town, 
				return;
			}
		}
		if( !hActor ) {
			assert(0);
			return;
		}
	}
	switch( pPacket->cLeaveType ) {
		case eLeaveType::LEAVE_LOGOUT:
			{
				if( pPacket->cAccountLevel < AccountLevel_New ) {
					DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
					if( hHandle ) {
						hHandle->SetPosition( *hActor->GetPosition() );
						hHandle->SetActionQueue( "Logout" );
					}
				}
			}
			break;
	}

	GetChatRoomTask().OnCharUserLeaved( hActor );

	// Rotha Mount system
	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
	{
		pPlayer->GetMyVehicleActor()->SetDestroy();
	}

	if(pPlayer)
		pPlayer->CancelCallVehicle(); // Exception handling because the process doesn't die immediately upon SetDestroy state: cancel if leaving mid-call.

	hActor->SetDestroy();

	bool bLocalPlayer = false;

	if( hActor == CDnActor::s_hLocalActor ) bLocalPlayer = true;
	if( bLocalPlayer )
	{
		GetInterface().Finalize( CDnInterface::Village );
	}
}

void CDnVillageTask::OnRecvCharHide( SCHide *pPacket )
{
	if( CDnLocalPlayerActor::s_hLocalActor && pPacket->nSessionID == CDnLocalPlayerActor::s_hLocalActor->GetUniqueID() ) {
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if( pLocalActor->IsDeveloperAccountLevel() )
			pLocalActor->SetAccountHide( pPacket->bHide );
	}
	else {
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
		if( !hActor ) {
			hActor = CDnActor::FindActorFromUniqueIDBySmartPtrList( pPacket->nSessionID );
			if( !hActor ) return;
		}
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
		if( !pPlayer ) return;

		if( hActor->IsProcess() == !pPacket->bHide ) return;
		pPlayer->Show( !pPacket->bHide );
		pPlayer->SetProcess( !pPacket->bHide );
		if( pPlayer->IsSummonPet() )
			pPlayer->HideSummonPet( pPacket->bHide );
	}
}

#ifdef PRE_PARTY_DB
bool CDnVillageTask::OnRecvPartyListInfo( SCPartyListInfo *pPacket )
{
	if (CVillageClientSession::OnRecvPartyListInfo( pPacket ))
	{
		GetInterface().RefreshPartyList();
		return true;
	}

	return false;
}
#else
void CDnVillageTask::OnRecvPartyListInfo( SCPartyListInfo *pPacket )
{
	CVillageClientSession::OnRecvPartyListInfo( pPacket );
	GetInterface().RefreshPartyList();
}
#endif

void CDnVillageTask::OnRecvPartyMemberInfo( SCPartyMemberInfo *pPacket )
{
	CVillageClientSession::OnRecvPartyMemberInfo( pPacket );
	GetInterface().RefreshPartyMemberList();
}

void CDnVillageTask::OnRecvPartyCreateParty( SCCreateParty *pPacket )
{
	CVillageClientSession::OnRecvPartyCreateParty( pPacket );

	switch( pPacket->nRetCode ) 
	{
	case ERROR_NONE: 
		GetInterface().SwapPartyDialog();
		//GetInterface().RefreshPartyInfoList();
		break;
	default: 
		// GetInterface().MessageBox( MESSAGEBOX_16, MB_OK ); 
		GetInterface().ServerMessageBox( pPacket->nRetCode );
		break;
	}
}

void CDnVillageTask::OnRecvPartyRefresh( SCRefreshParty *pPacket )
{
	CVillageClientSession::OnRecvPartyRefresh( pPacket );

	GetInterface().RefreshPartyInfoList();
	GetInterface().RefreshPartyGuage();
}

void CDnVillageTask::OnRecvPartyJoinParty( SCJoinParty *pPacket )
{
	CVillageClientSession::OnRecvPartyJoinParty( pPacket );

	switch( pPacket->nRetCode ) 
	{
	case ERROR_NONE:
		{
			GetInterface().RefreshPartyGuage();
		}
		break;
	default:
		{
			GetInterface().ServerMessageBox( pPacket->nRetCode, MB_OK, PARTY_JOIN_FAIL_DIALOG, this );
		}
		break;
	}
}

void CDnVillageTask::OnRecvPartyOutParty( SCPartyOut *pPacket )
{
	CVillageClientSession::OnRecvPartyOutParty( pPacket );

	switch( pPacket->nRetCode )
	{
	case ERROR_NONE:
		GetInterface().RefreshPartyGuage();
		GetInterface().SwapPartyDialog();
		break;
	case ERROR_PARTY_KICKOK:
		GetInterface().RefreshPartyGuage();
		GetInterface().SwapPartyDialog();
		GetInterface().ServerMessageBox( pPacket->nRetCode );
		break;
	default:
		GetInterface().ServerMessageBox( pPacket->nRetCode );
		break;
	}
}

void CDnVillageTask::OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket )
{
	if( m_bEnterLocal == false ) return;
	CVillageClientSession::OnRecvPartyRefreshGateInfo( pPacket );

	GetInterface().RefreshPartyGateInfo();
}

void CDnVillageTask::OnRecvPartyReadyGate( SCGateInfo *pPacket )
{
	CVillageClientSession::OnRecvPartyReadyGate( pPacket );

	std::vector<CDnWorld::GateStruct *> vecGateList;
	m_pWorld->GetGateStructList( pPacket->cGateNo, vecGateList );

	if( false == vecGateList.empty() ) 
	{
		GetInterface().ShowWorldZoneSelectDialog( true, pPacket->cGateNo, this );

#ifdef PRE_ADD_COOKING_SYSTEM
		if( CDnLifeSkillCookingTask::IsActive() && GetLifeSkillCookingTask().IsNowCooking() )
			GetLifeSkillCookingTask().CancelManufacture();
#endif // PRE_ADD_COOKING_SYSTEM
	}

	/*
	m_hLocalPlayerActor->ResetCustomAction();
	m_hLocalPlayerActor->ResetMixedAnimation( false );
	if( m_hLocalPlayerActor->IsMove() || m_hLocalPlayerActor->IsStay() ) {
		m_hLocalPlayerActor->CmdStop( "Stand" );
	}
	*/
	CDnLocalPlayerActor::StopAllPartyPlayer();

	CDnLocalPlayerActor::LockInput( true );
}

void CDnVillageTask::OnRecvPartyStageCancel( SCCancelStage *pPacket )
{
	CVillageClientSession::OnRecvPartyStageCancel( pPacket );

	//CDnLocalPlayerActor::LockInput( false );
	GetInterface().ShowWorldZoneSelectDialog( false );

	if ( pPacket->nRetCode != ERROR_NONE )
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
		CDnLocalPlayerActor::LockInput( false );

		if( CDnActor::s_hLocalActor )
		{
			// #48606 Re-enabled the blocked Cash Shop.
			GetInterface().DisableCashShopMenuDlg( false );

			if (CDnPartyTask::IsActive())
				CDnPartyTask::GetInstance().OnCancelStage();
		}
	}
	else
	{
		if (m_bRequestCancelStageSelf && CDnActor::s_hLocalActor )
		{
			if (CDnPartyTask::IsActive())
				CDnPartyTask::GetInstance().OnCancelStage();
			m_bRequestCancelStageSelf = false;
		}
		else
		{
			CDnLocalPlayerActor::LockInput( false );
		}
	}

	//switch( pPacket->nRetCode ) {
	//	case -1:
	//		GetInterface().MessageBox( 100024 );
	//		break;
	//}
}

void CDnVillageTask::OnRecvPartyMemberMove( SCPartyMemberMove *pPacket )
{
	CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyDataFromSessionID( pPacket->nSessionID );
	if( !pStruct ) return;

	pStruct->vPos = pPacket->Position;
}

void CDnVillageTask::OnRecvFarmInfo( SCFarmInfo * pPacket )
{
	CVillageClientSession::OnRecvFarmInfo( pPacket );

	CDnInterface::GetInstance().ShowFarmGateQuestionDialog( true, pPacket, this );	

	CDnLocalPlayerActor::StopAllPartyPlayer();

	CDnLocalPlayerActor::LockInput( true );
}

void CDnVillageTask::OnRecvFarmPlantedInfo( SCFarmPlantedInfo * pPacket )
{
	CVillageClientSession::OnRecvFarmPlantedInfo( pPacket );

	CDnInterface::GetInstance().UpdateFarmPlantedInfo( pPacket );
}

void CDnVillageTask::OnRecvOpenFarmList()
{
	CVillageClientSession::OnRecvOpenFarmList();

	GetInterface().OpenFarmChannel();
}

void CDnVillageTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	g_nPerfCounter = (int)LocalTime;
	CVillageClientSession::Process( LocalTime, fDelta );

	bool bProcess = true;
	if( !m_bEnterSuccess ) bProcess = false;
	if( m_bDestroyTask ) bProcess = false;
	if( !m_bEnterLocal ) bProcess = false;

	// Interface Process on Disconnect when bProcess returns false 
	// Handle it this way because it returns without looping, so the disconnect dialog would not show.
	if( bProcess == false ) {
		if( CDnBridgeTask::IsActive() && !CDnBridgeTask::GetInstance().IsInvalidDisconnect() ) return;
	}

	// Mouse Update
//	CDnMouseCursor::GetInstance().UpdateCursor();

	// Process Input Device
	GetInterface().LockMainMenu( false );
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	// Process Class
	// Camera Process
	CDnCamera::ProcessClass( LocalTime, fDelta );
	// Actor Process
	PROFILE_TIME_TEST( CDnActor::ProcessClass( LocalTime, fDelta ) );
	// Projectile Process
	CDnWeapon::ProcessClass( LocalTime, fDelta );
	// Etc Process
	CDnEtcObject::ProcessClass( LocalTime, fDelta );

	// Process Sound
	ProcessSound( LocalTime, fDelta );

	if( CDnCamera::GetActiveCamera() == m_hPlayerCamera ) {
		// Render Minimap
		if( CDnMinimap::IsActive() ) {
			CDnMinimap::GetInstance().Process( LocalTime, fDelta );
		}

	}
	// Process Interface
	GetInterface().Process( LocalTime, fDelta );
	GetInterface().PostProcess( LocalTime, fDelta );

	{
		CDnQuestTask *pTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask("QuestTask"));
		if (pTask && 
			pTask->IsRefreshNpcQuestState() &&
			CDnMinimap::IsActive()
			)
		{
			pTask->SetRefreshNpcQuestState(false);

			CDnMinimap::GetInstance().ReInitializeNPC(m_nVillageMapIndex);
		}
	}

	ProcessDebugMsg( LocalTime, fDelta );

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	CEtActionCoreMng::GetInstance().ProcessOptimize( fDelta );
#endif

	// Note : UI Process
	EtInterface::Process( fDelta );

	// Process World
	if( m_pWorld ) {
		PROFILE_TIME_TEST( m_pWorld->Process( LocalTime, fDelta ) );
	}

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	EternityEngine::RenderFrame( fDelta );
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );
	s_bCompleteFirstRender = true;
}

void CDnVillageTask::ProcessDebugMsg( LOCAL_TIME LocalTime, float fDelta )
{

#ifndef _FINAL_BUILD
	if( CGlobalValue::GetInstance().m_bShowFPS ) {
		char szBuf[256];
		float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
		float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

		sprintf_s( szBuf, "FPS : %.2f ( %d sec ) Res : %d x %d", m_pFrameSync->GetFps(), (int)m_pFrameSync->GetMSTime(),
			CEtDevice::GetInstance().Width(), CEtDevice::GetInstance().Height() );

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		float fDSP, fStream, fUpdate, fTotal;
		CEtSoundEngine::GetInstance().GetCPUUsage( fDSP, fStream, fUpdate, fTotal );
		sprintf_s( szBuf, "Sound Info : DSP : %.2f, Stream : %.2f, Update : %.2f, Total : %.2f", fDSP, fStream, fUpdate, fTotal );

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.03f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.03f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		sprintf_s( szBuf, "Actor : %d, Projectile : %d, DropItem : %d", CDnActor::s_pVecProcessList.size(), CDnProjectile::s_pVecProcessList.size(), CDnDropItem::s_pVecProcessList.size() );
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.06f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.06f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

#ifdef PRE_MOD_MEMORY_CHECK
		sprintf_s( szBuf, "Local Video Memory : %dMB / %dMB", GetEtDevice()->m_dwUsingLocalVideoMemory, GetEtDevice()->m_dwMaxLocalVideoMemory );
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.09f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.09f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
#endif

		PROCESS_MEMORY_COUNTERS_EX pmc;
		memset(&pmc, 0, sizeof(PROCESS_MEMORY_COUNTERS_EX));
		pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(PROCESS_MEMORY_COUNTERS_EX)))
		{
			char szTimeBuff[256] = {0,};
			struct tm *tm_ptr = NULL;
			time_t raw;

			time(&raw);
			tm_ptr = localtime(&raw);
			if (tm_ptr)
				strftime(szTimeBuff, 256, "(Time %H:%M:%S)", tm_ptr);

			sprintf_s(szBuf, "Current Memory Use : %dMB %s", pmc.WorkingSetSize / 1024 / 1024, szTimeBuff);
			EternityEngine::DrawText2D(EtVector2( 0.f + fWidthDot, 0.12f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0));
			EternityEngine::DrawText2D(EtVector2( 0.f, 0.12f ), szBuf, D3DCOLOR_ARGB(255,255,255,0));
			sprintf_s(szBuf, "Peak Memory Use : %dMB", pmc.PeakWorkingSetSize / 1024 / 1024);
			EternityEngine::DrawText2D(EtVector2( 0.f + fWidthDot, 0.14f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0));
			EternityEngine::DrawText2D(EtVector2( 0.f, 0.14f ), szBuf, D3DCOLOR_ARGB(255,255,255,0));
			sprintf_s(szBuf, "Private Usage Memory : %dMB", pmc.PrivateUsage / 1024 / 1024);
			EternityEngine::DrawText2D(EtVector2( 0.f + fWidthDot, 0.16f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0));
			EternityEngine::DrawText2D(EtVector2( 0.f, 0.16f ), szBuf, D3DCOLOR_ARGB(255,255,255,0));
		}

		if (m_fTimerSec >= 0.f)
		{
			m_fTimerSec -= fDelta;
		}
		else
		{
			InfoLog("[MEM] CURRENT:%dMB / PEAK:%dMB / PRIVATE:%dMB", pmc.WorkingSetSize / 1024 / 1024, pmc.PeakWorkingSetSize / 1024 / 1024, pmc.PrivateUsage / 1024 / 1024);
			m_fTimerSec = 60.f;
		}

		sprintf_s( szBuf, "UI Texture Usage : %dKB, Sound Usage : %dKB", CEtUIDialog::GetTextureMemorySize() / 1024, CEtSoundEngine::GetInstance().GetTotalSoundSize() / 1024 );
		EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.06f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.5f, 0.06f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		CGlobalValue::GetInstance().m_dwEndFPSTime = timeGetTime();
		DWORD dwTime = CGlobalValue::GetInstance().m_dwEndFPSTime - CGlobalValue::GetInstance().m_dwBeginFPSTime;
		float fAveFrame = ( CGlobalValue::GetInstance().m_dwFPSCount == 0 ) ? 0.f : CGlobalValue::GetInstance().m_fAverageFPS / (float)CGlobalValue::GetInstance().m_dwFPSCount;
		sprintf_s( szBuf, "REC Time : %dm %ds,   REC Frame : %.2f, ( %.2f ~ %.2f )", ( dwTime / 1000 ) / 60, ( dwTime / 1000 ) % 60, fAveFrame, CGlobalValue::GetInstance().m_fMinFPS, CGlobalValue::GetInstance().m_fMaxFPS );

		EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.0f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.5f, 0.0f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );

		CGlobalValue::GetInstance().ProcessFPSLog( m_pFrameSync );

		int nCount[8] = { 0, };
		const char *szType[] = { "RT_NONE", "RT_ANI", "RT_SKIN", "RT_MESH", "RT_SHADER", "RT_TEXTURE", "RT_PARTICLE", "RT_EFFECT" };
		for( int i=0; i<CEtResource::GetItemCount(); i++ ) {
			EtResourceHandle hResource = CEtResource::GetItem(i);
			if( hResource ) nCount[hResource->GetResourceType()]++;
		}
		for( int i=0; i<8; i++ ) {
			sprintf_s( szBuf, "%s : %d", szType[i], nCount[i] );
			EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.1f + fHeightDot + ( 0.03f * i ) ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
			EternityEngine::DrawText2D( EtVector2( 0.5f, 0.1f + ( 0.03f * i ) ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
		}

		const char *szSoundType[] = { "FMOD_SOUND_TYPE_UNKNOWN", "FMOD_SOUND_TYPE_AAC", "FMOD_SOUND_TYPE_AIFF", "FMOD_SOUND_TYPE_ASF", "FMOD_SOUND_TYPE_AT3", "FMOD_SOUND_TYPE_CDDA", "FMOD_SOUND_TYPE_DLS", "FMOD_SOUND_TYPE_FLAC", "FMOD_SOUND_TYPE_FSB", "FMOD_SOUND_TYPE_GCADPCM", "FMOD_SOUND_TYPE_IT", "FMOD_SOUND_TYPE_MIDI", "FMOD_SOUND_TYPE_MOD", "FMOD_SOUND_TYPE_MPEG", "FMOD_SOUND_TYPE_OGGVORBIS", "FMOD_SOUND_TYPE_PLAYLIST", "FMOD_SOUND_TYPE_RAW", "FMOD_SOUND_TYPE_S3M", "FMOD_SOUND_TYPE_SF2", "FMOD_SOUND_TYPE_USER", "FMOD_SOUND_TYPE_WAV", "FMOD_SOUND_TYPE_XM", "FMOD_SOUND_TYPE_XMA", "FMOD_SOUND_TYPE_VAG" };
		const char *szSoundFormat[] = { "FMOD_SOUND_FORMAT_NONE", "FMOD_SOUND_FORMAT_PCM8", "FMOD_SOUND_FORMAT_PCM16", "FMOD_SOUND_FORMAT_PCM24", "FMOD_SOUND_FORMAT_PCM32", "FMOD_SOUND_FORMAT_PCMFLOAT", "FMOD_SOUND_FORMAT_GCADPCM", "FMOD_SOUND_FORMAT_IMAADPCM", "FMOD_SOUND_FORMAT_VAG", "FMOD_SOUND_FORMAT_XMA", "FMOD_SOUND_FORMAT_MPEG", "FMOD_SOUND_FORMAT_CELT" };

		for( int i=0; i<FMOD_SOUND_TYPE_MAX; i++ ) {
			sprintf_s( szBuf, "%s : %d", szSoundType[i], CEtSoundEngine::GetInstance().GetSoundTypeCount((FMOD_SOUND_TYPE)i) );
			EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.18f + fHeightDot + ( 0.02f * i ) ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
			EternityEngine::DrawText2D( EtVector2( 0.f, 0.18f + ( 0.02f * i ) ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
		}
		for( int i=0; i<FMOD_SOUND_FORMAT_MAX; i++ ) {
			sprintf_s( szBuf, "%s : %d", szSoundFormat[i], CEtSoundEngine::GetInstance().GetSoundFormatCount((FMOD_SOUND_FORMAT)i) );
			EternityEngine::DrawText2D( EtVector2( 0.3f + fWidthDot, 0.18f + fHeightDot + ( 0.02f * i ) ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
			EternityEngine::DrawText2D( EtVector2( 0.3f, 0.18f + ( 0.02f * i ) ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
		}
	}

	if( CGlobalValue::GetInstance().m_bShowDPS )
	{
		char szBuf[256];
		float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
		float fHeightDot = 1.f / CEtDevice::GetInstance().Height();
		sprintf_s( szBuf, "DPS Sum = %d", CGlobalValue::GetInstance().m_nSumDPS );

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.15f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.15f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		CGlobalValue::GetInstance().m_fDPSTime += fDelta;
		sprintf_s( szBuf, "DPS Sum = %f", (float)(CGlobalValue::GetInstance().m_nSumDPS / CGlobalValue::GetInstance().m_fDPSTime) );
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.18f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.18f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	}

	if( CGlobalValue::GetInstance().m_bShowCurrentAction )
	{
		char szBuf[256];
		float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
		float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

		if(CDnActor::s_hLocalActor)
		{
			sprintf_s( szBuf, "CurrentAction = %s    Frame = %f", CDnActor::s_hLocalActor->GetCurrentAction(), CDnActor::s_hLocalActor->GetCurFrame() );
			EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.22f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
			EternityEngine::DrawText2D( EtVector2( 0.f, 0.22f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
		}
	}

	if( EtInterface::g_bEtUIRender )
	{
#ifdef PRE_FIX_SHOWVER
		std::string szVersion = CGlobalInfo::GetInstance().MakeVersionString();
#else
		std::string szVersion = CGlobalValue::GetInstance().MakeVersionString();
#endif
		EtVector2 vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 10.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 20.f ) );
		EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xff000000 );
		vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 9.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 21.f ) );
		EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xffffffff );
	}
#else
	if( CDnActor::s_hLocalActor && ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel() && CGlobalInfo::GetInstance().m_bShowVersion ) {
#ifdef PRE_FIX_SHOWVER
		std::string szVersion = CGlobalInfo::GetInstance().MakeVersionString();
#else
		std::string szVersion = CGlobalValue::GetInstance().MakeVersionString();
#endif
		EtVector2 vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 10.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 20.f ) );
		EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xff000000 );
		vPos = EtVector2( ( 1.f / CEtDevice::GetInstance().Width() ) * 9.f, ( 1.f / CEtDevice::GetInstance().Height() ) * ( CEtDevice::GetInstance().Height() - 21.f ) );
		EternityEngine::DrawText2D( vPos, szVersion.c_str(), 0xffffffff );
	}
#endif

}


void CDnVillageTask::ProcessSound( LOCAL_TIME LocalTime, float fDelta )
{
	if( CEtSoundEngine::IsActive() ) {
		CEtSoundEngine::GetInstance().Process( fDelta );
		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();

		MatrixEx Cross;
		if( hCamera ) {
			switch( hCamera->GetCameraType() ) {
				case CDnCamera::PlayerCamera:
					{
						DnActorHandle hAttachActor = ((CDnPlayerCamera*)hCamera.GetPointer())->GetAttachActor();
						if( hAttachActor ) {
							Cross = *hAttachActor->GetMatEx();
							MatrixEx MatExTemp;
							MatExTemp.m_vXAxis = *hAttachActor->GetMoveVectorX();
							MatExTemp.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
							MatExTemp.m_vZAxis = *hAttachActor->GetMoveVectorZ();

							MatExTemp.RotateYaw( -11.0003566f );
							Cross.m_vZAxis = MatExTemp.m_vZAxis;
						}
						else Cross = *hCamera->GetMatEx();
					}
					break;
				case CDnCamera::NpcTalkCamera:
					{
						Cross = *CDnActor::s_hLocalActor->GetMatEx();
						Cross.m_vZAxis = *CDnActor::s_hLocalActor->GetLookDir();
						EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis );
					}
					break;
				default:
					Cross = *hCamera->GetMatEx();
					break;
			}
		}
		else {
			if( CDnActor::s_hLocalActor ) Cross = *CDnActor::s_hLocalActor->GetMatEx();
		}

		CEtSoundEngine::GetInstance().SetListener( Cross.m_vPosition, Cross.m_vZAxis, Cross.m_vYAxis );
	}
}

bool CDnVillageTask::RenderScreen( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bEnterLocal ) return false;
	return true;
}

void CDnVillageTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) {
		case GATE_MAP_INDEX_SELECT_DIALOG:
			{
				if( nCommand == EVENT_BUTTON_CLICKED )
				{
					if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 )
					{
						BYTE cSelectIndex = GetInterface().GetWorldZoneSelectIndex();

						// There is no fade here and it cannot be checked via channel move, so handle it directly.
						GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
						if (CDnPartyTask::IsActive())
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
							CDnPartyTask::GetInstance().ClearInviteInfoList(true);
#else
							CDnPartyTask::GetInstance().ClearInviteInfoList();
#endif
#ifdef PRE_PARTY_DB
						SendStartStage( Dungeon::Difficulty::Easy, -1, false, 0, cSelectIndex );
#else
						SendStartStage(0, -1, false, 0, cSelectIndex);
#endif
					}
					else if( strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0 )
					{
						SendCancelStage();
						m_bRequestCancelStageSelf = true;
					}
					else if ( strcmp( pControl->GetControlName(), "ID_BT_PARTY" ) == 0)
					{
						SendCancelStage();
						GetPartyTask().SetWaitingPartyFindDlgOpen(true);
						m_bRequestCancelStageSelf = true;
					}
				}
			}
			break;

		case GATEQUESTION_DIALOG:
			if( nCommand == EVENT_BUTTON_CLICKED ) {
				if( strcmp( pControl->GetControlName(), "ID_START" ) == 0 ) {

					// There is no fade here and it cannot be checked via channel move, so handle it directly.
					GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
					if (CDnPartyTask::IsActive())
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
						CDnPartyTask::GetInstance().ClearInviteInfoList(true);
#else
						CDnPartyTask::GetInstance().ClearInviteInfoList();
#endif
#ifdef PRE_PARTY_DB
					SendStartStage(Dungeon::Difficulty::Easy);
#else
					SendStartStage(0);
#endif
				}
				else if( strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0)
				{
					SendCancelStage();
					m_bRequestCancelStageSelf = true;
				}
				else if ( strcmp( pControl->GetControlName(), "ID_PARTY" ) == 0)
				{
					SendCancelStage();
					GetPartyTask().SetWaitingPartyFindDlgOpen(true);
					m_bRequestCancelStageSelf = true;
				}
			}
			break;
		case NPC_DIALOG:
			{
				if ( nCommand == EVENT_TEXTBOX_SELECTION )
				{
					std::wstring szLinkIndex;
					std::wstring szLinkTarget;
					bool bResult = GetInterface().GetNpcDialog()->GetAnswerIndex(szLinkIndex, szLinkTarget);
					
					//
					if ( bResult  == false || szLinkIndex.empty() || szLinkTarget.empty() )
					{
						GetInterface().CloseNpcDialog();
						//CDnLocalPlayerActor::SetTakeNpcUID(0xffffffff);
						//CDnLocalPlayerActor::LockInput(false);
						return;
					}
					CDnCommonTask* pCommonTask = dynamic_cast<CDnCommonTask*>( CTaskManager::GetInstance().GetTask( "CommonTask" ) );
					if(pCommonTask)
					{
						if( !pCommonTask->IsRequestNpcTalk() )
						{
							UINT nNpcUID = CDnLocalPlayerActor::GetTakeNpcUID();
							SendNpcTalk(nNpcUID, szLinkIndex, szLinkTarget);
							pCommonTask->SetNpcTalkRequestWait();
						}
					}
				}
			}
			break;
		case MESSAGEBOX_10:
			{
				if( nCommand == EVENT_BUTTON_CLICKED ) 
				{
					if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
					{
						if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
						CTaskManager::GetInstance().RemoveAllTask( false );
					}
				}
			}
			break;
		case MESSAGEBOX_17:
			{
				if( nCommand == EVENT_BUTTON_CLICKED )
				{
					RequestPartyListInfo( m_nPartyListCurrentPage );
				}
			}
			break;
		case FADE_DIALOG:
			if( nCommand == EVENT_FADE_COMPLETE )
			{
				//BeginGameTask( m_nStageRandomSeed );
				GetInterface().ShowMapCaption( GetVillageMapIndex() );
			}
			break;

		case PARTY_JOIN_FAIL_DIALOG:
			{
				if( nCommand == EVENT_BUTTON_CLICKED )
				{
					if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
						GetInterface().OnPartyJoinFailed();
				}
			}
			break;
	}
}

void CDnVillageTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_KEY_DOWN ) {
		if( CDnActor::s_hLocalActor ) {
#ifdef _FINAL_BUILD
			if( ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel() )
#endif
			{
				if( IsPushKey( DIK_LCONTROL ) && IsPushKey( DIK_TAB ) ) {
					if( CDnCamera::GetActiveCamera() == m_hDebugCamera )
						CDnCamera::SetActiveCamera( m_hPlayerCamera );
					else {
						*m_hDebugCamera->GetCameraInfo() = *m_hPlayerCamera->GetCameraInfo();
						*m_hDebugCamera->GetMatEx() =  *m_hPlayerCamera->GetMatEx();
						m_hDebugCamera->GetMatEx()->m_vPosition.y += 50.0f;
						CDnCamera::SetActiveCamera( m_hDebugCamera );
					}
					ReleasePushKey( DIK_LCONTROL );
					ReleasePushKey( DIK_TAB );
				}
			}
		}
	}
}

//blondy
void CDnVillageTask::EnterPVPLobby()
{
	SendMovePvPVillageToPvPLobby();
}
//blondy end
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
void CDnVillageTask::InitializePreloadPlayer()
{
	if( m_bPreloadComplete ) return;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );
	for( int i=0; i<10; i++ ) {
		if( m_hPreloadPlayer[i] ) continue;
		CDnPlayerActor *pPlayer;
		m_hPreloadPlayer[i] = CreateActor( i + 1, false, false, false );
		if( !m_hPreloadPlayer[i] ) continue;
		pPlayer = (CDnPlayerActor*)m_hPreloadPlayer[i].GetPointer();

		pPlayer->RecomposeAction();
	}
	m_bPreloadComplete = true;
}

void CDnVillageTask::ReleasePreloadPlayer()
{
	if( !m_bPreloadComplete ) return;
	for( int i=0; i<10; i++ ) {
		SAFE_RELEASE_SPTR( m_hPreloadPlayer[i] );
	}
	m_bPreloadComplete = false;
}
#endif