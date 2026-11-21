#include "stdafx.h"
#include "DnChangeWeightBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnChangeWeightBlow::CDnChangeWeightBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ), m_fWeightDelta( 0.0f )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_131;

	SetValue( szValue );
	m_fValue = (float)atof( szValue );
}

CDnChangeWeightBlow::~CDnChangeWeightBlow(void)
{
}


void CDnChangeWeightBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Store the previous weight value. 
	// Multiple state effects can be applied, so store only the delta value and restore it later.
	float fNowWeightValue = m_hActor->GetWeight();
	m_fWeightDelta = fNowWeightValue - m_fValue;

	// Apply new weight value.
	m_hActor->SetWeight( m_fValue );

	OutputDebug( "CDnChangeWeightBlow::OnBegin Value: Now: %2.2f, Dest: %2.2f, Delta: %2.2f\n", fNowWeightValue, m_fValue, m_fWeightDelta );
}


void CDnChangeWeightBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	float fNowWeightValue = m_hActor->GetWeight();
	m_hActor->SetWeight( fNowWeightValue + m_fWeightDelta );

	OutputDebug( "CDnChangeWeightBlow::OnEnd Value: Now: %2.2f, Dest: %2.2f, Delta: %2.2f\n", fNowWeightValue, m_fValue, m_fWeightDelta );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChangeWeightBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnChangeWeightBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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