#pragma once
#include "DnBlow.h"


class CDnPartyAddDamageBlow : public CDnBlow, public TBoostMemoryPool< CDnPartyAddDamageBlow >
{
public:
	CDnPartyAddDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPartyAddDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

protected:
	int m_nAddDamage;
	CDnSkill::SkillInfo m_sDamageActorSkillInfo;
#endif

};
