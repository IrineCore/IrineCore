#pragma once
#include "DnBlow.h"


// State effect that prevents acting. Cannot use skills, attack, etc.
class CDnCantActionBlow : public CDnBlow, public TBoostMemoryPool< CDnCantActionBlow >
{
private:
	string m_strActionWhenCancelAttack;

public:
	CDnCantActionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCantActionBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void SetActionWhenCancelAttack( const char* pActionName ) { m_strActionWhenCancelAttack.assign( pActionName ); };

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

private:
	bool m_isAvailableNormalState;
public:
	void SetAvailableNormalState(bool isAvailableNormalState) { m_isAvailableNormalState = isAvailableNormalState; }
	bool IsAvailableNormalState() { return m_isAvailableNormalState; }

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
