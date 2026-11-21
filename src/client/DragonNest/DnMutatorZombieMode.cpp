#include "StdAfx.h"
#include "DnMutatorZombieMode.h"
#include "DnMonsterActor.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "DnPVPModeEndDlg.h"
#include "PvPScoreSystem.h"
#include "DnIndividualRoundModeHUD.h"
#include "DnLocalPlayerActor.h"
#include "DnMainDlg.h"
#include "TimeSet.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

CDnMutatorZombieMode::CDnMutatorZombieMode( CDnPvPGameTask * pGameTask ):CDnMutatorTeamGame( pGameTask )
{
	RestartGame();
	m_bFirstRound = true;
	m_iRound = 1;
}

CDnMutatorZombieMode::~CDnMutatorZombieMode()
{
	if(GetInterface().GetMainBarDialog())
	{
		GetInterface().GetMainBarDialog()->LockQuickSlot(false);
		GetInterface().GetMainBarDialog()->SwapEventSlot(false);
	}
}

IScoreSystem* CDnMutatorZombieMode::CreateScoreSystem()
{
	return new IBoostPoolPvPScoreSystem();
}

bool CDnMutatorZombieMode::Initialize( const UINT uiItemID, DNTableFileFormat*  pSox )
{
	CDnIndividualRoundModeHUD * pRoundModeHud = NULL;
	pRoundModeHud = static_cast< CDnIndividualRoundModeHUD *> (GetInterface().GetHUD());

	if( pRoundModeHud )
	{
		pRoundModeHud->SetRoomName(CDnBridgeTask::GetInstance().GetPVPRoomStatus().wszPVPRoomName.c_str() );
		pRoundModeHud->SetStage( CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nObjectiveCount );
		pRoundModeHud->ChangeTeamName(120085,120084);

		int iUIstring = 0;
		if ( pSox ) 
			iUIstring = pSox->GetFieldFromLablePtr( uiItemID  , "WinCondition_UIString" )->GetInteger();

		WCHAR wszWinCon[256];
		wsprintf(wszWinCon,L"%d%s",CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nObjectiveCount,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iUIstring ));
		GetInterface().GetHUD()->SetWinCondition( wszWinCon  );
	}

	if(GetInterface().GetMainBarDialog())
	{
		GetInterface().GetMainBarDialog()->LockQuickSlot(true);
		GetInterface().GetMainBarDialog()->SwapEventSlot(true);
	}


	return InitializeBase( uiItemID, pSox );
}

void CDnMutatorZombieMode::GameStart( int nObjectiveUIString , float nRemainCountDown , float RemainSec , bool IsInGameJoin , float nMaxSec)
{
	//Set own team
	if( CDnActor::s_hLocalActor )
	{
		m_nTeam = CDnActor::s_hLocalActor->GetTeam();
	}

	GetInterface().RestartRound();

	//Set the time and
	GetInterface().GetHUD()->SetTime( nMaxSec - RemainSec , CTimeSet().GetTimeT64_GM() , CTimeSet().GetMilliseconds()); // Time + milliseconds
	GetInterface().GetRespawnModeEnd()->Reset();
	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Round  , false , 0 , false );

	//m_iRound++;

	if( IsInGameJoin )
	{
		GetInterface().GetHUD()->StartTimer();
		CDnLocalPlayerActor::LockInput( false );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
		return;
	}

	if( nRemainCountDown )
	{
		m_fCountDownTime = float(nRemainCountDown);
		//GetInterface().BeginCountDown( nRemainCountDown  , L"");
		GetInterface().BeginCountDown( (int)nRemainCountDown  , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringStarting )  );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );		

	}
	else
	{
		GetInterface().GetHUD()->StartTimer();
		CDnLocalPlayerActor::LockInput( false );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}

	WCHAR wszMsg[256]={0,};
	wsprintf( wszMsg, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120078 ), PvPCommon::Common::RoundStartMutationTimeSec );
	GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4,  wszMsg , textcolor::ORANGERED ,5.0f );
}


bool CDnMutatorZombieMode::ProcessActorDie( DnActorHandle hKillerActor, DnActorHandle hKilledActor )
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
			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::Kill ),pPlayer->GetName());

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
			//GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::WaitRespawn ), m_pGameTask->GetPlayerDieRespawnDelta() );

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::Killed ),pPlayer->GetName());

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

			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::SeeKill ),pPlayer->GetName(),pOtherPlayer->GetName());

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
			//GetInterface().OpenpRespawnGauageDlg(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RespawnModeString::WaitRespawn ),m_pGameTask->GetPlayerDieRespawnDelta());
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

void CDnMutatorZombieMode::EndGame( void * pData )
{
	CPvPScoreSystem::SMyScore * sInfo = NULL;
	sInfo = (CPvPScoreSystem::SMyScore*)m_pScoreSystem->GetScoreStruct(CDnActor::s_hLocalActor->GetName());

	SCPVP_FINISH_PVPMODE * pPacket = NULL;	

	pPacket = ( SCPVP_FINISH_PVPMODE * )pData;

	GetInterface().GetHUD()->HaltTimer();

	GetInterface().OpenFinalResultDlg();	

	byte cResult = PVPResult::Win;

	if( m_pScoreSystem )
		m_pScoreSystem->SetGameModeScore( pPacket->uiATeamScore, pPacket->uiBTeamScore );

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
				GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory ), true ); // "Victory!"	
		}

		//--------------------------------------------------------
		// For PVP participating players, as before.
		else
		{
			if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory
			{
				if( CDnActor::s_hLocalActor &&   PvPCommon::Team::A == CDnActor::s_hLocalActor->GetTeam() || CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
				{
					cResult = PVPResult::Win;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory ) );
					//GetInterface().GetHUD()->AddMyScore( 1 );
				}
				else
				{
					cResult = PVPResult::Lose;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDefeated ) );
					//GetInterface().GetHUD()->AddEnemyScore( 1 );
				}		
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
			{
				if( CDnActor::s_hLocalActor &&   PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
				{
					cResult = PVPResult::Win;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory ) );
					//GetInterface().GetHUD()->AddMyScore( 1 );
				}
				else
				{
					cResult = PVPResult::Lose;
					GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDefeated ) );
					//GetInterface().GetHUD()->AddEnemyScore( 1 );
				}		
			}

			if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
			{
				cResult = PVPResult::Draw;
				GetInterface().GetRespawnModeEnd()->SetResultString1(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDraw ) );		
			}
		}
	}

#else

	if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory
	{
		if( CDnActor::s_hLocalActor &&   PvPCommon::Team::A == CDnActor::s_hLocalActor->GetTeam() || CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
		{
			cResult = PVPResult::Win;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory ) );
			//GetInterface().GetHUD()->AddMyScore( 1 );
		}
		else
		{
			cResult = PVPResult::Lose;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDefeated ) );
			//GetInterface().GetHUD()->AddEnemyScore( 1 );
		}		
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
	{
		if( CDnActor::s_hLocalActor &&   PvPCommon::Team::B == CDnActor::s_hLocalActor->GetTeam() )
		{
			cResult = PVPResult::Win;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory ) );
			//GetInterface().GetHUD()->AddMyScore( 1 );
		}
		else
		{
			cResult = PVPResult::Lose;
			GetInterface().GetRespawnModeEnd()->SetResultString2(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDefeated ) );
			//GetInterface().GetHUD()->AddEnemyScore( 1 );
		}		
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
	{
		cResult = PVPResult::Draw;
		GetInterface().GetRespawnModeEnd()->SetResultString1(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDraw ) );		
	}
#endif // PRE_MOD_PVPOBSERVER

	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Round  , true ,cResult, true );

}

void CDnMutatorZombieMode::_RenderGameModeScore()
{
	if( !m_pScoreSystem || m_nTeam ==0)
		return;

	UINT uiATeamScore, uiBTeamScore;
	m_pScoreSystem->GetGameModeScore( uiATeamScore, uiBTeamScore );

	int iMyScore		= (m_nTeam == PvPCommon::Team::A) ? uiATeamScore : uiBTeamScore;
	int iEnemyScore		= (m_nTeam == PvPCommon::Team::A) ? uiBTeamScore : uiATeamScore;

	if(m_nTeam == PvPCommon::Team::Observer) // If I am a spectator
	{
		iMyScore = uiATeamScore;
		iEnemyScore = uiBTeamScore;
	}

	CDnIndividualRoundModeHUD * pRoundModeHud = NULL;
	pRoundModeHud = static_cast< CDnIndividualRoundModeHUD *> (GetInterface().GetHUD());	

	//Set individual score and team score

	int iMyTeamReamin = 0;
	int iEnemyTeamReamin = 0;

	if( CDnPartyTask::IsActive() ) {
		for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if( !pStruct ) continue;
			if( !pStruct->hActor ) continue;
			if( !pStruct->bCompleteBreakInto ) continue;
			if( pStruct->hActor) 
			{
				CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(pStruct->hActor.GetPointer());
				
#ifdef PRE_MOD_PVPOBSERVER
				if( pStruct->usTeam != PvPCommon::Team::Observer && pPlayer )
#else
				if(pPlayer)
#endif // PRE_MOD_PVPOBSERVER
				{
					if( !pPlayer->IsTransformMode() )
						++iMyTeamReamin;
					else
						++iEnemyTeamReamin;
				}

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

	if( pRoundModeHud )
	{
#ifdef PRE_MOD_PVPOBSERVER
		pRoundModeHud->SetWinStage( iMyScore , iEnemyScore, true );
#else
		pRoundModeHud->SetWinStage( iMyScore , iEnemyScore );
#endif // PRE_MOD_PVPOBSERVER
		
		pRoundModeHud->SetMyScore( iMyTeamReamin );
		pRoundModeHud->SetEnemyScore( iEnemyTeamReamin );

		m_iRound = iMyScore + iEnemyScore + 1;
	}
}

void CDnMutatorZombieMode::RoundEnd(void * pData )
{
	WCHAR  wszResultMSG[256] = L"";

	SCPVP_FINISH_PVPROUND* pPacket = static_cast<SCPVP_FINISH_PVPROUND*>(pData);
	GetInterface().GetHUD()->HaltTimer();

	if( m_pScoreSystem )
		m_pScoreSystem->SetGameModeScore( pPacket->uiATeamScore, pPacket->uiBTeamScore );

	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Round  , true , PVPResult::RoundEND , false );

	wsprintf( wszResultMSG,L"%s %d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringRound ) , m_iRound );
	GetInterface().GetRespawnModeEnd()->SetResultString1( wszResultMSG );

	if( pPacket-> uiWinTeam == PvPCommon::Team::A ) //Ateam victory
	{
		GetInterface().GetRespawnModeEnd()->SetResultString3(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory_Human ) );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringGainAdditionalXP ) , textcolor::YELLOW ,4.0f );
		return;
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::B ) //Bteam victory
	{
		GetInterface().GetRespawnModeEnd()->SetResultString3(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringVictory_Zombie ) );
		return;
	}

	if( pPacket-> uiWinTeam == PvPCommon::Team::Max ) //Draw
	{
		GetInterface().GetRespawnModeEnd()->SetResultString1(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ZombieModeString::UIStringDraw ));
		return;
	}	

}
