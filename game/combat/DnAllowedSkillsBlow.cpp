#include "stdafx.h"
#include "DnAllowedSkillsBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnAllowedSkillsBlow::CDnAllowedSkillsBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_176;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
	//m_fValue = (float)atof(szValue);
	AddAllowSkills(szValue);
	m_fValue = 0.0f;
}

CDnAllowedSkillsBlow::~CDnAllowedSkillsBlow(void)
{
}

void CDnAllowedSkillsBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}


void CDnAllowedSkillsBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

void CDnAllowedSkillsBlow::AddAllowSkills(const char* szValue)
{
	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);

	std::vector<std::string>::iterator iter = tokens.begin();
	for ( ; iter != tokens.end(); ++iter)
	{
		AddAllowSkill(atoi(iter->c_str()));
	}
}

void CDnAllowedSkillsBlow::AddAllowSkill(int nSkillID)
{
	m_AllowSkillList.insert(ALLOW_SKILL_LIST::value_type(nSkillID, nSkillID));
}

bool CDnAllowedSkillsBlow::IsAllowSkill(int nSkillID)
{
	//Find the skill ID in the list.
	ALLOW_SKILL_LIST::iterator iter = m_AllowSkillList.find(nSkillID);
	
	//true if in the list, false otherwise
	return (iter != m_AllowSkillList.end());
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAllowedSkillsBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
	//Append ";" if missing at the end of szOrigValue, then append szAddValue.
	int nLength = (int)strlen(szOrigValue);

	if (nLength == 0 || szOrigValue[nLength - 1] == ';')
		szNewValue += szAddValue;
	else
	{
		szNewValue += ";";
		szNewValue += szAddValue;
	}
}

void CDnAllowedSkillsBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//"###;###;###;###;....;##;##;##"
	//|<--Previous string->|<-AddValue->|

	//Trim szOrigValue from the end by the length of szAddValue.
	int nOrigLength = (int)strlen(szOrigValue);
	int nAddLength = (int)strlen(szAddValue);
	int nCount = nOrigLength - nAddLength;

	sprintf_s(szBuff, "%s", szOrigValue);
	szBuff[nCount] = 0;

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
