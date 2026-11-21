#pragma once
#include "DnBlow.h"

// 179) Transitions to the action defined by the state blow on the first hit to a target after this state blow is applied.
// Hurricane Dance (automatically transitions to next action when attack in the skill's first action hits.)
class CDnActionChangeOnHitBlow : public CDnBlow, 
								 public TBoostMemoryPool< CDnActionChangeOnHitBlow >
{
private:
	string m_strActionNameToChange;
#if defined(PRE_FIX_55461)
	string m_strLimitAction;	//Restricted to trigger on OnTargetHit only when the actor's action matches
#endif // PRE_FIX_55461


public:
	CDnActionChangeOnHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnActionChangeOnHitBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#endif // #ifdef _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
