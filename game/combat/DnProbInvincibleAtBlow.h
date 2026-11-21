#pragma once
#include "DnBlow.h"


// State effect that evades a specific skill with a set probability.
// Prevents damage and state effects from being applied on hit signals of a specific skill.
class CDnProbInvincibleAtBlow : public CDnBlow,
								public TBoostMemoryPool< CDnProbInvincibleAtBlow >
{
protected:
	

public:
	CDnProbInvincibleAtBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnProbInvincibleAtBlow(void);

private:

#ifdef _GAMESERVER
	struct S_INVINCIBLE_INFO
	{
		int iMonsterID;
		map<int, float> mapSkillInfos;

		S_INVINCIBLE_INFO( void ) : iMonsterID( 0 ) {};
	};

	struct S_SKILL_INFO
	{
		DnSkillHandle hTargetSkill;
		bool bTargetSkillUsedProjectile;
		
		S_SKILL_INFO( void ) : bTargetSkillUsedProjectile( false ) {};
	};

	std::map<int, S_INVINCIBLE_INFO> m_mapInvincibleAt;
	map<int, float>* m_pNowMapSelectedSkillInfos;

	// Check whether it is in the ended state on every Process call.
	//DnSkillHandle m_hTargetSkill;
	bool m_bNowSelectedSkillUsedProjectile;
	list<S_SKILL_INFO> m_listTargetSkills;
	map<int, bool> m_mapTargetSkillUsedProjectile;
	deque<DnSkillHandle> m_dqProbInvincibleFailedSkills;
#endif // #ifdef _GAMESERVER

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void AddInvincibleAt(const char* szValue);

#ifdef _GAMESERVER
private:
	void CheckAllTargetSkillState( void );
	void CheckTargetSkillStateAndDoEnd( DnSkillHandle hTargetSkill );
	void GatherEndTargetState( /*IN OUT*/ vector<DnSkillHandle>& vlhEndedTargetSkill );
	void CheckProbFailedSkillState( void );
	bool IsSkillFinished( DnSkillHandle hSkill );
	bool IsProbFailedSkillObject( DnSkillHandle hSkill );

public:
	//Check if invincibility is set for nSkillID
	bool IsInvincibleAt( DnSkillHandle hTargetSkill );
	bool IsBeginInvincible( int iSkillID );
	S_SKILL_INFO* FindInvincibleTargetSkill( int iSkillID );
	void BeginInvincibleAt( DnSkillHandle hTargetSkill );
	void EndInvincibleAt( DnSkillHandle hTargetSkill );

	// If the damage is from a specific skill, set the damage to 0, 
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

	// Prevent state effects from applying while invincibility against a specific skill is active.
	bool CanAddThisSkillsStateBlow( int iSkillID );

	// Handle so that the resulting state blow is not applied either..
#else
	void OnSuccess( void );
#endif // _GAMESERVER

//#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
//public:
//	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
//	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
//#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
