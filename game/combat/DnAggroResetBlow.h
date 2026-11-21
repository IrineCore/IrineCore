#pragma once
#include "DnBlow.h"

// Team inversion state effect
//Effect index: 86
//Effect value: Not numeric
//Effect description:
//Forcibly inverts the team attribute during the effect duration.
//Allies become enemies, enemies become allies
class CDnAggroResetBlow : public CDnBlow
{
public:
	

private:
	int m_iType;


public:
	CDnAggroResetBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAggroResetBlow(void);

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
