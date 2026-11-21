#pragma once
#include "DnPropState.h"

// Only accepts a pointer to DnWorldActProp...
class CDnPropStateNormal : public CDnPropState, public TBoostMemoryPool< CDnPropStateNormal >
{	
private:
	

public:
	CDnPropStateNormal( DnPropHandle hEntity );
	virtual ~CDnPropStateNormal(void);

	virtual bool OnEntry( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnLeave( LOCAL_TIME LocalTime, float fDelta );
};
