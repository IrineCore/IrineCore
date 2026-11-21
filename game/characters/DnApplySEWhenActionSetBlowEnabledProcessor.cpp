#include "stdafx.h"
#include "DnApplySEWhenActionSetBlowEnabledProcessor.h"
#include "DnClericMentalChargeBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnApplySEWhenActionSetBlowEnabledProcessor::CDnApplySEWhenActionSetBlowEnabledProcessor( DnActorHandle hActor ) : IDnSkillProcessor( hActor )
{
	m_iType = APPLY_SE_WHEN_ACTIONSET_ENABLED;
}

CDnApplySEWhenActionSetBlowEnabledProcessor::~CDnApplySEWhenActionSetBlowEnabledProcessor(void)
{

}


void CDnApplySEWhenActionSetBlowEnabledProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	//// Add parent state effects excluding ChangeActionSet state effects.
	//// The skill side is configured not to add these state effects upon activation.
	//int iNumSE = m_hParentSkill->GetStateEffectCount();
	//for( int i = 0; i < iNumSE; ++i )
	//{
	//	const CDnSkill::StateEffectStruct* pStateEffect = m_hParentSkill->GetStateEffectFromIndex( i );
	//	if( STATE_BLOW::BLOW_129 != pStateEffect->nID ) // Store state effects to apply separately, excluding change action set state effect.
	//	{
	//		m_vlStateEffect
	//	}
	//}
}

#ifdef _GAMESERVER
void CDnApplySEWhenActionSetBlowEnabledProcessor::AddStateEffect( const CDnSkill::StateEffectStruct* pStateEffect )
{
	// State effects assigned to the skill object upon creation, excluding only the change action set state effect
	// Put everything in here first and then remove them. 
	m_vlpStateEffect.push_back( pStateEffect );
}

void CDnApplySEWhenActionSetBlowEnabledProcessor::OnChangeActionBegin( void )
{
	for( int i = 0; i < (int)m_vlpStateEffect.size(); ++i )
	{
		const CDnSkill::StateEffectStruct* pStateEffect = m_vlpStateEffect.at( i );
		int iID = m_hHasActor->CmdAddStateEffect( m_hParentSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pStateEffect->nID, 
												  /*pStateEffect->nDurationTime*/-1, pStateEffect->szValue.c_str() );

		if( -1 != iID )
			m_vlAppliedStateEffectID.push_back( iID );
	}
	
	// #33312 Force-assign parent skill's element
	if( m_hParentSkill )
	{
		CDnState::ElementEnum eElement = m_hParentSkill->GetElement();
		m_hHasActor->PushForceHitElement( eElement );
	}
}

void CDnApplySEWhenActionSetBlowEnabledProcessor::OnChangeActionSetBlowEnd( void )
{
	// Remove previously added state effects.
	for( int i = 0; i < (int)m_vlAppliedStateEffectID.size(); ++i )
	{
		m_hHasActor->CmdRemoveStateEffectFromID( m_vlAppliedStateEffectID.at(i) );
	}

	m_vlAppliedStateEffectID.clear();

	// #33312 Release the force-assigned parent skill element.
	if( m_hParentSkill )
	{
		CDnState::ElementEnum eElement = m_hParentSkill->GetElement();
		m_hHasActor->PopForceHitElement();
	}
}
#endif


void CDnApplySEWhenActionSetBlowEnabledProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{


}


void CDnApplySEWhenActionSetBlowEnabledProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

}