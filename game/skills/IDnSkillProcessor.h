#pragma once

// Abstraction of situations to handle such as abnormal states, etc.
class IDnSkillProcessor
{
public:
	//// Classification by state effect target
	//enum
	//{
	//	STATE_EFFECT_SELF,
	//	STATE_EFFECT_TARGET,
	//};
	enum
	{
		PLAY_ANI,
		PARTIAL_PLAY_ANI,
		CHANGE_PROJECTILE,
		DIVIDE_STATE_EFFECT_ARG,
		IMPACT_BLOW,
		AURA,
		CHANGE_ACTIONSTR,
		RANGE_STATE_EFFECT,
		APPLY_SE_WHEN_ACTIONSET_ENABLED,			// Apply skill state effects only when ActionProcessor is activated by ChangeActionSet state effect. (#21052)
		APPLY_SE_WHEN_TARGET_NORMAL_HIT,			// Activation type that applies skill Target state effects with a probability to targets hit by normal attack. Currently used only in item prefix system. (#23818)
		CALL_MY_SUMMONED_MONSTER,					// Skill that summons a monster summoned by self.
		CHANGE_ACTIONSTR_BY_BUBBLE,					// Skill action changes based on bubble count.
		STATE_EFFECT_APPLY_ONOFF_BY_BUBBLE,			// Set valid state effect index based on bubble count.
		PLAY_PET_ANI,								// Play pet animation
		COOLTIME_CHARGING = 14,						// Skill cooldown charging stack processor (CDnCoolTimeChargingProcess)
		ASSAULT = 15,								// Assault movement & hit processor (CDnAssaultProcessor)
		CHECK_KEY = 16,
		COUNT,
	};

protected:
	DnActorHandle					m_hHasActor;
	DnSkillHandle					m_hParentSkill;
	float							m_fTimeLength;
	int								m_iType;

protected:
	IDnSkillProcessor( DnActorHandle hActor );

public:
	IDnSkillProcessor( void ) : m_fTimeLength( 0.0f ), m_iType( 0 ) {};
	virtual ~IDnSkillProcessor( void );

	virtual void SetHasActor( DnActorHandle hActor ) { m_hHasActor = hActor; };
	virtual void SetParentSkill( DnSkillHandle hSkill ) { m_hParentSkill = hSkill; };

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill ) = 0;
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) = 0;
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta ) = 0;

	// Basic query function for whether processing has finished
	virtual bool IsFinished( void ) { return (m_fTimeLength <= 0.0f); };

	virtual int GetNumArgument( void ) = 0;
	virtual void CopyFrom( IDnSkillProcessor* pProcessor ) {};

	int GetType( void ) { return m_iType; };

	// static factory
	static IDnSkillProcessor* Create( DnActorHandle hActor, int iProcessorIndex, 
									  int iSkillLevelTableID, /*IN OUT*/ int* pIParamOffset, /*IN OUT*/ set<string>& vlUseActions );

public:
	virtual bool CheckAnimation() { return true; }
};
