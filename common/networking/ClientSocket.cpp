#include "stdafx.h"
#include "ClientSocket.h"
#include "ClientSessionManager.h"
#include "DNPacket.h"
#include "DNSecure.h"
#ifdef PRE_ADD_CL_DISCONNECT_LOG
#include "BugReporter.h"
#endif // #ifdef PRE_ADD_CL_DISCONNECT_LOG

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _PACKET_COMP
#include "zlib.h"
#endif

//UINT __stdcall SelectThreadCallBack(void *pParam)
//{
//	CClientSocket *pClientSocket = (CClientSocket*)pParam;
//	pClientSocket->Update();
//
//	return 0;
//}

CClientSocket::CClientSocket(void)
: m_bConnected(false), m_bWrite(false), m_bThreadClose(false), m_hEvent(NULL), m_hThread(INVALID_HANDLE_VALUE), m_bRunning(false)
{
	m_hTerminateEvent = CreateEvent( NULL, false, false, NULL );
	Init();
}

CClientSocket::CClientSocket(int nSize)
: m_bConnected(false), m_bWrite(false), m_bThreadClose(false), m_hEvent(NULL), m_hThread(INVALID_HANDLE_VALUE)
{
	m_hTerminateEvent = CreateEvent( NULL, false, false, NULL );
	Init();
}

CClientSocket::~CClientSocket(void)
{
	Clear();	
	CloseHandle(m_hTerminateEvent);
	delete[] m_pTempBuffer;
	m_pTempBuffer = NULL;
	m_iTempBufferSize = 0;
}

void CClientSocket::Init()
{
	Clear();

	m_Socket = INVALID_SOCKET;
	m_hEvent = WSACreateEvent();
	m_pReceiver = NULL;
	m_pTempBuffer = new char[65535];
	m_iTempBufferSize = 65535;

	CreateRecvThread();
}

void CClientSocket::Clear()
{
	Close( true, false );
	
	DestroyRecvThread();

	if (m_hEvent){
		WSACloseEvent(m_hEvent);
		m_hEvent = NULL;
	}

	m_hEvent = NULL;
	m_Socket = INVALID_SOCKET;
	m_bConnected = m_bWrite = m_bThreadClose = false;
}

bool CClientSocket::CreateRecvThread()
{
	if(m_hThread != INVALID_HANDLE_VALUE) 
		return false;
	m_bRunning = true;
	UINT nThreadID = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, &nThreadID);
	if( m_hThread == INVALID_HANDLE_VALUE )
	{
		m_bRunning = false;
		m_hThread = INVALID_HANDLE_VALUE;
		return false;
	}
	GetExitCodeThread( m_hThread, &m_dwThreadExitCode );
	return true;
}

void CClientSocket::DestroyRecvThread()
{
	m_bRunning = false;
	if( m_hThread != INVALID_HANDLE_VALUE )
	{
		WriteLog( 0, ", Info, Try to Destroy Recv Thread..\n" );
		SetEvent(m_hTerminateEvent);
		WaitForSingleObject(m_hThread, 3000);
		
		CloseHandle(m_hThread);
		ResetEvent(m_hTerminateEvent);
		m_hThread = INVALID_HANDLE_VALUE;
		WriteLog( 0, ", Info, Recv Thread Destroy..\n" );
	}
}

bool CClientSocket::Connect(const char *pIp, const USHORT nPort)
{
	if (nPort <= 0) return false;

	m_Socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Socket == INVALID_SOCKET) return false;

	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(pIp);
	sa.sin_port = htons(nPort);

	if (connect(m_Socket, (PSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR){
		closesocket(m_Socket);
		return false;
	}

	if (WSAEventSelect(m_Socket, m_hEvent, FD_CLOSE | FD_READ) == SOCKET_ERROR){
		closesocket(m_Socket);
		return false;
	}

	CreateRecvThread();
	m_szServerIP = pIp;
	m_nServerPort = nPort;
	m_bConnected = true;
	m_bThreadClose = false;
	OnConnect();

	return true;
}

void CClientSocket::Close(bool boGraceful, bool bValidDisconnect)
{
	if (m_Socket != INVALID_SOCKET) {
		DestroyRecvThread();
		OnDisconnect( bValidDisconnect );
		closesocket(m_Socket);
	}

	m_Socket = INVALID_SOCKET;	
	m_bConnected = false;
	m_bThreadClose = false;

	ClearBuffer();
}

int CClientSocket::AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen, int nInsertAfter)
{
	//You can send just a command without data, but the command must not be 0,0, right?
	_ASSERT(iMainCmd < 255);
	_ASSERT(iSubCmd < 255);
	//_ASSERT(iLen < 2049);

	//If you need to obfuscate security headers or process the buffer, handle it at this stage.
	
	//DNEncryptPacket EnPacket;
	DNEncryptPacketSeq EnPacket;
	memset(&EnPacket, 0, sizeof(EnPacket));
	
	EnPacket.Packet.iLen = static_cast<unsigned short>(sizeof(DNTPacketHeader)+iLen);
	EnPacket.Packet.cMainCmd = static_cast<unsigned char>(iMainCmd);
	EnPacket.Packet.cSubCmd = static_cast<unsigned char>(iSubCmd);
	memcpy(&EnPacket.Packet.buf, pData, iLen);

	EnPacket.nLen = EnPacket.Packet.iLen + sizeof(BYTE) + sizeof(USHORT);
	CDNSecure::GetInstance().Tea_encrypt( reinterpret_cast<char*>(&EnPacket.Packet), EnPacket.Packet.iLen );
	
	//Push into the sender buffer.
	switch( nInsertAfter ) {
		case -1:
			if (m_pSendBuffer->Push((char*)&EnPacket, EnPacket.nLen) < 0) return -1;
			break;
		case 0:
			if (m_pSendBuffer->Insert((char*)&EnPacket, EnPacket.nLen) < 0) return -1;
			break;
	}
	
	return 0;
}

int CClientSocket::AddRecvData(int iMainCmd, int iSubCmd, char *pData, int iLen, int nInsertAfter)
{
	//You can send just a command without data, but the command must not be 0,0, right?
	_ASSERT(iMainCmd < 255);
	_ASSERT(iSubCmd < 255);
	_ASSERT(iLen < 2049);

	//If you need to obfuscate security headers or process the buffer, handle it at this stage.

	/*DNEncryptPacket EnPacket;
	SecureLib_InitSA(&EnPacket.SA);
	EnPacket.Packet.iLen = static_cast<unsigned short>(sizeof(DNTPacketHeader)+iLen);
	EnPacket.Packet.cMainCmd = static_cast<unsigned char>(iMainCmd);
	EnPacket.Packet.cSubCmd = static_cast<unsigned char>(iSubCmd);
	memcpy(&EnPacket.Packet.buf, pData, iLen);

	EnPacket.nLen = (USHORT)(SecureEncrypt((BYTE*)&EnPacket.Packet, EnPacket.Packet.iLen) + sizeof(ClientSA) + sizeof(USHORT));*/

	/*
	//Create packet to send.
	DNTPacket dnPacket;
	memset(&dnPacket, 0, sizeof(DNTPacket));

	dnPacket.cMainCmd = (unsigned char)iMainCmd;
	dnPacket.cSubCmd = (unsigned char)iSubCmd;
	dnPacket.iLen = iLen + (int)(sizeof(dnPacket) - sizeof(dnPacket.buf));

	//Write data into the buffer.
	if ((iLen > 0) && pData)
		memcpy(dnPacket.buf, pData, iLen);
	*/

	//Push into the sender buffer.
	//switch( nInsertAfter ) {
	//	case -1:
	//		if (m_pRecvBuffer->Push((char*)&EnPacket, EnPacket.nLen) < 0) return -1;
	//		break;
	//	case 0:
	//		if (m_pRecvBuffer->Insert((char*)&EnPacket, EnPacket.nLen) < 0) return -1;
	//		break;
	//}

	return 0;
}

void CClientSocket::DoSend()
{
	int nBytes, nSend, nErr;
	char buffer[CLIENTPACKETMAX] = { 0, };

	if (m_bWrite == false) return;

	while (m_pSendBuffer->GetCount() > 0) {
		nSend = m_pSendBuffer->GetCount ();
		if (nSend > CLIENTPACKETMAX) nSend = CLIENTPACKETMAX;
		if (m_pSendBuffer->View(buffer, nSend) < 0) break;

		nBytes = send(m_Socket, buffer, nSend, 0);
		if (nBytes == SOCKET_ERROR) {
			nErr = WSAGetLastError();
			if (nErr == WSAEWOULDBLOCK) {
				//m_bWrite = false; --- Hit a would-block here and if m_bWrite becomes false, next packet cannot be sent, so commented out. by robust
				break;
			}

#ifdef PRE_ADD_CL_DISCONNECT_LOG
			gs_BugReporter.AddLogA( "CClientSocket::DoSend() - send() failed [WSAGetLastError() : %d]", nErr );
#endif // #ifdef PRE_ADD_CL_DISCONNECT_LOG

			m_bThreadClose = true;
			break;
		}
		m_pSendBuffer->Skip(nBytes);
	}
}

void CClientSocket::OnConnect()
{
	m_bConnected = true;
	m_bWrite = true;

	if( CClientSessionManager::IsActive() )
		CClientSessionManager::GetInstance().OnConnectTcp();
}

void CClientSocket::OnDisconnect( bool bValidDisconnect )
{
	CClientSession::OnDisconnect( bValidDisconnect );

	m_bConnected = false;
	m_bWrite = false;

	if( CClientSessionManager::IsActive() )
		CClientSessionManager::GetInstance().OnDisconnectTcp( bValidDisconnect );
}

void CClientSocket::OnRecv()
{
	//Recv Thread Call
	if (m_bThreadClose) return;

	int nRecv, nErr;
	char buffer[65535];
	memset(buffer, 0, sizeof(buffer));

	nRecv = recv(m_Socket, buffer, CLIENTPACKETMAX, 0);
	if (nRecv == SOCKET_ERROR || nRecv == 0){
		nErr = WSAGetLastError();
		if (nErr == WSAEWOULDBLOCK) return;

#ifdef PRE_ADD_CL_DISCONNECT_LOG
		gs_BugReporter.AddLogA( "CClientSocket::OnRecv() - recv() failed [WSAGetLastError() : %d]", nErr );
#endif // #ifdef PRE_ADD_CL_DISCONNECT_LOG

		m_bThreadClose = true;
		return;
	}

	m_pRecvBuffer->Push(buffer, nRecv);

#if defined( PRE_TEST_PACKETMODULE )
	return;
#endif // #if defined( PRE_TEST_PACKETMODULE )

	while (m_pRecvBuffer->GetCount () > sizeof(USHORT)) {
		if (m_pRecvBuffer->View((char *)&nRecv, sizeof(USHORT)) < 0) break;

		if (m_pRecvBuffer->GetCount () < nRecv) break;

		if( nRecv > m_iTempBufferSize )
		{
			delete[] m_pTempBuffer;
			m_pTempBuffer = new char[nRecv];
			m_iTempBufferSize = nRecv;
		}
		memset(m_pTempBuffer, 0, m_iTempBufferSize);
		if (m_pRecvBuffer->Pop(m_pTempBuffer, nRecv) < 0) break;

		DNEncryptPacketSeq *pEnPacket = (DNEncryptPacketSeq*)m_pTempBuffer;
#ifdef _PACKET_COMP

		int nLen = pEnPacket->nLen - sizeof(BYTE) - sizeof(USHORT);
		CDNSecure::GetInstance().Tea_decrypt( reinterpret_cast<char*>(&pEnPacket->Packet), nLen );
		if( m_pReceiver )
		{
			if (pEnPacket->cSeq == COMPRESSPACKET)
			{
				static DNTCompPacket sComPacket;
				UINT nSize = 0;
				for (int i = 0; i < pEnPacket->nLen; )
				{
					memset(&sComPacket, 0, sizeof(sComPacket));
					memcpy(&sComPacket.header, &pEnPacket->Buf[i], sizeof(DNTPacketCompHeader));

					UINT nUncompSize = sizeof(sComPacket.buf);
					uncompress((Bytef*)&sComPacket.buf, (uLongf*)&nUncompSize, (Bytef*)&pEnPacket->Buf[i+sizeof(DNTPacketCompHeader)], (uLongf)sComPacket.header.nLen-sizeof(DNTPacketCompHeader));
					
					nSize = sComPacket.header.nLen;
					sComPacket.header.nLen = nUncompSize;

					m_pReceiver->RecvData(&sComPacket);
					i += nSize;
				}
			}
			else
				m_pReceiver->RecvData(pEnPacket);
		}
		else
		{
			_ASSERT(0);
			return ;
		}
#else

		int nLen = pEnPacket->nLen - sizeof(BYTE) - sizeof(USHORT);
		CDNSecure::GetInstance().Tea_decrypt( reinterpret_cast<char*>(&pEnPacket->Packet), nLen );
		if( m_pReceiver )
			m_pReceiver->RecvData(pEnPacket);
#endif
	}
}

void CClientSocket::OnSend()
{
	m_bWrite = true;
}

void CClientSocket::OnError(int nError)
{
 
}

int CClientSocket::DoUpdate()
{
	//Process Thread Call
	if( !m_bConnected ) return 0;
	if (m_bThreadClose){
		return -1;
	}
	
	DoSend();
	return 0;	
}

int CClientSocket::ThreadUpdate()
{
	if (m_Socket == INVALID_SOCKET || m_hEvent == NULL){
		return -1;
	}
	
	if (WSAWaitForMultipleEvents(1, &m_hEvent, false, 500, false) == WSA_WAIT_EVENT_0){
		WSANETWORKEVENTS netEvent;

		if (WSAEnumNetworkEvents (m_Socket, m_hEvent, &netEvent) == 0) {
			if (netEvent.lNetworkEvents & FD_READ) {
				if (netEvent.iErrorCode [FD_READ_BIT] == 0) {
					OnRecv();
				}
			}
			if (netEvent.lNetworkEvents & FD_CLOSE) {
				m_bThreadClose = true;
#ifdef PRE_ADD_CL_DISCONNECT_LOG
				gs_BugReporter.AddLogA( "CClientSocket::ThreadUpdate() - WSAEnumNetworkEvents() -> FD_CLOSE [Server transfer or server shutdown]" );
#endif // #ifdef PRE_ADD_CL_DISCONNECT_LOG
				return ON_CLOSE;
			}
		}
		else {
			int nRet = WSAGetLastError();
#ifdef PRE_ADD_CL_DISCONNECT_LOG
			gs_BugReporter.AddLogA( "CClientSocket::ThreadUpdate() - WSAWaitForMultipleEvents() failed [WSAGetLastError() : %d]", nRet );
#endif // #ifdef PRE_ADD_CL_DISCONNECT_LOG
			m_bThreadClose = true;
			return ON_CLOSE;
		}
	}

	return 0;
}

UINT __stdcall CClientSocket::RecvThread(void *pParam)
{
	CClientSocket *pClientSocket = (CClientSocket*)pParam;
	while (pClientSocket->m_bRunning)
	{
		if(WaitForSingleObject(pClientSocket-> GetTerminateEvent(), 1) == WAIT_OBJECT_0 )
			break;
		if (pClientSocket->ThreadUpdate() == ON_CLOSE) break;
		Sleep(1);
	}

	return 0;
}
