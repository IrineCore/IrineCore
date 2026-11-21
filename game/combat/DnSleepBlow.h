#pragma once
#include "DnBlow.h"


class CDnCantMoveBlow;
class CDnCantActionBlow;

// State effect #45: Sleep
class CDnSleepBlow : public CDnBlow, public TBoostMemoryPool< CDnSleepBlow >
{
private:
	CDnCantMoveBlow*	m_pCantMoveBlow;
	CDnCantActionBlow*	m_pCantActionBlow;
	LOCAL_TIME			m_StartTime;
	bool				m_bAcceptHit;

#ifndef _GAMESERVER
	DnEtcHandle			m_hEffect;
#endif

public:
	CDnSleepBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSleepBlow(void);

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

#ifdef _GAMESERVER
	virtual bool OnDefenseAttack( DnActorHandle hActor,CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
