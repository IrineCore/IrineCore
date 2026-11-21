
#pragma once

#include "SignalHeader.h"

struct	ActionTable;
struct	MonsterSkillTable;
class	MAAiScript;

class CMAAiMultipleTarget
{
public:

	enum MultipleTargetType
	{
		Action = 0,
		Skill,
		Max,
	};

	CMAAiMultipleTarget();
	~CMAAiMultipleTarget();

	void	Reset();
#if defined (PRE_MOD_AIMULTITARGET)
	int		GetMultipleTarget(){ return m_nMultipleTarget; }
	bool	CalcTargetByAggro(DnActorHandle hActor);
#else
	bool	bIsMultipleTarget(){ return m_bIsMultipleTarget; }
#endif

	void	SetMultipleTargetAction( ActionTable* pActionTable );
	void	SetMultipleTargetSkill( MonsterSkillTable* pSkillTable );

	void	CalcTarget( DnActorHandle hActor, MAAiScript* pScript );
	void	CreateProjectile( CDnMonsterActor* pMonsterActor, ProjectileStruct* pStruct, int iSignalIndex );

	MultipleTargetType	GetType(){ return m_eType; }

private:

#if defined (PRE_MOD_AIMULTITARGET)
	int						m_nMultipleTarget;
	int						m_nSummonerTarget;
#else
	bool					m_bIsMultipleTarget;
#endif
	int						m_iMaxMultipleTargetCount;	// If this value is 0, all targets as before
	bool					m_bIsAnyMultipleTarget;
	bool					m_bIsExceptCannonPlayer;	// Ignore users in cannon mode.

	MultipleTargetType		m_eType;

	// MultipleTargetType::Action
	ActionTable*				m_pActionTable;
	std::vector<EtVector3>		m_vTargetPos;
	DNVector(DnActorHandle)		m_vTargetActor;


	// MultipleTargetType::Skill
	MonsterSkillTable*			m_pSkillTable;
};
