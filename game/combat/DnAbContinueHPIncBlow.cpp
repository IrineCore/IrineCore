#include "stdafx.h"
#include "DnAbContinueHPIncBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

// TODO: Probably should extract this into something like a config file later.
const LOCAL_TIME HP_INCREASE_INTERVAL = 2000;


CDnAbContinueHPIncBlow::CDnAbContinueHPIncBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																							  m_IntervalChecker( hActor, GetMySmartPtr() )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_011;
	SetValue( szValue );
	//m_nValue = atoi( szValue );

	std::vector<string> vlTokens;
	string strArgument( szValue );
	TokenizeA( strArgument, vlTokens, ";" );

	m_bShowValue = true;

#if defined(PRE_ADD_56253)
	m_fLimitMinValue = 0.0f;
#endif // PRE_ADD_56253

	int nTokenSize = (int)vlTokens.size();

	if( nTokenSize > 1 ) 
	{
		m_fValue = (float)atof( vlTokens[0].c_str() );
		m_bShowValue = ( strstr( vlTokens[1].c_str(), "true" ) || strstr( vlTokens[1].c_str(), "1" ) ) ? true : false;

#if defined(PRE_ADD_56253)
		if (nTokenSize > 2)
			m_fLimitMinValue = (float)atof(vlTokens[2].c_str());
#endif // PRE_ADD_56253
	}
	else 
	{
		m_fValue = (float)atof( szValue );
	}
}

void CDnAbContinueHPIncBlow::OnSetParentSkillInfo()
{

#if defined( _GAMESERVER )
	CalcHealValueLimit( m_StateBlow.emBlowIndex, m_fValue );
#endif

}

CDnAbContinueHPIncBlow::~CDnAbContinueHPIncBlow(void)
{

}

void CDnAbContinueHPIncBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_IntervalChecker.OnBegin( LocalTime, HP_INCREASE_INTERVAL );

	OutputDebug( "CDnAbContinueHPIncBlow::OnBegin, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}


bool CDnAbContinueHPIncBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	if( m_fValue != 0.0f )
	{
		// Send HP recovery packet. (May be negative)
		INT64 iMaxHP = m_hActor->GetMaxHP();
		INT64 iNowHP = m_hActor->GetHP();
		INT64 iDelta = INT64(m_fValue);
		INT64 iResultHP = iNowHP+iDelta;

		if( iResultHP > iMaxHP )
		{
			iDelta = iMaxHP - iNowHP;
			iResultHP = iMaxHP;
		}
		else
#if defined(PRE_ADD_56253)
		{
			bool isCheckLimitValue = false;
			//When starting HP is less than the limit value
			if (iNowHP < m_fLimitMinValue)
			{
				iDelta = 0;
				iResultHP = iNowHP;

				if (m_fLimitMinValue != 0.0f)
					isCheckLimitValue = true;
			}
			else
			if (iResultHP < m_fLimitMinValue)
			{
				iDelta = iDelta - iResultHP + (INT64)m_fLimitMinValue;
				iResultHP = (INT64)m_fLimitMinValue;

				if (m_fLimitMinValue != 0.0f)
					isCheckLimitValue = true;
			}

			//Terminate the skill if HP drops below the limit value.
			if (isCheckLimitValue == true && m_ParentSkillInfo.hSkillUser)
			{
				DnSkillHandle hSkill = m_ParentSkillInfo.hSkillUser->FindSkill(m_ParentSkillInfo.iSkillID);
				if (hSkill)
				{
					CDnSkill::DurationTypeEnum durationType = hSkill->GetDurationType();
					if (durationType == CDnSkill::DurationTypeEnum::Aura)
					{
						m_ParentSkillInfo.hSkillUser->OnSkillAura( hSkill, false );
						m_ParentSkillInfo.hSkillUser->CmdFinishAuraSkill(m_ParentSkillInfo.iSkillID);
					}
				}				
			}
		}
#else
		if( iResultHP < 0 )
		{
			iDelta = iDelta - iResultHP;
			iResultHP = 0;
		}
#endif // PRE_ADD_56253

		if( 0 != iDelta )
		{
			m_hActor->SetHP( iResultHP );
#ifdef PRE_FIX_77172
			if( m_ParentSkillInfo.iSkillID == 5209 )
				m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID(), false, m_bShowValue, NULL, true );
			else
				m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID(), false, m_bShowValue );
#else // PRE_FIX_77172
			m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID(), false, m_bShowValue );
#endif // PRE_FIX_77172
		}
	}
#endif

	return true;
}


void CDnAbContinueHPIncBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	m_IntervalChecker.Process( LocalTime, fDelta );

	//LOCAL_TIME ElapsedTime = LocalTime - m_StartTime;
	//int iNowHPIncCount = int(ElapsedTime / HP_INCREASE_INTERVAL);

	//while( m_iHPIncCount < iNowHPIncCount )
	//{
	//	int iMaxHP = m_hActor->GetMaxHP();
	//	int iNowHP = m_hActor->GetHP();

	//	int iHPResult = iNowHP+m_nValue;
	//	if( iHPResult < iNowHP )
	//	{
	//		//m_hActor->SetHP( iHPResult );

	//		// TODO: On client, handle as if packet was received.. since this is not damage, hold off for now!
	//	}

	//	m_LastIncreaseTime = LocalTime;

	//	++m_iHPIncCount;
	//}
}

void CDnAbContinueHPIncBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// Account for server start and end times.
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

	OutputDebug( "CDnAbContinueHPIncBlow::OnEnd, (HP : %d ) \n", m_hActor->GetHP());
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAbContinueHPIncBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnAbContinueHPIncBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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