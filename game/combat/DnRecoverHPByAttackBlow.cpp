#include "stdafx.h"
#include "DnRecoverHPByAttackBlow.h"

#if defined(_GAMESERVER)
#include "DnMonsterActor.h"
#endif // _GAMESERVER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnRecoverHPByAttackBlow::CDnRecoverHPByAttackBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_248;

	SetValue( szValue );
	
	m_fValue = 0.0f;
	m_nLimitHP = -1;

	std::string str = szValue;//"HealAmount;HealAmountLimit";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Category
	TokenizeA(str, tokens, delimiters);
	int nTokenSize = (int)tokens.size();
	
	if (nTokenSize >= 1)
	{
		m_fValue = (float)(atof(tokens[0].c_str()));
		if (nTokenSize >= 2)
			m_nLimitHP = atoi(tokens[1].c_str());
	}
	else
	{
		OutputDebug("%s Invalid Value %s\n", __FUNCTION__, szValue);
	}

#if defined(_GAMESERVER)
	AddCallBackType(SB_ONTARGETHIT);
#endif // _GAMESERVER

}

CDnRecoverHPByAttackBlow::~CDnRecoverHPByAttackBlow(void)
{

}

void CDnRecoverHPByAttackBlow::OnSetParentSkillInfo()
{

#if defined( _GAMESERVER )
	CalcHealValueLimit( m_StateBlow.emBlowIndex, m_fValue );
#endif

}

void CDnRecoverHPByAttackBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

	OutputDebug( "%s\n", __FUNCTION__ );
}


void CDnRecoverHPByAttackBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );

}


void CDnRecoverHPByAttackBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

#if defined(_GAMESERVER)
	//Remove state effects caused by the same skill...
	//#60311 Block removing same-skill state effects when removing state effects.
	//RemoveStateBlowBySkillID(m_ParentSkillInfo.iSkillID);
#endif // _GAMESERVER
	OutputDebug( "%s\n", __FUNCTION__);
}

#if defined(_GAMESERVER)
void CDnRecoverHPByAttackBlow::RemoveStateBlowBySkillID(int nSkillID)
{
	DNVector(DnBlowHandle) vlBlowList;

	FindSkillBlowList(m_hActor, nSkillID, vlBlowList);

	int nBlowCount = (int)vlBlowList.size();
	for (int n = 0; n < nBlowCount; ++n)
	{
		DnBlowHandle hBlow = vlBlowList[n];
		if (!hBlow)
			continue;

		// Skip checking skill user actor (state effects added by props have no skill user actor).
		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
			continue;

		//Queue state effect to apply to self.
		m_hActor->CmdRemoveStateEffectFromID(hBlow->GetBlowID());
	}

}

void CDnRecoverHPByAttackBlow::FindSkillBlowList(DnActorHandle hActor, int nSkillID, DNVector(DnBlowHandle)& vlBlowList)
{
	if (!hActor)
		return;

	int nStateBlowCount = hActor->GetNumAppliedStateBlow();

	for (int i = 0; i < nStateBlowCount; ++i)
	{
		DnBlowHandle hBlow = hActor->GetAppliedStateBlow(i);
		if (!hBlow)
			continue;

		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		//Skip if no skill info (there may be no skill user, e.g., state effect applied by a prop).
		if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
			continue;

		if (pSkillInfo->iSkillID == nSkillID)
			vlBlowList.push_back(hBlow);
	}
}


void CDnRecoverHPByAttackBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	CDnDamageBase::SHitParam *pHitParam = NULL;
#if defined(PRE_FIX_59680)
	if (hTargetActor)
	{
		//If it is a summoned monster actor, has a master actor, and has the TransmitDamageBlow(51) state effect, fetch the master actor's HitParam.
		//RequestDamage
		if (hTargetActor->IsMonsterActor())
		{
			CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>(hTargetActor.GetPointer());
			if (pMonsterActor && pMonsterActor->IsSummonedMonster() && pMonsterActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_051))
			{
				hTargetActor = pMonsterActor->GetSummonerPlayerActor();
			}
		}
		
		if (hTargetActor)
			pHitParam = hTargetActor->GetHitParam();
	}
#else
	if (hTargetActor)
		pHitParam = hTargetActor->GetHitParam();
#endif // PRE_FIX_59680

	INT64 nRecoverHP = 0;
	if (pHitParam)
		nRecoverHP = (INT64)(m_fValue * pHitParam->nCalcDamage);
	
	if (m_nLimitHP != -1)
	{
		//Restore HP only when the limit value is valid.
		if (m_nLimitHP <= 0)
		{
			OutputDebug("%s HP recovery limit...  [%d] - [%d]\n", __FUNCTION__, m_nLimitHP, nRecoverHP);
			return;
		}

		//If current heal amount is less than remaining HP heal cap, reduce remaining cap by current heal amount
		if (m_nLimitHP >= nRecoverHP)
			m_nLimitHP -= nRecoverHP;
		//If remaining HP heal cap is less than current heal amount, set remaining cap to 0 and heal amount to remaining cap.
		else
		{
			nRecoverHP = m_nLimitHP;
			m_nLimitHP = 0;

			//#60311 After HP recovers up to the limit, HP stops recovering while the state effect remains active.
			//If no heal amount remains, remove state effect.. When removed, also remove state effects of the same skill..
			//SetState(STATE_BLOW::STATE_END);
		}
	}

	if (nRecoverHP != 0)
	{
		//Do not exceed MaxHP..
		INT64 nMaxHP = m_hActor->GetMaxHP();
		INT64 nCurHP = m_hActor->GetHP();
		INT64 nTempHP = nCurHP + nRecoverHP;

		//Adjust nRecoverHP by the amount exceeding MaxHP.
		if (nTempHP > nMaxHP)
			nRecoverHP -= (nTempHP - nMaxHP);

		if (nRecoverHP > 0)
		{
			m_hActor->SetHP( m_hActor->GetHP() + nRecoverHP );
			m_hActor->RequestHPMPDelta( CDnState::ElementEnum::ElementEnum_Amount, nRecoverHP, UINT_MAX, false, true );
		}
	}
}
#endif // _GAMESERVER


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnRecoverHPByAttackBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnRecoverHPByAttackBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW