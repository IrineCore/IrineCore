#pragma once
#include "DnBlow.h"

//Effect Index : 68
//
//Value Type : float
//
//Setting: When using an MP-consuming skill, recharge or consume MP by <effect value 2>*<MP used> at the probability set in <effect value 1>.
//
//	 Usage: 
//- Reduces MP consumption of a used skill at a certain probability via buff or passive skill, or
//- Can apply a debuff to increase the target's MP consumption. (expected)
class CDnPayBackMPBlow : public CDnBlow, public TBoostMemoryPool< CDnPayBackMPBlow >
{
private:
	float m_fApplyProb;
	float m_fConsumeDecreaseProb;

#ifndef _GAMESERVER
	bool m_bShowActivateEffect;
	LOCAL_TIME m_ShowActivateEffectTime;
	LOCAL_TIME m_PrevFrameLocalTime;
#endif

public:
	CDnPayBackMPBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPayBackMPBlow();

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

#ifdef _GAMESERVER
	virtual int OnUseMP( int iMPDelta );
#else
	void OnActivate( void );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
