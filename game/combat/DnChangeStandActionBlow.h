#pragma once
#include "DnBlow.h"

class CDnCantMoveBlow;


//Effect to alter basic actions and give the player a different gameplay feel.
//High priority
//
//Effect index: 121
//Effect details
//During the duration, change the character's default Stand action to the Ani specified in the character Act file.
//Enter the target action name to change to in the parameter.

// Start and end actions must exist; won't crash if missing, just asserts. 
// It looks unnatural without start/end actions. 
// ChangeActionSet state effect differs in that it unconditionally replaces only the action name.
class CDnChangeStandActionBlow : public CDnBlow, public TBoostMemoryPool< CDnChangeStandActionBlow >
{
private:
	string m_strActionPrefixName;
	string m_strStartActionName;
	string m_strLoopActionName;
	string m_strEndActionName;
	bool m_bIgnoreEndAction;
	
public:
	CDnChangeStandActionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeStandActionBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	// #30882 In some cases, the end action is arbitrarily changed via a signal.
	void SetEndAction( const char* pEndAction );

#ifdef _GAMESERVER
	// Whether the action is derived from a stand action change.
	bool IsChangeActionSet( const char* szActionName );
	static void ReleaseStandChangeSkill( DnActorHandle hActor, bool bCheckUnRemovableSkill, const char* szEndAction = NULL );
	static bool CheckUsableAction( DnActorHandle hActor, bool bCheckUnRemovableSkill, const char* szActionName );
#endif
	
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
