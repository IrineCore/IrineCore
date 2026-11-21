#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


class CDnCurseBlow : public CDnBlow, public TBoostMemoryPool< CDnCurseBlow >
{
private:
	float m_fDamageRate;		// Attack power (excluding skill bonuses) % damage
	float m_fDamage;			// Absolute damage value?
	float m_fDelayTime;			// Time for self cooldown?
	
	float m_fCoolTime;			// Current cooldown? Remaining time?
	bool m_bAtivate;			//Enable damage calculation?
	int m_nDamage;
public:
	CDnCurseBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCurseBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual bool CalcDuplicateValue( const char* szValue );
	void OnAttackChange();
	float GetCoolTime() { return m_fCoolTime; }
	
protected:
	void RequestDamage();
	int CalcDamage();

	DnActorHandle m_hSkillUser;
#if defined(PRE_ADD_50903)
protected:
	virtual void OnSetParentSkillInfo();
#endif // PRE_ADD_50903

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
