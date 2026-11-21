#pragma once
#include "DnBlow.h"

// Team inversion state effect
//Effect index: 86
//Effect value: Not numeric
//Effect description:
//Forcibly inverts the team attribute during the effect duration.
//Allies become enemies, enemies become allies
class CDnReverseTeamBlow : public CDnBlow
{
public:
	// Related to #13885. If hit signal invert type, targets of all hit signals are inverted.
	// For targets affected by this effect, TargetType and DamageType of all hit signals in used projectiles and actions are inverted.
	// Enemy -> Friend
	// Friend -> Enemy
	// Not changed when set to all.
	// Will be used by Harpy Black (1197) appearing in late Saint Haven.
	enum
	{
		HIT_SIGNAL_TARGET_CHANGE,
		TEAM_NUMBER_CHANGE,
		HIT_SIGNAL_TARGET_ALL,			// Hits everyone regardless of friend or foe. (#24337)
	};

private:
	int m_iType;


public:
	CDnReverseTeamBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnReverseTeamBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	int GetType( void ) { return m_iType; };

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
