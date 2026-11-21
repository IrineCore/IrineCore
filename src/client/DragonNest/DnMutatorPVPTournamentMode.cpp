#include "StdAfx.h"
#include "DnMutatorPVPTournamentMode.h"
#include "DnMonsterActor.h"
#include "DnPlayerActor.h"
#include "DnInterface.h"
#include "DnPVPModeEndDlg.h"
#include "PvPScoreSystem.h"
#include "DnPVPTournamentModeHud.h"
#include "DnLocalPlayerActor.h"
#include "TimeSet.h"
#include "DnPVPTournamentDataMgr.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#endif

#ifdef PRE_ADD_PVP_TOURNAMENT

CDnMutatorPVPTournamentMode::CDnMutatorPVPTournamentMode(CDnPvPGameTask* pGameTask)
	: CDnMutatorGame(pGameTask)
{
	RestartGame();

	m_bFirstRound = true;

	m_iEnemyUserCount = 0;
	m_iFriendyUserCount = 0;

	m_iFinishDetailReason = 0;
	m_fFinishDetailReasonShowDelay = -1.f;
}

CDnMutatorPVPTournamentMode::~CDnMutatorPVPTournamentMode()
{

}

void CDnMutatorPVPTournamentMode::RestartGame()
{
	m_fCountDownTime = 0.f;
}

IScoreSystem* CDnMutatorPVPTournamentMode::CreateScoreSystem()
{
	return new IBoostPoolPvPScoreSystem();
}

bool CDnMutatorPVPTournamentMode::Initialize(const UINT uiItemID, DNTableFileFormat* pSox)
{
	CDnPVPTournamentModeHUD* pPVPTournamentModeHUD = dynamic_cast<CDnPVPTournamentModeHUD*>(GetInterface().GetHUD());
	if (pPVPTournamentModeHUD)
	{
		const CDnBridgeTask::sGameStatus& status = CDnBridgeTask::GetInstance().GetPVPRoomStatus();
		pPVPTournamentModeHUD->SetRoomName(status.wszPVPRoomName.c_str());
		pPVPTournamentModeHUD->ResetStage();
	}

	return InitializeBase( uiItemID, pSox );
}

void CDnMutatorPVPTournamentMode::Process(LOCAL_TIME LocalTime, float fDelta)
{
	CDnMutatorGame::Process(LocalTime , fDelta);

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

	if (m_fFinishDetailReasonShowDelay >= 0)
	{
		m_fFinishDetailReasonShowDelay -= fDelta;
	}
	else if (m_fFinishDetailReasonShowDelay != -1.f)
	{
		if (m_iFinishDetailReason != 0)
		{
			CDnPVPModeEndDlg* pModeEndDlg = GetInterface().GetRespawnModeEnd();
			if (pModeEndDlg)
				pModeEndDlg->SetResultString3(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, m_iFinishDetailReason));
		}

		m_fFinishDetailReasonShowDelay = -1.f;
		m_iFinishDetailReason = 0;
	}

	_RenderGameModeScore();
}

void CDnMutatorPVPTournamentMode::GameStart(int nObjectiveUIString, float nRemainCountDown, float RemainSec, bool IsInGameJoin, float nMaxSec)
{
	GetInterface().RestartRound();

	if (CDnBridgeTask::IsActive() == false || CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_Tournament) == false)
	{
		_ASSERT(0);
		return;
	}

	CDnPVPTournamentModeHUD* pTournamentHud = static_cast<CDnPVPTournamentModeHUD*>(GetInterface().GetHUD());
	if (pTournamentHud == NULL)
	{
		_ASSERT(0);
		return;
	}

	CDnPVPModeEndDlg* pModeEndDlg = GetInterface().GetRespawnModeEnd();
	if (pModeEndDlg == NULL)
	{
		_ASSERT(0);
		return;
	}

	pTournamentHud->SetTime(nMaxSec - RemainSec, CTimeSet().GetTimeT64_GM(), CTimeSet().GetMilliseconds()); // Time + milliseconds
	pModeEndDlg->Reset();
	GetInterface().ShowPVPModeEndDialog(PvPCommon::GameMode::PvP_Tournament, false, 0, false);

	if (IsInGameJoin)
	{
		pTournamentHud->StartTimer();
		CDnLocalPlayerActor::LockInput(false);
		GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nObjectiveUIString), textcolor::YELLOW, 4.0f);
		pTournamentHud->OrderShowDualInfo( true );
		return;
	}

	if (nRemainCountDown)
	{
		m_fCountDownTime = float(nRemainCountDown);
		GetInterface().BeginCountDown((int)nRemainCountDown, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::UIStringStarting));
		GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nObjectiveUIString), textcolor::YELLOW, 4.0f);
	}
	else
	{
		pTournamentHud->StartTimer();
		CDnLocalPlayerActor::LockInput(false);
		GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nObjectiveUIString), textcolor::YELLOW, 4.0f);
	}

#ifdef PRE_ADD_PVP_DUAL_INFO
	pTournamentHud->OrderShowDualInfo();
#endif
}

void CDnMutatorPVPTournamentMode::ChangeActorHair(DnActorHandle hActor)
{
	float fMyHairColor[3] = {200.0f/255.0f,300.0f/255.0f,350.0f/255.0f}; //Blue
	float fEnemyHairColor[3] = {300.0f/255.0f,50.0f/255.0f,10.0f/255.0f}; //Red

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(hActor.GetPointer());
	if (pPartsBody)
	{
		DWORD dwColor=0;

		if (CDnActor::s_hLocalActor->GetUniqueID() == hActor->GetUniqueID())
		{
			CDnParts::ConvertFloatToR10G10B10(&dwColor, fMyHairColor);
		}
		else
		{
			if (hActor->GetTeam() == 0)
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fMyHairColor);
			else
				CDnParts::ConvertFloatToR10G10B10(&dwColor, fEnemyHairColor);
		}

		pPartsBody->SetPartsColor(MAPartsBody::HairColor, dwColor);
	}
}

void CDnMutatorPVPTournamentMode::ProcessEnterUser(DnActorHandle hActor, bool isIngameJoin, void * pData)
{
	if (!hActor)
		return;

	CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());
	if (pPlayerActor)
	{
		if (hActor->GetTeam() == PvPCommon::Team::Observer)
		{
			hActor->Show(false);
			hActor->CmdShowExposureInfo(false);
		}
	}

	GetInterface().AddPVPGameUer(hActor);
}

bool CDnMutatorPVPTournamentMode::ProcessActorDie(DnActorHandle hKillerActor, DnActorHandle hKilledActor)
{
	std::wstring msg;
	if (CDnActor::s_hLocalActor && hKillerActor == CDnActor::s_hLocalActor) //When local player is the killer
	{
		if (hKilledActor->IsPlayerActor())
		{
			msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::Kill), hKilledActor->GetName());
			GetInterface().AddMessageText(msg.c_str(), textcolor::PVP_F_PLAYER);

			// Increment local kill count here.
			TPvPGroup *pPvPInfo = CDnPartyTask::GetInstance().GetPvPInfo();
			int nIndex = hKillerActor->GetClassID() - 1;
			if (pPvPInfo && nIndex >= 0 && nIndex < PvPCommon::Common::MaxClass)
				pPvPInfo->uiKOClassCount[nIndex] += 1;
			return true;
		}
	}

	if (hKillerActor && hKillerActor->IsPlayerActor()) // Another player is the killer
	{
		if (CDnActor::s_hLocalActor && hKilledActor == CDnActor::s_hLocalActor) // Local player is the victim
		{
			msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::Killed), hKillerActor->GetName());
			GetInterface().AddMessageText(msg.c_str(), textcolor::PVP_E_PLAYER);

			// Increment local death count here.
			TPvPGroup *pPvPInfo = CDnPartyTask::GetInstance().GetPvPInfo();
			int nIndex = hKillerActor->GetClassID() - 1;
			if (pPvPInfo && nIndex >= 0 && nIndex < PvPCommon::Common::MaxClass)
				pPvPInfo->uiKObyClassCount[nIndex] += 1;
			return true;
		}

		if (hKilledActor->IsPlayerActor()) //Another player is the victim 
		{
			msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::SeeKill), hKillerActor->GetName(), hKilledActor->GetName());
			GetInterface().AddMessageText(msg.c_str(), textcolor::PVP_E_PLAYER);

			return true;
		}		

		if (hKilledActor->IsMonsterActor())
			return true;
	}

	WarningLog("Nobody Killed ,Nobody Kill");
	return false;
}

void CDnMutatorPVPTournamentMode::EndGame(void* pData)
{
	if (m_pScoreSystem == NULL || !CDnActor::s_hLocalActor)
	{
		_ASSERT(0);
		return;
	}

	SCPVP_FINISH_PVPMODE* pPacket = (SCPVP_FINISH_PVPMODE*)pData;
	if (!pPacket)
		return;

	CDnPVPBaseHUD* pBaseHud = GetInterface().GetHUD();
	if (pBaseHud == NULL)
		return;

	CDnPVPTournamentDataMgr* pMgr = GetTournamentDataMgr();
	if (pMgr == NULL)
	{
		_ASSERT(0);
		return;
	}

	pMgr->ClearGameMatchCurrentUser();

	const SMatchUserInfo* pInfo = pMgr->GetSlotInfoBySessionID(pPacket->uiWinSessionID);
	if (pInfo)
		pMgr->SetFinalReportUser_TournamentStep(pInfo->uiSessionID, 1);

	pBaseHud->HaltTimer();
	GetInterface().OpenFinalResultDlg();

	std::wstring winChatMsg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3008); // UISTRING : Exit Game
	if (pInfo)
		winChatMsg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120189), pInfo->playerName.c_str()); // UISTRING : %s won!
	GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", winChatMsg.c_str(), false);

#ifdef PRE_MOD_PVPOBSERVER
	//--------------------------------------------------------
	// Handle UI when LocalPlayer is Observer and not Draw.
	//
	CDnPVPModeEndDlg* pModeEndDlg = GetInterface().GetRespawnModeEnd();
	if (pModeEndDlg)
	{
		if (pInfo)
		{
		std::wstring winTitle = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120209), pInfo->playerName.c_str()); // UISTRING : Tournament Winner!;
		pModeEndDlg->SetResultString1(winTitle.c_str());

		std::wstring winMsg = FormatW(L"%s %s", pInfo->playerName.c_str(), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120301)); // UISTRING : Victory!
		pModeEndDlg->SetResultString3(winMsg.c_str());
	}
		else
		{
			pModeEndDlg->SetResultString2(winChatMsg.c_str());
		}
	}

	GetInterface().ShowPVPModeEndDialog(PvPCommon::GameMode::PvP_Tournament, true, PVPResult::Win, true);
#else
	CDnPVPModeEndDlg* pModeEndDlg = GetInterface().GetRespawnModeEnd();
	if (pModeEndDlg)
	{
		byte cResult = PVPResult::Win;

		if (m_nTeam == pPacket->uiWinTeam || (m_nTeam == PvPCommon::Team::Observer && pPacket->uiWinTeam == PvPCommon::Team::A))
		{
			cResult = PVPResult::Win;
			pModeEndDlg->SetResultString1(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::UIStringVictory));
		}
		else if (pPacket->uiWinTeam == PvPCommon::Team::Max) //Draw
		{
			cResult = PVPResult::Draw;
			pModeEndDlg->SetResultString1(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::UIStringDraw));
		}
		else
		{
			cResult = PVPResult::Lose;
			pModeEndDlg->SetResultString1(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, PVPTournamentModeString::UIStringDefeated));
		}
	}

	GetInterface().ShowPVPModeEndDialog(PvPCommon::GameMode::PvP_Tournament, true, cResult, true);
#endif // PRE_MOD_PVPOBSERVER 

#ifdef PRE_ADD_PVP_DUAL_INFO
	if (CDnBridgeTask::IsActive() && CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_Tournament))
	{
		CDnPVPTournamentModeHUD* pTounamentModeHud = static_cast<CDnPVPTournamentModeHUD*>(pBaseHud);
		if (pTounamentModeHud)
			pTounamentModeHud->ClearDualInfo();
	}
#endif
}

void CDnMutatorPVPTournamentMode::RoundEnd(void * pData)
{
	if (!CDnActor::s_hLocalActor)
		return;

	if (CDnBridgeTask::IsActive() == false || CDnBridgeTask::GetInstance().IsPvPGameMode(PvPCommon::GameMode::PvP_Tournament) == false)
		return;

	CDnPVPTournamentModeHUD* pTournamentHud = static_cast<CDnPVPTournamentModeHUD*>(GetInterface().GetHUD());
	if (pTournamentHud == NULL)
		return;

	std::wstring resultMsg;
	SCPVP_FINISH_PVPROUND* pPacket = static_cast<SCPVP_FINISH_PVPROUND*>(pData);
	pTournamentHud->HaltTimer();

#ifdef PRE_ADD_PVP_DUAL_INFO
	pTournamentHud->ClearDualInfo();
#endif
	int nWinMessageString = 0;
	CDnPVPTournamentDataMgr* pMgr = GetTournamentDataMgr();
	if (pMgr == NULL)
	{
		_ASSERT(0);
		return;
	}

	pMgr->ClearGameMatchCurrentUser();

	std::wstring userMsg;
	const SMatchUserInfo* pInfo = pMgr->GetSlotInfoBySessionID(pPacket->uiWinSessionID);
	if (pInfo)
	{
		userMsg = FormatW(L"%s %s", pInfo->playerName.c_str(), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120301)); // UISTRING : Victory!

		int uiStringNum = 0;
		int forFlagNum = 0;
		switch (pPacket->cTournamentStep)
		{
		case 2:
		case 4: 
			{
				uiStringNum = 120188;
				forFlagNum = 120208; // UISTRING : Advanced to Finals!!
			}
			break;
		case 8:
			{
				uiStringNum = 120186;
				forFlagNum = 120207; // UISTRING : Advanced to Semifinals!!
			}
			break;
		case 16:
			{
				uiStringNum = 120185;
				forFlagNum = 120199; // UISTRING : Advanced to Quarterfinals!!
			}
			break;
		case 32:
			{
				uiStringNum = 120184;
				forFlagNum = 120198; // UISTRING : Advanced to Round of 16!!
			}
			break;
		default:
			{
				uiStringNum = 0;
				forFlagNum = 0;
			}
			break;
		}

		std::wstring msg;
		if (uiStringNum != 0)
		{
			msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, uiStringNum), pInfo->playerName.c_str());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", msg.c_str(), false);
		}

		if (forFlagNum != 0)
		{
			CDnPVPModeEndDlg* pModeEndDlg = GetInterface().GetRespawnModeEnd();
			if (pModeEndDlg == NULL)
				return;

			pModeEndDlg->SetResultString1(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, forFlagNum));
			pModeEndDlg->SetResultString3(userMsg.c_str());

			GetInterface().ShowPVPModeEndDialog(PvPCommon::GameMode::PvP_Tournament, true, 0, false);
		}
	}
	else
	{
		userMsg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 120196); // UISTRING : Both were eliminated due to a draw
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", userMsg.c_str(), true);
	}

	pMgr->SetFinalReportUser_TournamentStep(pPacket->uiWinSessionID, pPacket->cTournamentStep);
}

void CDnMutatorPVPTournamentMode::OnDefaultWin(const SCPvPTournamentDefaultWin& data)
{
	CDnPVPTournamentDataMgr* pMgr = GetTournamentDataMgr();
	if (pMgr == NULL)
	{
		_ASSERT(0);
		return;
	}

	std::wstring playerName;
	const SMatchUserInfo* pInfo = pMgr->GetSlotInfoBySessionID(data.uiWinSessionID);
	if (pInfo == NULL)
		return;

	int uiStringNum = 0;
	switch (data.cTournamentStep)
	{
	case 2: uiStringNum = 120195; break;
	case 4: uiStringNum = 120194; break;
	case 8: uiStringNum = 120193; break;
	case 16: uiStringNum = 120192; break;
	case 32: uiStringNum = 120191; break;
	default: 
		uiStringNum = 0;
	}

	std::wstring msg = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, uiStringNum), pInfo->playerName.c_str());
	GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", msg.c_str(), true);
};

void CDnMutatorPVPTournamentMode::SetFinishDetailReason(int nUIStringIndex)
{
	m_iFinishDetailReason = nUIStringIndex;
	m_fFinishDetailReasonShowDelay = 3.f;
}

CDnPVPTournamentDataMgr* CDnMutatorPVPTournamentMode::GetTournamentDataMgr() const
{
	if (CDnBridgeTask::IsActive())
	{
		CDnPVPTournamentDataMgr& dataMgr = CDnBridgeTask::GetInstance().GetPVPTournamentDataMgr();
		return (&dataMgr);
	}

	return NULL;
}

void CDnMutatorPVPTournamentMode::_RenderGameModeScore()
{
	if (!m_pScoreSystem)
		return;

	//Set individual score
	if( CDnPartyTask::IsActive() )
	{
		int i = 0;
		for (; i < (int)CDnPartyTask::GetInstance().GetPartyCount(); i++)
		{
			CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
			if (!pStruct)
				continue;

			if (!pStruct->hActor)
				continue;

			if (pStruct->hActor)
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
}
#endif