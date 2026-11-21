#include "stdafx.h"
#include "DNBaseObjectManager.h"
#include "Util.h"
#include "Log.h"

extern TVillageConfig g_Config;

CDNBaseObjectManager::CDNBaseObjectManager(void): m_nProcessCount(0)
{
	Clear();
}

CDNBaseObjectManager::~CDNBaseObjectManager(void)
{
	if (m_VecObjects.empty()) return;

	CDNBaseObject *pObj = NULL;

	m_VecLock.Lock();
	for (int i = 0; i < (int)m_VecObjects.size(); i++){
		pObj = m_VecObjects[i];
		if (!pObj) continue;
		pObj->LeaveWorld();
		pObj->FinalObject();
		delete pObj;
		pObj = NULL;
	}
	m_VecLock.UnLock();

	Clear();
}

void CDNBaseObjectManager::Clear()
{
	m_KeyLock.Lock();
	m_NameKeyLock.Lock();
	m_VecLock.Lock();

	m_KeyObjects.clear();
	m_NameKeyObjects.clear();
	m_VecObjects.clear();

	m_VecLock.UnLock();
	m_NameKeyLock.UnLock();
	m_KeyLock.UnLock();
}

void CDNBaseObjectManager::DoUpdate(DWORD CurTick)
{
	if (m_VecObjects.empty()) return;

	int nCount;
	CDNBaseObject *pObj = NULL;

	m_VecLock.Lock();
	nCount = (int)m_VecObjects.size();
	if (nCount > PROCESSCOUNT) nCount = PROCESSCOUNT;

	for (int i = 0; i < nCount; i++){
		if (m_nProcessCount >= (int)m_VecObjects.size())
			m_nProcessCount = 0;

		pObj = m_VecObjects[m_nProcessCount];
		if (pObj) {
			if (pObj->boAllowDelete())
				DelObject(pObj);
			else
				pObj->DoUpdate(CurTick);
		}

		m_nProcessCount++;
	}
	m_VecLock.UnLock();	
}

bool CDNBaseObjectManager::AddObject(CDNBaseObject *pObj)
{
	if (!pObj) return false;
	if (FindObject(pObj->GetObjectID())){
		g_Log.Log(LogType::_ERROR, L"[ObjID: %u] CDNBaseObjectManager::AddObject Duplicate!!\r\n", pObj->GetObjectID());
		return false;
	}

	if (g_IDGenerator.IsUser(pObj->GetObjectID()))
	{
		if (FindObjectByName(pObj->wszName())){
			g_Log.Log(LogType::_ERROR, L"[ObjID: %s] CDNBaseObjectManager::AddObject Duplicate!!\r\n", pObj->wszName());
			return false;
		}
	}

	m_VecLock.Lock();
	TVecObjects::iterator it = find(m_VecObjects.begin(), m_VecObjects.end(), pObj);
	if( it != m_VecObjects.end() )
	{
		m_VecLock.UnLock();
		_DANGER_POINT();
		return false;
	}
	m_VecObjects.push_back(pObj);
	m_VecLock.UnLock();

	m_KeyLock.Lock();
	m_KeyObjects[pObj->GetObjectID()] = pObj;
	m_KeyLock.UnLock();

	if (g_IDGenerator.IsUser(pObj->GetObjectID())){
		m_NameKeyLock.Lock();
		m_NameKeyObjects[pObj->wszName()] = pObj;
		m_NameKeyLock.UnLock();
	}

	return true;
}

bool CDNBaseObjectManager::DelObject(CDNBaseObject *pObj)
{
	if (!pObj) return false;

	if (g_IDGenerator.IsUser(pObj->GetObjectID())){

		ScopeLock<CSyncLock> lock( m_NameKeyLock );
		
		TMapNameObjects::iterator itername = m_NameKeyObjects.find(pObj->wszName());
		if (itername != m_NameKeyObjects.end()){
			if (itername->second != pObj){
				g_Log.Log(LogType::_ERROR, L"[ObjID:%u] CDNBaseObjectManager::DelObject m_NameKeyObjects invalid (%s) %x %x\r\n", pObj->GetObjectID(), pObj->wszName(), itername->second, pObj);
			}
			m_NameKeyObjects.erase(itername);
		}
		else
		{
			for (itername = m_NameKeyObjects.begin(); itername != m_NameKeyObjects.end(); itername++)
			{
				if ((*itername).second == (pObj))
				{
					//In this case find failed. Judging by container size mismatches, some entries seem not to be erased properly on deletion.
					g_Log.Log(LogType::_DELOBJECT_NAMEKEYFINDERR, L"[ObjID:%u] CDNBaseObjectManager::DelObject m_NameKeyObjects FindErr (%s) %x %x\r\n", pObj->GetObjectID(), pObj->wszName(), itername->second, pObj);
					m_NameKeyObjects.erase(itername);
					break;
				}
			}
		}
	}

	bool boFlag = false;

	m_KeyLock.Lock();
	TMapObjects::iterator iter = m_KeyObjects.find(pObj->GetObjectID());
	if (iter != m_KeyObjects.end()){
		if (iter->second == pObj){
			m_KeyObjects.erase(iter);
			boFlag = true;
		}
	}
	m_KeyLock.UnLock();

	ScopeLock<CSyncLock> lock( m_VecLock );
	TVecObjects::iterator it = find(m_VecObjects.begin(), m_VecObjects.end(), pObj);
	if (it != m_VecObjects.end()){
		m_VecObjects.erase(it);
		return true;
	}
	else {
		for (int i = 0; i < (int)m_VecObjects.size(); i++){
			if (m_VecObjects[i]->GetObjectID() == pObj->GetObjectID()){
				g_Log.Log(LogType::_ERROR, L"[ObjID:%u] CDNBaseObjectManager::DelObject m_VecObjects invalid %x %x\r\n", pObj->GetObjectID(), m_VecObjects[i], pObj);
				m_VecObjects.erase(m_VecObjects.begin() + i);
				return true;
			}
		}
	}

	g_Log.Log(LogType::_ERROR, L"[%d] [ObjID:%u] [CDNBaseObjectManager::DelObject] Fail!! Name:%d, Key:%d, Vec:%d\r\n", g_Config.nManagedID, pObj->GetObjectID(), m_NameKeyObjects.size(), m_KeyObjects.size(), m_VecObjects.size());
	return false;
}

CDNBaseObject* CDNBaseObjectManager::FindObject(UINT nUID)
{
	if (m_KeyObjects.empty()) return NULL;
	if (nUID <= 0) return NULL;

	CDNBaseObject *pObj = NULL;
	m_KeyLock.Lock();
	TMapObjects::iterator iter = m_KeyObjects.find(nUID);
	if (iter != m_KeyObjects.end()){
		pObj = iter->second;
		m_KeyLock.UnLock();
		return pObj;
	}

	m_KeyLock.UnLock();
	return NULL;
}

CDNBaseObject* CDNBaseObjectManager::FindObjectByName(const WCHAR *pwszName)
{
	if (m_NameKeyObjects.empty()) return NULL;

	CDNBaseObject *pObj = NULL;
	m_NameKeyLock.Lock();
	TMapNameObjects::iterator iter = m_NameKeyObjects.find(pwszName);
	if (iter != m_NameKeyObjects.end()){
		pObj = iter->second;
		m_NameKeyLock.UnLock();
		return pObj;
	}
	m_NameKeyLock.UnLock();
	return NULL;
}

int CDNBaseObjectManager::GetObjCount()
{
	m_VecLock.Lock();
	int nCount = (int)m_VecObjects.size();
	m_VecLock.UnLock();

	return nCount;
}

bool CDNBaseObjectManager::IsEmptyObject()
{
	if (m_KeyObjects.empty() && m_VecObjects.empty()) return true;
	return false;
}
