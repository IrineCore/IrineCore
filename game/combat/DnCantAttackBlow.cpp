#include "stdafx.h"
#include "DnCantAttackBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnCantAttackBlow::CDnCantAttackBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_130;

	// This state effect does not use any separate value.
	SetValue( szValue );

}

CDnCantAttackBlow::~CDnCantAttackBlow(void)
{

}

void CDnCantAttackBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hActor->GetCantActionSEReferenceCount() == 0 )
	{
		// Prevent attacking.
		// Make skills check state effects during UseSkill.
		// Also cancel the current action... otherwise a landing action or similar might retain an attack state..
		if( false == m_hActor->IsDie() )
		{
			if( m_hActor->IsMove() || m_hActor->IsAttack() )
				m_hActor->SetAction( "Stand", 0.0f, 3.0f );
		}

		m_hActor->SetStateEffect( m_hActor->GetStateEffect() | CDnActorState::Cant_AttackAction );
	}

	m_hActor->AddedCantAttackSE();

	OutputDebug( "CDnCantAttackBlow::OnBegin Ref [%d --> %d]\n", m_hActor->GetCantAttackSEReferenceCount()-1, m_hActor->GetCantAttackSEReferenceCount() );
}


void CDnCantAttackBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_hActor->RemovedCantAttackSE();

	if( m_hActor->GetCantAttackSEReferenceCount() == 0 )
	{
		// Restore original state
		m_hActor->SetStateEffect( m_hActor->GetStateEffect() & ~CDnActorState::Cant_AttackAction );
	}

	OutputDebug( "CDnCantAttackBlow::OnEnd Ref [%d --> %d]\n", m_hActor->GetCantAttackSEReferenceCount()+1, m_hActor->GetCantAttackSEReferenceCount() );
}

void CDnCantAttackBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process(LocalTime, fDelta);

	// [2010/11/26 semozz]
	// For StateBlow requiring server sync, receive packet from server to remove it
	// Do not transition to STATE_END on client.
	// [2010/12/13 semozz]
	// IsPermanent is not set, and
	if( !IsPermanent() &&
		m_StateBlow.fDurationTime <= 0.0f )
	{
#if defined(_GAMESERVER)
		//Server sends StateBlow removal packet
		m_hActor->SendRemoveStateEffectFromID(GetBlowID());
		m_StateBlow.fDurationTime = 0.0f;
		SetState( STATE_BLOW::STATE_END );

		// [2010/12/22 semozz]
		// Register in the delete list for removal in this frame.
		// If not registered here, it will be skipped on the next frame.
		m_hActor->RemoveStateBlowFromID(GetBlowID());
#else
		m_StateBlow.fDurationTime = 0.01f;
		SetState( STATE_BLOW::STATE_DURATION );
#endif	
	}
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnCantAttackBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnCantAttackBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW