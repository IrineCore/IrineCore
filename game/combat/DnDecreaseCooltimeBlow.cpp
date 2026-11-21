#include "stdafx.h"
#include "DnDecreaseCooltimeBlow.h"
#include "DnSkill.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnDecreaseCooltimeBlow::CDnDecreaseCooltimeBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_291;
	SetValue( szValue );
	m_fValue = szValue ? (float)atof( szValue ) : 0.0f;
}

CDnDecreaseCooltimeBlow::~CDnDecreaseCooltimeBlow( void )
{
}

void CDnDecreaseCooltimeBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor )
		return;

	DWORD dwCount = m_hActor->GetSkillCount();
	for( DWORD i = 0; i < dwCount; ++i )
	{
		DnSkillHandle hSkill = m_hActor->GetSkillFromIndex( i );
		if( !hSkill )
			continue;

		int nSkillID = hSkill->GetClassID();
		if( nSkillID == 2415 || nSkillID == 1412 )
			continue;

		float fCoolTime = hSkill->GetCoolTime();
		if( fCoolTime > 0.0f )
		{
			fCoolTime -= m_fValue;
			if( fCoolTime < 0.0f )
				fCoolTime = 0.0f;
			hSkill->SetForceCoolTime( fCoolTime );
		}
	}
}

void CDnDecreaseCooltimeBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnDecreaseCooltimeBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
}
