#include "stdafx.h"
#include "DnFrameBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnFrameBlow::CDnFrameBlow(DnActorHandle hActor, const char *szValue)
	: CDnBlow(hActor)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_025;
	SetValue(szValue);
	m_fValue = (float)atof(szValue);
}

CDnFrameBlow::~CDnFrameBlow(void)
{
}

void CDnFrameBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_hActor->UpdateFPS();

	OutputDebug( "CDnFrameBlow::OnBegin, Value:%f \n", m_fValue );
}

void CDnFrameBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{ 
	// When no time limit is set, it may be set to STATE_DURATION, so treat it as a valid state effect in UpdateFPS()
	// Cases occur where it gets classified and frame rate value gets applied.
	SetState( STATE_BLOW::STATE_END );
	
	m_hActor->UpdateFPS();

	OutputDebug( "CDnFrameBlow::OnEnd\n" );
}

void CDnFrameBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	m_hActor->UpdateFPS();
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFrameBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnFrameBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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