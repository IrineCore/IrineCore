#pragma once
#include "DnBlow.h"
#include <vector>
#include <string>

// =============================================================================
// Extended Combat State Blows (BLOW_282 .. BLOW_314)
// Full parameter parsing & combat hooks matched to GameRes DNT specs
// =============================================================================

struct SExtendedSubBlowSpec
{
	int nBlowIndex;
	std::string strParam;
	SExtendedSubBlowSpec() : nBlowIndex( 0 ) {}
	SExtendedSubBlowSpec( int idx, const std::string& param ) : nBlowIndex( idx ), strParam( param ) {}
};

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
	float m_fCoolTime;
	float m_fRemainCoolTime;
public:
	CDnSubSkillOnTargetHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSubSkillOnTargetHitBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#endif
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
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
	int m_nSubDurationMs;
	std::vector<int> m_vecAppliedServerBlowIDs;
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
	float m_fProb;
	int m_nDurationMs;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
	std::vector<int> m_vecTargetSkillIDs;
public:
	CDnDoWhenSkillBeginBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDoWhenSkillBeginBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnBeginSkill( DnSkillHandle hSkill );
#endif
};

// BLOW_286: Check immune-be-damage state effect (e.g. Breaking Point)
class CDnCheckImmuneBeDamage : public CDnBlow, public TBoostMemoryPool< CDnCheckImmuneBeDamage >
{
protected:
	float m_fProb;
	int m_nAddBlowIndex;
	int m_nMode;
public:
	CDnCheckImmuneBeDamage( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCheckImmuneBeDamage( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual bool OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif
};

// BLOW_288: Add state effect on target hit
// Supports both DNT bracket syntax "[1;-1;0;0;100;10000;0;20000;1][29/0.20]" and "blowIdx;dur;param"
class CDnAddStateOnTargetHitBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnTargetHitBlow >
{
protected:
	float m_fProb;
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	bool m_bApplySelf;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
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
	float m_fProb;
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnSkillBeginBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnSkillBeginBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnBeginSkill( DnSkillHandle hSkill );
#endif
};

// BLOW_290: Add state effect when summoning a monster
// Supports DNT bracket syntax "[32008][1;100;-1;0;0][58/1.00]" and "monID;blowIdx;dur;param"
class CDnAddStateOnSummonMonsterBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnSummonMonsterBlow >
{
protected:
	std::vector<int> m_vecMonsterIDs;
	float m_fProb;
	int m_nDurationTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnSummonMonsterBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnSummonMonsterBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	void OnSummonMonster( DnActorHandle hSummonedMonster, int nMonsterID );
#endif
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
// DNT format: "distRef;addRatio;stepDist;mode" (e.g. "300;0.10;100;0" or "0;0.10;100;0")
class CDnAddDamageByTargetDistanceBlow : public CDnBlow, public TBoostMemoryPool< CDnAddDamageByTargetDistanceBlow >
{
protected:
	float m_fRefDistance;
	float m_fAddDamageRatio;
	float m_fStepDistance;
	int m_nMode;
public:
	CDnAddDamageByTargetDistanceBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddDamageByTargetDistanceBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	float CalcDistanceDamageRatio( float fDistance ) const;
};

// BLOW_294: Add damage scaled by target weight (e.g. "5;1.0")
class CDnAddDamageByWeightBlow : public CDnBlow, public TBoostMemoryPool< CDnAddDamageByWeightBlow >
{
protected:
	float m_fWeightThreshold;
	float m_fWeightRatio;
public:
	CDnAddDamageByWeightBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddDamageByWeightBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	float CalcWeightDamageRatio( float fTargetWeight ) const;
};

// BLOW_295: Add state effect on breaking super armor
class CDnAddStateOnBreakSuperArmorBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnBreakSuperArmorBlow >
{
protected:
	float m_fProb;
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnBreakSuperArmorBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnBreakSuperArmorBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	void OnBreakSuperArmor( DnActorHandle hTargetActor );
#endif
};

// BLOW_296: Add state effect when appointed skill is used
// DNT format: "[prob;duration;cooltime][blow1/val1][blow2/val2]_[skillID1;skillID2;...]"
class CDnAddStateOnAppointedSkillBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnAppointedSkillBlow >
{
protected:
	std::vector<int> m_vecAppointedSkillIDs;
	float m_fProb;
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnAppointedSkillBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnAppointedSkillBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnBeginSkill( DnSkillHandle hSkill );
#endif
	int GetAppointedSkillID( void ) const { return m_vecAppointedSkillIDs.empty() ? 0 : m_vecAppointedSkillIDs[0]; }
};

// BLOW_297: Add state effect when appointed skill hits target
// DNT format: "[prob;duration;cooltime][blow1/val1]_[skillID1;skillID2;...]"
class CDnAddStateOnAppointedSkillHitBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnAppointedSkillHitBlow >
{
protected:
	std::vector<int> m_vecAppointedSkillIDs;
	float m_fProb;
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnAppointedSkillHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnAppointedSkillHitBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#endif
	int GetAppointedSkillID( void ) const { return m_vecAppointedSkillIDs.empty() ? 0 : m_vecAppointedSkillIDs[0]; }
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
#ifdef _GAMESERVER
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif
	float GetTransitionRatio( void ) const { return m_fTransitionRatio; }
};

// BLOW_300: Add state effect on successful block
// DNT format: "[duration;cooltime][blow1/val1]..." (e.g. "[10000;20000][2/0.20]")
class CDnAddStateOnBlockBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnBlockBlow >
{
protected:
	int m_nDurationTime;
	float m_fCoolTime;
	float m_fRemainCoolTime;
	std::vector<SExtendedSubBlowSpec> m_vecSubBlows;
public:
	CDnAddStateOnBlockBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnBlockBlow( void );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
#ifdef _GAMESERVER
	void OnBlockSuccess( void );
#endif
};

// BLOW_301: Gauge shield (absorbs damage up to shield gauge value; DNT format "mode;maxGauge;breakSkillID")
class CDnGaugeShieldBlow : public CDnBlow, public TBoostMemoryPool< CDnGaugeShieldBlow >
{
protected:
	int m_nMode;
	INT64 m_nMaxGauge;
	INT64 m_nCurGauge;
	int m_nBreakSkillID;
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
	float GetDecreaseRatio( void ) const { return m_fDecreaseRatio; }
};
