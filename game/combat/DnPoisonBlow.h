#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


// Because probability check is included, this object is used only on the game server!
class CDnPoisonBlow : public CDnBlow, public TBoostMemoryPool< CDnPoisonBlow >
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;
	float m_fEffectLength;
	bool m_bEffecting;

#ifdef _GAMESERVER
	float m_fIntervalDamage;
#endif

	float m_fDamageProb;

#if defined(PRE_FIX_53266)
	float m_fAddDamageValue;	//Additional damage absolute value??
#endif // PRE_FIX_53266

	//int m_aiColorCustomParam[ CDnParts:: ];

//#ifndef _GAMESERVER
//	void _SetDiffuse( float fRed, float fGreen, float fBlue, float fAlpha );
//#endif

public:
	CDnPoisonBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPoisonBlow(void);
	
	//virtual bool OnCheckIntervalProb( void );
	virtual float GetStatDelta( void ) { return m_fValue; };
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
