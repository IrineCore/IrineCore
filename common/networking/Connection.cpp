#include "stdafx.h"
#include "Connection.h"
#include "Log.h"
#include "DNPacket.h"
#include "DNProtocol.h"
#include "DNServerDef.h"
#include "DNServerProtocol.h"
#include "DNSecure.h"
#include "SpinBuffer.h"

#ifdef _PACKET_COMP
#include "zlib.h"
#endif

#if defined(_SERVER)

CConnection::CConnection(void)
: m_nSessionID(0), m_pIocpManager(NULL), m_pRecvBuffer(NULL), m_pSendBuffer(NULL), 
  m_bActive(false), m_bDelete(false), m_pSocketContext(NULL), m_bServerConnection(false)
#ifdef _PACKET_COMP
  ,m_pCompBuffer(NULL)
  ,m_bUseComp(false)
#endif
  ,m_bConnecting(false)
  ,m_bDetachFlag(false)
  ,m_hIocp(INVALID_HANDLE_VALUE)
{
	memset(m_DebugInfo, 0, sizeof(_DEBUG_INFO[32]));
	m_szIP[0] = '\0';
	m_wszIP[0] = '\0';
	m_wPort = 0;
	m_DebugInfoCount = 0;
	m_nStagnatedCount = 0;
	m_bPushSendQueue	= false;
	SecureLib_InitSA(&m_ClientSA);
}

CConnection::~CConnection(void)
{
	Final();
}

int CConnection::Init(int nRecvLen, int nSendLen, int nCompLen)
{
	Final();

	if (nRecvLen < 8192) nRecvLen = 8192;
	if (nSendLen < 8192) nSendLen = 8192;
	if (nCompLen < 6144) nCompLen = 6144;

#if defined( _DBSERVER ) || defined( _LOGSERVER ) || defined( _CASHSERVER )
	UINT uiAccessThreadCount = THREADMAX;
#else
	UINT uiAccessThreadCount = 1;
#endif
	m_pRecvBuffer = new (std::nothrow)CTcpRecvSpinBuffer( uiAccessThreadCount, nRecvLen );
	if (!m_pRecvBuffer)
		return -1;

	m_SendSync.Lock();
	m_pSendBuffer = new CBuffer(nSendLen);
	if (!m_pSendBuffer)
	{
		m_SendSync.UnLock();
		return -1;
	}
	m_SendSync.UnLock();

#ifdef _PACKET_COMP
	//Currently only supports user connections.
	if (nCompLen > 0)
	{
		m_pCompBuffer = new (std::nothrow)CCompPacketBuffer( 1, nCompLen );
		if (!m_pCompBuffer)
			return -1;
	}
#endif

	return 0;
}

void CConnection::Final()
{
	SAFE_DELETE(m_pRecvBuffer);
#ifdef _PACKET_COMP
	SAFE_DELETE(m_pCompBuffer);
#endif

	m_SendSync.Lock();
	SAFE_DELETE(m_pSendBuffer);
	m_SendSync.UnLock();

	if (m_pSocketContext && m_pIocpManager)
		m_pIocpManager->ClearSocketContext(m_pSocketContext);

	m_bActive = m_bServerConnection = false;
}

int CConnection::AddRecvData( CSocketContext* pSocketContext )
{
#if defined(_LOGINSERVER)
	if (m_bDetachFlag == true)
		return COMPLETE;
#else 
	if (m_bDetachFlag == true)
		return COMPLETE;
#endif // #if defined(_LOGINSERVER)

	if( m_pRecvBuffer->Push( pSocketContext ) < 0 )
		return -1;

	return COMPLETE;
}

int CConnection::AddRecvData( const DNTPacket& Packet )
{
	if( m_pRecvBuffer->Push( Packet ) < 0 )
		return -1;

	return COMPLETE;
}

int CConnection::AddRecvData(  const DNEncryptPacketSeq& Packet  )
{
	if( m_pRecvBuffer->Push( Packet ) < 0 )
		return -1;

	return COMPLETE;
}

int CConnection::AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen, TParamData* pParamData, BYTE cSeq, bool bForceUncomp)
{
	if (GetDetachFlag())
		return -1;

	_ASSERT(iMainCmd < 255);
	_ASSERT(iSubCmd < 255);
	_ASSERT(iLen>=0);

	int Ret = 0;
	int nBufLen = 0;

	if (GetDelete()) return -1;
	if( !GetActive() )
		return -1;

	if (m_pIocpManager && m_pSocketContext && (m_pSocketContext->m_Socket != INVALID_SOCKET))
	{
		switch (m_pSocketContext->m_dwKeyParam)
		{
		case CONNECTIONKEY_USER:	// User is encrypted
			{
#ifdef _PACKET_COMP
				if (m_bUseComp && bForceUncomp == false)
				{
					if (m_pCompBuffer && iLen > COMPRESSMINSIZE)
					{
						DNTCompPacket packet;
						memset(&packet, 0, sizeof(packet));

						packet.header.cSeq = cSeq;
						packet.header.cMainCmd = static_cast<unsigned char>(iMainCmd);
						packet.header.cSubCmd = static_cast<unsigned char>(iSubCmd);

						UINT nLeftBuffetrSize = ((CCompPacketBuffer*)m_pCompBuffer)->GetLeftBufferSize();

						UINT nCompPacketSize = sizeof(packet.buf);
						compress2((Bytef*)&packet.buf, (uLongf*)&nCompPacketSize, (Bytef*)pData, (uLongf)iLen, Z_BEST_SPEED);

						packet.header.nLen = static_cast<short>(sizeof(DNTPacketCompHeader) + nCompPacketSize);

						if (packet.header.nLen > nLeftBuffetrSize)
							FlushCompData();
						Ret = m_pCompBuffer->Push(packet);
					}
					else
						return AddSendData(iMainCmd, iSubCmd, pData, iLen, pParamData, cSeq, true);
				}
				else
#endif
				{
#ifdef _PACKET_COMP
					//To guarantee ordering, flush any queued buffers first.
					if (m_pCompBuffer) FlushCompData();
#endif

					if( pParamData && pParamData->bEncrypt )
					{
						m_SendSync.Lock();
						Ret = m_pSendBuffer->Push( (char*)&pParamData->sEncrypt, pParamData->sEncrypt.nLen, true );
						m_SendSync.UnLock();

						nBufLen =  pParamData->sEncrypt.nLen;
					}
					else
					{
						DNEncryptPacketSeq EnPacket = { 0, };
						EnPacket.cSeq = cSeq;
						EnPacket.Packet.iLen = static_cast<unsigned short>(sizeof(DNTPacketHeader)+iLen);
						EnPacket.Packet.cMainCmd = static_cast<unsigned char>(iMainCmd);
						EnPacket.Packet.cSubCmd = static_cast<unsigned char>(iSubCmd);

						if( iLen < 0 || iLen > sizeof(EnPacket.Packet.buf) )
						{
							_DANGER_POINT_MSG( L"if( iLen < 0 || iLen > sizeof(EnPacket.Packet.buf) )" );
							//_DANGER_POINT();
							return -1;
						}

						memcpy(&EnPacket.Packet.buf, pData, iLen);

						EnPacket.nLen = EnPacket.Packet.iLen + sizeof(BYTE) + sizeof(USHORT);
						CDNSecure::GetInstance().Tea_encrypt( reinterpret_cast<char*>(&EnPacket.Packet), EnPacket.Packet.iLen );

						if (EnPacket.nLen > 0)
						{
							DWORD dwTime = timeGetTime();
							do
							{
								bool bIncreaseBuffer = false;
								// Exception handling on cash inventory list for Chinese VVIP users
								if( iMainCmd == SC_ITEM && iSubCmd == eItem::SC_CASHINVENLIST )
									bIncreaseBuffer = true;

								m_SendSync.Lock();
								Ret = m_pSendBuffer->Push( (char*)&EnPacket, EnPacket.nLen, true, bIncreaseBuffer );
								m_SendSync.UnLock();

								// Insufficient buffer
								if( Ret == -2 )
								{
									FlushSendData();
								}
							}while( Ret < 0 && timeGetTime()-dwTime<1000 );
						}
						else Ret = -1;	//encrypt failed

						if (Ret < 0) _ASSERT(0);

						nBufLen = EnPacket.nLen;
						if( pParamData && pParamData->bEncrypt == false )
						{
							memcpy( &pParamData->sEncrypt, &EnPacket, EnPacket.nLen );
							pParamData->bEncrypt = true;
						}
					}
				}
			}
			break;

#ifdef PRE_ADD_DOORS
		case CONNECTIONKEY_DOORS:
			{
				DNDoorsPacketHeader dnDoors;
				memset(&dnDoors, 0, sizeof(DNDoorsPacketHeader));

				dnDoors.datasize = static_cast<unsigned short>(sizeof(DNDoorsPacketHeader) + iLen);
				dnDoors.protocol = static_cast<unsigned short>(iMainCmd);

				m_SendSync.Lock();
				Ret = m_pSendBuffer->Push( (char*)&dnDoors, sizeof(DNDoorsPacketHeader) );	// Header
				if( Ret == 0 && iLen > 0 )	
					Ret = m_pSendBuffer->Push( pData, iLen );						// Data
				m_SendSync.UnLock();

				nBufLen = dnDoors.datasize;
			}
			break;
#endif	// #ifdef PRE_ADD_DOORS

		default:	// Do not encrypt anything else
			{
				DNTPacketHeader dnHeader;
				dnHeader.iLen		= static_cast<unsigned short>(sizeof(dnHeader)+iLen);
				dnHeader.cMainCmd	= static_cast<unsigned char>(iMainCmd);
				dnHeader.cSubCmd	= static_cast<unsigned char>(iSubCmd);

				m_SendSync.Lock();
				Ret = m_pSendBuffer->Push( (char*)&dnHeader, sizeof(dnHeader) );	// Header
				if( Ret == 0 && iLen > 0 )	
					Ret = m_pSendBuffer->Push( pData, iLen );						// Data
				m_SendSync.UnLock();

				nBufLen = dnHeader.iLen;
			}
			break;
		}

		if (Ret == 0)
		{
			m_pIocpManager->m_nAddSendBufSize += nBufLen;
			m_pIocpManager->AddSendCall(m_pSocketContext);
		}
		else if (Ret < 0)
			m_pIocpManager->DetachSocket(m_pSocketContext, L"AddSendData Error");
	}
	return Ret;
}

// 2009.01.23 kimbap
// FlushSendData() returns 0 if Send succeeded normally or there is no data to send, and returns a non-zero value otherwise
// Handle in SendThread so the queue is not emptied.

int CConnection::FlushSendData()
{
	if (m_pSocketContext && m_pIocpManager)
	{
		//Under the current structure, this is accessed from the send Thread side. The places that frequently access SendBuffer are ProcessThread and
		//This will be the SendThread~
		ScopeLock<CSyncLock> Lock( m_SendSync );

		int nSize = GetSendCount();
		if (nSize <= 0)
		{
			if (GetConnectionKey() == CONNECTIONKEY_USER && GetDetachFlag())
				return -1;
			return 0;
		}

#ifdef _USE_SENDCONTEXTPOOL

		ScopeLock<CSyncLock> SendLock(m_pSocketContext->m_SendIOLock);
		
		while (1)
		{
			nSize = GetSendCount();
			if (nSize > 0)
			{
				TIOContext * pContext = m_pSocketContext->GetSendIO();
				if (pContext)
				{
					pContext->Len = ViewSendData(pContext->buffer, INTERNALBUFFERLENMAX);
					if(pContext->Len > 0)
					{
						int PostSendRet = m_pIocpManager->PostSend(m_pSocketContext, pContext);
						//Calling PostSend performs the actual AsyncroSend.
						if( PostSendRet == 0 )
						{
							//buffer skip
							SkipSendData(pContext->Len);
							//SendCount Test Codes
							m_pIocpManager->m_nPostSendSize += pContext->Len;
							nSize = 0;
						}
						else
						{
							m_pSocketContext->ReleaseSendIO(pContext, pContext->Len); //Return the Context and let it run again.
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
							if( PostSendRet == IN_DISCONNECT )
								return IN_DISCONNECT;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
							return 1;
						}
					}
					else
					{
						//Count was checked beforehand before entering, which shouldn't happen.
						_DANGER_POINT();
						m_pSocketContext->ReleaseSendIO(pContext, pContext->Len); //Return the Context and let it run again.
						return 1;
					}
				}
				else
				{
					// Add log
					//_DANGER_POINT();
					return 1;
				}
			}
			else 
			{
				if (GetConnectionKey() == CONNECTIONKEY_USER && GetDetachFlag())
					return -1;
				return 0;
			}
		}
#else
		if (m_pSocketContext->m_SendIO.Len == 0)
		{
			//IOLen decreases when SendComplete fires at the IOCP (lower) layer. So in this case, if completion does not fire,
			//SendProcess does not block, and proceeds to the next turn.
			//Then if complete does not arrive, it keeps piling up......
			m_pSocketContext->m_SendIO.Len = ViewSendData(m_pSocketContext->m_SendIO.buffer, INTERNALBUFFERLENMAX);
			if( m_pSocketContext->m_SendIO.Len > 0 )
			{
				//Calling PostSend performs the actual AsyncroSend.
				if( m_pIocpManager->PostSend(m_pSocketContext) == 0 )
				{
					//SendCount Test Codes
					m_pIocpManager->m_nPostSendSize += m_pSocketContext->m_SendIO.Len;
				}
			}

			if (GetConnectionKey() == CONNECTIONKEY_USER && GetDetachFlag() && GetSendCount() <= 0)
				return -1;
			return 0;
		}
#endif
	}

	return 1;
}

#ifdef _USE_SENDCONTEXTPOOL
bool CConnection::SendComplete(TIOContext * pContext, int nSize)
{
	ScopeLock<CSyncLock> Lock( m_SendSync );
	return m_pSocketContext->ReleaseSendIO(pContext, nSize);
}
#else
bool CConnection::SendComplete(int nSize)
{
	ScopeLock<CSyncLock> Lock( m_SendSync );

	SkipSendData(nSize);
	m_pSocketContext->m_SendIO.Len -= nSize;
	if (m_pSocketContext->m_SendIO.Len == 0)
		return false;
	return true;
}
#endif

int CConnection::GetSendCount()
{
	return m_pSendBuffer->GetCount();
}

int CConnection::ViewSendData(char *pData, int nMaxSize)
{
	// Function synchronized at a higher level
	int Size = m_pSendBuffer->GetCount();
	if (Size > 0){
		if (Size > nMaxSize) Size = nMaxSize;
		m_pSendBuffer->View(pData, Size);
	}
	return Size;
}

void CConnection::SkipSendData(int nSize)
{
	// Function synchronized at a higher level
	m_pSendBuffer->Skip(nSize);
}

bool CConnection::FlushRecvData(ULONG nCurTick)
{
#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( m_FlushRecvDataLock );
#endif
	ScopeSpinBufferSwitch Scope( m_pRecvBuffer );

	char*	pBuffer = Scope.pGetBuffer();
	UINT	uiSize	= Scope.uiGetSize();
	int nCnt = 0, nElapsed = 0, nBufLen = 0;

	int nRet = ERROR_UNKNOWN_HEADER, nElapsedTick = 0;
	while( uiSize )
	{
		if (m_pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER){	// User is encrypted
			DNEncryptPacketSeq * pEnPacket = (DNEncryptPacketSeq*)pBuffer;
			int nLen = pEnPacket->nLen - sizeof(BYTE) - sizeof(USHORT);

			// BufferOverrun
			if( nLen <= 0 || static_cast<int>(uiSize) < pEnPacket->nLen )
			{
				g_Log.Log( LogType::_ERROR, L"## FlushRecvData() BufferOverRun Check-1!!!" );
				return false;
			}

			CDNSecure::GetInstance().Tea_decrypt( reinterpret_cast<char*>(&pEnPacket->Packet), nLen );
			if( pEnPacket->Packet.cMainCmd == IN_DISCONNECT || nLen <= 0 )
				return false;

			nElapsedTick = 0;
			nRet = FlushRecvMessage( (char*)&pEnPacket->Packet, nElapsedTick, nCurTick );

			//Disconnect only if it is a user.
			//if (nRet == ERROR_INVALIDPACKET || nRet == ERROR_UNKNOWN_HEADER)
			if (nRet == ERROR_INVALIDPACKET )
			{
				DNTPacket * pPacket = (DNTPacket*)&pEnPacket->Packet;
				g_Log.Log( LogType::_ERROR, 0, 0, 0, GetSessionID(), L"## Invalid Packet RETCODE:%d SID:%u MCMD:%d SCMD:%d\r\n", nRet, GetSessionID(), pPacket->cMainCmd, pPacket->cSubCmd);
				return false;		//Disconnecting here means an unhandled or mismatched packet was received.
			}
			nElapsed += nElapsedTick;
			nBufLen = pEnPacket->nLen;
		}
#ifdef PRE_ADD_DOORS
		else if (m_pSocketContext->m_dwKeyParam == CONNECTIONKEY_DOORS)
		{
			DNDoorsPacket * pPacket = reinterpret_cast<DNDoorsPacket*>(pBuffer);
			nRet = MessageProcess(pPacket->header.protocol, 0, pPacket->data, pPacket->header.datasize - (int)(sizeof(*pPacket) - sizeof(pPacket->data)));;
			nBufLen = pPacket->header.datasize;
		}
#endif		//#ifdef PRE_ADD_DOORS
		else {	// Do not encrypt anything else
			DNTPacket* pHeader = reinterpret_cast<DNTPacket*>(pBuffer);
			if( pHeader->cMainCmd == IN_DISCONNECT )
				return false;

			nElapsedTick = 0;
			nRet = FlushRecvMessage( pBuffer, nElapsedTick, nCurTick );
			nElapsed += nElapsedTick;
			nBufLen = pHeader->iLen;
		}

		// BufferOverrun
		if( static_cast<int>(uiSize) < nBufLen )
		{
			g_Log.Log( LogType::_ERROR, L"## FlushRecvData() BufferOverRun Check-2!!!" );
			return false;
		}

		pBuffer += nBufLen;
		uiSize  -= nBufLen;

		nCnt++;
	}

	if (nCnt > STORED_RECV_LIMIT && nElapsed > STORED_RECV_ELAPSED_TICK)
	{
		if (m_nStagnatedCount > STAGNATED_DISCONNECT_COUNT)
			m_pIocpManager->StagnatePacket(nCnt, nElapsed, m_pSocketContext);
		else
			m_nStagnatedCount++;
	}
	else
		m_nStagnatedCount = 0;

	return true;
}

#if defined (_DBSERVER) || defined (_LOGSERVER) || defined(_CASHSERVER)

bool CConnection::FlushDBData( int nThreadID )
{
	ScopeSpinBufferSwitch Scope( m_pRecvBuffer );

	char*	pBuffer	= Scope.pGetBuffer();
	UINT32	uiSize	= Scope.uiGetSize();
	
	while( uiSize )
	{
		DNTPacket* pHeader = reinterpret_cast<DNTPacket*>(pBuffer);
		if( pHeader->cMainCmd == IN_DISCONNECT )
			return false;

		DBMessageProcess( pBuffer, nThreadID );

		pBuffer += pHeader->iLen;
		uiSize  -= pHeader->iLen;
	}

	return true;
}

#endif	// #if defined (_DBSERVER)

#ifdef _PACKET_COMP
void CConnection::SetPacketComp(bool bComp)
{
	if (m_pCompBuffer == NULL) return;
	if (bComp == false)
	{
		FlushCompData();
		m_bUseComp = false;
	}
	else
		m_bUseComp = true;
}

void CConnection::FlushCompData()
{
	if (m_pCompBuffer == NULL) return;
	ScopeSpinBufferSwitch Scope(m_pCompBuffer);

	char*	pBuffer	= Scope.pGetBuffer();
	UINT32	uiSize	= Scope.uiGetSize();
	if (uiSize <= 0) return;

	DNEncryptPacketSeq EnPacket = { 0, };

	m_SendSync.Lock();

	EnPacket.cSeq = COMPRESSPACKET;
	memcpy(&EnPacket.Buf, pBuffer, uiSize);

	EnPacket.nLen = EnPacket.Packet.iLen + sizeof(BYTE) + sizeof(USHORT);
	CDNSecure::GetInstance().Tea_encrypt( reinterpret_cast<char*>(&EnPacket.Packet), EnPacket.Packet.iLen );

	int Ret = 0;
	if (EnPacket.nLen > 0)	Ret = m_pSendBuffer->Push( (char*)&EnPacket, EnPacket.nLen, true );
	else Ret = -1;

	m_SendSync.UnLock();

	if (Ret < 0)	_ASSERT(0);
	int nBufLen = EnPacket.nLen;

	if (Ret == 0)
	{
		m_pIocpManager->m_nAddSendBufSize += nBufLen;
		m_pIocpManager->AddSendCall(m_pSocketContext);
	}
	else if (Ret < 0)
		m_pIocpManager->DetachSocket(m_pSocketContext, L"AddSendData Error");
}
#endif

int CConnection::FlushRecvMessage(char * pData, int &nElapsedTick, ULONG nCurTick)
{
	DNTPacket * pPacket = (DNTPacket*)pData;

	int cnt = m_DebugInfoCount++&31;
	m_DebugInfo[cnt]._DebugMainCmd = pPacket->cMainCmd;
	m_DebugInfo[cnt]._DebugSubCmd = pPacket->cSubCmd;
	m_DebugInfo[cnt]._DebugTick = nCurTick;

	int nRet = MessageProcess(pPacket->cMainCmd, pPacket->cSubCmd, pPacket->buf, pPacket->iLen - (int)(sizeof(*pPacket) - sizeof(pPacket->buf)));

	//test
	if (m_pIocpManager && m_pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER)
		m_pIocpManager->m_nProcessBufSize += pPacket->iLen;

	return nRet;
}

void CConnection::SetSocketContext(CIocpManager *pIocpManager, CSocketContext *pSocketContext)
{
	m_pIocpManager = pIocpManager;
	m_pSocketContext = pSocketContext;
}	

void CConnection::BufferClear()
{
	m_SendSync.Lock();
	if (m_pSendBuffer) m_pSendBuffer->Clear();
	m_SendSync.UnLock();

	m_pRecvBuffer->Clear();
}

void CConnection::RecvBufferClear()
{
	m_pRecvBuffer->Clear();
}

void CConnection::SendBufferClear()
{
	m_SendSync.Lock();
	if (m_pSendBuffer) m_pSendBuffer->Clear();
	m_SendSync.UnLock();
}

void CConnection::DetachConnection(wchar_t *pwszIdent)
{
	printf("[DETACH_CONNECTION] : %ws \n",pwszIdent);
	if( pwszIdent && wcslen(pwszIdent) > 0)	
		m_pSocketContext->SetDetachReason(pwszIdent);

	m_pIocpManager->DetachSocket(m_pSocketContext, pwszIdent == NULL ? const_cast<wchar_t*>(m_wstrDelayDetachReson.c_str()) : pwszIdent);	
}

void CConnection::DelayDetachConnection(wchar_t *pwszIdent)
{
	if (GetConnectionKey() == CONNECTIONKEY_USER)
	{
		m_wstrDelayDetachReson.clear();
		m_wstrDelayDetachReson = pwszIdent;
		m_bDetachFlag = true;
	}
}

void CConnection::SetDelete(bool bDelete)
{
	m_DeleteLock.Lock();
	m_bDelete = bDelete;
	m_DeleteLock.UnLock();
}

bool CConnection::GetDelete()
{
	m_DeleteLock.Lock();
	bool bDelete = m_bDelete;
	m_DeleteLock.UnLock();

	return bDelete;
}

DWORD CConnection::GetConnectionKey()
{
	if (m_pSocketContext) return m_pSocketContext->m_dwKeyParam;
	else return CONNECTIONKEY_DEFAULT;
}

#endif	// #if defined(_SERVER)