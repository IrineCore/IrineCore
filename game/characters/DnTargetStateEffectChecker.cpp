#include "stdafx.h"
#include "DnTargetStateEffectChecker.h"
#include "DnActor.h"
#include "DnBlow.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnTargetStateEffectChecker::CDnTargetStateEffectChecker( DnActorHandle hActor, STATE_BLOW::emBLOW_INDEX targetStateIndex, float fRadiusDistToSearch, bool bCheckSkillUser ) : IDnSkillUsableChecker( hActor ), 
m_targetStateIndex( targetStateIndex ),
m_fRadiusDistToSearch( fRadiusDistToSearch ),
m_bCheckSkillUser(bCheckSkillUser)
{
	m_iType = TARGET_STATEEFECT_CHECKER;
}

CDnTargetStateEffectChecker::~CDnTargetStateEffectChecker(void)
{

}

bool CDnTargetStateEffectChecker::CanUse( void )
{
	int nCheckCount = 0;

	if( m_hHasActor && m_targetStateIndex != STATE_BLOW::BLOW_NONE)
	{
		DNVector(DnActorHandle) vlhTargetActors;
		CDnActor::ScanActorByActorSize( ARG_STATIC_FUNC(m_hHasActor) *m_hHasActor->GetPosition(), m_hHasActor->GetUnitSize() + m_fRadiusDistToSearch, vlhTargetActors, false, true );
		EtVector3 vDistance;
		EtVector3 vDir;

		int iNumTargetActors = (int)vlhTargetActors.size();
		for( int i = 0; i < iNumTargetActors; ++i )
		{
			DnActorHandle hTargetActor = vlhTargetActors.at( i );

			if( hTargetActor == m_hHasActor->GetMySmartPtr() )
				continue;

			if( hTargetActor->IsDie() )
				continue;

			//Check if the configured state effect is applied to the target actor
			if (hTargetActor->IsAppliedThisStateBlow(m_targetStateIndex) == true)
			{
				//When checking skill user
				if (m_bCheckSkillUser == true)
				{
					DNVector( DnBlowHandle ) vlhBlows;
					hTargetActor->GatherAppliedStateBlowByBlowIndex(m_targetStateIndex, vlhBlows);

					int nBlowCount = (int)vlhBlows.size();
					for (int i = 0; i < nBlowCount; ++i)
					{
						DnBlowHandle hBlow = vlhBlows[i];
						if (!hBlow || hBlow->IsEnd())
							continue;

						CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
						if (pSkillInfo && pSkillInfo->hSkillUser == m_hHasActor)
							nCheckCount++;
					}
				}
				//When not checking skill user
				else
					nCheckCount++;
			}
		}
	}

	return (nCheckCount > 0);
}

IDnSkillUsableChecker* CDnTargetStateEffectChecker::Clone()
{
	CDnTargetStateEffectChecker* pNewChecker = new CDnTargetStateEffectChecker(m_hHasActor, m_targetStateIndex, m_fRadiusDistToSearch, m_bCheckSkillUser);

	return pNewChecker;
}
