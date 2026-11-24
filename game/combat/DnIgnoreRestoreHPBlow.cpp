#include "stdafx.h"
#include "DnIgnoreRestoreHPBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnIgnoreRestoreHPBlow::CDnIgnoreRestoreHPBlow( DnActorHandle hActor, const char *szValue )
: CDnBlow( hActor )
, m_nAllowSkillID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_275;
	SetValue( szValue );

	m_nAllowSkillID = atoi( szValue );
}

CDnIgnoreRestoreHPBlow::~CDnIgnoreRestoreHPBlow(void)
{
}

void CDnIgnoreRestoreHPBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnIgnoreRestoreHPBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin( LocalTime, fDelta );

	if( m_hActor->IsDie() )
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}
}

void CDnIgnoreRestoreHPBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd( LocalTime, fDelta );
}

bool CDnIgnoreRestoreHPBlow::CanBegin( void )
{
	bool bCanBegin = true;
	return bCanBegin;
}
