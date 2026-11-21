#pragma once
#include "DnBlow.h"


class CDnPushBlow : public CDnBlow, public TBoostMemoryPool< CDnPushBlow >
{
public:
	CDnPushBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPushBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	//virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
	virtual void OnTargetHit( DnActorHandle hTargetActor );

	void PushActor(DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam);
#endif

	enum ePushType
	{
		KnockBack = 0,
		Push = 1,
	};

	enum eTriggerType
	{
		OnDefense = 0,
		OnHit = 1,
	};

	static EtVector3 s_vTestVelocity;
	static EtVector3 s_vTestResistance;
protected:
	void SetInfo(const char* szValue);

	EtVector3 m_vVelocity[2];
	EtVector3 m_vResistance[2];
	std::string m_ActionName[2];

	ePushType m_PushType;
	eTriggerType m_TriggerType;

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
