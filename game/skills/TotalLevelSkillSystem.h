#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#pragma once
#include "Singleton.h"

class CDnTotalLevelSkillSystem
{
public:
	CDnTotalLevelSkillSystem(DnActorHandle hActor);
	virtual ~CDnTotalLevelSkillSystem();

	enum _TotalLevelSkillType
	{
		TotalLevelSkill_Type_None = -1,
		TotalLevelSkill_Type_Attack = 0,
		TotalLevelSkill_Type_Defence,
		TotalLevelSkill_Type_Ability,
	};

	enum _TotalLevelSkillSlotIndex
	{
		TotalLevelSkill_Slot_0,
		TotalLevelSkill_Slot_1,
		TotalLevelSkill_Slot_2,
		TotalLevelSkill_Slot_3,		//Cash slot.
		TotallevelSkill_Slot_Count,
	};

	enum _ErrorCode
	{
		TotalLevelSystem_Error_None = 0,
		TotalLevelSystem_Error_NotUsable,
		TotalLevelSystem_Error_NotActivateSlot,
		TotalLevelSystem_Error_SameSkillType,
	};

	struct _TotalLevelSkillInfo
	{
		int m_TotalLevelLimit;
		int m_ActorLevelLimit;
		int m_SkillType;
		int m_SkillTreeSlotIndex;

		int m_SkillID;
	};

	struct _TotalLevelSkillSlotInfo
	{
		int m_LevelLimit;
		int m_SlotIndex;
		bool m_isCashSlot;
		__time64_t m_ExpireDate;
	};

protected:
	DnActorHandle m_hActor;													//Unified level skill actor owner

	vector<DnSkillHandle> m_vlTotalLevelSkillList;
	
	std::map<int, _TotalLevelSkillInfo> m_TotalLevelSkillInfoList;			//Unified level skill info
	std::map<int, _TotalLevelSkillSlotInfo> m_TotalLevelSlotInfoList;		//Unified level skill slot info

	std::map<int, bool> m_TotalLevelSkillSlotActivate;						//Unified level slot activation info
	std::map<int, DnSkillHandle> m_ActiveTotalLevelSkillList;				//Unified level skill registration info

	int m_TotalLevel;	//Current unified level value

	void RemoveTotalLevelSkillFromList(int slotIndex);

public:
	std::map<int, bool>& GetSlotActivateList() { return m_TotalLevelSkillSlotActivate; }
	_TotalLevelSkillSlotInfo* GetSlotInfo(int nSlotIndex);

	vector<DnSkillHandle>& GetTotalLevelSkillList() { return m_vlTotalLevelSkillList; }
	_TotalLevelSkillInfo* GetTotalLevelSkillInfo(int nSkillID);

	void LoadTotalLevelSkillList();
	void LoadTotalLevelSkillSlotInfo();

	DnSkillHandle FindTotalLevelSkill( int nSkillTableID );
	bool IsTotalLevelSkill( int nSkillTableID );

	bool IsActivateSlot(int nSlotIndex);

	//Register unified level skill..
	void AddTotalLevelSkill(int slotIndex, DnSkillHandle hSkill, bool isInitialize = false);
	//Remove unified level skill
	void RemoveTotallevelSkill(int slotIndex);
	//Return registered unified level skill
	DnSkillHandle GetActivateTotalLevelSkill(int slotIndex);

	//Set unified level skill slot activation.
	void ActivateTotalLevelSkillSlot(int slotIndex, bool bActivate);
	//Configure cash slot activation..
	void ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate);

	void SetTotalLevel(int totalLevel);
	int GetTotalLevel() { return m_TotalLevel; }

	//Check whether the skill can be registered.
	int CanAddSkill(int slotIndex, DnSkillHandle hSkill);

	//Whether unified level skill is activated..
	bool IsUsableSkill(DnSkillHandle hSkill);

	void UpdateTotalLevel();

	void Process(LOCAL_TIME localTime, float fDelta);

	int FindEmptySlot();

	int FindEmptyNormalSlot(int nSkillID);
	int FindEmptyCashSlot(int nSkillID);
	int FindEmptySlot(int nSkillID);
	
};

#endif // PRE_ADD_TOTAL_LEVEL_SKILL