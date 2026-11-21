#include "stdafx.h"
#include "DnApplyStateBlowOnTimeBlow.h"
#include "DnStateBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnApplyStateBlowOnTimeBlow::CDnApplyStateBlowOnTimeBlow(DnActorHandle hActor, const char *szValue)
	: CDnBlow(hActor)
#if defined(_GAMESERVER)
	,m_IntervalChecker( hActor, GetMySmartPtr() )
#endif // _GAMESERVER
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_229;
	SetValue(szValue);
	
	m_fValue = 0.0f;

	m_nTime = 0;
	m_fRate = 0.0f;
	m_nSkillID = 0;
	m_nSkillLevel = 0;

	SetInfo(szValue);
}

CDnApplyStateBlowOnTimeBlow::~CDnApplyStateBlowOnTimeBlow(void)
{
}

void CDnApplyStateBlowOnTimeBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );

#if defined(_GAMESERVER)
	m_IntervalChecker.Process( LocalTime, fDelta );
#endif // _GAMESERVER
}

void CDnApplyStateBlowOnTimeBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	m_hSkill = CDnSkill::CreateSkill(m_hActor, m_nSkillID, 1);

	m_IntervalChecker.OnBegin( LocalTime, m_nTime);
#endif // _GAMESERVER
}

void CDnApplyStateBlowOnTimeBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

#if defined(_GAMESERVER)
	if (m_hSkill)
		m_hSkill->OnEnd(LocalTime, fDelta);

	SAFE_RELEASE_SPTR( m_hSkill );
#endif // _GAMESERVER
	OutputDebug( "%s\n", __FUNCTION__ );
}

void CDnApplyStateBlowOnTimeBlow::SetInfo(const char* szValue)
{
	//1. Basic info
	std::string defaultInfo = szValue;//"@sec;probability;skillID;skillLevel";
	std::vector<std::string> infoTokens;
	std::string delimiters = ";";

	TokenizeA(defaultInfo, infoTokens, delimiters);
	if (4 == infoTokens.size())
	{
		m_nTime = atoi(infoTokens[0].c_str());
		m_fRate = (float)atof(infoTokens[1].c_str());
		m_nSkillID = atoi(infoTokens[2].c_str());
		m_nSkillLevel = atoi(infoTokens[3].c_str());		
	}
	else
	{
		//If state effect value setup is invalid, just end the state effect...
		SetState( STATE_BLOW::STATE_END );
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);
	}
}

#if defined(_GAMESERVER)
bool CDnApplyStateBlowOnTimeBlow::OnCustomIntervalProcess( void )
{
	if (!m_hSkill)
		return true;

	bool bExecuteable = rand() % 10000 <= (m_fRate * 10000.0f);
	if (bExecuteable)
		m_hActor->AddSkillStateEffect(m_hSkill);

	return true;
}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnApplyStateBlowOnTimeBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	szNewValue = szOrigValue;
}

void CDnApplyStateBlowOnTimeBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW