#include "stdafx.h"
#include "DnImpactBlowProcessor.h"
#include "DnClericMentalChargeBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnImpactBlowProcessor::CDnImpactBlowProcessor( DnActorHandle hActor ) : IDnSkillProcessor( hActor ), 
																		 m_pState( new CDnState ),
																		 m_iNumChargeBlow( 0 )
{

}

CDnImpactBlowProcessor::~CDnImpactBlowProcessor(void)
{
	SAFE_DELETE( m_pState );
}


void CDnImpactBlowProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	vector<DnBlowHandle> vlChargeBlow;

	int iNumAppliedStateBlow = m_hHasActor->GetNumAppliedStateBlow();
	for( int iBlow = 0; iBlow < iNumAppliedStateBlow; ++iBlow )
	{
		DnBlowHandle hBlow = m_hHasActor->GetAppliedStateBlow(iBlow);
		if( STATE_BLOW::BLOW_072 == hBlow->GetBlowIndex() )
			vlChargeBlow.push_back( hBlow );
	}

	// Attack power increase
	if( false == vlChargeBlow.empty() )
	{
		int iNumChargeBlow = (int)vlChargeBlow.size();
		float fValue = 1.0f;
		for( int iBlow = 0; iBlow < iNumChargeBlow; ++iBlow )
		{
			DnBlowHandle hBlow = vlChargeBlow.at( iBlow );
			fValue = fValue + fValue*hBlow->GetFloatValue();
		}

		m_pState->SetAttackPMinRatio( fValue );
		m_pState->SetAttackPMaxRatio( fValue );
		m_pState->CalcValueType();

		m_hHasActor->AddBlowState( m_pState );

		m_iNumChargeBlow = (int)vlChargeBlow.size();
	}
}


void CDnImpactBlowProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{


}


void CDnImpactBlowProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// Must clear all energy charges once Impact Blow skill fires
	// No need to send a separate packet; client and game server can each delete it..
	// TODO: Needs testing after all.
	for( int iChargeBlow = 0; iChargeBlow < m_iNumChargeBlow; ++iChargeBlow )
	{
		m_hHasActor->RemoveStateBlowByBlowDefineIndex( STATE_BLOW::BLOW_072 );
	}

	m_hHasActor->DelBlowState( m_pState );
	m_hHasActor->RefreshState();
}