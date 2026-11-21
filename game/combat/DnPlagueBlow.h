#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


// Contagious disease state effect.
//Effect Index : 136
//
//Effect Name : DISEASE
//
//Effect description
//1) Inflicts "Contagion" debuff when hit by an attack with the contagion attribute.
//
//2) Deals <final damage calculated by the skill> * <effect value (%)> damage for <duration> to self when hit and allies within <specified distance>.
//* Damage is the same as the currently implemented <Poison>.
//
//3) Transfers plague to damaged allies with <specified probability>.
//
//Effect exception handling 
//1) On death, points are awarded to the "caster", same as Charger or Ignition.
//
//Usage purpose
//Induce behavior to increase distance between users
class CDnPlagueBlow : public CDnBlow
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;
	float m_fDamageProb;
	float m_fDamageAbsolute;
	float m_fPlagueRange;		// If not needed on client, remove later.
	float m_nPlageProbability;

	float m_fEffectLength;
	bool m_bEffecting;

#ifdef _GAMESERVER
	float m_fIntervalDamage;
#endif

public:
	CDnPlagueBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPlagueBlow(void);

	virtual float GetStatDelta( void ) { return m_fValue; };
	virtual bool OnCustomIntervalProcess( void );

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
