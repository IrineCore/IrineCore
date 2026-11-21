#include "stdafx.h"
#include "DnContinueBaseMPIncBlow.h"
#include "DnPlayerActor.h"

#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const LOCAL_TIME SP_INCREASE_INTERVAL = 2000;


CDnContinueBaseMPIncBlow::CDnContinueBaseMPIncBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
m_IntervalChecker( m_hActor, GetMySmartPtr() )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_231;
	SetValue( szValue );

	std::vector<string> vlTokens;
	string strArgument( szValue );
	TokenizeA( strArgument, vlTokens, ";" );

	m_bShowValue = false;
	if( vlTokens.size() > 1 ) {
		m_fValue = (float)atof( vlTokens[0].c_str() );
		m_bShowValue = ( strstr( vlTokens[1].c_str(), "true" ) || strstr( vlTokens[1].c_str(), "1" ) ) ? true : false;
	}
	else m_fValue = (float)atof( szValue );

}

CDnContinueBaseMPIncBlow::~CDnContinueBaseMPIncBlow(void)
{

}

void CDnContinueBaseMPIncBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_IntervalChecker.OnBegin( LocalTime, SP_INCREASE_INTERVAL );

	OutputDebug( "CDnContinueBaseMPIncBlow::OnBegin, Value:%d (MP : %d ) \n", (int)m_fValue, m_hActor->GetSP());
}

#if defined(PRE_FIX_46381)
float CDnContinueBaseMPIncBlow::GetMPIncValue()
{
	float fRatio = CPlayerLevelTable::GetInstance().GetValueFloat( ((CDnPlayerActor*)m_hActor.GetPointer())->GetJobClassID(), 
		m_hActor->GetLevel(), 
		CPlayerLevelTable::SPDecreaseRatio ) * m_hActor->GetLevelWeightValue();

	float deltaValue = m_fValue * fRatio;

	return deltaValue;
}
#endif // PRE_FIX_46381

bool CDnContinueBaseMPIncBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	// Send MP recovery packet. (May be negative)
	int iMaxMP = m_hActor->GetMaxSP();
	int iNowMP = m_hActor->GetSP();
	float fRatio = CPlayerLevelTable::GetInstance().GetValueFloat( ((CDnPlayerActor*)m_hActor.GetPointer())->GetJobClassID(), 
		m_hActor->GetLevel(), 
		CPlayerLevelTable::SPDecreaseRatio ) * m_hActor->GetLevelWeightValue();

	int iDelta = int(m_fValue * fRatio);
	int iResultMP = iNowMP+iDelta;

	if( iResultMP > iMaxMP )
	{
		iDelta = iMaxMP - iNowMP;
		iResultMP = iMaxMP;
	}
	else
	if( iResultMP < 0 )
	{
		iDelta = iDelta - iResultMP;
		iResultMP = 0;
	}

	if( 0 != iDelta )
	{
		m_hActor->SetSP( iResultMP );
		m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID(), true, m_bShowValue );
	}
#endif
	return true;
}


void CDnContinueBaseMPIncBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	m_IntervalChecker.Process( LocalTime, fDelta );

	//LOCAL_TIME ElapsedTime = LocalTime - m_StartTime;
	//int iNowSPInc = int(ElapsedTime / SP_INCREASE_INTERVAL);

	//while( m_iSPIncCount < iNowSPInc )
	//{
	//	int iMaxSP = m_hActor->GetMaxSP();
	//	int iNowSP = m_hActor->GetSP();

	//	int iSPResult = iNowSP+m_nValue;
	//	if( iSPResult < iNowSP )
	//	{
	//		//m_hActor->SetSP( iSPResult );
	//	}

	//	m_LastIncreaseTime = LocalTime;

	//	++m_iSPIncCount;
	//}

}

void CDnContinueBaseMPIncBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

	OutputDebug( "CDnContinueBaseMPIncBlow::OnEnd, (MP : %d ) \n", m_hActor->GetSP());
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnContinueBaseMPIncBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fValue[2];
	bool bShowValue[2];


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	bShowValue[0] = true;
	if( vlTokens[0].size() > 1 ) {
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		bShowValue[0] = ( strstr( vlTokens[0][1].c_str(), "true" ) || strstr( vlTokens[0][1].c_str(), "1" ) ) ? true : false;

		nValueCount = 2;
	}
	else 
	{
		fValue[0] = (float)atof( szOrigValue );

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	bShowValue[1] = true;
	if( vlTokens[1].size() > 1 ) {
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		bShowValue[1] = ( strstr( vlTokens[1][1].c_str(), "true" ) || strstr( vlTokens[1][1].c_str(), "1" ) ) ? true : false;
	}
	else 
	{
		fValue[1] = (float)atof( szOrigValue );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];
	bool fResultShow = (bShowValue[0] || bShowValue[1]);

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%s", fResultValue, fResultShow ? "true" : "false");
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultValue);
	}

	szNewValue = szBuff;
}

void CDnContinueBaseMPIncBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	int nValueCount = 0;

	//Required value variable
	float fValue[2];
	bool bShowValue[2];


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	bShowValue[0] = true;
	if( vlTokens[0].size() > 1 ) {
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		bShowValue[0] = ( strstr( vlTokens[0][1].c_str(), "true" ) || strstr( vlTokens[0][1].c_str(), "1" ) ) ? true : false;

		nValueCount = 2;
	}
	else 
	{
		fValue[0] = (float)atof( szOrigValue );

		nValueCount = 1;
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	bShowValue[1] = true;
	if( vlTokens[1].size() > 1 ) {
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		bShowValue[1] = ( strstr( vlTokens[1][1].c_str(), "true" ) || strstr( vlTokens[1][1].c_str(), "1" ) ) ? true : false;
	}
	else 
	{
		fValue[1] = (float)atof( szOrigValue );
	}
	//////////////////////////////////////////////////////////////////////////

	//Calculate the two values.
	float fResultValue = fValue[0] - fValue[1];
	bool fResultShow = (bShowValue[0] || bShowValue[1]);

	if (nValueCount == 2)
	{
		sprintf_s(szBuff, "%f;%s", fResultValue, fResultShow ? "true" : "false");
	}
	else
	{
		sprintf_s(szBuff, "%f", fResultValue);
	}

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW