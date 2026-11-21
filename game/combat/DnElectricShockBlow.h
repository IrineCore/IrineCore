#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnCantMoveBlow;
class CDnCantActionBlow;
class CDnBasicBlow;

// Electrocution state effect. By default, causes hitstun with 15% chance every 5 seconds.
// Uses probability, so this object is only used on the game server! 
class CDnElectricShockBlow : public CDnBlow, public TBoostMemoryPool< CDnElectricShockBlow >
{
private:
	LOCAL_TIME				m_StartTime;
	LOCAL_TIME				m_LastCheckTime;
	LOCAL_TIME				m_LastStiffStartTime;
	bool					m_bStiff;
	bool					m_bAirStiff;
	bool					m_bMyLocalPlayer;

	// Added light element adjustment value. (#16000)
	float					m_fAddSuccessProb;
	float					m_fLightResistDelta;
	float					m_fDamageProb;
	float					m_fIntervalDamage;

	CDnCantMoveBlow*		m_pCantMove;
	CDnCantActionBlow*		m_pCantAction;
	CDnBasicBlow*			m_pLightDefenseBlow;

	// #27679 Effect is applied to named, boss, and 8-player nest bosses, but the additional effect of freeze is not actually applied.
	bool					m_bBossMonster;
	bool					m_bNestMap;

#if defined(_GAMESERVER)
protected:

	float CalcIntervalDamage(CDnState::ElementEnum haElement);
	virtual void OnSetParentSkillInfo();
#endif	// #if defined(_GAMESERVER)

public:
	CDnElectricShockBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnElectricShockBlow(void);

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnCmdActionFromPacket( const char* pActionName );

	virtual bool CalcDuplicateValue( const char* szValue );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

#if defined(PRE_FIX_51048)
public:
	void RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta);
#endif // PRE_FIX_51048
};
