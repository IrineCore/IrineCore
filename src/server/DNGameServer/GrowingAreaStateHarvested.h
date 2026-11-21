
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateHarvested:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateHarvested>
{
public:

	CFarmAreaStateHarvested( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
		m_biHarvestedCharacterDBID = 0;
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::HARVESTED; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// HarvestedInfo
	void	SetHarvestedInfo( CDNUserSession* pSession );

protected:

	INT64	m_biHarvestedCharacterDBID;
};
