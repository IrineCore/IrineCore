#pragma once

#if defined(_SERVER)

#define WORKERTHREADCOUNTMAX	16
const BYTE SENDIOPOOLSIZE = 2;

#define IOPOST_NONE			0
#define IOPOST_RECV			1
#define IOPOST_SEND			2

#ifdef _USE_ACCEPTEX
#define IOPOST_ACCEPT		3
#endif

#define IOPOST_CONNECT		4

#ifdef _USE_ACCEPTEX
class CSocketContext;
#elif defined _USE_SENDCONTEXTPOOL
class CSocketContext;
#endif

struct TIOContext : public WSAOVERLAPPED
{
	BYTE mode;
	char buffer[INTERNALBUFFERLENMAX];
	int Len;
#if defined( _USE_ACCEPTEX ) || defined( _USE_SENDCONTEXTPOOL )
	CSocketContext*	pSocketContext;
#endif // #if defined( _USE_ACCEPTEX ) || defined( _USE_SENDCONTEXTPOOL )
};


class CSocketContext
{
private:
	void *m_pParam;

#ifdef _USE_ACCEPTEX
	DWORD	m_dwCloseTime;
	UINT	m_uiListenID;
#endif

public:
	SOCKET m_Socket;	

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	enum
	{
		 DeleteFlag = 1<<30,
	};
	long m_lActiveCount;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	volatile long m_lDetached;
#endif

#ifdef _USE_SENDCONTEXTPOOL
	TIOContext m_RecvIO;

	//To perform asyncsend without waiting for sendcomplete...
	std::vector <TIOContext*> m_SendIOList;
	CSyncLock m_SendIOLock;

	TIOContext * GetSendIO();
	bool ReleaseSendIO(TIOContext * pIO, int nSize);
#else
	TIOContext m_RecvIO;
	TIOContext m_SendIO;
#endif

	DWORD m_dwKeyParam;		// eConnectionKey
	int m_nRefCount;

	CSyncLock m_RefLock;
	std::wstring m_wstrDestachReason;
	CSyncLock m_DetachReasonLock;

public:
	CSocketContext(void);
	~CSocketContext(void);

	void Clear();
	void SetParam(void *pParam);
	void* GetParam();

	void IncRef();
	int AddRef();
	int DelRef();

#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	bool Detach();
#endif

#ifdef _USE_ACCEPTEX
	
private:
	bool	_bInitSocket( const BYTE cIOMode );
public:
	void	OnAccept( sockaddr_in* pLocalAddr, sockaddr_in* pPeerAddr );
	void	OnDisconnect();
	bool	bInitAcceptContext();

	void	SetCloseTime( const DWORD dwTime ){ m_dwCloseTime = dwTime; }
	void	SetListenID( const UINT uiListenID ){ m_uiListenID = uiListenID; }
	
	DWORD	dwGetCloseTime()	const { return m_dwCloseTime; }
	UINT	uiGetListenID()		const { return m_uiListenID; }
#endif

	void SetDetachReason(const WCHAR * pReason);
	void GetDetachReason(std::wstring &wstrReason);
};

class CScopeInterlocked
{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
public:

	CScopeInterlocked( long* p ):m_p(p),m_bDelete(false),m_bIncreased(false)
	{
		if( InterlockedIncrement( m_p )&CSocketContext::DeleteFlag )		
			m_bDelete = true;
		m_bIncreased = true;
	}
	CScopeInterlocked( long* p, const int nKey ):m_p(p),m_bDelete(false),m_bIncreased(false)
	{
		if (nKey != CONNECTIONKEY_USER)
		{
			if( InterlockedIncrement( m_p )&CSocketContext::DeleteFlag )
				m_bDelete = true;
			m_bIncreased = true;
		}
	}
	~CScopeInterlocked()
	{
		if (m_bIncreased)
			InterlockedDecrement( m_p );
	}

	bool bIsDelete(){ return m_bDelete; }

private:
	long* m_p;
	bool  m_bDelete;
	bool m_bIncreased;
#else
public:
	CScopeInterlocked( long* p ):m_p(p)
	{
		InterlockedIncrement( m_p );
	}
	~CScopeInterlocked()
	{
		InterlockedDecrement( m_p );
	}

private:
	long* m_p;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
};


#endif	// #if defined(_SERVER)
