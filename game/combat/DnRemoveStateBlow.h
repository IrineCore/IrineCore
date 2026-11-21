#pragma once
#include "DnBlow.h"

class CDnRemoveStateBlow : public CDnBlow, public TBoostMemoryPool< CDnRemoveStateBlow >
{
private:
	std::map<int, int> m_SkillIDList;
	
	void SetInfo(const char* szValue);
	
#if defined(_GAMESERVER)
	void RemoveStateBlowBySkillID();
	bool FindSkillID(int nSkillID);
	void FindSkillBlowList(DnActorHandle hActor, DNVector(DnBlowHandle)& vlBlowList);
#endif // _GAMESERVER

public:
	CDnRemoveStateBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnRemoveStateBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
