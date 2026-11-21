#include "stdafx.h"
#include "Buffer.h"
#include "Log.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CBuffer::CBuffer(const int nSize): m_Head(0), m_Tail(0), m_Count(0), m_MaxSize(0)
{
	m_MaxSize = nSize;
	m_pBuffer = new char[nSize];
	memset(m_pBuffer, 0, sizeof(char)*nSize);
}

CBuffer::~CBuffer(void)
{
	SAFE_DELETEA(m_pBuffer);
}

int CBuffer::GetCount()
{
	m_Lock.Lock();
	int nCount = m_Count;
	m_Lock.UnLock();

	return nCount;
}

void CBuffer::Clear(bool bBufferClear)
{
	m_Lock.Lock();
	m_Head = m_Tail = m_Count = 0;
	if (bBufferClear) memset(m_pBuffer, 0, sizeof(char)*m_MaxSize);
	m_Lock.UnLock();
}

int CBuffer::Push(const char *pData, int nSize, bool bIsUserSession/*=false*/, bool bIncreaseBuffer/*=false*/ )
{
	m_Lock.Lock();

	if (nSize <= 0)
	{
		m_Lock.UnLock();
		return -1;
	}

	// Expand buffer
	if (m_Count + nSize > m_MaxSize){
		int NewSize = m_MaxSize * 2;
		if (NewSize < m_Count + nSize) NewSize = m_Count + nSize;		

		if( (bIsUserSession&&!bIncreaseBuffer) || NewSize > 10000000 )
		{
			m_Lock.UnLock();
			return -2;	// insufficient buffer
		}

		char *pNewBuffer = new char [NewSize];
		View(pNewBuffer, m_Count);
		m_Head = 0;
		m_Tail = m_Count;
		m_MaxSize = NewSize;

		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
	}

	if (m_Tail + nSize <= m_MaxSize)
	{
		memcpy(m_pBuffer + m_Tail, pData, nSize);
		m_Tail += nSize;
	}
	else {
		int Temp = m_MaxSize - m_Tail;
		memcpy(m_pBuffer + m_Tail, pData, Temp);
		memcpy(m_pBuffer, pData + Temp, nSize - Temp);
		m_Tail = (m_Tail + nSize) % m_MaxSize;
	}

	m_Count += nSize;
	m_Lock.UnLock();

	return 0;
}

int CBuffer::Insert( const char *pData, int nSize )
{
	m_Lock.Lock();

	if (nSize <= 0)
	{
		m_Lock.UnLock();
		return -1;
	}

	// Expand buffer
	if (m_Count + nSize > m_MaxSize) {
		int NewSize = m_MaxSize * 2;
		if (NewSize < m_Count + nSize) NewSize = m_Count + nSize;

		if( NewSize > 10000000 )
		{
			m_Lock.UnLock();
			return -1;
		}

		char *pNewBuffer = new char [NewSize];
		memcpy( pNewBuffer, pData, nSize );
		View(pNewBuffer + nSize, m_Count);
		m_Head = 0;
		m_Tail = m_Count + nSize;
		m_MaxSize = NewSize;

		delete []m_pBuffer;
		m_pBuffer = pNewBuffer;
	}
	else {
		if (m_Head - nSize < 0)
		{
			int Temp = nSize - m_Head;
			memcpy(m_pBuffer, pData + nSize - m_Head, m_Head);
			memcpy(m_pBuffer + m_MaxSize - Temp, pData, Temp);
			m_Head = m_MaxSize - Temp;
		}
		else {
			memcpy( m_pBuffer + m_Head - nSize, pData, nSize );
			m_Head -= nSize;
		}
	}
	m_Count += nSize;
	m_Lock.UnLock();

	return 0;
}

int CBuffer::Pop(char *pData, const int nSize)
{
	m_Lock.Lock();

	if ((nSize <= 0) || (m_Count < nSize)){
		m_Lock.UnLock();
		return -1;
	}

	if (m_Head + nSize < m_MaxSize){
		memcpy(pData, m_pBuffer + m_Head, nSize);
		m_Head += nSize;
	}
	else {
		int Temp = m_MaxSize - m_Head;
		memcpy(pData, m_pBuffer + m_Head, Temp);
		memcpy(pData + Temp, m_pBuffer, nSize - Temp);
		m_Head = (m_Head + nSize) % m_MaxSize;
	}
	m_Count -= nSize;
	m_Lock.UnLock();

	return 0;
}

int CBuffer::View(char *pData, const int nSize)
{
	m_Lock.Lock();

	if ((nSize <= 0) || (m_Count < nSize)){
		m_Lock.UnLock();
		return -1;
	}

	if (m_Head + nSize < m_MaxSize){
		memcpy(pData, m_pBuffer + m_Head, nSize);
	}
	else {
		int Temp = m_MaxSize - m_Head;
		memcpy(pData, m_pBuffer + m_Head, Temp);
		memcpy(pData + Temp, m_pBuffer, nSize - Temp);
	}

	m_Lock.UnLock();
	return 0;
}

int CBuffer::Skip(const int nSize)
{
	m_Lock.Lock();

	if ((nSize <= 0) || (m_Count < nSize)){
		m_Lock.UnLock();
		return -1;
	}

	if (m_Head + nSize < m_MaxSize){
		m_Head += nSize;
	}
	else {
		m_Head = (m_Head + nSize) % m_MaxSize;
	}
	m_Count -= nSize;

	m_Lock.UnLock();
	return 0;
}

bool CBuffer::IsComplete()	// For SendBuffer
{
	USHORT SendSize = 0; 

	m_Lock.Lock();
	if (m_Count >= (int)sizeof(USHORT)){
		View((char*)&SendSize, sizeof(USHORT));

		if (m_Count >= SendSize){
			m_Lock.UnLock();
			return true;
		}
	}
	m_Lock.UnLock();
	return false;
}

int CBuffer::IsComplete(bool boServer)	// For RecvBuffer
{
	USHORT RecvSize = 0;

	m_Lock.Lock();

	if (m_Count >= (int)sizeof(USHORT)){
		View((char*)&RecvSize, sizeof(USHORT));

		if (RecvSize == 0 || (!boServer && RecvSize > 4096) || (boServer && RecvSize > 51200)){
			m_Lock.UnLock();
			return SIZEERR;
		}
		if (m_Count >= RecvSize){
			m_Lock.UnLock();
			return COMPLETE;
		}
	}

	m_Lock.UnLock();
	return NONE;
}

USHORT CBuffer::GetComplete(char *pData)	// Get completion packet
{
	USHORT Recv = 0;

	m_Lock.Lock();
	View((char*)&Recv, sizeof(USHORT));

	if (Pop(pData, Recv) < 0){
		m_Lock.UnLock();
		return 0;
	}

	m_Lock.UnLock();
	return Recv;
}
