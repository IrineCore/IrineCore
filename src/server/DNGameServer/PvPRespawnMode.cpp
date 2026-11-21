
#include "stdafx.h"
#include "PvPRespawnMode.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "DnActor.h"
#include "PvPScoreSystem.h"

CPvPRespawnMode::CPvPRespawnMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket )
:CPvPGameMode( pGameRoom, pPvPGameModeTable, pPacket )
{
}

CPvPRespawnMode::~CPvPRespawnMode()
{
}

// End GameMode since RespawnMode only plays 1 round
void CPvPRespawnMode::OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason )
{
	UINT uiWinTeam = PvPCommon::Team::Max;

	switch( Reason )
	{
		case PvPCommon::FinishReason::OpponentTeamAllGone:
		{
			uiWinTeam = OnCheckZeroUserWinTeam();
			if( uiWinTeam == PvPCommon::Team::Max )
				return;
			break;
		}
		case PvPCommon::FinishReason::AchieveWinCondition:
		case PvPCommon::FinishReason::TimeOver:
		{
			uiWinTeam = OnCheckFinishWinTeam();
			break;
		}
		default:
		{
			return;
		}
	}

	// Summarize round results
	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNGameRoom::PartyStruct* pPartyStruct = GetGameRoom()->GetPartyData(i);
		// Send only to users in SESSION_STATE_GAME_PLAY.
		if( pPartyStruct->pSession && pPartyStruct->pSession->GetState() == SESSION_STATE_GAME_PLAY )
		{
			DnActorHandle hActor = pPartyStruct->pSession->GetActorHandle();
			if( hActor )
				m_pScoreSystem->OnFinishRound( hActor, (uiWinTeam == hActor->GetTeam() ? true : false) );
		}		
	}	

	FinishGameMode( uiWinTeam, Reason );
}

void CPvPRespawnMode::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	CPvPGameMode::OnDie( hActor, hHitter );

	if( !(GetGameModeCheck()&PvPCommon::Check::CheckScore) )
		return;

	UINT uiATeamScore,uiBTeamScore;
	GetGameModeScore( uiATeamScore, uiBTeamScore );

	UINT uiMaxScore = max( uiATeamScore, uiBTeamScore );

	// Check score
	if( !bIsFinishFlag() && uiMaxScore >= GetSelectWinCondition() )
		OnCheckFinishRound( PvPCommon::FinishReason::AchieveWinCondition );
}

// End of ProcessDie()
void CPvPRespawnMode::OnFinishProcessDie( DnActorHandle hActor )
{
	if( hActor )
	{
		// Set respawn position
		_SetRespawnPosition( hActor );
		// Resurrect
		UINT uiRespawnHPPercent = GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnHPPercent			: 100;
		UINT uiRespawnMPPercent = GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnMPPercent			: 100;
		UINT uiImmortalTime		= GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnNoDamageTimeSec	: 5;
		
		char szParam[32];
		sprintf_s( szParam, "Coin/%d/%d/%d", uiRespawnHPPercent, uiRespawnMPPercent, uiImmortalTime );
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_057, uiImmortalTime*1000, szParam );
	}
}
