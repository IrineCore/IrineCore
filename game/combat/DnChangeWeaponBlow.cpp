#include "stdafx.h"
#include "DnChangeWeaponBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

int CDnChangeWeaponBlow::ms_InstanceCount = 0;
std::vector<DnWeaponHandle> CDnChangeWeaponBlow::ms_WeaponList;

CDnChangeWeaponBlow::CDnChangeWeaponBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_238;
	SetValue( szValue );
	m_fValue = 0.0f;

	m_nWeaponIndex = 0;
	m_nWeaponID = 0;

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);
	if (tokens.size() == 2)
	{
		m_nWeaponIndex = atoi(tokens[0].c_str());
		m_nWeaponID = atoi(tokens[1].c_str());

		CreateWeapon();
	}
	else
	{
		OutputDebug("%s Invalid Value[%s]\n", __FUNCTION__, szValue);
	}

	ms_InstanceCount++;
}

CDnChangeWeaponBlow::~CDnChangeWeaponBlow(void)
{
	ms_InstanceCount--;

// 	if (ms_InstanceCount == 0)
// 	{
// 		SAFE_RELEASE_SPTRVEC(ms_WeaponList);
// 	}
}

bool CDnChangeWeaponBlow::CanBegin( void )
{
	bool bResult = true;

	//Available only for monster actors.
	if (m_hActor->IsMonsterActor() == false)
		return false;

	//If the Disarmament state effect is applied, the weapon change state effect cannot be applied.
	if (m_hActor && m_hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_237))
		bResult = false;

	return bResult;
}

void CDnChangeWeaponBlow::CreateWeapon()
{
	if (!m_hNewWeapon)
	{
#if defined(_GAMESERVER)
		m_hNewWeapon = CDnWeapon::CreateWeapon(m_hActor->GetRoom(), m_nWeaponID, 0);
#else
		m_hNewWeapon = CDnWeapon::CreateWeapon(m_nWeaponID, 0);
#endif // _GAMESERVER

		//ms_WeaponList.push_back(m_hNewWeapon);
	}
}

void CDnChangeWeaponBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);

#if defined(PRE_ADD_50917)
	if (m_hActor)
		m_hActor->AddChangeWeaponRefCount();

	//Remove the feature that reverts to the original weapon.
// 	// Store the original weapon initially.
// 	if (m_hActor && m_hActor->GetChangeWeaponRefCount() == 1)
// 	{
// 		DnWeaponHandle hOrigWeapon = m_hActor->GetWeapon(m_nWeaponIndex);
// 		bool bSelefDelete = m_hActor->GetWeaponSelfDelete(m_nWeaponIndex);
// 
// 		m_hActor->SetOrigWeaponWhenChangeWeapon(hOrigWeapon, bSelefDelete);
// 	}

	m_hActor->AttachWeapon(m_hNewWeapon, m_nWeaponIndex, true);
#endif // PRE_ADD_50917
}


void CDnChangeWeaponBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
	
#if defined(PRE_ADD_50917)
	if (m_hActor)
		m_hActor->RemoveChangeWeaponRefCount();

	//Remove the feature that reverts to the original weapon.
// 	if (m_hActor && m_hActor->GetChangeWeaponRefCount() == 0)
// 	{
// 		DnWeaponHandle hOrigWeapon = m_hActor->GetOrigWeaponWhenChangeWeapon();
// 		bool bSelfDelete = m_hActor->GetOrigWeaponWhenChangeWeaponSelfDelete();
// 
// 		if (hOrigWeapon)
// 			m_hActor->AttachWeapon(hOrigWeapon, m_nWeaponIndex, bSelfDelete);
// 		else
// 			m_hActor->DetachWeapon(m_nWeaponIndex);
// 	}
#endif // PRE_ADD_50917
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChangeWeaponBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnChangeWeaponBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
