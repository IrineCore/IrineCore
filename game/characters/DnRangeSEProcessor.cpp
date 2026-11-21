#include "stdafx.h"
#include "DnRangeSEProcessor.h"
#include "DnStateBlow.h"


CDnRangeSEProcessor::CDnRangeSEProcessor( DnActorHandle hActor, float fRange, float fProbability, int iTargetEffectOutputID, int iDurationTime ) : 
																									IDnSkillProcessor( hActor ), 
																									 m_fRange( fRange ), m_fProbability( fProbability ),
																									 m_bFinished( false ),
																									 m_iTargetEffectOutputID( iTargetEffectOutputID ),
																									 m_iDurationTime( iDurationTime )
{
	m_iType = RANGE_STATE_EFFECT;
}

CDnRangeSEProcessor::~CDnRangeSEProcessor(void)
{
}


void CDnRangeSEProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	m_setAffectedActor.clear();

	m_fTimeLength = (float)m_iDurationTime / 1000.0f;
}


// Search nearby only once and insert.
void CDnRangeSEProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_fTimeLength -= fDelta;
	if( m_fTimeLength <= 0.0f )
		m_fTimeLength = 0.0f;

	// Search nearby area and apply state effect with a certain probability.
	CDnSkill::TargetTypeEnum TargetType = m_hParentSkill->GetTargetType();
	//_ASSERT( CDnSkill::Self != TargetType );

	DNVector(DnActorHandle) vlhNearbyActors;
	CDnActor::ScanActor( m_hHasActor->GetRoom(), *(m_hHasActor->GetPosition()), m_fRange, vlhNearbyActors );

	int iNumActors = (int)vlhNearbyActors.size();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		DnActorHandle hActor = vlhNearbyActors.at( iActor );
		if( m_hHasActor == hActor )
			continue;

		if( m_setAffectedActor.find( hActor->GetUniqueID() ) != m_setAffectedActor.end() )
			continue;

		// Only targets whose canhit signal is true..
		if( false == hActor->IsHittable( m_hHasActor, LocalTime ) )
			continue;

		switch( TargetType )
		{
			case CDnSkill::Enemy:
			case CDnSkill::Self:
				if( m_hHasActor->GetTeam() == hActor->GetTeam() )
					continue;
				break;

			case CDnSkill::Friend:
			case CDnSkill::Party:
				if( m_hHasActor->GetTeam() != hActor->GetTeam() )
					continue;
				break;
		}

		if( _rand(m_hHasActor->GetRoom())%100 <= m_fProbability*100.0f )
		{
			// The range state effect add manifestation type object does not separately handle skill state effect stacking.
			map<int, bool> mapDuplicateResult;
			CDnSkill::CanApply eResult = CDnSkill::CanApplySkillStateEffect( hActor, m_hParentSkill, mapDuplicateResult );
			if( CDnSkill::Fail != eResult )
			{
				m_setAffectedActor.insert( hActor->GetUniqueID() );

				DWORD dwNumStateEffect = m_hParentSkill->GetStateEffectCount();
				for( DWORD dwStateEffect = 0; dwStateEffect < dwNumStateEffect; ++dwStateEffect )
				{
					const CDnSkill::StateEffectStruct* pSE = m_hParentSkill->GetStateEffectFromIndex( dwStateEffect );
					if( CDnSkill::ApplyTarget == pSE->ApplyType )
					{
						CDnSkill::SkillInfo SkillInfo = *(m_hParentSkill->GetInfo());
						char szBuff[128] = {0, };
						_snprintf_s(szBuff, _countof(szBuff), _TRUNCATE, "%d", m_iTargetEffectOutputID );
						SkillInfo.szEffectOutputIDToClient = szBuff;

						if( !hActor->IsImmuned( (STATE_BLOW::emBLOW_INDEX)pSE->nID ) ) // #47379 Configure not to trigger on immune targets.
						{
							int iBlowID = hActor->CmdAddStateEffect( &SkillInfo, (STATE_BLOW::emBLOW_INDEX)pSE->nID, pSE->nDurationTime, pSE->szValue.c_str() );
						}

					}
				}
			}
		}
	}

	//m_bFinished = true;
}


void CDnRangeSEProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	
}