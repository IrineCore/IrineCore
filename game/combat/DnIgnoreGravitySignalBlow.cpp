#include "stdafx.h"
#include "DnIgnoreGravitySignalBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnIgnoreGravitySignalBlow::CDnIgnoreGravitySignalBlow( DnActorHandle hActor, const char *szValue )
: CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_273;
}

CDnIgnoreGravitySignalBlow::~CDnIgnoreGravitySignalBlow(void)
{
}

void CDnIgnoreGravitySignalBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
}

void CDnIgnoreGravitySignalBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin( LocalTime, fDelta );

	if( !m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_273 ) )
	{
		m_hActor->SetVelocityZ( 0.0f );
		m_hActor->SetResistanceZ( 0.0f );
#ifdef PRE_ADD_GRAVITY_PROPERTY
		m_hActor->SetVelocityY( 0.01f );
		m_hActor->SetResistanceY( -30.0f );
#endif // PRE_ADD_GRAVITY_PROPERTY
		m_hActor->ResetLook();
	}

	if( m_hActor->IsDie() )
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}
}

void CDnIgnoreGravitySignalBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd( LocalTime, fDelta );
}

bool CDnIgnoreGravitySignalBlow::CanBegin( void )
{
	bool bCanBegin = true;
	return bCanBegin;
}
