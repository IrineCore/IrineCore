#include "stdafx.h"
#include "HttpClientManager.h"

CHttpClientManager * g_pHttpClientManager = NULL;

CHttpClientManager::CHttpClientManager()
{
	Clear();
}

CHttpClientManager::~CHttpClientManager()
{
	Clear();
}

void CHttpClientManager::Clear()
{
	TVecHttpClient::iterator iter = m_pVecHttpClient.begin();
	for (; m_pVecHttpClient.end() != iter ; ++iter) {
		CHttpClient* pHttpClient = (*iter);		
		delete pHttpClient;
	}
	m_pVecHttpClient.clear();

	while(!m_pQueueHttpClient.empty()) {
		m_pQueueHttpClient.pop();
	}
}

void CHttpClientManager::CreateHttpClinet(int nWorkerThreadSize, const TCHAR* szServerAddress, const TCHAR* szUserName, const TCHAR* szPassWord)
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	int nConnectionCount = nWorkerThreadSize == 0 ? (SysInfo.dwNumberOfProcessors * 2) + 1 : nWorkerThreadSize;

	for (int i = 0; i < nConnectionCount; i++)
	{
		CHttpClient * pHttpClient = new CHttpClient();
		pHttpClient->Open(szServerAddress, CHttpClient::EV_HTTP_CONNECTION_KEEPALIVE, DF_DEFAULT_HTTP_PORTNUMBER, szUserName, szPassWord);
		m_pVecHttpClient.push_back(pHttpClient);
		m_pQueueHttpClient.push(pHttpClient);
	}	
}

CHttpClient* CHttpClientManager::FindHttpClient()
{
	CHttpClient* pHttpClient = NULL;
	{
		ScopeLock<CSyncLock> aLock(m_Sync);

		DWORD dwThreadID = ::GetCurrentThreadId();		

		TMapHttpClient::iterator iter = m_pMapHttpClient.find(dwThreadID);
		if (m_pMapHttpClient.end() != iter) {
			pHttpClient = (iter->second);
			LONG nRefCount = pHttpClient->IncRefCount();
			if (1 >= nRefCount) {
				DN_ASSERT(0,	"Check!");
			}
			return pHttpClient;
		}

		if (m_pQueueHttpClient.empty()) {
			return NULL;
		}

		pHttpClient = m_pQueueHttpClient.front();
		LONG nRefCount = pHttpClient->IncRefCount();
		if (1 != nRefCount) {
			DN_ASSERT(0,	"Check!");
		}
		m_pQueueHttpClient.pop();
		m_pMapHttpClient.insert(TMapHttpClient::value_type(dwThreadID, pHttpClient));
	}
	return pHttpClient;
}

void CHttpClientManager::FreeHttpClient(CHttpClient* pHttpClient)
{
	if( pHttpClient )
	{		
		ScopeLock<CSyncLock> aLock(m_Sync);

		LONG nRefCount = pHttpClient->DecRefCount();
		if (0 == nRefCount) {
			m_pMapHttpClient.erase(::GetCurrentThreadId());
			m_pQueueHttpClient.push(pHttpClient);
		}		
	}
}