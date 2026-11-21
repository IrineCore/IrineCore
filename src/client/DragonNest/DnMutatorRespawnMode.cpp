#include "StdAfx.h"
#include "DnMutatorRespawnMode.h"
#include "DnMonsterActor.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "DnRespawnModeHUD.h"
#include "DnPVPModeEndDlg.h"
#include "PvPScoreSystem.h"
#include "DnMutatorGame.h"
#include "DnCaptionDlg_04.h"
#include "DnPVPGameResultAllDlg.h"
#include "Timeset.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

CDnMutatorRespawnMode::CDnMutatorRespawnMode( CDnPvPGameTask * pGameTask ):CDnMutatorTeamGame( pGameTask )
{
	RestartGame();
}

CDnMutatorRespawnMode::~CDnMutatorRespawnMode()
{
	
}

IScoreSystem* CDnMutatorRespawnMode::CreateScoreSystem()
{
	return new IBoostPoolPvPScoreSystem();
}


bool CDnMutatorRespawnMode::Initialize( const UINT uiItemID, DNTableFileFormat*  pSox )
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

void CDnMutatorRespawnMode::GameStart( int nObjectiveUIString , float nRemainCountDown , float RemainSec , bool IsInGameJoin , float nMaxSec)
{

	//Set own team
	if( CDnActor::s_hLocalActor )
	{
		m_nTeam = CDnActor::s_hLocalActor->GetTeam();
	}

	//Set the time and
	GetInterface().GetHUD()->SetTime( nMaxSec - RemainSec , CTimeSet().GetTimeT64_GM() , CTimeSet().GetMilliseconds()); // Time + milliseconds

	if( IsInGameJoin )
	{
		GetInterface().GetHUD()->StartTimer();
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
		return;
	}

	if( nRemainCountDown )
	{
		m_fCountDownTime = float(nRemainCountDown);
		GetInterface().BeginCountDown( (int)nRemainCountDown  , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringStarting )  );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}
	else
	{
		GetInterface().GetHUD()->StartTimer();
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}

#ifdef PRE_MOD_PVPOBSERVER
	CDnRespawnModeHUD * pHud = dynamic_cast<CDnRespawnModeHUD * >( GetInterface().GetHUD() );
	if( pHud )
		pHud->GameStart();
#endif // PRE_MOD_PVPOBSERVER


}


bool CDnMutatorRespawnMode::ProcessActorDie( DnActorHandle hKillerActor, DnActorHandle hKilledActor )
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
		{
			return true;
		}		

		pPlayer = dynamic_cast<CDnPlayerActor *>(hKilledActor.GetPointer());

		if( pPlayer ) //Victim is a player
		{
			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::Kill ),pPlayer->GetName());

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
			GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::WaitRespawn ), m_pGameTask->GetPlayerDieRespawnDelta() );

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::Killed ),pPlayer->GetName());

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

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::SeeKill ),pPlayer->GetName(),pOtherPlayer->GetName());

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
		{
			return true;
		}

	}

	pMonster = NULL;
	pPlayer = NULL;

	if(  (pMonster = dynamic_cast<CDnMonsterActor *>(hKillerActor.GetPointer())) )//Monster is the killer 
	{
		if( CDnActor::s_hLocalActor && hKilledActor == CDnActor::s_hLocalActor )// Local player is the victim 
		{
			GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::WaitRespawn ),m_pGameTask->GetPlayerDieRespawnDelta());
			return true;
		}

		pPlayer = dynamic_cast<CDnPlayerActor *>(hKilledActor.GetPointer());

		if( pPlayer ) //Player is the victim 
		{
			return true;
		}

	}

	WarningLog("Nobody Killed ,Nobody Kill ");
	return false;
}

void CDnMutatorRespawnMode::EndGame( void * pData )
{

	CPvPScoreSystem::SMyScore * sInfo = NULL;
	sInfo = (CPvPScoreSystem::SMyScore*)m_pScoreSystem->GetScoreStruct(CDnActor::s_hLocalActor->GetName());

	SCPVP_FINISH_PVPMODE * pPacket = NULL;	

	pPacket = ( SCPVP_FINISH_PVPMODE * )pData;

	GetInterface().GetHUD()->HaltTimer();
	GetInterface().OpenFinalResultDlg();

	byte cResult = PVPResult::Win;

	if(IsIndividualMode())
	{

		WCHAR wzStrName[128];
		SecureZeroMemory(wzStrName,sizeof(wzStrName));

		
		if(pPacket->uiWinTeam == PvPCommon::Team::Max) // When a draw message is received
		{
			cResult = PVPResult::Draw;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringTimeOver ) );
			GetInterface().GetCaptionDlg_04()->SetCaption(GetEtUIXML().GetUIString(CEtUIXML::idCategory1,RespawnModeString::UIStringIndividualDraw),textcolor::YELLOW,3.f,GetInterface().GetCaptionDlg_04()->SecondCaption); 
			GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Respawn  , true ,cResult, true );
			return;
		}

		if(CDnActor::s_hLocalActor && GetInterface().GetPVPGameResultAllDlg()->GetBestUserName() == CDnActor::s_hLocalActor->GetName())
			cResult = PVPResult::Win;   
		else
			cResult = PVPResult::Lose;  
		
		wsprintf(wzStrName,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringIndividualVictory ),GetInterface().GetPVPGameResultAllDlg()->GetBestUserName().c_str());
		GetInterface().GetRespawnModeEnd()->SetResultString2(wzStrName);
		
		GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Respawn  , true ,cResult, true );
		return;
	}

#ifdef PRE_MOD_PVPOBSERVER

	if( CDnActor::s_hLocalActor )
	{
		//--------------------------------------------------------
		// Handle UI when LocalPlayer is Observer and not Draw.
		//
		if( CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer && 
			pPacket->uiWinTeam != PvPCommon::Team::Max ) 
		{
			GetInterface().GetRespawnModeEnd()->SetResultStringObServer( pPacket->uiWinTeam, 
				GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringVictory ) ); // "Victory!"	
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
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringVictory ) );
				}
				else
				{
					cResult = PVPResult::Lose;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDefeated ) );
				}
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
			{
				if( PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
				{
					cResult = PVPResult::Win;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringVictory ) );			
				}
				else
				{
					cResult = PVPResult::Lose;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDefeated ) );
				}
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
			{
				cResult = PVPResult::Draw;
				GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDraw ) );
			}

		}

	}

#else	
	if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory 
	{
		if( CDnActor::s_hLocalActor &&   PvPCommon::Team::A == CDnActor::s_hLocalActor->GetTeam() || CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
		{
			cResult = PVPResult::Win;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringVictory ) );
		}
		else
		{
			cResult = PVPResult::Lose;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDefeated ) );
		}
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
	{
		if( CDnActor::s_hLocalActor &&   PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
		{
			cResult = PVPResult::Win;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringVictory ) );			
		}
		else
		{
			cResult = PVPResult::Lose;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDefeated ) );
		}
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
	{
		cResult = PVPResult::Draw;
		GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::UIStringDraw ) );
	}
#endif // PRE_MOD_PVPOBSERVER

	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Respawn  , true ,cResult, true );
	
}

void CDnMutatorRespawnMode::_RenderGameModeScore()
{
	if( !m_pScoreSystem || m_nTeam ==0)
		return;

	UINT uiATeamScore, uiBTeamScore;
	m_pScoreSystem->GetGameModeScore( uiATeamScore, uiBTeamScore );


	int iMyScore		= (m_nTeam == PvPCommon::Team::A) ? uiATeamScore : uiBTeamScore;
	int iEnemyScore		= (m_nTeam == PvPCommon::Team::A) ? uiBTeamScore : uiATeamScore;


	if(m_nTeam == PvPCommon::Team::Observer)
	{
		iMyScore = uiATeamScore;
		iEnemyScore = uiBTeamScore;
	}

	if( GetInterface().GetHUD() )
	{
		GetInterface().GetHUD()->SetMyScore( iMyScore );
		GetInterface().GetHUD()->SetEnemyScore( iEnemyScore );
	}

	//Set individual score

	if( CDnPartyTask::IsActive() )
		for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;
			if( !pStruct->hActor ) continue;
			if( pStruct->hActor) 
			{

				GetInterface().SetPVPGameScore( pStruct->hActor->GetUniqueID(),
					m_pScoreSystem->GetKillCount(pStruct->hActor),
					m_pScoreSystem->GetDeathCount(pStruct->hActor),
					0,                                                // Send 0 because there is no captain
					m_pScoreSystem->GetKillScore(pStruct->hActor),
					m_pScoreSystem->GetAssistScore(pStruct->hActor),
					m_pScoreSystem->GetTotalScore(pStruct->hActor) );
			}
	}
}


void CDnMutatorRespawnMode::PushUsedRespawnArea( const std::string& name )
{
	m_UsedRespawnAreaList.push_back( name );
}

bool CDnMutatorRespawnMode::bIsUsedRespawnArea( const std::string& name )
{
	std::list<std::string>::iterator itor = std::find( m_UsedRespawnAreaList.begin(), m_UsedRespawnAreaList.end(), name );
	if( itor == m_UsedRespawnAreaList.end() )
		return false;
	return true;
}
