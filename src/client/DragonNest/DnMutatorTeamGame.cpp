#include "StdAfx.h"
#include "DnMutatorTeamGame.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "ScoreSystem.h"
#include "DnPartsHair.h"
#include "DnPVPBaseHUD.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnMutatorTeamGame::CDnMutatorTeamGame( CDnPvPGameTask * pGameTask ):CDnMutatorGame( pGameTask )
{
	m_bIsHairColorMode = true;
	m_bIsIndividualMode = false;
	m_bIsTeamHairColorMode = false;
	m_fCountDownTime = 0.0f;
	m_nTeam = 0;
}

CDnMutatorTeamGame::~CDnMutatorTeamGame()
{
	
}

void CDnMutatorTeamGame::RestartGame()
{
	m_nTeam = 0;
	m_fCountDownTime = 0.0f;
}

void CDnMutatorTeamGame::ProcessEnterUser( DnActorHandle hActor , bool isIngameJoin, void * pData )
{
	if( !hActor)
		return;

	WCHAR wszMessage[256];
	SecureZeroMemory(wszMessage,sizeof(wszMessage));

	CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());

	// isIngameJoin indicates mid-game join (intrusion).
	if( pPlayerActor && isIngameJoin )
	{
		if( hActor->GetTeam() != PvPCommon::Team::Observer 
			&& !CDnBridgeTask::GetInstance().IsPvPGameMode( PvPCommon::GameMode::PvP_GuildWar )	)
		{
			wsprintf(wszMessage,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, GameString::EnterGame ),pPlayerActor->GetName());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", wszMessage);
		}
		// Hide observers.
		else
		{
			if(hActor)
			{
				hActor->Show( false );
				hActor->CmdShowExposureInfo( false );
			}
		}
	}

	GetInterface().AddPVPGameUer( hActor );

	float fMyHairColor[3] =       {200.0f/255.0f,300.0f/255.0f,350.0f/255.0f}; //Blue
	float fEnemyHairColor[3] =  {300.0f/255.0f,50.0f/255.0f,10.0f/255.0f};	//Red

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
	if( pPartsBody ) {

		DWORD dwColor=0;

		if( m_bIsTeamHairColorMode || ( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer ) )
		{
			if( PvPCommon::Team::A == hActor->GetTeam() )
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fMyHairColor);
			else if( PvPCommon::Team::B == hActor->GetTeam() )
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fEnemyHairColor);
		}
		else
		{
			if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == hActor->GetTeam() )
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fMyHairColor);
			else
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fEnemyHairColor);
		}
		if(m_bIsHairColorMode)
			pPartsBody->SetPartsColor( MAPartsBody::HairColor, dwColor );

	}
}

void CDnMutatorTeamGame::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_fCountDownTime )
	{
		m_fCountDownTime -= fDelta;

		CDnWorldEnvironment* pWorldEnv = CDnWorld::GetInstance().GetEnvironment();

		if( m_fCountDownTime < 0.0f)
		{
			if( GetInterface().GetHUD() )
			{
				GetInterface().GetHUD()->StartTimer();
				m_fCountDownTime = 0.0f;
			}
			if( pWorldEnv )
				pWorldEnv->ResetSaturation();
		}else
		{
			if( pWorldEnv )
				pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, 0.0f, 1000 );
		}

	}

	if( GetInterface().GetHUD() )
	{
		if( GetInterface().GetHUD()->NearEndGame()) //Game end time is imminent
		{
			GetInterface().ToggleShowAllPVPPlayerInMap( true );
		}else
			GetInterface().ToggleShowAllPVPPlayerInMap( false );
	}	

	_RenderGameModeScore();
}