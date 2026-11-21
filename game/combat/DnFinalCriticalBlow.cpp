#include "stdafx.h"
#include "DnFinalCriticalBlow.h"

#ifdef _CLIENT
#include "DnInterface.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnFinalCriticalBlow::CDnFinalCriticalBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_251;
	SetValue( szValue );

	m_fValue = (float)atof(szValue);
}

CDnFinalCriticalBlow::~CDnFinalCriticalBlow(void)
{

}


void CDnFinalCriticalBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW) && defined (PRE_FIX_CHARSTATUS_REFRESH) && defined(_CLIENT)
	// A bit tricky: CharStatus normally updates when stats change, but since this state effect does not touch stats, there is no clear refresh trigger.
	// Rarely expressed state effect, so keep as this state effect for now; if more cases arise later, lower to DnBlow level and update.
	if( CDnActor::s_hLocalActor && m_hActor == CDnActor::s_hLocalActor )
		GetInterface().OnRefreshLocalPlayerStatus();
#endif

}

void CDnFinalCriticalBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
}


void CDnFinalCriticalBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

#if defined(PRE_ADD_CHAR_STATUS_SECOND_RENEW) && defined (PRE_FIX_CHARSTATUS_REFRESH) && defined(_CLIENT)
	if( m_hActor == CDnActor::s_hLocalActor )
		GetInterface().OnRefreshLocalPlayerStatus();
#endif

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFinalCriticalBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	szNewValue = szOrigValue;
}

void CDnFinalCriticalBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
