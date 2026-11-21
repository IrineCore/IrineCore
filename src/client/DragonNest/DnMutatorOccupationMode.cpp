#include "stdafx.h"
#include "DnMutatorOccupationMode.h"
#include "TaskManager.h"
#include "DnGuildZoneMapDlg.h"
#include "DnInterface.h"
#include "DnPVPBaseHud.h"
#include "DnRespawnModeHUD.h"
#include "DnPVPModeEndDlg.h"
#include "DnBridgeTask.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"
#include "DnOccupationTask.h"
#include "Timeset.h"
#include "DnMonsterActor.h"
#include "PvPOccupationScoreSystem.h"
#include "DnCommonUtil.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "DnWorldSound.h"
#include "DnOccupationModeHUD.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

CDnMutatorOccupationMode::CDnMutatorOccupationMode( CDnPvPGameTask * pGameTask )
: CDnMutatorTeamGame( pGameTask )
, m_pOccupationTask( NULL )
, m_nClimaxTime( 0 )
, m_nClimaxRespawnTime( 10 )
, m_bClimaxMode( false )
{
	for( int itr = 0; itr < TARGET_RESOURCE_MAX; ++itr )
	{
		m_nMsgResource[itr] = 0;
		m_nMsgResourceStringID[itr] = 0;
		m_bMsgResource[itr] = false;
	}

	RestartGame();
}

CDnMutatorOccupationMode::~CDnMutatorOccupationMode()
{
	if( m_pOccupationTask )
	{
		m_pOccupationTask->Finalize();
		m_pOccupationTask->DestroyTask( true );
		m_pOccupationTask = NULL;
	}
}

IScoreSystem * CDnMutatorOccupationMode::CreateScoreSystem()
{
	return new CPvPOccupationScoreSystem();
}

bool CDnMutatorOccupationMode::Initialize( const UINT uiItemID, DNTableFileFormat*  pSox )
{

	if( GetInterface().GetHUD())
	{
		GetInterface().GetHUD()->SetRoomName(CDnBridgeTask::GetInstance().GetPVPRoomStatus().wszPVPRoomName.c_str() );

		int iUIstring = 0;
		
		if ( pSox ) 
			iUIstring = pSox->GetFieldFromLablePtr( uiItemID  , "WinCondition_UIString" )->GetInteger();

		WCHAR wszWinCon[256];
		wsprintf(wszWinCon,L"%d%s",CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nObjectiveCount,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iUIstring ));
		GetInterface().GetHUD()->SetWinCondition( wszWinCon  );
	}	

	return InitializeBase( uiItemID, pSox );
}

void CDnMutatorOccupationMode::OnPostInitializeStage()
{
	m_pOccupationTask = new CDnOccupationTask;
	m_pOccupationTask->Initialize( PvPCommon::GameMode::PvP_Occupation );

	CTaskManager::GetInstance().AddTask( m_pOccupationTask, "OccupationTask", -1, false );

	CDnGuildWarZoneMapDlg * pMapDlg = GetInterface().GetGuildWarZoneMapDlg();
	if( !pMapDlg ) return;
	pMapDlg->PostInitialize();
	pMapDlg->Show( true );

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TPVPGAMEMODE );

	int nBattleGroundID = pSox->GetFieldFromLablePtr( CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nGameModeTableID , "_BattleGroundID" )->GetInteger();;

	pSox = GetDNTable( CDnTableDB::TBATTLEGROUNDMODE );
	if( !pSox ) return;

	char szColumName[512];
	for( int itr = 0; itr < TARGET_RESOURCE_MAX; ++itr )
	{
		sprintf_s( szColumName, _countof(szColumName), "_Score_msg%d", itr + 1 );
		m_nMsgResource[itr] = pSox->GetFieldFromLablePtr(nBattleGroundID , szColumName)->GetInteger();

		sprintf_s( szColumName, _countof(szColumName), "_Score_msg%d_UIString", itr + 1 );
		m_nMsgResourceStringID[itr] = pSox->GetFieldFromLablePtr(nBattleGroundID , szColumName)->GetInteger();
	}

	m_nClimaxTime = pSox->GetFieldFromLablePtr(nBattleGroundID , "_ClimaxTime")->GetInteger();
	m_nClimaxRespawnTime = pSox->GetFieldFromLablePtr(nBattleGroundID , "_Cl_Respawn_Time")->GetInteger();

	pSox = GetDNTable( CDnTableDB::TBATTLEGROUNDRESWAR );
	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		int nItemID = pSox->GetItemID(itr);
		int nGameModeTableID = pSox->GetFieldFromLablePtr(nItemID, "_PvPGamemodeId")->GetInteger();

		if( nGameModeTableID == CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nGameModeTableID )
		{
			m_szClimaxBGM = std::string( pSox->GetFieldFromLablePtr( nItemID, "_Cl_BGM" )->GetString() );
			break;
		}
	}
}

void CDnMutatorOccupationMode::GameStart( int nObjectiveUIString, float nRemainCountDown, float RemainSec, bool IsInGameJoin, float nMaxSec )
{
	//Set own team
	if( CDnActor::s_hLocalActor )
		m_nTeam = CDnActor::s_hLocalActor->GetTeam();

	if( CDnOccupationTask::IsActive() )
	{
		GetOccupationTask().SetBreakInfoUser( IsInGameJoin );
		GetOccupationTask().SetScoreSystem( m_pScoreSystem );
	}

	//Set the time and
	GetInterface().GetHUD()->SetTime( nMaxSec - RemainSec , CTimeSet().GetTimeT64_GM() , CTimeSet().GetMilliseconds()); // Time + milliseconds

	CPvPOccupationScoreSystem * pScoreSystem = dynamic_cast<CPvPOccupationScoreSystem *>(m_pScoreSystem);
	pScoreSystem->InitializeOccupationSystem();

	if( IsInGameJoin )
	{
		GetInterface().GetHUD()->StartTimer();
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
		return;
	}

	if( nRemainCountDown )
	{
		m_fCountDownTime = float(nRemainCountDown);
		GetInterface().BeginCountDown( (int)nRemainCountDown  , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringStarting )  );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}
	else
	{
		GetInterface().GetHUD()->StartTimer();
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}
}

bool CDnMutatorOccupationMode::ProcessActorDie( DnActorHandle hKillerActor , DnActorHandle hKilledActor )
{
	CDnMonsterActor *pMonster = NULL;
	CDnPlayerActor *pPlayer = NULL;
	CDnPlayerActor *pOtherPlayer = NULL;

	WCHAR wszMessage[256];

	SecureZeroMemory(wszMessage,sizeof(wszMessage));

	if( CDnActor::s_hLocalActor && hKillerActor == CDnActor::s_hLocalActor ) //When local player is the killer 
	{
		pMonster = dynamic_cast<CDnMonsterActor *>(hKilledActor.GetPointer());

		if( pMonster ) //Victim is a monster
			return true;

		pPlayer = dynamic_cast<CDnPlayerActor *>(hKilledActor.GetPointer());

		if( pPlayer ) //Victim is a player
		{
			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::Kill ),pPlayer->GetName());

			GetInterface().AddMessageText( wszMessage , textcolor::PVP_F_PLAYER );

			// Increment local kill count here.
			TPvPGroup *pPvPInfo = CDnPartyTask::GetInstance().GetPvPInfo();
			int nIndex = pPlayer->GetClassID()-1;
			if( pPvPInfo && nIndex >= 0 && nIndex < PvPCommon::Common::MaxClass )
				pPvPInfo->uiKOClassCount[nIndex] += 1;
			return true;
		}
	}

	pMonster = NULL;
	pPlayer = NULL;

	if( (pPlayer = dynamic_cast<CDnPlayerActor *>(hKillerActor.GetPointer())))//Another player is the killer 
	{
		if( CDnActor::s_hLocalActor && hKilledActor == CDnActor::s_hLocalActor )// Local player is the victim 
		{
			float fRespawnTime = m_pGameTask->GetPlayerDieRespawnDelta();
			if( m_bClimaxMode )
				fRespawnTime = (float)m_nClimaxRespawnTime;

			GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::WaitRespawn ), fRespawnTime  );

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::Killed ),pPlayer->GetName());

			GetInterface().AddMessageText( wszMessage , textcolor::PVP_E_PLAYER );

			// Increment local death count here.
			TPvPGroup *pPvPInfo = CDnPartyTask::GetInstance().GetPvPInfo();
			int nIndex = pPlayer->GetClassID()-1;
			if( pPvPInfo && nIndex >= 0 && nIndex < PvPCommon::Common::MaxClass )
				pPvPInfo->uiKObyClassCount[nIndex] += 1;
			return true;
		}

		if( pOtherPlayer = dynamic_cast<CDnPlayerActor *>(hKilledActor.GetPointer()) )//Another player is the victim 
		{

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::SeeKill ),pPlayer->GetName(),pOtherPlayer->GetName());

			if( m_nTeam ==  pOtherPlayer->GetTeam() )//Victim is on our team, point lost
			{
				GetInterface().AddMessageText( wszMessage , textcolor::PVP_E_PLAYER );

			}else										//Score
			{
				pOtherPlayer->SetExposure( false );
				GetInterface().AddMessageText( wszMessage , textcolor::PVP_F_PLAYER );
			}			

		}		

		pMonster = dynamic_cast<CDnMonsterActor *>(hKilledActor.GetPointer());

		if( pMonster ) //Monster is the victim 
			return true;
	}

	pMonster = NULL;
	pPlayer = NULL;

	if(  (pMonster = dynamic_cast<CDnMonsterActor *>(hKillerActor.GetPointer())) )//Monster is the killer 
	{
		if( CDnActor::s_hLocalActor && hKilledActor == CDnActor::s_hLocalActor )// Local player is the victim 
		{
			float fRespawnTime = m_pGameTask->GetPlayerDieRespawnDelta();
			if( m_bClimaxMode )
				fRespawnTime = (float)m_nClimaxRespawnTime;

			GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::WaitRespawn ), fRespawnTime);
			return true;
		}

		pPlayer = dynamic_cast<CDnPlayerActor *>(hKilledActor.GetPointer());

		if( pPlayer ) //Player is the victim 
			return true;
	}

	WarningLog("Nobody Killed ,Nobody Kill ");
	return false;
}

void CDnMutatorOccupationMode::EndGame( void * pData )
{
	if( m_pOccupationTask )
	{
		int iMyScore		= (m_nTeam == PvPCommon::Team::A) ? m_pOccupationTask->GetResource( PvPCommon::Team::A ) : m_pOccupationTask->GetResource( PvPCommon::Team::B );
		int iEnemyScore		= (m_nTeam == PvPCommon::Team::A) ? m_pOccupationTask->GetResource( PvPCommon::Team::B ) : m_pOccupationTask->GetResource( PvPCommon::Team::A );

		if(m_nTeam == PvPCommon::Team::Observer)
		{
			iMyScore = m_pOccupationTask->GetResource( PvPCommon::Team::A );
			iEnemyScore = m_pOccupationTask->GetResource( PvPCommon::Team::B );
		}

		if( GetInterface().GetHUD() )
		{
			GetInterface().GetHUD()->SetMyScore( iMyScore );
			GetInterface().GetHUD()->SetEnemyScore( iEnemyScore );
		}
	}

	SCPVP_FINISH_PVPMODE * pPacket = NULL;	

	pPacket = ( SCPVP_FINISH_PVPMODE * )pData;

	GetInterface().GetHUD()->HaltTimer();

	if( CDnPartyTask::IsActive() )
	{
		for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) 
		{
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;
			if( !pStruct->hActor ) continue;
			if( pStruct->hActor) 
			{
				CPvPOccupationScoreSystem * pScoreSystem = dynamic_cast<CPvPOccupationScoreSystem *>(m_pScoreSystem);

				GetInterface().SetPVPGameScore( pStruct->hActor->GetUniqueID(),
					m_pScoreSystem->GetKillCount(pStruct->hActor),
					m_pScoreSystem->GetDeathCount(pStruct->hActor),
					0,                                                // Send 0 because there is no captain
					m_pScoreSystem->GetKillScore(pStruct->hActor),
					m_pScoreSystem->GetAssistScore(pStruct->hActor),
					pScoreSystem->GetMyOccupationScore(pStruct->hActor->GetName()) );

				GetInterface().SetPvPGameOccupationScore( pStruct->hActor->GetUniqueID(),
					m_pScoreSystem->GetOccupationAcquireScore( pStruct->hActor ),
					m_pScoreSystem->GetOccupationStealScore( pStruct->hActor ) );
			}
		}
	}

	GetInterface().OpenFinalResultDlg();

	byte cResult = PVPResult::RoundEND;


#ifdef PRE_MOD_PVPOBSERVER
	if( CDnActor::s_hLocalActor )
	{
		//--------------------------------------------------------
		// Handle UI when LocalPlayer is Observer and not Draw.
		//
		if( CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer && 
			pPacket->uiWinTeam != PvPCommon::Team::Max ) 
		{
			cResult = PVPResult::Win;
			GetInterface().GetRespawnModeEnd()->SetResultStringObServer( pPacket->uiWinTeam, 
				GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringVictory ) ); // "Victory!"	
		}

		//--------------------------------------------------------
		// For PVP participating players, as before.
		else
		{
			if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory 
			{
				if( PvPCommon::Team::A == CDnActor::s_hLocalActor->GetTeam() )
				{
					cResult = PVPResult::Win;
					if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossVictory ) );
					else
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeVictory ) );
				}
				else
				{
					cResult = PVPResult::Lose;
					if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossDefeated ) );
					else
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeDefeated ) );
				}

				//return;
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
			{
				if( PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
				{
					cResult = PVPResult::Win;
					if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossVictory ) );
					else
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeVictory ) );
				}
				else
				{
					cResult = PVPResult::Lose;
					if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossDefeated ) );
					else
						GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeDefeated ) );
				}

				//return;
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
			{
				cResult = PVPResult::Draw;
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringDraw ) );
				//return;
			}
		}
	}

#else

	if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory 
	{
		if( CDnActor::s_hLocalActor && PvPCommon::Team::A == CDnActor::s_hLocalActor->GetTeam() || CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
		{
			cResult = PVPResult::Win;
			if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossVictory ) );
			else
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeVictory ) );
		}
		else
		{
			cResult = PVPResult::Lose;
			if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossDefeated ) );
			else
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeDefeated ) );
		}

		//return;
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
	{
		if( CDnActor::s_hLocalActor && PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
		{
			cResult = PVPResult::Win;
			if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossVictory ) );
			else
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeVictory ) );
		}
		else
		{
			cResult = PVPResult::Lose;
			if( PvPCommon::FinishReason::OpponentTeamBossDead == pPacket->Reason )
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringBossDefeated ) );
			else
				GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringTimeDefeated ) );
		}

		//return;
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
	{
		cResult = PVPResult::Draw;
		GetInterface().GetRespawnModeEnd()->SetResultString4(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, OccupationModeString::UIStringDraw ) );
		//return;
	}
#endif // PRE_MOD_PVPOBSERVER

	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Occupation  , true ,cResult, true );
}

void CDnMutatorOccupationMode::_RenderGameModeScore()
{
	if( !m_pOccupationTask ) return;

	int iMyScore		= (m_nTeam == PvPCommon::Team::A) ? m_pOccupationTask->GetResource( PvPCommon::Team::A ) : m_pOccupationTask->GetResource( PvPCommon::Team::B );
	int iEnemyScore		= (m_nTeam == PvPCommon::Team::A) ? m_pOccupationTask->GetResource( PvPCommon::Team::B ) : m_pOccupationTask->GetResource( PvPCommon::Team::A );

	if(m_nTeam == PvPCommon::Team::Observer)
	{
		iMyScore = m_pOccupationTask->GetResource( PvPCommon::Team::A );
		iEnemyScore = m_pOccupationTask->GetResource( PvPCommon::Team::B );
	}

	if( GetInterface().GetHUD() )
	{
		GetInterface().GetHUD()->SetMyScore( iMyScore );
		GetInterface().GetHUD()->SetEnemyScore( iEnemyScore );
	}

	if( CDnPartyTask::IsActive() )
	{
		for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) 
		{
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;
			if( !pStruct->hActor ) continue;
			if( pStruct->hActor) 
			{
				CPvPOccupationScoreSystem * pScoreSystem = dynamic_cast<CPvPOccupationScoreSystem *>(m_pScoreSystem);

				GetInterface().SetPVPGameScore( pStruct->hActor->GetUniqueID(),
					m_pScoreSystem->GetKillCount(pStruct->hActor),
					m_pScoreSystem->GetDeathCount(pStruct->hActor),
					0,                                                // Send 0 because there is no captain
					m_pScoreSystem->GetKillScore(pStruct->hActor),
					m_pScoreSystem->GetAssistScore(pStruct->hActor),
					pScoreSystem->GetMyOccupationScore(pStruct->hActor->GetName()) );

				GetInterface().SetPvPGameOccupationScore( pStruct->hActor->GetUniqueID(),
					m_pScoreSystem->GetOccupationAcquireScore( pStruct->hActor ),
					m_pScoreSystem->GetOccupationStealScore( pStruct->hActor ) );
			}
		}
	}
}

void CDnMutatorOccupationMode::RenderCustom( float fElapsedTime )
{
	_RenderOccupationZone();
}

void CDnMutatorOccupationMode::_RenderOccupationZone()
{
	if( !m_pOccupationTask ) return;

	EtCameraHandle hEtCam = CEtCamera::GetActiveCamera();
	if( !hEtCam ) return;

	EtTextureHandle hTex = CDnInterface::GetInstance().GetPointMarkTex();
	EtTextureHandle hTailTex = CDnInterface::GetInstance().GetPointMarkTailTex();
	if( !hTex || !hTailTex ) return;

	std::vector<CDnOccupationZone *> vecOccupationZone = m_pOccupationTask->GetFlagVector();

	if( hEtCam->GetPosition() && hEtCam->GetDirection() )
	{
		SUICoord screenUICoord, UVCoord;
		EtVector3 vScreenPos, vTemp;
		EtVector3 vCurlook, vLookFromMember, vCross;
		int nCountX = 4;
		EtVector2 vSize(128.f, 128.f);
		float fTextureDelta = 16.f / (float)GetEtDevice()->Width();	// Adjusting size for texture rotation added extra margin, so compensate with this value to fit the screen

		for( int itr = 0; itr < (int)vecOccupationZone.size(); ++itr )
		{
			vTemp = vecOccupationZone[itr]->GetPropPosition();
			vTemp.y += 8.f;

			bool bBackOfCam = false;

			vCurlook = *hEtCam->GetDirection();
			EtVec3Normalize(&vLookFromMember, &(vTemp - *hEtCam->GetPosition()));
			vLookFromMember.y = vCurlook.y;
			if ( EtVec3Dot(&vCurlook, &vLookFromMember) < 0.f)
				bBackOfCam = true;

			screenUICoord.SetSize(vSize.x / (float)GetEtDevice()->Width(), vSize.y / (float)GetEtDevice()->Height());

			if (bBackOfCam)
			{
				screenUICoord.fY = 1.f - screenUICoord.fHeight * 2.f;
				EtVec3Cross( &vCross, &vCurlook, &(EtVector3(0.f, 1.f, 0.f)) );
				if( vCross.y > 0.0f )
					screenUICoord.fX = 1.f - screenUICoord.fWidth;
				else
					screenUICoord.fX = 0.f;
			}
			else
			{
				EtViewPort vp;
				EtMatrix* matViewProj;
				GetEtDevice()->GetViewport(&vp);
				matViewProj = hEtCam->GetViewProjMatForCull();
				EtVec3TransformCoord( &vScreenPos, &vTemp, matViewProj );

				screenUICoord.fX = (( vScreenPos.x + 1.0f ) / 2.0f + vp.X);
				screenUICoord.fY = (( 2.0f - ( vScreenPos.y + 1.35f ) ) / 2.0f + vp.Y);

				CommonUtil::ClipNumber(screenUICoord.fX, screenUICoord.fWidth * 0.5f, 1.f - screenUICoord.fWidth * 0.5f);
				CommonUtil::ClipNumber(screenUICoord.fY, screenUICoord.fHeight * 0.5f, 1.f - screenUICoord.fHeight * 0.5f);

				screenUICoord.fX -= screenUICoord.fWidth / 2.f;
				float fEndOfY = 1.f - screenUICoord.fHeight * 2.f;
				if (screenUICoord.fY > fEndOfY)
					screenUICoord.fY = fEndOfY;
			}

			float fDegree = 0.f;
			fDegree = EtToDegree( atan2f( -(screenUICoord.fX - 0.5f), screenUICoord.fY - 0.5f ) );

			int nIndex = vecOccupationZone[itr]->GetImageIndex();
			DWORD dwColor = vecOccupationZone[itr]->GetImageColor();
			UVCoord = SUICoord((vSize.x * (float)(nIndex % nCountX)) / hTex->Width(), (vSize.y * (float)(nIndex / nCountX)) / hTex->Height(), vSize.x / hTex->Width(), vSize.y / hTex->Height());
			SUICoord tailUVCoord = SUICoord( 0.f, 0.f, 1.f, 1.f );

			CEtSprite::GetInstance().Begin(0);
			CEtSprite::GetInstance().DrawSprite((EtTexture*)hTex->GetTexturePtr(), hTex->Width(), hTex->Height(), UVCoord, dwColor, screenUICoord, 0.f);
			CEtSprite::GetInstance().DrawSprite((EtTexture*)hTailTex->GetTexturePtr(), hTailTex->Width(), hTailTex->Height(), tailUVCoord, dwColor, screenUICoord, fDegree);
			CEtSprite::GetInstance().End();
		}
	}
}

void CDnMutatorOccupationMode::ClimaxModeStart()
{
	m_bClimaxMode = true;

	CDnWorldGrid *pGrid = (CDnWorldGrid *)CDnWorld::GetInstance().GetGrid();
	if( !pGrid ) return;

	for( DWORD i=0; i<pGrid->GetActiveSectorCount(); i++ ) {
		CDnWorldSector *pSector = (CDnWorldSector *)pGrid->GetActiveSector(i);
		if( !pSector ) continue;
		CDnWorldSound *pSound = (CDnWorldSound *)pSector->GetSoundInfo();
		if( !pSound ) continue;
		pSound->ChangeBGM( m_szClimaxBGM.c_str() );
	}

	if( !GetInterface().GetHUD() ) 
		return;

	CDnOccupationModeHUD * pHud = dynamic_cast<CDnOccupationModeHUD * >( GetInterface().GetHUD() );
	pHud->StartClimaxMode( true );
}

void CDnMutatorOccupationMode::CheckResource()
{
	for( int itr = 0; itr < TARGET_RESOURCE_MAX; ++itr )
	{
		if( m_bMsgResource[itr] ) continue;

		if( m_pOccupationTask->GetResource( PvPCommon::Team::A ) >= m_nMsgResource[itr] ||
			m_pOccupationTask->GetResource( PvPCommon::Team::B ) >= m_nMsgResource[itr]
		)
		{
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, m_nMsgResourceStringID[itr] ), true );
			m_bMsgResource[itr] = true;
			break;
		}
	}
}

bool CDnMutatorOccupationMode::IsClimaxMode()
{
	return m_bClimaxMode;
}