#include "stdafx.h"
#include "DnSleepBlow.h"
#include "DnCantMoveBlow.h"
#include "DnCantActionBlow.h"
#include "DnMonsterActor.h"
#include "DnTableDB.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

const int DEFAULT_SLEEP_PROBABILITY = 10000;		// Default chance to fall asleep. Unit is percentage * 10
const LOCAL_TIME ACCEPT_HIT_GAP = 500;				// Prevent the Sleep state effect from breaking when hit within ~0.5 seconds after application; otherwise it breaks immediately.

#define SLEEP_ACTION_NAME "Sleep"


// Sleep action handling etc. is processed in the action tool.
CDnSleepBlow::CDnSleepBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_bAcceptHit = false;
																		  
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_045;
	SetValue( szValue );
	m_fValue = (float)atof(szValue);

	m_StartTime = 0;

	AddCallBackType( SB_ONDEFENSEATTACK );

	m_pCantMoveBlow =  new CDnCantMoveBlow( hActor, NULL );
	m_pCantActionBlow = new CDnCantActionBlow( hActor, NULL );
	
	if (m_pCantMoveBlow)
		m_pCantMoveBlow->SetPermanent(true);
	if (m_pCantActionBlow)
		m_pCantActionBlow->SetPermanent(true);

#ifndef _GAMESERVER
	m_pCantMoveBlow->UseTableDefinedGraphicEffect( false );
#endif
}

CDnSleepBlow::~CDnSleepBlow(void)
{
	SAFE_DELETE( m_pCantMoveBlow );
	SAFE_DELETE( m_pCantActionBlow );
}


#ifdef _GAMESERVER
bool CDnSleepBlow::CanBegin( void )
{
	float fWeight = 0.0f;
	if( m_hActor && m_hActor->IsPlayerActor() )
		fWeight = CPlayerWeightTable::GetInstance().GetValue( m_hActor->GetClassID(), CPlayerWeightTable::ElementDefense );

	float fAvoidProb = 1.f;
	fAvoidProb -= m_hActor->GetElementDefense( CDnState::Dark ) * fWeight;

	if( m_fValue > 0.f ) // #83276 Request to treat a value of 0 as 100%  
		fAvoidProb -= (1.f - m_fValue);
		
	if( _rand(GetRoom()) % 10000 > int(fAvoidProb * 10000.0f) )
		return true;

	return false;
}
#endif


void CDnSleepBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor )
		return;

	if( strcmp( m_hActor->GetCurrentAction(), SLEEP_ACTION_NAME ) != 0 )
	{
		m_hActor->SetActionQueue( SLEEP_ACTION_NAME, 0, 0.0f, 0.0f, false );
		CEtActionBase::ActionElementStruct* pElement = m_hActor->GetElement( SLEEP_ACTION_NAME );
		if( pElement )
		{
			if( m_hActor->CDnActionBase::GetFPS() > 0.0f )
				m_StateBlow.fDurationTime += (pElement->dwLength / m_hActor->CDnActionBase::GetFPS());
		}
		else
		{
			SetState( STATE_BLOW::STATE_END );
		}
	}
	m_hActor->SetDownRemainDelta( m_StateBlow.fDurationTime );
	

#ifndef _GAMESERVER
	_AttachGraphicEffect();
#endif

	m_pCantMoveBlow->OnBegin( LocalTime, fDelta );
	m_pCantActionBlow->OnBegin( LocalTime, fDelta );

	m_bAcceptHit = false;
	m_StartTime = LocalTime;
}


void CDnSleepBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	if( false == m_bAcceptHit )
	{
		if( LocalTime - m_StartTime > ACCEPT_HIT_GAP )
		{
			m_bAcceptHit = true;
		}
	}
}


void CDnSleepBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_pCantMoveBlow->OnEnd( LocalTime, fDelta );
	m_pCantActionBlow->OnEnd( LocalTime, fDelta );
	m_hActor->SetDownRemainDelta( 0.2f );

#ifndef _GAMESERVER
	_DetachGraphicEffect();
#else
	// On the game server, CDnActor::CmdRemoveStateEffect() does not call the subclass virtual function
	// Separated completely because a recursive call loop into state effect ending was discovered when called immediately.
	m_hActor->SendRemoveStateEffect( m_StateBlow.emBlowIndex );
#endif

}


#ifdef _GAMESERVER
bool CDnSleepBlow::OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	if( m_bAcceptHit )
	{
		// Wake up and end sleep when hit once!
		SetState( STATE_BLOW::STATE_END );
		// Send packet to client to end state effect.
		m_hActor->CmdRemoveStateEffect( m_StateBlow.emBlowIndex, false );
	}

	return false;
}
#endif


void CDnSleepBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	// Update downdelta to the newly incoming time.
	m_hActor->SetDownRemainDelta( m_StateBlow.fDurationTime );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnSleepBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];
	fValue[0] = (float)atof( szOrigValue );
	fValue[1] = (float)atof( szAddValue );

	float fResultValue = fValue[0] + fValue[1];
	sprintf_s(szBuff, "%f", fResultValue);
	szNewValue = szBuff;
}

void CDnSleepBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	float fValue[2];

	fValue[0] = (float)atof( szOrigValue );
	fValue[1] = (float)atof( szAddValue );

	float fResultValue = fValue[0] - fValue[1];
	sprintf_s(szBuff, "%f", fResultValue);
	szNewValue = szBuff;
}
#endif