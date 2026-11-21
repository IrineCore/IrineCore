#include "stdafx.h"
#include "DnMonsterOrderBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnMonsterOrderBlow::CDnMonsterOrderBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_nOrderID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_287;
	SetValue( szValue );
	m_nOrderID = szValue ? atoi( szValue ) : 0;
}

CDnMonsterOrderBlow::~CDnMonsterOrderBlow( void )
{
}

void CDnMonsterOrderBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
}

void CDnMonsterOrderBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnMonsterOrderBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
}
