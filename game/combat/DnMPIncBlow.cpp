#include "stdafx.h"
#include "DnMPIncBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifndef _GAMESERVER
#include "DnInterface.h"
#endif

CDnMPIncBlow::CDnMPIncBlow(DnActorHandle hActor, const char *szValue)
	: CDnBlow(hActor)
	, m_fElapsedTime(0.0f)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_018;
	SetValue(szValue);
	//m_nValue = atoi( szValue );
	m_fValue = (float)atof(szValue);
}

CDnMPIncBlow::~CDnMPIncBlow(void)
{
}

void CDnMPIncBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	//m_fElapsedTime += fDelta;
	//if( m_fElapsedTime >= 2.0f )
	//{
	//	m_fElapsedTime = 0.0f;

	//	int nSP = m_hActor->GetSP();
	//	int nMaxSP = m_hActor->GetMaxSP();
	//	nSP += nMaxSP * int(m_fValue);
	//	if( nSP > nMaxSP ) nSP = nMaxSP;
	//	m_hActor->SetSP(nSP);
	//}
}

void CDnMPIncBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Just process everything at once
	int nSP = m_hActor->GetSP();
	int nMaxSP = m_hActor->GetMaxSP();
	int nIncSP = nSP;

	nSP += int((float)nMaxSP * m_fValue);

	if( nSP > nMaxSP ) 
		nSP = nMaxSP;

	if( nSP < 0 ) 
		nSP = 0;

	m_hActor->SetSP(nSP);
#ifndef _GAMESERVER
	nIncSP = nSP - nIncSP;
	if( nIncSP > 0 ) {
		GetInterface().SetRecovery( m_hActor->GetHeadPosition(), 0, nIncSP, false, (m_hActor == CDnActor::s_hLocalActor) );
	}
#endif

	OutputDebug( "CDnSPIncBlow::OnBegin\n" );
}

void CDnMPIncBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnSPIncBlow::OnEnd\n" );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnMPIncBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnMPIncBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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