
#include "stdafx.h"
#include "DNUserDeleteManager.h"
#include "Util.h"
#include "DNManager.h"

CDNUserDeleteManager* g_pUserDeleteManager = NULL;


CDNUserDeleteManager::CDNUserDeleteManager()
{
	
}

bool CDNUserDeleteManager::Add(UINT pAccountDBID, bool bDoLock)
{
	DN_ASSERT(0 != pAccountDBID,	"Invalid!");

	TP_LOCK* pLock = (bDoLock)?(&m_Lock):(NULL);
	if (pLock) {
		DN_ASSERT(!m_Lock.IsLock(),	"Already Locked!");		// !!! Caution - When using a lock, it must not already be locked externally
	}

	TP_LOCKAUTO aLock(pLock);
	if (!pLock) {
		DN_ASSERT(m_Lock.IsLock(),	"Not Locked!");			// !!! Caution - When not using a lock, locking must be performed externally
	}

	std::pair<TP_LIST_ITR, bool> aRetVal = m_List.insert(TP_LIST::value_type(pAccountDBID, USERDELDAT(pAccountDBID)));
	if (!aRetVal.second) {
		USERDELDAT* aUserDelDat = (&(aRetVal.first->second));
		DN_ASSERT(NULL != aUserDelDat,	"Invalid!");
		DN_ASSERT(aUserDelDat->IsSet(),	"Invalid!");

		// For already added users, only delay the deletion interval
		aUserDelDat->SetTick();
		return false;
	}
	
	return true;
}

void CDNUserDeleteManager::Remove(UINT pAccountDBID, bool bDoLock)
{
	DN_ASSERT(0 != pAccountDBID,	"Invalid!");

	TP_LOCK* pLock = (bDoLock)?(&m_Lock):(NULL);
	if (pLock) {
		DN_ASSERT(!m_Lock.IsLock(),	"Already Locked!");		// !!! Caution - When using a lock, it must not already be locked externally
	}

	TP_LOCKAUTO aLock(pLock);
	if (!pLock) {
		DN_ASSERT(m_Lock.IsLock(),	"Not Locked!");			// !!! Caution - When not using a lock, locking must be performed externally
	}

	m_List.erase(pAccountDBID);
}

bool CDNUserDeleteManager::IsExist(UINT pAccountDBID, bool bDoLock) const
{
	DN_ASSERT(0 != pAccountDBID,	"Invalid!");

	TP_LOCK* pLock = (bDoLock)?(&m_Lock):(NULL);
	if (pLock) {
		DN_ASSERT(!m_Lock.IsLock(),	"Already Locked!");		// !!! Caution - When using a lock, it must not already be locked externally
	}

	TP_LOCKAUTO aLock(pLock);
	if (!pLock) {
		DN_ASSERT(m_Lock.IsLock(),	"Not Locked!");			// !!! Caution - When not using a lock, locking must be performed externally
	}

	TP_LIST_CTR aIt = m_List.find(pAccountDBID);
	if (m_List.end() != aIt) {
		return true;
	}

	return false;
}

void CDNUserDeleteManager::GetList(TP_LISTAUTO& pList, bool bDoLock) const
{
	TP_LOCK* pLock = (bDoLock)?(&m_Lock):(NULL);
	if (pLock) {
		DN_ASSERT(!m_Lock.IsLock(),	"Already Locked!");		// !!! Caution - When using a lock, it must not already be locked externally
	}

	TP_LOCKAUTO aLock(pLock);
	if (!pLock) {
		DN_ASSERT(m_Lock.IsLock(),	"Not Locked!");			// !!! Caution - When not using a lock, locking must be performed externally
	}

	TP_LIST_CTR aIt = m_List.begin();
	for (; m_List.end() != aIt ; ++aIt) {
		pList.push_back(aIt->second);
	}
}

void CDNUserDeleteManager::DoUpdate()
{
	DN_ASSERT(!m_Lock.IsLock(),	"Invalid!");

	// NOTE - This method must be executed on the same thread as packet processing for safe release of CDNUserRepository.

	TP_LISTAUTO aList;
	{
		TP_LOCKAUTO aLock(&m_Lock);

		GetList(aList, false);
	}
	if (aList.empty()) {
		return;
	}

	DWORD aCurTick = ::GetTickCount();

	TP_LISTAUTO_CTR aIt = aList.begin();
	for (; aList.end() != aIt ; ++aIt) {
		const USERDELDAT* aUserDelDat = (&(*aIt));
		DN_ASSERT(NULL != aUserDelDat,	"Invalid!");
		DN_ASSERT(aUserDelDat->IsSet(),	"Invalid!");

		if (CASHDELUSERDELAY_LIMITTICK <= GetTickTerm(aUserDelDat->m_OldTick, aCurTick)) {
			bool bRetVal = g_pManager->DelUser(aUserDelDat->m_AccountDBID);
			if (!bRetVal) {
				// ??
			}

			Remove(aUserDelDat->m_AccountDBID, true);
		}
	}
}
