#include "stdafx.h"
#include "DnAvoidDebufActionBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAvoidDebufActionBlow::CDnAvoidDebufActionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_235;

	SetValue( szValue );
	m_fValue = (float)atof( szValue );
}

CDnAvoidDebufActionBlow::~CDnAvoidDebufActionBlow(void)
{
}


void CDnAvoidDebufActionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAvoidDebufActionBlow::OnBegin: %2.2f\n", m_fValue );

#if defined(PRE_FIX_51048)
	if (m_hActor)
	{
		m_hActor->RemoveDebufAction(LocalTime, fDelta);
	}
#endif // PRE_FIX_51048
}


void CDnAvoidDebufActionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnAvoidDebufActionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAvoidDebufActionBlow::OnEnd: %2.2f\n", m_fValue );
}