
#include "stdafx.h"
#include "PvPOccupationSystem.h"
#include "EtWorldEventArea.h"
#include "DNPvPGameRoom.h"
#include "PvPOccupationPoint.h"
#include "PvPOccupationTeam.h"
#include "DNUserSession.h"
#include "PvPGameMode.h"
#include "DnPvPGameTask.h"
#include "DNGameDataManager.h"
#include "DNMasterConnectionManager.h"
#include "PvPOccupationMode.h"

CPvPOccupactionSystem::CPvPOccupactionSystem(CDNGameRoom * pGameRoom, CPvPGameMode * pMode) : m_pGameRoom(pGameRoom), m_pGameMode(pMode), m_bInit(false), \
m_nOccupationSystemState(PvPCommon::OccupationSystemState::None), m_nStartOccupationSystem(0)
{
	memset(&m_OccupationModeInfo, 0, sizeof(TBattleGourndModeInfo));
	m_bClimaxMode = false;
	m_nForceWinTem = PvPCommon::Team::Max;
}

CPvPOccupactionSystem::~CPvPOccupactionSystem()
{
	for (std::list <CPvPOccupationPoint*>::iterator ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
		SAFE_DELETE((*ii));
	m_lOccupationPointList.clear();

	for (std::list <CPvPOccupationTeam*>::iterator ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
		SAFE_DELETE((*ii));
	m_lOccupationTeamList.clear();
}

bool CPvPOccupactionSystem::InitializeOccupationSystem(TBattleGourndModeInfo * pModeInfo)
{
	if (pModeInfo == NULL)
	{
		_DANGER_POINT();
		return false;
	}

	if (m_bInit)
	{
		_DANGER_POINT();		//Reinit!
		return false;
	}

	if (m_pGameMode == NULL)
	{
		_DANGER_POINT();
		return false;
	}

	if (m_pGameMode->InitializeOccupationMode(&pModeInfo->ModeInfo) == false)
	{
		_DANGER_POINT();
		return false;
	}

	m_OccupationModeInfo = *pModeInfo;
	m_bInit = true;
	return true;
}

void CPvPOccupactionSystem::Process(LOCAL_TIME LocalTime, float fDelta)
{
	if (m_pGameMode == NULL) return;
	if (m_pGameMode->bIsStarting() == false && m_nStartOccupationSystem <= 0)
	{
		m_nStartOccupationSystem = LocalTime + (m_OccupationModeInfo.nWaitingMin * (1000 * 60));
		m_nOccupationSystemState = PvPCommon::OccupationSystemState::Wait;
		_SyncOccupationModeState();
		return;
	}

	if (m_pGameMode->bIsFinishFlag()) return;
	if (m_bClimaxMode == false && m_OccupationModeInfo.nClimaxTime > 0)
	{
		if (m_pGameMode->GetTimeOver() - ((float)(m_OccupationModeInfo.nClimaxTime/1000)) <= 0.0f)
		{
			m_bClimaxMode = true;
			_SendClimaxMode();

			std::list <CPvPOccupationPoint*>::iterator ii;
			for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
				(*ii)->ChangeClimaxMode();
		}
	}

	if (m_nOccupationSystemState == PvPCommon::OccupationSystemState::Wait)
	{
		if (LocalTime > m_nStartOccupationSystem)
		{
			if (_PrepareOccupation())
			{
				m_nOccupationSystemState = PvPCommon::OccupationSystemState::Play;
				_SyncOccupationModeState();
			}
			else
			{
				_DANGER_POINT(); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//Handle termination here
				m_pGameMode->OnCheckFinishRound(PvPCommon::FinishReason::OpponentTeamQualifierScore);
			}
		}
		return;
	}
	else if (m_nOccupationSystemState == PvPCommon::OccupationSystemState::Play)
	{
		std::list <CPvPOccupationPoint*>::iterator ii;
		for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
		{
			if (m_pGameMode->bIsFinishFlag()) continue;
			(*ii)->Process(LocalTime, fDelta);
		}
	}
}

void CPvPOccupactionSystem::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Play)
		return;

	if (m_pGameMode == NULL || !hActor)
	{
		_DANGER_POINT();
		return;
	}

	if (hActor->IsPlayerActor()) return;
	CPvPOccupationTeam * pTeam = _GetOccupationTeam(hActor->GetTeam());
	if (pTeam)
	{
		if (pTeam->GetBossID() <= 0)
			return;

		if (pTeam->IsBoss(hActor))
		{
			SendOccupationrealTimeScore(hHitter->GetTeam());
			m_pGameMode->OnCheckFinishRound(PvPCommon::FinishReason::OpponentTeamBossDead);
		}
	}
}

void CPvPOccupactionSystem::OnDamage(DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage)
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Play)
		return;

	if (hActor->IsPlayerActor() == false) return;
	std::list <CPvPOccupationPoint*>::iterator ii;
	for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
		(*ii)->OnDamage(hActor, hHitter, iDamage);
}

void CPvPOccupactionSystem::OnSuccessBreakInto(CDNUserSession * pSession)
{
	_SyncOccupationState(pSession);
}

bool CPvPOccupactionSystem::OnCheckFinishRound(PvPCommon::FinishReason::eCode Reason, USHORT &nTeam, UINT &nWinGuild, UINT &nLoseGuild, bool bForce/* = false*/)
{
	if (m_pGameMode == NULL)
		return false;

	switch (Reason)
	{
	case PvPCommon::FinishReason::TimeOver:
		{
			int nScore = 0;
			int nWinTeam = 0;

			std::list <CPvPOccupationTeam*>::iterator ii;
			for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
			{
				int nTeamScore = 0;

				if (m_pGameMode->GetPvPGameModeTable()->uiGameMode != PvPCommon::GameMode::PvP_Occupation)
				{
					static_cast<CPvPOccupationMode*>(m_pGameMode)->GetOccupationTeamScore((*ii)->GetTeamID(), nTeamScore);				
					nTeamScore += (int)(*ii)->GetTeamResourceSum();
				}
				else
					nTeamScore += (int)(*ii)->GetTeamResource();
					
				if (nTeamScore > nScore)
				{
					nTeam = nWinTeam = (*ii)->GetTeamID();
					nWinGuild = (*ii)->GetGuildID();
					nScore = nTeamScore;
					continue;
				}

				nLoseGuild = (*ii)->GetGuildID();
				if (nScore == nTeamScore && m_lOccupationTeamList.begin() != ii)
				{
					if (m_pGameMode->bIsGuildWarSystem())
					{
						nLoseGuild = 0;
						return CPvPOccupactionSystem::OnCheckFinishRound(PvPCommon::FinishReason::OpponentTeamQualifierScore, nTeam, nWinGuild, nLoseGuild, true);
					}

					nTeam = nWinTeam = PvPCommon::Team::Max;
					nWinGuild = 0;
					nLoseGuild = 0;
				}
			}

			if (m_pGameMode->bIsGuildWarSystem() && nWinTeam != PvPCommon::Team::Max && nLoseGuild == 0)
			{
				for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
				{
					if ((*ii)->GetGuildID() != nWinGuild)
					{
						nLoseGuild = (*ii)->GetGuildID();
						break;
					}
				}
			}
			return true;
		}

	case PvPCommon::FinishReason::OpponentTeamBossDead:
		{
			bool bCheck = false;
			int nLimitParam = 0;
			for (int i = 0; i < PvPCommon::Common::MaximumVitoryCondition; i++)
			{
				if (m_OccupationModeInfo.ModeInfo.nVictoryCondition[i] == PvPCommon::OccupationVictoryCondition::BossKill)
				{
					bCheck = true;
					nLimitParam = m_OccupationModeInfo.ModeInfo.nVictoryParam[i];
					break;
				}
			}

			for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
			{
				if (m_OccupationModeInfo.ModeInfo.nBossID[i] <= 0)
					return false;
			}
			
			bCheck = false;
			int nWinTeam = 0;
			std::list <CPvPOccupationTeam*>::iterator ii;
			for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
			{
				if ((*ii)->IsBossDie())
				{
					nLoseGuild = (*ii)->GetGuildID();
					continue;
				}

				nWinTeam = (*ii)->GetTeamID();
				nWinGuild = (*ii)->GetGuildID();
				bCheck = true;
			}

			if (bCheck && nWinTeam)
			{
				nTeam = nWinTeam;
				return true;
			}
			break;
		}

	case PvPCommon::FinishReason::OpponentTeamResourceLimit:
		{
			bool bCheck = false;
			int nLimitParam = 0;
			for (int i = 0; i < PvPCommon::Common::MaximumVitoryCondition; i++)
			{
				if (m_OccupationModeInfo.ModeInfo.nVictoryCondition[i] == PvPCommon::OccupationVictoryCondition::ResourceLimit)
				{
					bCheck = true;
					nLimitParam = m_OccupationModeInfo.ModeInfo.nVictoryParam[i];
					break;
				}
			}

			if (bForce == false && bCheck == false)
				return false;

			bCheck = false;
			int nWinTeam = 0;
			std::list <CPvPOccupationTeam*>::iterator ii;
			for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
			{
				if ((int)(*ii)->GetTeamResource() < nLimitParam)
				{
					nLoseGuild = (*ii)->GetGuildID();
					continue;
				}

				nWinTeam = (*ii)->GetTeamID();
				nWinGuild = (*ii)->GetGuildID();
				bCheck = true;
			}

			if (bCheck && nWinTeam)
			{
				nTeam = nWinTeam;
				return true;
			}
			break;
		}

	case PvPCommon::FinishReason::OpponentTeamScoreLimit:
		{
			bool bCheck = false;
			int nLimitParam = 0;
			for (int i = 0; i < PvPCommon::Common::MaximumVitoryCondition; i++)
			{
				if (m_OccupationModeInfo.ModeInfo.nVictoryCondition[i] == PvPCommon::OccupationVictoryCondition::ScoreLimit)
				{
					bCheck = true;
					nLimitParam = m_OccupationModeInfo.ModeInfo.nVictoryParam[i];
					break;
				}
			}

			if (bForce == false && bCheck == false)
				return false;

			bCheck = false;
			int nWinTeam = 0;
			std::list <CPvPOccupationTeam*>::iterator ii;
			for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
			{
				int nScore = 0;
				if (static_cast<CPvPOccupationMode*>(m_pGameMode)->GetOccupationTeamScore((*ii)->GetTeamID(), nScore) == false)
				{
					_DANGER_POINT();
					continue;
				}

				if (nScore < nLimitParam)
				{
					nLoseGuild = (*ii)->GetGuildID();
					continue;
				}

				nWinTeam = (*ii)->GetTeamID();
				nWinGuild = (*ii)->GetGuildID();
				bCheck = true;
			}

			if (bCheck && nWinTeam)
			{
				nTeam = nWinTeam;
				return true;
			}
			break;
		}

	case PvPCommon::FinishReason::OpponentTeamQualifierScore:
		{
			//Fetch and store the preliminaries scores.
			if (m_pGameRoom && m_pGameRoom->bIsGuildWarSystem())
			{
				bool bCheck = false;
				int nWinTeam = 0;
				int nBiggerScore = 0;
				std::list <CPvPOccupationTeam*>::iterator ii;
				for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
				{
					int nTempScore = (*ii)->GetQualifyingScore();
					if (nTempScore < nBiggerScore)
					{
						nLoseGuild = (*ii)->GetGuildID();
						continue;
					}

					nBiggerScore = nTempScore;
					nWinTeam = (*ii)->GetTeamID();
					nWinGuild = (*ii)->GetGuildID();
					bCheck = true;
				}

				nTeam = nWinTeam;

				if (nLoseGuild == 0)
				{
					for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
					{
						if ((*ii)->GetGuildID() != nWinGuild)
						{
							nLoseGuild = (*ii)->GetGuildID();
							break;
						}
					}
				}
				return true;
			}
			break;
		}

	case PvPCommon::FinishReason::OpponentTeamAllGone:
		{
			if (m_pGameRoom && m_pGameRoom->bIsGuildWarSystem() == false)
			{
				nTeam = m_pGameMode->OnCheckZeroUserWinTeam();
				if(nTeam != PvPCommon::Team::Max)
					return true;
			}
			break;
		}

	case PvPCommon::FinishReason::OpponentTeamForceWin:
		{
			if (m_pGameRoom && m_pGameRoom->bIsGuildWarSystem())
			{
				if (m_nForceWinTem != PvPCommon::Team::Max)
				{
					std::list <CPvPOccupationTeam*>::iterator ii;
					for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
					{
						if ((*ii)->GetTeamID() == m_nForceWinTem)
						{
							nTeam = (*ii)->GetTeamID();
							nWinGuild = (*ii)->GetGuildID();
						}
						else
						{
							nLoseGuild = (*ii)->GetGuildID();
						}
					}
					return true;
				}
			}
		}
	}

	return false;
}

bool CPvPOccupactionSystem::OnTryAcquirePoint(DnActorHandle hActor, int nAreaID, LOCAL_TIME LocalTime)
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Play)
		return false;

	if (hActor && hActor->IsPlayerActor())
	{
		CPvPOccupationPoint * pPoint = _GetOccupationArea(hActor->GetPosition());
		if (pPoint == NULL)
		{
			_DANGER_POINT();
			return false;
		}

		//Verify
		if (pPoint->GetID() != nAreaID)
		{
			_DANGER_POINT();
			return false;
		}

		if (pPoint->TryAcquirePoint(hActor, LocalTime))
		{
			if (m_pGameMode)
			{
				m_pGameMode->OnTryAcquirePoint(hActor, nAreaID, LocalTime);
				SendOccupationrealTimeScore(hActor->GetTeam());
				return true;
			}
		}
	}
	return false;
}

bool CPvPOccupactionSystem::OnAcquirePoint(const WCHAR * pName, int nAcquireTeam, int nSnatchedTeam, int nAreaID, LOCAL_TIME LocalTime)
{
	CPvPOccupationTeam * pTeam = _GetOccupationTeam(nAcquireTeam);
 	if (pTeam == NULL)
	{
		_DANGER_POINT();
		return false;
	}	

	if (pTeam->AcquirePoint(nAreaID))
	{
		if (m_pGameMode->OnAcquirePoint(pTeam->GetTeamID(), pName, nSnatchedTeam != 0 ? true : false))
		{
			SendOccupationrealTimeScore(pTeam->GetTeamID());
		}
		else
			_DANGER_POINT();
	}
	else
		_DANGER_POINT();

	if (nAcquireTeam != nSnatchedTeam && nSnatchedTeam > 0)
	{
		pTeam = _GetOccupationTeam(nSnatchedTeam);
		if (pTeam)
		{
			if (pTeam->SnatchedPoint(nAreaID) == false)
				_DANGER_POINT();
			return true;
		}
		return false;
	}
	
	return true;
}

bool CPvPOccupactionSystem::OnCancelAcquirePoint(DnActorHandle hActor)
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Play)
		return false;

	bool bCheck = false;
	if (hActor && hActor->IsPlayerActor())
	{
		std::list <CPvPOccupationPoint*>::iterator ii;
		for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
		{
			(*ii)->CancelAcquirePoint(hActor);
			bCheck = true;
		}

		return bCheck;
	}
	return false;
}

bool CPvPOccupactionSystem::OnTryAcquireSkill(CDNUserSession * pSession, int nSKillID, int nLevel, int nSlotIndex)
{
	if (pSession == NULL)
		return false;

	CPvPOccupationTeam * pTeam = _GetOccupationTeam(pSession->GetTeam());
	if (pTeam == NULL)
		return false;

	TBattleGroundSkillInfo * pSkill = g_pDataManager->GetBattleGroundSkill(nSKillID);
	if (pSkill == NULL)
		return false;

	TBattleGroundEffectValue * pLevel = g_pDataManager->GetBattleGroundSkillLevel(nSKillID, nLevel);
	if (pLevel == NULL)
		return false;

	if (pTeam->AddTeamSkill(nSlotIndex, pSkill, pLevel))
	{
		if (m_pGameRoom)
		{
			CDNUserSession * pUser;
			for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
			{
				pUser = m_pGameRoom->GetUserData(i);
				if (pUser && pUser->GetTeam() == pSession->GetTeam())
					pUser->SendPvPTryAcquireSkill(nSKillID, nLevel, ERROR_NONE);
			}
			return true;
		}
	}

	return false;
}

bool CPvPOccupactionSystem::OnInitSkill(CDNUserSession * pSession)
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Wait)
		return false;		//Not always possible

	CPvPOccupationTeam * pTeam = _GetOccupationTeam(pSession->GetTeam());
	if (pTeam == NULL)
		return false;

	pTeam->ClearTeamSkill();

	if (m_pGameRoom)
	{
		CDNUserSession * pUser;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUser = m_pGameRoom->GetUserData(i);
			if (pUser && pUser->GetTeam() == pSession->GetTeam())
				pUser->SendPvPInitSkill(ERROR_NONE);
		}
		return true;
	}
	return true;
}

bool CPvPOccupactionSystem::OnUseSkill(CDNUserSession * pSession, int nSkillID)
{
	if (m_nOccupationSystemState != PvPCommon::OccupationSystemState::Play)
		return false;		//Not always possible

	CPvPOccupationTeam * pTeam = _GetOccupationTeam(pSession->GetTeam());
	if (pTeam == NULL)
		return false;

	if (pTeam->UseSkill(pSession, nSkillID))
	{
		if (m_pGameRoom)
		{
			CDNUserSession * pUser;
			for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
			{
				pUser = m_pGameRoom->GetUserData(i);
				//if (pUser && pUser->GetTeam() == pSession->GetTeam()) // Broadcast usage packet to all because the action must be performed
					pUser->SendPvPUseSkill(pSession->GetSessionID(), nSkillID, ERROR_NONE);
			}
			SendOccupationTeamState(pTeam->GetTeamID(), pTeam->GetTeamResource(), pTeam->GetTeamResourceSum());
			return true;
		}
	}
	return false;
}

bool CPvPOccupactionSystem::SwapSkillIndex(CDNUserSession * pSession, char cFrom, char cTo)
{
	CPvPOccupationTeam * pTeam = _GetOccupationTeam(pSession->GetTeam());
	if (pTeam == NULL)
		return false;

	if (pTeam->SwapSkillIndex(cFrom, cTo))
	{
		if (m_pGameRoom)
		{
			CDNUserSession * pUser;
			for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
			{
				pUser = m_pGameRoom->GetUserData(i);
				if (pUser && pUser->GetTeam() == pSession->GetTeam())
					pUser->SendPvPSwapSkillIndex(cFrom, cTo, ERROR_NONE);
			}
			return true;
		}
	}
	return false;
}

bool CPvPOccupactionSystem::AddOccupationArea(CEtWorldEventArea * pArea, TPositionAreaInfo * pInfo)
{
	if (IsExistPoint(pInfo->nAreaID) != NULL)
	{
		_ASSERT_EXPR(0, L"Reset Occupation Zone!");
		return false;
	}

	CPvPOccupationPoint * pPoint = new CPvPOccupationPoint(m_pGameRoom, this, pArea->GetOBB(), pInfo);
	m_lOccupationPointList.push_back(pPoint);
	return true;
}

bool CPvPOccupactionSystem::IsTeamBoss(DnActorHandle hActor)
{	
	for (std::list <CPvPOccupationTeam*>::iterator ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
	{
		if ((*ii)->IsBoss(hActor))
			return true;
	}
	return false;
}

CPvPOccupationTeam * CPvPOccupactionSystem::AddOccupationTeam(int nTeam, UINT nGuildDBID, int nQualifyingScore)
{
	if (IsExistTeam(nTeam))
	{
		_DANGER_POINT();
		return NULL;
	}

	int nBossIndex = PvPCommon::Team::A == nTeam ? PvPCommon::TeamIndex::A : PvPCommon::TeamIndex::B;
	CPvPOccupationTeam * pTeam = new CPvPOccupationTeam(m_pGameRoom, m_pGameMode, nTeam, m_OccupationModeInfo.nMaximumResourceLimit, m_OccupationModeInfo.nPvPSkillPoint, \
		m_OccupationModeInfo.ModeInfo.nBossID[nBossIndex], nGuildDBID, nQualifyingScore);
	if (!pTeam)
		return NULL;

	m_lOccupationTeamList.push_back(pTeam);
	return pTeam;
}

void CPvPOccupactionSystem::GainResource(int nTeam, int nGain)
{
	CPvPOccupationTeam * pTeam = _GetOccupationTeam(nTeam);
	if (pTeam == NULL)
	{
		_DANGER_POINT();
		return;
	}

	int nGainSum = nGain + _GetBonusResource(nGain, pTeam);
	pTeam->GainPoint(nGainSum);

	if (m_pGameMode)
		m_pGameMode->OnCheckFinishRound(PvPCommon::FinishReason::OpponentTeamResourceLimit);
}

void CPvPOccupactionSystem::SendOccupationPointState(BYTE cCount, PvPCommon::OccupationStateInfo * pInfo, CDNUserSession * pSession/* = NULL*/)
{
	if (pInfo == NULL || m_pGameMode == NULL)
		return;

	if (pSession)
	{
		pSession->SendOccupationState(cCount, pInfo);
	}
	else if (m_pGameRoom)
	{
		CDNUserSession * pUserSession;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUserSession = m_pGameRoom->GetUserData(i);
			if (pUserSession)
				pUserSession->SendOccupationState(cCount, pInfo);
			else
				_DANGER_POINT();
		}
	}
}

void CPvPOccupactionSystem::SendOccupationTeamState(int nTeam, int nResource, int nResourceSum, CDNUserSession * pSession/* = NULL*/)
{
	if (pSession)
	{
		pSession->SendOccupationTeamState(nTeam, nResource, nResourceSum);
	}
	else if (m_pGameRoom)
	{
		CDNUserSession * pUserSession;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUserSession = m_pGameRoom->GetUserData(i);
			
			if (pUserSession)
			{
				pUserSession->SendOccupationTeamState(nTeam, nResource, nResourceSum);
			}
			else
				_DANGER_POINT();
		}
	}
}

void CPvPOccupactionSystem::SendOccupationSkillState(int nTeam, PvPCommon::OccupationSkillSlotInfo * pInfo, CDNUserSession * pSession/* = NULL*/)
{
	if (pInfo == NULL)
		return;

	if (pSession)
	{
		if (pSession->GetTeam() == nTeam)
			pSession->SendOccupationSkillState(pInfo);
	}
	else if (m_pGameRoom)
	{
		CDNUserSession * pUserSession;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUserSession = m_pGameRoom->GetUserData(i);

			if (pUserSession)
			{
				//Broadcast team info only to the team.
				if (pUserSession->GetActorHandle() && pUserSession->GetActorHandle()->GetTeam() == nTeam)
					pUserSession->SendOccupationSkillState(pInfo);
			}
			else
				_DANGER_POINT();
		}
	}
}

void CPvPOccupactionSystem::SendOccupationrealTimeScore(int nTeam)
{
	if (g_pMasterConnectionManager && m_pGameRoom->bIsGuildWarSystem())
	{
		int nScore = 0;
		if (static_cast<CPvPOccupationMode*>(m_pGameMode)->GetOccupationTeamScore(nTeam, nScore))
		{
			CPvPOccupationTeam * pTeam = _GetOccupationTeam(nTeam);
			if (pTeam)
				g_pMasterConnectionManager->SendPvPRealTimeScore(m_pGameRoom->GetWorldSetID(), pTeam->GetGuildID(), nScore);
			else
				_DANGER_POINT();
		}
		else
			_DANGER_POINT();
	}
}

bool CPvPOccupactionSystem::SetForceWinGuild(UINT nWinGuildDBID)
{
	if (m_pGameRoom->bIsGuildWarSystem())
	{
		std::list <CPvPOccupationTeam*>::iterator ii;
		for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
		{
			if ((*ii)->GetGuildID() == nWinGuildDBID)
			{
				m_nForceWinTem = (*ii)->GetTeamID();
				return true;
			}
		}
	}
	return false;
}

void CPvPOccupactionSystem::OccupationReportLog(PvPCommon::FinishReason::eCode Reason, int nWinGuildDBID, int nLoseGuildID)
{
	if (m_pGameRoom->bIsGuildWarSystem())
	{
		g_Log.Log(LogType::_GUILDWAR, L"GuildWarResult RoomID[%d] Reason[%d] WinGuildID[%d] LoseGuildID[%d]\n", m_pGameRoom->GetRoomID(), Reason, nWinGuildDBID, nLoseGuildID);
		std::list <CPvPOccupationTeam*>::iterator ii;
		for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
		{
			int nScore = 0;
			static_cast<CPvPOccupationMode*>(m_pGameMode)->GetOccupationTeamScore((*ii)->GetTeamID(), nScore);
			g_Log.Log(LogType::_GUILDWAR, L"GuildWarResult RoomID[%d] GuildID[%d] Team[%d] Res[%d] ResSum[%d] Qualify[%d] TeamScore[%d]", m_pGameRoom->GetRoomID(), (*ii)->GetGuildID(), (*ii)->GetTeamID(), (*ii)->GetTeamResource(), (*ii)->GetTeamResourceSum(), (*ii)->GetQualifyingScore(), nScore);
		}
	}
}

UINT CPvPOccupactionSystem::GetRespawnSec()
{
	return m_bClimaxMode ? m_OccupationModeInfo.nClimaxRespawnTime : 0;
}

int CPvPOccupactionSystem::_GetBonusResource(int nGain, CPvPOccupationTeam * pTeam)
{
	if (pTeam == NULL) return 0;
	if (pTeam->GetTeamResource() <= 0) return 0;

	int nOwnedCount = pTeam->GetOwnedCount();
	if (nOwnedCount <= 0) return 0;

	int nBonusRate = 0;
	if (PvPCommon::Common::MaximumCapturePosition <= nOwnedCount - 1)
		nBonusRate = m_OccupationModeInfo.nAllOcuupationBonusGain;
	else
	{
		for (int i = 0; i < PvPCommon::Common::MaximumCapturePosition; i++)
		{
			if (i == (nOwnedCount - 1))
				nBonusRate = m_OccupationModeInfo.nOccupationBonusGain[i];
		}
	}

	return nBonusRate == 0 ? 0 : (int)((float)(((float)nGain * (float)((float)(nBonusRate)/100)) + 0.5f));
}

bool CPvPOccupactionSystem::IsExistPoint(int nAreaID)
{
	std::list <CPvPOccupationPoint*>::iterator ii;
	for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
	{
		if ((*ii)->GetID() == nAreaID)
			return true;
	}
	return false;
}

CPvPOccupationPoint * CPvPOccupactionSystem::_GetOccupationArea(EtVector3 * pPosition)
{
	std::list <CPvPOccupationPoint*>::iterator ii;
	for (ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
	{
		if ((*ii)->CheckInside(*pPosition))
			return (*ii);
	}
	return NULL;
}

bool CPvPOccupactionSystem::IsExistTeam(int nTeam)
{
	std::list <CPvPOccupationTeam*>::iterator ii;
	for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
	{
		if ((*ii)->GetTeamID() == nTeam)
			return true;
	}
	return false;
}

CPvPOccupationTeam * CPvPOccupactionSystem::_GetOccupationTeam(UINT nTeamID)
{
	std::list <CPvPOccupationTeam*>::iterator ii;
	for (ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
	{
		if ((*ii)->GetTeamID() == nTeamID)
			return (*ii);
	}
	return NULL;
}

bool CPvPOccupactionSystem::_PrepareOccupation()
{
	if (m_pGameRoom)
	{
		CDnGameTask * pTask = m_pGameRoom->GetGameTask();
		if (pTask == NULL)
		{
			_DANGER_POINT();
			return false;
		}

		for (std::list <CPvPOccupationTeam*>::iterator ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
		{
			if ((*ii)->GetBossID() > 0)
			{
				DnActorHandle hBoss = pTask->RequestGenerationMonsterFromMonsterID((*ii)->GetBossID(), *(*ii)->GetSpawnPos(), EtVector3( 0.f, 0.f, 0.f ), EtVector3( 0.f, 0.f, 0.f ), (*ii)->GetSOBB(), -1, (*ii)->GetTeamID());
				if (hBoss)
					(*ii)->SetBossHandle(hBoss);
				else
					return false;
			}
		}
		
		return true;
	}
	return false;
}

void CPvPOccupactionSystem::_SyncOccupationModeState()
{
	if (m_pGameRoom)
	{
		CDNUserSession * pUserSession;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUserSession = m_pGameRoom->GetUserData(i);
			if (pUserSession)
				pUserSession->SendOccupationModeState(m_nOccupationSystemState);
			else
				_DANGER_POINT();
		}
	}
}

void CPvPOccupactionSystem::_SyncOccupationState(CDNUserSession * pSession)
{
	//Sync current mode state
	_SyncOccupationModeState();

	//Get the point state value and
	PvPCommon::OccupationStateInfo Info[PARTYMAX];
	memset(Info, 0, sizeof(Info));

	int i = 0;
	for (std::list <CPvPOccupationPoint*>::iterator ii = m_lOccupationPointList.begin(); ii != m_lOccupationPointList.end(); ii++)
	{
		(*ii)->GetPointState(Info[i]);
		i++;
	}
	SendOccupationPointState(i, Info, pSession);

	PvPCommon::OccupationSkillSlotInfo Skill;
	for (std::list <CPvPOccupationTeam*>::iterator ii = m_lOccupationTeamList.begin(); ii != m_lOccupationTeamList.end(); ii++)
	{
		memset(&Skill, 0, sizeof(Skill));

		SendOccupationTeamState((*ii)->GetTeamID(), (*ii)->GetTeamResource(), (*ii)->GetTeamResourceSum());
		(*ii)->GetTeamSkillInfo(Skill);
		SendOccupationSkillState((*ii)->GetTeamID(), &Skill);
	}
}

void CPvPOccupactionSystem::_SendClimaxMode()
{
	if (m_pGameRoom)
	{
		CDNUserSession * pUserSession;
		for (int i = 0; i < (int)m_pGameRoom->GetUserCount(); i++)
		{
			pUserSession = m_pGameRoom->GetUserData(i);
			if (pUserSession)
				pUserSession->SendOccupationClimaxMode();
		}
	}
}
