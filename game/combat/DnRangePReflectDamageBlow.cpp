#include "stdafx.h"
#include "DnRangePReflectDamageBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _GAMESERVER
extern int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom );
#endif

CDnRangePReflectDamageBlow::CDnRangePReflectDamageBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_112;

	AddCallBackType( SB_ONDEFENSEATTACK );
	SetValue( szValue );

#ifdef _GAMESERVER
	m_fReflectRatio = (float)atof( szValue );
	m_fReflectRatioMax = 1.0f;
#endif // #ifdef _GAMESERVER
}

CDnRangePReflectDamageBlow::~CDnRangePReflectDamageBlow(void)
{

}

void CDnRangePReflectDamageBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	

	OutputDebug( "CDnRangePReflectDamageBlow::OnBegin\n" );
}


void CDnRangePReflectDamageBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnRangePReflectDamageBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	

	OutputDebug( "CDnRangePReflectDamageBlow::OnEnd\n" );
}

#ifdef _GAMESERVER
bool CDnRangePReflectDamageBlow::OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	if( hActor && pAttackerState &&
		CDnActor::PropActor != hActor->GetActorType() &&
		CDnDamageBase::DistanceTypeEnum::Range == HitParam.DistanceType &&
		0 == HitParam.cAttackType && !hActor->IsDie() )
	{
		// For ranged attacks, extract AttackerState holding the character's stats at skill cast time from the projectile and pass it as an argument to this function.
		//int iMAttackMin = hActor->GetAttackMMin();
		//int iMAttackMax = hActor->GetAttackMMax();
		int iPAttackMin = pAttackerState->GetAttackPMin();
		int iPAttackMax = pAttackerState->GetAttackPMax();

		int iReflectDamage = int(GetGaussianRandom( iPAttackMin, iPAttackMax, hActor->GetRoom() ) * m_fReflectRatio * HitParam.fDamage);
		
		// Related to #29810.. Exact cause unknown, so skip if the value is invalid.
		if( iReflectDamage < 0 )
			return false;
		//////////////////////////////////////////////////////////////////////////

		// Increase by light element attack power.
#ifdef PRE_ADD_49660
		if( DVINE_PUNISHMENT_SKILL_ID == m_ParentSkillInfo.iSkillID )
		{
			float fLightElementAttack = m_hActor->GetElementAttack( CDnState::Light );
			iReflectDamage += int((float)iReflectDamage * fLightElementAttack);
		}
#endif // #ifdef PRE_ADD_49660

#ifdef PRE_FIX_REFLECT_SE_LIMIT
		// So as not to exceed the maximum base attack power.
		CDnState* pState = m_hActor->GetStateStep( 0 );
		int iLimit = int((float)pState->GetAttackPMax() * m_fReflectRatioMax);
		if( iLimit < iReflectDamage )
			iReflectDamage = iLimit;
#endif // #ifdef PRE_FIX_REFLECT_SE_LIMIT
		
#if defined(PRE_FIX_61382)
		DnActorHandle hOwnerActor = CDnActor::GetOwnerActorHandle(hActor);
		if (hOwnerActor &&
			hOwnerActor->IsDie() == false)
		{
			hActor->RequestDamageFromStateBlow(GetMySmartPtr(), iReflectDamage);

			if (hOwnerActor->IsDie())
				hOwnerActor->Die(m_hActor);
		}
#else
		hActor->RequestDamageFromStateBlow( GetMySmartPtr(), iReflectDamage );

		// Check if dead
		if( hActor->GetHP() <= 0.f )
			hActor->Die( m_hActor );
#endif // PRE_FIX_61382

	}

	return false;
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnRangePReflectDamageBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnRangePReflectDamageBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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