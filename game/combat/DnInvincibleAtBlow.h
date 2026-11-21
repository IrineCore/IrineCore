#pragma once
#include "DnBlow.h"

/********************************************************************
	created:	2010/11/22
	author:		semozz
	
purpose: Invincibility state against specific skills
*********************************************************************/
class CDNInvincibleAtBlow : public CDnBlow, public TBoostMemoryPool< CDNInvincibleAtBlow >
{
protected:
	bool m_bSendResist;

public:
	CDNInvincibleAtBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDNInvincibleAtBlow(void);

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
	bool IsEnableSendResist();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
