
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateCompleted:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateCompleted>
{
public:

	CFarmAreaStateCompleted( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::COMPLETED; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// CheckState
	virtual bool					CanStartHarvest(){ return true; }			// State where harvest can be performed?
};
