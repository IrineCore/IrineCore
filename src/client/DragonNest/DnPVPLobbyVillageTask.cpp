#include "StdAfx.h"
#include "DnPVPLobbyVillageTask.h"
#include "SystemSendPacket.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnInterfaceCamera.h"
#include "VillageSendPacket.h"
#include "PVPSendPacket.h"
#include "DnInCodeResource.h"
#include "TaskManager.h"
#include "EtPrimitiveDraw.h"
#include "DnPVPGameRoomDlg.h"
#include "DnGameTipTask.h"
#include "DnQuestTask.h"
#include "DnTableDB.h"
#include "DnActorClassDefine.h"
#include "EtActionCoreMng.h"
#include "DnPVPLadderSystemDlg.h"

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#include "DnPVPLadderTabDlg.h"
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_MOD_PVP_LADDER_XP
#include "DnPVPLobbyChatTabDlg.h"
#endif // PRE_MOD_PVP_LADDER_XP


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
DnActorHandle CDnPVPLobbyVillageTask::m_hPreloadPlayer[10];
bool CDnPVPLobbyVillageTask::s_bPreloadComplete = false;
#endif

CDnPVPLobbyVillageTask::CDnPVPLobbyVillageTask()
{
	m_bFirstConnect = false;
	m_bEntered =  false;
	m_nLobbyBGMIndex =-1;
	memset(&m_sUserInfo,0,sizeof(m_sUserInfo));


	m_VillageType = PvPLobbyVillage;
 }

CDnPVPLobbyVillageTask::~CDnPVPLobbyVillageTask()
{
	SAFE_RELEASE_SPTR( m_hLobbyBGM );
	SAFE_RELEASE_SPTR( m_hTexture );
	FinalizeStage();
}

bool CDnPVPLobbyVillageTask::Initialize( bool bFirstConnect )
{
	//CDnLoadingTask::GetInstance().EnableBackgroundLoading( false );
	//CDnPlayerActor::EnableAnotherPlayers( true );
	m_bFirstConnect = bFirstConnect;

	m_bEntered =  false;

	//blondymarry start
	CDnMouseCursor::GetInstance().SetStaticMouse(true);//Mouse static 
	CDnMouseCursor::GetInstance().ShowCursor( true , true );

	CDnPlayerActor::EnableAnotherPlayers( false );

	m_hCamera = (new CDnInterfaceCamera)->GetMySmartPtr();

	SAFE_RELEASE_SPTR( m_hTexture );
	int previousMapIndex = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
	CDnBridgeTask *pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
	if (pBridgeTask)
		previousMapIndex = pBridgeTask->GetPreviousMapIndexForPvP();

#define _MAPID_CATHEROCK	8
#define _MAPID_SAINTHEAVEN	11
	std::string bgFileName;
	if (previousMapIndex == _MAPID_CATHEROCK)			bgFileName = "Pvp_BG_Cataract.dds";
	else if (previousMapIndex == _MAPID_SAINTHEAVEN)	bgFileName = "Pvp_BG_SaintHaven.dds";
	else
		bgFileName = "Pvp_BG.dds";

	m_hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName(bgFileName.c_str()).c_str() );

	CDnCamera::SetActiveCamera( m_hCamera );

	// Interface Initialize
	GetInterface().Initialize( CDnInterface::PVPVillage, this );

	SendVillageReady( bFirstConnect, false );

	GetInterface().FadeDialog( 0xff000000, 0x00000000, 5.f, this );

	PlayLobbyBGM();

	GetInterface().CloseSlideCaptionMessage();
	GetGameTipTask().ResetGameTip();

	if(GetInterface().GetPvPLadderSystemDlg())
		GetInterface().GetPvPLadderSystemDlg()->SetPVPLadderState(LadderSystem::RoomState::WaitUser);

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	GetQuestTask().LoadQuestTalk();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

	return true;
}

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
void CDnPVPLobbyVillageTask::InitializePreloadPlayer()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );
	for( int i=0; i<10; i++ ) {
		if( m_hPreloadPlayer[i] ) continue;
		CDnPlayerActor *pPlayer;
		CDnPVPLobbyVillageTask::m_hPreloadPlayer[i] = CreateActor( i + 1, false, false, false );
		if( !CDnPVPLobbyVillageTask::m_hPreloadPlayer[i] ) continue;
		pPlayer = (CDnPlayerActor*)CDnPVPLobbyVillageTask::m_hPreloadPlayer[i].GetPointer();

		std::vector<int> nVecList;
		pSox->GetItemIDListFromField( "_BaseClass", i + 1, nVecList );
		pPlayer->SetJobHistory( nVecList );

		pPlayer->RecomposeAction();
	}
	s_bPreloadComplete = true;
}

void CDnPVPLobbyVillageTask::ReleasePreloadPlayer()
{
	for( int i=0; i<10; i++ ) {
		SAFE_RELEASE_SPTR( CDnPVPLobbyVillageTask::m_hPreloadPlayer[i] );
	}
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
	s_bPreloadComplete = false;
}
#endif

void CDnPVPLobbyVillageTask::FinalizeStage()
{
	CDnMouseCursor::GetInstance().SetStaticMouse(false);//Release mouse static 

	GetInterface().Finalize( CDnInterface::PVPVillage );

	CDnActor::ReleaseClass();
	CDnEtcObject::ReleaseClass();

	CDnCamera::ReleaseClass();

	GetCurRenderStack()->EmptyRenderElement();


	SAFE_RELEASE_SPTR( m_hCamera );

	CDnMouseCursor::GetInstance().ShowCursor( false , true );

	StopLobbyBGM();

#if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)
	g_DataManager.ClearTalkData();
#endif	// #if defined(PRE_OPT_CLIENT_QUEST_SCRIPT)

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
	if( GetEtEngine() ) GetEtEngine()->FlushBuffer();
#endif
}

void CDnPVPLobbyVillageTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	//Render floor 
	if( m_bDestroyTask ) return;

	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	GetEtDevice()->BeginScene();

	if( m_hCamera ) 
	{
		EtVector4 Vertices[ 4 ];
		EtVector2 TexCoord[ 4 ];
		int nWidth, nHeight;

		if( m_hTexture ) {
			nWidth = CEtDevice::GetInstance().Width();
			nHeight = CEtDevice::GetInstance().Height();

			float fTemp = ( nWidth - ( nHeight / 9.f ) * 16.f ) / 2.f;
			Vertices[0] = EtVector4( fTemp, 0.f, 0.0f, 0.0f );
			Vertices[1] = EtVector4( nWidth - fTemp, 0.f, 0.0f, 0.0f );
			Vertices[2] = EtVector4( nWidth - fTemp, (float)nHeight, 0.0f, 0.f );
			Vertices[3] = EtVector4( fTemp, (float)nHeight, 0.0f, 0.0f );

			TexCoord[0] = EtVector2( 0.0f, 0.0f );
			TexCoord[1] = EtVector2( 1.0f, 0.0f );
			TexCoord[2] = EtVector2( 1.0f, 1.0f );
			TexCoord[3] = EtVector2( 0.0f, 1.0f );
			EternityEngine::DrawQuad2DWithTex( Vertices, TexCoord, m_hTexture->GetTexturePtr() );
		}
	}

	CEtPrimitiveDraw::GetInstance().Render( m_hCamera );

	GetEtDevice()->EndScene();


	CVillageClientSession::Process( LocalTime, fDelta );
	// Mouse Update
//	CDnMouseCursor::GetInstance().UpdateCursor();

	// Process Input Device
	CInputDevice::GetInstance().Process( LocalTime, fDelta );

	CDnCamera::ProcessClass( LocalTime, fDelta );

	// Process Sound
	if( CEtSoundEngine::IsActive() ) {
		CEtSoundEngine::GetInstance().Process( fDelta );

		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
		if( hCamera ) 
			CEtSoundEngine::GetInstance().SetListener( hCamera->GetMatEx()->m_vPosition, hCamera->GetMatEx()->m_vZAxis, hCamera->GetMatEx()->m_vYAxis );

	}

	GetInterface().PostProcess( LocalTime, fDelta );

	// Process Interface
	GetInterface().Process( LocalTime, fDelta );

	// Note : UI Process
	EtInterface::Process( fDelta );

//	EternityEngine::ClearScreen( 0xFFFFFFFF, 1.0f, 0, false );
	EternityEngine::RenderFrame( fDelta );
	EtInterface::Render( fDelta );
	EternityEngine::ShowFrame( NULL );

}


bool CDnPVPLobbyVillageTask::RenderScreen( LOCAL_TIME LocalTime, float fDelta )
{
	return !CDnPVPLobbyVillageTask::s_bPreloadComplete;
}

void CDnPVPLobbyVillageTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) 
	{
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
	}
}

void CDnPVPLobbyVillageTask::OnRecvCharMapInfo( SCMapInfo *pPacket )
{
	FUNC_LOG();
	CVillageClientSession::OnRecvCharMapInfo( pPacket );
	SendEnter();

	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, 0.f, true );
}

void CDnPVPLobbyVillageTask::OnRecvCharEntered( SCEnter *pPacket )
{
	FUNC_LOG();
	CVillageClientSession::OnRecvCharEntered( pPacket );
	SendCompleteLoading();

	m_sUserInfo = *pPacket; // Remember user info
	m_bEntered = true;

	CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, 0.f, false );
#ifdef PRE_MOD_PVP_LADDER_XP
	GetInterface().GetPVPLobbyChatTabDlg()->RefreshMyPVPInfo();
#endif // PRE_MOD_PVP_LADDER_XP
	//SendPvPRoomList( 0 );

	CDnBridgeTask *pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	
	{
		CDnPVPLadderTabDlg* pPVPLadderTabDlg = GetInterface().GetPVPLadderTabDlg();
		if( pPVPLadderTabDlg )
		{
			pPVPLadderTabDlg->Show(true);

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			// Auto-select tab only when entering lobby from PvP town. Do not select tab when entering a room directly from the lobby window in town.		
			PvPCommon::RoomType::eRoomType eSelectedChannel = PvPCommon::RoomType::regular;
			if( pBridgeTask )
			{
				eSelectedChannel = pBridgeTask->GetSelectedChannelEnterPVPLobbyFromVillageAccessMode();
				pBridgeTask->ResetSelectedChannelEnterPVPLobbyFromVillageAccessMode();
			}

			// If a channel is selected upon entering from village, treat as selecting that channel since entered from village.
			// Sending the packet to the server is fine as it ensures consistency.
			if( PvPCommon::RoomType::max != eSelectedChannel )
			{
				// When packet response arrives, UI is automatically selected.
				SendPVPChangeChannel( eSelectedChannel );
			}
			else
			{
				// Only when not ladder, check own grade and auto-select channel.
				// When entering from village, ladder or not is already decided by selecting tab in village, but
				// When entering the lobby from PvP town, auto-select according to one's rank.
				bool bEnterLadderFromVillageAccessMode = false;
				if( pBridgeTask )
				{
					bEnterLadderFromVillageAccessMode = pBridgeTask->GetEnterLadderFromVillageAccessMode();
					pBridgeTask->ResetEnterLadderFromVillageAccessMode();
				}

				if( false == bEnterLadderFromVillageAccessMode )
				{
					PvPCommon::RoomType::eRoomType eGameStartedGradeChannel = PvPCommon::RoomType::max;
					int iGameStartedLadderChannel = LadderSystem::MatchType::eCode::None;
					pBridgeTask->GetSelectedPVPGradeChannelOrLadder( eGameStartedGradeChannel, iGameStartedLadderChannel );
					pBridgeTask->ResetSelectedPVPGradeChannelOrLadder();

					// Since the UI is always recreated after a match ends, if returning to the lobby after finishing a game,
					// Remember the channel played on and restore it.
					// If none of these cases apply, automatically select a channel according to your PVP rank.
					if( PvPCommon::RoomType::max == eGameStartedGradeChannel && 
						LadderSystem::MatchType::eCode::None == iGameStartedLadderChannel )
					{
						pPVPLadderTabDlg->UpdateAndSelectTabProperly();
					}
					else
					{
						// In ladder mode, a separate ladder enter packet arrives after the ladder match ends, so no need to select the tab.
						// Only need to distinguish Beginner/Normal channels.
						if( PvPCommon::RoomType::max != eGameStartedGradeChannel )
						{
							// When packet response arrives, UI is automatically selected.
							SendPVPChangeChannel( eGameStartedGradeChannel );
						}
					}
				}
				else
				{
					pPVPLadderTabDlg->UpdatePvPChannelTab();
				}
			}
#else	// #ifdef PRE_ADD_PVP_VILLAGE_ACCESS
			pPVPLadderTabDlg->UpdateAndSelectTabProperly();
#endif  // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		}
	}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

}

void CDnPVPLobbyVillageTask::OnRecvStartMsg( SCPVP_STARTMSG * pData )
{
	FUNC_LOG();	

	WCHAR wszMessage[128]=L"";

	//Starts in seconds.
	wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121015 ),pData->cSec);	 

	GetInterface().AddChatMessage(CHATTYPE_SYSTEM,L"",wszMessage,false);

	CDnBaseRoomDlg * pBaseRoomDlg = GetInterface().GetGameRoomDlg();

	if( pBaseRoomDlg )
		pBaseRoomDlg->PlaySound( 0 );

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	if( 1 == pData->cSec )
	{
		// Save the currently selected beginner channel and ladder.
		CDnBridgeTask *pBridgeTask = (CDnBridgeTask *)CTaskManager::GetInstance().GetTask( "BridgeTask" );
		CDnPVPLadderTabDlg* pPVPLadderTabDlg = GetInterface().GetPVPLadderTabDlg();
		if( pBridgeTask && pPVPLadderTabDlg )
		{	
			pBridgeTask->SetSelectedPVPGradeChannelOrLadder( pPVPLadderTabDlg->GetSelectedGradeChannel(), pPVPLadderTabDlg->GetSelectedChannel() );
		}
	}
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
}

void CDnPVPLobbyVillageTask::OnDisconnectTcp( bool bValidDisconnect )
{
	CVillageClientSession::OnDisconnectTcp( bValidDisconnect );
	if( !bValidDisconnect ) {
		WriteLog( 0, ", Error, Disconnect Village Server\n" );
#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
		g_bInvalidDisconnect = true;
#endif
		GetInterface().MessageBox( MESSAGEBOX_10, MB_OK, MESSAGEBOX_10, this );
	}
}

//If the message below arrives, it is a server error 
void CDnPVPLobbyVillageTask::OnRecvCharUserEntered( SCEnterUser *pPacket )
{

}

void CDnPVPLobbyVillageTask::OnRecvCharUserLeave( SCLeaveUser *pPacket )
{

}

#ifdef PRE_PARTY_DB
bool CDnPVPLobbyVillageTask::OnRecvPartyListInfo( SCPartyListInfo *pPacket )
{
	return true;
}
#else
void CDnPVPLobbyVillageTask::OnRecvPartyListInfo( SCPartyListInfo *pPacket )
{

}
#endif

void CDnPVPLobbyVillageTask::OnRecvPartyMemberInfo( SCPartyMemberInfo *pPacket )
{

}

void CDnPVPLobbyVillageTask::OnRecvPartyCreateParty( SCCreateParty *pPacket )
{

}

void CDnPVPLobbyVillageTask::OnRecvPartyRefresh( SCRefreshParty *pPacket )
{

}

void CDnPVPLobbyVillageTask::OnRecvPartyJoinParty( SCJoinParty *pPacket ){

}

void CDnPVPLobbyVillageTask::OnRecvPartyOutParty( SCPartyOut *pPacket ){

}

void CDnPVPLobbyVillageTask::OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket ){

}

void CDnPVPLobbyVillageTask::OnRecvPartyReadyGate( SCGateInfo *pPacket ){

}

void CDnPVPLobbyVillageTask::OnRecvPartyStageCancel( SCCancelStage *pPacket ){

}

void CDnPVPLobbyVillageTask::PlayLobbyBGM()
{
	StopLobbyBGM();
	m_nLobbyBGMIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( "DN_BGM_PvP_Ready.ogg" ).c_str(), false, true );
	if( m_nLobbyBGMIndex == -1 ) return;

	m_hLobbyBGM = CEtSoundEngine::GetInstance().PlaySound__("BGM", m_nLobbyBGMIndex, true, true );
	
	if( m_hLobbyBGM ) {
		m_hLobbyBGM->SetVolume( CEtSoundEngine::GetInstance().GetMasterVolume( "BGM" ) );
		m_hLobbyBGM->Resume();
	}
	
}

void CDnPVPLobbyVillageTask::StopLobbyBGM()
{
	SAFE_RELEASE_SPTR( m_hLobbyBGM );
	if( m_nLobbyBGMIndex > -1 ) {
		CEtSoundEngine::GetInstance().RemoveSound( m_nLobbyBGMIndex );
		m_nLobbyBGMIndex = -1;
	}
}