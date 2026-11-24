#include "stdafx.h"
#include "DnSummonMonsterChangeSkillLevelBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnSummonMonsterChangeSkillLevelBlow::CDnSummonMonsterChangeSkillLevelBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_167;
	SetValue( szValue );

	m_fValue = (float)atof(szValue);

	m_fRate = m_fValue;
}

CDnSummonMonsterChangeSkillLevelBlow::~CDnSummonMonsterChangeSkillLevelBlow(void)
{

}

void CDnSummonMonsterChangeSkillLevelBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}


void CDnSummonMonsterChangeSkillLevelBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnSummonMonsterChangeSkillLevelBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
	float fResultValue = max(fValue[0], fValue[1]);

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnSummonMonsterChangeSkillLevelBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
	float fResultValue = min(fValue[0], fValue[1]);

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW