#pragma once

#if defined(_SERVER)

#include "SocketContext.h"

using namespace std;
class EventAcceptor;
class CSendThread;
class CConnection;

typedef std::list<CSocketContext*> TSocketContextList;

class CIocpManager
{
#if !defined(_WIN32)
public:
	struct LinuxEpollContext;
	void PostCustomCompletion(DWORD dwBytesTransferred, CSocketContext* pSocketContext, TIOContext* pIOContext);
#ifdef _USE_ACCEPTEX
	void OnAcceptLinux(CSocketContext* pSocketContext, const sockaddr_in* pPeerAddr);
#endif
private:
	LinuxEpollContext* m_pLinuxEpoll;
#endif
private:
	LONG			m_WaitThreadCount;
	LONG			m_SendIOCount;
	CSendThread*	m_pSendThread;
protected:
	HANDLE m_hIOCP;
	bool m_bThreadSwitch;

	TSocketContextList m_SocketContexts;	// Pre-create socket
	CSyncLock m_SocketLock;

	EventAcceptor*	m_pAcceptor;

	static UINT __stdcall WorkerThread(void *pParam);

public:
	CIocpManager(void);
	virtual ~CIocpManager(void);
	
	//store dump!
	void StoreMiniDump();			//Mini? Minhee!

	//Test
	UINT m_nAddSendBufSize;
	UINT m_nPostSendSize;		//buffer pop
	UINT m_nRealSendSize;
	void GetSendReport(UINT * a, UINT * p, UINT * r)
	{
		*a = m_nAddSendBufSize;
		*p = m_nPostSendSize;
		*r = m_nRealSendSize;
	}

	UINT m_nAddRecvBufSize;
	UINT m_nPostRecvBufSize;
	UINT m_nProcessBufSize;
	void GetRecvReport(UINT * a, UINT * p, UINT * r)
	{
		*a = m_nAddRecvBufSize;
		*p = m_nPostRecvBufSize;
		*r = m_nProcessBufSize;
	}

	virtual int Init(int nSocketCountMax, int nWorkerThreadSize = 0);
	virtual void Final();

	virtual void OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort);
	virtual void OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort, UINT nSessionKey) {}
	virtual void OnConnected(CSocketContext *pSocketContext);
	virtual void OnDisconnected(CSocketContext *pSocketContext);
	virtual void OnReceive(CSocketContext *pSocketContext, DWORD dwBytesTransferred) = 0;
	// Override this to log connect failures.
	virtual void OnConnectFail(CSocketContext *pSocketContext);
	int AddConnectionEx(CConnection* pCon, const int nKey, const char *pIp, const int nPort);

	int AttachSocket(CSocketContext *pSocketContext);
	int DetachSocket(CSocketContext *pSocketContext, wchar_t *pwszIdent);

	bool DelSocket(CSocketContext *pSocketContext);

	int AddAcceptConnection(const int nKey, const int nPort, int nBackLog);
	int AddConnection(const int nKey, const char *pIp, const int nPort);
	int CreateConnection(const int nKey, const char * pIP, const int nPort, UINT nSessionKey);

	bool SetSocketOption (SOCKET Socket);
	int PostRecv(CSocketContext *pSocketContext);
#ifdef _USE_SENDCONTEXTPOOL
	int PostSend(CSocketContext *pSocketContext, TIOContext * pContext);
	void SendComplete(CSocketContext *pSocketContext, TIOContext * pContext, int nSize);
#else
	int PostSend(CSocketContext *pSocketContext);
	void SendComplete(CSocketContext *pSocketContext, int nSize);
#endif

	void PutSocketContext(CSocketContext *pSocketContext);
	CSocketContext *GetSocketContext (int nKey, SOCKET Socket, sockaddr_in *pAddr);
	void ClearSocketContext(CSocketContext *pSocketContext);
	int GetSocketContextCount();
	void GetPublicIP(std::string &strIP) { strIP = m_strPublicIP; }
	void GetPrivateIP(std::string &strIP) { strIP = m_strPrivateIP; }

#ifdef _USE_ACCEPTEX
	virtual void ThreadStop();
#else
	void ThreadStop();
#endif
	void CloseAcceptors();

	void AddSendCall(CSocketContext *pSocketContext);

	inline HANDLE GetIocpHandle() { return m_hIOCP; }
	inline void IncThreadCount() { InterlockedIncrement(&m_WaitThreadCount); }
	inline void DecThreadCount() { InterlockedDecrement(&m_WaitThreadCount); }
	inline void IncSendIOCount() { InterlockedIncrement(&m_SendIOCount); }
	inline void DecSendIOCount() { InterlockedDecrement(&m_SendIOCount); }

	//process status
	virtual void ProcessDelay(int nConKey, int nMainCmd, int nSubCmd, int nTick) {}
	virtual void StagnatePacket(int nCnt, int nTick, CSocketContext *pSocketContext) {}

#ifdef _USE_ACCEPTEX
	bool	bOnReUse( CSocketContext* pSocketContext );
	UINT	uiGetWorkerThreadCount() const { return m_uiWorkerThreadCount; }
private:
	void	_OnAccept( CSocketContext* pSocketContext );

	UINT		m_uiWorkerThreadCount;
	std::vector<HANDLE>	m_vWorkerThreadHandle;
#endif

	void	_OnConnect( CSocketContext* pSocketContext );

	void	_GetHostIPAddress();
	std::string	m_strPublicIP;
	std::string	m_strPrivateIP;
};


#endif	// #if defined(_SERVER)