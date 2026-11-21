
#include "stdafx.h"
#include "DNWaitUserManager.h"
#include "Log.h"
#include "DNExtManager.h"
#include "DNDivisionManager.h"

extern TMasterConfig g_Config;
CDNWaitUserManager * g_pWaitUserManager = NULL;

CDNWaitUserManager::CDNWaitUserManager()
{
	m_nWorldMaxUser = 0;
	m_nWorldCurUser = 0;
	m_nWorldPreCurUser = 0;	
}

CDNWaitUserManager::~CDNWaitUserManager()
{
}

bool CDNWaitUserManager::Initialize(int nMaxuser)
{
	if (nMaxuser < 0)
	{
		ScopeLock <CSyncLock> sync(m_Sync);
		m_nWorldMaxUser = g_pExtManager->GetWorldMaxUser(g_Config.nWorldSetID);
	}
	else
	{
		//Forced setting section
		{
			ScopeLock <CSyncLock> sync(m_Sync);
			m_nWorldMaxUser = nMaxuser;
		}
		if (g_pDivisionManager)
			UpdateCurCount(g_pDivisionManager->GetCurUserCount());
	}
	return true;
}

void CDNWaitUserManager::UpdateCurCount(UINT nCurCount)
{
	if (m_WaitUserList.empty()) return;

	ScopeLock <CSyncLock> sync(m_Sync);
	if (m_nWorldMaxUser > nCurCount || m_nWorldPreCurUser > nCurCount)
		WaitUserProcessAsync(nCurCount);
	m_nWorldPreCurUser = m_nWorldCurUser = nCurCount;
}

bool CDNWaitUserManager::IsWaitUser(UINT nAccountDBID)
{
	ScopeLock <CSyncLock> sync(m_Sync);

	std::list <_WAITUSER>::iterator ii;
	for (ii = m_WaitUserList.begin(); ii != m_WaitUserList.end(); ii++)
		if ((*ii).nAccountDBID == nAccountDBID)
			return true;
	return false;
}

int CDNWaitUserManager::AddWaitUser(int nServerID, UINT nAccountDBID, USHORT &nLeftTicketNum, USHORT &nLastEstimateCalcTime)
{
	if (IsWaitUser(nAccountDBID))
	{
		g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, nAccountDBID, 0, 0, L"CDNWaitUserManager::AddWaitUser Fail AlreadyAdd\n");
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	ScopeLock <CSyncLock> sync(m_Sync);

	_WAITUSER wait;

	wait.nServerID = nServerID;
	wait.nAccountDBID = nAccountDBID;
	wait.nAddTimeTick = timeGetTime();
	wait.nWaitTicketNum = (USHORT)m_WaitUserList.size()+1;

	//Calculate estimated time based on current time and send it.
	nLeftTicketNum = wait.nWaitTicketNum;
	nLastEstimateCalcTime = CalcEstimateTime(nLeftTicketNum, m_nWorldCurUser);

	m_WaitUserList.push_back(wait);

	return ERROR_NONE;
}

void CDNWaitUserManager::DelWaitUser(int nServerID, UINT nAccountDBID)
{
	ScopeLock <CSyncLock> sync(m_Sync);

	USHORT nDelTicketNum = 0;
	std::list <_WAITUSER>::iterator ii;
	for (ii = m_WaitUserList.begin(); ii != m_WaitUserList.end(); ii++)
	{
		if ((*ii).nAccountDBID == nAccountDBID)
		{
			nDelTicketNum = (*ii).nWaitTicketNum;
			m_WaitUserList.erase(ii);
			break;
		}
	}

	if (nDelTicketNum > 0)
	{
		//update ticketnum
		for (ii = m_WaitUserList.begin(); ii != m_WaitUserList.end(); ii++)
		{
			if ((*ii).nWaitTicketNum > nDelTicketNum)
				(*ii).nWaitTicketNum--;
		}
		UpdateProcessIndexAsync(nDelTicketNum, 1);
	}
}

UINT CDNWaitUserManager::GetWaitUserCount()
{
	ScopeLock <CSyncLock> sync(m_Sync);
	return (UINT)m_WaitUserList.size();
}

USHORT CDNWaitUserManager::CalcEstimateTime(UINT nLeftWaitUser, UINT nWorldUserCount)
{
	USHORT nEstimate = (USHORT)((((float)(1.0f / (((float)nWorldUserCount / 2.0f) / 60.0f)) * (float)nLeftWaitUser) * 2.8f) + 0.5f);
	return nEstimate <= 0 ? 5 : nEstimate;
}

void CDNWaitUserManager::WaitUserProcessAsync(UINT nCurCnt)
{
	int i, nProcessCnt = m_nWorldMaxUser - nCurCnt;
	if (nProcessCnt <= 0) return;

	std::vector <int> vServerList;
	std::vector <std::pair<int, UINT>> vProcessList;
	std::list <_WAITUSER>::iterator ii;
	for (ii = m_WaitUserList.begin(), i = 0; ii != m_WaitUserList.end() && i < nProcessCnt;)
	{
		std::vector <int>::iterator ih = std::find(vServerList.begin(), vServerList.end(), (*ii).nServerID);
		if (ih == vServerList.end()) vServerList.push_back((*ii).nServerID);

		vProcessList.push_back(std::make_pair((*ii).nServerID, (*ii).nAccountDBID));
		ii = m_WaitUserList.erase(ii);
		i++;
		if (i >= WAITPROCESSMAX) break;
	}

	MALOWaitUserProcess Process;
	std::vector <std::pair<int, UINT>>::iterator iv;
	//construct
	for (i = 0; i < (int)vServerList.size(); i++)
	{
		memset(&Process, 0, sizeof(MALOWaitUserProcess));
		Process.nCurCount = nCurCnt;
		for (iv = vProcessList.begin(); iv != vProcessList.end(); iv++)
		{
			if ((*iv).first == vServerList[i])
			{
				Process.nAccountArr[Process.nCount] = (*iv).second;
				Process.nCount++;
			}
		}

		g_pDivisionManager->SendWaitProcess(vServerList[i], &Process);
	}

	for (ii = m_WaitUserList.begin(); ii != m_WaitUserList.end(); ii++)
		(*ii).nWaitTicketNum = (*ii).nWaitTicketNum - nProcessCnt;

	UpdateProcessIndexAsync(0, nProcessCnt);
}

void CDNWaitUserManager::UpdateProcessIndexAsync(int nIndex, int nCount)
{
	//Caller must acquire synchronization lock first.
	std::vector <std::pair<int, int>>::iterator ii;
	for (ii = m_OutList.begin(); ii != m_OutList.end(); ii++)
	{
		if ((*ii).first == nIndex)
			break;
	}

	if (ii != m_OutList.end())
		(*ii).second += nCount;
	else
		m_OutList.push_back(std::make_pair(nIndex, nCount));
}
