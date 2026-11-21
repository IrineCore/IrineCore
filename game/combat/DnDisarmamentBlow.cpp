#include "stdafx.h"
#include "DnDisarmamentBlow.h"
#include "DnPlayerActor.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnDisarmamentBlow::CDnDisarmamentBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_237;
	SetValue( szValue );
	m_fValue = 0.0f;

}

CDnDisarmamentBlow::~CDnDisarmamentBlow(void)
{

}

void CDnDisarmamentBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);

#if defined(PRE_ADD_50907)
	if (m_hActor)
		m_hActor->AddDisarmamentRefCount();

	//Store the original weapon initially.
	if (m_hActor && m_hActor->GetDisarmamentRefCount() == 1)
	{
		DnWeaponHandle hOrigWeapon = m_hActor->GetWeapon(0);
		bool bOrigSelefDelete = m_hActor->GetWeaponSelfDelete(0);

		//Regardless of ChangeWeaponBlow state, just revert to the current weapon

// 		// If weapon was changed by ChangeWeapon, original weapon must be set to the one saved in ChangeWeapon.
// 		if (m_hActor->GetChangeWeaponRefCount() > 0)
// 		{
// 			hOrigWeapon = m_hActor->GetOrigWeaponWhenChangeWeapon();
// 			bOrigSelefDelete = m_hActor->GetOrigWeaponWhenChangeWeaponSelfDelete();
// 		}

		m_hActor->SetOrigWeaponWhenDisarmament(hOrigWeapon, bOrigSelefDelete);

		m_hActor->SetSkipOnAttatchDetachWeapon(true); //Must set this so DetachWeapon won't call MASkillUser::OnDetachWeapon (which may remove state effects internally).
		m_hActor->SetChangeWeaponLock(false);
		m_hActor->SetSkipChangeWeaponAction(true);
		m_hActor->DetachWeapon(0);
		m_hActor->SetSkipChangeWeaponAction(false);
		m_hActor->SetChangeWeaponLock(true);
		m_hActor->SetSkipOnAttatchDetachWeapon(false);

		if (m_hActor->IsPlayerActor())
		{
			CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
			if (pPlayerActor)
				pPlayerActor->SetBattleMode(false);
		}
	}
#endif // PRE_ADD_50907
}


void CDnDisarmamentBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);

#if defined(PRE_ADD_50907)
	if (m_hActor)
		m_hActor->RemoveDisarmamentRefCount();

	if (m_hActor && m_hActor->GetDisarmamentRefCount() == 0)
	{
		//If it does not run when re-equipping a weapon, basic attacks will not register.
		//So make it not trigger when unequipping a weapon, and trigger when equipping the original weapon.
		m_hActor->SetChangeWeaponLock(false);
		//m_hActor->SetSkipChangeWeaponAction(true);
		m_hActor->SetSkipOnAttatchDetachWeapon(true);	//Must set this so AttachWeapon/DetachWeapon won't call MASkillUser::OnAttachWeapon/OnDetachWeapon (which may add/remove state effects internally).

		DnWeaponHandle hOrigWeapon = m_hActor->GetOrigWeaponWhenDisarmament();
		bool bSelfDelete = m_hActor->GetOrigWeaponWhenDisarmamentSelfDelete();

		if (hOrigWeapon)
			m_hActor->AttachWeapon(hOrigWeapon, 0, bSelfDelete);
		else
			m_hActor->DetachWeapon(0);

		//m_hActor->SetSkipChangeWeaponAction(false);
		m_hActor->SetSkipOnAttatchDetachWeapon(false);

		if (m_hActor->IsPlayerActor())
		{
			CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
			if (pPlayerActor)
			{
				if (hOrigWeapon)
					pPlayerActor->SetBattleMode(true);
				else
					pPlayerActor->SetBattleMode(false);
			}
		}
	}
#endif // PRE_ADD_50907
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDisarmamentBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnDisarmamentBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
