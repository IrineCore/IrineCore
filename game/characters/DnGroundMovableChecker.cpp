#include "stdafx.h"
#include "DnGroundMovableChecker.h"
#include "DnActor.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#ifndef _GAMESERVER
bool CDnGroundMovableChecker::s_bUseSignalSkillCheck = false;
bool CDnGroundMovableChecker::s_PrimUseSignalSkillCheck = false;
LOCAL_TIME CDnGroundMovableChecker::s_EndTime = 0;
#endif


CDnGroundMovableChecker::CDnGroundMovableChecker( DnActorHandle hActor ) : IDnSkillUsableChecker( hActor )
{
	m_iType = GROUNDMOVABLE_CHECKER;
}

CDnGroundMovableChecker::~CDnGroundMovableChecker(void)
{
}


#ifndef _GAMESERVER
void CDnGroundMovableChecker::SetUseSignalSkillCheck( bool bUseSignalSkillCheck, LOCAL_TIME SignalEndTime )
{
	s_bUseSignalSkillCheck = bUseSignalSkillCheck;
	s_EndTime = SignalEndTime;
}
#endif


bool CDnGroundMovableChecker::CanUse( void )
{
	bool bResult = false;

	if( m_hHasActor )
	{	
#ifndef _GAMESERVER
		if (s_PrimUseSignalSkillCheck)
			return true;

		if( s_bUseSignalSkillCheck )
			return true;
#else
		if( m_hHasActor && m_hHasActor->IsUseSignalSkillCheck() && m_hHasActor->IsAllowedSkillCheck(2) )
			return true;
#endif
		
		if( !m_hHasActor->IsAir() )
		{
			// Skills must be usable when only root/immobility is applied, so even though it's messy, we have no choice but to check like this.. (#22550)
			if( !m_hHasActor->IsProcessSkill() && 
				!m_hHasActor->IsStun() && 
				!m_hHasActor->IsDown() && 
				(m_hHasActor->GetStateEffect() & CDnActorState::Cant_Move) == CDnActorState::Cant_Move )
			{
				bResult = true;
			}
			else if( m_hHasActor->IsMovable() )
			{
				bResult = true;
			}
		}
	}	

	return bResult;
}

IDnSkillUsableChecker* CDnGroundMovableChecker::Clone()
{
	CDnGroundMovableChecker* pNewChecker = new CDnGroundMovableChecker(m_hHasActor);

	return pNewChecker;
}