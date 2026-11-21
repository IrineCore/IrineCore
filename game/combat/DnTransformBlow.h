#pragma once
#include "DnBlow.h"


class CDnTransformBlow : public CDnBlow, public TBoostMemoryPool< CDnTransformBlow >
{
protected:
	int m_nTransformIndex;
	std::string m_strEndActionName;
	bool m_bSkillDefendency;

public:
	CDnTransformBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnTransformBlow(void);
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	
	bool CheckSkillDefendency() { return m_bSkillDefendency; }
};
