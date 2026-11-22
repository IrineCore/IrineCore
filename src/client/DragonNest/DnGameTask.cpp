#include "StdAfx.h"
#include "DnGameTask.h"
#include "DnDebugConsole.h"
#include "DnWorld.h"
#include "EtMatrixEx.h"
#include "DnFreeCamera.h"
#include "DnPlayerCamera.h"
#include "DnActor.h"
#include "FrameSync.h"
#include "DnLocalPlayerActor.h"
#include "DnMonsterActor.h"
#include "DnActorClassDefine.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "EtWorldSector.h"
#include "PerfCheck.h"
#include "EtSoundEngine.h"
#include "DnProjectile.h"
#include "DnInterface.h"
#include "DnMinimap.h"
#include "InputWrapper.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnWorldSector.h"
#include "DnDropItem.h"
#include "ClientSessionManager.h"
#include "VillageSendPacket.h"
#include "DnMainFrame.h"
#include "DnVillageTask.h"
#include "DnPartyTask.h"
#include "DnCharStatusDlg.h"
#include "PartySendPacket.h"
#include "ItemSendPacket.h"
#include "DnItemTask.h"
#include "NetSocketRUDP.h"
#include "DnDataManager.h"
#include "DnNpcActor.h"
#include "DnInterfaceDlgID.h"
#include "DnGateQuestionDlg.h"
#include "DnWorldActProp.h"
#include "DNPacket.h"
#include "DnBridgeTask.h"
#include "GameSendPacket.h"
#include "DnSkillTask.h"
#include "MAAiBase.h"
#include "DnWorldTrapProp.h"
#include "DnWorldBrokenProp.h"
#include "DnDungeonEnterDlg.h"
#include "DnDungeonClearDlg.h"
#include "DnStateBlow.h"
#include "DnGaugeDlg.h"
#include "DnCommonTask.h"
#include <mmsystem.h>
#include "DnStageClearReportDlg.h"
#include "DnStageClearDlg.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnInCodeResource.h"
#include "DnPartsHair.h"
#include "DnMainMenuDlg.h"
#include "DnInterfaceCamera.h"
#include "DnServiceSetup.h"
#include "DnGameTipTask.h"
#include "DnDungeonClearImp.h"
#include "DnUIString.h"
#include "DnQuestTask.h"
#include "DNGestureTask.h"
#include "FriendSendPacket.h"
#include "DnIsolate.h"
#include "DnNestInfoTask.h"
#include "EtActionCoreMng.h"
#include "DnLifeSkillPlantTask.h"
#include "DnChatRoomTask.h"
#include "DnPartsVehicleEffect.h"
#include "BugReporter.h"
#include "DnVehicleTask.h"
#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnLifeSkillFishingTask.h"
#include "DnLifeSkillCookingTask.h"
#endif // PRE_ADD_SECONDARY_SKILL
#include "DnPetTask.h"
#include "DnHideMonsterActor.h"
#ifdef PRE_ADD_BESTFRIEND
#include "DnAppellationTask.h"
#endif
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
#include "DnMessageBox.h"
#endif
#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
#include "GameOption.h"
#endif 
#if defined(PRE_ADD_TALISMAN_SYSTEM)
#include "DnTalisman.h"
#endif

#ifdef PRE_ADD_MONSTER_CHEAT
#include "DnAggroBlow.h"
#endif

#ifdef _FINAL_BUILD
#else
#include "psapi.h"
#endif

bool g_bNaviTest = false;
extern EtVector3 g_vStart;
extern EtVector3 g_vEnd;

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

bool CDnGameTask::s_bCompleteFirstRender = true;
CDnGameTask::CDnGameTask()
: CInputReceiver( true )
{
	m_GameTaskType = GameTaskType::Normal;
	m_bIsBreakIntoTask	= false;
	m_pWorld = NULL;
	m_nStageConstructionLevel = -1;
	m_nStartPositionIndex = 1;
	m_nExtendDropRate = 0;

	m_pMinimap = NULL;
	m_nMapTableID = 0;
#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	m_nRootMapTableID = 0;
#endif

	m_bFinalizeStage = true;
	m_bInitComplete = false;
	m_nAggroValue = 0;
	m_WarTimeType = WarTimeEnum::None;

	m_BridgeState = eBridgeState::BridgeUnknown;

	m_nDirectConnectMapIndex = -1;
	m_nDirectConnectGateIndex = -1;

	m_bIsCombat = false;
	m_fCombatDelta = 0.f;


	m_bNeedSendReqSyncWait = false;	
	m_bSyncStartToggleWeapon = false;
	m_pStageClearImp = NULL;
	m_bLookStageEntrance = false;

	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );

	m_hDummyCamera = (new CDnInterfaceCamera)->GetMySmartPtr();
	CDnCamera::SetActiveCamera( m_hDummyCamera );

	m_bEnableDungeonClearLeaderWarp = false;
	m_bShowDungeonOpenNotice = false;

	m_bIsPVPShowHelmetMode = false;
	m_nMaxCharCount = 0;
	s_bCompleteFirstRender = false;

	m_bIsCreateLocalActor = false;
	m_pCDCameraEffect = NULL;

#ifdef PRE_ADD_BESTFRIEND
	memset(&m_TempBFpacket,0,sizeof(SCPartyBestFriend));
#endif

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	memset(&m_sStageDamageLimitStruct,0,sizeof(StageDamageLimitStruct));
#endif

#if defined( PRE_ADD_CP_RANK )
	m_bDungeonClearBestInfo = false;
#endif	// #if defined( PRE_ADD_CP_RANK )
	m_bTutorial = false;

#ifdef _FINAL_BUILD
#else
	m_fTimerSec = 60.f;
#endif

#ifdef PRE_ADD_CRAZYDUC_UI
	m_bShowModDlg = false;
	m_sTimeAttackStruct.nOrigin = m_sTimeAttackStruct.nRemain = 0;
#endif
}

CDnGameTask::~CDnGameTask()
{
	if( m_pCDCameraEffect )
	{
		m_pCDCameraEffect->Clear();
		SAFE_DELETE( m_pCDCameraEffect );		
	}

	CClientSessionManager::GetInstance().FinalizeUdpSession();
	FinalizeStage();

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	CDnMainFrame::GetInstance().FlushPreloadPlayer();
#endif
	s_bCompleteFirstRender = true;
}

bool CDnGameTask::Initialize()
{
	CDnMouseCursor::GetInstance().ShowCursor( false, true );
	CDnPlayerActor::EnableAnotherPlayers( true );

	SendReady2Receive( CDnBridgeTask::GetInstance().GetSessionID() );
	GetInterface().CloseSlideCaptionMessage();
	GetGameTipTask().ResetGameTip();
	GetInterface().ResetCp();

	m_bEnableDungeonClearLeaderWarp = false;
	m_bTutorial = false;

	return true;
}

void CDnGameTask::OnDisconnectUdp( bool bValidDisconnect )
{
	CGameClientSession::OnDisconnectUdp( bValidDisconnect );
	if( !bValidDisconnect )
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, MESSAGEBOX_10, this );
}

void CDnGameTask::OnDisconnectTcp( bool bValidDisconnect )
{
	CGameClientSession::OnDisconnectTcp( bValidDisconnect );
	if( !bValidDisconnect ) {
		WriteLog( 0, ", Error, Disconnect Game Server\n" );
#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
		g_bInvalidDisconnect = true;
#endif
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, MESSAGEBOX_10, this );

#ifdef PRE_ADD_CL_DISCONNECT_LOG
		if( CDnActor::s_hLocalActor )
		{
			gs_BugReporter.AddLogW( L"CDnGameTask::OnDisconnectTcp() - UserName[%s]", CDnActor::s_hLocalActor->GetName() );
		}
		else
		{
			gs_BugReporter.AddLogW( L"CDnGameTask::OnDisconnectTcp() While Loading ");
		}
		
		gs_BugReporter.ForceReporting();
#endif // PRE_ADD_CL_DICONNECT_LOG

	}
}

void CDnGameTask::SetSyncComplete( bool bFlag )
{
	DebugLog("CDnGameTask::SetSyncComplete :: start");
	CGameClientSession::SetSyncComplete( bFlag );
	if( bFlag == true ) {
		ResetTimer();
		if( m_hLocalPlayerActor ) {
			m_hLocalPlayerActor->ResetActor();
		}
		CInputDevice::GetInstance().ResetAllInput();		
		CDnLocalPlayerActor::LockInput( false );
		CDnStateBlow::ResumeProcess();

		if( m_bIsBreakIntoTask ) {
			for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ ) {
				CDnActor *pActor = CDnActor::s_pVecProcessList[i];
				bool bCheckDestroy = false;

				// note by kalliste : There are cases where an npc prop is hidden depending on quest progress, etc. 
				// Since hidden NPCs are checked and destroyed on drop-in, if an NPC is needed later for quest progression etc.,
				// Issue where messages cannot be received. Since there are not many hidden NPC props placed in one map 
				// Therefore memory allocation issues are expected to be minimal, and issues with prop names being displayed also
				// Verified no issues, so disabled the code for now.
#ifdef PRE_FIX_NPCHIDE_ON_BREAKINTO
#else
				if( pActor->IsNpcActor() ) {
					bCheckDestroy = true;
				}
#endif

				if (pActor->IsMonsterActor() || pActor->IsPartsMonsterActor())
				{
					bCheckDestroy = true;
					if( pActor->GetActorType() == CDnActorState::SimpleRush &&
						dynamic_cast<CDnHideMonsterActor*>(pActor) != NULL)
						bCheckDestroy = false;
				}

				if (bCheckDestroy)
				{
					if( !pActor->IsShow() ) {
						pActor->SetDestroy();
					}
				}
			}
		}
	}
	g_nPerfCounter = 0;

	bool bShowMapCaption = true;
	switch( GetGameTaskType() ) {
		case GameTaskType::PvP: bShowMapCaption = false; break;
	}

	if( bShowMapCaption )
		GetInterface().ShowMapCaption( m_nMapTableID );

	CDnWorld::GetInstance().EnableTriggerEventCallback( true );

	if( CDnActor::s_hLocalActor && m_bSyncStartToggleWeapon ) {
		if( ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsCanBattleMode() ) {
			if( GetGameTaskType() != GameTaskType::Farm )
				((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->CmdToggleBattle( true );
		}
	}

	DebugLog("CDnGameTask::SetSyncComplete ::end");
}

bool CDnGameTask::CheckSyncProcess()
{
	if( CDnPartyTask::IsActive() && !CDnPartyTask::GetInstance().IsSyncComplete() ) {
		if( m_pWorld ) CDnWorld::GetInstance().EnableTriggerEventCallback( false );

		for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;
			if( !pStruct->hActor ) continue;
			if( pStruct->hActor->GetHP() > 0 && pStruct->hActor->GetState() != CDnActor::Stay ) 
			{

				pStruct->hActor->SetActionQueue( "Stand" );
				OutputDebug( "You shouldn't do this~\n" );
			}
		}

		if( !CDnLocalPlayerActor::IsLockInput() )
			CDnLocalPlayerActor::LockInput( true );
		return true;
	}
	return false;
}

void CDnGameTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	UpdateHang();
	CGameClientSession::Process( LocalTime, fDelta );

	if( m_bDestroyTask ) return;

	bool bProcess = true;
	if( RenderScreen( LocalTime, fDelta ) ) {
		Sleep(1);
		bProcess = false;
	}
	if( bProcess == false ) {
		if( CDnBridgeTask::IsActive() && !CDnBridgeTask::GetInstance().IsInvalidDisconnect() ) return;
	}

	g_nPerfCounter = (int)LocalTime;

	bool bSyncWait = CheckSyncProcess();

	// Mouse Update
//	if( !bSyncWait ) CDnMouseCursor::GetInstance().UpdateCursor();

	// Process Input Device
	GetInterface().LockMainMenu( bSyncWait );
//	if( !bSyncWait || GetGameTaskType() == GameTaskType::PvP ) 
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	// Process Class
	// Camera Process
	CDnCamera::ProcessClass( LocalTime, fDelta );
	// Actor Process
	CDnActor::ProcessClass( LocalTime, fDelta );
	// Weapon(Projectile) Process
	CDnWeapon::ProcessClass( LocalTime, fDelta );
	// Drop Item Process
	CDnDropItem::ProcessClass( LocalTime, fDelta );
	// Etc Object Process
	CDnEtcObject::ProcessClass( LocalTime, fDelta );

	CDnActor::ProcessFlushPacketQueue();

	// Process Sound
	ProcessSound( LocalTime, fDelta );


	// Render Minimap
	if( CDnMinimap::IsActive() ) {
		CDnMinimap::GetInstance().Process( LocalTime, fDelta );
	}

	// Process Interface
	GetInterface().Process( LocalTime, fDelta );
	GetInterface().PostProcess( LocalTime, fDelta );

	ProcessCombat( LocalTime, fDelta );

	{
		CDnQuestTask *pTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask("QuestTask"));
		if (pTask && 
			pTask->IsRefreshNpcQuestState() &&
			CDnMinimap::IsActive()
			)
		{
			pTask->SetRefreshNpcQuestState(false);

			CDnMinimap::GetInstance().ReInitializeNPC(m_nMapTableID);
		}
	}

	ProcessDebugMsg(LocalTime,fDelta);

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	CEtActionCoreMng::GetInstance().ProcessOptimize( fDelta );
#endif

	// Note : UI Process
	EtInterface::Process( fDelta );

	// Process World
	if( m_pWorld ) {
		m_pWorld->Process( LocalTime, fDelta );
	}

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	//DebugLog("EternityEngine::RenderFrame start");
	EternityEngine::RenderFrame( fDelta );
	//DebugLog("EternityEngine::RenderFrame end :delta = %f",fDelta);
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );
	s_bCompleteFirstRender = true;

	if( m_bNeedSendReqSyncWait ) //To send sync after lightmap build finishes (only needs one frame to run) 
	{
		m_bNeedSendReqSyncWait = false;
		if( CDnPartyTask::IsActive() )
			CDnPartyTask::GetInstance().RequestSyncWaitMsg();
	}

#ifndef _FINAL_BUILD // Fix map transfer to also work in single-player mode.
	if( CGlobalValue::GetInstance().m_bSingleTest ) {
		CEtWorldGrid *pGrid = CDnWorld::GetInstance().GetGrid();

		if( !pGrid ) return;
		if( CDnActor::s_hLocalActor->IsDie() || CDnActor::s_hLocalActor->IsHit() ) return;

		DWORD dwCount = pGrid->GetActiveSectorCount();
		CEtWorldEventControl *pControl = NULL;
		CEtWorldSector *pSector = NULL;		
		EtVector2 vStartPos( 0.f, 0.f );

		for( DWORD i=0; i<dwCount; i++ ) {
			pSector = pGrid->GetActiveSector(i);
			pControl = pSector->GetControlFromUniqueID( ETE_EventArea );
			if( !pControl ) continue;

			pControl->Process( *CDnActor::s_hLocalActor->GetPosition() );

			for( DWORD j=0; j<pControl->GetCheckAreaCount(); j++ ) {
				CEtWorldEventArea *pArea = pControl->GetCheckArea(j);
				if( strstr( pArea->GetName(), "Gate" ) ) {
					sscanf_s( pArea->GetName(), "Gate %d\n", &m_nGateIndex, sizeof(int) );
					if( m_nGateIndex < 1 ) continue;
					InitializeStage( m_nMapTableID, m_nGateIndex, CGlobalValue::GetInstance().m_nMapLevel, m_nStageRandomSeed, true, false );
					CDnPartyTask *pPartyTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
					pPartyTask->GetPartyData(0)->bSync = true;
					pPartyTask->SetSyncComplete( true );
					return;
				}
			}
		}
	}
#endif //_FINAL_BUILD
}

void CDnGameTask::ShowFPS_Information(const float& fDelta)
{
	char szBuf[256] = {0,};
	float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
	float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

	sprintf_s( szBuf, "FPS : %.2f ( %d sec ) Res : %d x %d", m_pFrameSync->GetFps(), (int)m_pFrameSync->GetMSTime(),
		CEtDevice::GetInstance().Width(), CEtDevice::GetInstance().Height());

	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	float fDSP, fStream, fUpdate, fTotal;
	CEtSoundEngine::GetInstance().GetCPUUsage( fDSP, fStream, fUpdate, fTotal );
	int nPlayChannelCount = CEtSoundEngine::GetInstance().GetUsingChannelCount();
	sprintf_s( szBuf, "Sound Info : DSP : %.2f, Stream : %.2f, Update : %.2f, Total : %.2f, Count : %d", fDSP, fStream, fUpdate, fTotal, nPlayChannelCount );

	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.03f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.03f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	sprintf_s( szBuf, "Actor : %d, Projectile : %d, DropItem : %d, CEtResource : %d", CDnActor::s_pVecProcessList.size(), CDnProjectile::s_pVecProcessList.size(), CDnDropItem::s_pVecProcessList.size(), CEtResource::GetItemCount() );
	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.06f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.06f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

#ifdef PRE_MOD_MEMORY_CHECK
	sprintf_s( szBuf, "Local Video Memory : %dMB / %dMB", GetEtDevice()->m_dwUsingLocalVideoMemory, GetEtDevice()->m_dwMaxLocalVideoMemory );
	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.09f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.09f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
#endif

#ifdef _FINAL_BUILD
#else
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
			strftime(szTimeBuff, 256, "(Time : %H:%M:%S)", tm_ptr);

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
#endif

	sprintf_s( szBuf, "UI Texture Usage : %dKB, Sound Usage : %dKB", CEtUIDialog::GetTextureMemorySize() / 1024, CEtSoundEngine::GetInstance().GetTotalSoundSize() / 1024 );
	EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.06f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.5f, 0.06f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	CGlobalValue::GetInstance().m_dwEndFPSTime = timeGetTime();
	DWORD dwTime = CGlobalValue::GetInstance().m_dwEndFPSTime - CGlobalValue::GetInstance().m_dwBeginFPSTime;
	float fAveFrame = ( CGlobalValue::GetInstance().m_dwFPSCount == 0 ) ? 0.f : CGlobalValue::GetInstance().m_fAverageFPS / (float)CGlobalValue::GetInstance().m_dwFPSCount;
	sprintf_s( szBuf, "REC Time : %dm %ds,   REC Frame : %.2f, ( %.2f ~ %.2f )", ( dwTime / 1000 ) / 60, ( dwTime / 1000 ) % 60, fAveFrame, CGlobalValue::GetInstance().m_fMinFPS, CGlobalValue::GetInstance().m_fMaxFPS );

	EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.0f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.5f, 0.0f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );

	if( CDnActor::s_hLocalActor ) {
		sprintf_s( szBuf, "Actor fps : %.2f / RenderBase fps : %.2f / Default fps : %.2f", static_cast<float>(CDnActor::s_hLocalActor->CDnActionBase::GetFPS()), static_cast<float>(CDnActor::s_hLocalActor->MAActorRenderBase::GetFPS()), static_cast<float>(s_fDefaultFps) );
		EternityEngine::DrawText2D( EtVector2( 0.5f, 0.12f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );
	}

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
}

void CDnGameTask::ShowDPS_Information( const float fDelta )
{
	char szBuf[256] = {0,};
	float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
	float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

	sprintf_s( szBuf, "DPS Sum = %d", CGlobalValue::GetInstance().m_nSumDPS );
	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.15f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.15f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	CGlobalValue::GetInstance().m_fDPSTime += fDelta;
	sprintf_s( szBuf, "DPS = %f", (float)(CGlobalValue::GetInstance().m_nSumDPS / CGlobalValue::GetInstance().m_fDPSTime) );
	EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.18f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.f, 0.18f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	int nProjectileCount = 0;
	for(DWORD n=0; n<CDnWeapon::s_pVecProcessList.size(); n++)
	{
		if( CDnWeapon::s_pVecProcessList[n]->GetWeaponType() == CDnWeapon::WeaponTypeEnum::Projectile )
		{
			CDnProjectile *pProjectile = dynamic_cast<CDnProjectile*>(CDnWeapon::s_pVecProcessList[n]);

			if(pProjectile && pProjectile->GetLinkActor() == CDnActor::s_hLocalActor )
				nProjectileCount++;
		}
	}

	if(nProjectileCount == 0)
	{
		if( CDnActor::s_hLocalActor && CGlobalValue::GetInstance().m_bShowDPS == true && CGlobalValue::GetInstance().m_bShowSkillDPS == true )
		{

			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

			if(!pPlayer->IsProcessSkill())
			{
				float fDPS = 0.f;
				if( CGlobalValue::GetInstance().m_fDPSTime > 0.f )
					fDPS = (float)( (float)CGlobalValue::GetInstance().m_nSumDPS / CGlobalValue::GetInstance().m_fDPSTime );				

				wstring wstr;
				wstr = FormatW( L" Total Damage %d \n DPS %f \n Elapsed Time(sec) %f" , CGlobalValue::GetInstance().m_nSumDPS ,  fDPS , CGlobalValue::GetInstance().m_fDPSTime );
				CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption2, wstr.c_str(), textcolor::RED );
				CGlobalValue::GetInstance().m_nSumDPS = 0;
				CGlobalValue::GetInstance().m_fDPSTime = 0.f;
				CGlobalValue::GetInstance().m_bShowDPS = false;
			}
		}
	}
}

void CDnGameTask::ShowCurrentAction_Information()
{
	char szBuf[256] = {0,};
	float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
	float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

	if(CDnActor::s_hLocalActor)
	{
		sprintf_s( szBuf, "CurrentAction = %s     Frame = %f", CDnActor::s_hLocalActor->GetCurrentAction(), CDnActor::s_hLocalActor->GetCurFrame() );
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.22f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.22f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
	}
}

void CDnGameTask::ShowMonsterAction_Information()
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID(CGlobalValue::GetInstance().m_dwSetMonsterID);
	if(hActor)
	{
#ifdef PRE_ADD_MONSTER_CHEAT
		char szBuf[512] = {0,};
#else
		char szBuf[256] = {0,};
#endif 
		float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
		float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

		std::string szCustomEnum;
		switch(hActor->GetCustomState())
		{
		case CDnActorState::ActorCustomStateEnum::Custom_None:
			{
				szCustomEnum += "Custom_None";
			}
			break;

		case CDnActorState::ActorCustomStateEnum::Custom_Ground:
			{
				szCustomEnum += "Custom_Ground";
			}
			break;

		case CDnActorState::ActorCustomStateEnum::Custom_Fly:
			{
				szCustomEnum += "Custom_Fly";
			}
			break;

		case CDnActorState::ActorCustomStateEnum::Custom_UnderGround:
			{
				szCustomEnum += "Custom_UnderGround";
			}
			break;

		case CDnActorState::ActorCustomStateEnum::Custom_LifeSkill:
			{
				szCustomEnum += "Custom_LifeSkill";
			}
			break;

		case CDnActorState::ActorCustomStateEnum::Custom_Fly2:
			{
				szCustomEnum += "Custom_Fly2";
			}
			break;

		default:
			{
				szCustomEnum += "None";
			}
			break;
		}

#ifdef PRE_ADD_MONSTER_CHEAT
		CDnMonsterState* pMonsterState = NULL; CDnMonsterActor* pMonster = NULL;
		if( hActor->IsMonsterActor() )
		{
			pMonster = dynamic_cast<CDnMonsterActor*>(hActor.GetPointer());
			pMonsterState = dynamic_cast<CDnMonsterState*>(pMonster);
		}
		sprintf_s( szBuf, _countof(szBuf), 
			"Monster UniqueID: %d / Action: %s / Custom_State: %s / Monster_ID: %u / Actor_ID: %u", 
			CGlobalValue::GetInstance().m_dwSetMonsterID, 
			hActor->GetCurrentAction(), 
			szCustomEnum.c_str(), 
			pMonster != NULL ? pMonster->GetMonsterClassID() : 0,
			pMonster != NULL ? pMonster->GetActorTableID() : 0 );
		
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.26f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.26f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		sprintf_s( szBuf, _countof(szBuf), "MAX_HP: %u / CUR_HP: %u / PATK_Min: %u / PATK_Max: %u / MATK_Min: %u / MATK_Max: %u", 
			(UINT)hActor->GetMaxHP(), 
			(UINT)hActor->GetHitParam()->nCurrentHP, 
			hActor->GetAttackPMin(), 
			hActor->GetAttackPMax(), 
			hActor->GetAttackMMin(), 
			hActor->GetAttackMMax() );
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.29f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.29f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		sprintf_s( szBuf, _countof(szBuf), "DeadDurability: %u / Max SuperArmor: %u / Current SuperArmmor: %u / ItemDropGroupID: %u", 
			pMonsterState != NULL ? pMonsterState->GetDeadDurability() : NULL, 
			hActor->GetMaxSuperArmor(),
			hActor->GetCurrentSuperArmor(), 
			pMonsterState != NULL ? pMonsterState->GetItemDropGroupID() : NULL );

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.32f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.32f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
		
		char szTemp[256]= { 0, };
		sprintf_s( szBuf, _countof(szBuf), "SkillTableID: %u / AIFileName : %s", 
			pMonster != NULL ? pMonster->GetSkillTableID() : 0,  
			pMonster != NULL ? pMonster->GetAIFildName().c_str() : "");

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.35f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.35f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		CDnStateBlow::BLOW_HANDLE_LIST lBlowHandle;
		CDnStateBlow::BLOW_HANDLE_LIST_ITER iter;
		if( pMonster != NULL )
		{ 
			lBlowHandle = pMonster->GetStateBlow()->GetStateBlowList();
			iter = lBlowHandle.begin();
		}
		std::string szBindString;
		szBindString.reserve( lBlowHandle.size() * 4 );
		for( iter; iter != lBlowHandle.end(); ++iter )
		{	
			sprintf_s( szBuf, _countof(szBuf), "/%d/", (*iter)->GetBlowIndex() ); 
			szBindString.append(szBuf);
		}

		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.38f + fHeightDot ), szBindString.c_str(), D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.38f ), szBindString.c_str(), D3DCOLOR_ARGB(255,255,255,0) );

		MAAiReceiver *pAiReceiver = dynamic_cast<MAAiReceiver *>(pMonster);
		if( pMonster && pAiReceiver && pAiReceiver->GetAggroTarget() )
		{
			//WideCharToMultiByte( CP_ACP, 0,  pAiReceiver->GetAggroTarget()->GetName() , -1, szTemp, 256, NULL, NULL );
			static DWORD dwCurTick = 0; static DWORD dwPreTick = 0;		
			// Print once every 1 second
			dwCurTick = GetTickCount();
			if( dwCurTick - dwPreTick >= 1000 )
			{
				GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"", pAiReceiver->GetAggroTarget()->GetName() );
				dwPreTick = dwCurTick;
			}
		}
#else
		sprintf_s( szBuf, 256, "Monster UniqueID: %d / Action: %s / Custom_State: %s", CGlobalValue::GetInstance().m_dwSetMonsterID , hActor->GetCurrentAction(), szCustomEnum.c_str() );
#endif 

#ifdef PRE_ADD_MONSTER_CHEAT
#else
		EternityEngine::DrawText2D( EtVector2( 0.f + fWidthDot, 0.26f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.f, 0.26f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
#endif 
		// Death handling. Reset.
		if( strcmp( hActor->GetCurrentAction() , "Die" ) == 0 )
		{
			CGlobalValue::GetInstance().m_bShowMonsterAction = false;
			CGlobalValue::GetInstance().m_bSetMonster		 = false;
		}
	}
}

void CDnGameTask::ShowCP_Information()
{
#if defined( PRE_ADD_CP_RENEWAL )
	static const char * szCPName[MACP::CPTypeEnum::CPType_Max] =
	{
		"MaxComboCount",
		"KillBossCount",
		"KillMonsterScore",
		"CriticalHitScore",
		"StunHitScore",
		"SuperAmmorBreakScore",
		"BrokenShieldScore",
		"GenocideScore",
		"AirComboScore",
		"PartyComboScore",
		"RebirthPlayerScore",
		"UseSkillScore",
		"ComboScore",
		"AttackedCount",
		"AttackedHitScore",
		"AttackedCriticalHitScore",
		"AttackedStunHitScore",
		"DieCount",
		"DeadScore",
		"PropBreakScore",
		"GetItemScore",
		"AssistMonsterScore"
	};

	if( !CDnActor::s_hLocalActor )
		return;
	CDnLocalPlayerActor *pPlayer = (CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer());
	if( NULL == pPlayer )
		return;

	char szBuf[256] = {0,};
	float fWidthDot = 1.f / CEtDevice::GetInstance().Width();
	float fHeightDot = 1.f / CEtDevice::GetInstance().Height();

	const MACP::SMACP sData = pPlayer->GetCP_Data();
	const int * pIndexCPData = pPlayer->GetCP_IndexData();

	sprintf_s( szBuf, "Table CP Score" );
	EternityEngine::DrawText2D( EtVector2( 0.35f + fWidthDot, 0.03f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.35f, 0.03f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	sprintf_s( szBuf, "My CP Score" );
	EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.03f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.5f, 0.03f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	sprintf_s( szBuf, "My Total CP Score : %d", sData.nAccumulationCP );
	EternityEngine::DrawText2D( EtVector2( 0.65f + fWidthDot, 0.03f + fHeightDot ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
	EternityEngine::DrawText2D( EtVector2( 0.65f, 0.06f ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

	for( int itr = 0; itr < MACP::CPTypeEnum::CPType_Max; ++itr )
	{
		float fHeightDelta = 0.03f * itr;

		sprintf_s( szBuf, "%s : %d", szCPName[itr], sData.pScore[itr] );
		EternityEngine::DrawText2D( EtVector2( 0.35f + fWidthDot, 0.06f + fHeightDot + fHeightDelta ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.35f, 0.06f + fHeightDelta ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );

		sprintf_s( szBuf, "%s : %d", szCPName[itr], pIndexCPData[itr] );
		EternityEngine::DrawText2D( EtVector2( 0.5f + fWidthDot, 0.06f + fHeightDot + fHeightDelta ), szBuf, D3DCOLOR_ARGB(255,0,0,0) );
		EternityEngine::DrawText2D( EtVector2( 0.5f, 0.06f + fHeightDelta ), szBuf, D3DCOLOR_ARGB(255,255,255,0) );
	}
#endif	// #if defined( PRE_ADD_CP_RENEWAL )
}

void CDnGameTask::ShowVersion_Information()
{
#if !defined( _FINAL_BUILD )
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

	ShowDebugInfo();
#else	// #if !defined( _FINAL_BUILD )
	if( CDnActor::s_hLocalActor && ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel() && CGlobalInfo::GetInstance().m_bShowVersion )
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
#endif	//	#if !defined( _FINAL_BUILD )
}

void CDnGameTask::ProcessDebugMsg( LOCAL_TIME LocalTime, float fDelta )
{
#if !defined( _FINAL_BUILD )
	if( CGlobalValue::GetInstance().m_bShowFPS )
		ShowFPS_Information(fDelta);

	if( CGlobalValue::GetInstance().m_bShowDPS )
		ShowDPS_Information( fDelta );

	if( CGlobalValue::GetInstance().m_bShowCurrentAction )
		ShowCurrentAction_Information();


	// View monster action (#46533 [Snapshot Improvement] Cheat key to check monster action)
	if (CGlobalValue::GetInstance().m_bShowMonsterAction)
		ShowMonsterAction_Information();

	if( CGlobalValue::GetInstance().m_bShowCP )
		ShowCP_Information();

#endif	// #if !defined( _FINAL_BUILD )


	ShowVersion_Information();
}

void CDnGameTask::ProcessSound( LOCAL_TIME LocalTime, float fDelta )
{
	if( CEtSoundEngine::IsActive() ) {
		CEtSoundEngine::GetInstance().Process( fDelta );
		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();

		MatrixEx matEx;
		if( hCamera ) {
			switch( hCamera->GetCameraType() ) {
				case CDnCamera::PlayerCamera:
					{
						DnActorHandle hAttachActor = ((CDnPlayerCamera*)hCamera.GetPointer())->GetAttachActor();
						if( hAttachActor ) {
							matEx = *hAttachActor->GetMatEx();
							MatrixEx MatExTemp;
							MatExTemp.m_vXAxis = *hAttachActor->GetMoveVectorX();
							MatExTemp.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
							MatExTemp.m_vZAxis = *hAttachActor->GetMoveVectorZ();

							MatExTemp.RotateYaw( -11.0003566f );
							matEx.m_vZAxis = MatExTemp.m_vZAxis;
						}
						else matEx = *hCamera->GetMatEx();
					}
					break;
				case CDnCamera::NpcTalkCamera:
					{
						matEx = *CDnActor::s_hLocalActor->GetMatEx();
						matEx.m_vZAxis = *CDnActor::s_hLocalActor->GetLookDir();
					}
					break;
				default:
					matEx = *hCamera->GetMatEx();
					break;
			}
		}
		else {
			if( CDnActor::s_hLocalActor ) matEx = *CDnActor::s_hLocalActor->GetMatEx();
		}

		CEtSoundEngine::GetInstance().SetListener( matEx.m_vPosition, matEx.m_vZAxis, matEx.m_vYAxis );
	}
}

bool CDnGameTask::InitializeStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex )
{
	DebugLog("CDnGameTask::InitializeStage :: start");

	m_pWorld = &CDnWorld::GetInstance();
	m_nMapTableID = nMapTableID;

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	CalcStageDamageLimit( nMapTableID );
#endif

	m_nStageConstructionLevel = nStageConstructionLevel;
	m_nStartPositionIndex = nStartPositionIndex;

	// World Initialize
	if( InitializeWorld( szGridName ) == false ) return false;

	// Initialize Actor
	CDnActor::InitializeClass();
	// Initialize Weapon
	CDnWeapon::InitializeClass();
	// Initialize Drop Item
	CDnDropItem::InitializeClass();
	// Initialize Etc
	CDnEtcObject::InitializeClass();
	// Initialize globally used resources

	if( m_pWorld->ForceInitialize() == false )
	{
		ErrorLog("CDnGameTask::InitializeStage :: m_pWorld->ForceInitialize() failed...");
		return false;
	}

	m_bSyncStartToggleWeapon = true;
	SAFE_RELEASE_SPTR( m_hDummyCamera );
	// Create one debug camera and one player camera
	m_hDebugCamera = (new CDnFreeCamera)->GetMySmartPtr();
	m_hPlayerCamera = (new CDnPlayerCamera)->GetMySmartPtr();
	CDnCamera::SetActiveCamera( m_hPlayerCamera );

	// Environment and minimap settings
	InitializeEnvi( m_nMapTableID, szGridName, szEnviName );

	if( m_pWorld ) m_pWorld->PlayBGM();

	m_bEnableDungeonClearLeaderWarp = false;

	if(CDnVehicleTask::IsActive())
		GetVehicleTask().RemoveInstantVehicleItem();

	DebugLog("CDnGameTask::InitializeStage :: end");
	return true;
}

bool CDnGameTask::InitializeNextStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex )
{
	DnDebugConsole::LogA( "INSTANCE", "InitializeNextStage: MapID=%d Diff=%d StartPos=%d Grid=%s PartyCount=%u",
		nMapTableID, nStageConstructionLevel, nStartPositionIndex, szGridName ? szGridName : "", (UINT)CDnPartyTask::GetInstance().GetPartyCount() );
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);
		
		if( !pParty || !pParty->hActor || !pParty->hActor->IsPlayerActor()) 
			continue;
		
		((CDnPlayerActor*)pParty->hActor.GetPointer())->UnRideVehicle(true);
		((CDnPlayerActor*)pParty->hActor.GetPointer())->EndCannonMode();
	}

	DebugLog("CDnGameTask::InitializeNextStage :: start");
	DnCameraHandle hActiveCamera = CDnCamera::GetActiveCamera();
	if( hActiveCamera != m_hPlayerCamera ) {
		switch( hActiveCamera->GetCameraType() ) {
			case CDnCamera::PlayerCamera:
			case CDnCamera::FreeCamera:
				break;
			default:
				SAFE_RELEASE_SPTR( hActiveCamera );
				break;
		}
		CDnCamera::SetActiveCamera( m_hPlayerCamera );
	}

	SAFE_DELETE( m_pMinimap );
	if( m_pWorld ) m_pWorld->Destroy();

	GetCurRenderStack()->EmptyRenderElement();

	// 2009.4.13 Removed after discussing with Jiho due to status effect duration issue. -Hanki
	CDnDropItem::ReleaseClass();
	CDnWeapon::ReleaseClass( CDnWeapon::Projectile );
	CDnEtcObject::ReleaseClass();

	if (GetInterface().IsShowItemChoiceDialog())
		GetInterface().CloseItemChoiceDialog();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	CDnWorld::MapTypeEnum PrevMapType = (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr( m_nMapTableID, "_MapType" )->GetInteger();

	m_nMapTableID = nMapTableID;

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	CalcStageDamageLimit( nMapTableID );
#endif

	m_nStageConstructionLevel = nStageConstructionLevel;
	m_nStartPositionIndex = nStartPositionIndex;

	m_nGateIndex = -1;

	m_hLocalPlayerActor->RemoveOctreeNode();
	m_hLocalPlayerActor->ResetActor();

	if( m_pWorld->GetMapType() != PrevMapType ) 
	{
		if( CDnActor::s_hLocalActor ) ((CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->ResetCP();
	}
	m_bSyncStartToggleWeapon = true;
	if( m_pWorld->GetMapType() == CDnWorld::MapTypeWorldMap )
	{
		m_bSyncStartToggleWeapon = false;
	}

	// Scope for Lock
	{
		ScopeLock<CSyncLock> Lock( m_ConnectListLock );
		for( DWORD i=0; i<m_VecConnectList.size(); i++ ) {
			if( !m_VecConnectList[i].hActor ) continue;
			m_VecConnectList[i].hActor->RemoveOctreeNode();
			m_VecConnectList[i].hActor->ResetActor();
			if( m_bSyncStartToggleWeapon )
				((CDnPlayerActor*)m_VecConnectList[i].hActor.GetPointer())->SetBattleMode( false );
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_VecConnectList[i].hActor.GetPointer());
			if( pPlayer->IsDie() && !pPlayer->IsGhost() ) {
				pPlayer->ToggleGhostMode( true );
			}
		}
	}

	SAFE_RELEASE_SPTRVEC( m_hVecMonsterList );
	SAFE_RELEASE_SPTRVEC( m_hVecNpcList );
	SAFE_DELETE_PVEC( m_pVecActorBirthList );

	if( CDnActor::GetProcessCount() != CDnPartyTask::GetInstance().GetPartyCount() )
	{
		CDnActor::InitializeNextStage();
	}
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
#endif

	// World Initialize
	if( InitializeWorld( szGridName ) == false ) return false;

	// Initialize Actor
	CDnActor::InitializeClass();
	CDnStateBlow::StopProcess();
	// Initialize Drop Item
	CDnDropItem::InitializeClass();
	// Initialize Etc
	CDnEtcObject::InitializeClass();
	// Initialize globally used resources

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	GetQuestTask().ClearNpcReaction();
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pParty ) continue;
		if( !pParty->hActor ) {
			bool bLocal = (CDnBridgeTask::IsActive() && pParty->nSessionID == CDnBridgeTask::GetInstance().GetSessionID());
			InitializePlayerActor( pParty, i, bLocal );
			if( !pParty->hActor ) continue;
		}
		else {
			pParty->hActor->OnInitializeNextStage();
		}

		if( m_bSyncStartToggleWeapon )
			((CDnPlayerActor*)pParty->hActor.GetPointer())->CDnPlayerActor::CmdToggleBattle( false );
	}

	if( m_pWorld->ForceInitialize() == false )
	{
		ErrorLog("CDnGameTask::InitializeNextStage :: m_pWorld->ForceInitialize() failed...");
		return false;
	}

	DnActorHandle hActor;
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pParty || !pParty->hActor ) continue;
		SetStartPosition( CDnPartyTask::GetInstance().GetPartyData(i), i, CDnPartyTask::GetInstance().GetPartyData(i)->nSessionID );
		hActor = CDnPartyTask::GetInstance().GetPartyData(i)->hActor;
		((CDnPlayerActor*)hActor.GetPointer())->GetStateBlow()->RestoreAllBlowGraphicEffect();
		hActor->InitializeUsingEnvironmentEffect();
	}

	if( m_hLocalPlayerActor ) m_hLocalPlayerActor->InsertOctreeNode();
	{ // Scope for Lock
		ScopeLock<CSyncLock> Lock( m_ConnectListLock );

		for( DWORD i=0; i<m_VecConnectList.size(); i++ ) {
			if( !m_VecConnectList[i].hActor ) continue;
			m_VecConnectList[i].hActor->InsertOctreeNode();
		}
	}

	// Handle GM intrusion.
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pParty || !pParty->hActor ) continue;
		if( pParty->hActor == CDnActor::s_hLocalActor ) {
			((CDnLocalPlayerActor*)pParty->hActor.GetPointer())->SetAccountHide( true );
			continue;
		}
		if( pParty->bGMTrace ) {
			pParty->hActor->Show( false );
			pParty->hActor->SetProcess( false );
		}
	}

	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pParty || !pParty->hActor ) continue;
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(pParty->hActor.GetPointer());
		if(pPlayer)
			pPlayer->RideVehicle(pPlayer->GetVehicleInfo(),true);
	}

	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) 
	{
		CDnPartyTask::PartyStruct* pParty = CDnPartyTask::GetInstance().GetPartyData( i );
		if( !pParty || !pParty->hActor ) continue;
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( pParty->hActor.GetPointer() );
		if( pPlayer )
		{
//			pPlayer->SummonPet( -1 );
			GetPetTask().OnRecvPetEquipList( pPlayer, pPlayer->GetPetInfo() );
			pPlayer->RefreshState( CDnActorState::RefreshEquip );
		}
	}

	CDnLocalPlayerActor::StopAllPartyPlayer();

	InitializeEnvi( m_nMapTableID, szGridName, szEnviName );

	if( m_pWorld ) m_pWorld->PlayBGM();

	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->IsDie() ) {
		CDnLocalPlayerActor* pPlayer = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if (pPlayer)
			pPlayer->SetWorldColorFilterOnDie();
	}

	GetGameTipTask().ResetGameTip();

	DebugLog("CDnGameTask::InitializeNextStage :: end");

	// Once the drop-in task moves through a portal, treat as a normal user.
	if( m_bIsBreakIntoTask )
		m_bIsBreakIntoTask = false;

	m_bEnableDungeonClearLeaderWarp = false;

#ifdef PRE_ADD_CASH_COSTUME_AURA 
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) 
	{
		CDnPartyTask::PartyStruct *pParty = CDnPartyTask::GetInstance().GetPartyData(i);

		if( !pParty || !pParty->hActor || !pParty->hActor->IsPlayerActor()) 
			continue;

		if( ((CDnPlayerActor*)pParty->hActor.GetPointer())->IsEnableCostumeAura() )
		{
			((CDnPlayerActor*)pParty->hActor.GetPointer())->LoadCostumeAuraEffect(true);
		}
	}
#endif

	return true;
}

DnActorHandle CDnGameTask::InitializePlayerActor( CDnPartyTask::PartyStruct *pStruct ,int nPartyIndex , bool bLocalPlayer, bool bIsBreakInto/*=false*/ )
{
	DnActorHandle hActor;

	hActor = CreateActor( pStruct->cClassID, bLocalPlayer, false, false );
	if( !hActor ) return CDnActor::Identity();
	if( bIsBreakInto )
		InterlockedIncrement( &hActor->m_lBreakIntoLoadingFlag );
	hActor->Show( false );
	hActor->SetPosition( EtVector3( 0.f, -100000.f, 0.f ) );
	CDnPlayerActor *pActor = (CDnPlayerActor *)hActor.GetPointer();
	pActor->SetDefaultPartsInfo( pStruct->nDefaultPartsIndex );

	hActor->Initialize();
	hActor->SetUniqueID( pStruct->nSessionID );
	pActor->SetJobHistory( pStruct->nVecJobHistoryList );
	pActor->SetLevel( pStruct->cLevel );
	pActor->SetExperience( pStruct->nExp );
	pActor->InitializeSession( this );
	pActor->SetPvPLevel( pStruct->cPvPLevel );

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	pActor->RecomposeAction();
#endif

	pActor->SetWeaponViewOrder( 0, GetBitFlag(pStruct->cViewCashEquipBitmap, CASHEQUIP_WEAPON1) );
	pActor->SetWeaponViewOrder( 1, GetBitFlag(pStruct->cViewCashEquipBitmap, CASHEQUIP_WEAPON2) );

	for( int i = 0; i < CDnParts::SwapParts_Amount; ++i )
		pActor->SetPartsViewOrder( i, GetBitFlag(pStruct->cViewCashEquipBitmap, i) );

	pActor->SetHideHelmet( GetBitFlag(pStruct->cViewCashEquipBitmap, HIDEHELMET_BITINDEX) );


	pActor->SetGuildSelfView( pStruct->GuildSelfView );
	pActor->SetPvPLevel( pStruct->cPvPLevel );
	if( bLocalPlayer ) pActor->SetAccountLevel( CGlobalInfo::GetInstance().m_cLocalAccountLevel );
	pStruct->hActor = hActor;
	pActor->SetPartsColor( MAPartsBody::HairColor, pStruct->dwHairColor );
	pActor->SetPartsColor( MAPartsBody::EyeColor, pStruct->dwEyeColor );
	pActor->SetPartsColor( MAPartsBody::SkinColor, pStruct->dwSkinColor );

	AddConnectUser( hActor, pStruct->wszCharacterName, bLocalPlayer );

	pActor->SetVehicleSessionID(pStruct->nSessionID+1); // The mount's session ID is the character's ID + 1; each player's session ID increments by +5.
                                                        // Handled this way because party struct does not contain mount session ID info.

	GetPetTask().OnRecvPetEquipList( pActor, pStruct->PetInfo );

	if(m_bIsPVPShowHelmetMode)
		pActor->SetPVPShowHelmet(true); // Inside MAPARTSBODY 
	if( bLocalPlayer ) {
		// Normal Item
		for( int j=0; j<EQUIP_WEAPON1; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetEquipItem(j);
			if( !pItem ) continue;
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)j );
			}
			ThreadDelay();
		}
		for( int j=0; j<2; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetEquipItem(j + EQUIP_WEAPON1);
			if( !pItem ) continue;
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				hWeapon->SetUniqueID( CDnWeapon::s_dwUniqueCount );
				CDnWeapon::s_dwUniqueCount++;
				pActor->AttachWeapon( hWeapon, j );
			}
			ThreadDelay();
		}

		// Cash Item
		for( int j=0; j<CASHEQUIP_WEAPON1; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetCashEquipItem(j);
			if( !pItem ) continue;
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)j );
			}
			ThreadDelay();
		}
		for( int j=0; j<2; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetCashEquipItem(j + CASHEQUIP_WEAPON1);
			if( !pItem ) continue;
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				hWeapon->SetUniqueID( CDnWeapon::s_dwUniqueCount );
				CDnWeapon::s_dwUniqueCount++;
				pActor->AttachCashWeapon( hWeapon, j, false, false );
			}
			ThreadDelay();
		}

		// Setup when local character enters game server
		CDnItem *pVehicleEffectItem = CDnItemTask::GetInstance().GetCashEquipItem(CASHEQUIP_EFFECT);
		if(pVehicleEffectItem)
		{
			if( dynamic_cast<CDnPartsVehicleEffect*>(pVehicleEffectItem) && pVehicleEffectItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT )
			{
				DnPartsVehicleEffectHandle hVehicleEffect = ((CDnPartsVehicleEffect*)pVehicleEffectItem)->GetMySmartPtr();
				if(hVehicleEffect)
				{
					pActor->SetPartsVehicleEffect(hVehicleEffect);
				}
			}
		}

		// When entering game server first (e.g., tutorial), skill list arrives before job id.
		// Therefore, when job info arrives, initialize skill tree info with the received skill list.
		// TODO: Later loop over job history.
		if( pActor->IsNeedInitializeSkillTree() )
			GetSkillTask().InitializeSkillTree();

		GetSkillTask().InitializeLocalPlayerSkills();
		GetInterface().RefreshSkillDialog();

		// Update skill point usage limit per job
		static_cast<CDnLocalPlayerActor*>(pActor)->UpdateSPUsingLimitByJob();

		// Glyph Item
		for( int j=0; j<GLYPHMAX; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetGlyphItem(j);
			if( !pItem ) continue;
			if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
				DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
				pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)j );
			}
			ThreadDelay();
		}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
		// Talisman Item
		for( int j=0; j<TALISMAN_MAX; j++ ) {
			CDnItem *pItem = CDnItemTask::GetInstance().GetTalismanItem(j);
			if( !pItem ) continue;
			if( dynamic_cast<CDnTalisman*>(pItem) && pItem->GetItemType() == ITEMTYPE_TALISMAN ) {
				DnTalismanHandle hTalisman = ((CDnTalisman*)pItem)->GetMySmartPtr();
				pActor->AttachTalisman( hTalisman, j, CDnItemTask::GetInstance().GetTalismanEfficiency(j) );
			}
			ThreadDelay();
		}
#else
		// Talisman Item
#endif

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
		pActor->PreloadSkillSoundSignal();
#endif
		pActor->InitializeIdleMotion();
	}
	else {
		// Normal Item
		// Equip Attach
		for( DWORD j=0; j<pStruct->VecParts.size(); j++ ) {
			TItemInfo *pInfo = &pStruct->VecParts[j];
			if( pInfo->Item.nItemID < 1 ) continue;
			CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
			if( !pItem ) continue;
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex, true );
			}
			ThreadDelay();
		}
		// Weapon Attach
		for( DWORD j=0; j<2; j++ ) {
			TItemInfo *pInfo = &pStruct->Weapon[j];
			if( pInfo->Item.nItemID < 1 ) continue;
			CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
			if( !pItem ) continue;
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				pActor->AttachWeapon( hWeapon, j, true );
			}
			ThreadDelay();
		}
		// Cash Item
		// Equip Attach
		for( DWORD j=0; j<pStruct->VecCashParts.size(); j++ ) {
			TItemInfo *pInfo = &pStruct->VecCashParts[j];
			if( pInfo->Item.nItemID < 1 ) continue;
			CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
			if( !pItem ) continue;
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex, true );
			}
			else if(dynamic_cast<CDnPartsVehicleEffect*>(pItem) && pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
			{
				DnPartsVehicleEffectHandle hVehicleEffect = ((CDnPartsVehicleEffect*)pItem)->GetMySmartPtr();
				pActor->SetPartsVehicleEffect(hVehicleEffect, true);
			}

			ThreadDelay();
		}

		// Weapon Attach
		for( DWORD j=0; j<2; j++ ) {
			TItemInfo *pInfo = &pStruct->CashWeapon[j];
			if( pInfo->Item.nItemID < 1 ) continue;
			CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
			if( !pItem ) continue;
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				pActor->AttachCashWeapon( hWeapon, j, true, false );
			}
			ThreadDelay();
		}

		// Passive & EnchantPassive Skill - Enchant passive skills must also match so enchanted skills used by others update properly in my client.
		for( DWORD j=0; j<pStruct->VecSkill.size(); j++ ) {
			TSkill *pInfo = &pStruct->VecSkill[j];

			if( hActor->CanAddSkill(pInfo->nSkillID,pInfo->cSkillLevel) == true )
				hActor->AddSkill( pInfo->nSkillID, pInfo->cSkillLevel );
			ThreadDelay();
		}

		// Glyph Item
		for( DWORD j=0; j<pStruct->VecGlyph.size(); j++ ) {
			TItemInfo *pInfo = &pStruct->VecGlyph[j];
			if( pInfo->Item.nItemID < 1 ) continue;
			CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
			if( !pItem ) continue;
			if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
				DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
				pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)pInfo->cSlotIndex, true );
			}
			ThreadDelay();
		}

		// Talisman Item

		hActor->GetStateBlow()->Process( 0, 0.f, true );
	}
	pActor->RefreshHideHelmet();

	pActor->SetBattleMode( false );

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
	pPartsBody->CombineParts();
	GetInterface().UpdateMyPortrait();

	pActor->SetAppellationIndex( pStruct->nSelectCoverAppellation, pStruct->nSelectAppellation );
	pActor->SetAccountLevel(pStruct->cAccountLevel);
#if defined(PRE_ADD_VIP)
	if (bLocalPlayer == false)
		pActor->SetVIP(pStruct->bVIP);
#endif	// #if defined(PRE_ADD_VIP)
	hActor->RefreshState();
	hActor->SetHP( hActor->GetMaxHP() );
	hActor->SetSP( hActor->GetMaxSP() );

	if( pStruct->bSyncHPSP )
	{
		hActor->SetHP( pStruct->iSyncHP );
		hActor->SetSP( pStruct->iSyncSP );
	}
	pActor->SetBattleMode( pStruct->cSyncBattleMode ? true : false );

	if( hActor->GetHP() <= 0 )
		hActor->SetActionQueue( "Die_Down", 0, 0.f );
	else
		hActor->SetActionQueue( "Stand", 0, 0.f );

	SetStartPosition( pStruct, nPartyIndex, pStruct->nSessionID );	

	pActor->SetVehicleInfo(pStruct->VehicleInfo); // Set in this case.
	pActor->RideVehicle(pStruct->VehicleInfo,true);

#ifdef PRE_ADD_NAMEDITEM_SYSTEM
	if( CDnItemTask::IsActive() )
		CDnItemTask::GetInstance().ApplyBackUpEffectSkillData( hActor );
#endif

	if (bLocalPlayer)
	{
		hActor->Show( true );
		hActor->SetProcess( true );

		if ( pStruct->bGMTrace )
			((CDnLocalPlayerActor*)pActor)->SetAccountHide( true );
	}
	else
	{
		bool bEnableProcess = false;
		bEnableProcess = !pStruct->bGMTrace;

		hActor->Show( bEnableProcess );
		hActor->SetProcess( bEnableProcess );
	}

#ifdef _SHADOW_TEST
	if( hActor->IsEnableShadowActor() )
		if( hActor->GetShadowActorHandle() ) {
			pPartsBody = dynamic_cast<MAPartsBody*>(hActor->GetShadowActorHandle().GetPointer());
			pPartsBody->CombineParts();
		}
#endif

#if !defined( _FINAL_BUILD )
	WCHAR wszBuf[MAX_PATH];
	wsprintf( wszBuf, L"%s HP:%d SP:%d", hActor->GetName(), hActor->GetHP(), hActor->GetSP() );
	GetInterface().AddChatMessage( CHATTYPE_NORMAL, L"", wszBuf  );
#endif

	return hActor;
}

bool CDnGameTask::InitializeAllPlayerActor()
{	
	DnDebugConsole::LogW( L"INSTANCE", L"InitializeAllPlayerActor: PartyCount=%u", (UINT)CDnPartyTask::GetInstance().GetPartyCount() );
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) 
	{
		bool bLocalPlayer = false;
		
		CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
		
		if (pStruct && CDnBridgeTask::IsActive() && pStruct->nSessionID == CDnBridgeTask::GetInstance().GetSessionID()) {
//			if( Initialize() == false ) return false;
			bLocalPlayer = true;			
		}

		if( pStruct )
		{
			bool bIsBot = ( pStruct->nSessionID >= 0x70000000 );
			DnDebugConsole::LogW( bIsBot ? L"BOT" : L"PARTY", L"  Spawn[%u] %s SessionID=%u Name=%s Class=%d Lv=%d Local=%d",
				i, bIsBot ? L"[BOT]" : L"[USER]", pStruct->nSessionID, pStruct->wszCharacterName, (int)pStruct->cClassID, (int)pStruct->cLevel, bLocalPlayer ? 1 : 0 );
		}

#ifdef PRE_MOD_SYNCPACKET
		if (pStruct->IsCompleteStruct() == false)
			_ASSERT(0);
#endif		//#ifdef PRE_MOD_SYNCPACKET

		InitializePlayerActor( pStruct ,i , bLocalPlayer );

		//blondy end
	}

#ifdef PRE_ADD_BESTFRIEND
	SetBestFriendData();
#endif

	return true;
}

void CDnGameTask::RefreshConnectUser()
{
	if( !CDnPartyTask::IsActive() ) return;

	ScopeLock<CSyncLock> Lock( m_ConnectListLock );

	for( DWORD i=0; i<m_VecConnectList.size(); i++ ) {
		bool bValid = false;

		//blondy bugfix
		for( DWORD j=0; j<CDnPartyTask::GetInstance().GetPartyCount(); j++ ) {
			if(  m_VecConnectList[i].hActor && ( m_VecConnectList[i].hActor->GetUniqueID() == CDnPartyTask::GetInstance().GetPartyData(j)->nSessionID )) {
				bValid = true;
				break;
			}
		}
		CDnPartyTask::GetInstance().LockBreakIntoPartyList( true );
		
		for( DWORD j=0; j<CDnPartyTask::GetInstance().GetBreakIntoPartyCountWithoutSync(); j++ ) 
		{
			if(  m_VecConnectList[i].hActor && ( m_VecConnectList[i].hActor->GetUniqueID() == CDnPartyTask::GetInstance().GetBreakIntoPartyDataWithoutSync(j)->nSessionID )) 
			{
				bValid = true;
				break;
			}
		}
		CDnPartyTask::GetInstance().LockBreakIntoPartyList( false );

		//blondy end
		if( !bValid ) {
//			SAFE_RELEASE_SPTR( m_VecConnectList[i].hActor );
			if( m_VecConnectList[i].hActor ) 
			{
				if( m_VecConnectList[i].hActor->m_lBreakIntoLoadingFlag > 0 ) {
					InterlockedIncrement( &m_VecConnectList[i].hActor->m_lBreakIntoLoadingFlag );
					continue;
				}

				// Rotha: If exiting the game server while mounted on a vehicle, remove the vehicle first.
				CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(m_VecConnectList[i].hActor.GetPointer());
				if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
					pPlayer->GetMyVehicleActor()->SetDestroy();


#ifndef _FINAL_BUILD
				std::wstring wszName = m_VecConnectList[i].hActor->GetName();
				WCHAR wszTemp[256] = {0,};
				swprintf_s( wszTemp, 256, L"Destroy PlayerActor Name = %s", wszName.c_str() );
				OutputDebugStringW( wszTemp );
#endif

				m_VecConnectList[i].hActor->SetDestroy();
				// If GM is in Hide state, they are excluded from Process, so force delete.
				if( !m_VecConnectList[i].hActor->IsProcess() ) {
					SAFE_RELEASE_SPTR( m_VecConnectList[i].hActor );
				}
			}
			
			m_VecConnectList.erase( m_VecConnectList.begin() + i );

			i--;
		}
	}
}

bool CDnGameTask::InitializeStage( int nCurrentMapIndex, int nGateIndex, int nStageConstructionLevel, int nRandomSeed, bool bContinueStage, bool bDirectConnect )
{
	s_bCompleteFirstRender = false;
	CDnMonitorObjectPauseHelper HangPauser(this);

	GetInterface().CloseNpcDialog();
	GetInterface().FinalizeMODDialog();
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );

	m_bFinalizeStage = false;
	m_bInitComplete = false;

	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, 0.f, true );
	if( CDnPartyTask::IsActive() ) {
		CDnPartyTask::GetInstance().ResetSync();
		CDnPartyTask::GetInstance().ResetGateIndex();
		CDnPartyTask::GetInstance().SetPartyState(CDnPartyTask::NORMAL);
		m_bSyncComplete = false;
	}
	CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( pCommonTask ) pCommonTask->ResetPlayedCutScene();

	//GetInterface().RefreshPartyGateInfo();
	//GetInterface().ClearListDialog();

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );

	char szLabel[64];
	CDnWorld::MapTypeEnum MapType;
	CDnWorld::MapSubTypeEnum MapSubType;
	int nMapIndex;
	int nStartPositionIndex;
	int nDungeonEnterTableID;
	int nUsableRevbirthCoinCount = 0;
	CDnWorld::LandEnvironment Environment;
	std::string szGridName;
	int nGridIndex = 0;
//	std::vector<std::string> szVecWeatherName[CDnWorld::WeatherEnum_Amount];

	if( bDirectConnect ) {
		nMapIndex = nCurrentMapIndex;
		nStartPositionIndex = nGateIndex;
	}
	else {
		// Originally obtained via InitializeGateInfo..
		// Unpack it and change it to get only what is needed.
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( nCurrentMapIndex <= 0 ) return false;
		if( nGateIndex < 1 || nGateIndex > DNWORLD_GATE_COUNT ) return false;

		std::vector<int> vecMapIndex, vecStartGate;
		CDnWorld::GetGateMapIndex( nCurrentMapIndex, nGateIndex - 1, vecMapIndex );
		CDnWorld::GetGateStartGate( nCurrentMapIndex, nGateIndex - 1, vecStartGate );

		BYTE cGateSelectIndex = CDnBridgeTask::GetInstance().GetGateSelectIndex();
		if( vecMapIndex.size() > cGateSelectIndex )
			nMapIndex = vecMapIndex[cGateSelectIndex];
		else
			nMapIndex = vecMapIndex[0];

		if( vecStartGate.size() > cGateSelectIndex )
			nStartPositionIndex = vecStartGate[cGateSelectIndex];
		else
			nStartPositionIndex = vecStartGate[0];
	}

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
	_srand( nRandomSeed );
	OutputDebug( "RandomSeed : %d\n", nRandomSeed );

	if( vecToolMapInfo.empty() ) return false;
	int nArrayIndex = _rand()%(int)vecToolMapInfo.size();
	OutputDebug( "MapArrayIndex : %d\n", nArrayIndex );

	szGridName = vecToolMapInfo[nArrayIndex].second;
	nGridIndex = nArrayIndex;
	if( vecToolMapInfo[nArrayIndex].first > 0 )
	{
		nMapIndex = vecToolMapInfo[nArrayIndex].first;
		nGridIndex = 0;
	}
#endif
	
	MapType = (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapType" )->GetInteger();
	MapSubType = (CDnWorld::MapSubTypeEnum)pSox->GetFieldFromLablePtr( nMapIndex, "_MapSubType" )->GetInteger();
	Environment = (CDnWorld::LandEnvironment)pSox->GetFieldFromLablePtr( nMapIndex, "_Environment" )->GetInteger();
	int iAllowMapType = pSox->GetFieldFromLablePtr( nMapIndex, "_AllowMapType" )->GetInteger();

	switch( MapType ) {
		case CDnWorld::MapTypeWorldMap:
			CDnItemTask::GetInstance().SetMaxUsableRebirthCoin( -1 );
			break;
	}

	//Initialize Sea Dragon Nest settings..
	CDnWorld::MapTypeEnum ePrevWorld = CDnWorld::GetInstance().GetMapType();
	switch(MapType)
	{
	case CDnWorld::MapTypeDungeon:
		switch(ePrevWorld)
		{
		case CDnWorld::MapTypeDungeon:
			break;
			//Village/World -> Dungeon
		case CDnWorld::MapTypeWorldMap:
		case CDnWorld::MapTypeVillage:
			CDnWorld::GetInstance().SetDragonNestType(eDragonNestType::None);
			break;
		}
		break;
		// From anywhere -> when going to the world map
	case CDnWorld::MapTypeWorldMap:
		CDnWorld::GetInstance().SetDragonNestType(eDragonNestType::None);
		break;	
	}

	nDungeonEnterTableID = pSox->GetFieldFromLablePtr( nMapIndex, "_EnterConditionTableID" )->GetInteger();
	if( ePrevWorld != CDnWorld::MapTypeDungeon && nDungeonEnterTableID > 0 )
	{
		DNTableFileFormat* pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
		if( pDungeonSOX->IsExistItem( nDungeonEnterTableID ) ) 
		{
			nUsableRevbirthCoinCount = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_MaxUsableCoin" )->GetInteger();
			// Set usable coins
			CDnItemTask::GetInstance().SetMaxUsableRebirthCoin( nUsableRevbirthCoinCount );
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().Initialize( this, nMapIndex, nStageConstructionLevel );

#ifdef _FINAL_BUILD
	// Called to immediately clear preloaded actors when leaving game server from village, but too slow in dev build so gated with a define.
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
#endif

	CDnWorld::GetInstance().SetMapType( MapType );
	CDnWorld::GetInstance().SetMapSubType( MapSubType );
	CDnWorld::GetInstance().SetLandEnvironment( Environment );
	CDnWorld::GetInstance().SetAllowMapType( iAllowMapType );

#if !defined(PRE_ADD_RENEW_RANDOM_MAP)
	std::vector<std::string> szVecToolMapName;
	std::string szTemp;

	for( int i=0; i<10; i++ ) {
		sprintf_s( szLabel, "_ToolName%d", i + 1 );
		szTemp = pSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetString();
		if( szTemp.empty() ) continue;

		szVecToolMapName.push_back( szTemp );
	}

	OutputDebug( "RandomSeed : %d\n", nRandomSeed );
	// Sync RandomSeed

	_srand( nRandomSeed );
	if( szVecToolMapName.empty() ) return false;
	int nArrayIndex = _rand()%(int)szVecToolMapName.size();

	szGridName = szVecToolMapName[nArrayIndex];
	nGridIndex = nArrayIndex;
#endif

	CDnWorld::WeatherEnum Weather = CDnWorld::FineDay;
	switch( GetGameTaskType() ) {
		case GameTaskType::Normal:
		case GameTaskType::DarkLair:
		case GameTaskType::Farm:
			Weather = CDnWorld::GetInstance().GetNextWeather( nMapIndex ); 
			CDnWorld::GetInstance().SetCurrentWeather( Weather );
			break;
		case GameTaskType::PvP:
			_srand( nRandomSeed );
			Weather = (CDnWorld::WeatherEnum)(_rand()%CDnWorld::WeatherEnum_Amount);
			CDnWorld::GetInstance().SetCurrentWeather( Weather );
			break;
	}

	std::string szEnviName = CDnWorld::GetEnviName( nMapIndex, nStageConstructionLevel, Weather );

	CGlobalInfo::GetInstance().m_nCurrentMapIndex = nMapIndex;

	// Update Nest clear limit count
	if( CDnNestInfoTask::IsActive() ) 
		CDnNestInfoTask::GetInstance().RefreshExpandTryCount();

	// Also synchronize RandomSeed
	_srand( nRandomSeed );
	DnDebugConsole::LogA( "INSTANCE", "Enter Game Stage: MapID=%d Type=%d SubType=%d Diff=%d StartPos=%d Continue=%d Direct=%d Seed=%d Grid=%s",
		nMapIndex, (int)MapType, (int)MapSubType, nStageConstructionLevel, nStartPositionIndex, bContinueStage ? 1 : 0, bDirectConnect ? 1 : 0, nRandomSeed, szGridName.c_str() );
	if( bContinueStage ) {
		if( InitializeNextStage( szGridName.c_str(), szEnviName.c_str(), nMapIndex, nStageConstructionLevel, nStartPositionIndex ) == false ) return false;
		if( PostInitializeStage( nRandomSeed ) == false ) return false;
	}
	else {
		if( InitializeStage( szGridName.c_str(), szEnviName.c_str(), nMapIndex, nStageConstructionLevel, nStartPositionIndex ) == false ) return false;
		if( InitializeAllPlayerActor() == false ) return false;
		if( PostInitializeStage( nRandomSeed ) == false ) return false;
	}

	m_pMinimap->InitializeNPC( nMapIndex );
	MAChatBalloon::ClearCount();

	CDnWorld::GetInstance().InitializeGateInfo( m_nMapTableID, nGridIndex );
	InitializeCP();

	m_nAggroValue = 0;
	m_WarTimeType = WarTimeEnum::None;
	SAFE_DELETE_VEC( m_hVecAggroList );

#ifdef PRE_FIX_PLAYER_CROSSHAIR_NOBATTLE
	m_bIsCombat = false;
	m_fCombatDelta = 0.f;
#endif

#if defined( PRE_ADD_CP_RANK )
	m_bDungeonClearBestInfo = false;
	m_DungeonClearBestInfo.Reset();
#endif	// #if defined( PRE_ADD_CP_RANK )

	m_BridgeState = eBridgeState::GameToGame;
	GetInterface().RefreshStageInfoDlg();
	GetInterface().RefreshPartyGuage();	
	GetInterface().RefreshQuestSummaryInfo();
	GetInterface().ShowHardCoreModeTimeAttackDlg( false );
	GetInterface().ResetHardCoreModeTimer();

	if( false == bContinueStage )
		GetItemTask().ResetSpecialRebirthItemID();

	if( bDirectConnect && pSox->GetFieldFromLablePtr( m_nMapTableID, "_MapType" )->GetInteger() == CDnWorld::MapTypeWorldMap ) {
		m_bShowDungeonOpenNotice = true;
	}

	if( CDnMainFrame::IsActive() )
		CDnMainFrame::GetInstance().FlushWndMessage();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
	CEtResource::FlushWaitDelete();
#endif

#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
	GetTableDB().SetTableAutoUnloadForGame( true );
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
	return true;
}

bool CDnGameTask::InitializeWorld( const char *szGridName )
{
	// Map Load
	bool bResult = m_pWorld->Initialize( CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szGridName, this );
//blondy
	if( bResult == false )
	{
		ErrorLog("CDnGameTask::InitializeWorld :: %s Not Found!",szGridName);
		return false;
	}
//blondy end

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	g_DataManager.InitNpcTalkData( m_nMapTableID );
	GetQuestTask().LoadQuestTalk();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	
	return true;
}

bool CDnGameTask::InitializeEnvi( int nMapIndex, const char *szGridName, const char *szEnviName )
{
	// Set camera Far value before initializing the environment map.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	int nCameraFar = -1;
	float fWaterFarRatio = 0.5f;
	if( pSox->IsExistItem( nMapIndex ) ) {
		nCameraFar = pSox->GetFieldFromLablePtr( nMapIndex, "_CameraFar" )->GetInteger();
		fWaterFarRatio = pSox->GetFieldFromLablePtr( nMapIndex, "_WaterFarProb" )->GetFloat();
	}
	
	// Load environment map
	m_pWorld->InitializeEnviroment( CEtResourceMng::GetInstance().GetFullName( szEnviName ).c_str(), m_hPlayerCamera, nCameraFar, fWaterFarRatio );
	m_pWorld->InitializeBattleBGM( nMapIndex, m_nStageConstructionLevel );

	m_pMinimap = new CDnMinimap();
	m_pMinimap->Initialize();

	char szTemp[_MAX_PATH];
	sprintf_s( szTemp, "%s\\Grid\\%s\\%s.dds", CEtResourceMng::GetInstance().GetFullPath( "MapData" ).c_str(), szGridName, szGridName );
	m_pMinimap->SetTextures( szTemp );

	CDnWorldEnvironment* pWorldEnv = CDnWorld::GetInstance().GetEnvironment();
	CDnLocalPlayerActor::SetSaturationColor( pWorldEnv->GetColorSaturation() );

	return true;
}

bool CDnGameTask::PostInitializeStage( int nRandomSeed )
{
	InitializeInterface();

	// Start with Weapon Unique ID set above a certain value, because the player uses the initial indices.
	CDnWeapon::s_dwUniqueCount = 100000;

#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
	PreGenerationMonster( nRandomSeed );
#else
	GenerationMonster( nRandomSeed );
	GenerationSetMonster( nRandomSeed );
#endif
	GenerationNpc( nRandomSeed );
	
	// Handle drop-in task
	_PostInitializeBreakIntoStage( nRandomSeed );
	
//	CClientSessionManager::GetInstance().ClearPacket();
	// Send that it is waiting.	
	m_bNeedSendReqSyncWait = true;

	ResetTimer();
	CInputDevice::GetInstance().ResetAllInput();

	m_bInitComplete = true;
	//GetInterface().RefreshPartyGateInfo();

	SendReqFriendList();
	GetIsolateTask().ReqIsolateList(true);

	if( OnPostInitializeStage() == false ) return false;

	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().EnableBackgroundLoading( true );
		CDnLoadingTask::GetInstance().WaitForComplete();		
	}

	DnDebugConsole::LogW( L"INSTANCE", L"PostInitializeStage complete: MapID=%d Monsters=%u NPCs=%u",
		m_nMapTableID, (UINT)m_hVecMonsterList.size(), (UINT)m_hVecNpcList.size() );
	return true;
}

bool CDnGameTask::_PostInitializeBreakIntoStage( int nRandomSeed )
{
	if( !m_bIsBreakIntoTask )
		return true;

	// 1. Process monsters
	for( UINT i=0 ; i<m_hVecMonsterList.size() ; ++i )
	{
		m_hVecMonsterList[i]->Show( false );
	}

	// 2. Process BrokenProp
	CDnWorldSector* pSector = static_cast<CDnWorldSector*>(CDnWorld::GetInstance().GetSector( 0.0f, 0.0f ));
	for( UINT i=0 ; i<pSector->GetPropCount() ; ++i )
	{
		CEtWorldProp* pProp = pSector->GetPropFromIndex( i );
		if( !pProp )
			continue;
		
		if( pProp->bIsCanBroken() )
		{
			static_cast<CDnWorldProp*>(pProp)->Show( false );
		}
	}

	// 3. NPC processing
	for( UINT i=0 ; i<m_hVecNpcList.size() ; ++i )
	{
		m_hVecNpcList[i]->Show( false );
	}

	return true;
}

void CDnGameTask::FinalizeStage()
{
	m_bIsCreateLocalActor = false;

	if( m_bFinalizeStage == true ) return;
	DnDebugConsole::LogW( L"INSTANCE", L"FinalizeStage: MapID=%d", m_nMapTableID );

	s_bCompleteFirstRender = false;
	CDnPartyTask* pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
	if( pPartyTask )
		pPartyTask->FinalizeGameTask();

	if( CDnActor::s_hLocalActor && CDnBridgeTask::IsActive() ) {
		CDnBridgeTask::GetInstance().SetCurrentCharLevel( CDnActor::s_hLocalActor->GetLevel() );
	}

	switch( GetGameTaskType() ) {
		case GameTaskType::Normal:
			GetInterface().Finalize( CDnInterface::Game );
			break;
		case GameTaskType::DarkLair:
			GetInterface().Finalize( CDnInterface::DLGame );
			break;
		case GameTaskType::PvP:
		{
			if( CDnActor::s_hLocalActor )
				CDnActor::s_hLocalActor->ResetSkillCoolTime();

			GetInterface().Finalize( CDnInterface::PVPGame );
			break;
		}
		case GameTaskType::Farm:
			GetInterface().Finalize( CDnInterface::Farm );
			break;
	}
	SAFE_DELETE( m_pStageClearImp );
	SAFE_DELETE_PVEC( m_pVecActorBirthList );

	if (GetInterface().IsShowItemChoiceDialog())
		GetInterface().CloseItemChoiceDialog();

	//END BLOWS
	if (CDnActor::s_hLocalActor)
	{
		OutputDebug("%s Finalize Blows", __FUNCTION__);
	}

	//blondy end
	GetCurRenderStack()->EmptyRenderElement();
	SAFE_DELETE( m_pMinimap );
	SAFE_RELEASE_SPTR( m_hDebugCamera );
	SAFE_RELEASE_SPTR( m_hPlayerCamera );
	SAFE_RELEASE_SPTR( m_hDummyCamera );
	CDnDropItem::ReleaseClass();
	CDnActor::ReleaseClass();
	CDnWeapon::ReleaseClass( CDnWeapon::Projectile );
	CDnEtcObject::ReleaseClass();
	CDnCamera::ReleaseClass();
	CClientSessionManager::GetInstance().RUDPDisConnect();
	SAFE_DELETE_VEC( m_hVecMonsterList );
	if( CDnLifeSkillPlantTask::IsActive() )
		GetLifeSkillPlantTask().FinalizeStage();
	if( CDnChatRoomTask::IsActive() )
		GetChatRoomTask().ClearChatRoomInfoList();

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	g_DataManager.ClearTalkData();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

	if( m_pWorld ) m_pWorld->Destroy();
	m_bFinalizeStage = true;
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif
	if( CEtFontMng::IsActive() )
		CEtFontMng::GetInstance().FlushFontCache();

#ifdef PRE_ADD_BESTFRIEND
	memset(&m_TempBFpacket,0,sizeof(SCPartyBestFriend));
#endif
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
	GetTableDB().SetTableAutoUnloadForGame( false );
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE

}

void CDnGameTask::AddConnectUser( DnActorHandle hActor, TCHAR *szUserID, bool bLocalPalyer )
{
	hActor->SetName( szUserID );

	if( bLocalPalyer ) {
		((CDnPlayerCamera*)m_hPlayerCamera.GetPointer())->AttachActor( hActor );
		m_hLocalPlayerActor = hActor;
		CDnActor::s_hLocalActor = hActor;
		m_bIsCreateLocalActor = true;

#ifdef PRE_ADD_BESTFRIEND
		GetItemTask().InitBestfriendInfo();
#endif

	}
	else CGameClientSession::AddConnectUser( hActor, szUserID );
}

void CDnGameTask::OnInitializeStartPosition( CDnPartyTask::PartyStruct *pStruct, const int iPartyIndex, EtVector3& vStartPos, float& fRotate, const UINT uiSessionID )
{
	DnActorHandle hActor = pStruct->hActor;

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
				case 0: vStartPos = pBox->Vertices[4];									break;
				case 1: vStartPos = pBox->Vertices[5];									break;
				case 2: vStartPos = pBox->Vertices[0];									break;
				case 3: vStartPos = pBox->Vertices[1];									break;
				case 4: vStartPos = pBox->Center + ( pBox->Axis[2] * pBox->Extent[2] ); break;
				case 5: vStartPos = pBox->Center - ( pBox->Axis[2] * pBox->Extent[2] ); break;
				case 6: vStartPos = pBox->Center - ( pBox->Axis[0] * pBox->Extent[0] ); break;
				case 7: vStartPos = pBox->Center + ( pBox->Axis[0] * pBox->Extent[0] ); break;
			}
			vStartPos.y = m_pWorld->GetHeight( vStartPos.x, vStartPos.z ) + pArea->GetMin()->y;

			fRotate = pArea->GetRotate();
			if( m_bLookStageEntrance ) {
				fRotate += 180.0f;
				m_bLookStageEntrance = false;
			}
		}
	}
}

bool CDnGameTask::OnInitializeBreakIntoActor( CDnPartyTask::PartyStruct* pStruct, const int iVectorIndex )
{
	_srand( m_nStageRandomSeed );
	DnActorHandle hActor = InitializePlayerActor( pStruct ,iVectorIndex, false, true );

#ifdef PRE_ADD_BESTFRIEND
	SetBestFriendData();
#endif

	if( !hActor )
		return false;

	hActor->Show( false );
	hActor->SetProcess( !pStruct->bGMTrace );

	return true;
}

void CDnGameTask::SetStartPosition( CDnPartyTask::PartyStruct* pStruct, int nPartyIndex, const UINT uiSessionID )
{
	DnActorHandle hActor = pStruct->hActor;

	// Get UnitArea and position the player.
	EtVector3 vStartPos;
	float fRotate = 0.f;

	OnInitializeStartPosition( pStruct, nPartyIndex, vStartPos, fRotate, uiSessionID );

	if( hActor ) 
	{
		EtVector2 vDir;

		hActor->SetPosition( vStartPos );
		vDir.x = sin( EtToRadian( fRotate ) );
		vDir.y = cos( EtToRadian( fRotate ) );
		hActor->Look( vDir );

		if( hActor->GetObjectHandle() )
			hActor->GetObjectHandle()->Update( *hActor->GetMatEx() );

		hActor->SetActionQueue( "Stand", 0, 0.f );
		hActor->ResetActor();

		if( ((CDnPlayerActor*)hActor.GetPointer())->IsLocalActor() ) 
			((CDnPlayerCamera*)m_hPlayerCamera.GetPointer())->ResetCamera();
	}
}

void CDnGameTask::InitializeInterface()
{
	switch( GetGameTaskType() ) {
		case GameTaskType::Normal:
			GetInterface().Initialize( CDnInterface::Game, this );
			break;
		case GameTaskType::DarkLair:
			GetInterface().Initialize( CDnInterface::DLGame, this );
			break;
		case GameTaskType::PvP:
			GetInterface().Initialize( CDnInterface::PVPGame, this );
			break;
		case GameTaskType::Farm:
			GetInterface().Initialize( CDnInterface::Farm, this );
			break;
	}

	CDnItemTask::GetInstance().GetCharInventory().RefreshInventory();
	CDnItemTask::GetInstance().RefreshEquip();
}

#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
void CDnGameTask::PreGenerationMonster( int nRandomSeed )
{
#ifdef PRE_MOD_PRELOAD_MONSTER
	// Considered two main approaches for preloaded monster optimization.
	// First is removing preload completely like Dark Lair and deleting all unused act files whenever a boss monster dies,
	// The second way is to preload as before (to prevent hitches when monsters spawn), and when the boss monster dies, delete only the resources related to that boss monster's act file.
	// Each has its pros and cons,
	// The first method is easy to implement, but since it does no preloading, initial loading may stutter slightly,
	// The second method avoids hitches on initial load, but leaves other preloaded monster resources intact
	// Since only the .act files of boss monsters defeated by players must be selectively deleted, the check routine becomes more complex.
	//
	// However, above all, I thought the main cause of out-of-memory during loading is monster preloading itself.
	// Assuming you go to Sard Field and fight the Cocktae boss monster for the first time, the Manti boss .act file stays in memory, gradually running low on memory.
	// Memory is already tight with player info, so trying to keep monsters that won't fight right away
	// If the client crashes from out-of-memory, isn't that much costlier than trying to avoid stutter?
	// So decided to go with the first method.
	//
	// Thought about extracting the check for non-preloaded maps into a type or separate table,
	// Leaving out-of-memory handling to data feels risky, so handle it directly in code (unlikely to change once set).
	// Tried classifying by type, but there is no way to distinguish Dragon Nest type (and classifying by entry count might break if it changes later..)
	// Sea Dragon Nest and Green Dragon Nest do not run out of memory and there is no reason to remove preload, so decided to check via hardcoding.
	if( IsNoPreloadMonsterMap() )
		return;
#endif

	GenerationMonster( nRandomSeed );
	GenerationSetMonster( nRandomSeed );
}
#endif

#ifdef PRE_MOD_PRELOAD_MONSTER
bool CDnGameTask::IsNoPreloadMonsterMap()
{
	if (m_nMapTableID == 14201) //desert dragon+black
		return true;
	return false;
}
#endif

bool CDnGameTask::GenerationMonster( int nRandomSeed )
{
	_srand( nRandomSeed );
	char szTemp[256];

	std::vector<GenerationMonsterStruct> VecGenerationResult;
	std::vector<MonsterTableStruct> VecMonsterList;

	// Search monster name
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );

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

	// Search monster spawn position
	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	std::vector<AreaStruct> VecCommonArea;
	char szAreaName[256];
	char szMonsterName[256];
	char szNpcName[256];

	for( DWORD i=0; i<dwCount; i++ ) {
		pSector = m_pWorld->GetGrid()->GetActiveSector(i);
		pControl = pSector->GetControlFromUniqueID( ETE_UnitArea );
		if( !pControl ) continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			pArea = pControl->GetAreaFromIndex(j);
			vPos.x = pArea->GetMin()->x + ( _rand()%(int)( pArea->GetMax()->x - pArea->GetMin()->x ) );
			vPos.z = pArea->GetMin()->z + ( _rand()%(int)( pArea->GetMax()->z - pArea->GetMin()->z ) );
			vPos.y = m_pWorld->GetHeight( vPos );

			sprintf_s( szAreaName, pArea->GetName() );
			_strlwr_s( szAreaName );

			memset( szMonsterName, 0, sizeof(szMonsterName) );
			memset( szNpcName, 0, sizeof(szNpcName) );
			
			sscanf_s( szAreaName, "%s", szMonsterName, 256 );

			if( strcmp( szMonsterName, "npc" ) == NULL ) continue;

			if( strcmp( szMonsterName, "monster" ) == NULL ) {
				int nMonsterTableID = -1;
				sscanf_s( szAreaName, "%s %d", szMonsterName, 256, &nMonsterTableID );
				if( nMonsterTableID == -1 ) continue;

				GenerationMonsterStruct Struct;
				Struct.nActorTableID = -1;
				Struct.pArea = pArea;

				UnitAreaStruct *pStruct = (UnitAreaStruct*)pArea->GetData();
				int nCount = 1;
				if( pStruct->vCount ) {
					nCount = (int)pStruct->vCount->x + _rand()%( ( (int)pStruct->vCount->y + 1 ) - (int)pStruct->vCount->x );
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
				Struct.pArea = pArea;
				Struct.nCompoundIndex = nCompoundIndex;
				
				UnitAreaStruct *pStruct = (UnitAreaStruct*)pArea->GetData();
				if( pStruct->vCount == NULL ) {
					Struct.nMin = Struct.nMax = 0;
				}
				else {
					Struct.nMin = (int)pStruct->vCount->x;
					Struct.nMax = (int)pStruct->vCount->y;
				}

				Struct.nMonsterSpawn = pStruct->MonsterSpawn;
				VecCommonArea.push_back( Struct );
			}
			else {
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

						UnitAreaStruct *pStruct = (UnitAreaStruct*)pArea->GetData();
						int nCount = 1;
						if( pStruct->vCount ) {
							nCount = (int)pStruct->vCount->x + _rand()%( ( (int)pStruct->vCount->y + 1 ) - (int)pStruct->vCount->x );
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
		std::vector<MonsterCompoundStruct> VecCompoundList;
		std::vector<int> nVecOffset;
		MonsterCompoundStruct Struct;
		char szLabel[32];
		int nRandomMax = 0;
		bool bFavoritism = true;
	
		
		for( int j=0; j<15; j++ ) {
			sprintf_s( szLabel, "_MonsterTableID%d_%d", m_nStageConstructionLevel + 1, j + 1 );
			Struct.nMonsterTableID = pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetInteger();
			if( Struct.nMonsterTableID < 1 ) continue;

			_fpreset();
			sprintf_s( szLabel, "_Ratio%d_%d", m_nStageConstructionLevel + 1, j + 1 );
			Struct.nPossessionProb = (int)( ( pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetFloat() + 0.0001f ) * 100 );
			if( Struct.nPossessionProb <= 0 ) continue;

			sprintf_s( szLabel, "_Value%d_%d", m_nStageConstructionLevel + 1, j + 1 );
			Struct.nValue = (int)( pSox->GetFieldFromLablePtr( nTableID, szLabel )->GetInteger() );
			if( Struct.nValue < 1 ) Struct.nValue = 1;

			nRandomMax += Struct.nPossessionProb;
			if( Struct.nPossessionProb != 100 ) bFavoritism = false;

			nVecOffset.push_back( nRandomMax );
			VecCompoundList.push_back( Struct );
		}
		if( VecCompoundList.empty() ) continue;

		int nMaxValue = VecCommonArea[i].nMin + _rand() % ( ( VecCommonArea[i].nMax + 1 ) - VecCommonArea[i].nMin );
		int nCurValue = 0;

		GenerationMonsterStruct GenStruct;

		GenStruct.pArea = VecCommonArea[i].pArea;
		GenStruct.nMonsterSpawn = VecCommonArea[i].nMonsterSpawn;

		if( bFavoritism == true ) {
			MonsterCompoundStruct Struct;
			int nIndex = _rand()%(int)VecCompoundList.size();
			Struct = VecCompoundList[nIndex];
			VecCompoundList.clear();
			nVecOffset.clear();

			VecCompoundList.push_back( Struct );
			nVecOffset.push_back( 100 );
			nRandomMax = 100;
		}

		while(1) {
			int nResult = _rand()%nRandomMax;
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
	SAABox Box;

	for( DWORD i=0; i<VecGenerationResult.size(); i++ ) 
	{
		int nMonsterID = VecGenerationResult[i].nMonsterTableID;
		if( VecGenerationResult[i].nActorTableID == -1 ) {
			VecGenerationResult[i].nActorTableID = pSox->GetFieldFromLablePtr( nMonsterID, "_ActorTableID" )->GetInteger();
		}
		if( VecGenerationResult[i].nActorTableID < 1 ) continue;

		GenerationMonsterStruct& genMonStructInfo = VecGenerationResult[i];
		SOBB *pOBB = genMonStructInfo.pArea->GetOBB();
		if (genMonStructInfo.nMonsterSpawn == 0)
			vPos = GetGenerationRandomPosition( pOBB );
		else
			vPos = pOBB->Center;

		hMonster = CreateActor( VecGenerationResult[i].nActorTableID ); // rhrnak
		if( !hMonster ) continue;
		hMonster->SetPosition( vPos );

		InitializeMonster( hMonster, CDnActor::s_dwUniqueCount++, nMonsterID, *pOBB, VecGenerationResult[i].pArea );

		if( hMonster->IsExistAction( "Stand" ) )
			hMonster->SetActionQueue( "Stand" );
		m_hVecMonsterList.push_back( hMonster );

		int nSkillTableId = pSox->GetFieldFromLablePtr( nMonsterID, "_SkillTable")->GetInteger();
		DNTableFileFormat* pSkillSox = GetDNTable( CDnTableDB::TMONSTERSKILL );

		if ( !pSkillSox )
			continue;

		for( int j=0; j<30; j++ )
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

			int nSkillLevel = pField->GetInteger();

			if ( nSkillId < 0 || nSkillLevel < 0 )
				continue;

			hMonster->AddSkill(nSkillId, nSkillLevel);
		}




		// Set immune state effect index last if present
		const char* pImmunedStateEffects = pSox->GetFieldFromLablePtr( nMonsterID, "_ImmunedStateBlow" )->GetString();
		const char* pImmunePercent = pSox->GetFieldFromLablePtr( nMonsterID, "_ImmunePercent" )->GetString();
		if( pImmunedStateEffects && 0 < strlen(pImmunedStateEffects) &&
			pImmunePercent && 0 < strlen(pImmunePercent) )
		{
			int nISFoundPos = -1;
			int nIPFoundPos = -1;
			string strImmunedStateEffects( pImmunedStateEffects );
			string strImmunePercent( pImmunePercent );
			if( strImmunedStateEffects.at(strImmunedStateEffects.length()-1) != ';' )
				strImmunedStateEffects.push_back( ';' );
			if( strImmunePercent.at(strImmunePercent.length()-1) != ';' )
				strImmunePercent.push_back( ';' );
			while( true )
			{
				int nISStartPos = nISFoundPos+1;
				int nIPStartPos = nIPFoundPos+1;
				nISFoundPos = (int)strImmunedStateEffects.find_first_of( ';', nISStartPos );
				nIPFoundPos = (int)strImmunePercent.find_first_of( ';', nIPStartPos );
				if( nISFoundPos != (int)string::npos &&
					nIPFoundPos != (int)string::npos )
				{
					// Set the immunity state effect (77) to the state effect index specified in the table.
					// Client and game server each configure from their own tables, so do not send a packet.
					string strArg( strImmunedStateEffects.substr(nISStartPos, nISFoundPos-nISStartPos) );
					strArg.append(";");
					strArg.append( strImmunePercent.substr(nIPStartPos, nIPFoundPos-nIPStartPos) );
					hMonster->CDnActor::AddStateBlow( NULL, STATE_BLOW::BLOW_077, -1, strArg.c_str() );
				}
				else
				{
					_ASSERT( nISFoundPos == (int)string::npos &&
							 nIPFoundPos == (int)string::npos && "Monster state effect immunity column and probability count mismatch." );
					if( !(nISFoundPos == (int)string::npos && nIPFoundPos == (int)string::npos) )
						OutputDebug( "[Monster Table Error] id: %d, monster state effect immunity column and probability count mismatch\n", nMonsterID );
					break;
				}
			}
		}
	}
	return true;
}

int CDnGameTask::CalcSetMonsterReference( int nSetID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERSET );

	std::vector<int> nVecItemID;
	std::vector<MonsterSetProbStruct> VecSetProb;
	pSox->GetItemIDListFromField( "_SetNo", nSetID, nVecItemID );
	int nTotalProb = 0;

	for( DWORD i=0; i<nVecItemID.size(); i++ ) {
		if( pSox->GetFieldFromLablePtr( nVecItemID[i], "_Difficulty" )->GetInteger() == m_nStageConstructionLevel ) {
			MonsterSetProbStruct ProbStruct;
			ProbStruct.nItemID = nVecItemID[i];
			int nProb = (int)( ( pSox->GetFieldFromLablePtr( nVecItemID[i], "_SetWeight" )->GetFloat() + 0.0001f ) * 100.f );
			ProbStruct.nProb = nTotalProb + nProb;
			nTotalProb += nProb;
			VecSetProb.push_back( ProbStruct );
		}
	}
	if( nTotalProb == 0 ) return -1;
	int nRandomOffset = _rand()%nTotalProb;
	int nResultSetItemID = -1;
	for( DWORD i=0; i<VecSetProb.size(); i++ ) {
		if( nRandomOffset < VecSetProb[i].nProb ) {
			return VecSetProb[i].nItemID;
		}
	}
	return -1;
}

void CDnGameTask::CalcSetMonsterGroupList( int nSetTableID, std::map<int, std::vector<MonsterGroupStruct>> &nMapResult )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERSET );

	char szLabel[32];
	std::map<int, std::vector<MonsterGroupStruct>>::iterator iterGroup;

	for( int i=0; i<20; i++ ) {
		sprintf_s( szLabel, "_PosID%d", i + 1 );
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
			std::vector<MonsterGroupStruct> VecGroupList;
			VecGroupList.push_back( GroupStruct );

			nMapResult.insert( make_pair( nPosID, VecGroupList ) );
		}
		else {
			iterGroup->second.push_back( GroupStruct );
		}
	}
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
	_srand( nRandomSeed );

	DWORD dwCount = m_pWorld->GetGrid()->GetActiveSectorCount();
	CEtWorldEventControl *pControl = NULL;
	CEtWorldSector *pSector = NULL;
	CEtWorldEventArea *pArea = NULL;
	EtVector3 vPos;
	DnActorHandle hMonster;

	std::vector<MonsterSetStruct> VecSetMonsterList;

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
	if( VecSetMonsterList.empty() ) return false;

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

	DNTableFileFormat* pMonsterSetSox = GetDNTable( CDnTableDB::TMONSTERSET );
	DNTableFileFormat* pMonsterSox = GetDNTable( CDnTableDB::TMONSTER );

	// Select by referencing actual group ID from organized list
	for( iterSet = nMapUseSetList.begin(); iterSet != nMapUseSetList.end(); iterSet++ ) {
		int nSetID = iterSet->first;
		int nResultSetItemID = CalcSetMonsterReference( nSetID );
		if( nResultSetItemID == -1 ) continue;

		// Organize monster group list using selected set item ID
		std::map<int, std::vector<MonsterGroupStruct>> nMapGroupList;
		std::map<int, std::vector<MonsterGroupStruct>>::iterator iterGroup;

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
				int nOffset = _rand() % (int)nVecResultMonsterList.size();
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

				hMonster = CreateActor( nActorTableID );
				hMonster->SetPosition( vPos );

				InitializeMonster( hMonster, CDnActor::s_dwUniqueCount++, nMonsterID, *pOBB, pArea );

				m_hVecMonsterList.push_back( hMonster );		

				nAreaCount++;
			}
		}
	}

	//	pMonsterSetSox->GetItemIDListFromField( 

	return true;
}

int CDnGameTask::CalcMonsterIDFromMonsterGroupID( int nItemID )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERGROUP );

	int nRandomOffset = _rand()%1000000000;
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
	vPos += pOBB->Axis[0] * (float)( ( _rand()%nX ) * 50.f );
	vPos += pOBB->Axis[2] * (float)( ( _rand()%nZ ) * 50.f );
	vPos.y = m_pWorld->GetHeight( vPos );
	if( pOBB->Extent[1] != 100000.f )
		vPos += pOBB->Axis[1] * pOBB->Extent[1];

	return vPos;
}

bool CDnGameTask::GenerationNpc( int nRandomSeed )
{
	_srand( nRandomSeed );

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
			vPos.x = pArea->GetMin()->x + ( _rand()%(int)( pArea->GetMax()->x - pArea->GetMin()->x ) );
			vPos.z = pArea->GetMin()->z + ( _rand()%(int)( pArea->GetMax()->z - pArea->GetMin()->z ) );
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
				pNpcData = g_DataManager.GetNpcData(nNpcIndex);
				if (!pNpcData) continue;

				EtVector3 vPos;
				vPos.x = pArea->GetMin()->x + ((int)(pArea->GetMax()->x - pArea->GetMin()->x) / 2);
				vPos.z = pArea->GetMin()->z + ((int)(pArea->GetMax()->z - pArea->GetMin()->z) / 2);
				vPos.y = m_pWorld->GetHeight( vPos ) + pArea->GetMin()->y;

				DnActorHandle hActor = CreateNpcActor( CDnActor::s_dwUniqueCount++, nNpcIndex, vPos, pArea->GetRotate());
				if( hActor ) {
					m_hVecNpcList.push_back( hActor );
					InsertBirthAreaList( hActor, pArea->GetCreateUniqueID() );
				}
			}
		}
	}

	// For Npc Prop, Prop is loaded before Event Area, so settings like SAfeZone do not apply.
	// So call RefreshSafeZone once here so it takes effect.
	for( DWORD i=0; i<CDnActor::GetProcessCount(); i++ ) {
		CDnActor *pActor = CDnActor::s_pVecProcessList[i];
		if( !pActor->IsNpcActor() ) continue;
		if( ((CDnNPCActor*)pActor)->IsNpcProp() )
			((CDnNPCActor*)pActor)->RefreshSafeZone();
	}
	return true;
}

void CDnGameTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
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

#ifndef _FINAL_BUILD
	bool b = IsPushKey(DIK_LCONTROL);
	if ( g_bNaviTest  )
	{
		EtVector3 vPlayerPos;
		vPlayerPos = *(CDnLocalPlayerActor::s_hLocalActor->GetPosition());
		
		POINT pt;
		GetCursorPos(&pt);
		EtVector2 Pos;
		Pos.x = (float)pt.x;
		Pos.y = (float)pt.y;
		EtVector3 vOutPos;


		CDnWorld::GetInstance().PickFromScreenCoord(Pos.x, Pos.y, vOutPos);

		g_vStart = vPlayerPos;
		g_vEnd = vOutPos;


	}
#endif
}

bool CDnGameTask::RenderScreen( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bInitComplete ) {
		CDnCommonTask *pTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
		if( pTask && pTask->IsExistInitTimeProcess() ) return true;
		return false;
	}

	return true;
}

void CDnGameTask::OnRecvRoomGenerationMonsterMsg( SCGenerationMonster *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvRoomGenerationMonsterMsg( pPacket );

	OutputDebug( "Generation Monster Packet : %d - %d\n", pPacket->nSessionID, pPacket->nSeed );

	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().InsertLoadObject( 
			OnLoadRecvGenerationMonsterCallback, 
			OnLoadRecvGenerationMonsterUniqueID,
			OnCheckLoadingPacket, 
			NULL,
			this, (void*)pPacket, sizeof(SCGenerationMonster), m_LocalTime, CDnLoadingTask::CreateActor );
	}
}

bool __stdcall CDnGameTask::OnLoadRecvGenerationMonsterCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	FUNC_LOG();
	SCGenerationMonster *pPacket = (SCGenerationMonster *)pParam;
	CDnGameTask *pTask = (CDnGameTask *)pThis;

	std::vector<CEtWorldEventArea *> pVecArea;
	CDnWorld::GetInstance().FindEventAreaFromCreateUniqueID( pPacket->nEventAreaUniqueID, &pVecArea );
	CEtWorldEventArea *pBirthArea = ( pVecArea.empty() ) ? NULL : pVecArea[0];
	SOBB Box;
	if( pBirthArea == NULL ) {
		Box.Center = pPacket->vPos;
		Box.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
		Box.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
		Box.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
		Box.Extent[0] = 100.f;
		Box.Extent[1] = 0.f;
		Box.Extent[2] = 100.f;
	}
	else Box = *pBirthArea->GetOBB();

	EtQuat* qRotation = NULL;
	if( pPacket->bUseRotationQuat )
		qRotation = &pPacket->qRotation;

	OutputDebug( "Generation Monster Callback : %d - %d\n", pPacket->nSessionID, pPacket->nSeed );
	pTask->GenerationMonsterFromMonsterID(pPacket->nSeed, pPacket->nMonsterID, pPacket->nSessionID, pPacket->vPos,
										   pPacket->vVel, pPacket->vResistance, &Box, pBirthArea, LocalTime, pPacket->nTeam, pPacket->bRandomFrameSummon, qRotation );
	CDnActor::s_dwUniqueCount = pPacket->nSessionID + 1;
	OutputDebug("Generation Monster Callback END: %d - %d\n", pPacket->nSessionID, pPacket->nSeed);

	return true;
}

int __stdcall CDnGameTask::OnLoadRecvGenerationMonsterUniqueID( void *pParam, int nSize )
{
	return ((SCGenerationMonster*)pParam)->nSessionID;
}

/*
void CDnGameTask::OnRecvRoomDestoryMonsterMsg( SCDestroyMonster* pPacket ) 
{
	DestroyMonster(pPacket->nUniqueID);
}

void CDnGameTask::OnRecvRoomDestoryAllMonster( SCDestroyAllMonster *pPacket )
{
	DnActorHandle hActor;
	CDnMonsterActor *pMonster;
	for( DWORD i=0; i<CDnActor::s_pVecProcessList.size(); i++ ) {
		hActor = CDnActor::s_pVecProcessList[i]->GetMySmartPtr();
		if( !hActor ) continue;
		pMonster = dynamic_cast<CDnMonsterActor*>(hActor.GetPointer());
		if( !pMonster ) continue;;
		if( pMonster->IsDie() ) continue;

		DestroyMonster( pM
	}
}
*/

void CDnGameTask::OnRecvRoomGateInfoMsg( SCGateInfoMessage *pPacket )
{
	CGameClientSession::OnRecvRoomGateInfoMsg( pPacket );

	for( int i=0; i<pPacket->cCount; i++ ) {
		CDnWorld::GetInstance().SetPermitGate( pPacket->cGateIndex[i], (CDnWorld::PermitGateEnum)pPacket->cPermitFlag[i] );
	}
}

void CDnGameTask::OnRecvRoomDungeonClearMsg( SCDungeonClear *pPacket )
{
	SAFE_DELETE( m_pStageClearImp );
	m_pStageClearImp = new CDnDungeonClearImp( this );
	m_pStageClearImp->OnCalcClearProcess( (void*)pPacket );
}

void CDnGameTask::OnRecvRoomDungeonFailedMsg( SCDungeonFailed *pPacket )
{
#ifdef PRE_ADD_RETURN_VILLAGE_SHOWNAME
	GetInterface().SetReturnVillage( pPacket->nLastVillageMapIndex );
#endif 
	GetInterface().OpenMissionFailDialog( pPacket->bTimeOut, pPacket->bCanWarpWorldMap );
}

void CDnGameTask::OnRecvRoomDungeonClearSelectRewardItem( char *pPacket )
{
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	if (m_pStageClearImp == NULL)
	{
		OnAbnormalBreakIntoDungeonClearProcess(true);
		return;
	}
#endif
	GetInterface().OpenStageClearDialog();
}

#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
void CDnGameTask::OnAbnormalBreakIntoDungeonClearProcess(bool bShow)
{
	CDnMessageBox* pMsgBox = GetInterface().GetMessageBox();
	if (pMsgBox)
	{
	#ifdef PRE_FIX_GM_BREAKINTO_DGNCLEAR
		if (bShow)
		{
			bool bWarningModalOnShow = true;
			if (pMsgBox->IsShow())
				bWarningModalOnShow = false;

			if (CDnActor::s_hLocalActor && static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer())->IsDeveloperAccountLevel())
				bWarningModalOnShow = false;
				
			if (bWarningModalOnShow)
			{
				GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3453), MB_OK, MESSAGEBOX_PARTY_RECONN_ON_DGNCLEAR, NULL, true, false, true); // UISTRING : A party member is clearing a dungeon. Please wait a moment.
				return;
			}
		}
	#else
		if (bShow && pMsgBox->IsShow() == false)
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3453), MB_OK, MESSAGEBOX_PARTY_RECONN_ON_DGNCLEAR, NULL, true, false, true); // UISTRING : A party member is clearing a dungeon. Please wait a moment.
			return;
		}
	#endif

		if (bShow == false)
		{
			GetInterface().CloseMessageBox();
			GetInterface().OpenBaseDialog();
		}
	}
}
#endif

void CDnGameTask::OnRecvRoomSelectRewardItem( SCSelectRewardItem *pPacket )
{
	CDnStageClearDlg *pDlg = GetInterface().GetStageClearDialog();
	if( pDlg ) pDlg->SelectBonusBox( pPacket->nSessionID, pPacket->cItemIndex );
}

void CDnGameTask::OnRecvRoomDungeonClearRewardItem( SCDungeonClearRewardItem *pPacket )
{
	if( m_pStageClearImp ) 
		m_pStageClearImp->OnCalcClearRewardItemProcess( (void*)pPacket );
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	else
	{
		OnAbnormalBreakIntoDungeonClearProcess(true);
		return;
	}
#endif

	CDnStageClearDlg *pDlg = GetInterface().GetStageClearDialog();
	if( pDlg ) pDlg->OpenBonusBox();
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnGameTask::OnRecvRoomDungeonClearRewardBoxType(SCDungeonClearBoxType* pData)
{
	if( m_pStageClearImp ) 
		m_pStageClearImp->OnCalcClearRewardBoxProcess( (void*)pData );
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	else
	{
		OnAbnormalBreakIntoDungeonClearProcess(true);
		return;
	}
#endif

	CDnStageClearDlg *pDlg = GetInterface().GetStageClearDialog();
	if( pDlg )
	{
		pDlg->ShowBonusBox();

#if defined( PRE_SKIP_REWARDBOX )
		pDlg->ResetCount();
#endif	// #if defined( PRE_SKIP_REWARDBOX )
	}
}
#endif

void CDnGameTask::OnRecvRoomDungeonClearRewardItemResult( SCDungeonClearRewardItemResult *pPacket )
{
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	if (m_pStageClearImp == NULL)
	{
		OnAbnormalBreakIntoDungeonClearProcess(true);
		return;
	}
#endif

	CDnStageClearDlg *pDlg = GetInterface().GetStageClearDialog();
	if( pDlg ) {
		for( int i=0; i<pPacket->cCount; i++ ) {
			pDlg->SelectBonusBox( pPacket->Info[i].nSessionID, pPacket->Info[i].cIndex );
		}
		pDlg->OpenBonusItem();
	}
}

void CDnGameTask::OnRecvRoomDungeonClearWarpFailed(SCDungeonClearWarpOutFail* pPacket)
{
	//GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3578)); // UISTRING : Cannot leave the dungeon while distributing bound items
	if (pPacket->nRet != ERROR_NONE)
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnGameTask::OnRecvRoomDungeonClearEnableLeaderWarp(SCDungeonClearLeaderWarp* pPacket)
{
	m_bEnableDungeonClearLeaderWarp = pPacket->bEnable;
	/*
	if( CDnActor::s_hLocalActor ) {
		CDnPlayerActor *pActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		OnGhost( CDnActor::s_hLocalActor );x
	}
	*/
}

void CDnGameTask::OnRecvRoomWarpDungeonClear( SCWarpDungeonClear *pPacket )
{
	bool bLeader = (CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::LEADER || CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::SINGLE);
	CDnPartyTask::GetInstance().SetPartyState(CDnPartyTask::STAGE_CLEAR_WARP_STANDBY);

#ifdef PRE_ADD_RETURN_VILLAGE_SHOWNAME
	GetInterface().SetReturnVillage( pPacket->nLastVillageMapIndex );
#endif 
	GetInterface().CloseStageClearDialog(bLeader);

#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	if (m_pStageClearImp == NULL)
		OnAbnormalBreakIntoDungeonClearProcess(false);
#endif
	/*
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		DnActorHandle hActor = CDnPartyTask::GetInstance().GetPartyData(i)->hActor;
		if( !hActor ) continue;

		hActor->SetHP( hActor->GetMaxHP() );
		hActor->SetSP( hActor->GetMaxSP() );
	}
	*/
}

void CDnGameTask::OnRecvRoomChangeGateState( SCChangeGateState *pPacket )
{
	CDnWorld::GetInstance().SetPermitGate( pPacket->cGateInde, (CDnWorld::PermitGateEnum)pPacket->nPermitFlag );
}

//void CDnGameTask::OnRecvRoomGenerationPropMsg( SCGenerationProp *pPacket )
//{
//	if( CDnLoadingTask::IsActive() ) {
//		CDnLoadingTask::GetInstance().InsertLoadObject( 
//			OnLoadRecvGenerationPropCallback, 
//			OnLoadRecvGenerationPropUniqueID,
//			OnCheckLoadingPacket,
//			NULL,
//			this, (void*)pPacket, sizeof(SCGenerationProp), m_LocalTime );
//	}
//}

void CDnGameTask::OnRecvRoomGenerationPropMsg( char* pPacket, int nSize )
{
	if( CDnLoadingTask::IsActive() ) {
		CDnLoadingTask::GetInstance().InsertLoadObject( 
			OnLoadRecvGenerationPropCallback, 
			OnLoadRecvGenerationPropUniqueID,
			OnCheckLoadingPacket,
			NULL,
			this, (void*)pPacket, nSize, m_LocalTime, CDnLoadingTask::CreateNpc );
	}
}

bool __stdcall CDnGameTask::OnLoadRecvGenerationPropCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	CDnGameTask *pTask = (CDnGameTask *)pThis;

	DWORD dwUniqueID = 0;
	int iPropTableID = 0;
	int iClassID = 0;
	int iSeed = 0;
	EtVector3 vPosition, vRotation, vScale;
	int iLifeTime = 0;
	
	CPacketCompressStream PacketStream( pParam, nSize );
	PacketStream.Read( &dwUniqueID, sizeof(DWORD) );
	PacketStream.Read( &iPropTableID, sizeof(int) );
	PacketStream.Read( &iClassID, sizeof(int) );
	PacketStream.Read( &iSeed, sizeof(int) );
	PacketStream.Read( &vPosition, sizeof(EtVector3) );
	PacketStream.Read( &vRotation, sizeof(EtVector3) );
	PacketStream.Read( &vScale, sizeof(EtVector3) );
	PacketStream.Read( &iLifeTime, sizeof(int) );

	DNTableFileFormat* pPropSox = GetDNTable( CDnTableDB::TPROP );
	if( pPropSox->IsExistItem( iPropTableID ) == false ) 
		return false;
	string szPropName = pPropSox->GetFieldFromLablePtr( iPropTableID, "_Name" )->GetString();

	void* pAdditionalPropInfo = NULL;
	switch( iClassID )
	{
		case PTE_Chest:
			{
				ChestStruct* pChestInfo = new ChestStruct;
				PacketStream.Read( &pChestInfo->nNeedKeyID, sizeof(int) );
				PacketStream.Read( &pChestInfo->nItemDropGroupTableID, sizeof(int) );
				pAdditionalPropInfo = (void*)pChestInfo; 
			}
			break;

		case PTE_Broken:
			{
				BrokenStruct* pBrokenInfo = new BrokenStruct;
				PacketStream.Read( &pBrokenInfo->nDurability, sizeof(int) );
				PacketStream.Read( &pBrokenInfo->nItemDropGroupTableID, sizeof(int) );
				pAdditionalPropInfo = (void*)pBrokenInfo;
			}
			break;

		case PTE_BrokenDamage:
			{
				BrokenDamageStruct* pBrokenDamageInfo = new BrokenDamageStruct;
				PacketStream.Read( &pBrokenDamageInfo->nMonsterTableID, sizeof(int) );
				PacketStream.Read( &pBrokenDamageInfo->nItemDropGroupTableID, sizeof(int) );
				PacketStream.Read( &pBrokenDamageInfo->nDurability, sizeof(int) );
				PacketStream.Read( &pBrokenDamageInfo->nSkillTableID, sizeof(int) );
				PacketStream.Read( &pBrokenDamageInfo->nSkillLevel, sizeof(int) );
				PacketStream.Read( &pBrokenDamageInfo->SkillApplyType, sizeof(int) );
				pAdditionalPropInfo = (void*)pBrokenDamageInfo;
			}
			break;

		case PTE_HitMoveDamageBroken:
			{
				HitMoveDamageBrokenStruct* pHitMoveDamageBrokenInfo = new HitMoveDamageBrokenStruct;
				PacketStream.Read( &pHitMoveDamageBrokenInfo->nMonsterTableID, sizeof(int) );
				PacketStream.Read( &pHitMoveDamageBrokenInfo->nItemDropGroupTableID, sizeof(int) );
				PacketStream.Read( &pHitMoveDamageBrokenInfo->nDurability, sizeof(int) );
				PacketStream.Read( &pHitMoveDamageBrokenInfo->MovingAxis, sizeof(int) );
				PacketStream.Read( &pHitMoveDamageBrokenInfo->fDefaultAxisMovingSpeed, sizeof(float) );
				PacketStream.Read( &pHitMoveDamageBrokenInfo->fMaxAxisMoveDistance, sizeof(float) );
				pAdditionalPropInfo = (void*)pHitMoveDamageBrokenInfo;
			}
			break;

		case PTE_Buff:
			{
				BuffStruct* pBuffStructInfo = new BuffStruct;
				PacketStream.Read( &pBuffStructInfo->nMonsterTableID, sizeof(int) );
				PacketStream.Read( &pBuffStructInfo->nSkillLevel, sizeof(int) );
				PacketStream.Read( &pBuffStructInfo->fCheckRange, sizeof(float) );
				PacketStream.Read( &pBuffStructInfo->nTeam, sizeof(int) );
				pAdditionalPropInfo = (void*)pBuffStructInfo;
			}
			break;

		case PTE_BuffBroken:
			{
				BuffBrokenStruct* pBuffBrokenStruct = new BuffBrokenStruct;
				PacketStream.Read( &pBuffBrokenStruct->nMonsterTableID, sizeof(int) );
				PacketStream.Read( &pBuffBrokenStruct->nDurability, sizeof(int) );
				PacketStream.Read( &pBuffBrokenStruct->nItemDropGroupTableID, sizeof(int) );
				PacketStream.Read( &pBuffBrokenStruct->nSkillTableID, sizeof(int) );
				PacketStream.Read( &pBuffBrokenStruct->nSkillLevel, sizeof(int) );
				PacketStream.Read( &pBuffBrokenStruct->fCheckRange, sizeof(float) );
				PacketStream.Read( &pBuffBrokenStruct->nTeam, sizeof(int) );
				pAdditionalPropInfo = (void*)pBuffBrokenStruct;
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

	_srand( iSeed );
	CDnWorldProp *pProp = (CDnWorldProp *)pTask->m_pWorld->AddProp( szPropName.c_str(), vPosition, vRotation, vScale, pAdditionalPropInfo );
	if( !pProp )
		return false;

	pProp->SetCreateUniqueID( (int)dwUniqueID );

	if( -1 < iLifeTime )
		pProp->SetLifeTime( (LOCAL_TIME)iLifeTime );

	// If it is an action prop, execute the Summon_On action
	CDnWorldActProp* pBuffProp = dynamic_cast<CDnWorldActProp*>(pProp);
	if( pBuffProp && pBuffProp->IsExistAction( "Summon_On" ) )
		pBuffProp->SetActionQueue( "Summon_On" );

	//SCGenerationProp *pPacket = (SCGenerationProp *)pParam;

	//DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSUMMONPROP );
	//DNTableFileFormat* pPropSox = GetDNTable( CDnTableDB::TPROP );
	//if( !pSox ) return false;

	//if( pSox->IsExistItem( pPacket->nSummonPropTableID ) == false ) return false;
	//int nPropTableID = pSox->GetFieldFromLablePtr( pPacket->nSummonPropTableID, "_PropTableID" )->GetInteger();
	//if( !pPropSox->IsExistItem( nPropTableID ) ) return false;

	//int nClassID = pPropSox->GetFieldFromLablePtr( nPropTableID, "_ClassID" )->GetInteger();
	//std::string szPropName = pPropSox->GetFieldFromLablePtr( nPropTableID, "_Name" )->GetString();

	//void *pAdditionalPropInfo = NULL;
	//switch( nClassID ) {
	//	case PTE_Broken:
	//		{
	//			BrokenStruct *pStruct = new BrokenStruct;
	//			pStruct->nDurability = pSox->GetFieldFromLablePtr( pPacket->nSummonPropTableID, "_Durability" )->GetInteger();
	//			pStruct->nItemDropGroupTableID = pSox->GetFieldFromLablePtr( pPacket->nSummonPropTableID, "_ItemDropID" )->GetInteger();
	//			pAdditionalPropInfo = pStruct;
	//		}
	//		break;
	//	case PTE_Trap:
	//		{
	//			TrapStruct *pStruct = new TrapStruct;
	//			pStruct->nMonsterTableID = pSox->GetFieldFromLablePtr( pPacket->nSummonPropTableID, "_MonsterID" )->GetInteger();
	//			pAdditionalPropInfo = pStruct;
	//		}
	//		break;
	//}

	//_srand( pPacket->nSeed );
	//CDnWorldProp *pProp = (CDnWorldProp *)pTask->m_pWorld->AddProp( szPropName.c_str(), pPacket->vPos, pPacket->vRotate, pPacket->vScale, pAdditionalPropInfo );
	//if( !pProp ) return false;
	
	//pProp->SetCreateUniqueID( pPacket->nPropUniqueID );
	return true;
}

int __stdcall CDnGameTask::OnLoadRecvGenerationPropUniqueID( void *pParam, int nSize )
{
	CPacketCompressStream PacketStream( pParam, nSize );
	DWORD dwUniqueID = 0;
	PacketStream.Read( &dwUniqueID, sizeof(DWORD) );

	return dwUniqueID;
	//return ((SCGenerationProp*)pParam)->nPropUniqueID;
}


void CDnGameTask::OnRecvRoomChangeGameSpeed( SCChangeGameSpeed *pPacket )
{
	SetLocalTimeSpeed( pPacket->fSpeed, pPacket->dwDelay );
}

void CDnGameTask::OnRecvRoomGateEjectInfo( SCGateEjectInfo *pPacket )
{
	CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( pPacket->cGateIndex );
	if( !pStruct ) return;

	pStruct->cVecLessLevelActorIndex.clear();
	pStruct->cVecNotEnoughItemActorIndex.clear();
	pStruct->cVecExceedTryActorIndex.clear();

	int nOffset = 0;
	for( int i=0; i<pPacket->cLessLevelCount; i++ )
		pStruct->cVecLessLevelActorIndex.push_back( pPacket->cActorIndex[i+nOffset] );
	nOffset += pPacket->cLessLevelCount;

	for( int i=0; i<pPacket->cNotEnoughItemCount; i++ ) {
		pStruct->cVecNotEnoughItemActorIndex.push_back( pPacket->cActorIndex[i+nOffset] );
	}
	nOffset += pPacket->cNotEnoughItemCount;

	for( int i=0; i<pPacket->cExceedCount; i++ ) {
		pStruct->cVecExceedTryActorIndex.push_back( pPacket->cActorIndex[i+nOffset] );
	}
	nOffset += pPacket->cExceedCount;

	for( int i=0; i<pPacket->cNotRideVehicleCount; i++ ) {
		pStruct->cVecNotRideVehicleActorIndex.push_back( pPacket->cActorIndex[i+nOffset] );
	}
	nOffset += pPacket->cNotRideVehicleCount;
}

void CDnGameTask::OnRecvRoomOpenDungeonLevel( SCOpenDungeonOpenLevel *pPacket )
{
	CDnStageClearReportDlg *pDlg = GetInterface().GetStageClearReportDialog();
	if( pDlg ) pDlg->SetOpenDungeonInfo( pPacket->nMapIndex, pPacket->cDifficulty );
}

void CDnGameTask::OnRecvPartyRefresh( SCRefreshParty *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvPartyRefresh(pPacket);
	RefreshConnectUser();

	GetInterface().RefreshPartyInfoList();
	GetInterface().RefreshPartyGuage();
	GetInterface().RefreshRebirthInterface();
}

void CDnGameTask::OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvPartyRefreshGateInfo( pPacket );
	GetInterface().RefreshPartyGateInfo();
}

void CDnGameTask::OnRecvPartyReadyGate( SCGateInfo *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvPartyReadyGate( pPacket );

	CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
	CDnWorld::GateStruct *pStruct = CDnWorld::GetInstance().GetGateStruct( m_nGateIndex );
	if( pStruct == NULL ) return;

	if( pPacket->cMapCount > 0 ) {
		if( pStruct->MapType == CDnWorld::MapTypeDungeon ) {
			CDnWorld::DungeonGateStruct *pDungeonStruct = (CDnWorld::DungeonGateStruct *)pStruct;
			if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
				if( pPacket->cMapCount != (char)pDungeonStruct->pVecMapList.size() ) assert(0&&"Ooh! That's cool!!");
				for( int i=0; i<pPacket->cMapCount; i++ ) {
					if( i >= (char)pDungeonStruct->pVecMapList.size() ) continue;

					((CDnWorld::DungeonGateStruct*)pDungeonStruct->pVecMapList[i])->PermitFlag = (CDnWorld::PermitGateEnum)pPacket->cPermitFlag[i];
					memcpy( ((CDnWorld::DungeonGateStruct*)pDungeonStruct->pVecMapList[i])->cCanDifficult, pPacket->cCanEnter[i], sizeof(char) * 5 );
				}
			}
			else {
				pDungeonStruct->PermitFlag = (CDnWorld::PermitGateEnum)pPacket->cPermitFlag[0];
				memcpy( pDungeonStruct->cCanDifficult, pPacket->cCanEnter[0], sizeof(char) * 5 );

			}
		}
	}

	bool bSendStartStage = false;
	switch( CDnWorld::GetInstance().GetMapType() ) {
		case CDnWorld::MapTypeWorldMap:
			if( pStruct->MapType == CDnWorld::MapTypeDungeon )
			{
				// From WorldMap to dungeon...
				CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstance().GetTask( "PartyTask" ));
				if( !pPartyTask ) return;

				int nCount = pPartyTask->GetPartyCount();
				if( !pPartyTask->IsPartyCountToEnterGate( pPartyTask->GetPartyCountExcepGM(), (CDnWorld::DungeonGateStruct*)pStruct ) )
				{
					bSendStartStage = false;
					SendCancelStage();
					break;
				}

				CDnPartyTask::PartyStruct* pParty(NULL);
				for( int i = 0; i<nCount; i++ )
				{
					pParty = pPartyTask->GetPartyData( i );
					if( !pParty ) continue;

					if( !pPartyTask->IsPartyLevelToEnterGate( pParty->cLevel, (CDnWorld::DungeonGateStruct*)pStruct ) )
					{
						bSendStartStage = false;
						SendCancelStage();
						break;
					}
				}
				bSendStartStage = OnDungeonReadyGate( (CDnWorld::DungeonGateStruct*)pStruct );
			}
			else bSendStartStage = true;
			break;
		case CDnWorld::MapTypeDungeon:
			bSendStartStage = true;
			break;
	}

	CDnLocalPlayerActor::LockInput( !(pTask && pTask->IsGMTracing()) );
	CDnLocalPlayerActor::StopAllPartyPlayer();

	if( bSendStartStage ) {
		if( pTask->GetPartyRole() == CDnPartyTask::LEADER || pTask->GetPartyRole() == CDnPartyTask::SINGLE ) {
			SendStartStage( Dungeon::Difficulty::Max );
		}
	}

	GetInterface().ShowMapMoveCashItemDlg( false, NULL, 0 );
}

bool CDnGameTask::OnDungeonReadyGate( CDnWorld::DungeonGateStruct *pStruct )
{
	if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
		GetInterface().OpenDungeonEnterDialog( (CDnWorld::DungeonGateStruct*)pStruct, this );
		return false;
	}
	return true;
}

void CDnGameTask::OnRecvPartyStageStart( SCStartStage *pPacket )
{
	FUNC_LOG();

	DebugLog("CDnGameTask::OnRecvPartyStageStart");
	DnDebugConsole::LogW( L"INSTANCE", L"OnRecvPartyStageStart seed=%d diff=%d bridgeState=%d",
		pPacket->nRandomSeed, (int)pPacket->Difficulty, (int)m_BridgeState );

	CGameClientSession::OnRecvPartyStageStart( pPacket );

	m_nStageRandomSeed = pPacket->nRandomSeed;
	m_nStageConstructionLevel = pPacket->Difficulty;
	m_nExtendDropRate = pPacket->nExtendDropRate;

	CDnLocalPlayerActor::LockInput( true );
	CDnLocalPlayerActor::StopAllPartyPlayer();
	switch( m_BridgeState ) {
		case eBridgeState::VillageToGame:
			{
				CDnBridgeTask *pTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
				if( !pTask ) break;
				if( InitializeStage( pTask->GetVillageMapIndex(), pTask->GetEnteredGateIndex(), pTask->GetStageConstructionLevel(), m_nStageRandomSeed, false, false ) == false ) {
					MessageBox( CDnMainFrame::GetInstance().GetHWnd(), _T("pGameTask->InitializeStage Failed"), _T("Error"), MB_OK );
					CDnMainFrame::GetInstance().DestroyMainFrame();
					return;
				}
			}
			break;
		default:
			GetInterface().FadeDialog( 0x00000000, 0xff000000, CGlobalInfo::GetInstance().m_fFadeDelta, this );
			CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );

			GetInterface().ShowGateQuestionDialog( false );
			if( m_bShowDungeonOpenNotice ) {
				GetInterface().ShowDungeonOpenNoticeDialog( true );
				m_bShowDungeonOpenNotice = false;
			}

			m_BridgeState = eBridgeState::GameToGame;
			CClientSessionManager::GetInstance().SkipProcess();
			break;
	}
}

void CDnGameTask::OnRecvPartyStageStartDirect( SCStartStageDirect *pPacket )
{
	FUNC_LOG();

	DebugLog("CDnGameTask::OnRecvPartyStageStartDirect");
	DnDebugConsole::LogW( L"INSTANCE", L"OnRecvPartyStageStartDirect mapIdx=%d gate=%d diff=%d seed=%d bridgeState=%d",
		pPacket->nMapIndex, (int)pPacket->cGateNo, (int)pPacket->Difficulty, pPacket->nRandomSeed, (int)m_BridgeState );

	CGameClientSession::OnRecvPartyStageStartDirect( pPacket );

	m_nStageRandomSeed = pPacket->nRandomSeed;
	m_nDirectConnectMapIndex = pPacket->nMapIndex;
	m_nDirectConnectGateIndex = pPacket->cGateNo;
	m_nStageConstructionLevel = pPacket->Difficulty;
	m_nExtendDropRate = pPacket->nExtendDropRate;

	CDnLocalPlayerActor::LockInput( true );
	CDnLocalPlayerActor::StopAllPartyPlayer();
	switch( m_BridgeState ) {
		case eBridgeState::VillageToGame:
		case eBridgeState::LoginToGame:
			if( InitializeStage( m_nDirectConnectMapIndex, m_nDirectConnectGateIndex, m_nStageConstructionLevel, m_nStageRandomSeed, false, true ) == false ) {
				MessageBox( CDnMainFrame::GetInstance().GetHWnd(), _T("pGameTask->InitializeStage Failed"), _T("Error"), MB_OK );
				CDnMainFrame::GetInstance().DestroyMainFrame();
				return;
			}
			break;
		default:
			GetInterface().FadeDialog( 0x00000000, 0xff000000, CGlobalInfo::GetInstance().m_fFadeDelta, this );
			CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );

			GetInterface().ShowGateQuestionDialog( false );
			if( m_bShowDungeonOpenNotice ) {
				GetInterface().ShowDungeonOpenNoticeDialog( true );
				m_bShowDungeonOpenNotice = false;
			}

			m_BridgeState = eBridgeState::GameToGame_Direct;
			CClientSessionManager::GetInstance().SkipProcess();
			break;
	}
}

void CDnGameTask::OnRecvPartyStageCancel( SCCancelStage *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvPartyStageCancel( pPacket );

	CDnLocalPlayerActor::LockInput( false );
	GetInterface().ShowGateQuestionDialog( false );
	GetInterface().CloseBlind();

	if ( pPacket->nRetCode != ERROR_NONE )
		GetInterface().ServerMessageBox( pPacket->nRetCode );

	//switch( pPacket->cRetCode ) 
	//{
	//case -1:
	//	GetInterface().MessageBox( 100024 );
	//	break;
	//}
}

void CDnGameTask::OnRecvPartyVillageStart( char *pPacket )
{
	FUNC_LOG();
	CGameClientSession::OnRecvPartyVillageStart( pPacket );

	GetInterface().FadeDialog( 0x00000000, 0xff000000, CGlobalInfo::GetInstance().m_fFadeDelta, this );
	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );

	GetInterface().ShowGateQuestionDialog( false );
	//GetInterface().CloseDungeonClearDialog();

	m_BridgeState = eBridgeState::GameToVillage;	
}

void CDnGameTask::OnRecvCharEntered( SCEnter *pPacket )
{
	if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().Initialize( this, 0 );
	// Player Load
	DnActorHandle hPlayer = CreateActor( pPacket->cClassID, true, true, false );
	CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor *)hPlayer.GetPointer();
	CDnActor::s_hLocalActor = hPlayer;
	m_bIsCreateLocalActor = true;

	hPlayer->SetUniqueID( CDnBridgeTask::GetInstance().GetSessionID() );

	std::vector<int> nVecJobList;
	for( int i=0; i<JOBMAX; i++ ) {
		if( pPacket->cJobArray[i] == 0 ) break;
		nVecJobList.push_back( pPacket->cJobArray[i] );
	}
	pActor->SetJobHistory( nVecJobList );

	hPlayer->SetLevel( pPacket->cLevel );
	pActor->SetExperience( pPacket->nExp );
	pActor->SetPvPLevel( pPacket->sPvPInfo.cLevel );

	pActor->SetPosition( pPacket->Position );

	EtVector2 vDir( sin( EtToRadian( pPacket->fRotate ) ), cos( EtToRadian( pPacket->fRotate ) ) );
	pActor->Look( vDir );

	pActor->InitializeSession( this );
	pActor->SetName( pPacket->wszCharacterName );
	pActor->SetPartsColor( MAPartsBody::HairColor, pPacket->dwHairColor );
	pActor->SetPartsColor( MAPartsBody::EyeColor, pPacket->dwEyeColor );
	pActor->SetPartsColor( MAPartsBody::SkinColor, pPacket->dwSkinColor );

	m_hLocalPlayerActor = hPlayer;
	
	// Set guild info
	pActor->SetGuildSelfView( pPacket->GuildSelfView );

	// Interface Initialize
	GetInterface().Initialize( CDnInterface::Game, this );

	if( CDnPartyTask::IsActive() ) {
		CDnPartyTask::GetInstance().SetPvPInfo( &pPacket->sPvPInfo );
		CDnPartyTask::GetInstance().SetLocalData( hPlayer );
	}

#ifdef PRE_ADD_BESTFRIEND
	CDnAppellationTask::GetInstance().SelectAppellation( pPacket->nCoverAppellation, pPacket->nAppellation );
#endif

	pActor->RefreshState();

	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eDAILYCOIN, pPacket->cRebirthCoin);
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eCASHCOIN, pPacket->wRebirthCashCoin);
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::ePCBANGCOIN, pPacket->cPCBangRebirthCoin);
#ifdef PRE_ADD_VIP
	CDnItemTask::GetInstance().SetRebirthCoin(CDnItemTask::eVIPCOIN, pPacket->cVIPRebirthCoin);
#endif

#ifdef PRE_FIX_NESTREBIRTH
	CDnItemTask::GetInstance().SetUpdateRebirthCoinOrItem(true);
#endif

	CDnItemTask::GetInstance().SetFatigue( pPacket->wFatigue, pPacket->wWeekFatigue, pPacket->wPCBangFatigue, pPacket->wEventFatigue );
	CDnItemTask::GetInstance().SetMaxFatigue( CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::Fatigue ),
		CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::WeekFatigue ),
		0,
		pPacket->wMaxEventFatigue );

#ifdef PRE_ADD_VIP
	CDnItemTask::GetInstance().SetFatigue(CDnItemTask::eVIPFTG, pPacket->wVIPFatigue);
	CDnItemTask::GetInstance().SetMaxFatigue(CDnItemTask::eVIPFTG, CPlayerLevelTable::GetInstance().GetValue( pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::VIPFatigue ));
#endif

	// SC_ENTER reportedly arrives at the game task only when entering the tutorial stage.
	m_bTutorial = true;
	if( CDnGestureTask::IsActive() ) GetGestureTask().RefreshGestureList( pPacket->cLevel );
	SAFE_RELEASE_SPTR( hPlayer );	

#if defined(_WORK)
	CDnCommonTask *pCommonTask = (CDnCommonTask *)CTaskManager::GetInstance().GetTask( "CommonTask" );
	if( pCommonTask ) pCommonTask->ResetPlayTimeNotice();
#endif

#if defined( PRE_ADD_PRESET_SKILLTREE )
	GetSkillTask().SendPresetSkillTreeList();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
}

void CDnGameTask::OnRecvCharHide( SCHide *pPacket )
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

void CDnGameTask::InitializeMonster( DnActorHandle hMonster, DWORD dwUniqueID, int nMonsterID, SOBB &GenerationArea, CEtWorldEventArea *pBirthArea )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	CDnMonsterActor *pMonster = dynamic_cast<CDnMonsterActor *>(hMonster.GetPointer());

	hMonster->SetUniqueID( dwUniqueID );
	pMonster->SetMonsterClassID( nMonsterID );
	pMonster->SetAIDifficult( (CDnMonsterActor::AIDifficult)m_nStageConstructionLevel );
	hMonster->Initialize();

	char szWeaponLable[32];
	for( int j=0; j<2; j++ ) {
		sprintf_s( szWeaponLable, "_Weapon%dIndex", j + 1 );
		int nWeapon = pSox->GetFieldFromLablePtr( nMonsterID, szWeaponLable )->GetInteger();
		if( nWeapon < 1 ) continue;
		DnWeaponHandle hWeapon = CDnWeapon::CreateWeapon( nWeapon, _rand() );
		hMonster->AttachWeapon( hWeapon, j, true );
	}
	// Random size
	_fpreset();
	float fScale = 1.f;
	int nMin = (int)( pSox->GetFieldFromLablePtr( nMonsterID, "_SizeMin" )->GetFloat() * 100.f );
	int nMax = (int)( pSox->GetFieldFromLablePtr( nMonsterID, "_SizeMax" )->GetFloat() * 100.f );

#ifdef PRE_ADD_MONSTER_CHEAT
	int nSkillTableId = pSox->GetFieldFromLablePtr( nMonsterID, "_SkillTable")->GetInteger();
	CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>( hMonster.GetPointer() );
	pMonsterActor->SetSkillTableID( nSkillTableId );
	std::string szFileName = pSox->GetFieldFromLablePtr( nMonsterID, "_CustomAI")->GetString();
	pMonsterActor->SetAIFileName( szFileName );
#endif

	if( nMin <= nMax ) {
		fScale = ( nMin + ( _rand()%( ( nMax + 1 ) - nMin ) ) ) / 100.f;
	}
	pMonster->SetScale( fScale );

	pMonster->SetGenerationArea( GenerationArea );

	pMonster->InitializeSession( this );

	std::wstring wszMonsterName;
	int nNameID = pSox->GetFieldFromLablePtr( nMonsterID, "_NameID" )->GetInteger();
	char *szParam = pSox->GetFieldFromLablePtr( nMonsterID, "_NameIDParam" )->GetString();
	MakeUIStringUseVariableParam( wszMonsterName, nNameID, szParam );
	hMonster->SetName( wszMonsterName.c_str() );
	hMonster->SetLevel( pSox->GetFieldFromLablePtr( nMonsterID, "_Level" )->GetInteger() );
	hMonster->RefreshState();
	hMonster->SetHP( hMonster->GetMaxHP() );
	hMonster->SetSP( hMonster->GetMaxSP() );
	hMonster->Look( EtVec3toVec2( GenerationArea.Axis[2] ) );

#ifdef PRE_ADD_MONSTER_PARTS_UI_TRIGGER
	hMonster->SetCurrentSuperArmor( pSox->GetFieldFromLablePtr( nMonsterID, "_SuperAmmor" )->GetInteger() );
	hMonster->SetMaxSuperArmor( pSox->GetFieldFromLablePtr( nMonsterID, "_SuperAmmor" )->GetInteger() );
#endif 
	// StaticMovement Type has no Look, so force-set it.
	if( hMonster->GetMovement() && hMonster->GetMovement()->bIsStaticMovement() == true )
		hMonster->GetMovement()->ForceLook( EtVec3toVec2( GenerationArea.Axis[2] ) );

	if( pBirthArea ) {
		InsertBirthAreaList( hMonster, pBirthArea->GetCreateUniqueID() );
//		UnitAreaStruct *pStruct = (UnitAreaStruct *)pBirthArea->GetData();
//		pMonster->SetBoss( ( pStruct->bBoss == TRUE ) );
		EtVector2 vDir( sin( EtToRadian( pBirthArea->GetRotate() ) ), cos( EtToRadian( pBirthArea->GetRotate() ) ) );
		hMonster->Look( vDir );
	}
	OutputDebug( "Generation Monster : %d - %d\n", hMonster->GetClassID(), dwUniqueID );
}

DnActorHandle CDnGameTask::GenerationMonsterFromMonsterID( int nSeed, int nMonsterID, DWORD dwUniqueID, EtVector3 &vPosition, EtVector3 &vVel, 
														    EtVector3 &vResistance, SOBB *pGenerationArea, CEtWorldEventArea *pBirthArea, 
															LOCAL_TIME CreateTime, int nTeam, bool bRandomFrameSummon, EtQuat* pqRotation )
{
	DWORD dwPrevTime = timeGetTime();
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	if( pSox->IsExistItem( nMonsterID ) == false ) return CDnActor::Identity();
	int nActorTableID = pSox->GetFieldFromLablePtr( nMonsterID, "_ActorTableID" )->GetInteger();

	DWORD dwTime1 = timeGetTime();
	_srand( nSeed );
	DnActorHandle hMonster = CreateActor( nActorTableID, false, false, false );
	if( !hMonster ) return CDnActor::Identity();
	hMonster->Show( false );

	hMonster->SetPosition( vPosition );
	InitializeMonster( hMonster, dwUniqueID, nMonsterID, *pGenerationArea, pBirthArea );

	// #28351 Apply rotation value when the packet arrives via SummonMonster signal.
	if( pqRotation )
	{
		EtMatrix matRot;
		EtMatrixRotationQuaternion( &matRot, pqRotation );
		MatrixEx CrossRotation = matRot;
		hMonster->GetMatEx()->CopyRotationFromThis( &CrossRotation );
	}

	hMonster->SetVelocity(vVel);
	hMonster->SetResistance(vResistance);
	if( nTeam >= 0 ) {
		hMonster->SetTeam( nTeam );
	}

	DWORD dwCurTime = timeGetTime();
	float fFps = hMonster->CDnActionBase::GetFPS();
	float fFrame = 0.f;
	if( fFps > 0.f ) {
		LOCAL_TIME elapsedMs = ( m_LocalTime - CreateTime ) + (LOCAL_TIME)( dwCurTime - dwPrevTime );
		if( elapsedMs > 0 )
			fFrame = ( (float)elapsedMs / 1000.f ) * fFps;
	}
	if( !hMonster->IsDie() && hMonster->IsExistAction("Summon_On") ) {
		CDnActionBase::ActionElementStruct* pSummonElem = hMonster->GetElement( "Summon_On" );
		if( pSummonElem && pSummonElem->dwLength > 0 && fFrame >= (float)pSummonElem->dwLength )
			fFrame = (float)pSummonElem->dwLength - 1.f;
		if( fFrame < 0.f )
			fFrame = 0.f;

		hMonster->SyncClassTime( m_LocalTime );
		hMonster->SetAction("Summon_On", (bRandomFrameSummon) ? fFrame : 0.f, 0.f, false);

		if (bRandomFrameSummon) {
			hMonster->SetPlaySpeed( ( _rand()%20 ) * 100, 0.f );
			hMonster->SetAlphaBlend( 0.f );
		}
	}

	hMonster->Show( true );
	hMonster->SetProcess( true );
	m_hVecMonsterList.push_back( hMonster );

#ifdef PRE_MOD_PRELOAD_MONSTER
	if (nTeam == 3)
	{
		OutputDebug("Team 3 Monster Destroy");
		hMonster->SetDestroy();
	}
#endif

	OutputDebug("dwUniqueID: %d UniqueID : %d x:%0.2f y:%0.2f z:%0.2f actorid: %d team: %d\n", dwUniqueID ,hMonster->GetUniqueID(), vPosition.x, vPosition.y, vPosition.z, hMonster->GetClassID(), hMonster->GetTeam());
	LogWnd::Log( 10, L"Creation Time : %d " , dwCurTime );
	OutputDebug("Creation Time : %d ", dwCurTime );

	return hMonster;
}

/*
void CDnGameTask::GenerationMonsterFromMonsterGroupID( int nMonsterGroupID, int nCount, SAABox &GenerationArea, std::vector<DnActorHandle> *pVecResult )
{
	using namespace TempStruct;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERCOMPOUND );
	std::vector<MonsterCompoundStruct> VecCompoundList;
	std::vector<int> nVecOffset;
	MonsterCompoundStruct Struct;
	int nRandomMax = 0;

	char szLabel[32];
	for( int j=0; j<20; j++ ) {
		sprintf_s( szLabel, "_MonsterTableID%d", j + 1 );
		Struct.nMonsterTableID = pSox->GetFieldFromLablePtr( nMonsterGroupID, szLabel )->GetInteger();
		if( Struct.nMonsterTableID < 1 ) continue;

		sprintf_s( szLabel, "_Ratio%d", j + 1 );
		Struct.nPossessionProb = (int)( pSox->GetFieldFromLablePtr( nMonsterGroupID, szLabel )->GetFloat() * 100 );

		nRandomMax += Struct.nPossessionProb;

		nVecOffset.push_back( nRandomMax );
		VecCompoundList.push_back( Struct );
	}

	if( nRandomMax < 100 ) nRandomMax = 100;

	EtVector3 vPos;
	DnActorHandle hMonster;
	for( int j=0; j<nCount; j++ ) {
		int nResult = _rand()%nRandomMax;
		for( DWORD k=0; k<nVecOffset.size(); k++ ) {
			if( nResult < nVecOffset[k] ) {
				vPos.x = GenerationArea.Min.x + ( _rand()%(int)( GenerationArea.Max.x - GenerationArea.Min.x ) );
				vPos.z = GenerationArea.Min.z + ( _rand()%(int)( GenerationArea.Max.z - GenerationArea.Min.z ) );

				hMonster = GenerationMonsterFromMonsterID( VecCompoundList[k].nMonsterTableID, vPos, &GenerationArea );
				if( !hMonster ) break;
				if( pVecResult ) pVecResult->push_back( hMonster );
				break;
			}
		}
	}
}
*/


void CDnGameTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) 
	{
	case DUNGEON_ENTER_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_BUTTON_ENTER" ) == 0 )
				{
#ifdef PRE_MOD_MISSION_HELPER
					GetMissionTask().AutoRegisterMissionNotifier();
#endif
					CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
					if( pTask->GetPartyRole() == CDnPartyTask::LEADER || pTask->GetPartyRole() == CDnPartyTask::SINGLE ) {
						int nSelectDungeonIndex = GetInterface().GetCurrentDungeonEnterDialog()->GetDungeonGateIndex(); 
						m_nStageConstructionLevel = GetInterface().GetCurrentDungeonEnterDialog()->GetDungeonLevel();					 
#ifdef PRE_PARTY_DB
						SendStartStage( (TDUNGEONDIFFICULTY)m_nStageConstructionLevel, (char)nSelectDungeonIndex );
#else
						SendStartStage( (char)m_nStageConstructionLevel, (char)nSelectDungeonIndex );
#endif

					}
				}
				else if( strcmp( pControl->GetControlName(), "ID_BUTTON_CANCEL" ) == 0 )
				{
					SendCancelStage();
				}
			}
		}
		break;
	case GATEQUESTION_DIALOG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( strcmp( pControl->GetControlName(), "ID_START" ) == 0 ) 
				{
					CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
					if( pTask->GetPartyRole() == CDnPartyTask::LEADER || pTask->GetPartyRole() == CDnPartyTask::SINGLE ) {
						m_nStageConstructionLevel = 0;
#ifdef PRE_PARTY_DB
						SendStartStage( (TDUNGEONDIFFICULTY)m_nStageConstructionLevel );
#else
						SendStartStage( (char)m_nStageConstructionLevel );
#endif
					}
				}
				else if( strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0 )
				{
					SendCancelStage();
				}
			}
		}
		break;
	case MESSAGEBOX_10:
		if( nCommand == EVENT_BUTTON_CLICKED ) 
		{
			if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
			{
				if( CDnLoadingTask::IsActive() ) CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
				CTaskManager::GetInstance().RemoveAllTask( false );
			}
		}
		break;
		//Inseon confirmed it is not being used
	/*case DUNGEON_CLEAR_MOVE_DIALOG:
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 ) 
			{
				SendWarpDungeonClear( false );
			}
			else if( strcmp( pControl->GetControlName(), "ID_CANCEL" ) == 0 )
			{
				SendWarpDungeonClear( true );
			}
		}
		break;*/
	case FADE_DIALOG:
		if( nCommand == EVENT_FADE_COMPLETE )
		{
			//GetInterface().CloseDungeonClearDialog();

			switch( m_BridgeState ) {
				case eBridgeState::GameToGame:
					{
						CDnLoadingTask *pTask = (CDnLoadingTask *)CTaskManager::GetInstance().GetTask( "LoadingTask" );
						if( pTask ) pTask->Initialize( this );

						InitializeStage( m_nMapTableID, m_nGateIndex, m_nStageConstructionLevel, m_nStageRandomSeed, true, false );
						if( CDnItemTask::IsActive() ) CDnItemTask::GetInstance().ResetRequestWait();
					}
					break;
				case eBridgeState::GameToGame_Direct:
					{
						CDnLoadingTask *pTask = (CDnLoadingTask *)CTaskManager::GetInstance().GetTask( "LoadingTask" );
						if( pTask ) pTask->Initialize( this );

						InitializeStage( m_nDirectConnectMapIndex, m_nDirectConnectGateIndex, m_nStageConstructionLevel, m_nStageRandomSeed, true, true );
						if( CDnItemTask::IsActive() ) CDnItemTask::GetInstance().ResetRequestWait();
					}
					break;
				case eBridgeState::GameToVillage:
					{
						DestroyTask( true );
						FinalizeStage();

						CDnVillageTask *pVillageTask = new CDnVillageTask;
						CTaskManager::GetInstance().AddTask( pVillageTask, "VillageTask", -1, false );
						pVillageTask->Initialize( true );

						CDnLoadingTask *pTask = (CDnLoadingTask *)CTaskManager::GetInstance().GetTask( "LoadingTask" );
						if( pTask ) pTask->Initialize( pVillageTask );
					}
					break;
			}
		}
		break;
	}
}

void CDnGameTask::CheckBattleMode( DnActorHandle hActor, bool bAddRemoveValue )
{
	if( CDnActor::s_hLocalActor && hActor->GetTeam() == CDnActor::s_hLocalActor->GetTeam() ) return;

	if( bAddRemoveValue ) {
		std::vector<DnActorHandle>::iterator it = std::find( m_hVecAggroList.begin(), m_hVecAggroList.end(), hActor );
		if( it == m_hVecAggroList.end() ) {
			m_hVecAggroList.push_back( hActor );
			m_nAggroValue++;
		}
	}
	else {
		std::vector<DnActorHandle>::iterator it = std::find( m_hVecAggroList.begin(), m_hVecAggroList.end(), hActor );
		if( it != m_hVecAggroList.end() ) {
			m_hVecAggroList.erase( it );
			m_nAggroValue--;
		}
	}

	if( !m_pWorld ) return;

//	m_bIsCombat = false;
	int nAggroValue = 0;
	char cFlag = 0;
	for( DWORD i=0; i<m_hVecAggroList.size(); i++ ) {
		if( !m_hVecAggroList[i] || m_hVecAggroList[i]->IsDie() ) continue;
		CDnMonsterActor *pMonster = dynamic_cast<CDnMonsterActor *>(m_hVecAggroList[i].GetPointer());
		MAAiReceiver *pAiReceiver = dynamic_cast<MAAiReceiver *>(pMonster);
		if( !pMonster || !pAiReceiver || !pAiReceiver->GetAIBase() ||!pAiReceiver->GetAIBase()->GetTarget() ) continue;

		switch( pMonster->GetGrade() ) {
			case CDnMonsterState::Minion:
			case CDnMonsterState::Normal:
			case CDnMonsterState::Champion:
			case CDnMonsterState::Elite:
			case CDnMonsterState::Named:
				if( cFlag < 1 ) cFlag = 1;
				break;
			case CDnMonsterState::Boss:
			case CDnMonsterState::BossHP4:
				if( cFlag < 2 ) cFlag = 2;
				break;
			case CDnMonsterState::NestBoss:
			case CDnMonsterState::NestBoss8:
				if( cFlag < 3 ) cFlag = 3;
				break;
		}
	}
	switch( cFlag ) {
		case 0: m_pWorld->ChangeBattleBGM( CDnWorld::NoneBattle, 0.f, 5.f, 1.f, 5.f ); break;
		case 1: m_pWorld->ChangeBattleBGM( CDnWorld::NormalBattle, 0.f, 1.f, 1.f, 1.f ); break;
		case 2: m_pWorld->ChangeBattleBGM( CDnWorld::BossBattle, 0.f, 1.f, 1.f, 1.f ); break;
		case 3: m_pWorld->ChangeBattleBGM( CDnWorld::NestBattle, 0.f, 1.f, 1.f, 1.f ); break;
	}
	m_bIsCombat = true;

	if( cFlag == 0 ) {
		m_fCombatDelta = 3.f;		
	}
	else m_fCombatDelta = 0.f;
	OutputDebug( "Change BGM : %d\n", cFlag );
}

void CDnGameTask::ProcessCombat( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bIsCombat && m_fCombatDelta > 0.f ) {
		m_fCombatDelta -= fDelta;
		if( m_fCombatDelta <= 0.f ) {
			m_fCombatDelta = 0.f;
			m_bIsCombat = false;
		}
	}
}

void CDnGameTask::InsertNpcList( DnActorHandle hActor )
{
	m_hVecNpcList.push_back( hActor );

	if( m_pMinimap ) {
		m_pMinimap->InsertNpc( hActor );
	}
}

void CDnGameTask::RemoveNpcList( DnActorHandle hActor )
{
	std::vector<DnActorHandle>::iterator it = std::find( m_hVecNpcList.begin(), m_hVecNpcList.end(), hActor );
	if( it == m_hVecNpcList.end() ) return;

	m_hVecNpcList.erase( it );

	if( m_pMinimap ) {
		m_pMinimap->RemoveNpc( hActor );
	}
}

void CDnGameTask::GetEnableGateIndex( CDnWorld::DungeonGateStruct *pStruct, std::vector<int> &vecGateIndex )
{
	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstance().GetTask( "PartyTask" ));
	if( !pPartyTask ) return;

	CDnPartyTask::PartyStruct* pParty(NULL);
	int nPartyCount = pPartyTask->GetPartyCount();

	CDnWorld::DungeonGateStruct *pGateStruct(NULL);

	for( int i=0; i<(int)pStruct->pVecMapList.size(); i++ )
	{
		pGateStruct = (CDnWorld::DungeonGateStruct*)pStruct->pVecMapList[i];
		if( !pGateStruct ) continue;

		int nRealPartyCount = pPartyTask->GetPartyCountExcepGM();
		if( !pGateStruct->IsEnablePartyCount( nRealPartyCount ) )
			continue;

		bool bEnable(true);

		for( int j = 0; j<nPartyCount; j++ )
		{
			pParty = pPartyTask->GetPartyData( j );

			if( !pParty || pParty->bGMTrace ) continue;
			if( !pGateStruct->IsEnableLevel( pParty->cLevel ) )
			{
				bEnable = false;
			}
		}

		if (pGateStruct->PermitFlag & CDnWorld::PermitClose)
			bEnable = false;

		if( bEnable )
		{
			vecGateIndex.push_back(i);
		}
	}
}

void CDnGameTask::ResetPlayerCamera()
{
	if( !m_hPlayerCamera ) return;
	((CDnPlayerCamera*)m_hPlayerCamera.GetPointer())->ResetCamera();
}

void CDnGameTask::InitializeCP()
{
	if( !CDnActor::s_hLocalActor ) return;
	// Set up CP table.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );

	char szLabel[64];
	static char *szStaticLabel[] = { "Easy", "Normal", "Hard", "VeryHard", "Nightmare" };

	sprintf_s( szLabel, "_CPTableID_%s", szStaticLabel[m_nStageConstructionLevel] );
	int nCPTableID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();

	sprintf_s( szLabel, "_ClearDungeonTableID_%s", szStaticLabel[m_nStageConstructionLevel] );
	int nDungeonClearID = pSox->GetFieldFromLablePtr( m_nMapTableID, szLabel )->GetInteger();

	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if( nCPTableID > 0 )
		pPlayer->InitializeCPScore( nCPTableID, pPlayer->GetClassID(), nDungeonClearID );
}

void CDnGameTask::OnGhost( DnActorHandle hActor )
{
	if( !hActor ) return;
	CDnPlayerActor *pActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
	if( pActor->IsLocalActor() ) {
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(hActor.GetPointer());

	#ifdef PRE_MOD_NESTREBIRTH
		if (pLocalActor)
			SetRebirthCaptionOnGhost(pLocalActor, true);
	#else
		if( pLocalActor->IsGhost() ) {
			if( pLocalActor->GetCurrentGhostType() == Ghost::Type::PartyRestore || CDnItemTask::GetInstance().IsCanRebirth() ) {
				GetInterface().OpenRebirthCaptionDlg();
			}

			pLocalActor->SetWorldColorFilterOnDie();
		}
		else {
			GetInterface().CloseRebirthCaptionDlg();
		}
	#endif // PRE_MOD_NESTREBIRTH
	}
	GetInterface().RefreshRebirthInterface();
}

void CDnGameTask::SetRebirthCaptionOnGhost( CDnLocalPlayerActor* pLocalActor, bool bColorFilterOnDie )
{
	if( pLocalActor->IsGhost() )
	{
		if( pLocalActor->GetCurrentGhostType() == Ghost::Type::PartyRestore || CDnItemTask::GetInstance().IsCanRebirth() )
		{
			GetInterface().OpenRebirthCaptionDlg();
		}

#ifdef PRE_ADD_INSTANT_CASH_BUY
		if( CDnItemTask::GetInstance().IsCanRebirthIfHaveCoin() )
		{
			GetInterface().OpenRebirthCaptionDlg();
		}
#endif // PRE_ADD_INSTANT_CASH_BUY

		if( bColorFilterOnDie )
			pLocalActor->SetWorldColorFilterOnDie();
	}
	else
	{
		GetInterface().CloseRebirthCaptionDlg();
	}
}

void CDnGameTask::OnRecvRoomIdentifyRewardItem( SCIdentifyRewardItem *pPacket )
{
	CDnStageClearDlg *pDlg = GetInterface().GetStageClearDialog();
	if( !pDlg ) return;
	pDlg->OpenBonusBox( pPacket->cItemIndex, pPacket->cBoxType );
}

void CDnGameTask::InsertBirthAreaList( DnActorHandle hActor, int nAreaUniqueID )
{	for( DWORD i=0; i<m_pVecActorBirthList.size(); i++ ) {
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

void CDnGameTask::OnRecvMaxCharCount( SCMaxLevelCharacterCount* pPacket )
{
	m_nMaxCharCount = pPacket->iCount;
}

void CDnGameTask::OnRecvSpecialRebirthItem( SCSpecialRebirthItem * pData )
{
	CDnItemTask::GetInstance().SetSpecialRebirthItemID( pData->nItemID );
	CDnItemTask::GetInstance().SetSpecialRebirthItemCount( pData->nRebirthItemCount );

	if (CDnActor::s_hLocalActor)
	{
		CDnLocalPlayerActor* pLocalActor = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		SetRebirthCaptionOnGhost(pLocalActor, false);
	}
}

#if defined(PRE_ADD_REBIRTH_EVENT)
void CDnGameTask::OnRecvRebirthMaxCoin( SCRebirthMaxCoin* pPacket )
{
	CDnItemTask::GetInstance().SetMaxUsableRebirthCoin( pPacket->nRebirthMaxCoin );
}
#endif

#ifdef PRE_ADD_CRAZYDUC_UI
void CDnGameTask::ShowMODDialog( bool bShow )
{
	m_bShowModDlg = bShow;
}

void CDnGameTask::SetTimeAttack( int nRemainSec, int nOriginSec )
{
	m_sTimeAttackStruct.nRemain = nRemainSec;
	m_sTimeAttackStruct.nOrigin = nOriginSec;
}
#endif

void CDnGameTask::OnRecvDungeonTimeAttack( SCSyncDungeonTimeAttack* pPacket )
{
	if( pPacket == NULL )
		return;

	if( pPacket->iRemainSec > 0 )
	{
#ifdef PRE_ADD_CRAZYDUC_UI
		ShowMODDialog( true );
		SetTimeAttack( pPacket->iRemainSec, pPacket->iOrgSec );
#endif 
		GetInterface().ShowHardCoreModeTimeAttackDlg( true );
		GetInterface().SetHardCoreModeRemainTime( pPacket->iOrgSec, pPacket->iRemainSec );
	}
}

void CDnGameTask::OnRecvDungeonTimeAttackReset()
{
#ifdef PRE_ADD_CRAZYDUC_UI
	ShowMODDialog( false ); 
	SetTimeAttack( 0, 0 );
#endif 
	GetInterface().ShowHardCoreModeTimeAttackDlg( false );
	GetInterface().ResetHardCoreModeTimer();
 
}

#ifdef PRE_ADD_BESTFRIEND
//void CDnGameTask::OnRecvBestFriendData( char * pData )
//{
//	SCPartyBestFriend * packet = (SCPartyBestFriend *)( pData );
//	
//	int size = CDnPartyTask::GetInstance().GetPartyCount();
//	if( m_VecConnectList.empty() || m_VecConnectList.size() < size )
//		return;
//
//	DnActorHandle hActor;
//	for( int i=0; i<packet->nCount; ++i )
//	{
//		for( DWORD k=0; k<size; k++ )
//		{
//			hActor = m_VecConnectList[k].hActor;
//			if( hActor && ( hActor->GetUniqueID() == packet->List[i].nSessionID ))
//			{
//				hActor->SetBFserial( packet->List[i].biBFItemSerial );
//				hActor->SetBestfriendName( packet->List[i].wszBFName );
//				break;
//			}
//		}	
//	}
//}

void CDnGameTask::OnRecvBestFriendData( char * pData )
{	
	if( pData )
	{
		memset( &m_TempBFpacket, 0, sizeof(SCPartyBestFriend) );
		SCPartyBestFriend * pTemp = (SCPartyBestFriend*)(pData);
		if( pTemp->nCount > 0 && pTemp->nCount < PARTYMAX )
		{
			m_TempBFpacket.nCount = pTemp->nCount;
			for( int i=0; i<pTemp->nCount; ++i )
				memcpy( &(m_TempBFpacket.List[i]), &(pTemp->List[i]), sizeof(TPartyBestFriend) );
		}
	}
	//memcpy(&m_TempBFpacket, pData, sizeof(SCPartyBestFriend));	
}

void CDnGameTask::SetBestFriendData()
{
	if( m_TempBFpacket.nCount <= 0 )
		return;

	//int size = CDnPartyTask::GetInstance().GetPartyCount();
	if( m_VecConnectList.empty() )
		return;

	int size = (int)m_VecConnectList.size();

	DnActorHandle hActor;
	for( int i=0; i<m_TempBFpacket.nCount; ++i )
	{
		for( int k=0; k<size; ++k )
		{
			hActor = m_VecConnectList[k].hActor;
			if( hActor && ( hActor->GetUniqueID() == m_TempBFpacket.List[i].nSessionID ))
			{
				hActor->SetBFserial( m_TempBFpacket.List[i].biBFItemSerial );
				hActor->SetBestfriendName( m_TempBFpacket.List[i].wszBFName );


				((CDnPlayerActor *)hActor.GetPointer())->SetAppellationName( m_TempBFpacket.List[i].wszBFName );

				break;
			}
		}	
	}
}


#endif // PRE_ADD_BESTFRIEND


#if defined(PRE_ADD_CP_RANK)
void CDnGameTask::OnRecvCPRank( SCAbyssStageClearBest *pData )
{
	m_bDungeonClearBestInfo = true;

	m_DungeonClearBestInfo.sMyClearBest.cRank = pData->cMyBestRank;
	m_DungeonClearBestInfo.sMyClearBest.nClearPoint = pData->nMyBestCP;

	m_DungeonClearBestInfo.sLegendClearBest.cRank = pData->sLegendClearBest.cRank;
	m_DungeonClearBestInfo.sLegendClearBest.nClearPoint = pData->sLegendClearBest.nClearPoint;
	m_DungeonClearBestInfo.sLegendClearBest.wszName = std::wstring( pData->sLegendClearBest.wszCharName );

	m_DungeonClearBestInfo.sMonthlyClearBest.cRank = pData->sMonthlyClearBest.cRank;
	m_DungeonClearBestInfo.sMonthlyClearBest.nClearPoint = pData->sMonthlyClearBest.nClearPoint;
	m_DungeonClearBestInfo.sMonthlyClearBest.wszName = std::wstring( pData->sMonthlyClearBest.wszCharName );
}
#endif	// #if defined(PRE_ADD_CP_RANK)

// Compare .cam filename.
bool CDnGameTask::CompareCamFileName( std::string & str )
{
	return ( m_strPreCameraEffectCamFile == str );
}

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