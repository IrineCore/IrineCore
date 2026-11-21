#pragma once
#include "DnBlow.h"

/********************************************************************
	created:	2011/02/11
	author:		semozz
	
purpose: Restricts usage to only specific skills
*********************************************************************/
class CDnAllowedSkillsBlow : public CDnBlow, public TBoostMemoryPool< CDnAllowedSkillsBlow >
{
protected:
	

public:
	CDnAllowedSkillsBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAllowedSkillsBlow(void);

private:
	typedef std::map<int, int> ALLOW_SKILL_LIST;
	ALLOW_SKILL_LIST m_AllowSkillList;

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	//Process invincible target skill IDs configured as string (##;##;...;)
	void AddAllowSkills(const char* szValue);

	//Add invincibility setting for nSkillID.
	void AddAllowSkill(int nSkillID);

	//Check if invincibility is set for nSkillID
	bool IsAllowSkill(int nSkillID);

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
