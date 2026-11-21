#include "stdafx.h"
#include "DnEnergyChargeChecker.h"
#include "DnActor.h"
#include "DnBlow.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif


CDnEnergyChargeChecker::CDnEnergyChargeChecker( DnActorHandle hActor ) : IDnSkillUsableChecker( hActor )
{
	m_iType = ENERGY_CHARGE_CHECKER;
}

CDnEnergyChargeChecker::~CDnEnergyChargeChecker(void)
{

}


bool CDnEnergyChargeChecker::CanUse( void )
{
	bool bResult = false;

	int iNumAppliedStateBlow = m_hHasActor->GetNumAppliedStateBlow();
	for( int iBlow = 0; iBlow < iNumAppliedStateBlow; ++iBlow )
	{
		DnBlowHandle hBlow = m_hHasActor->GetAppliedStateBlow( iBlow );
		if( STATE_BLOW::BLOW_072 == hBlow->GetBlowIndex() )
		{
			bResult = true;
			break;
		}
	}

	return bResult;
}

IDnSkillUsableChecker* CDnEnergyChargeChecker::Clone()
{
	CDnEnergyChargeChecker* pNewChecker = new CDnEnergyChargeChecker(m_hHasActor);

	return pNewChecker;
}
