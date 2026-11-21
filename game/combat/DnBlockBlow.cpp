#include "stdafx.h"
#include "DnBlockBlow.h"
#if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBlockBlow::CDnBlockBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_030;
	SetValue( szValue );
	m_fValue = (float)atof( szValue );

	m_iCanBlockCount = (int)m_fValue;

	AddCallBackType( SB_ONDEFENSEATTACK );
}

CDnBlockBlow::~CDnBlockBlow(void)
{

}

void CDnBlockBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnBlockBlow::OnBegin, Value:%d \n", (int)m_fValue );
}

void CDnBlockBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	if( m_iCanBlockCount <= 0 )
	{
		// Send only state effect removal packet to client; server releases blow normally.
		m_hActor->CmdRemoveStateEffect( m_StateBlow.emBlowIndex, false );
		SetState( STATE_BLOW::STATE_END );
	}
#endif
}


void CDnBlockBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnBlockBlow::OnEnd\n");
}


#ifdef _GAMESERVER
bool CDnBlockBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	//  #25797 Do not trigger if bIgnoreParring is enabled on the hit signal.
	if( HitParam.bIgnoreParring )
		return false;

	// Does not trigger while in cannon mode. 
	// Block state effect is always used only by players.
	_ASSERT( m_hActor->IsPlayerActor() );
	if( m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( pPlayerActor->IsCannonMode() )
			return false;

		// Does not activate when not holding a weapon. #26772
		if( false == (pPlayerActor->IsBattleMode() && pPlayerActor->GetWeapon( 0 ) && pPlayerActor->GetWeapon( 1 )) )
			return false;
	}

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

	bool bResult = false;


#if defined(PRE_ADD_49166)
	// Prevent block from triggering if target has Freezing / FrameStop debuff.
	if (IsInVaildBlockCondition(m_hActor) == true)
		return false;
#endif // PRE_ADD_49166

	if( m_iCanBlockCount > 0 )
	{
		--m_iCanBlockCount;

		HitParam.szActionName.assign( "Skill_Block" );	
		OutputDebug( "CDnBlockBlow::OnDefenseAttack, CanBlockCount:%d \n", m_iCanBlockCount );

		// Notify those who applied it. /////////////////////////////////////////////
		boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_BUBBLE_BLOCK_SUCCESS ) );
		pEvent->SetSkillID( m_ParentSkillInfo.iSkillID );
		Notify( pEvent );
		//////////////////////////////////////////////////////////////////////////

		bResult = true;
	}

	return bResult;
}
#endif


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnBlockBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnBlockBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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