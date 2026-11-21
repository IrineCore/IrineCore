#include "stdafx.h"
#include "DnDamageBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnDamageBlow::CDnDamageBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_177;
	SetValue( szValue );
	
#ifdef _GAMESERVER
	
	m_nAddDamage = atoi(szValue);
#endif
}

CDnDamageBlow::~CDnDamageBlow(void)
{

}

void CDnDamageBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#if defined(_GAMESERVER)
#if defined(PRE_FIX_61382)
	DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
	if (0 != m_nAddDamage && 
		hActor &&
		hActor->IsDie() == false)
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), m_nAddDamage);

		// Check if dead
		if( hActor->IsDie() )
			hActor->Die( m_ParentSkillInfo.hSkillUser	);
	}

#else
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
	if (0 != m_nAddDamage && !m_hActor->IsDie())
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), m_nAddDamage);

		// Check if dead
		if( m_hActor->GetHP() <= 0.f )
			m_hActor->Die( m_ParentSkillInfo.hSkillUser	);
	}
#endif // PRE_FIX_61382

#endif // _GAMESERVER
}

void CDnDamageBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
	//OutputDebug( "CDnFireBurnBlow::Process, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}


void CDnDamageBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDamageBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	int iValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	iValue[0] = atoi( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	iValue[1] = atoi( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultValue = iValue[0] + iValue[1];

	sprintf_s(szBuff, "%d", iResultValue);

	szNewValue = szBuff;
}

void CDnDamageBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	int iValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	iValue[0] = atoi( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	iValue[1] = atoi( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultValue = iValue[0] - iValue[1];

	sprintf_s(szBuff, "%d", iResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
