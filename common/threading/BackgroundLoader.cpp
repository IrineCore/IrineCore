#include "stdafx.h"
#include "Stream.h"
#include "BackgroundLoader.h"
#include "SundriesFunc.h"
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CBackgroundLoaderCallback::~CBackgroundLoaderCallback()
{
	int i;

	for( i = 0; i < ( int )m_vecCallbackPushedLoader.size(); i++ )
	{
		m_vecCallbackPushedLoader[ i ]->EraseCallback( this );
	}
}

void CBackgroundLoaderCallback::OnLoadComplete( CBackgroundLoader *pLoader )
{
	std::vector< CBackgroundLoader * >::iterator it;

	it = std::find( m_vecCallbackPushedLoader.begin(), m_vecCallbackPushedLoader.end(), pLoader );
	if( it != m_vecCallbackPushedLoader.end() ) 
	{
		m_vecCallbackPushedLoader.erase( it );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CBackgroundLoader::s_bBeginBackgroundLoad = false;
bool CBackgroundLoader::s_bNowLoading = false;
HANDLE CBackgroundLoader::s_hThread = NULL;
std::vector< SLoaderInfo > CBackgroundLoader::s_vecLoadList;
CSyncLock CBackgroundLoader::m_LoadListLock;
HANDLE CBackgroundLoader::s_hLoaderEvent[ 2 ] = { NULL, NULL };


CBackgroundLoader::CBackgroundLoader(void)
{
	m_bReady = true;
}

CBackgroundLoader::~CBackgroundLoader(void)
{
}

void CBackgroundLoader::BeginBackgroundLoad()
{
	int i;

	s_bBeginBackgroundLoad = true;

	for( i = 0; i < 2; i++ )
	{
		if( s_hLoaderEvent[ i ] == NULL )
		{
			s_hLoaderEvent[ i ] = CreateEvent( NULL, FALSE, FALSE, NULL );
		}
	}
	if( s_hThread == NULL )
	{
		s_hThread = CreateThread( NULL, 0, BackgroundLoaderCallback, NULL, 0, NULL );
	}
}

void CBackgroundLoader::EndBackgroundLoad()
{
	int i;

	if( s_bBeginBackgroundLoad )
	{
		if( s_hLoaderEvent[ 1 ] )
		{
			SetEvent( s_hLoaderEvent[ 1 ] );
		}
		for( i = 0; i < 2; i++ )
		{
			if( s_hLoaderEvent[ i ] )
			{
				CloseHandle( s_hLoaderEvent[ i ] );
			}
		}
	}
	if( s_hThread )
	{
		CloseHandle( s_hThread );
	}
	s_bBeginBackgroundLoad = false;
}

void CBackgroundLoader::SetReady( bool bReady )
{
	m_LoadListLock.Lock();
	m_bReady = bReady;
	m_LoadListLock.UnLock();
}

bool CBackgroundLoader::AddCallback( CBackgroundLoaderCallback *pCallback )
{
	bool bRet;
	m_LoadListLock.Lock();
	if( !m_bReady )
	{
		m_vecCallback.push_back( pCallback );
		pCallback->CallbackPush( this );
		bRet = true;
	}
	else
	{
		bRet = false;
	}
	m_LoadListLock.UnLock();

	return bRet;
}

void CBackgroundLoader::EraseCallback( CBackgroundLoaderCallback *pCallback )
{
	m_LoadListLock.Lock();
	std::vector< CBackgroundLoaderCallback * >::iterator it;

	it = std::find( m_vecCallback.begin(), m_vecCallback.end(), pCallback );
	if( it != m_vecCallback.end() ) 
	{
		m_vecCallback.erase( it );
	}
	m_LoadListLock.UnLock();
}


void CBackgroundLoader::ProcessCallback()
{
	int i;

	m_LoadListLock.Lock();
	for( i = 0; i < ( int )m_vecCallback.size(); i++ )
	{
		m_vecCallback[ i ]->OnLoadComplete( this );
	}
	m_LoadListLock.UnLock();
}

int CBackgroundLoader::GetRemainLoaderCount() 
{
	int nCount;

	nCount = ( int )s_vecLoadList.size();
	if( s_bNowLoading )
	{
		nCount++;
	}

	return nCount;
}

void CBackgroundLoader::PushLoader( CBackgroundLoader *pLoader, CStream *pStream )
{
	SLoaderInfo LoaderInfo;

	LoaderInfo.pLoader = pLoader;
	LoaderInfo.pStream = pStream;
	m_LoadListLock.Lock();
	pLoader->SetReady( false );
	if( s_vecLoadList.empty() )
	{
		SetEvent( s_hLoaderEvent[ 0 ] );
	}
	s_vecLoadList.push_back( LoaderInfo );
	m_LoadListLock.UnLock();
}

bool CBackgroundLoader::PopLoader( SLoaderInfo &LoaderInfo )
{
	bool bRet;

	m_LoadListLock.Lock();
	if( s_vecLoadList.empty() )
	{
		bRet = false;
	}
	else
	{
		bRet = true;
		LoaderInfo = s_vecLoadList.front();
		s_vecLoadList.erase( s_vecLoadList.begin() );
	}
	m_LoadListLock.UnLock();

	return bRet;
}

DWORD WINAPI CBackgroundLoader::BackgroundLoaderCallback( LPVOID pParam )
{
	while( 1 )
	{
		SLoaderInfo LoaderInfo;

		if( PopLoader( LoaderInfo ) )
		{
			if( ( LoaderInfo.pLoader ) && ( LoaderInfo.pStream ) )
			{
				s_bNowLoading = true;
				if( !LoaderInfo.pStream->IsValid() )
				{
					LoaderInfo.pStream->Activate();
				}
				if( LoaderInfo.pStream->IsValid() )
				{
/*					int nSize;
					char *pLoadBuf;
					CMemoryStream MemStream;

					nSize = LoaderInfo.pStream->Size();
					pLoadBuf = new char[ nSize ];
					LoaderInfo.pStream->Read( pLoadBuf, nSize );
					MemStream.Initialize( pLoadBuf, nSize );
					LoaderInfo.pLoader->LoadResource( &MemStream );
					delete [] pLoadBuf;*/
					LoaderInfo.pLoader->LoadResource( LoaderInfo.pStream );
				}
//				OutputDebug( "ThreadLoad : %s\n", ((CFileStream*)LoaderInfo.pStream)->GetFileName() );

				delete LoaderInfo.pStream;
				LoaderInfo.pLoader->SetReady( true );
				LoaderInfo.pLoader->ProcessCallback();
			}
		}
		else
		{
			s_bNowLoading = false;
			while( 1 )
			{
#if defined( PRE_FIX_CLIENT_FREEZING )
				CommonUtil::PrintFreezingLog( "CBackgroundLoader::BackgroundLoaderCallback - Before" );
#endif

				DWORD dwRet;
				dwRet = WaitForMultipleObjects( 2, s_hLoaderEvent, FALSE, INFINITE );

#if defined( PRE_FIX_CLIENT_FREEZING )
				CommonUtil::PrintFreezingLog( "CBackgroundLoader::BackgroundLoaderCallback - After\n" );
#endif

				if( dwRet == WAIT_OBJECT_0 )
				{
					break;
				}
				else if( dwRet == WAIT_OBJECT_0 + 1 )
				{
					return 1;
				}
			}
		}
		Sleep(1);
	}

	return 0;
}
