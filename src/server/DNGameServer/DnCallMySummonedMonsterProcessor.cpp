#include "stdafx.h"
#include "DnCallMySummonedMonsterProcessor.h"
#include "DnPlayerActor.h"
#include "DnMonsterActor.h"
#include "DnBlow.h"


CDnCallMySummonedMonsterProcessor::CDnCallMySummonedMonsterProcessor( DnActorHandle hActor, int iSummonedMonsterIDToCall ) : IDnSkillProcessor( hActor ), 
																															 m_iSummonedMonsterIDToCall( 0 ),
																															 m_bCalled( false )
																		   
{
	m_iType = CALL_MY_SUMMONED_MONSTER;

}

CDnCallMySummonedMonsterProcessor::~CDnCallMySummonedMonsterProcessor(void)
{

}


void CDnCallMySummonedMonsterProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill  )
{
	// Check if the monster ID is among my summoned monsters and warp it next to me.
	if( !m_hHasActor || !m_hHasActor->IsPlayerActor() )
		return;

	const list<DnMonsterActorHandle>& listSummonMonster = m_hHasActor->GetSummonedMonsterList();
	const map<int, list<DnMonsterActorHandle> >& mapSummonMonsterByGroup = m_hHasActor->GetGroupingSummonedMonsterList();

	if( false == listSummonMonster.empty() )
	{
		list<DnMonsterActorHandle>::const_iterator iter = listSummonMonster.begin();
		for( iter; iter != listSummonMonster.end(); ++iter )
		{
			DnMonsterActorHandle hMonster = (*iter);
			int iMonsterClassID = hMonster->GetMonsterClassID();
			if( iMonsterClassID == m_iSummonedMonsterIDToCall )
			{
				hMonster->CmdWarp( *m_hHasActor->GetPosition(), EtVec3toVec2( *m_hHasActor->GetLookDir() ) );
			}
		}
	}

	if( false == mapSummonMonsterByGroup.empty() )
	{
		map<int, list<DnMonsterActorHandle> >::const_iterator iterMap = mapSummonMonsterByGroup.begin();
		for( iterMap; iterMap != mapSummonMonsterByGroup.end(); ++iterMap )
		{
			const list<DnMonsterActorHandle>& listSummonMonster = iterMap->second;
			list<DnMonsterActorHandle>::const_iterator iter = listSummonMonster.begin();
			for( iter; iter != listSummonMonster.end(); ++iter )
			{
				DnMonsterActorHandle hMonster = (*iter);
				int iMonsterClassID = hMonster->GetMonsterClassID();
				if( iMonsterClassID == m_iSummonedMonsterIDToCall )
				{
					hMonster->CmdWarp( *m_hHasActor->GetPosition(), EtVec3toVec2( *m_hHasActor->GetLookDir() ) );
				}
			}
		}
	}

	m_bCalled = true;
}

void CDnCallMySummonedMonsterProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnCallMySummonedMonsterProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
}
