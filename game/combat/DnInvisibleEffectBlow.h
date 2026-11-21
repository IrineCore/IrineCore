#pragma once
#include "DnBlow.h"

// Skill effect ID: 298
// Swaps player actor to single-skin actor ID during invisibility/effect state
class CDnInvisibleEffectBlow : public CDnBlow, public TBoostMemoryPool< CDnInvisibleEffectBlow >
{
protected:
	int m_nTransformActorID;

public:
	CDnInvisibleEffectBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnInvisibleEffectBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
