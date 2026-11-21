#pragma once

#include "DnActorState.h"
class CDnMonsterState : virtual public CDnActorState
{
public:
	CDnMonsterState();
	virtual ~CDnMonsterState();

	enum GradeEnum {
		Minion,
		Normal,
		Champion,
		Elite,
		Named,
		Boss,
		BossHP4,	// Boss type used in hardcore or practice mode. Boss displayed with 4 HP bars.
		NestBoss,
		NestBoss8,
	};

protected:
	int m_nDeadExperience;	// EXP on death
	int m_nCompleteExperience; // EXP on completion

	int m_nDeadDurability; // Party durability reduction value when a monster dies
	GradeEnum m_Grade;
	int m_nItemDropGroupTableID;
	bool m_bSlowByDie;
	bool m_bSummonRandomFrame;
	bool m_bApplyEffectScale;
	int m_nRaceID;
	bool m_bBossKillCheck;

protected:
	virtual void CalcBaseState( StateTypeEnum Type = ST_All );
	virtual void CalcState( RefreshStateExtentEnum Extent, StateTypeEnum Type );
	virtual void GetStateList( int ListType, CDnState::ValueType Type, std::vector<CDnState *> &VecList );

public:
	virtual void Initialize( int nClassID );

	__forceinline int GetDeadExperience() { return m_nDeadExperience; }
	__forceinline int GetCompleteExperience() { return m_nCompleteExperience; }
	__forceinline int GetDeadDurability() { return m_nDeadDurability; }

	GradeEnum GetGrade() { return m_Grade; }
	bool IsSlowByDie() { return m_bSlowByDie; }
	bool IsSummonRandomFrame() { return m_bSummonRandomFrame; }
	bool IsApplyEffectScale() { return m_bApplyEffectScale; }
	int GetRaceID() {return m_nRaceID;}
	bool IsBossKillCheck() { return m_bBossKillCheck; }
#ifdef PRE_ADD_MONSTER_CHEAT
	int GetItemDropGroupID() { return m_nItemDropGroupTableID; }
#endif 
};