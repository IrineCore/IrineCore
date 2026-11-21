#include "stdafx.h"
#include "DnInvincibleAtBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDNInvincibleAtBlow::CDNInvincibleAtBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_bSendResist = false;
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_154;
	SetValue( szValue );
	AddInvincibleAt(szValue);
	m_fValue = 0.0f;
}

CDNInvincibleAtBlow::~CDNInvincibleAtBlow(void)
{
}

void CDNInvincibleAtBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}


void CDNInvincibleAtBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

void CDNInvincibleAtBlow::AddInvincibleAt(const char* szValue)
{

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);

	std::vector<std::string>::iterator iter = tokens.begin();
	for ( ; iter != tokens.end(); ++iter)
	{
		if( strcmp( iter->c_str() ,"Resist" ) == NULL )
		{
			m_bSendResist = true;
		}
		else
		{
			AddInvincibleAt(atoi(iter->c_str()));
		}
	}
}

bool CDNInvincibleAtBlow::IsEnableSendResist()
{

#ifdef PRE_FIX_INVINCIBLE_AT_BLOW_SEND_RESIST 
	return m_bSendResist;
#else
	return false;
#endif

}
void CDNInvincibleAtBlow::AddInvincibleAt(int nSkillID)
{
	m_InvincibleAtList.insert(INVINCIBLEAT_LIST::value_type(nSkillID, nSkillID));
}

bool CDNInvincibleAtBlow::IsInvincibleAt(int nSkillID)
{
	//Find the skill ID in the list.
	INVINCIBLEAT_LIST::iterator iter = m_InvincibleAtList.find(nSkillID);
	
	//true if in the list, false otherwise
	return (iter != m_InvincibleAtList.end());
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDNInvincibleAtBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDNInvincibleAtBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
