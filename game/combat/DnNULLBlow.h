#pragma once
#include "DnBlow.h"


// A no-op state effect that only displays an effect.
class CDnNULLBlow : public CDnBlow, public TBoostMemoryPool< CDnNULLBlow >
{
protected:
#ifdef _CLIENT
	bool m_bUseCommonEffect;
	std::string m_strCommonEffectName;
#endif
	
public:
	CDnNULLBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnNULLBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
