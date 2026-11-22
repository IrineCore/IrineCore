#include "stdafx.h"
#include "DNNpcObjectManager.h"
#include "DNNpcObject.h"

CDNNpcObjectManager::CDNNpcObjectManager(void): CDNBaseObjectManager()
{
}

CDNNpcObjectManager::~CDNNpcObjectManager(void)
{
}

CDNNpcObject* CDNNpcObjectManager::FindNpcByNpcID(int nNpcID)
{
	if (nNpcID <= 0) return NULL;

	ScopeLock<CSyncLock> lock(m_VecLock);
	for (size_t i = 0; i < m_VecObjects.size(); ++i)
	{
		CDNNpcObject* pNpc = static_cast<CDNNpcObject*>(m_VecObjects[i]);
		if (pNpc && pNpc->GetNpcData() && pNpc->GetNpcData()->nNpcID == nNpcID)
			return pNpc;
	}
	return NULL;
}
