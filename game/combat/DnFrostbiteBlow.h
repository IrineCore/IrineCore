#pragma once
#include "DnBlow.h"

class CDnFrameBlow;
class CDnBasicBlow;

// [Frostbite] - Frostbite
// Effect index: 144
// Reduces "frame/move speed" by the specified amount for the duration and additionally reduces "ice resistance"
// Stacks additively with other frame reduction or move speed increase effects (30% frame reduction + 30% frostbite = 60% frame reduction)
// Duration increases/decreases by target's "Ice Resistance" % (higher ice resistance reduces duration)
// Parameters: move speed/frame delta % ; ice resist delta %
// 
// Implement as a combination of existing state effects.
class CDnFrostbiteBlow : public CDnBlow, 
					     public TBoostMemoryPool< CDnFrostbiteBlow >
{
private:
	//CDnFrameBlow* m_pFrameBlow;
	float m_fFrameBlowArg;
	CDnBasicBlow* m_pMoveSpeedBlow;
	CDnBasicBlow* m_pIceDefenseBlow;

	// #28385 On a nest map, applied as a state effect that has no effect on bosses.
	bool m_bNestMap;

public:
	CDnFrostbiteBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFrostbiteBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

	float GetFrameBlowArg( void ) { return m_fFrameBlowArg; };

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

#if defined(PRE_FIX_51048)
public:
	void RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta);
#endif // PRE_FIX_51048
};
