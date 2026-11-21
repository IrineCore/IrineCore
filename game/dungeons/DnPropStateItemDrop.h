#pragma once
#include "DnPropState.h"

// Represents the state of dropping an item..
class CDnPropStateItemDrop : public CDnPropState, public TBoostMemoryPool< CDnPropStateItemDrop >
{
private:
	DnActorHandle m_hDropItemOwner;

public:
	CDnPropStateItemDrop( DnPropHandle hEntity );
	virtual ~CDnPropStateItemDrop(void);

	virtual bool OnEntry( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnLeave( LOCAL_TIME LocalTime, float fDelta );

	void SetDropItemOwner( DnActorHandle hOwner ) { m_hDropItemOwner = hOwner; };
};
