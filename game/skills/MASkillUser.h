#pragma once

#include "EtMatrixEx.h"
#include "DnItem.h"
#include "DnSkill.h"

class CDnActor;
class CDnProjectile;

class MASkillUser 
{
public:
	MASkillUser();
	virtual ~MASkillUser();

protected:
	LOCAL_TIME m_LocalTime;
	DnSkillHandle m_hProcessSkill;
	DnSkillHandle m_hCanceledSkill;

	DnSkillHandle m_hToggleSkill;		//This is no longer used.. code traces remain here and there so only keeping the variable..
	DnSkillHandle m_hAuraSkill;

	DnSkillHandle m_hItemSkill;

#ifndef PRE_FIX_SKILLLIST
	DNVector(DnSkillHandle) m_vlhSkillList;
	vector<bool> m_vbSelfAllocList;
#else
	struct S_SKILL_OBJECT
	{
		DnSkillHandle hSkill;
		bool bSelfAlloc;

		S_SKILL_OBJECT( void ) : bSelfAlloc( false ) {};
		S_SKILL_OBJECT( DnSkillHandle _hSkill, bool _bSelfAlloc ) 
		{  
			hSkill = _hSkill;
			bSelfAlloc = _bSelfAlloc;
		};
	};
#endif // #ifndef PRE_FIX_SKILLLIST
	

	// Group skills with duration attached to items (those following standard skill routines like throwing knives) separately.
	deque<DnSkillHandle> m_dqhItemSkillList;

	DnActorHandle m_hSkillTarget;
	EtVector3 m_vSkillTargetPos;

	// Collect toggle and aura skills.
	DNVector(DnSkillHandle) m_vlhToggleSkills;
	DNVector(DnSkillHandle) m_vlhAuraSkills;

	// Collect skills that permanently increase stats (like Power Mastery) separately to control when their effects are applied and removed.
	DNVector(DnSkillHandle) m_vlhSelfPassiveBlowSkill;
	CDnActor *m_pActor;
	bool m_bIsValidActor;

#ifdef _GAMESERVER
	// Store the state to be applied when the toggle skill is turned on (to attach and send with projectiles)
	CDnState m_ActorStateSnapshotForToggleProjectile;
#endif

	// List of skills to terminate. Processed in MASkillUser::Process.
	DNVector(DnSkillHandle) m_vlhReservedFinishSkill;

	// Ratio variable affected by cooldown state effect adjustment value.
	//float m_fAdjustCoolTime;

#ifdef _GAMESERVER
	// cheat
	bool m_bIgnoreCoolTime;

	list<DnSkillHandle> m_listAutoPassiveSkills;
	list<DnSkillHandle> m_listProcessAutoPassiveSkills;

	// State effects specified as the skill's Target in an item, applied with a certain probability to targets hit by basic attacks 
	// Collection of granted manifestation types.
	DNVector(IDnSkillProcessor*) m_vlpApplySEWhenNormalHitProcessor;
#else
	bool m_bSkillExecutedThisFrame;		// Prevent multiple skills from being used simultaneously by pressing multiple quickslots the moment the allow-skill-use signal arrives. (#9886)

	// Collection of action names used by owned skills
	set<string> m_setUseActionNames;
#endif

	// Cooldown adjustment state effect implementation changed. Currently affects cooldown delta during effect duration when calculating cooldown. (#21146)
	float m_fCoolTimeDeltaAdjustValue;

private:
#ifdef PRE_FIX_SKILLLIST
	vector<S_SKILL_OBJECT> m_vlSkillObjects;
#endif // #ifdef PRE_FIX_SKILLLIST

protected:
	bool IsValidActor();

	CDnSkill::UsingResult CanExecuteSkill( DnSkillHandle hSkill );

	// Process
	virtual void OnSkillProcess( LOCAL_TIME LocalTime, float fDelta );
	// Use skill
	// Auto passive skill flag started on game server is received as a function argument only on client.
#ifdef _GAMESERVER
	virtual bool ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta );
#else
	virtual bool ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta, bool bCheckValid = true, bool bAutoUsedFromServer = false, bool bSendPacketToServer = true );
#endif
	// When a projectile is fired while a toggle is active, called with the toggled skill and projectile pointer.
	virtual void OnSkillToggleProjectile( DnSkillHandle hSkill, CDnProjectile *pProjectile );
public:
	// Called when a toggle skill is used.
	virtual void OnSkillToggle( DnSkillHandle hSkill, bool bEnable );
	// Called when an aura skill is used.
	virtual void OnSkillAura( DnSkillHandle hSkill, bool bEnable );
	// Projectile
	virtual void OnSkillProjectile( CDnProjectile *pProjectile );
	virtual void OnAddSkill( DnSkillHandle hSkill, bool isInitialize = false );
	virtual void OnRemoveSkill( DnSkillHandle hSkill );
	virtual void OnReplacementSkill( DnSkillHandle hLegacySkill, DnSkillHandle hNewSkill ) {};

public:
	DnSkillHandle FindSkill( int nSkillTableID );

	virtual bool CanAddSkill( int nSkillTableID, int nLevel = 1 ){ return true; }
	virtual bool AddSkill( int nSkillTableID, int nLevel = 1, int iSkillLevelApplyType = CDnSkill::PVE );

#ifndef PRE_FIX_SKILLLIST		// For future search convenience.
#else
	virtual void AddSkillObject( S_SKILL_OBJECT& SkillObject ) { m_vlSkillObjects.push_back( SkillObject ); };
#endif // #ifdef PRE_FIX_SKILLIST

	// On game server, used only for monsters.
	// Do not use this function on the server for player actors due to intermediate skill tree management.
	bool AddSkill( DnSkillHandle hSkill );
	bool RemoveSkill( int nSkillTableID );
	bool RemoveSkillAll();
	bool EndStateEffectSkill(int nSkillTableID);
	// Currently used only on the client.
#ifndef _GAMESERVER
	bool ReplacementSkill( int nSkillTableID, int nLevel = 1 );		// Skill object self allocation used on game server
	bool ReplacementSkill( DnSkillHandle hNewSkill );
#endif
	
	virtual bool ApplyPassiveSkill( DnSkillHandle hSkill, bool isInitialize = false );
	virtual void OnApplyPassiveSkillBlow( int iBlowID ) {};
	void ApplyPassiveSkills( void );					// Called on resurrection to reapply state effects that were cleared
	bool bIsPassiveSkill( DnBlowHandle hBlow );
	bool IsExistSkill( int nSkillTableID, int nLevel = -1 );
	void SetSkillLevel( int nSkillTableID, int nValue );
	bool IsSelfAllocSkill( int nSkillTableID );

	virtual CDnSkill::UsingResult UseSkill( int nSkillTableID, bool bCheckValid = true, bool bAutoUseFromServer = false, int nLuaSkillIndex=-1 );	// Added LuaSkillIndex as needed by monster AI.
	bool IsProcessSkill();
	void ResetToggleSkill();
	void ResetAuraSkill();

	bool IsEnabledToggleSkill() 
	{ 
		//So there are no more references to the m_hToggleSkill variable..
		return false;
	};
	
	bool IsEnabledAuraSkill() { return (m_hAuraSkill != NULL); };

	DnSkillHandle GetProcessSkill();
	DnSkillHandle GetEnabledToggleSkill() { return m_hToggleSkill; };
	DnSkillHandle GetEnabledAuraSkill() { return m_hAuraSkill; };

	DnActorHandle GetSkillTarget() { return m_hSkillTarget; }
	EtVector3 *GetSkillTargetPos() { return &m_vSkillTargetPos; }

	DWORD GetSkillCount();
	DnSkillHandle GetSkillFromIndex( DWORD dwIndex );

	bool UseItemSkill( int nSkillTableID, int nSkillLevelTableID, CDnItem::ItemSkillApplyType ItemSkillApplyType, int nItemID = -1 );
	bool IsUsingItemSkill( void ) { return m_hItemSkill ? true : false; }

	void ResetSkillCoolTime( void );
	void ResetPvPSkillCoolTime();
	void ResetLadderSkillCoolTime();
#ifdef _GAMESERVER
	void IgnoreSkillCoolTime( void );
	bool IsIgnoreSkillCoolTime( void ) { return m_bIgnoreCoolTime; };
#endif

#ifndef _GAMESERVER
	bool IsUsingSkillAction( const char *szAction );
#endif
	void ReserveFinishSkill( DnSkillHandle hSkill );
	
	// Stop using the currently active skill.
	void CancelUsingSkill( void );

#ifdef _GAMESERVER
	// for Fire/Ice Shield
	void UseAutoPassiveSkill( LOCAL_TIME LocalTime, float fDelta );
	bool IsProcessingAutoPassive( int iSkillID );
	void EndAutoPassiveSkill( LOCAL_TIME LocalTime, float fDelta );
	//const CDnState *GetActorStateSnapshotForProjectile( void ) { return &m_ActorStateSnapshotForToggleProjectile; };
#endif

	//void AddCoolTimeAdjustValue( float fCoolTimeAdjustValue ) 
	//{
	//	_ASSERT( 0.0f < fCoolTimeAdjustValue+fCoolTimeAdjustValue && fCoolTimeAdjustValue+fCoolTimeAdjustValue < 1.0f ); 
	//	m_fAdjustCoolTime += fCoolTimeAdjustValue; 
	//};
	//float GetCoolTimeAdjustValue( void ) { return m_fAdjustCoolTime; };

	// Weapon changed. Since passive state effects can modify specific weapon ranges, need to check if the weapon is usable for the skill as defined in the table.
	void OnAttachWeapon( DnWeaponHandle hWeapon, int iAttachedWeaponIndex );
	void OnDetachWeapon( DnWeaponHandle hWeapon, int iDetachedWeaponIndex );

	float GetCoolTimeDeltaAdjustValue( void ) { return m_fCoolTimeDeltaAdjustValue; };

#ifdef _GAMESERVER
	void SelectSkillLevelDataType( int iLevelDataType, bool bPlayerSummonedMonster = false );
	int GetSelectedSkillLevelDataType( void );
#endif

protected:
	std::map<int, int> m_vlStateBlowIDToRemove;	// [2010/12/15 semozz] Changed to map to prevent duplicate state effect IDs
public:
	void AddStateBlowIDToRemove(int blowID);	//Register state effect ID to remove
	void InitStateBlowIDToRemove();				//Initialize list of state effect IDs to remove
	virtual void RemoveResetStateBlow() {};		//Remove state effects to be removed

#ifdef _GAMESERVER
	void AddApplySEWhenTargetNormalHitProcessor( IDnSkillProcessor* pProcessor ) { m_vlpApplySEWhenNormalHitProcessor.push_back( pProcessor ); };
	void RemoveApplySEWhenTargetNormalHitProcessor( IDnSkillProcessor* pProcessor );
#endif

#if defined(_GAMESERVER)
protected:

	//For storing skill info per slot.
	struct PREFIX_SYSTEM_SKILLINFO
	{
		int slotIndex;				//Target slot Index
		float fRatio;				//Skill probability
		DnSkillHandle hSkill;		//Skill handle
	};

	typedef std::vector<PREFIX_SYSTEM_SKILLINFO> PRESIX_SYSTEM_SKILLS;			//Prefix skill info list
	typedef std::map<int, PRESIX_SYSTEM_SKILLS> PREFIX_SYSTEM_SKILL_COLLECTION;	//List for grouping by Skill ID / grouping by priority
	typedef std::map<int, PREFIX_SYSTEM_SKILLINFO> PREFIX_SYSTEM_SKILL_LIST;	//Per-slot skill info list for prefix system
	
	PREFIX_SYSTEM_SKILL_LIST m_prefixSystemDefenceSkills;			//Defense skills of prefix system
	PREFIX_SYSTEM_SKILL_COLLECTION m_PrefixSystemCandiateDefenceSkills;
	
	PREFIX_SYSTEM_SKILL_LIST m_prefixSystemOffenceSkills;			//Attack skills of prefix system
	PREFIX_SYSTEM_SKILL_COLLECTION m_PrefixSystemCandiateOffenceSkills;

	list<DnSkillHandle> m_listProcessPreFixDefenceSkills;
	list<DnSkillHandle> m_listProcessPreFixOffenceSkills;

	void RefreshPrefixDefenceSkills();	//Update prefix system defensive skill info on equip/unequip (more efficient to do once on equip/unequip than every time a skill is used).
	void RefreshPrefixOffenceSkills(float fHitDamageProb = 1.0f);	//Update prefix system offensive skill info on equip/unequip.

	//Prefix skill cooldown management
	CoolTimeManager m_PrefixSkillCoolTimeManager;
public:
	void EndPrefixSystemSkill( LOCAL_TIME LocalTime, float fDelta );

	bool AddPreFixSystemDefenceSkill(int slotIndex, DnSkillHandle hSkill);
	void RemovePreFixSystemDefenceSkill(int slotIndex);
	void RemoveProcessPrefixDefenceSkill(DnSkillHandle hSkill);
	
	bool AddPreFixSystemOffenceSkill(int slotIndex, DnSkillHandle hSkill);
	void RemovePreFixSystemOffenceSkill(int slotIndex);
	void RemoveProcessPrefixOffenceSkill(DnSkillHandle hSkill);

	// Collect skills with the same skill ID.
	void GroupingPrefixDefenceSkillsBySameSkillID(PREFIX_SYSTEM_SKILL_LIST &skillList, PREFIX_SYSTEM_SKILL_COLLECTION &sameSkillList);
	// Adjust (+) the activation probability of collected skills with the same skill ID and register candidate skills.
	void CalculateProbabilitySkill(PREFIX_SYSTEM_SKILL_LIST &candidateSkills, PREFIX_SYSTEM_SKILL_COLLECTION &sameSkillList);
	// Group the finally selected skills among those with the same skill ID by priority.
	void GroupingSkillByPriority(PREFIX_SYSTEM_SKILL_COLLECTION& prefixSystemSkills, PREFIX_SYSTEM_SKILL_LIST& tempCandiateSkills, float fHitDamageProb = 1.0f);
	
	void ProcessPrefixDefenceSkill(DnActorHandle hHitter);	//Activate defense skill in prefix system
	
	// Apply only state effects meant to apply only to self.
	void ProcessPrefixOffenceSkill(float fHitDamageProb);	//Activate attack skill in prefix system
	void ApplyPrefixOffenceSkillToTarget(DnActorHandle hTarget);//Apply state effects to target from prefix system attack skill
	
	// Reset after applying prefix system attack skill.
	void InitPrefixOffenceSkills();

	bool IsPrefixTriggerSkill();

	bool IsApplyPrefixOffenceSkill() { return !m_listProcessPreFixOffenceSkills.empty(); }

#endif // _GAMESERVER

protected:
	struct SkillLevelUpInfo
	{
		int nSkillID;
		int nLevelUp;
	};

	std::map<int, SkillLevelUpInfo> m_SkillLevelUpInfoList;
	std::map<int, int> m_SkillLevelUpInfo;
	
	void UpdateSkillLevelUpInfo();
	void SkillLevelUp(int nSkillID, int nLevelUp);
public:
	void AddSkillLevelUpInfo(int nSlotIndex, int nSkillID, int nLevelUp);
	void RemoveSkillLevelUpInfo(int nSlotIndex);

	int GetSkillLevelUpValue(int nSkillID);
	bool ExistSkillLevelUpValue();

	void ApplySkillLevelUp( int nSkillTableID, int nValue ); //Added function due to first condition issue in SetSkillLevel function

	//Declare virtual function to distinguish client LocalPlayer (on client, LocalPlayerActor changes skills via SkillTask)
	//Non-LocalPlayer characters on client and PlayerActor on server are handled in MaSkillUser
	virtual void ChangeSkillLevelUp(int nSkillID, int nSkillLevel);

protected:
	std::map<int, SkillLevelUpInfo> m_CashSkillLevelUpInfoList;
	std::map<int, int> m_CashSkillLevelUpInfo;

	void UpdateSkillLevelUpInfoByCashItem();

public:
	void AddSkillLevelUpInfoByCashItem(int nSlotIndex, int nSkillID, int nLevelUp);
	void RemoveSkillLevelUpInfoByCashItem(int nSlotIndex);


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
#if defined(_GAMESERVER)
protected:
	typedef std::map<int, CDnPrefixSkill*> PREFIX_SKILL_INFO;
	PREFIX_SKILL_INFO m_PrefixDefenceSkills;
	PREFIX_SKILL_INFO m_PrefixOffenceSkills;

	std::list<CDnPrefixSkill*> m_ProcessPrefixOffenceSkills;

public:
	void AddPrefixDefenceSkill(DnSkillHandle hSkill);
	void RemovePrefixDefenceSkill(DnSkillHandle hSkill);

	void ProcessPrefixDefenceSkill_New(DnActorHandle hHitter);


	void AddPrefixOffenceSkill(DnSkillHandle hSkill);
	void RemovePrefixOffenceSkill(DnSkillHandle hSkill);
	void RemoveProcessPrefixOffenceSkill(CDnPrefixSkill* pRemovePrefixSkill);

	void ProcessPrefixOffenceSkill_New();
	void ApplyPrefixOffenceSkillToTarget_New(DnActorHandle hTarget);
#endif // _GAMESERVER
	
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


protected:
	typedef std::map<int, DnSkillHandle> TOGGLESKILLLIST;
	TOGGLESKILLLIST m_ToggleOnSkillList;					//Manage list of toggled-on skills

	void SkillToggle(DnSkillHandle hSkill, bool isOn);		//Skill toggle On/Off

public:
	TOGGLESKILLLIST& GetActiveToggleSkillList() { return m_ToggleOnSkillList; }

	void SummonOnOffSkillInit(DWORD dwSummonMonsterID);

#if defined(PRE_FIX_62052)
	void ApplyGlobalSkillCoolTime(DnSkillHandle hSkill);
#endif // PRE_FIX_62052


#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
public:
	void UpdateGlobalCoolTime(float fRate);
	void ResetGlobalCoolTime(float fRate);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
};
