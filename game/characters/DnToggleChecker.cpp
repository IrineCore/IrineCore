#include "stdafx.h"
#include "DnToggleChecker.h"
#include "DnActor.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif


CDnToggleChecker::CDnToggleChecker( DnActorHandle hActor, int iToggleSkillID ) : IDnSkillUsableChecker( hActor ), m_iRequireToggleSkillID( iToggleSkillID )
																				 
{

}

CDnToggleChecker::~CDnToggleChecker(void)
{

}


bool CDnToggleChecker::CanUse( void )
{
	bool bResult = false;

	DnSkillHandle hToggledSkill = m_hHasActor->GetEnabledToggleSkill();
	if( hToggledSkill )
	{
		bResult = (m_iRequireToggleSkillID == hToggledSkill->GetClassID());
	}

	return bResult;
}

IDnSkillUsableChecker* CDnToggleChecker::Clone()
{
	CDnToggleChecker* pNewChecker = new CDnToggleChecker(m_hHasActor, m_iRequireToggleSkillID);

	return pNewChecker;
}

