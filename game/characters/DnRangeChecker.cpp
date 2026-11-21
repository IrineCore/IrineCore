#include "stdafx.h"
#include "DnRangeChecker.h"
#include "DnActor.h"

#ifdef _CLIENT
#include "DnHideMonsterActor.h"
#include "DnPetActor.h"
#ifdef PRE_FIX_GUIDEDARROW_ON_PVPTOURNAMENT
#include "DnStateBlow.h"
#endif
#endif

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif


CDnRangeChecker::CDnRangeChecker( DnActorHandle hActor ) : IDnSkillUsableChecker( hActor ),
														   m_iRange( 0 )
{
	m_iType = RANGE_CHECKER;
}

CDnRangeChecker::~CDnRangeChecker(void)
{

}


bool CDnRangeChecker::CanUse( void )
{
#ifdef _GAMESERVER
	return true;
#else
	bool bResult = false;

	DNVector(DnActorHandle) vlActors;
	CDnActor::ScanActor( (*m_hHasActor->GetPosition()), (float)m_iRange, vlActors );

	int iNumActors = (int)vlActors.size();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		DnActorHandle hTargetActor = vlActors.at( iActor );

		// #32115 Skip actors created as invisible monsters for a specific purpose.
		if( hTargetActor->IsMonsterActor() )
		{
			if( NULL != dynamic_cast<CDnHideMonsterActor*>(hTargetActor.GetPointer()) )
				continue;

			if( hTargetActor->GetActorType() == CDnActor::NoAggroTrap )
				continue;
		}

		if ( NULL != dynamic_cast<CDnPetActor*>(hTargetActor.GetPointer()))
			continue;

		// Check actors on other teams excluding NPCs.
		if( hTargetActor->GetTeam() != m_hHasActor->GetTeam() )
		{
#ifdef PRE_FIX_GUIDEDARROW_ON_PVPTOURNAMENT
			CDnStateBlow* pTargetStateBlow = hTargetActor->GetStateBlow();
			if (pTargetStateBlow && pTargetStateBlow->IsApplied( STATE_BLOW::BLOW_230 ))
				continue;
#endif

			// Exclude dead actors and NPCs
			if( (hTargetActor->GetActorType() != CDnActorState::Npc) &&
				(hTargetActor->GetActorType() != CDnActorState::Vehicle) &&
				(false == hTargetActor->IsDie()) && hTargetActor->CDnActorState::IsHittable() )
			{
				bResult = true;
				break;
			}
		}
	}

	return bResult;
#endif

}

IDnSkillUsableChecker* CDnRangeChecker::Clone()
{
	CDnRangeChecker* pNewChecker = new CDnRangeChecker(m_hHasActor);
	if (pNewChecker)
		pNewChecker->SetRange(GetRange());

	return pNewChecker;
}
