#pragma once
#include "DnBlow.h"

//Additional action state effect on stack
class CDnPileAddEffectBlow : public CDnBlow, 
								 public TBoostMemoryPool< CDnPileAddEffectBlow >
{
public:
	CDnPileAddEffectBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPileAddEffectBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
	virtual void OnSetParentSkillInfo();
	virtual void SignalProcess( LOCAL_TIME LocalTime, float fDelta );

	//Set activation state when the state effect reaches the stack count threshold.
	void SetActivateBlow(bool bActivate) { m_bActivatedBlow = bActivate; }
	bool IsActivatedBlow() { return m_bActivatedBlow; }

private:
	std::string m_szTargetAction;

	CEtActionBase::ActionElementStruct* m_pActionElement;
	float m_fFrame;
	DWORD m_ActionDurationTime;
	vector<CEtActionSignal *> m_vlpSignalList;

	bool m_bActivatedBlow;		//Activated when the configured stack count is reached..

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
