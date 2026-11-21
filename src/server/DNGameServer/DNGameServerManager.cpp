
#include "stdafx.h"
#include "DNIocpManager.h"
#include "DNGameServerManager.h"
#include "DNRUDPGameServer.h"
#include "DNUserSession.h"
#include "DNGameRoom.h"
#include "DNUserTcpConnection.h"
#include "DNBackGroundLoader.h"
#include "Log.h"

extern TGameConfig g_Config;
CDNGameServerManager * g_pGameServerManager = NULL;

CDNGameServerManager::CDNGameServerManager()
{
	if (InitializeRUDP() == false)
		g_Log.Log(LogType::_ERROR, L"UDP Init Err\n");

#ifndef _SKIP_BLOCK
	m_nLastCheckTick = 0;
#endif

	MAGAReqRoomID Packet;
	memset( &Packet, 0, sizeof(Packet) );
	m_pRootRoom = new CDNGameRoom( NULL, 0, &Packet );

	m_GameRoomList.reserve( MAX_SESSION_COUNT );
	m_RoomCountInfo.reserve( MAX_SESSION_COUNT );

	CDNGameServerManager::sRoomstate Room;

	for( UINT i=0 ; i<MAX_SESSION_COUNT ; ++i )
	{
		m_GameRoomList.push_back(Room);
		m_RoomCountInfo.push_back( std::make_pair(0,0) );
	}

	m_uiRoomCount = 0;
	m_bZeroPopulation = false;
	m_iThreadCount = 0;
}
	
CDNGameServerManager::~CDNGameServerManager()
{
	g_Log.Log(LogType::_NORMAL, L"graceful close server\n");

	for (unsigned int i = 0; i < m_GameServerList.size(); i++)
		delete m_GameServerList[i];
	m_GameServerList.clear();
	m_pRootRoom->FinalizeGameRoom();
	SAFE_DELETE( m_pRootRoom );
	SAFE_DELETE(g_pBackLoader);
}

bool CDNGameServerManager::InitializeRUDP()
{
	WSADATA	WSAData;
	if (WSAStartup(MAKEWORD(1, 1), &WSAData))
		return false;
	return true;
}

void CDNGameServerManager::CalcOpenCount(int nProcessorCount, int nCreateCount, int nCreateIndex, int &nOpenCount, int &nStartAffinityCount, int &nBackLoaderIndex)
{
	int nDivine = nProcessorCount/(nCreateCount == 0 ? 1 : nCreateCount);
	//int nOpenMinus = nDivine > 4 ? 2 : 1;
	int nOpenMinus = 1;

	nOpenCount = (nDivine - nOpenMinus) <= 0 ? 1 : nDivine - nOpenMinus;
	nStartAffinityCount = nDivine * nCreateIndex;
	nBackLoaderIndex = (nDivine * (nCreateIndex + 1)) - nOpenMinus;
}

void CDNGameServerManager::PreOpenGameServer()
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	int nOpenThreadCount, nAffinityStartCount, nBackLoaderAffinityIndex;
	CalcOpenCount(SysInfo.dwNumberOfProcessors, g_Config.nCreateCount, g_Config.nCreateIndex, nOpenThreadCount, nAffinityStartCount, nBackLoaderAffinityIndex);

	m_iThreadCount = nOpenThreadCount;
}

bool CDNGameServerManager::StartGameServer(USHORT nBeginAcceptPort, const char * pExternalProbeIP, unsigned short nExternalProbePort)
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	int nOpenThreadCount, nAffinityStartCount, nBackLoaderAffinityIndex;
	CalcOpenCount(SysInfo.dwNumberOfProcessors, g_Config.nCreateCount, g_Config.nCreateIndex, nOpenThreadCount, nAffinityStartCount, nBackLoaderAffinityIndex);

	m_iThreadCount = nOpenThreadCount;
	
	int nThreadCount;
	for (nThreadCount = 0; nThreadCount < m_iThreadCount; nThreadCount++, nAffinityStartCount++)
	{//Woohoo~ let's go!
		CDNRUDPGameServer * pServer = new CDNRUDPGameServer(this, pExternalProbeIP, nExternalProbePort);
		if (pServer->Start(nBeginAcceptPort+nThreadCount, nThreadCount, nAffinityStartCount) == false)
		{
			_tprintf(_T("Cant Open RUDP Port [cnt:%d][%d]\n"), nThreadCount, nBeginAcceptPort+nThreadCount);
			return false;
		}

		m_GameServerList.push_back(pServer);
		Sleep(10);
	}

	g_pBackLoader = new CDNBackGroundLoader;
	if (!g_pBackLoader)	
		return false;
	
	if( !g_pBackLoader->SetThreadAffinityMask( 1 << nBackLoaderAffinityIndex ) )
	{
		std::cout << "SetThreadAffinityMask() Failed!!!" << std::endl;
		return false;
	}

	return true;
}

void CDNGameServerManager::GetGameServerInfo(unsigned long * iIP, unsigned short * iPort, unsigned char * cIdx, bool * margin)
{
	if (iIP == NULL || iPort == NULL)
	{
		//Should not reach here, right? How was this coded~
		_DANGER_POINT();
		return;
	}

	//If info grows (e.g., server supporting special game types), change to a struct later
	for (int i = 0; i < (int)m_GameServerList.size(); i++)
	{
		m_GameServerList[i]->GetAddress(&iIP[i], &iPort[i]);
		cIdx[i] = m_GameServerList[i]->GetServerID();
		margin[i] = m_GameServerList[i]->IsAcceptRoom();
	}
}

CDNRUDPGameServer * CDNGameServerManager::GetGameServer()
{
	//Check the Process call timing of each UDP server and assign to the one running with lower load.
	//Getting the server via this function only happens at initial creation time!
	ScopeLock<CSyncLock>	Lock( m_Lock );

	//Check if a randomly drawn item can be received!
	int nCnt = 0;
	CDNRUDPGameServer* pServer = NULL;
	while (nCnt < 24)
	{
		unsigned int id;
		errno_t err = rand_s(&id);
		if (err == 0)
		{
			pServer = m_GameServerList[(unsigned int)((double)id / (double)UINT_MAX * m_GameServerList.size())];
			if (pServer && pServer->IsAcceptRoom())
				return pServer;
		}
		else _DANGER_POINT();
		nCnt++;
	}
	_DANGER_POINT();
	return pServer;
}

CDNRUDPGameServer * CDNGameServerManager::GetGameServerByUID(unsigned int iUID)
{
	ScopeLock<CSyncLock> Lock( m_Lock );
	
	CDNRUDPGameServer * pServer = NULL;
	std::map <unsigned int, CDNRUDPGameServer*>::iterator ii = m_UserConnectionInfoList.find(iUID);
	if (ii != m_UserConnectionInfoList.end())
		pServer = (*ii).second;
	else
		_DANGER_POINT();
	
	return pServer;
}

CDNRUDPGameServer * CDNGameServerManager::GetGameServerIndex(unsigned int index)
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	CDNRUDPGameServer * pServer = NULL;
	if ( index < m_GameServerList.size() )
		pServer = m_GameServerList[index];
	
	return pServer;
}

CDNRUDPGameServer * CDNGameServerManager::GetGameServerByAID(unsigned int iAccountDBID)
{
	ScopeLock<CSyncLock> Lock( m_UserLock );

	CDNRUDPGameServer * pServer = NULL;
	std::map <unsigned int, CDNRUDPGameServer*>::iterator ii = m_UserConnectionInfoListByAccountDBID.find(iAccountDBID);
	if (ii != m_UserConnectionInfoListByAccountDBID.end())
		pServer = (*ii).second;

	return pServer;
}

CDNRUDPGameServer * CDNGameServerManager::GetGameServerByRoomID(unsigned int iRoomID)
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	if( iRoomID < m_GameRoomList.size() )
	{
		return m_GameRoomList[iRoomID].bCrashed == false ? m_GameRoomList[iRoomID].pServer : NULL;
	}
	else
	{
		_DANGER_POINT();
		return NULL;
	}
}

unsigned int CDNGameServerManager::GenRoomID(CDNRUDPGameServer * pServer)
{
	ScopeLock<CSyncLock> Lock( m_Lock );
	
	if( GetRoomCount()+1 >= MAX_SESSION_COUNT )
	{
		g_Log.Log(LogType::_ERROR, L"[%d] CDNGameServerManager::GenRoomID() Room OverFlow %d/%d\n", g_Config.nManagedID, GetRoomCount()+1, MAX_SESSION_COUNT );
		return 0;
	}

	while(true)
	{
		unsigned int id;
		errno_t err = rand_s(&id);
		if( err == 0 )
		{
			unsigned int nRoomID = (unsigned int)((double)id / (double)UINT_MAX * (MAX_SESSION_COUNT-1)) + 1;

			if( nRoomID < m_GameRoomList.size() && m_GameRoomList[nRoomID].bCrashed == false && m_GameRoomList[nRoomID].pServer == NULL )
			{
				m_GameRoomList[nRoomID].bCrashed = false;
				m_GameRoomList[nRoomID].pServer = pServer;
				++m_uiRoomCount;
				return nRoomID;
			}
		}
		else 
			_DANGER_POINT();
	}

	return 0;
}

bool CDNGameServerManager::VerifyUserIDs(UINT nAccountDBID, UINT nSessionID)
{
	std::map <unsigned int, CDNRUDPGameServer*>:: iterator ii = m_UserConnectionInfoList.find(nSessionID);
	if (ii != m_UserConnectionInfoList.end()) return false;
	ii = m_UserConnectionInfoListByAccountDBID.find(nAccountDBID);
	if (ii != m_UserConnectionInfoListByAccountDBID.end())	return false;
	return true;
}

void CDNGameServerManager::AddGameUser(UINT nRoomID, UINT nAccountDBID, UINT nSessionID, CDNRUDPGameServer * pServer)
{
	{
		ScopeLock<CSyncLock> Lock( m_UserLock );

	std::map <unsigned int, CDNRUDPGameServer*>::iterator ii = m_UserConnectionInfoList.find(nSessionID);
	if (ii != m_UserConnectionInfoList.end())
		g_Log.Log(LogType::_ERROR, L"GameServer AddUser Err [SID:%d]\n", nSessionID);
	else
	{
		m_UserConnectionInfoList[nSessionID] = pServer;
		m_UserConnectionInfoListByAccountDBID[nAccountDBID] = pServer;
	}
	}

	{
		ScopeLock<CSyncLock> Lock( m_Lock );
	if( nRoomID < m_RoomCountInfo.size() )
		++m_RoomCountInfo[nRoomID].second;
	else
		_DANGER_POINT();
}
}

void CDNGameServerManager::RemoveGameRoom(unsigned int nRoomID, CDNRUDPGameServer * pServer, bool bCrashed/* = false*/)
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	if( nRoomID < m_GameRoomList.size() && m_GameRoomList[nRoomID].pServer )
	{
#if defined( PRE_THREAD_ROOMDESTROY )
		g_pBackLoader->DestroyConfirm( nRoomID );
#endif // #if defined( PRE_THREAD_ROOMDESTROY )
		m_GameRoomList[nRoomID].bCrashed = bCrashed;
		m_GameRoomList[nRoomID].pServer = NULL;
		
		if(m_uiRoomCount)
			--m_uiRoomCount;	
	}
	else
		_DANGER_POINT();

	if( nRoomID < m_RoomCountInfo.size() )
		m_RoomCountInfo[nRoomID] = std::make_pair(0,0);
	else
		_DANGER_POINT();
}

void CDNGameServerManager::RemoveGameUser(unsigned int nRoomID, unsigned int nAccountDBID, unsigned int nSessionID, CDNRUDPGameServer * pServer)
{
	{
		ScopeLock<CSyncLock> Lock( m_UserLock );

	std::map <unsigned int, CDNRUDPGameServer*>::iterator ii = m_UserConnectionInfoList.find(nSessionID);
	if (ii != m_UserConnectionInfoList.end())
	{
		m_UserConnectionInfoList.erase(ii);
		if (m_UserConnectionInfoListByAccountDBID.find(nAccountDBID) != m_UserConnectionInfoListByAccountDBID.end())
			m_UserConnectionInfoListByAccountDBID.erase(m_UserConnectionInfoListByAccountDBID.find(nAccountDBID));
		else
			_DANGER_POINT();
		}	else
			g_Log.Log(LogType::_ERROR, L"GameServer RemoveUser Err [SID:%d]\n", nSessionID);
		}

	{
		ScopeLock<CSyncLock> Lock( m_Lock );

	if( nRoomID < m_RoomCountInfo.size() )
	{
		if( m_RoomCountInfo[nRoomID].second )
			--m_RoomCountInfo[nRoomID].second;
	}
	else
		_DANGER_POINT();
}
}

void CDNGameServerManager::UpdateRoomCountInfo(unsigned int nRoomID, int nMapIdx)
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	if( nRoomID < m_RoomCountInfo.size() )
		m_RoomCountInfo[nRoomID].first = nMapIdx;
	else
		_DANGER_POINT();
}

void CDNGameServerManager::GetRoomUserCount(UINT &nUserCount, UINT &nRoomCount, UINT &nTotalRoomCount)
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	nUserCount = (UINT)m_UserConnectionInfoList.size();
	nTotalRoomCount = (UINT)m_RoomCountInfo.size();

	nRoomCount = m_uiRoomCount;
}

int CDNGameServerManager::GetRoomCount()
{
	ScopeLock<CSyncLock> Lock( m_Lock );

	return static_cast<int>(m_uiRoomCount);
}

void CDNGameServerManager::CheckCloseGameServer(CDNRUDPGameServer * pServer)
{
	//Once each server thread has deleted all game rooms and sent all data for each user
	bool bCheck = true;
	std::vector <CDNRUDPGameServer*>::iterator ii;
	for (ii = m_GameServerList.begin(); ii != m_GameServerList.end(); ii++)
	{
		if ((*ii)->GetFlushSaveData() == false)
			bCheck = false;
	}

	(void)bCheck;
}

void CDNGameServerManager::MasterDisConnected(int nWorldSetID)
{
	std::vector <CDNRUDPGameServer*>::iterator ii;
	for (ii = m_GameServerList.begin(); ii != m_GameServerList.end(); ii++)
#if defined( PRE_WORLDCOMBINE_PARTY )
		(*ii)->StoreExternalBuffer(0, IN_DESTROY, 0, NULL, 0, EXTERNALTYPE_MASTER, 0, nWorldSetID );
#else
		(*ii)->StoreExternalBuffer(0, IN_DESTROY, 0, NULL, 0, EXTERNALTYPE_MASTER);
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
}

#ifdef _SKIP_BLOCK
CDNTcpConnection * CDNGameServerManager::CreateTcpConnection(const char * pIp, const int nPort)
{
	CDNTcpConnection * pTcpCon = new CDNTcpConnection(this);
	if( pTcpCon == NULL )
		return NULL;

	pTcpCon->SetIp(pIp);
	pTcpCon->SetPort(nPort);

	return pTcpCon;
}
#else
CDNTcpConnection * CDNGameServerManager::CreateTcpConnection(const char * pIp, const int nPort)
{
	ScopeLock<CSyncLock> Lock( m_ConSync );

	CDNTcpConnection * pTcpCon = new CDNTcpConnection(this);
	if( pTcpCon == NULL )
		return NULL;

	_TCPCON * pCon = new _TCPCON;
	if( pCon == NULL )
		return NULL;

	pCon->nCreateTick	= timeGetTime();
	pCon->pCon			= pTcpCon;
	m_TcpConnectionList.push_back(pCon);

	pTcpCon->SetIp(pIp);
	pTcpCon->SetPort(nPort);

	return pTcpCon;
}

void CDNGameServerManager::PushToEjectTcpConnection(CDNTcpConnection * pCon, CDNUserSession * pSession)		//with Async
{
	m_EjectTcpConnectionList.push_back(std::make_pair(pCon, pSession));
}

void CDNGameServerManager::PushOrphanPtr(CDNTcpConnection * pCon)
{
	m_OrphanTcpConnectionList.push_back(pCon);
}

void CDNGameServerManager::FlushConnectionBuffer(ULONG nCurTick)
{
	//If kept here.. annoying single bottleneck can occur.. only store briefly initially; GameServer manages and deletes once set up
	ScopeLock<CSyncLock> Lock( m_ConSync );
	
	for( std::list<_TCPCON*>::iterator ii = m_TcpConnectionList.begin(); ii != m_TcpConnectionList.end(); )
	{
		if ((*ii)->pCon->FlushRecvData(0) == false)
		{
			//What, before even connecting via TCP and moving..... -_-;
			//This only removes it from the temporary container; actual deletion of the Meber object still needs to be handled...
			//No way here... handle zombies by checking connections and deleting in per-room or server idle.
			g_pIocpManager->ClearSocketContext((*ii)->pCon->GetSocketContext());
			(*ii)->pCon->SetSocketContext(NULL, NULL);
			SAFE_DELETE((*ii));
			ii = m_TcpConnectionList.erase(ii);
		}
		else
			ii++;
	}

	EjectTcpConnection();
	OrphanPtr();

	//No need to call it too frequently.
	if (m_nLastCheckTick == 0)
		m_nLastCheckTick = nCurTick;

	if (m_nLastCheckTick != 0 && nCurTick - m_nLastCheckTick > (5*1000*60))	//Check roughly once every 5 minutes
	{
		CheckOrphan(nCurTick);
		m_nLastCheckTick = nCurTick;
	}
}
#endif

void CDNGameServerManager::ConnectedWorld(char cWorldID)
{
	std::vector <CDNRUDPGameServer*>::iterator ii;
	for (ii = m_GameServerList.begin(); ii != m_GameServerList.end(); ii++)
		(*ii)->SendRestoreMaster(cWorldID);
}

#ifndef _SKIP_BLOCK
void CDNGameServerManager::CheckOrphan(ULONG nCurTick)
{
	//Find and remove orphaned connections that completed TCP connect but never sent a connect msg (or failed to match)
	for( std::list<_TCPCON*>::iterator ii = m_TcpConnectionList.begin(); ii != m_TcpConnectionList.end();)
	{
		if (nCurTick - (*ii)->nCreateTick > (3*60*1000))		//About three minutes?...
		{
			g_pIocpManager->ClearSocketContext((*ii)->pCon->GetSocketContext());
			(*ii)->pCon->SetSocketContext(NULL, NULL);
			SAFE_DELETE((*ii));
			ii = m_TcpConnectionList.erase(ii);
		}
		else
			ii++;
	}
}

void CDNGameServerManager::OrphanPtr()
{
	std::vector <CDNTcpConnection*>::iterator ii;
	for (ii = m_OrphanTcpConnectionList.begin(); ii != m_OrphanTcpConnectionList.end(); ii++)
	{
		g_pIocpManager->ClearSocketContext((*ii)->GetSocketContext());
		(*ii)->SetSocketContext(NULL, NULL);
		SAFE_DELETE((*ii));
	}
	m_OrphanTcpConnectionList.clear();
}

void CDNGameServerManager::EjectTcpConnection()
{
	DNVector(std::pair<CDNTcpConnection*, CDNUserSession*>)::iterator ii;
	for (ii = m_EjectTcpConnectionList.begin(); ii != m_EjectTcpConnectionList.end(); ii++)
	{
		for ( std::list<_TCPCON*>::iterator hi = m_TcpConnectionList.begin(); hi != m_TcpConnectionList.end();)
		{
			if ((*ii).first == (*hi)->pCon)
			{
				_TCPCON * pTcpCon = (*hi);
				if (pTcpCon->pCon->SetSession((*ii).second) == false)
					_DANGER_POINT();
				hi = m_TcpConnectionList.erase(hi);
			}
			else
				hi++;
		}
	}

	m_EjectTcpConnectionList.clear();
}
#endif