#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


class CDnDOTBlow : public CDnBlow, public TBoostMemoryPool< CDnDOTBlow >
{
private:
	CDnActorStatIntervalManipulator	m_IntervalChecker;

	float m_fEffectLength;
	bool m_bEffecting;

	int m_IntervalTime;
	float m_IntervalDamage;

	int m_nDamageCount;
	//bool m_bRequestDamage;

public:
	CDnDOTBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDOTBlow(void);

	//virtual float GetStatDelta( void ) { return (m_fValue-m_fFireResist); };

	virtual bool OnCustomIntervalProcess( void );


	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual bool CalcDuplicateValue( const char* szValue );

	void RequestDamage();
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
