#include "stdafx.h"
#include "ClientSession.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CClientSession::CClientSession(void)
: m_Socket(INVALID_SOCKET), m_SessionType(0)
{
	int nSize = 1024 * 32;
	m_pRecvBuffer = new CBuffer(nSize);
	m_pSendBuffer = new CBuffer(nSize);
}

CClientSession::CClientSession(int nSize)
: m_Socket(INVALID_SOCKET), m_SessionType(0)
{
	m_pRecvBuffer = new CBuffer(nSize);
	m_pSendBuffer = new CBuffer(nSize);
}

CClientSession::~CClientSession(void)
{
	Clear();

	SAFE_DELETE(m_pRecvBuffer);
	SAFE_DELETE(m_pSendBuffer);
}

void CClientSession::OnDisconnect( bool bValidDisconnect )
{
	/*
	if( !bValidDisconnect ) {
		if (m_pRecvBuffer) m_pRecvBuffer->Clear();
		if (m_pSendBuffer) m_pSendBuffer->Clear();
	}
	*/
}

void CClientSession::Clear()
{
}

BOOL CClientSession::GetLocalIP(WCHAR* pIP)
{
	if(!m_Socket) return FALSE;

	char Name[256] = {0,};

	gethostname(Name, sizeof(Name));

	PHOSTENT host = gethostbyname(Name);
	if (host)
	{
		if(MultiByteToWideChar(CP_ACP, 0, inet_ntoa(*(struct in_addr*)*host->h_addr_list), -1, pIP, 32) > 0)
			return TRUE;
	}

	return FALSE;
}

USHORT CClientSession::GetLocalPort(void)
{
	if (!m_Socket) return 0;

	SOCKADDR_IN Addr;
	ZeroMemory(&Addr, sizeof(Addr));

	int AddrLength = sizeof(Addr);
	if (getsockname(m_Socket, (sockaddr*) &Addr, &AddrLength) != SOCKET_ERROR)
		return ntohs(Addr.sin_port);

	return 0;
}


void CClientSession::ClearBuffer()
{
	m_pRecvBuffer->Clear();
	m_pSendBuffer->Clear(true);
}