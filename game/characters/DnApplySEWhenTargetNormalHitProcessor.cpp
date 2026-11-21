#include "stdafx.h"
#include "DnApplySEWhenTargetNormalHitProcessor.h"
#include "DnActor.h"

#ifdef _GAMESERVER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnApplySEWhenTargetNormalHitProcessor::CDnApplySEWhenTargetNormalHitProcessor( DnActorHandle hActor, float fProbability ) : 
																				IDnSkillProcessor( hActor ),
																				m_fProbability( fProbability )
{
	m_iType = APPLY_SE_WHEN_TARGET_NORMAL_HIT;

	// Register pointer to self object in MASkillUser.
	m_hHasActor->AddApplySEWhenTargetNormalHitProcessor( this );
}

CDnApplySEWhenTargetNormalHitProcessor::~CDnApplySEWhenTargetNormalHitProcessor( void )
{
	// Remove from MASkillUser.
	m_hHasActor->RemoveApplySEWhenTargetNormalHitProcessor( this );
}


void CDnApplySEWhenTargetNormalHitProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{

}

void CDnApplySEWhenTargetNormalHitProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{

}


void CDnApplySEWhenTargetNormalHitProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnApplySEWhenTargetNormalHitProcessor::OnNormalHitSuccess( DnActorHandle hHittedTarget )
{
	if( !m_hParentSkill )
		return;

	if( !hHittedTarget )
		return;

	if( hHittedTarget->IsDie() )
		return;

	int iNumSE = m_hParentSkill->GetStateEffectCount();
	for( int i = 0; i < iNumSE; ++i )
	{
		const CDnSkill::StateEffectStruct* pSE = m_hParentSkill->GetStateEffectFromIndex( i );

		// Insert only those designated as target.
		if( CDnSkill::ApplyTarget == pSE->ApplyType )
		{
			hHittedTarget->CmdAddStateEffect( m_hParentSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, pSE->nDurationTime, pSE->szValue.c_str() );
		}
	}
}

#endif // #ifdef _GAMESERVER