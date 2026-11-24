#include "stdafx.h"
#include "DnSpectatorBlow.h"
#include "DnPlayerActor.h"


#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnSpectatorBlow::CDnSpectatorBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_pCantUseMPUsingSkillBlow = new CDnCantUseMPUsingSkillBlow(hActor, NULL);

	if (m_pCantUseMPUsingSkillBlow)
		m_pCantUseMPUsingSkillBlow->SetPermanent(true);

	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_230;
	SetValue( szValue );
	m_fValue = 0.0f;
}

CDnSpectatorBlow::~CDnSpectatorBlow(void)
{
	SAFE_DELETE( m_pCantUseMPUsingSkillBlow );
}

void CDnSpectatorBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

	if( !m_hActor || !m_hActor->IsPlayerActor() )
		return;

	if( m_pCantUseMPUsingSkillBlow ) m_pCantUseMPUsingSkillBlow->OnBegin( LocalTime, fDelta );
	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
	pPlayer->OnApplySpectator(true);
}

void CDnSpectatorBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	if( m_pCantUseMPUsingSkillBlow ) m_pCantUseMPUsingSkillBlow->Process( LocalTime, fDelta );
}


void CDnSpectatorBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

	if( !m_hActor || !m_hActor->IsPlayerActor() )
		return;

	if( m_pCantUseMPUsingSkillBlow ) m_pCantUseMPUsingSkillBlow->OnEnd( LocalTime, fDelta );

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
	pPlayer->OnApplySpectator(false);
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnSpectatorBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnSpectatorBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
