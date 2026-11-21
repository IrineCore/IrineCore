#pragma once
#include "DnBlow.h"
#include "DnObservable.h"

class CDnParryBlow : public CDnBlow, 
					 public TBoostMemoryPool< CDnParryBlow >,
					 // CDnPlayerActor static_casts to this class to pass Observer objects, so removing this inheritance alone would cause major issues.
					 public CDnObservable
{
private:
#ifdef _GAMESERVER
	string m_strParringActionName;
	bool m_bEnable;
	bool m_bEnableLastLoop;
	float m_fAdditionalSignalProb;		// from ParringSignal
	float m_fAdditionalSignalProbLastLoop; // from ParringSignal
#endif

public:
	CDnParryBlow(DnActorHandle hActor, const char *szValue);
	virtual ~CDnParryBlow(void);

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	bool IsEnabled( void ) { return m_bEnable; };
	void EnableParrying( float fAdditionalProb ) { m_bEnable = true; m_fAdditionalSignalProb = fAdditionalProb; };
	void SetParringActionName( const char* pParringActionName ) { _ASSERT( pParringActionName ); m_strParringActionName.assign( pParringActionName ); };

	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};