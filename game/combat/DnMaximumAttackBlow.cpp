#include "stdafx.h"
#include "DnMaximumAttackBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMaximumAttackBlow::CDnMaximumAttackBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_074;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
}

CDnMaximumAttackBlow::~CDnMaximumAttackBlow(void)
{

}


void CDnMaximumAttackBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnMaximumAttackBlow::OnBegin" );
}


void CDnMaximumAttackBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnMaximumAttackBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnMaximumAttackBlow::OnEnd" );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnMaximumAttackBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnMaximumAttackBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
