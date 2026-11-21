#include "stdafx.h"
#include "EtPackingStream.h"
#include "EtFileSystem.h"
#include <assert.h>


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-------------------------------------------------------------------------------------------------------------

CPackingStream::CPackingStream()
{
	m_pStream = NULL;	
}

CPackingStream::CPackingStream( const char *pFileName )
{
	m_pStream = NULL;
	Open( pFileName);
}

CPackingStream::~CPackingStream()
{
	Close();
}

void CPackingStream::Activate()
{
	Open( m_szFileName.c_str());
}

bool CPackingStream::Open( const char *pFileName )
{
	Close();

#ifdef _UNICODE
	std::wstring wstr;

	WCHAR wszBuff[8192] = L"";
	ZeroMemory(wszBuff, sizeof(WCHAR)*8192);
	MultiByteToWideChar(CP_ACP, 0, pFileName, -1, wszBuff, 8192 );
	wstr = wszBuff;

	m_pStream = CEtFileSystem::GetInstance().OpenFile(wstr.c_str());
#else
	m_pStream = CEtFileSystem::GetInstance().OpenFile(pFileName);
#endif
	
	if ( m_pStream == NULL )
		return false;

	m_szFileName = pFileName;

	return IsValid();
}

void CPackingStream::Close()
{
	if ( m_pStream )
	{
		CEtFileSystem::GetInstance().CloseFile(m_pStream);
		m_pStream = NULL;
	}
}

bool CPackingStream::IsEnd() const
{
	return false;
}

long CPackingStream::Tell()
{
	_ASSERT( m_pStream );

	if (!m_pStream)
		return 0;

	return Seek(0, SEEK_CUR);
}

int CPackingStream::Size() const
{
	_ASSERT( m_pStream );

	if (!m_pStream)
		return -1;

	return m_pStream->GetFileContext()->dwOriginalSize;

}

bool CPackingStream::IsValid() const
{
	return ( m_pStream != NULL );
}

int CPackingStream::Seek( long lOffset, int nOrigin ) 
{
	_ASSERT( m_pStream );

	if ( m_pStream )
	{
		return m_pStream->Seek(lOffset, nOrigin);
	}
	return -1;
}

int CPackingStream::Read( void *pBuffer, int nSize )
{
	if ( m_pStream )
	{
		return m_pStream->Read(pBuffer, nSize);
	}
	return -1;
}

int CPackingStream::Write( const void *pBuffer, int nSize )
{
	_ASSERT(false && "Can't write this Stream." );
	return -1;
}
