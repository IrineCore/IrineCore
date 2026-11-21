
#include "stdafx.h"
#include "DNPvPGameRoom.h"
#include "PvPOccupationPoint.h"
#include "PvPOccupationSystem.h"
#include "PvPGameMode.h"
#include "DNUserSession.h"
#include "DNMissionSystem.h"
#include "DnPlayerActor.h"

CPvPOccupationPoint::CPvPOccupationPoint(CDNGameRoom * pGameRoom, CPvPOccupactionSystem * pSystem, SOBB * pOBB, TPositionAreaInfo * pAreaInfo)
{
	m_pGameRoom = pGameRoom;
	m_OBB = *pOBB;
	m_OccupationInfo = *pAreaInfo;
	m_pOccupationSystem = pSystem;

	m_nOccupantState = PvPCommon::OccupationState::None;
	m_nOccupantTeamID = 0;
	m_nOccupantUniqueID = 0;	
	
	m_nTryUniqueID = 0;		//Attempting player/entity
	m_nTryTeamID = 0;
	m_wstrTryName.clear();

	m_nWaitUniqueID = 0;
	m_nWaitTeamID = 0;
	m_wstrMoveWaitName.clear();
	
	m_nTryOccupationTime = 0;
	m_nLastOccupationCheckTime = 0;
	m_nMoveWaitOccupationTime = 0;
	m_bClimaxMode = false;
}

CPvPOccupationPoint::~CPvPOccupationPoint()
{
}

void CPvPOccupationPoint::Process(LOCAL_TIME LocalTime, float fDelta)
{
	_CheckPoint(LocalTime);
}

void CPvPOccupationPoint::OnDamage(DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage)
{
	if (!hActor || !hHitter) return;
	if (!(m_nOccupantState&PvPCommon::OccupationState::Try))
		return;

	if (hActor->GetTeam() != hHitter->GetTeam())
	{
		CancelAcquirePoint(hActor);
	}
}

bool CPvPOccupationPoint::CheckInside(EtVector3 &Position)
{
	return m_OBB.IsInside(Position);
}

void CPvPOccupationPoint::GetPointState(PvPCommon::OccupationStateInfo &Info)
{
	Info.nAreaID = GetID();
	Info.nOccupationState = m_nOccupantState;

	Info.nOwnedUniqueID = m_nOccupantUniqueID;
	Info.nOwnedTemID = m_nOccupantTeamID;

	Info.nTryUniqueID = m_nTryUniqueID;
	Info.nTryTeamID = m_nTryTeamID;

	Info.nMoveWaitUniqueID = m_nWaitUniqueID;
	Info.nMoveWaitTeamID = m_nWaitTeamID;
}

bool CPvPOccupationPoint::TryAcquirePoint(DnActorHandle hActor, LOCAL_TIME LocalTime)
{
	if (!hActor) return false;
	if (m_nOccupantState&PvPCommon::OccupationState::CantTryMask)
	{
		_DANGER_POINT();
		return false;
	}
	
	m_nTryUniqueID = hActor->GetUniqueID();
	m_wstrTryName = hActor->GetName();
	m_nTryTeamID = hActor->GetTeam();
	m_nTryOccupationTime = LocalTime;

	if (m_nOccupantState&PvPCommon::OccupationState::MoveWait)
	{
		if (m_nWaitTeamID == m_nTryTeamID)
		{
			m_nOccupantState &= ~PvPCommon::OccupationState::MoveWait;
			m_nMoveWaitOccupationTime = 0;
			m_nWaitTeamID = 0;
		}
	}

	m_nOccupantState |= PvPCommon::OccupationState::Try;

	_SendOccupationPointState();
	return true;
}

bool CPvPOccupationPoint::CancelAcquirePoint(DnActorHandle hActor)
{
	if (!hActor) return false;
	if (!(m_nOccupantState&PvPCommon::OccupationState::Try))
		return false;

	if (m_nTryUniqueID == hActor->GetUniqueID())
	{
		_ClearTry();
		_SendOccupationPointState();
		return true;
	}
	return false;
}

bool CPvPOccupationPoint::ChangeClimaxMode()
{
	m_bClimaxMode = true;
	return m_bClimaxMode;
}

void CPvPOccupationPoint::_CheckPoint(LOCAL_TIME LocalTime)
{
	if (m_nOccupantState&PvPCommon::OccupationState::Own)
	{
		//CheckOwn
		if (_IsAcquireResourceTerm(LocalTime))
		{
			if (m_pOccupationSystem)
			{
				m_pOccupationSystem->GainResource(m_nOccupantTeamID, (m_bClimaxMode ? m_OccupationInfo.nClimaxGainVal : m_OccupationInfo.nGainResource));
			}
			else
				_DANGER_POINT();
		}
	}

	if (m_nOccupantState&PvPCommon::OccupationState::Try)
	{
		//CheckTry
		if (_IsFinishedTryTerm(LocalTime))
		{
			if (m_nOccupantTeamID == m_nTryTeamID)
			{
				m_nOccupantUniqueID = m_nTryUniqueID;		
				_OnOccupyArea();
				m_nOccupantTeamID = m_nTryTeamID;

				_ClearTry();
				_ClearMoveWait();

				m_nOccupantState |= PvPCommon::OccupationState::Own;
			}
			else
			{
				m_nWaitTeamID = m_nTryTeamID;
				m_nWaitUniqueID = m_nTryUniqueID;
				m_wstrMoveWaitName = m_wstrTryName;
				m_nMoveWaitOccupationTime = LocalTime;

				_ClearTry();

				m_nOccupantState &= ~PvPCommon::OccupationState::Own;		//Entering waiting state clears the acquired state
				m_nOccupantState |= PvPCommon::OccupationState::MoveWait;
			}

			//Notify the room of the state change.
			_SendOccupationPointState();
		}
	}

	if (m_nOccupantState&PvPCommon::OccupationState::MoveWait)
	{
		//CheckMoveWait
		if (_IsFinishedMoveWaitTerm(LocalTime))
		{
			if (m_pOccupationSystem)
				m_pOccupationSystem->OnAcquirePoint(m_wstrMoveWaitName.c_str(), m_nWaitTeamID, m_nOccupantTeamID, GetID(), LocalTime);

			m_nOccupantUniqueID = m_nWaitUniqueID;
			_OnOccupyArea();
			m_nOccupantTeamID = m_nWaitTeamID;

			_ClearTry();
			_ClearMoveWait();

			m_nOccupantState |= PvPCommon::OccupationState::Own;			

			//Notify the room of the state change.
			_SendOccupationPointState();
		}
	}
}

void CPvPOccupationPoint::_ClearTry()
{
	m_nTryUniqueID = 0;		
	m_nTryTeamID = 0;
	m_wstrTryName.clear();
	m_nTryOccupationTime = 0;

	m_nOccupantState &= ~PvPCommon::OccupationState::Try;
}

void CPvPOccupationPoint::_ClearMoveWait()
{
	m_nWaitUniqueID = 0;
	m_nWaitTeamID = 0;
	m_wstrMoveWaitName.clear();
	m_nMoveWaitOccupationTime = 0;

	m_nOccupantState &= ~PvPCommon::OccupationState::MoveWait;
}

bool CPvPOccupationPoint::_IsAcquireResourceTerm(LOCAL_TIME LocalTime)
{
	if (m_nLastOccupationCheckTime <= 0)
		m_nLastOccupationCheckTime = LocalTime;
	else if (m_nLastOccupationCheckTime + (m_bClimaxMode ? m_OccupationInfo.nClimaxGainTermTick : m_OccupationInfo.nGainResourceTermTick) < LocalTime)
	{
		m_nLastOccupationCheckTime = LocalTime;
		return true;
	}
	return false;
}

bool CPvPOccupationPoint::_IsFinishedTryTerm(LOCAL_TIME LocalTime)
{
	if (m_nTryOccupationTime == 0)
		_DANGER_POINT();
	return (m_nTryOccupationTime > 0 && m_nTryOccupationTime + (m_bClimaxMode ? m_OccupationInfo.nClimaxTryTick : m_OccupationInfo.nRequireTryTick) < LocalTime) ? true : false;
}

bool CPvPOccupationPoint::_IsFinishedMoveWaitTerm(LOCAL_TIME LocalTime)
{
	if (m_nMoveWaitOccupationTime == 0)
		_DANGER_POINT();
	return (m_nMoveWaitOccupationTime > 0 && m_nMoveWaitOccupationTime + (m_bClimaxMode ? m_OccupationInfo.nClimaxCompleteOccupationTick : m_OccupationInfo.nCompleteOccupationTick) < LocalTime) ? true : false;
}

void CPvPOccupationPoint::_SendOccupationPointState()
{
	if (m_pOccupationSystem)
	{
		PvPCommon::OccupationStateInfo Info;
		memset(&Info, 0, sizeof(PvPCommon::OccupationStateInfo));

		Info.nAreaID = GetID();
		Info.nOccupationState = m_nOccupantState;

		Info.nOwnedUniqueID = m_nOccupantUniqueID;
		Info.nOwnedTemID = m_nOccupantTeamID;

		Info.nTryUniqueID = m_nTryUniqueID;
		Info.nTryTeamID = m_nTryTeamID;

		Info.nMoveWaitUniqueID = m_nWaitUniqueID;
		Info.nMoveWaitTeamID = m_nWaitTeamID;


		m_pOccupationSystem->SendOccupationPointState(1, &Info);
	}	
}

void CPvPOccupationPoint::_OnOccupyArea()
{
	CDNUserSession* pGameSession = m_pGameRoom->GetUserSession(m_nOccupantUniqueID);
	if( !pGameSession ) 
		return;

	CPvPGameMode * pGameMode = m_pGameRoom->GetPvPGameMode();
	if( !pGameMode )
		return;

	switch( pGameMode->GetPvPGameModeTable()->uiGameMode )
	{
	case PvPCommon::GameMode::PvP_Occupation:
		{
			if (m_nOccupantTeamID == 0)		// Capture
			{
				pGameSession->IncCommonVariableData(CommonVariable::Type::PvPOccupation_OccupyArea);
				pGameSession->GetEventSystem()->OnEvent(EventSystem::OnOccupyArea);
			}
			else	// Conquest
			{
				pGameSession->IncCommonVariableData(CommonVariable::Type::PvPOccupation_StealArea);
				pGameSession->GetEventSystem()->OnEvent(EventSystem::OnStealArea);
			}
		}
		break;
	}
}
