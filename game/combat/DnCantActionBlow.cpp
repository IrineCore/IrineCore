#include "stdafx.h"
#include "DnCantActionBlow.h"
#include "DnChangeStandActionBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnCantActionBlow::CDnCantActionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ), 
																					m_strActionWhenCancelAttack( "Stand" )
																					,m_isAvailableNormalState(false)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_071;
}

CDnCantActionBlow::~CDnCantActionBlow(void)
{
}


void CDnCantActionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

	if( m_hActor->GetCantActionSEReferenceCount() == 0 )
	{
		// Prevent using attacks and skills. 
		// Make skills check state effects during UseSkill.
		// Also cancel the action currently in progress
		if( false == m_hActor->IsDie() )
		{
			bool availableAction = false;
			if (m_isAvailableNormalState)
				availableAction = true;
			else
				availableAction = m_hActor->IsMove() || m_hActor->IsAttack();

			if (availableAction)
				m_hActor->SetAction( m_strActionWhenCancelAttack.c_str(), 0.0f, 3.0f );

		}
		
		m_hActor->SetStateEffect( m_hActor->GetStateEffect() | CDnActorState::Cant_AttackAction );
	}

	m_hActor->AddedCantActionSE();

	if( m_hActor->IsPlayerActor() == true )
	{
#ifdef PRE_FIX_83091
#ifdef _GAMESERVER
		CDnChangeStandActionBlow::ReleaseStandChangeSkill( m_hActor , true );
#endif
#endif
	}

	OutputDebug( "CDnCantActionBlow::OnBegin Ref [%d --> %d]\n", m_hActor->GetCantActionSEReferenceCount()-1, m_hActor->GetCantActionSEReferenceCount() );
}


void CDnCantActionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_hActor->RemovedCantActionSE();
	
	if( m_hActor->GetCantActionSEReferenceCount() == 0 )
	{
		// Restore original state
		m_hActor->SetStateEffect( m_hActor->GetStateEffect() & ~CDnActorState::Cant_AttackAction );
	}
	
	OutputDebug( "CDnCantActionBlow::OnEnd Ref [%d --> %d]\n", m_hActor->GetCantActionSEReferenceCount()+1, m_hActor->GetCantActionSEReferenceCount() );

}

void CDnCantActionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);

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
void CDnCantActionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnCantActionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
