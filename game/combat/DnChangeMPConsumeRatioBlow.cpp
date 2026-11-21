#include "stdafx.h"
#include "DnChangeMPConsumeRatioBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChangeMPConsumeRatioBlow::CDnChangeMPConsumeRatioBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_139;

	SetValue( szValue );
	m_fValue = (float)atof( szValue );

	// Catch and deduct on MP consumption.
	AddCallBackType( SB_ONUSEMP );
}

CDnChangeMPConsumeRatioBlow::~CDnChangeMPConsumeRatioBlow(void)
{

}

void CDnChangeMPConsumeRatioBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	

	OutputDebug( "CDnChangeMPConsumeRatioBlow::OnBegin Value:%2.2f\n", m_fValue );
}


void CDnChangeMPConsumeRatioBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnChangeMPConsumeRatioBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	

	OutputDebug( "CDnChangeMPConsumeRatioBlow::OnEnd Value:%2.2f\n", m_fValue );
}

#ifdef _GAMESERVER
int CDnChangeMPConsumeRatioBlow::OnUseMP( int iMPDelta )
{
	// Since MP consumption is decided and sent by the server, changing it only here also updates the client.
	int iResult = int((float)iMPDelta * m_fValue);

	return iResult;
}
#endif 

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChangeMPConsumeRatioBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnChangeMPConsumeRatioBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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