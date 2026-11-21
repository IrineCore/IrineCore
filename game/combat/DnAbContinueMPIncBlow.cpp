#include "stdafx.h"
#include "DnAbContinueMPIncBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const LOCAL_TIME SP_INCREASE_INTERVAL = 2000;


CDnAbContinueMPIncBlow::CDnAbContinueMPIncBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																							  m_IntervalChecker( m_hActor, GetMySmartPtr() )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_013;
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

CDnAbContinueMPIncBlow::~CDnAbContinueMPIncBlow(void)
{

}

void CDnAbContinueMPIncBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_IntervalChecker.OnBegin( LocalTime, SP_INCREASE_INTERVAL );

	OutputDebug( "CDnAbContinueMPIncBlow::OnBegin, Value:%d (MP : %d ) \n", (int)m_fValue, m_hActor->GetSP());
}


bool CDnAbContinueMPIncBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	if( m_fValue != 0.0f )
	{
		// Send MP recovery packet. (May be negative)
		int iMaxMP = m_hActor->GetMaxSP();
		int iNowMP = m_hActor->GetSP();
		int iDelta = int(m_fValue);
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
	}
#endif

	// If using an aura skill, end the skill when MP runs out.
	if( m_hActor->GetEnabledAuraSkill() )
	{
		DnSkillHandle hAuraSkill = m_hActor->GetEnabledAuraSkill();
		if( m_hActor->GetSP() < hAuraSkill->GetDecreaseMP() )
		{
			m_hActor->ReserveFinishSkill( hAuraSkill );
			//m_hActor->OnSkillAura( hAuraSkill, false );
		}
	}

	return true;
}


void CDnAbContinueMPIncBlow::Process( LOCAL_TIME LocalTime, float fDelta )
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

void CDnAbContinueMPIncBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

	OutputDebug( "CDnAbContinueMPIncBlow::OnEnd, (MP : %d ) \n", m_hActor->GetSP());
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAbContinueMPIncBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnAbContinueMPIncBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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