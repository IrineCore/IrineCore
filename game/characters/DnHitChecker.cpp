#include "stdafx.h"
#include "DnHitChecker.h"
#include "DnActor.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnHitChecker::CDnHitChecker( DnActorHandle hActor ) : IDnSkillUsableChecker( hActor )
{
	m_iType = HIT_CHECKER;
}

CDnHitChecker::~CDnHitChecker(void)
{

}


bool CDnHitChecker::CanUse( void )
{
	bool bResult = false;

	if( m_hHasActor )
	{
		bResult = m_hHasActor->IsHit();
		//if( !m_hHasActor->IsAir() &&
		//	!m_hHasActor->IsDown() &&
		//	!m_hHasActor->IsAttack() &&
		//	!m_hHasActor->IsProcessSkill() )
		//	bResult = true;
		//else
		//	bResult = false;
	}

	return bResult;
}

IDnSkillUsableChecker* CDnHitChecker::Clone()
{
	CDnHitChecker* pNewChecker = new CDnHitChecker(m_hHasActor);

	return pNewChecker;
}