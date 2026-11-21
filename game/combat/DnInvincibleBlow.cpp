#include "stdafx.h"
#include "DnInvincibleBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnInvincibleBlow::CDnInvincibleBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_099;
	SetValue( szValue );
	m_fValue = (float)atof( szValue ); 

	// If parameter value is -1, also ignore attacks that bypass ID 99 such as Hurricane Dance.
	// Prevent specific attacks from hitting while invincible in special situations
}

CDnInvincibleBlow::~CDnInvincibleBlow(void)
{
}


void CDnInvincibleBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnInvincibleBlow::OnBegin\n");
}


void CDnInvincibleBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnInvincibleBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnInvincibleBlow::OnEnd\n");
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnInvincibleBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnInvincibleBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
