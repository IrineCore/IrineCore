#include "stdafx.h"
#include "DnCantMoveBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnCantMoveBlow::CDnCantMoveBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )/*, m_iOriginalCalcPositionFlag( 0 )*/,
																				m_bCantMoveXZ( false )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_070;

	if( NULL != szValue )
	{
		SetValue( szValue );
		m_fValue = (float)atof( szValue );
	}

	if( 1.0f == m_fValue )
		m_bCantMoveXZ = true;
}

CDnCantMoveBlow::~CDnCantMoveBlow(void)
{

}

void CDnCantMoveBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bCantMoveXZ )
	{
		if( 0 == m_hActor->GetCantXZMoveSEReferenceCount() )
		{
			m_hActor->SetMovable( false );
			m_hActor->SetStateEffect( m_hActor->GetStateEffect() | CDnActorState::Cant_Move );

			//When an opponent character stops moving due to this state effect, non-local clients
			//Continuous movement info is set, so character movement may be processed in MAWalkMovement's ProcessCommand function.
			//So call CmdStop to reset Move info when this state blow is first applied.
			//#36970 When not moving
			EtVector3 *pMovePos = m_hActor->GetMovePos();
			if (pMovePos && EtVec3LengthSq( pMovePos ) > 0.0f)
				m_hActor->CmdStop("Stand");
		}

		m_hActor->AddedCantXZMoveSE();

		OutputDebug( "CDnCantMoveBlow::OnBegin XZ Ref: [%d --> %d]\n", m_hActor->GetCantXZMoveSEReferenceCount()-1, m_hActor->GetCantXZMoveSEReferenceCount() );
	}
	else
	{
		if( 0 == m_hActor->GetCantMoveSEReferenceCount() )
		{
			m_hActor->SetMovable( false );
			m_hActor->SetStateEffect( m_hActor->GetStateEffect() | CDnActorState::Cant_Move );

			//When an opponent character stops moving due to this state effect, non-local clients
			//Continuous movement info is set, so character movement may be processed in MAWalkMovement's ProcessCommand function.
			//So call CmdStop to reset Move info when this state blow is first applied.
			//#36970 When not moving
			EtVector3 *pMovePos = m_hActor->GetMovePos();
			if (pMovePos && EtVec3LengthSq( pMovePos ) > 0.0f)
				m_hActor->CmdStop("Stand");
		}

		m_hActor->AddedCantMoveSE();

		OutputDebug( "CDnCantMoveBlow::OnBegin Ref: [%d --> %d]\n", m_hActor->GetCantMoveSEReferenceCount()-1, m_hActor->GetCantMoveSEReferenceCount() );
	}
}


void CDnCantMoveBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
 	if( m_bCantMoveXZ )
	{
		m_hActor->RemovedCantXZMoveSE();

		if( 0 == m_hActor->GetCantXZMoveSEReferenceCount() )
		{
			// Leave as is here since the CanMove signal keeps updating it
			//m_hActor->SetMovable( true );
			if( m_hActor->GetCantMoveSEReferenceCount() == 0 ) // Must check both.
				m_hActor->SetStateEffect( m_hActor->GetStateEffect() & ~CDnActorState::Cant_Move );
		}

		OutputDebug( "CDnCantMoveBlow::OnEnd XZ Ref: [%d --> %d]\n", m_hActor->GetCantMoveSEReferenceCount()+1, m_hActor->GetCantMoveSEReferenceCount() );
	}
	else
	{
		m_hActor->RemovedCantMoveSE();

		if( 0 == m_hActor->GetCantMoveSEReferenceCount() )
		{
			// Leave as is here since the CanMove signal keeps updating it
			//m_hActor->SetMovable( true );
			if( m_hActor->GetCantXZMoveSEReferenceCount() == 0 ) // Must check both.
				m_hActor->SetStateEffect( m_hActor->GetStateEffect() & ~CDnActorState::Cant_Move );
		}

		OutputDebug( "CDnCantMoveBlow::OnEnd Ref: [%d --> %d]\n", m_hActor->GetCantMoveSEReferenceCount()+1, m_hActor->GetCantMoveSEReferenceCount() );
	}
}

void CDnCantMoveBlow::Process( LOCAL_TIME LocalTime, float fDelta )
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
		//Server only sends the packet. Deletion happens in StateBlow's Process
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
void CDnCantMoveBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnCantMoveBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW