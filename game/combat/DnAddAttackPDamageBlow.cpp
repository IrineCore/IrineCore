#include "stdafx.h"
#include "DnAddAttackPDamageBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnAddAttackPDamageBlow::CDnAddAttackPDamageBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_213;

	SetValue(szValue);

	m_fValue = (float)atof(szValue);
}

CDnAddAttackPDamageBlow::~CDnAddAttackPDamageBlow(void)
{
}


void CDnAddAttackPDamageBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}


void CDnAddAttackPDamageBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);
}


void CDnAddAttackPDamageBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAddAttackPDamageBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnAddAttackPDamageBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Calculate value
	float fResultValue = fValue[0] - fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
