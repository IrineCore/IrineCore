#include "stdafx.h"
#include "DnFearBlow.h"
#include "DnCantActionBlow.h"
#include "DnPlayerActor.h"
#ifndef _GAMESERVER
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "InputWrapper.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnFearBlow::CDnFearBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_pCantActionBlow( NULL )
	, m_nParam0( 0 )
	, m_fReduceTimePerMash( 0.0f )
#ifndef _GAMESERVER
	, m_ComboCalc( CDnComboCalculator::CIRCULAR_CHECK )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_305;
	SetValue( szValue );

	m_pCantActionBlow = new CDnCantActionBlow( hActor, NULL );
	if( m_pCantActionBlow )
		m_pCantActionBlow->SetPermanent( true );

	std::vector<std::string> vlTokens;
	std::string strValue = szValue ? szValue : "";
	TokenizeA( strValue, vlTokens, ";" );
	if( vlTokens.size() == 2 )
	{
		m_nParam0 = atoi( vlTokens[0].c_str() );
		m_fReduceTimePerMash = (float)atoi( vlTokens[1].c_str() ) * 0.001f;
	}

#ifndef _GAMESERVER
	std::vector<BYTE> vlKeysToCheck;
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
	vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );
	m_ComboCalc.SetKeysToCheck( vlKeysToCheck );

	vlKeysToCheck.clear();
	vlKeysToCheck.push_back( IW_MOVELEFT );  vlKeysToCheck.push_back( IW_MOVELEFT );  vlKeysToCheck.push_back( IW_MOVELEFT );
	vlKeysToCheck.push_back( IW_MOVERIGHT ); vlKeysToCheck.push_back( IW_MOVERIGHT ); vlKeysToCheck.push_back( IW_MOVERIGHT );
	vlKeysToCheck.push_back( IW_MOVEBACK );  vlKeysToCheck.push_back( IW_MOVEBACK );  vlKeysToCheck.push_back( IW_MOVEBACK );
	vlKeysToCheck.push_back( IW_MOVEFRONT ); vlKeysToCheck.push_back( IW_MOVEFRONT ); vlKeysToCheck.push_back( IW_MOVEFRONT );
	m_ComboCalc.SetPadsToCheck( vlKeysToCheck, 3.0f );
#endif
}

CDnFearBlow::~CDnFearBlow( void )
{
	SAFE_DELETE( m_pCantActionBlow );
}

void CDnFearBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_pCantActionBlow )
		m_pCantActionBlow->OnBegin( LocalTime, fDelta );

	if( m_hActor && m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayer = static_cast<CDnPlayerActor*>( m_hActor.GetPointer() );
		if( pPlayer && !pPlayer->IsBattleMode() )
			pPlayer->CmdToggleBattle( true );
	}

#ifndef _GAMESERVER
	if( CDnLocalPlayerActor::s_hLocalActor == m_hActor && m_fReduceTimePerMash > 0.0f )
	{
		GetInterface().ShowStickAniDialog( true );
	}
#endif
}

void CDnFearBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	if( m_pCantActionBlow )
		m_pCantActionBlow->Process( LocalTime, fDelta );

#ifndef _GAMESERVER
	if( CDnLocalPlayerActor::s_hLocalActor == m_hActor && m_fReduceTimePerMash > 0.0f )
	{
		int nComboCount = 0;
		m_ComboCalc.Process( LocalTime, fDelta, &nComboCount );
		for( int i = 0; i < nComboCount; ++i )
			m_StateBlow.fDurationTime -= m_fReduceTimePerMash;

		if( m_StateBlow.fDurationTime < 0.2f )
		{
			CDnLocalPlayerActor* pLocal = static_cast<CDnLocalPlayerActor*>( m_hActor.GetPointer() );
			if( pLocal )
				pLocal->CmdRemoveStateEffectByServerBlowID( GetServerBlowID() );
		}
	}
#endif
}

void CDnFearBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_pCantActionBlow )
		m_pCantActionBlow->OnEnd( LocalTime, fDelta );

#ifndef _GAMESERVER
	if( CDnLocalPlayerActor::s_hLocalActor == m_hActor && m_fReduceTimePerMash > 0.0f )
	{
		GetInterface().ShowStickAniDialog( false );
	}
#endif
}
