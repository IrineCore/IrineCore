#pragma once
#include "DnBlow.h"

#ifndef _GAMESERVER
#include "DnComboCalculator.h"
#endif

//Add an effect that completely stops time (sets frame rate to 0)
//Effect index: 146
//Takes highest priority among all frame-related effects (ignores other frame control effects). 
class CDnFrameStopBlow : public CDnBlow, public TBoostMemoryPool< CDnFrameStopBlow >
{
public:
	CDnFrameStopBlow(DnActorHandle hActor, const char *szValue);
	virtual ~CDnFrameStopBlow(void);

protected:

#if defined(PRE_FIX_53274)
	// Whether a player character is affected
	bool					m_bPlayerCharacter;

#ifndef _GAMESERVER
	// Escape crisis via rapid button presses
	CDnComboCalculator m_ComboCalc;
#endif // _GAMESERVER
#endif // PRE_FIX_53274

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#if defined(PRE_FIX_53274)
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
#endif // PRE_FIX_53274

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
