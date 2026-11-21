#include "stdafx.h"
#include "DnShieldBlow.h"
#if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnShieldBlow::CDnShieldBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_055;
	SetValue( szValue );
	m_fValue = (float)atof( szValue );

	m_iDurability = (int)m_fValue;

	m_bCalcDamage = false;

#if defined(_GAMESERVER)
	AddCallBackType( SB_AFTERONDEFENSEATTACK );
	AddCallBackType( SB_ONCALCDAMAGE );
#endif // _GAMESERVER
}

CDnShieldBlow::~CDnShieldBlow(void)
{

}

void CDnShieldBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnBlockBlow::OnBegin, Value:%d \n", (int)m_fValue );
}

void CDnShieldBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	if( m_iDurability <= 0 )
	{
		// Send only state effect removal packet to client; server releases blow normally.
		m_hActor->CmdRemoveStateEffect( m_StateBlow.emBlowIndex, false );
		SetState( STATE_BLOW::STATE_END );
	}
#endif
}


void CDnShieldBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnBlockBlow::OnEnd\n");
}


#ifdef _GAMESERVER
bool CDnShieldBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
/*	// #25797 Do not trigger if bIgnoreParring is enabled on the hit signal.
	if( HitParam.bIgnoreParring )
		return false;

#if defined(PRE_FIX_BLOCK_CONDITION)
	//Allow block to trigger if attacker is an enemy even when fDamage is 0..
	//For allies, do not block when fDamage is 0 as before.
	if (IsCanBlock(hHitter, m_hActor, HitParam) == false)
		return false;
#else
	// Changed so it does not trigger when hit percent is 0%. (#21175)
	if( 0.0f == HitParam.fDamage )
		return false;
#endif // PRE_FIX_BLOCK_CONDITION

#if defined(PRE_ADD_49166)
	// Prevent block from triggering if target has Freezing / FrameStop debuff.
	if (IsInVaildBlockCondition(m_hActor) == true)
		return false;
#endif // PRE_ADD_49166
*/
	bool bResult = false;

	//Damage modifier calculation should not happen here..
	m_bCalcDamage = false;
	int fDamage = (int)m_hActor->PreCalcDamage(hHitter, HitParam);

	//////////////////////////////////////////////////////////////////////////
	//Related to #37215..
	//If durability exceeds damage and defense succeeds here, reduce durability by damage
	//If durability is less than damage, defense fails here..
	//OnCalcDamage is called due to defense failure.
	//OnCalcDamage is not called if defense succeeds.
	//////////////////////////////////////////////////////////////////////////
	if( m_iDurability >= fDamage )
	{
		m_iDurability -= fDamage;

		//Prevent hit motion from playing.
		HitParam.szActionName.assign( "" );	

		bResult = true;
	}
	else
	{
		//If it enters here, it goes into OnCalcDamage
		//Need to adjust damage by durability.
		m_bCalcDamage = true;
	}
	
	return bResult;
}

float CDnShieldBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	//Since it is also calculated in OnDefenseAttack, apply damage correction only during actual damage calculation.
	if (m_bCalcDamage == false)
		return 0.0f;

	//Calculate damage correction
	//Adjust damage by remaining durability and set durability to 0.
	float fDelta = (float)m_iDurability;
	m_iDurability = 0;

	return -fDelta;
}
#endif


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnShieldBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnShieldBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
