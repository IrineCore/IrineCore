#include "StdAfx.h"
#include "DnDLGameTask.h"
#include "DnInterface.h"
#include "TaskManager.h"
#include "PartySendPacket.h"
#include "DnTableDB.h"
#include "DnDarklairClearImp.h"
#include "DnDarkLairClearRewardDlg.h"
#include "DnDarklairDungeonEnterDlg.h"
#include "DnDarklairRequestChallengeDlg.h"
#include "DnLocalPlayerActor.h"
#ifdef PRE_MOD_DARKLAIR_RECONNECT
#include "DnDarklairInfoDlg.h"
#endif // PRE_MOD_DARKLAIR_RECONNECT
#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
#include "EtActionCoreMng.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnDLGameTask::CDnDLGameTask()
{
	m_GameTaskType = GameTaskType::DarkLair;
	m_nCurrentRound = 0;
	m_nTotalRound = 0;
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	m_bChallengeDarkLair = false;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )

	m_pDnDarklairRequestChallengeDlg = new CDnDarklairRequestChallengeDlg( UI_TYPE_MODAL );
	m_pDnDarklairRequestChallengeDlg->Initialize( false );

#ifdef PRE_MOD_DARKLAIR_RECONNECT
	m_bBossRound = false;
#endif // PRE_MOD_DARKLAIR_RECONNECT
}

CDnDLGameTask::~CDnDLGameTask()
{
}

void CDnDLGameTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnGameTask::Process( LocalTime, fDelta );
}

bool CDnDLGameTask::OnDungeonReadyGate( CDnWorld::DungeonGateStruct *pStruct )
{
	GetInterface().OpenDarkLairEnterDialog( (CDnWorld::DungeonGateStruct*)pStruct, this );
	return false;
}

#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
void CDnDLGameTask::PreGenerationMonster( int nRandomSeed )
{
	return;
}
#endif

void CDnDLGameTask::ResetRound( bool bForceReset /*= false*/ )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDLMAP );
	if( !pSox ) return;
	if( !pSox->IsExistItem( m_nMapTableID ) ) return;

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	if( false == m_bChallengeDarkLair || true == bForceReset )
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	{
		m_nCurrentRound = pSox->GetFieldFromLablePtr( m_nMapTableID, "_StartRound" )->GetInteger();
		m_nTotalRound = pSox->GetFieldFromLablePtr( m_nMapTableID, "_StageCount" )->GetInteger() + m_nCurrentRound;
	}
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	m_bChallengeDarkLair = pSox->GetFieldFromLablePtr( m_nMapTableID, "_ChallengeType" )->GetInteger() == 0 ? false : true;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
}

void CDnDLGameTask::UpdateRound( bool bBoss )
{
	if (m_bEnableDungeonClearLeaderWarp)
		return;

	bool bFinal = false;
	m_nCurrentRound++;

#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
	if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
	CEtResource::FlushWaitDelete();
#else
	//	In darklair, monsters keep spawning without changing stages, so resource flush does not happen for a long time.
	//	Force a resource flush on each round in case low-spec users fail to allocate memory addresses by kalliste
	CEtResource::FlushWaitDeleteByCount(10);
#endif

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	if( false == m_bChallengeDarkLair )
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	{
		if( m_nCurrentRound >= m_nTotalRound )
		{
			m_nCurrentRound = m_nTotalRound;
			bFinal = true;
		}
	}
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	m_bBossRound = bBoss;
#endif // PRE_MOD_DARKLAIR_RECONNECT
	// Need to display UI.
	GetInterface().OpenDarkLariRoundCountDialog( m_nCurrentRound, bBoss, bFinal );
}

void CDnDLGameTask::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	switch( nID ) {
		case DAKRLAIR_ENTER_DIALOG:
			{
				if( nCommand == EVENT_BUTTON_CLICKED ) 
				{
					if( strcmp( pControl->GetControlName(), "ID_BUTTON_ENTER" ) == 0 )
					{
#ifdef PRE_MOD_MISSION_HELPER
						GetMissionTask().AutoRegisterMissionNotifier();
#endif
						CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance().GetTask( "PartyTask" );
						if( pTask->GetPartyRole() == CDnPartyTask::LEADER || pTask->GetPartyRole() == CDnPartyTask::SINGLE )
						{
							int nSelectDungeonIndex = -1;
							if( GetInterface().GetDarklairDungeonEnterDlg() )
								nSelectDungeonIndex = GetInterface().GetDarklairDungeonEnterDlg()->GetFloorIndex();
							m_nStageConstructionLevel = 4;

							SendStartStage( CONVERT_TO_DUNGEONDIFFICULTY(m_nStageConstructionLevel), (char)nSelectDungeonIndex );
						}
					}
					else if( strcmp( pControl->GetControlName(), "ID_BUTTON_CANCEL" ) == 0 )
					{
						SendCancelStage();
					}
				}
			}
			break;

		case MSGBOX_REQUEST_CHALLENGE:
			{
				if( nCommand == EVENT_BUTTON_CLICKED )
				{
					if( strcmp( pControl->GetControlName(), "ID_YES" ) == 0 )
					{
						CSDLChallengeResponse Packet;
						memset( &Packet, 0, sizeof(CSDLChallengeResponse) );
						Packet.bAccept = true;

						CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_DLCHALLENGE_RESPONSE, (char*)&Packet, sizeof(CSDLChallengeResponse) );
					}
					else if( strcmp( pControl->GetControlName(), "ID_NO" ) == 0 )
					{
						CSDLChallengeResponse Packet;
						memset( &Packet, 0, sizeof(CSDLChallengeResponse) );
						Packet.bAccept = false;

						CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_DLCHALLENGE_RESPONSE, (char*)&Packet, sizeof(CSDLChallengeResponse) );
					}
				}
			}
			break;
	}
	CDnGameTask::OnUICallbackProc( nID, nCommand, pControl, uiMsg );
}

bool CDnDLGameTask::InitializeStage( int nCurrentMapIndex, int nGateIndex, int nStageConstructionLevel, int nRandomSeed, bool bContinueStage, bool bDirectConnect )
{
	CDnWorld::MapTypeEnum PrevMapType = CDnWorld::GetInstance().GetMapType();

	bool bResult = CDnGameTask::InitializeStage( nCurrentMapIndex, nGateIndex, nStageConstructionLevel, nRandomSeed, bContinueStage, bDirectConnect );
	if( !bResult ) return false;

	CDnWorld::MapTypeEnum CurMapType = CDnWorld::GetInstance().GetMapType();

	switch( CurMapType ) {
		case CDnWorld::MapTypeDungeon:
			switch( PrevMapType ) {
				case CDnWorld::MapTypeDungeon:
					ResetRound();
					break;
				case CDnWorld::MapTypeWorldMap:
					ResetRound( true );
					break;
#ifdef PRE_MOD_DARKLAIR_RECONNECT
				case CDnWorld::MapTypeVillage:
					{
						bool bFinal = false;
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
						if( false == m_bChallengeDarkLair )
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
						{
							if( m_nCurrentRound >= m_nTotalRound )
								bFinal = true;
						}
						if( GetInterface().GetDarklairInfoDlg() )
							GetInterface().GetDarklairInfoDlg()->RefreshRound( m_nCurrentRound, m_bBossRound, bFinal );
					}
					break;
#endif // PRE_MOD_DARKLAIR_RECONNECT
			}
			break;
		case CDnWorld::MapTypeWorldMap:
			ResetRound();
			break;
	}

	return true;
}

bool CDnDLGameTask::InitializeNextStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex )
{

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TDLMAP );
	if( pSox && pSox->IsExistItem( nMapTableID ) )
	{
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
		m_bChallengeDarkLair = pSox->GetFieldFromLablePtr( nMapTableID, "_ChallengeType" )->GetInteger() == 0 ? false : true;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	}
#if defined( PRE_FIX_73930 ) && defined( PRE_ADD_CHALLENGE_DARKLAIR )
	CDnWorld::MapTypeEnum CurMapType = CDnWorld::GetInstance().GetMapType();
	if( CurMapType == CDnWorld::MapTypeWorldMap )
		m_bChallengeDarkLair = false;
#endif	// #if defined( PRE_FIX_73930 ) && defined( PRE_ADD_CHALLENGE_DARKLAIR )

	return CDnGameTask::InitializeNextStage( szGridName, szEnviName, nMapTableID, nStageConstructionLevel, nStartPositionIndex );
}

void CDnDLGameTask::OnRecvRoomDLDungeonClearMsg( SCDLDungeonClear *pPacket )
{
	SAFE_DELETE( m_pStageClearImp );
	m_pStageClearImp = new CDnDarklairClearImp( this );
	m_pStageClearImp->OnCalcClearProcess( (void*)pPacket );
}

void CDnDLGameTask::OnRecvRoomDLDungeonClearRankInfo( SCDLRankInfo *pPacket )
{
	if( m_pStageClearImp ) {
		((CDnDarklairClearImp*)m_pStageClearImp)->OnRecvRoomDLDungeonClearRankInfo( pPacket );
	}
}

void CDnDLGameTask::OnRecvRoomDungeonClearSelectRewardItem( char *pPacket )
{
	GetInterface().OpenDarkLairClearRewardItemDialog();
}

void CDnDLGameTask::OnRecvRoomSelectRewardItem( SCSelectRewardItem *pPacket )
{
	CDnDarklairClearRewardDlg *pDlg = GetInterface().GetDarkLairClearRewardDialog();
	if( pDlg ) pDlg->SelectBonusBox( pPacket->nSessionID, pPacket->cItemIndex );
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDnDLGameTask::OnRecvRoomDungeonClearRewardBoxType(SCDungeonClearBoxType* pData)
{
	if( m_pStageClearImp ) 
		m_pStageClearImp->OnCalcClearRewardBoxProcess( (void*)pData );

	CDnDarklairClearRewardDlg *pDlg = GetInterface().GetDarkLairClearRewardDialog();
	if( pDlg )
	{
		pDlg->ShowBonusBox();

#if defined( PRE_SKIP_REWARDBOX )
		pDlg->ResetCount();
#endif	// #if defined( PRE_SKIP_REWARDBOX )
	}
}
#endif

void CDnDLGameTask::OnRecvRoomDungeonClearRewardItem( SCDungeonClearRewardItem *pPacket )
{
	if( m_pStageClearImp ) 
		m_pStageClearImp->OnCalcClearRewardItemProcess( (void*)pPacket );

	CDnDarklairClearRewardDlg *pDlg = GetInterface().GetDarkLairClearRewardDialog();
	if( pDlg ) pDlg->OpenBonusBox();
}

void CDnDLGameTask::OnRecvRoomDungeonClearWarpFailed(SCDungeonClearWarpOutFail* pPacket)
{
	if (pPacket->nRet != ERROR_NONE)
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnDLGameTask::OnRecvRoomDungeonClearRewardItemResult( SCDungeonClearRewardItemResult *pPacket )
{
	CDnDarklairClearRewardDlg *pDlg = GetInterface().GetDarkLairClearRewardDialog();
	if( pDlg ) {
		for( int i=0; i<pPacket->cCount; i++ ) {
			pDlg->SelectBonusBox( pPacket->Info[i].nSessionID, pPacket->Info[i].cIndex );
		}
		pDlg->OpenBonusItem();
	}
}

void CDnDLGameTask::OnGhost( DnActorHandle hActor )
{
	if( !hActor ) return;
	if (m_bEnableDungeonClearLeaderWarp)
		return;

	CDnPlayerActor *pActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
	if( pActor->IsLocalActor() ) 
	{
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(hActor.GetPointer());
		if( pLocalActor->IsGhost() ) 
		{
			if( pLocalActor->GetCurrentGhostType() == Ghost::Type::PartyRestore || CDnItemTask::GetInstance().IsCanRebirth() ) 
			{
				GetInterface().OpenRebirthCaptionDlg();
				GetInterface().RefreshRebirthInterface();
			}
		}
		else
		{
			GetInterface().CloseRebirthCaptionDlg();
		}
	}
}

void CDnDLGameTask::OnRecvRoomWarpDungeonClear( SCWarpDungeonClear *pPacket )
{
	bool bLeader = (CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::LEADER || CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::SINGLE);
	CDnPartyTask::GetInstance().SetPartyState(CDnPartyTask::STAGE_CLEAR_WARP_STANDBY);
	
#ifdef PRE_ADD_RETURN_VILLAGE_SHOWNAME
	GetInterface().SetReturnVillage( pPacket->nLastVillageMapIndex );
#endif 
	GetInterface().CloseDarkLairClearRewardItemDialog();
}

void CDnDLGameTask::OnRecvRoomIdentifyRewardItem( SCIdentifyRewardItem *pPacket )
{
	CDnDarklairClearRewardDlg *pDlg = GetInterface().GetDarkLairClearRewardDialog();
	if( !pDlg ) return;

	pDlg->OpenBonusBox( pPacket->cItemIndex, pPacket->cBoxType );
}

void CDnDLGameTask::OnRecvDLChallengeRequest()
{
	int nCurrentMapIndex = GetGameMapIndex();

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TMAP );

	if( pSox == NULL )
		return;

	int nNextMapIndex = pSox->GetFieldFromLablePtr( nCurrentMapIndex, "_Gate2_MapIndex" )->GetInteger();

	std::wstring wszString = wstring( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 829 ) );
	std::wstring::size_type nPosInStr = wszString.find( L"\\n" );

	WCHAR wszMessage[256];
	WCHAR pString[256];
	std::wstring wszFirst = wszString.substr( 0, nPosInStr );
	std::wstring wszSecond = wszString.substr( nPosInStr + 2 );

	wsprintf( pString, wszFirst.c_str(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nNextMapIndex, "_MapNameID" )->GetInteger() ) );
	swprintf_s( wszMessage, _countof(wszMessage), L"%s\n%s", pString, wszSecond.c_str() );

	ShowRequestChallengeDlg( wszMessage );
}

void CDnDLGameTask::OnRecvDLChallengeResponse( SCDLChallengeResponse *pPacket )
{
	if( m_pDnDarklairRequestChallengeDlg )
		m_pDnDarklairRequestChallengeDlg->Show( false );
}

void CDnDLGameTask::ShowRequestChallengeDlg( LPCWSTR pwszMessage )
{
	if( m_pDnDarklairRequestChallengeDlg )
	{
		m_pDnDarklairRequestChallengeDlg->SetMessageBox( CDnInterface::DLGame, pwszMessage, MB_YESNO, MSGBOX_REQUEST_CHALLENGE, this );
		m_pDnDarklairRequestChallengeDlg->Show( true );

		CDnPartyTask::PartyRoleEnum partyState = GetPartyTask().GetPartyRole();
		bool bEnableButton = ( partyState == CDnPartyTask::MEMBER ) ? false : true;

		EnableRequestChallengeDlg(bEnableButton);
	}
}

void CDnDLGameTask::EnableRequestChallengeDlg(bool bEnable)
{
	if (m_pDnDarklairRequestChallengeDlg && m_pDnDarklairRequestChallengeDlg->IsShow())
		m_pDnDarklairRequestChallengeDlg->EnableButton(bEnable);
}

#ifdef PRE_MOD_DARKLAIR_RECONNECT
void CDnDLGameTask::OnRecvDarkLairRoundInfo( SCDLRoundInfo* pData )
{
	m_nCurrentRound = pData->nCurrentRound;
	m_nTotalRound = pData->nTotalRound;	
	m_bBossRound = pData->bBossRound;
}
#endif // PRE_MOD_DARKLAIR_RECONNECT