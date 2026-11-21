#pragma once
#include "DnBlow.h"
#include "DnObservable.h"

// State effect that unconditionally guards for a set number of hits
class CDnBlockBlow : public CDnBlow,
					 public TBoostMemoryPool< CDnBlockBlow >,
					 // CDnPlayerActor static_casts to this class to pass Observer objects, so removing this inheritance alone would cause major issues.
					 public CDnObservable 
					 //////////////////////////////////////////////////////////////////////////
{
private:
	int					m_iCanBlockCount;

public:
	CDnBlockBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnBlockBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
