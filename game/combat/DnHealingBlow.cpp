#include "stdafx.h"
#include "DnHealingBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

// dnactor.cpp
#ifdef _GAMESERVER
extern int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom );
#endif

CDnHealingBlow::CDnHealingBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_140;

	// This state effect does not use any separate value.
	SetValue( szValue );
	m_fValue = (float)atof( szValue );
}

CDnHealingBlow::~CDnHealingBlow(void)
{

}

void CDnHealingBlow::OnSetParentSkillInfo()
{

#if defined( _GAMESERVER )
	CalcHealValueLimit( m_StateBlow.emBlowIndex, m_fValue );
#endif

}

void CDnHealingBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

#ifdef _GAMESERVER
	// Just process everything at once
	INT64 iHP = m_hActor->GetHP();
	INT64 iMaxHP = m_hActor->GetMaxHP();
	
	// Restores the target's HP by <Magic Attack> * <Effect Ratio> of the user (skill caster).
	if( m_ParentSkillInfo.hSkillUser )
	{
		int iAttackMMin = m_ParentSkillInfo.hSkillUser->GetAttackMMin();
		int iAttackMMax = m_ParentSkillInfo.hSkillUser->GetAttackMMax();
		
		float fMAttack = (float)GetGaussianRandom( iAttackMMin, iAttackMMax, m_hActor->GetRoom() );


		INT64 iDelta = INT64(fMAttack * m_fValue);
		
		if(iDelta < 0 )
			return;  // Healing Blow never reduces HP. Return early in case the value gets corrupted or garbage is passed in.

		INT64 iResult = iHP + iDelta;

		if( iMaxHP < iResult )
		{
			iResult = iMaxHP;
			iDelta = iMaxHP - iHP;
		}
		else if( iResult < 0 )
		{
			iResult = 1;
			iDelta = 1 - iHP;
		}

		m_hActor->SetHP( iResult );
		m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID() );

		OutputDebug( "[CDnHealingBlow::OnBegin] ActorID: %d : %d + %d = %d\n", m_hActor->GetUniqueID(), iHP, iDelta, iResult );
	}

	// Processed, so end immediately.
	SetState( STATE_BLOW::STATE_END );

#else
		OutputDebug( "CDnHealingBlow::OnBegin Value:%2.2f\n", m_fValue );
#endif
}


void CDnHealingBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnHealingBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

	OutputDebug( "CDnHealingBlow::OnEnd Value:%2.2f\n", m_fValue );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnHealingBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnHealingBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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