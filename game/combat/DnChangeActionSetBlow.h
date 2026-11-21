#pragma once
#include "DnBlow.h"

class CDnChangeActionStrProcessor;

//
//Adding a skill that enhances basic attacks as an additional Cleric-Paladin skill.
//Additionally, a new required skill effect was introduced.
//Same concept as changing the existing stand action
//However, an action to return back like end is not needed.
//
//Change normal attack
//Changes the character's normal attack to a specific designated action.
//Functionally possible if only the Attack1 action can be modified to another action during the duration
//(After this, connects to input action)
//Must take priority over modifying per-weapon actions to other actions
//Effect index: 129 (Attack_Change)
//Skill name: Sacred Hammering - Adds bonus damage to normal attacks during the duration.

// 129) State effect that changes action name in CDnActor::SetActionQueue using action-replace skill processor.
class CDnChangeActionSetBlow : public CDnBlow, public TBoostMemoryPool< CDnChangeActionSetBlow >
{
private:
	string m_strTriggerAction;
	string m_strCancleAction;
	bool m_bEnable;
	CDnChangeActionStrProcessor* m_pChangeActionStrProcessor;

	bool m_bChangedActionBegun;

public:
	CDnChangeActionSetBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeActionSetBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void UpdateEnable( const char* pPrevAction, const char* pCurrentAction );
	CDnChangeActionStrProcessor* GetChangeActionStrProcessor( void );

#ifdef _GAMESERVER
	void OnChangeAction( void );
	void OnNotChangeAction( void );
#endif

#if defined(PRE_FIX_68645)
	bool IsEnable() { return m_bEnable; }
#endif // PRE_FIX_68645

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
