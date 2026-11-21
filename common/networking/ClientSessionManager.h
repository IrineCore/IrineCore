#pragma once

#include "Singleton.h"
#include "RUDPCommonDef.h"
#include "DNCommonDef.h"
#include "DNPacket.h"
#include "CriticalSection.h"
#include "Buffer.h"

class CNetSocketRUDP;
class CNetSocketUDP;
class CClientSocket;
class CTaskListener;

//probe info
static char s_szProbeIP[2][IPLENMAX];		// probe server TCP, UDP server IP
static int s_nProbePort[2];
static bool s_bNetOpen;
static int s_nPeerOpenPort;

struct _SEQ_DATA
{
	BYTE cSeq;
	USHORT nLen;
	BYTE cMainCmd;
	BYTE cSubCmd;
	char * pBuf;
};

enum _SEQDATA_TYPE
{
	_SEQDATA_TYPE_DIRECT,
	_SEQDATA_TYPE_SEQ,
	_SEQDATA_TYPE_RECURSIVE,
};

enum SkipProcess
{
	SkipFlag = 1<<30,
};

class CSeqReceiver
{
public:
	CSeqReceiver();
	virtual ~CSeqReceiver();
	
	void RecvData(DNEncryptPacketSeq * pPacket);
	void RecvData(DNTCompPacket * pPacket);
	void RecvData(DNGAME_PACKET * pPacket);
	void SkipProcess();

protected:
	std::list <_SEQ_DATA> m_pProcessList;
	CSyncLock m_ProcessSync;
	volatile long m_nSkipProcess;
	
	void AddProcessData(BYTE cMainCmd, BYTE cSubCmd, char * pData, int nSize, int nSeqDataType);
	void ClearProcessData();
	void ResetSeq();

private:
	volatile BYTE m_cDataSeq;
	std::list <_SEQ_DATA> m_pRecvList;
	CSyncLock m_RecvSync;

	void RecvSeqData(BYTE cSeq, DNTPacket * pData);
	void RecvSeqData(BYTE cSeq, BYTE cMainCmd, BYTE cSubCmd, char * pData, int nSize);
	void CheckSeqData();
};

class MessageListener;
class CClientSessionManager : public CSingleton<CClientSessionManager>, public CSeqReceiver
{
public:
#if defined( PRE_TEST_PACKETMODULE )
	CClientSessionManager( CClientSocket* pTcpSocket=NULL );
#else
	CClientSessionManager();
#endif // #if defined( PRE_TEST_PACKETMODULE )
	~CClientSessionManager();	

protected:
	std::vector <CTaskListener*> m_pTaskListener;
	std::vector<MessageListener*>	 m_MessageListenerArray;

	bool m_bConnectLoginServer;
	bool m_bConnectVillageServer;
	bool m_bConnectGameServer;
	volatile bool m_bDisConnectServer;
	bool m_bValidDisConnect;

	//Connect Info
	eBridgeState m_ConnectState;

	char m_szVillageIP[IPLENMAX];
	USHORT m_nVillagePort;

	unsigned long m_nGameServerIP;
	unsigned short m_nGameServerUdpPort;
	unsigned short m_nGameServerTcpPort;
	UINT m_nSessionID;
	char m_cReqGameIDType;
	int m_nGameTaskType;

	std::vector<std::string> m_szLoginVecIPList;
	std::vector<USHORT> m_nVecLoginPortList;
	int m_nLastLoginConnectOffset;

private:
	CClientSocket * m_pTcpSocket;
	CNetSocketRUDP * m_pRUDPSocket;
	CNetSocketUDP * m_pUDPSocket;

	void ConnectServer();

	bool ReconnectGameServer(UINT nSessionID);
	void DisconnectLoginServer( bool bValidDisconnect );
	void DisconnectVillageServer( bool bValidDisconnect );
	void DisconnectGameServer( bool bValidDisconnect );

	bool ConnectLoginServer(const char *pIP, USHORT nPort, bool bFirstConnect);

public:
	//	Common Send Interface
	void SendTcpPacket(int iMainCmd, int iSubCmd, char * pData, int iLen);
	void SendPacket(int iMainCmd, int iSubCmd, char * pData, int iLen, int iPrior = _RELIABLE, BYTE cSeqLevel = 0);
	void RecvPacket(int iMainCmd, int iSubCmd, char * pData, int iLen, int nInsertAfter = -1);

	void DisConnectServer(bool bValidDisconnect);

	// TCP 
	bool ConnectLoginServer(std::vector<std::string> &szVecIP, std::vector<USHORT> &nVecPort, bool bFirstConnect);
	bool ConnectVillageServer(const char *pIP, USHORT nPort, int nBridgeState);
	bool ConnectGameServer(unsigned long iIP, unsigned short iPort, unsigned short nTcpPort, int iUID, int nBridgeState, char cReqGameIDType, int nGameTaskType);

	void OnConnectTcp();
	void OnDisconnectTcp( bool bValidDisconnect );

	//void SendGameTcpConnect(UINT nSessionID, UINT nAccountDBID, INT64 biCertifyingKey);

	//UDP
	bool RUDPConnect(unsigned long iIP, unsigned short iPort, int iUID);
	//bool RUDPConnect(const char * pIP, unsigned short iPort, int iUID);
	void RUDPDisConnect();

	bool InitializeUdpSession( USHORT nPort );
	bool FinalizeUdpSession();

	void OnDisconnectUdp( bool bValidDisconnect );

	void Process();

	void AddTaskListener(CTaskListener * pListener);
	void RemoveTaskListener(CTaskListener * pListener);

	/*void AddTcpSession( CClientTcpSession *pSession );
	void AddUdpSession( CClientUdpSession *pSession );
	void RemoveTcpSession( CClientTcpSession *pSession );
	void RemoveUdpSession( CClientUdpSession *pSession );*/

	BOOL GetLocalIP(WCHAR* pIP);
	USHORT GetLocalPort(void);

	const char *GetUdpIP();
	USHORT GetUdpPort();

#ifdef _USE_NATPROBE
	void SetProbeInfo(const char * pTcpIP, const char * pUdpIP, int nTcpPort, int nUdpPort, int nOpenPort);
#endif

	int AddListener(MessageListener* pListener) { m_MessageListenerArray.push_back(pListener); return 1; }
	void RemoveListener(MessageListener* pListener);
	void RemoveAllListener() { m_MessageListenerArray.clear(); }

	void LockProcessDispatchMessage( bool bLock );

private:
	//udp accept test process
	static DWORD WINAPI _Accept_Test(void * param);

	static SOCKET CreateUDPSocket(int port);
	static SOCKET CreateTCPSocket(int port);

	//Peer Process
	struct _PING
	{
		unsigned long nSendTick;
		unsigned long nPing;
	};

	struct _PEER
	{
		_ADDR addr;
		unsigned int iUID;

		unsigned char nSeq;
		_PING Ping[32];
	};
	std::vector<_PEER> m_UDPPeerList;

	bool IsPeerConnected(unsigned int iUID);
	void AddPeer(unsigned long iAddr, unsigned short iPort, unsigned int iUID);
	void RemovePeer(unsigned int iUID);
	void CheckPeerPing();

	void RUDPSendPacket(int iMainCmd, int iSubCmd, void * pData, int iSize, int iPrior, BYTE cSeqLevel);
	void UDPSendPacket(int iMainCmd, int iSubCmd, void * pData, int iSzie, unsigned long iIP, unsigned short iPort);
};
