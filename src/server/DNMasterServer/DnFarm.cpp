
#include "stdafx.h"
#include "Log.h"
#include "DNFarm.h"
#include "DNDivisionManager.h"


#if defined( PRE_ADD_FARM_DOWNSCALE )
CDNFarm::CDNFarm( const TFarmItemFromDB& Farm )
{
	m_nFarmDBID = Farm.iFarmDBID;
	m_eFarmStatus = FARMSTATUS_NONE;
	m_nMapID = Farm.iFarmMapID;
	m_nCreateTick = timeGetTime();
	m_nFarmMaxUser = 0;
	m_bStartFarm = Farm.bStartActivate;
	m_iAttr = Farm.iAttr;
	m_nManagedID = 0;

	ResetData();
}
#elif defined( PRE_ADD_VIP_FARM )
CDNFarm::CDNFarm( const TFarmItemFromDB& Farm )
{
	m_nFarmDBID = Farm.iFarmDBID;
	m_eFarmStatus = FARMSTATUS_NONE;
	m_nMapID = Farm.iFarmMapID;
	m_nCreateTick = timeGetTime();
	m_nFarmMaxUser = 0;
	m_bStartFarm = Farm.bStartActivate;
	m_Attr = Farm.Attr;
	m_nManagedID = 0;

	ResetData();
}
#else
CDNFarm::CDNFarm(UINT nFarmDBID, int nMapID, bool bStart)
{
	m_nFarmDBID = nFarmDBID;
	m_eFarmStatus = FARMSTATUS_NONE;
	m_nMapID = nMapID;
	m_nCreateTick = timeGetTime();
	m_nFarmMaxUser = 0;
	m_bStartFarm = bStart;

	ResetData();
}
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )

CDNFarm::~CDNFarm()
{
	m_vWaitingUser.clear();
}

void CDNFarm::DestroyFarm()
{
	if (m_eFarmStatus == FARMSTATUS_DESTROY)
		return;

	ResetData();
	m_eFarmStatus = FARMSTATUS_DESTROY;
}

bool CDNFarm::SetAssignedServerID(int nGameServerID)
{
	if (m_eFarmStatus != FARMSTATUS_NONE && m_eFarmStatus != FARMSTATUS_DESTROY)
	{
		g_Log.Log(LogType::_FARM, L"FarmID[%d] SetAssignedServerID Fail Reason[StateMisMatch]\n", GetFarmDBID());
		return false;
	}

	m_nAssignedGameID = nGameServerID;
	m_eFarmStatus = FARMSTATUS_READY;
	return true;
}

bool CDNFarm::SetAssignedFarmData(int nGameServerID, int nGameThreadIdx, UINT nRoomID, int nMapID, int nFarmMaxUser)
{
	//Timing when game server finishes loading and signals it can accept users.
	if (m_eFarmStatus != FARMSTATUS_READY)
	{
		g_Log.Log(LogType::_FARM, L"FarmID[%d] SetAssignedFarmData Fail Reason[StateMisMatch]\n", GetFarmDBID());
		return false;		//Setting can only be done once initially; if this happens, it's a bug!
	}

	if ((m_nAssignedGameID != nGameServerID) || m_nMapID != nMapID)
	{
		//What is this!!!!!!!!!!!!!
		g_Log.Log(LogType::_FARM, L"FarmID[%d] SetFarmDataLoaded Fail Reason[DataMisMatch]\n", GetFarmDBID());
		return false;
	}

	m_nAssignedThreadIdx = nGameThreadIdx;
	m_nAssignedRoomID = nRoomID;
	m_nMapID = nMapID;
	m_nPlayTick = timeGetTime();
	m_eFarmStatus = FARMSTATUS_PLAY;
	m_nFarmMaxUser = nFarmMaxUser;
	
	//When entering play state, push in waiting entities.
	ProcessFarmWaiting();
	return true;
}
void CDNFarm::SetFarmCurUserCount(int nFarmCurUserCount, bool bStarted, int nManagedID)
{
	m_nFarmCurUser = nFarmCurUserCount;
	m_bStartFarm = bStarted;
	m_nManagedID = nManagedID;
}

bool CDNFarm::VerifyWaitAndPush(const VIMAReqGameID * pPacket)
{
	if (m_eFarmStatus == FARMSTATUS_READY || m_eFarmStatus == FARMSTATUS_NONE)
	{
		std::vector <VIMAReqGameID>::iterator ii;
		for (ii = m_vWaitingUser.begin(); ii != m_vWaitingUser.end(); ii++)
		{
			if ((*ii).InstanceID == pPacket->InstanceID)
			{
				_DANGER_POINT();	//Already exists! This shouldn't happen!
				return false;
			}
		}

		VIMAReqGameID req;
		memcpy(&req, pPacket, sizeof(VIMAReqGameID));

		m_vWaitingUser.push_back(req);		//Push in waiting users.
		return true;
	}
	return false;
}

bool CDNFarm::GetAssignedServerInfo(int &nGameServerID, int &nGameServerThreaIdx, UINT &nGameRoomID)
{
	if (m_eFarmStatus == FARMSTATUS_NONE)			//There is no room info when only created.
		return false;

	nGameServerID = m_nAssignedGameID;
	nGameServerThreaIdx = m_nAssignedThreadIdx;
	nGameRoomID = m_nAssignedRoomID;
	return true;
}

void CDNFarm::ResetData()
{
	m_nPlayTick = 0;
	m_nAssignedGameID = -1;
	m_nAssignedThreadIdx = -1;
	m_nAssignedRoomID = 0;
	m_nFarmCurUser = 0;

	m_vWaitingUser.clear();
}

void CDNFarm::ProcessFarmWaiting()
{
	//Push in users requesting entry during room init or creation. Just pushing them in for now, but to protect state
	//Should add a few state checks.
	std::vector <VIMAReqGameID>::iterator ii;
	for (ii = m_vWaitingUser.begin(); ii != m_vWaitingUser.end(); ii++)
	{
		if (g_pDivisionManager->RequestGameRoom(&(*ii)) == false)
		{
			//Handling on entry failure
		}
	}
	m_vWaitingUser.clear();
}
