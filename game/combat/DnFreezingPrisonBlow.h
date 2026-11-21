#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnCantMoveBlow;
class CDnCantActionBlow;
class CDnFrameStopBlow;

//Description
//Target: cannot take any action.
//Release: Debuff is removed when nearby party members attack the ice attached to the target and reduce its durability to 0 or below.
//Duration: As soon as debuff duration exceeds 0, the debuff state is cleared while dealing final damage to the target.
//
//	   1. Durability: enter durability HP value
//
//	   2. Final damage: requires value input for what % of physical/magic attack power to deal as damage
//	   e.g. P30
//	   M50
//	   10000 (flat damage value)
//	   3. Damage per sec: deals damage per second for duration to debuffed target. Input method same as above
//
//	   4. Duration: Same as existing duration
//	   5. Effect: Show frozen state while debuffed
//	   Two damage visual representations are needed when dealing final damage
class CDnFreezingPrisonBlow : public CDnBlow, 
							  public TBoostMemoryPool< CDnFreezingPrisonBlow >
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;

	CDnCantMoveBlow*		m_pCantMoveBlow;
	CDnCantActionBlow*		m_pCantActionBlow;
	CDnFrameStopBlow*		m_pFrameStopBlow;

	float m_fIntervalDamage;
	int m_iDurability;				// Durability. State effect is removed when depleted.
	bool m_bDurabilityDestroyed;	// Destroyed due to depleted durability.
	int m_iFinalDamage;				// Deals specified damage if duration expires before being destroyed.
	int m_iOriginalTeam;			// Original team of the actor on which this state effect is active.

	float m_fOrigWight;				// Character's default Weight value
	int m_iOrigPressLevel;			// Character's default PressLevel value
	int m_iMaxDurability;			// Initial durability value for calculating durability percentage

protected:
#ifdef _GAMESERVER
	void _RequestDamage( float fDamage );
#endif

public:
	CDnFreezingPrisonBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFreezingPrisonBlow(void);

	virtual bool OnCustomIntervalProcess( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if !defined(_GAMESERVER)
	void OnHit();
	EtVector3 GetGaugePos();
#endif

#ifdef _GAMESERVER
	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
