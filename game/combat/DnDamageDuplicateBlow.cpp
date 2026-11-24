#include "stdafx.h"
#include "DnDamageDuplicateBlow.h"
#include "DnSkill.h"
#ifdef _GAMESERVER
#include "DnHighLanderBlow.h"
#endif 

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnDamageDuplicateBlow::CDnDamageDuplicateBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_158;
	SetValue( szValue );

#ifdef _GAMESERVER
	AddCallBackType( SB_ONDEFENSEATTACK );
	m_DuplicateDamage = 0.0f;//
#endif
}

CDnDamageDuplicateBlow::~CDnDamageDuplicateBlow(void)
{

}

void CDnDamageDuplicateBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	

#endif
}

void CDnDamageDuplicateBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
#if defined(_GAMESERVER)
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
#if defined(PRE_FIX_61382)
	DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
	if (m_DuplicateDamage != 0 && hActor && hActor->IsDie() == false )
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), static_cast<int>(m_DuplicateDamage) );

		// Check if dead
		if( hActor->IsDie() )
			hActor->Die( m_ParentSkillInfo.hSkillUser	);

		OutputDebug("%s Damage :: %d\n", __FUNCTION__, m_DuplicateDamage);

		m_DuplicateDamage = 0;
	}
#else
	if (m_DuplicateDamage != 0 && !m_hActor->IsDie())
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), static_cast<int>(m_DuplicateDamage) );
		
		// Check if dead
		if( m_hActor->GetHP() <= 0.f )
			m_hActor->Die( m_ParentSkillInfo.hSkillUser	);

		OutputDebug("%s Damage :: %d\n", __FUNCTION__, m_DuplicateDamage);

		m_DuplicateDamage = 0;
	}
#endif // PRE_FIX_61382
#endif // _GAMESERVER
}


void CDnDamageDuplicateBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}


#if defined(_GAMESERVER)
bool CDnDamageDuplicateBlow::CalcDuplicateValue( const char* szValue )
{
	return true;
}

bool CDnDamageDuplicateBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	m_DuplicateDamage = m_hActor->PreCalcDamage(hHitter, HitParam);

	return false;
}

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDamageDuplicateBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnDamageDuplicateBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
