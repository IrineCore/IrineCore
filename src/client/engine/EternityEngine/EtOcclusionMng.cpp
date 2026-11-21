#include "StdAfx.h"
#include "EtOcclusionMng.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtOcclusionMng::CEtOcclusionMng(void)
{
	m_nAllocCount = 0;
}

CEtOcclusionMng::~CEtOcclusionMng(void)
{
	SAFE_DELETE_PVEC( m_vecOcclusionQuery );
}

int CEtOcclusionMng::AllocOcclusion()
{
	int nAllocIndex;

	nAllocIndex = m_nAllocCount;
	if( nAllocIndex >= ( int )m_vecOcclusionQuery.size() )
	{
		CEtOcclusionQuery *pQuery = new CEtOcclusionQuery();
//		pQuery->SetAsyncType( true );
		m_vecOcclusionQuery.push_back( pQuery );
	}
	m_nAllocCount++;

	return nAllocIndex;
}

void CEtOcclusionMng::BeginOcclusion( int nIndex )
{
	ASSERT( nIndex >= 0 && nIndex < ( int )m_vecOcclusionQuery.size() );
	if( nIndex < 0 || nIndex >= ( int )m_vecOcclusionQuery.size() )
	{
		return;
	}
	m_vecOcclusionQuery[ nIndex ]->Begin();
}

void CEtOcclusionMng::EndOcclusion( int nIndex )
{
	ASSERT( nIndex >= 0 && nIndex < ( int )m_vecOcclusionQuery.size() );
	if( nIndex < 0 || nIndex >= ( int )m_vecOcclusionQuery.size() )
	{
		return;
	}
	m_vecOcclusionQuery[ nIndex ]->End();
}

DWORD CEtOcclusionMng::ResultOcclusion( int nIndex )
{
	ASSERT( nIndex >= 0 && nIndex < ( int )m_vecOcclusionQuery.size() );
	if( nIndex < 0 || nIndex >= ( int )m_vecOcclusionQuery.size() )
	{
		return 0xffffffff;
	}
	DWORD dwResult;
	if( m_vecOcclusionQuery[ nIndex ]->GetResult( dwResult ) )
	{
		return dwResult;
	}
	else
	{
		// Occlusion check failed, so draw unconditionally.
		return 0xffffffff;
	}
}

void CEtOcclusionMng::FlushBuffer()
{
	SAFE_DELETE_PVEC( m_vecOcclusionQuery );
}