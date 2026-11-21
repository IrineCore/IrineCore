#include "stdafx.h"

#if defined(PRE_ADD_REVENGE)
#include "RevengeSystem.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "PvPGameMode.h"

CRevengeSystem::CRevengeSystem(CPvPGameMode* pGameMode) : m_pGameMode(pGameMode)
{

}

CRevengeSystem::~CRevengeSystem()
{
	ResetAllRevengeConditionCount();
	SAFE_DELETE_MAP(m_MapRevengeTarget);
}

bool CRevengeSystem::CheckRevenge( CDNUserSession *pGameSession, CDNUserSession *pHitterSession )
{
	if( !pGameSession || !pHitterSession )
		return false;

	if( GetRevengeTargetSessionID(pGameSession->GetSessionID()) <= 0)
	{
		int nCount = IncreaseRevengeConditionCount(pGameSession->GetSessionID(), pHitterSession->GetSessionID());
		if(nCount >= Revenge::RevengeConditionCount)
		{
			SetRevengeTargetSessionID(pGameSession->GetSessionID(), pHitterSession->GetSessionID());
			ResetRevengeConditionCount(pGameSession->GetSessionID());

			pGameSession->SendPvPSetRevengeTarget( pGameSession->GetSessionID(), pHitterSession->GetSessionID(), Revenge::TargetReason::SetNewRevengeTarget);
			pGameSession->GetEventSystem()->OnEvent( EventSystem::OnSetRevengeTarget );
		}
	}

	if( GetRevengeTargetSessionID(pHitterSession->GetSessionID()) == pGameSession->GetSessionID() )
	{
		SetRevengeTargetSessionID(pHitterSession->GetSessionID(), 0);
		pHitterSession->SendPvPSetRevengeTarget( pHitterSession->GetSessionID(), 0, Revenge::TargetReason::SuccessRevenge );
		//Broadcast success packet
		static_cast<CDNPvPGameRoom*>(pHitterSession->GetGameRoom())->BroadcastRevengeSuccess( pHitterSession->GetSessionID(), pGameSession->GetSessionID() );
		return true;
	}

	return false;
}

void CRevengeSystem::LeaveUser(UINT nSessionID)
{
	CDNUserSession * pLeaveUserSession = m_pGameMode->GetGameRoom()->GetUserSession(nSessionID);	
	if( pLeaveUserSession && pLeaveUserSession->GetTeam() == PvPCommon::Team::Observer )
		return;		//Just return if observer

	//Reset revenge condition info of the leaving user
	SetRevengeTargetSessionID(nSessionID, 0);
	ResetRevengeConditionCount(nSessionID);
	ResetRevengeConditionTarget(nSessionID);

	//Reset users whose revenge target is the leaving user
	std::map<UINT,UINT>::iterator iter = m_MapRevengeTarget.begin();
	for( ; iter != m_MapRevengeTarget.end() ; iter++ )
	{
		if( iter->second == nSessionID )
		{
			SetRevengeTargetSessionID(iter->first, 0);
			CDNUserSession * pSession= m_pGameMode->GetGameRoom()->GetUserSession(iter->first);
			if( pSession )
				pSession->SendPvPSetRevengeTarget(iter->first, 0, Revenge::TargetReason::TargetLeaveGame);
		}
	}
}

int CRevengeSystem::IncreaseRevengeConditionCount(UINT nSessionID, UINT nTargetSessionID)
{
	std::map<UINT,int>::iterator iter = m_MapRevengeConditions[nSessionID].find(nTargetSessionID);
	if( iter == m_MapRevengeConditions[nSessionID].end() )
		m_MapRevengeConditions[nSessionID][nTargetSessionID] = 0;

	return ++m_MapRevengeConditions[nSessionID][nTargetSessionID];
}

void CRevengeSystem::ResetRevengeConditionCount(UINT nSessionID)
{
	m_MapRevengeConditions[nSessionID].clear();
}

void CRevengeSystem::ResetRevengeConditionTarget(UINT nSessionID)
{
	std::map<UINT,std::map<UINT,int>>::iterator iter = m_MapRevengeConditions.begin();
	for(  ; iter != m_MapRevengeConditions.end(); iter++ )
	{
		if( iter->second.find(nSessionID) != iter->second.end() )
			(iter->second)[nSessionID] = 0;
	}
}

void CRevengeSystem::ResetAllRevengeConditionCount()
{
	std::map<UINT,std::map<UINT,int>>::iterator iter = m_MapRevengeConditions.begin();
	for(  ; iter != m_MapRevengeConditions.end(); iter++ )
	{
		SAFE_DELETE_MAP(iter->second);
	}

	SAFE_DELETE_MAP(m_MapRevengeConditions);
}
#endif