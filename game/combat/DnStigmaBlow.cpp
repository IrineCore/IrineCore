#include "stdafx.h"
#include "DnStigmaBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnStigmaBlow::CDnStigmaBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_246;

	SetValue( szValue );
	m_fValue = (float)atof(szValue);

#if defined(_GAMESERVER)
	AddCallBackType(SB_ONTARGETHIT);
#endif // _GAMESERVER
}

CDnStigmaBlow::~CDnStigmaBlow(void)
{

}

void CDnStigmaBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

	OutputDebug( "%s\n", __FUNCTION__ );
}


void CDnStigmaBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

}


void CDnStigmaBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

	OutputDebug( "%s\n", __FUNCTION__);
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnStigmaBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnStigmaBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW