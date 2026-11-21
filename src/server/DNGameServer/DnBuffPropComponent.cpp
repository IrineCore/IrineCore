#include "stdafx.h"
#include "DnBuffPropComponent.h"
#include "DnActor.h"
#include "SMScanActor.h"
#include <boost/shared_ptr.hpp>
#include "DnMonsterActor.h"


CDnBuffPropComponent::CDnBuffPropComponent(void) : m_iTargetTeamType( -1 ), m_iMyTeam( -1 ), m_pGameRoom(NULL)
{
}

CDnBuffPropComponent::~CDnBuffPropComponent(void)
{
	// Clear all actors that have buffs applied.
	set<int>::iterator iter = m_setAffectingActors.begin();
	for( iter; iter != m_setAffectingActors.end(); ++iter )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_pGameRoom, *iter );
		if( hActor )
		{
			map<int, vector<int> >::iterator iterMap = m_mapAppliedStateEffect.find( hActor->GetUniqueID() );
			if( m_mapAppliedStateEffect.end() != iterMap )
			{
				const vector<int>& vlThisPropSEIDs = iterMap->second;

				int iNumIDs = (int)vlThisPropSEIDs.size();
				for( int i = 0; i < iNumIDs; ++i )
					hActor->CmdRemoveStateEffectFromID( vlThisPropSEIDs.at(i) );
			}
		}
	}
}


void CDnBuffPropComponent::Initialize( CMultiRoom* pRoom, int iTargetTeamType, int iSkillID, 
									   int iSkillLevel, const set<int>* pSetTargetActorIDs/* = NULL*/ )
{
	m_SkillComponent.Initialize( iSkillID, iSkillLevel );

	m_pGameRoom = pRoom;

	m_iTargetTeamType = iTargetTeamType;

	// Valid only when target team specification targets a specific actor ID.
	if( pSetTargetActorIDs )
	{
		_ASSERT( 3 == m_iTargetTeamType );
		m_setTargetActorIDs = *pSetTargetActorIDs;
	}
}


void CDnBuffPropComponent::OnMessage( const boost::shared_ptr<IStateMessage>& pMessage )
{
	const boost::shared_ptr<CSMScanActor> pScanActorMsg = boost::static_pointer_cast<CSMScanActor>(pMessage);
	set<int> setScanActor = pScanActorMsg->GetScanActorSet();

	// Newly added actor.
	vector<int> vlhNewActors;
	set_difference( setScanActor.begin(), setScanActor.end(), 
					m_setAffectingActors.begin(), m_setAffectingActors.end(), 
					back_inserter(vlhNewActors) );

	// Actor that went outside.
	vector<int> vlhOutActors;
	set_difference( m_setAffectingActors.begin(), m_setAffectingActors.end(), 
					setScanActor.begin(), setScanActor.end(),
					back_inserter(vlhOutActors) );

	// Add effect to the added actor.
	bool bUpdated = false;
	if( false == vlhNewActors.empty() )
	{
		vector<int>::iterator iter = vlhNewActors.begin();
		for( iter; iter != vlhNewActors.end(); ++iter )
		{
			// Handle by target type. Set duration to permanent and delete if out of range.
			// Exclude NPC
			DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_pGameRoom, *iter );
			if( hActor && hActor->GetActorType() < CDnActorState::Npc )
			{
				// When targeting specific actors, process only those with matching IDs.
				bool bTargetActor = false;
				if( 3 == m_iTargetTeamType )
				{
					if( m_setTargetActorIDs.end() != m_setTargetActorIDs.find(hActor->GetClassID()) )
					{
						bTargetActor = true;
					}
				}
				else
				{
					bTargetActor = true;
				}

				if( false == bTargetActor )
					continue;

				// Need to handle duration type index when adding status ailment.
				// Identify the Actor's continuous effect index and add the state effect on success.
				map<int, bool> mapDuplicateResult;
				const CDnSkill::SkillInfo& HasSkillInfo = m_SkillComponent.GetSkillInfo();
				CDnSkill::CanApply eResult = CDnSkill::CanApplySkillStateEffect( &HasSkillInfo, hActor, HasSkillInfo.iSkillID, HasSkillInfo.iLevel, 
																				 HasSkillInfo.iSkillDuplicateMethod, 
																				 HasSkillInfo.iDuplicateCount, mapDuplicateResult );

				if( CDnSkill::CanApply::Fail != eResult )
				{
					vector<int> vlAddedSEIDs;
					int iNumStateEffect = m_SkillComponent.GetNumStateEffect();
					for( int iStateEffect = 0; iStateEffect < iNumStateEffect; ++iStateEffect  )
					{
						int iActorType = hActor->GetActorType();
						const CDnSkill::StateEffectStruct* pStateEffectInfo = m_SkillComponent.GetStateEffectInfo( iStateEffect );
						bool bCheckTeam = false;
						if( 3 != m_iTargetTeamType )		// When not targeting a specific actor ID.
						{
							if( -1 != m_iMyTeam )
							{								
								bCheckTeam = (2 == m_iTargetTeamType) ||		// all
											 (m_iTargetTeamType == 0 && (m_iMyTeam == hActor->GetTeam()) ) ||	// friend
											 (m_iTargetTeamType == 1 && (m_iMyTeam != hActor->GetTeam()) );		// enemy
							}
							else
							{
								// #45096 Treat monsters summoned by a player as being on the player's team.
								if( hActor->IsMonsterActor() )
								{
									CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
									DnActorHandle hSummonerPlayer = pMonsterActor->GetSummonerPlayerActor();
									if( hSummonerPlayer )
										iActorType = hSummonerPlayer->GetActorType();
								}

								bCheckTeam = (2 == m_iTargetTeamType) ||		// all
									(m_iTargetTeamType == 0 && iActorType <= CDnActorState::Reserved6 || iActorType == CDnActorState::Vehicle ) ||	// player
											 (m_iTargetTeamType == 1 && iActorType > CDnActorState::Reserved6 );	// monster
							}
						}
						else
						{
							bCheckTeam = true;
						}

						if( bCheckTeam )
						{
							// When stacking the same skill, probability-based skill effects already have their probability checked in CanApplySkillStateEffect
							// Already passed. So here, check if the probability check was already done, and if so, call the function to skip checking probability again.
							bool bAllowAddThisSE = true;
							bool bCheckCanBegin = true;
							if( CDnSkill::CanApply::ApplyDuplicateSameSkill == eResult )
							{
								map<int, bool>::iterator iter = mapDuplicateResult.find( pStateEffectInfo->nID );
								// If not in map, the current actor does not have the state effect, so just run the normal state effect addition routine.
								if( mapDuplicateResult.end() != iter )
								{
									// Called CanAdd because a probabilistic state effect of the same skill is currently applied, but it failed.
									// Do not add state effect in this case.
									if( false == (iter->second) )
										bAllowAddThisSE = false;
									else
										// Since CanAdd already passed, skip separate check when calling CmdAddStateEffect.
										bCheckCanBegin = false;
								}
							}

							int iID = hActor->CmdAddStateEffect( &m_SkillComponent.GetSkillInfo(), (STATE_BLOW::emBLOW_INDEX)pStateEffectInfo->nID, 
																 -1, pStateEffectInfo->szValue.c_str(), false, bCheckCanBegin );
							if( -1 != iID )
								vlAddedSEIDs.push_back( iID );
						}
					}

					if( false == vlAddedSEIDs.empty() )
						m_mapAppliedStateEffect.insert( make_pair(hActor->GetUniqueID(), vlAddedSEIDs) );
				}
			}
		}

		bUpdated = true;
	}

	// Remove effect from actor that went outside.
	if( false == vlhOutActors.empty() )
	{
		vector<int>::iterator iter = vlhOutActors.begin();
		for( iter; iter != vlhOutActors.end(); ++iter )
		{
			DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_pGameRoom, *iter );
			if( hActor && hActor->GetActorType() < CDnActorState::Npc )
			{
				map<int, vector<int> >::iterator iterMap = m_mapAppliedStateEffect.find( hActor->GetUniqueID() );
				if( m_mapAppliedStateEffect.end() != iterMap )
				{
					const vector<int>& vlThisPropSEIDs = iterMap->second;

					int iNumIDs = (int)vlThisPropSEIDs.size();
					for( int i = 0; i < iNumIDs; ++i )
						hActor->CmdRemoveStateEffectFromID( vlThisPropSEIDs.at(i) );

					m_mapAppliedStateEffect.erase( iterMap );
				}
			}
		}

		bUpdated = true;
	}

	if( bUpdated )
		m_setAffectingActors = setScanActor;
}

void CDnBuffPropComponent::RemoveAffectedStateEffects( void )
{
	set<int>::iterator iter = m_setAffectingActors.begin();
	for( iter; iter != m_setAffectingActors.end(); ++iter )
	{
		map<int, vector<int> >::iterator iterMap = m_mapAppliedStateEffect.find( *iter );
		if( m_mapAppliedStateEffect.end() != iterMap )
		{
			DnActorHandle hActor = CDnActor::FindActorFromUniqueID( m_pGameRoom, *iter );
			if( hActor )
			{
				vector<int>& vlBlowsToRemove = iterMap->second;
				for( int i = 0; i < (int)vlBlowsToRemove.size(); ++i )
				{
					int iBlowID = vlBlowsToRemove.at( i );
					hActor->CmdRemoveStateEffectFromID( iBlowID );
				}
			}
		}
	}

	m_setAffectingActors.clear();
	m_mapAppliedStateEffect.clear();
}