#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


// State effect #42: Burn
class CDnFireBurnBlow : public CDnBlow, public TBoostMemoryPool< CDnFireBurnBlow >
{
private:
	CDnState				m_State;

	//LOCAL_TIME				m_StartTime;
	//LOCAL_TIME				m_LastDamageTime;

	float m_fIntervalDamage;

	CDnActorStatIntervalManipulator	m_IntervalChecker;

	float m_fEffectLength;
	bool m_bEffecting;

	float m_fDamageProb;
	
#if defined(PRE_FIX_53266)
	float m_fAddDamageValue;	//Additional damage absolute value??
#endif // PRE_FIX_53266


public:
	CDnFireBurnBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFireBurnBlow(void);

	//virtual float GetStatDelta( void ) { return (m_fValue-m_fFireResist); };

	virtual bool OnCustomIntervalProcess( void );

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
	virtual bool CalcDuplicateValue( const char* szValue );

#if defined(PRE_ADD_SKILLBUF_RENEW)
protected:
	float CalcIntervalDamage(CDnState::ElementEnum haElement);
	virtual void OnSetParentSkillInfo();
#endif // PRE_ADD_SKILLBUF_RENEW

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
