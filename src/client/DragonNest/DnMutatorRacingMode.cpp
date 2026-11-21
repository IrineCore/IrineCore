#include "stdafx.h"
#include "DnMutatorRacingMode.h"
#include "PvPScoreSystem.h"
#include "DnInterface.h"
#include "DnRespawnModeHUD.h"
#include "DnPlayerActor.h"
#include "Timeset.h"
#include "DnPvPRacingResultDlg.h"
#include "DnPVPModeEndDlg.h"
#include "DnInterface.h"
#include "DnCaptionDlg_04.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

#if defined( PRE_ADD_RACING_MODE )

CDnMutatorRacingMode::CDnMutatorRacingMode( CDnPvPGameTask * pGameTask )
: CDnMutatorTeamGame( pGameTask )
, m_nWarningTime( CDnMutatorRacingMode::eWARNING_TIME )
, m_bLocalPlayerGoal( false )
{
	RestartGame();
}

CDnMutatorRacingMode::~CDnMutatorRacingMode()
{

}

IScoreSystem* CDnMutatorRacingMode::CreateScoreSystem()
{
	return new IBoostPoolPvPScoreSystem();
}

bool CDnMutatorRacingMode::Initialize( const UINT uiItemID, DNTableFileFormat*  pSox )
{
	if( GetInterface().GetHUD())
	{
		GetInterface().GetHUD()->SetRoomName( CDnBridgeTask::GetInstance().GetPVPRoomStatus().wszPVPRoomName.c_str() );

		int iUIstring = 0;

		if ( pSox ) 
			iUIstring = pSox->GetFieldFromLablePtr( uiItemID  , "WinCondition_UIString" )->GetInteger();

		WCHAR wszWinCon[256];
		wsprintf(wszWinCon,L"%d%s",CDnBridgeTask::GetInstance().GetPVPRoomStatus().m_nObjectiveCount,GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iUIstring ));
		GetInterface().GetHUD()->SetWinCondition( wszWinCon  );
	}	

	return InitializeBase( uiItemID, pSox );
}

void CDnMutatorRacingMode::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnMutatorTeamGame::Process( LocalTime, fDelta );

	int nRemainSec = 0;

	if( true == IsWarning( nRemainSec ) )
	{
		std::wstring wszUIString = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000023355 );	// UISTRING : Race ends in %d seconds
		WCHAR wszOutputString[256] = {0,};
		swprintf_s( wszOutputString, _countof(wszOutputString), wszUIString.c_str(), nRemainSec );

		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4, wszOutputString, textcolor::YELLOW, 2.0f );
	}
}

void CDnMutatorRacingMode::GameStart( int nObjectiveUIString , float nRemainCountDown , float RemainSec , bool IsInGameJoin , float nMaxSec)
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
		GetInterface().BeginCountDown( (int)nRemainCountDown  , GetEtUIXML().GetUIString( CEtUIXML::idCategory1, RacingModeString::UIStringStarting )  );
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}
	else
	{
		GetInterface().GetHUD()->StartTimer();
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2,  GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nObjectiveUIString ) , textcolor::YELLOW ,4.0f );
	}

	CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
	if( NULL == pInvenDlg )
		return;


#ifdef PRE_MOD_RC_MODE_OBSERVER_BE_NOT_OPEN_PETDLG
	
	CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer());
	
	// #62470 - Skip opening inventory for spectators 
	if( pPlayerActor && pPlayerActor->GetTeam() == PvPCommon::Team::Observer )
		return;

	GetInterface().OpenInvenDialog();
	pInvenDlg->ShowTab( ST_INVENTORY_VEHICLE );

#else
	GetInterface().OpenInvenDialog();
	pInvenDlg->ShowTab( ST_INVENTORY_VEHICLE );

#endif




}

void CDnMutatorRacingMode::EndGame( void * pData )
{
	SCPVP_FINISH_PVPMODE * pPacket = NULL;	

	pPacket = ( SCPVP_FINISH_PVPMODE * )pData;

	GetInterface().GetHUD()->HaltTimer();
	GetInterface().OpenFinalResultDlg();

	byte cResult = PVPResult::Win;
	if(IsIndividualMode())
	{
		GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Racing, false ,cResult, true );
		return;
	}

	GetInterface().ShowPVPModeEndDialog( PvPCommon::GameMode::PvP_Racing, true ,cResult, true );
}

void CDnMutatorRacingMode::SetLapTime( const SCPVP_RACING_RAPTIME * pData )
{
	for( int itr = 0; itr < pData->cCount; ++itr )
		GetInterface().SetPvPRacingScore( pData->sRapTimeArr[itr].uiSessionID, pData->sRapTimeArr[itr].dwLapTime );
}

bool CDnMutatorRacingMode::IsWarning( int & nRemainSec )
{
	if( NULL == GetInterface().GetHUD() )
		return false;

	float fRemainSec = GetInterface().GetHUD()->GetFloatRemainSec();

	nRemainSec = static_cast<int>(ceil(fRemainSec))%60;

	if( nRemainSec < m_nWarningTime 
		&& 0 != nRemainSec 
		&& 60.0f > fRemainSec 
		&& false == m_bLocalPlayerGoal )
	{
		m_nWarningTime = nRemainSec;
		return true;
	}

	return false;
}
#endif	// #if defined( PRE_ADD_RACING_MODE )