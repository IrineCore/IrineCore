
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateNone:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateNone>
{
public:

	CFarmAreaStateNone( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::NONE; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// CheckState
	virtual bool					CanStartPlantSeed();							// State where seed can be planted?
};
