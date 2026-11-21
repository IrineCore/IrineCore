#include "stdafx.h"
#include "DnChangeMAttackDamageBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnChangeMAttackDamageBlow::CDnChangeMAttackDamageBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_135;
	
	SetValue( szValue );
	m_fValue = (float)atof( szValue );
}

CDnChangeMAttackDamageBlow::~CDnChangeMAttackDamageBlow(void)
{
}

void CDnChangeMAttackDamageBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnChangeMAttackDamageBlow::OnBegin\n" );
}


void CDnChangeMAttackDamageBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnChangeMAttackDamageBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnChangeMAttackDamageBlow::OnEnd\n" );
}

#ifdef _GAMESERVER
void CDnChangeMAttackDamageBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	// Time is replaced with the passed value
	m_StateBlow.fDurationTime = StateBlowInfo.fDurationTime;

	const char* szTempValue = StateBlowInfo.szValue.c_str();
	SetValue( szTempValue );
	float fDuplicateValue = (float)atof( szTempValue );

	if( 1.0f < fDuplicateValue )
		fDuplicateValue -= 1.0f;

	m_fValue += fDuplicateValue;
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChangeMAttackDamageBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//Value 0.75 -> only 25% applies. Subtract 1.0 from the second value before adding for the correct value
	//When adding both 0.75 and 0.8 settings
	//Final value is 0.75 + (0.8 - 1.0) = 0.55
	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue ) - 1.0f;
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnChangeMAttackDamageBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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