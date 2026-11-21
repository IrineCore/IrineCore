
#pragma once

#include "CriticalSection.h"
#include "DNUserSession.h"

class CDNGameRoom;
class CDNRUDPGameServer;
class CDNAgentConnection;
class CDNTcpConnection;

class CDNGameServerManager
{
public :
	CDNGameServerManager();
	~CDNGameServerManager();

	bool InitializeRUDP();
	void CalcOpenCount(int nProcessorCount, int nCreateCount, int nCreateIndex, int &nOpenCount, int &nStartAffinityCount, int &nBackLoaderIndex);
	void PreOpenGameServer();
	bool StartGameServer(USHORT nBeginAcceptPort, const char * pExternalProbeIP, unsigned short nExternalProbePort);

	void GetGameServerInfo(unsigned long * iIP, unsigned short * iPort, unsigned char * cIdx, bool * margin);

	//GetServerPtr
	CDNRUDPGameServer * GetGameServer();
	CDNRUDPGameServer * GetGameServerByUID(unsigned int iUID);
	CDNRUDPGameServer * GetGameServerIndex(unsigned int index);
	CDNRUDPGameServer * GetGameServerByAID(unsigned int iAccountDBID);
	CDNRUDPGameServer * GetGameServerByRoomID(unsigned int iRoomID);

	int GetGameServerSize() { return (int)m_GameServerList.size(); }

	//manage room & user
	unsigned int GenRoomID(CDNRUDPGameServer * pServer);
	bool VerifyUserIDs(UINT nAccountDBID, UINT nSessionID);
	void AddGameUser(UINT nRoomID, UINT nAccountDBID, UINT nSessionID, CDNRUDPGameServer * pServer);
	void RemoveGameRoom(unsigned int nRoomID, CDNRUDPGameServer * pServer, bool bCrashed = false);
	void RemoveGameUser(unsigned int nRoomID, unsigned int nAccountDBID, unsigned int nSessionID, CDNRUDPGameServer * pServer);

	void UpdateRoomCountInfo(unsigned int nRoomID, int nMapIdx);
	void GetRoomUserCount(UINT &nUserCount, UINT &nRoomCount, UINT &nTotalRoomCount);

	int GetRoomCount();
	CDNGameRoom *GetRootRoom() { return m_pRootRoom; }

	void CheckCloseGameServer(CDNRUDPGameServer * pServer);
	void MasterDisConnected(int nWorldSetID);

	//tcp connection
	CDNTcpConnection * CreateTcpConnection(const char * pIp, const int nPort);
#ifndef _SKIP_BLOCK
	void PushToEjectTcpConnection(CDNTcpConnection * pCon, CDNUserSession * pSession);	//with Async
	void PushOrphanPtr(CDNTcpConnection * pCon);

	//Connection Flush Msg
	void FlushConnectionBuffer(ULONG nCurTick);
#endif
	
	void ConnectedWorld(char cWorldID);
	int GetThreadCount() { return m_iThreadCount; }
	void SetZeroPopulation(bool bZero) { m_bZeroPopulation = bZero; };
	bool GetZeroPopulation() { return m_bZeroPopulation; }
	
private:
	CSyncLock	m_Lock;
	CSyncLock	m_UserLock;
	int m_iThreadCount;

	//GameServer Managing Conteiner
	std::vector <CDNRUDPGameServer*> m_GameServerList;

	//Session and Room Managing Conteiner
	struct sRoomstate
	{
		bool bCrashed;
		CDNRUDPGameServer * pServer;

		sRoomstate()
		{
			bCrashed = false;
			pServer = NULL;
		}
	};
	std::vector <CDNGameServerManager::sRoomstate> m_GameRoomList;		//first : bCrash , second : Room

	UINT	m_uiRoomCount;
	std::map <unsigned int, CDNRUDPGameServer*> m_UserConnectionInfoList;							//first = uid, second = server;
	std::map <unsigned int, CDNRUDPGameServer*> m_UserConnectionInfoListByAccountDBID;					//first = AccountDBID, second = server pointer;	

	//Counting
	std::vector<std::pair<int,int>> m_RoomCountInfo;					//first = roomid, second = <mapidx, usercount>

#ifndef _SKIP_BLOCK
	//tcp Connection Manage
	struct _TCPCON:public TBoostMemoryPool<_TCPCON>
	{
		ULONG nCreateTick;
		CDNTcpConnection * pCon;
	};

	CSyncLock m_ConSync;		//Newly created to avoid using frequently accessed m_MultiSync
	std::list <_TCPCON*> m_TcpConnectionList;					// 2009.02.09 kimbap changed vector->list
	DNVector(std::pair<CDNTcpConnection*, CDNUserSession*>) m_EjectTcpConnectionList;
	std::vector <CDNTcpConnection*> m_OrphanTcpConnectionList;

	//check Tcp Orphan
	ULONG m_nLastCheckTick;
	void CheckOrphan(ULONG nCurTick);
	void OrphanPtr();

	void EjectTcpConnection();
#endif

	//Root Room
	CDNGameRoom *m_pRootRoom;

	//IsOpen
	bool m_bZeroPopulation;
};

extern CDNGameServerManager * g_pGameServerManager;