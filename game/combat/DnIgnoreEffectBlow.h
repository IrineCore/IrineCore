#pragma once
#include "DnBlow.h"

/********************************************************************
purpose: State effect that processes HitSignal but skips applying state effects
*********************************************************************/
class CDnIngnoreEffectBlow : public CDnBlow, public TBoostMemoryPool< CDnIngnoreEffectBlow >
{
protected:
	

public:
	CDnIngnoreEffectBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnIngnoreEffectBlow(void);

private:
	typedef std::map<int, int> INVINCIBLEAT_LIST;
	INVINCIBLEAT_LIST m_InvincibleAtList;

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	//Process invincible target skill IDs configured as string (##;##;...;)
	void AddInvincibleAt(const char* szValue);

	//Add invincibility setting for nSkillID.
	void AddInvincibleAt(int nSkillID);

	//Check if invincibility is set for nSkillID
	bool IsInvincibleAt(int nSkillID);

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
