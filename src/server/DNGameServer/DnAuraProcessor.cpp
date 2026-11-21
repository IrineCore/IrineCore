#include "stdafx.h"
#include "DnAuraProcessor.h"
#include "DnActor.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DNUserSession.h"
#include "DNGameRoom.h"
#include "DnBlow.h"
#include "DnGameTask.h"


CDnAuraProcessor::CDnAuraProcessor( DnActorHandle hActor, float fRange ) : IDnSkillProcessor( hActor ), 
																		   m_fRange( fRange ),
																		   m_fRangeSQ( fRange*fRange ),
																		   m_LastAuraCheckTime( 0 ),
																		   m_bAuraOff( false )
{

}

CDnAuraProcessor::~CDnAuraProcessor(void)
{

}


void CDnAuraProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill  )
{
	m_LastAuraCheckTime = LocalTime;
	m_setPartyMemberInAura.clear();
	m_mapAuraMemberSEID.clear();
	m_bAuraOff = false;
}

void CDnAuraProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(m_hHasActor->GetRoom()).GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
		return;

	int nRefreshTime = 30;

#ifdef PRE_ADD_AURA_FOR_MONSTER
	if(m_hHasActor->IsMonsterActor()) // Since monsters use ScanActor, increase the refresh time gap a bit more.
		nRefreshTime = 600;
#endif

	// Gather nearby allies and set targets for the newly entered actor to buff
	if( LocalTime - m_LastAuraCheckTime > nRefreshTime )
	{
		//assert( m_hHasActor->IsEnabledAuraSkill() && "Aura skill must be active while aura processor is running" );
		DnSkillHandle hAuraSkill = m_hHasActor->GetEnabledAuraSkill();

		if( NULL == hAuraSkill )
			return;

		// Since monsters can now also use aura skills, gather candidates by party for players, 
		// For monsters, we have no choice but to use ScanActor.. (#15461)
		DNVector( DnActorHandle ) vlActors;
		if( m_hHasActor->IsPlayerActor() )
		{
			CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr(m_hHasActor->GetRoom())->GetTask( "PartyTask" ));
			if (pPartyTask)
			{
				vlActors.reserve( pPartyTask->GetUserCount() );
				for( int i = 0; i < (int)pPartyTask->GetUserCount(); ++i )
				{
					CDNGameRoom::PartyStruct* pPartyData = pPartyTask->GetPartyData( i );
					vlActors.push_back( pPartyData->pSession->GetActorHandle() );
				}
			}
		}
		else
		{
			CDnActor::ScanActor( m_hHasActor->GetRoom(), *(m_hHasActor->GetPosition()), m_fRange, vlActors );
		}


		set<DWORD> setPrevAuraMember = m_setPartyMemberInAura;
		m_setPartyMemberInAura.clear();

		for( int i = 0; i < (int)vlActors.size(); ++i )
		{
			DnActorHandle hActor = vlActors.at( i );

			// Ignore invalid objects.
			if( !hActor )
				continue;

			// Skip self..
			if( hActor != m_hHasActor )
			{
#if defined(PRE_FIX_55383)
				bool bSkip = false;
				CDnSkill::TargetTypeEnum skillTargetType = hAuraSkill->GetTargetType();
				switch(skillTargetType)
				{
				case CDnSkill::TargetTypeEnum::Self: //Self is skipped above, but apply code here for now.
					{
						bSkip = hActor != m_hHasActor;
					}
					break;
				case CDnSkill::TargetTypeEnum::Enemy: //Set as enemy, but skip if on the same team
					{
						int nMyTeam = m_hHasActor->GetTeam();
						if (m_hHasActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nMyTeam = m_hHasActor->GetOriginalTeam();

						int nTargetTeam = hActor->GetTeam();
						if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nTargetTeam = hActor->GetOriginalTeam();

						bSkip = nMyTeam == nTargetTeam;
					}
					break;
				case CDnSkill::TargetTypeEnum::Friend: //Skip if set to ally but on a different team
					{
						int nMyTeam = m_hHasActor->GetTeam();
						if (m_hHasActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nMyTeam = m_hHasActor->GetOriginalTeam();

						int nTargetTeam = hActor->GetTeam();
						if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nTargetTeam = hActor->GetOriginalTeam();

						bSkip = nMyTeam != nTargetTeam;
					}
					break;
				case CDnSkill::TargetTypeEnum::Party:	//Handle same as Friend for now; if requested later, treat summoned monsters as not Party?
					{
						int nMyTeam = m_hHasActor->GetTeam();
						if (m_hHasActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nMyTeam = m_hHasActor->GetOriginalTeam();

						int nTargetTeam = hActor->GetTeam();
						if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nTargetTeam = hActor->GetOriginalTeam();

						bSkip = nMyTeam != nTargetTeam;
					}
					break;
				case CDnSkill::TargetTypeEnum::All:
					bSkip = false;
					break;
				}

				if (bSkip == true)
					continue;
				
#else
				// For players in PvP, everyone is in PartyStruct, so team distinction is needed.
				// Same for monsters since scanactor is used.
				if( hActor->GetTeam() != m_hHasActor->GetTeam() 
#ifdef PRE_ADD_AURA_FOR_MONSTER
					&& hAuraSkill->GetTargetType() != CDnSkill::TargetTypeEnum::Enemy
					|| (hAuraSkill->GetTargetType() == CDnSkill::TargetTypeEnum::Enemy && hActor->GetTeam() == m_hHasActor->GetTeam())
#endif
					)
					continue;
#endif // PRE_FIX_55383

				// Exclude targets casting the same aura skill.
				DnSkillHandle hIfNewMemberAuraSkill = hActor->GetEnabledAuraSkill();
				if( hIfNewMemberAuraSkill &&
					hIfNewMemberAuraSkill->GetClassID() == hAuraSkill->GetClassID() )
					continue;

				// Exclude targets already affected by the same aura skill from another character, when aura skill stacking is allowed. 
				// If the effect was replaced, the target remains in another actor's AuraProcessor, so 
				// If aura is released, effects do not apply even if inside another character's aura area.
				bool bSameAuraSkillsBlow = false;
				DNVector(DnBlowHandle) vlhAppliedBlow;
				hActor->GetAllAppliedStateBlow( vlhAppliedBlow );
				DNVector(DnBlowHandle)::iterator iter = vlhAppliedBlow.begin();
				for( iter; iter != vlhAppliedBlow.end(); ++iter )
				{
					const CDnSkill::SkillInfo* pParentSkillInfo = (*iter)->GetParentSkillInfo();
					if( m_hHasActor != pParentSkillInfo->hSkillUser &&
						pParentSkillInfo->iSkillID == hAuraSkill->GetClassID() )
					{
						bSameAuraSkillsBlow = true;
						break;
					}
				}

				if( bSameAuraSkillsBlow )
					continue;

				// For monsters, since ScanActor is used, only those within range are searched 
				// No need to check distance separately.
				bool bInRange = true;
				if( m_hHasActor->IsPlayerActor() )
				{
					bInRange = false;
					const EtVector3* pPos = hActor->GetPosition();
					const EtVector3* pMyPos = m_hHasActor->GetPosition();

					EtVector3 vDist = *pPos - *pMyPos;
					float fDistSQ = EtVec3LengthSq( &vDist );

					// Add those within a certain distance; set the distance arbitrarily for now. (300)
					if( fDistSQ < m_fRangeSQ )
						bInRange = true;
				}
				
				if( bInRange )
					m_setPartyMemberInAura.insert( hActor->GetUniqueID() );
			}
		}

		// Select newly added and removed elements using set difference.
		vector<DWORD> vlNewMember;
		set_difference( m_setPartyMemberInAura.begin(), m_setPartyMemberInAura.end(), setPrevAuraMember.begin(), setPrevAuraMember.end(), 
						back_inserter(vlNewMember) );

		vector<DWORD> vlOutMember;
		set_difference( setPrevAuraMember.begin(), setPrevAuraMember.end(), m_setPartyMemberInAura.begin(), m_setPartyMemberInAura.end(),
						back_inserter(vlOutMember) );

		int iNumNewMember = (int)vlNewMember.size();
		for( int iMember = 0; iMember < iNumNewMember; ++iMember )
		{
			// Add target state effects held by the aura.
			DnActorHandle hNewMember = CDnActor::FindActorFromUniqueID( m_hHasActor->GetRoom(), vlNewMember.at(iMember) );
			if( hNewMember )
			{
				DWORD dwNumStateEffect = hAuraSkill->GetStateEffectCount();
				for( DWORD dwStateEffect = 0; dwStateEffect < dwNumStateEffect; ++dwStateEffect )
				{
					const CDnSkill::StateEffectStruct* pSE = hAuraSkill->GetStateEffectFromIndex( dwStateEffect );
#if defined(PRE_FIX_55383)
					bool bCanApply = false;
					switch(pSE->ApplyType)
					{
					case CDnSkill::ApplySelf:	//If Self, target must be oneself.
						{
							bCanApply = m_hHasActor == hNewMember;
						}
						break;
					case CDnSkill::ApplyTarget: //Regardless of team if target is not self
						{
							bCanApply = m_hHasActor != hNewMember;
						}
						break;
					case CDnSkill::ApplyAll: //Unconditionally
						{
							bCanApply = true;
						}
						break;
					case CDnSkill::ApplyEnemy: //Only when on different teams
						{
							int nMyTeam = m_hHasActor->GetTeam();
							if (m_hHasActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
								nMyTeam = m_hHasActor->GetOriginalTeam();

							int nTargetTeam = hNewMember->GetTeam();
							if (hNewMember->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
								nTargetTeam = hNewMember->GetOriginalTeam();

							bCanApply = nMyTeam != nTargetTeam;
						}
						break;
					case CDnSkill::ApplyFriend: //Only when on the same team..
						{
							int nMyTeam = m_hHasActor->GetTeam();
							if (m_hHasActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
								nMyTeam = m_hHasActor->GetOriginalTeam();

							int nTargetTeam = hNewMember->GetTeam();
							if (hNewMember->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
								nTargetTeam = hNewMember->GetOriginalTeam();

							bCanApply = nMyTeam == nTargetTeam;
						}
						break;
					}

					if (bCanApply == true)
					{
						int iID = hNewMember->CmdAddStateEffect( /*m_hHasActor,*/ hAuraSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1/*SE.nDurationTime*/, pSE->szValue.c_str() );
						m_mapAuraMemberSEID[ hNewMember->GetUniqueID() ].push_back( iID );
					}
#else
					if( CDnSkill::ApplyTarget == pSE->ApplyType )
					{
						int iID = hNewMember->CmdAddStateEffect( /*m_hHasActor,*/ hAuraSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1/*SE.nDurationTime*/, pSE->szValue.c_str() );
						m_mapAuraMemberSEID[ hNewMember->GetUniqueID() ].push_back( iID );
					}
#endif // PRE_FIX_55383
				}
			}
		}

		int iNumOutMember = (int)vlOutMember.size();
		for( int iMember = 0; iMember < iNumOutMember; ++iMember )
		{
			// Remove state effects held by the aura.
			DnActorHandle hOutMember = CDnActor::FindActorFromUniqueID( m_hHasActor->GetRoom(), vlOutMember.at(iMember) );
			if( hOutMember )
			{
				//// Ignore targets casting the same aura skill.
				//DnSkillHandle hIfOutMemberAuraSkill = hOutMember->GetEnabledAuraSkill();
				//if( hIfOutMemberAuraSkill &&
				//	hIfOutMemberAuraSkill->GetClassID() == hAuraSkill->GetClassID() )
				//	continue;

				// Even if the target is casting the same aura skill, the active buff might have been applied by my aura, so 
				// Remove state effect using the ID assigned upon creation.
				//DWORD dwNumStateEffect = hAuraSkill->GetStateEffectCount();
				//for( DWORD dwStateEffect = 0; dwStateEffect < dwNumStateEffect; ++dwStateEffect )
				//{
				//	const CDnSkill::StateEffectStruct* pSE = hAuraSkill->GetStateEffectFromIndex( dwStateEffect );
				//	if( CDnSkill::ApplyTarget == pSE->ApplyType )
				//		hOutMember->CmdRemoveStateEffect( (STATE_BLOW::emBLOW_INDEX)pSE->nID );
				//}

				vector<int>& vlBlowIDsToRemove = m_mapAuraMemberSEID[ hOutMember->GetUniqueID() ];
				for( int i = 0; i < (int)vlBlowIDsToRemove.size(); ++i )
					hOutMember->CmdRemoveStateEffectFromID( vlBlowIDsToRemove.at(i) );

				m_mapAuraMemberSEID.erase( hOutMember->GetUniqueID() );
			}
		}

		m_LastAuraCheckTime = LocalTime;
	}
}

void CDnAuraProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_setPartyMemberInAura.empty() )
		return;

	// If it is an aura skill, remove the state effects held by the aura.
	set<DWORD>::iterator iter = m_setPartyMemberInAura.begin();

	//assert( m_hHasActor->IsEnabledAuraSkill() && "Aura skill must be active while aura processor is running" );
	DnSkillHandle hAuraSkill = m_hHasActor->GetEnabledAuraSkill();

	if( hAuraSkill )
	{
		for( iter; iter != m_setPartyMemberInAura.end(); ++iter )
		{
			DnActorHandle hMember = CDnActor::FindActorFromUniqueID( m_hHasActor->GetRoom(), *iter );

			if( hMember )
			{
				vector<int>& vlBlowIDsToRemove = m_mapAuraMemberSEID[ hMember->GetUniqueID() ];
				for( int i = 0; i < (int)vlBlowIDsToRemove.size(); ++i )
					hMember->CmdRemoveStateEffectFromID( vlBlowIDsToRemove.at(i) );
			}
		}

		m_setPartyMemberInAura.clear();
		m_mapAuraMemberSEID.clear();
	}

	m_bAuraOff = true;
}
