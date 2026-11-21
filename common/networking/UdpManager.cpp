#include "stdafx.h"
#include "UdpManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CUdpManager::CUdpManager(void): m_Socket(INVALID_SOCKET)
{
}

CUdpManager::CUdpManager(int nSize): m_Socket(INVALID_SOCKET)
{
}

CUdpManager::~CUdpManager(void)
{
}

void CUdpManager::Clear()
{
	if (m_Socket != INVALID_SOCKET){
		closesocket(m_Socket);
	}

	m_Socket = INVALID_SOCKET;
}

void CUdpManager::OnRecv()
{
}

void CUdpManager::OnSend()
{
}

UINT __stdcall CUdpManager::SendThread(void *pParam)
{
	return 1;
}

UINT __stdcall CUdpManager::RecvThread(void *pParam)
{
	CUdpManager *pUdp = (CUdpManager*)pParam;

	while (1){
		pUdp->OnRecv();
	}
	return 1;
}

bool CUdpManager::Listen(const USHORT nPort)
{
	if (nPort <= 0) return false;

	Clear();

	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_Socket == INVALID_SOCKET) return false;
	
	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(nPort);

	if (::bind(m_Socket, (sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR){
		int nError = WSAGetLastError();
		closesocket(m_Socket);
		return false;
	}

	DWORD dwMode = 1;
	::ioctlsocket( m_Socket, FIONBIO, &dwMode );

	HANDLE hThread;
	UINT nThreadID = 0;
	hThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, &nThreadID);
	if (hThread == INVALID_HANDLE_VALUE) return false;
	CloseHandle(hThread);

	return true;
}

void CUdpManager::SendData(char *pData, int nSize, SOCKADDR_IN *pAddr)
{
	sendto(m_Socket, pData, nSize, 0, (sockaddr*)pAddr, sizeof(SOCKADDR_IN));
}
