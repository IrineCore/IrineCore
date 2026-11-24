#include "stdafx.h"
#include "DnDestroySuperArmorBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnDestroySuperArmorBlow::CDnDestroySuperArmorBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_079;
	SetValue( szValue );
}

CDnDestroySuperArmorBlow::~CDnDestroySuperArmorBlow(void)
{

}

void CDnDestroySuperArmorBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}

void CDnDestroySuperArmorBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnDestroySuperArmorBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDestroySuperArmorBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnDestroySuperArmorBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW