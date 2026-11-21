#pragma once

#include "DnActor.h"
#include "DnWeapon.h"
#include "SmartPtr.h"
#include "MultiSingleton.h"

class CDnSkillBase : public CMultiSmartPtrBase< CDnSkillBase, MAX_SESSION_COUNT > {
public:
	CDnSkillBase( DnActorHandle hActor );
	virtual ~CDnSkillBase();

	enum SkillTypeEnum {
		Active,
		Passive,
	};
	enum DurationTypeEnum {
		Instantly,
		Buff,
		Debuff,
		TimeToggle,
		ActiveToggle,
		Aura,
	};
	enum TargetTypeEnum {
		Self,
		Enemy,
		Friend,
	};

	enum StateEffectApplyType {
		ApplySelf,
		ApplyTarget,
	};

	struct StateEffectStruct {
		int nID;
		StateEffectApplyType ApplyType;
		int nDurationTime;
		std::string szValue;
	};

protected:
	DnActorHandle m_hActor;

	int m_nSkillID;
	std::string m_szStaticName;

	int m_nNeedJobClassID;
	CDnWeapon::EquipTypeEnum m_RequireWeaponType;
	int m_nNeedItemID;
	int m_nNeedItemDecreaseCount;

	int m_nLevel;
	int m_nMaxLevel;
	int m_nLevelLimit;
	int m_nIncreaseRange;
	int m_nDecreaseHP;
	int m_nDecreaseSP;
	int m_nDelayTime;
	LOCAL_TIME m_UseTime;
	float m_fCoolTime;
	int m_nAdditionalThreat;
	
	SkillTypeEnum m_SkillType;
	DurationTypeEnum m_DurationType;
	TargetTypeEnum m_TargetType;

	std::vector<StateEffectStruct> m_VecStateEffectList;
	std::vector<int> m_nVecApplyStateEffectList;

protected:
	void CheckAndAddStateEffect();
	void CheckAndRemoveStateEffect();

public:
	DWORD GetStateEffectCount();
	StateEffectStruct *GetStateEffectFromIndex( DWORD dwIndex );
//	const std::vector<StateEffectStruct>& GetStateEffectSelf() { return m_VecStateEffectSelt; }
//	void ClearStateEffectSelf() { m_VecStateEffectSelt.clear(); }

public:
	// Static functions
	static DnSkillHandle_ CreateSkill( DnActorHandle hActor, int nSkillTableID, int nLevel );

	// DnSkillBase Member
	DnActorHandle GetActor() { return m_hActor; }
	int GetClassID() { return m_nSkillID; }
	SkillTypeEnum GetSkillType() { return m_SkillType; }
	DurationTypeEnum GetDurationType() { return m_DurationType; }
	TargetTypeEnum GetTargetType() { return m_TargetType; }
	int GetIncreaseRange() { return m_nIncreaseRange; }
	int GetDecreaseHP() { return m_nDecreaseHP; }
	int GetDecreaseSP() { return m_nDecreaseSP; }
	int GetLevel() { return m_nLevel; }
	int GetMaxLevel() { return m_nMaxLevel; }
	int GetLevelLimit() { return m_nLevelLimit; }
	CDnWeapon::EquipTypeEnum GetRequireWeaponType() { return m_RequireWeaponType; }
	int GetNeedItemID() { return m_nNeedItemID; }
	int GetNeedItemDecreaseCount() { return m_nNeedItemDecreaseCount; }
	int GetAdditionalThreat() { return m_nAdditionalThreat; }

	void Ready( LOCAL_TIME LocalTime, float fDelta );
	void Execute( LOCAL_TIME LocalTime, float fDelta );
	void Finish( LOCAL_TIME LocalTime, float fDelta );

	virtual bool Initialize( int nSkillTableID, int nSkillLevelTableID );
	virtual bool CanReady();
	virtual bool CanExecute();
	virtual bool IsFinish();

	virtual void ProcessReady( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessExecute( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	float GetCoolTime() { return m_fCoolTime; }
};

template < class UsingType, class ActionType >
class TDnSkillBase : public CDnSkillBase, public UsingType, public ActionType
{
public:
	TDnSkillBase( DnActorHandle hActor ) : CDnSkillBase( hActor ) {}
	virtual ~TDnSkillBase() {}

protected:
};
