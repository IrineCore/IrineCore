#include "stdafx.h"
#include "DnDamageTransitionBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnDamageTransitionBlow::CDnDamageTransitionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_207;
	SetValue( szValue );

	m_fValue = 0.0f;

	m_fRate = (float)atof(szValue);

#if defined(_GAMESERVER)
	AddCallBackType(SB_ONCALCDAMAGE);
#endif // _GAMESERVER

	m_nDamage = 0;
}

CDnDamageTransitionBlow::~CDnDamageTransitionBlow(void)
{

}

void CDnDamageTransitionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

}

void CDnDamageTransitionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
	
}


void CDnDamageTransitionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

}

#if defined(_GAMESERVER)

float CDnDamageTransitionBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	m_nDamage = (int)(fOriginalDamage * m_fRate);	//Calculate damage by ratio and store it.

	DamageTransition(m_nDamage);

	return (float)-m_nDamage;	//Return negated value calculated above to reduce damage (reduces incoming damage)
}

void CDnDamageTransitionBlow::DamageTransition(int nDamage)
{
	if (nDamage != 0 && 
		m_ParentSkillInfo.hSkillUser && 
		!m_ParentSkillInfo.hSkillUser->IsDie())
	{
		char buffer[65];
		_itoa_s(nDamage, buffer, 65, 10 );
		m_ParentSkillInfo.hSkillUser->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer, false, false);
	}
}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDamageTransitionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnDamageTransitionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
