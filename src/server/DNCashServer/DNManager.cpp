#include "stdafx.h"
#include "DNManager.h"
#include "DNUserRepository.h"

CDNManager *g_pManager = NULL;

CDNManager::CDNManager(void)
{
	m_pMapUserList.clear();
}

CDNManager::~CDNManager(void)
{
	SAFE_DELETE_MAP(m_pMapUserList);
}

CDNUserRepository *CDNManager::AddUser(UINT nAccountDBID)
{
	if (nAccountDBID == 0) return NULL;

	CDNUserRepository *pUser = GetUser(nAccountDBID);
	if (pUser) return pUser;

	pUser = new CDNUserRepository;
	pUser->SetAccountDBID(nAccountDBID);

	ScopeLock<CSyncLock> Lock(m_UserLock);

	std::pair<TMapUser::iterator, bool> RetVal = m_pMapUserList.insert(TMapUser::value_type(pUser->GetAccountDBID(), pUser));
	if (!RetVal.second) {
		delete pUser;
		return NULL;
	}

	m_pMapUserList[pUser->GetAccountDBID()] = pUser;		// Just set it directly. 
	return pUser;
}

bool CDNManager::DelUser(UINT nAccountDBID)
{
	if (m_pMapUserList.empty()) return false;

	ScopeLock<CSyncLock> Lock(m_UserLock);

	TMapUser::iterator iter = m_pMapUserList.find(nAccountDBID);
	if (iter == m_pMapUserList.end()) return false;

	CDNUserRepository *pUser = iter->second;
	m_pMapUserList.erase(iter);
	SAFE_DELETE(pUser);
	return true;
}

CDNUserRepository *CDNManager::GetUser(const char* pszAccountName)
{
	if (m_pMapUserList.empty()) return NULL;

	ScopeLock<CSyncLock> Lock(m_UserLock);

	TMapUser::iterator iter;
	for (iter = m_pMapUserList.begin(); iter != m_pMapUserList.end(); ++iter){
		if (strcmp(iter->second->GetAccountName(), pszAccountName) == 0)
			return iter->second;
	}

	return NULL;
}

CDNUserRepository *CDNManager::GetUser(UINT nAccountDBID)
{
	if (m_pMapUserList.empty()) return NULL;

	ScopeLock<CSyncLock> Lock(m_UserLock);

	TMapUser::iterator iter = m_pMapUserList.find(nAccountDBID);
	if (iter == m_pMapUserList.end()) return NULL;

	return iter->second;
}
