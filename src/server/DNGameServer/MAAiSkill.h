#pragma once

#include "DNUserItem.h"
#include "MAAiChecker.h"

struct MonsterSkillTable 
{
	enum _TargetType
	{
		SELF = 1,
		SAMETEAM,				// Same team (including self)
		OPPONENTTEAM,			// State team
		SAMETEAM_EXPECTME,		// Same team (excluding self)
	};

	enum _ConditionType
	{
		BUFF = 1,
		DEBUFF
	};

	MonsterSkillTable()
	{
		nSkillIndex = nCooltime = nRangeMin = nRangeMax = nTargetType = nCancelLook = 0;
		pMAAiCheckerManager = NULL;
#if defined (PRE_MOD_AIMULTITARGET)
		nMultipleTarget	= 0;
		nSummonerTarget = 0;
#else
		bMultipleTarget	= false;
#endif
		iMaxMultipleTargetCount = 0;
		bIsAnyMultipleTarget = false;
		bIsExceptCannonPlayer = false;
		iNextLuaSkillIndex = -1;
		iLuaSkillIndex = -1;
#if defined( PRE_FIX_68096 )
		memset(iGlobalCoolTimeIndex, -1, sizeof(iGlobalCoolTimeIndex));
#else
		iGlobalCoolTimeIndex = -1;
#endif
#if defined( PRE_FIX_67719 )
		fRandomTarget = 0.f;
		nIgnoreAggroTarget = 0;
#endif
		iPriority = 0;
	}

	int						nSkillIndex;
	int						iLuaSkillIndex;
	int						nCooltime;
	int						nRangeMin;
	int						nRangeMax;
	int						nTargetType;
	int						nCancelLook;		// Whether to unlock Look when performing this action; 1 unlocks it.
#if defined (PRE_MOD_AIMULTITARGET)
	int						nMultipleTarget;	// Multiple target type
	int						nSummonerTarget;
#else
	bool					bMultipleTarget;	// Flag indicating whether Multiple target
#endif
	int						iMaxMultipleTargetCount;
	bool					bIsAnyMultipleTarget;
	bool					bIsExceptCannonPlayer;
	int						iNextLuaSkillIndex;
#if defined( PRE_FIX_68096 )
	int						iGlobalCoolTimeIndex[5];
#else
	int						iGlobalCoolTimeIndex;
#endif
	int						iPriority;
#if defined( PRE_FIX_67719 )
	float					fRandomTarget;		// Random target
	int 					nIgnoreAggroTarget;	// (0: Do not use skill if no target, 1: Check skill usage against current target if no target)
#endif

	CMAAiCheckerManager*	pMAAiCheckerManager;
};

class MonsterSkillAI
{
public:
	MonsterSkillAI():m_pMonsterActor(NULL),m_pSkillTable(NULL) 
	{ 
		m_iNextLuaSkillIndex = -1;
	}
	virtual ~MonsterSkillAI() {	}

	void Initialize(std::vector<MonsterSkillTable>* pSkilltable, DnActorHandle hActor);

	int GetSkillIndexFromLuaSkillIndex( UINT uiIndex )
	{
		if( m_pSkillTable->size() <= uiIndex )
			return -1;
		return m_pSkillTable->at(uiIndex).nSkillIndex;
	}
	// Calculate cooldown for skill AI.
	void Process( LOCAL_TIME LocalTime, float fDelta );
	// 
	bool UseSkill( int iArrayIndex=-1 );
	void AddWaitOrderCount( DnActorHandle hActor, int iSkillID );
	void DelWaitOrderCount( DnActorHandle hAcotr, int iSkillID );

	bool TargetCheck(int nSkillTableID, DnSkillHandle hSkill, OUT DnActorHandle& hTargetActor);
	bool ScanActor(float fMinDist, float fMaxDist, int nTargetType, OUT DNVector(DnActorHandle)& ActorList);
	int		GetNextLuaSkillIndex(){ return m_iNextLuaSkillIndex; }

#if defined( PRE_FIX_67719 )
	bool _SelectRandomTarget( MonsterSkillTable& table );
#endif

#if defined(PRE_ADD_64990)
	static void CheckSkillTable(lua_State* pLua, vector<string>& skillTableName);
	static bool LoadSkillTable(lua_State* pLua, OUT std::vector<MonsterSkillTable>&	SkillTable, string& tableName);
#endif // PRE_ADD_64990

	static bool LoadSkillTable(lua_State* pLua, OUT std::vector<MonsterSkillTable>&	SkillTable);

private:

	DnActorHandle					m_hActor;
	CDnMonsterActor*				m_pMonsterActor;
	std::vector<MonsterSkillTable>*	m_pSkillTable;
	std::map<int,int>				m_mSkillTable;	// SkillID/m_pSkillTable vector index
	CDNCoolTime						m_CoolTime;
	int								m_iNextLuaSkillIndex;

	struct _UsableSkill 
	{
		int nTableIdx;
		int nSkillID;
		DnActorHandle hTargetActor;
	};

	bool _bIsActorChecker( const MonsterSkillTable& table );
	void _bIsTargetActorChecker( MonsterSkillTable& table, DNVector(DnActorHandle)& SelectedList );

};
