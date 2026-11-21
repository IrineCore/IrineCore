#pragma once
#include "DnBlow.h"

/*
Issue #16331

Requesting because a new skill effect is needed.

<Situations Requiring Skill Effects>
When Asai Shaman casts a buff on an ally
When buffed monsters reach 0 HP
Dies while casting the skill (e.g., Charge Bolt).

<Effect Definition>
As a buff-type skill
When HP reaches 0 while this buff is active
Casts a skill upon dying.

<Development Request Details>
1. Add a new effect to SkillTable's effect class (_EffectClass) (e.g., DieSkill)
2. Allow specifying the skill index and skill level to use in SKillLevelTable's effect value (_EffectClassValue).

Details the design team asked verbally are as follows.

- Monsters with this state effect have a 100% chance to use a skill upon dying.
- When using a skill while dying, becomes invincible just like normal death.
- Currently no skill performs a specific action on death, though one may be added later.
- Currently Slow, Freeze, and Charge Bolt lightning are considered.
*/

class CDnInvincibleBlow;

class CDnUsingSkillWhenDieBlow : public CDnBlow
{
private:
	bool m_bDead;					// In dead state. OnDie has been called.
	bool m_bUsedDieSkill;			// Skill was used on death.

public:
	CDnUsingSkillWhenDieBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnUsingSkillWhenDieBlow( void );

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void OnDie( void );

	// Whether the skill fired upon death.
	bool IsUsedSkill( void ) { return m_bUsedDieSkill; };

#if defined(_GAMESERVER)
#else
	//#44884 Send a packet to sync when the flag is set on the server due to server/client sync issue.
	void ShowGraphicEffect(bool bShow) { 
		m_bUsedDieSkill = bShow; 
	}
#endif // _GAMESERVER

#if defined(PRE_FIX_44884)
protected:
	DnActorHandle m_FinalHitterActor;		//Save the actor that called Die..
public:
	void SetFinalHitterActor(DnActorHandle hHitter) { m_FinalHitterActor = hHitter; }
#endif // PRE_FIX_44884


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
