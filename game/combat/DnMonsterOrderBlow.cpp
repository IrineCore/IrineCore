#include "stdafx.h"
#include "DnMonsterOrderBlow.h"
#ifdef _GAMESERVER
#include "DnMonsterActor.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnMonsterOrderBlow::CDnMonsterOrderBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_nOrderID( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_287;
	SetValue( szValue );
	m_nOrderID = szValue ? atoi( szValue ) : 0;
}

CDnMonsterOrderBlow::~CDnMonsterOrderBlow( void )
{
}

void CDnMonsterOrderBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	if( !m_hActor || m_nOrderID <= 0 )
		return;

	if( m_hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>( m_hActor.GetPointer() );
		if( pMonster && !pMonster->IsDie() && pMonster->FindSkill( m_nOrderID ) )
		{
			pMonster->UseSkill( m_nOrderID, false, true );
			return;
		}
	}

	const std::list<DnMonsterActorHandle>& listSummoned = m_hActor->GetSummonedMonsterList();
	for( std::list<DnMonsterActorHandle>::const_iterator it = listSummoned.begin(); it != listSummoned.end(); ++it )
	{
		DnMonsterActorHandle hSummon = *it;
		if( hSummon && !hSummon->IsDie() && hSummon->FindSkill( m_nOrderID ) )
			hSummon->UseSkill( m_nOrderID, false, true );
	}
#endif
}

void CDnMonsterOrderBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnMonsterOrderBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
}
