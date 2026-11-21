#include "stdafx.h"
#include "DnInvisibleEffectBlow.h"
#include "DnPlayerActor.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnInvisibleEffectBlow::CDnInvisibleEffectBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_nTransformActorID( -1 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_298;
	SetValue( szValue );
	m_nTransformActorID = szValue ? atoi( szValue ) : -1;
}

CDnInvisibleEffectBlow::~CDnInvisibleEffectBlow( void )
{
}

void CDnInvisibleEffectBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor || !m_hActor->IsPlayerActor() )
		return;

	CDnPlayerActor* pPlayer = static_cast<CDnPlayerActor*>( m_hActor.GetPointer() );
	if( pPlayer && !pPlayer->IsSwapSingleSkin() )
	{
#ifdef _GAMESERVER
		pPlayer->SwapSingleSkin( m_nTransformActorID );
#else
		pPlayer->SwapSingleSkin( m_nTransformActorID, true );
#endif
	}
}

void CDnInvisibleEffectBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnInvisibleEffectBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor || !m_hActor->IsPlayerActor() )
		return;

	CDnPlayerActor* pPlayer = static_cast<CDnPlayerActor*>( m_hActor.GetPointer() );
	if( pPlayer )
	{
#ifdef _GAMESERVER
		pPlayer->SwapSingleSkin( -1 );
#else
		pPlayer->SwapSingleSkin( -1, true );
#endif
	}
}
