#pragma once
#include "DnBlow.h"

// =============================================================================
// Post-Cap 60 / Steam NA v353 Combat State Blows (BLOW_282 .. BLOW_314)
// Exact RTTI class names & emBlowIndex IDs matched to DragonNest_dump.exe
// =============================================================================

// BLOW_282: Force critical damage suffered by target
class CDnSufferForceCirticalDamage : public CDnBlow, public TBoostMemoryPool< CDnSufferForceCirticalDamage >
{
public:
	CDnSufferForceCirticalDamage( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSufferForceCirticalDamage( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_283: Trigger sub-skill on target hit
class CDnSubSkillOnTargetHitBlow : public CDnBlow, public TBoostMemoryPool< CDnSubSkillOnTargetHitBlow >
{
protected:
	int m_nSkillID;
	float m_fProb;
public:
	CDnSubSkillOnTargetHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSubSkillOnTargetHitBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetTriggerSkillID( void ) const { return m_nSkillID; }
	float GetTriggerProb( void ) const { return m_fProb; }
};

// BLOW_284: Time-goes-growing progressive buff/stacking effect
class CDnTimeGoesGrowingBlow : public CDnBlow, public TBoostMemoryPool< CDnTimeGoesGrowingBlow >
{
protected:
	float m_fInterval;
	float m_fElapsed;
	int m_nMaxStack;
	int m_nCurStack;
public:
	CDnTimeGoesGrowingBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnTimeGoesGrowingBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetCurrentStack( void ) const { return m_nCurStack; }
};

// BLOW_285: Trigger callback/effect when a skill begins (SB_ONBEGIN_SKILL = 0x100)
class CDnDoWhenSkillBeginBlow : public CDnBlow, public TBoostMemoryPool< CDnDoWhenSkillBeginBlow >
{
protected:
	int m_nActionType;
	int m_nParam;
public:
	CDnDoWhenSkillBeginBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDoWhenSkillBeginBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_286: Check immune-be-damage state effect (e.g. Breaking Point)
class CDnCheckImmuneBeDamage : public CDnBlow, public TBoostMemoryPool< CDnCheckImmuneBeDamage >
{
protected:
	float m_fProb;
	int m_nAddBlowIndex;
public:
	CDnCheckImmuneBeDamage( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCheckImmuneBeDamage( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_288: Add state effect on target hit
class CDnAddStateOnTargetHitBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnTargetHitBlow >
{
protected:
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnTargetHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnTargetHitBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#endif
};

// BLOW_289: Add state effect on skill begin
class CDnAddStateOnSkillBeginBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnSkillBeginBlow >
{
protected:
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnSkillBeginBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnSkillBeginBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_290: Add state effect when summoning a monster
class CDnAddStateOnSummonMonsterBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnSummonMonsterBlow >
{
protected:
	int m_nMonsterID;
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnSummonMonsterBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnSummonMonsterBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_292: Add weapon-based bonus damage
class CDnAddWeaponBasedDamageBlow : public CDnBlow, public TBoostMemoryPool< CDnAddWeaponBasedDamageBlow >
{
protected:
	float m_fDamageRatio;
public:
	CDnAddWeaponBasedDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddWeaponBasedDamageBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	float GetDamageRatio( void ) const { return m_fDamageRatio; }
};

// BLOW_293: Add damage scaled by distance to target
class CDnAddDamageByTargetDistanceBlow : public CDnBlow, public TBoostMemoryPool< CDnAddDamageByTargetDistanceBlow >
{
protected:
	float m_fMinDistance;
	float m_fMaxDistance;
	float m_fAddDamageRatio;
public:
	CDnAddDamageByTargetDistanceBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddDamageByTargetDistanceBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	float CalcDistanceDamageRatio( float fDistance ) const;
};

// BLOW_294: Add damage scaled by target weight
class CDnAddDamageByWeightBlow : public CDnBlow, public TBoostMemoryPool< CDnAddDamageByWeightBlow >
{
protected:
	float m_fWeightRatio;
public:
	CDnAddDamageByWeightBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddDamageByWeightBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_295: Add state effect on breaking super armor
class CDnAddStateOnBreakSuperArmorBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnBreakSuperArmorBlow >
{
protected:
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnBreakSuperArmorBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnBreakSuperArmorBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_296: Add state effect when appointed skill is used
class CDnAddStateOnAppointedSkillBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnAppointedSkillBlow >
{
protected:
	int m_nSkillID;
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnAppointedSkillBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnAppointedSkillBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetAppointedSkillID( void ) const { return m_nSkillID; }
};

// BLOW_297: Add state effect when appointed skill hits target
class CDnAddStateOnAppointedSkillHitBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnAppointedSkillHitBlow >
{
protected:
	int m_nSkillID;
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnAppointedSkillHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnAppointedSkillHitBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetAppointedSkillID( void ) const { return m_nSkillID; }
};

// BLOW_299: Transfer incoming damage to summoned monster
class CDnDamageTransitionToSummonMonsterBlow : public CDnBlow, public TBoostMemoryPool< CDnDamageTransitionToSummonMonsterBlow >
{
protected:
	float m_fTransitionRatio;
	int m_nSummonMonsterID;
public:
	CDnDamageTransitionToSummonMonsterBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDamageTransitionToSummonMonsterBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	float GetTransitionRatio( void ) const { return m_fTransitionRatio; }
};

// BLOW_300: Add state effect on successful block
class CDnAddStateOnBlockBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnBlockBlow >
{
protected:
	int m_nStateEffectIndex;
	int m_nDurationTime;
	std::string m_strStateParam;
public:
	CDnAddStateOnBlockBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnBlockBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_301: Gauge shield (absorbs damage up to shield gauge value)
class CDnGaugeShieldBlow : public CDnBlow, public TBoostMemoryPool< CDnGaugeShieldBlow >
{
protected:
	INT64 m_nMaxGauge;
	INT64 m_nCurGauge;
public:
	CDnGaugeShieldBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnGaugeShieldBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif
	INT64 GetCurGauge( void ) const { return m_nCurGauge; }
	INT64 GetMaxGauge( void ) const { return m_nMaxGauge; }
};

// BLOW_303: Duplicate explosion state effect
class CDnDuplicateExplosionBlow : public CDnBlow, public TBoostMemoryPool< CDnDuplicateExplosionBlow >
{
protected:
	int m_nTargetBlowIndex;
	float m_fDamageRatio;
public:
	CDnDuplicateExplosionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDuplicateExplosionBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};

// BLOW_314: Decrease super armor
class CDnDecreaseSuperArmorBlow : public CDnBlow, public TBoostMemoryPool< CDnDecreaseSuperArmorBlow >
{
protected:
	float m_fDecreaseRatio;
public:
	CDnDecreaseSuperArmorBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDecreaseSuperArmorBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
