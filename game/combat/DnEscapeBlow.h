#pragma once
#include "DnBlow.h"
#ifndef _GAMESERVER
#include "DnComboCalculator.h"
#endif

#include "DnActorStatIntervalManipulator.h"

class CDnCantMoveBlow;
class CDnCantActionBlow;
class CDnFrameStopBlow;


// Freeze state effect
class CDnEscapeBlow : public CDnBlow, public TBoostMemoryPool< CDnEscapeBlow >
{
private:
	//////////////////////////////////////////////////////////////////////////
	//For incapacitated state
	CDnCantMoveBlow*		m_pCantMoveBlow;
	CDnCantActionBlow*		m_pCantActionBlow;
	CDnFrameStopBlow*		m_pFrameStopBlow;
	//////////////////////////////////////////////////////////////////////////

	DWORD					m_dwOriSpeedFrame;
	///float					m_fOriFPS;

// 	// Durability reduced by hit count
// 	int						m_iDurabilityCount;

	// Whether a player character is affected
	bool					m_bPlayerCharacter;
	

	bool m_bAllowShakeControl;		//Shake? flag
	std::string m_strHitedAction;	//Hit action

	bool m_isAbleIntervalChecker;
	CDnActorStatIntervalManipulator	m_IntervalChecker;

	float m_fOrigWight;
	int m_iOrigPressLevel;

	// Escape crisis via rapid button presses
#ifndef _GAMESERVER
	CDnComboCalculator m_ComboCalc;

	vector<string> m_vlBreakActions;
	bool m_bShake;
	LOCAL_TIME m_ShakeStartTime;

	void _ProcessShake( LOCAL_TIME LocalTime, float fDelta );
#endif

	// Whether current map is a Nest.
	bool m_bNestMap;

public:
	CDnEscapeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnEscapeBlow(void);

public:
#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	//virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

	virtual bool OnCustomIntervalProcess( void );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

protected:
	void CheckBossMonster();

#if defined(PRE_FIX_51048)
public:
	void RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta);
#endif // PRE_FIX_51048
};
