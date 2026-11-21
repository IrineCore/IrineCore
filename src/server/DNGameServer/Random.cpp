#include "stdafx.h"
#include "Random.h"


STATIC_DECL_INIT( CRandom, int, s_nStaticCount ) = { 0, };
STATIC_DECL_INIT( CRandom, int, s_nLastSeed ) = { 0, };
STATIC_DECL_INIT( CRandom, int, s_nLockSeed ) = { 0, };
STATIC_DECL_INIT( CRandom, int, s_nLockLeft ) = { 0, };
STATIC_DECL_INIT( CRandom, int, s_nLockInitf ) = { 0, };
STATIC_DECL_INIT( CRandom, unsigned long *,s_pNext ) = { 0, };

int CRandom::Seed( CMultiRoom *pRoom )
{
	int nSeed = timeGetTime();
	int nTemp = nSeed;
	if( nSeed == STATIC_INSTANCE_( s_nLastSeed ) ) {
		nSeed += STATIC_INSTANCE_( s_nStaticCount )++;
	}
	STATIC_INSTANCE_( s_nLastSeed ) = nTemp;

	return nSeed;
} 

void CRandom::LockSeed( CMultiRoom *pRoom )
{
	STATIC_INSTANCE_( s_nLockSeed ) = CRandom::GetInstance(pRoom).m_seed;
	STATIC_INSTANCE_( s_nLockLeft ) = CRandom::GetInstance(pRoom).m_left;
	STATIC_INSTANCE_( s_nLockInitf ) = CRandom::GetInstance(pRoom).m_initf;
	STATIC_INSTANCE_( s_pNext ) = CRandom::GetInstance(pRoom).m_next;
}

void CRandom::UnlockSeed( CMultiRoom *pRoom )
{
	((CMtRandom*)CRandom::GetInstancePtr(pRoom))->m_seed = STATIC_INSTANCE_( s_nLockSeed );
	((CMtRandom*)CRandom::GetInstancePtr(pRoom))->m_left = STATIC_INSTANCE_( s_nLockLeft );
	((CMtRandom*)CRandom::GetInstancePtr(pRoom))->m_initf = STATIC_INSTANCE_( s_nLockInitf );
	((CMtRandom*)CRandom::GetInstancePtr(pRoom))->m_next = STATIC_INSTANCE_( s_pNext );
}
