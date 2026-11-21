#include "stdafx.h"
#include "TotalLevelSkillSystem.h"
#include "DnTableDB.h"
#include "DnActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)

CDnTotalLevelSkillSystem::CDnTotalLevelSkillSystem(DnActorHandle hActor)
{
	m_hActor = hActor;

	m_TotalLevel = 0;

	//Disable slots by default.
	for (int i = 0; i < TotallevelSkill_Slot_Count; ++i)
	{
		DnSkillHandle hSkill;

		ActivateTotalLevelSkillSlot(i, false);
		AddTotalLevelSkill(i, hSkill);
	}

	LoadTotalLevelSkillList();
	LoadTotalLevelSkillSlotInfo();
}

CDnTotalLevelSkillSystem::~CDnTotalLevelSkillSystem()
{
	std::map<int, DnSkillHandle>::iterator iter = m_ActiveTotalLevelSkillList.begin();
	std::map<int, DnSkillHandle>::iterator endIter = m_ActiveTotalLevelSkillList.end();
	for (; iter != endIter; ++iter)
	{
		if (m_hActor && iter->second)
			m_hActor->RemoveSkill(iter->second->GetClassID());
	}

	SAFE_RELEASE_SPTRVEC( m_vlTotalLevelSkillList );
	m_TotalLevelSkillInfoList.clear();

	m_TotalLevelSlotInfoList.clear();
}

void CDnTotalLevelSkillSystem::LoadTotalLevelSkillList()
{
	SAFE_RELEASE_SPTRVEC( m_vlTotalLevelSkillList );
	m_TotalLevelSkillInfoList.clear();

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TTOTALLEVELSKILL );
	if( pSox == NULL )
	{
		DN_ASSERT( 0, "Invalid TotalLevelSkill" );
		return;
	}

	for( int i=0; i<=pSox->GetItemCount(); i++ )
	{
		int nItemID = pSox->GetItemID( i );
		if (nItemID == -1)
			continue;

		_TotalLevelSkillInfo _info;

		_info.m_SkillID = pSox->GetFieldFromLablePtr( nItemID, "_SkillTableID" )->GetInteger();
		_info.m_TotalLevelLimit = pSox->GetFieldFromLablePtr( nItemID, "_TotalLevelLimit" )->GetInteger();
		_info.m_ActorLevelLimit = pSox->GetFieldFromLablePtr( nItemID, "_LevelLimit" )->GetInteger();
		_info.m_SkillType = pSox->GetFieldFromLablePtr( nItemID, "_SkillType" )->GetInteger();
		_info.m_SkillTreeSlotIndex = pSox->GetFieldFromLablePtr( nItemID, "_TreeSlotIndex" )->GetInteger();

		m_TotalLevelSkillInfoList.insert(std::make_pair(_info.m_SkillID, _info));

		DnSkillHandle hSkill;
		hSkill = CDnSkill::CreateSkill( m_hActor, _info.m_SkillID, 1 );
		if( hSkill )
		{
			m_vlTotalLevelSkillList.push_back( hSkill );
		}
	}
}

void CDnTotalLevelSkillSystem::LoadTotalLevelSkillSlotInfo()
{
	m_TotalLevelSlotInfoList.clear();

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TTOTALLEVELLSKILLSLOT );
	if( pSox == NULL )
	{
		DN_ASSERT( 0, "Invalid TotalLevelSkillSlot" );
		return;
	}

	for( int i=0; i<=pSox->GetItemCount(); i++ )
	{
		int nItemID = pSox->GetItemID( i );
		if (nItemID == -1)
			continue;

		_TotalLevelSkillSlotInfo _info;

		_info.m_LevelLimit = pSox->GetFieldFromLablePtr( nItemID, "_LevelLimit" )->GetInteger();
		_info.m_SlotIndex = pSox->GetFieldFromLablePtr( nItemID, "_SlotID" )->GetInteger() - 1;
		_info.m_isCashSlot = pSox->GetFieldFromLablePtr( nItemID, "_IsCash" )->GetBool();
		_info.m_ExpireDate = 0;

		m_TotalLevelSlotInfoList.insert(std::make_pair(_info.m_SlotIndex, _info));
	}
}

DnSkillHandle CDnTotalLevelSkillSystem::FindTotalLevelSkill( int nSkillTableID )
{
	DnSkillHandle hRetSkill;
	vector<DnSkillHandle>::iterator iter = m_vlTotalLevelSkillList.begin();

	for( ; iter != m_vlTotalLevelSkillList.end(); iter++ )
	{
		DnSkillHandle hSkill = (*iter); 
		if( hSkill->GetClassID() == nSkillTableID )
		{
			hRetSkill = (*iter);
			break;
		}
	}

	return hRetSkill;
}

bool CDnTotalLevelSkillSystem::IsTotalLevelSkill( int nSkillTableID )
{
	DnSkillHandle hSkill = FindTotalLevelSkill( nSkillTableID );
	if( hSkill )
		return true;

	return false;
}

CDnTotalLevelSkillSystem::_TotalLevelSkillInfo* CDnTotalLevelSkillSystem::GetTotalLevelSkillInfo(int nSkillID)
{
	_TotalLevelSkillInfo* pInfo = NULL;

	std::map<int, _TotalLevelSkillInfo>::iterator skillInfoIter = m_TotalLevelSkillInfoList.find(nSkillID);
	if (skillInfoIter != m_TotalLevelSkillInfoList.end())
		pInfo = &skillInfoIter->second;

	return pInfo;
}

bool CDnTotalLevelSkillSystem::IsUsableSkill(DnSkillHandle hSkill)
{
	if (!hSkill)
		return false;

	if (!m_hActor)
		return false;

	_TotalLevelSkillInfo* pSkillInfo = GetTotalLevelSkillInfo(hSkill->GetClassID());
	if (pSkillInfo == NULL)
		return false;

	//Check unified level limit..
	if (m_TotalLevel < pSkillInfo->m_TotalLevelLimit)
		return false;

	//Actor level limit
	int nActorLevel = m_hActor->GetLevel();
	if (nActorLevel < pSkillInfo->m_ActorLevelLimit)
		return false;

	return true;
}

int CDnTotalLevelSkillSystem::CanAddSkill(int slotIndex, DnSkillHandle hSkill)
{
	int nErrorCode = 0;

	if (IsUsableSkill(hSkill) == false)
		return TotalLevelSystem_Error_NotUsable;

	//Check whether the slot is active.
	bool bSlotActivate = false;
	std::map<int, bool>::iterator slotActivateIter = m_TotalLevelSkillSlotActivate.find(slotIndex);
	if (slotActivateIter != m_TotalLevelSkillSlotActivate.end())
	{
		bSlotActivate = slotActivateIter->second;
	}

	//Cannot add if the slot is not active.
	if (bSlotActivate == false)
		return TotalLevelSystem_Error_NotActivateSlot;

	//Check the unified level skill type to add..
	int nAddSkillType = TotalLevelSkill_Type_None;
	_TotalLevelSkillInfo* pSkillInfo = GetTotalLevelSkillInfo(hSkill->GetClassID());
	if (pSkillInfo)
		nAddSkillType = pSkillInfo->m_SkillType;
	else
		return TotalLevelSystem_Error_NotUsable;

	bool bCheckSlotType = false;
	std::map<int, _TotalLevelSkillSlotInfo>::iterator slotIter = m_TotalLevelSlotInfoList.find(slotIndex);
	if (slotIter != m_TotalLevelSlotInfoList.end())
	{
		_TotalLevelSkillSlotInfo& _info = slotIter->second;
		//Adding to a cash slot always succeeds if the slot is active
		if (_info.m_isCashSlot == true)
			nErrorCode = TotalLevelSystem_Error_None;
		else
		{
			if (m_ActiveTotalLevelSkillList.empty() == true)
				nErrorCode = TotalLevelSystem_Error_None;
			else
			{
				bool bExistSkillType = false;

				//Cannot add if the same unified-level skill type is already added..
				std::map<int, DnSkillHandle>::iterator activateSkillIter = m_ActiveTotalLevelSkillList.begin();
				std::map<int, DnSkillHandle>::iterator activateSkillEndIter = m_ActiveTotalLevelSkillList.end();
				for (; activateSkillIter != activateSkillEndIter; ++activateSkillIter)
				{
					if (!activateSkillIter->second)
						continue;

					int nTotalLevelSkillType = TotalLevelSkill_Type_None;

					pSkillInfo = GetTotalLevelSkillInfo(activateSkillIter->second->GetClassID());
					if (pSkillInfo)
						nTotalLevelSkillType = pSkillInfo->m_SkillType;

					//If the slot index is the same, ignore any existing skill of the same type and add the skill.
					if (activateSkillIter->first == slotIndex)
					{
						if (nTotalLevelSkillType == nAddSkillType)
						{
							if (hSkill->GetClassID() == activateSkillIter->second->GetClassID())
							{
								nErrorCode = TotalLevelSystem_Error_NotUsable;
								return nErrorCode;
							}
						}
					}
					else
					{
						//Can add if this slot is a cash slot...
						//If not a cash slot and same skill type, do not add.
						std::map<int, _TotalLevelSkillSlotInfo>::iterator oldSlotIter = m_TotalLevelSlotInfoList.find(activateSkillIter->first);
						if (oldSlotIter != m_TotalLevelSlotInfoList.end())
						{
							if (oldSlotIter->second.m_isCashSlot != true &&
								nTotalLevelSkillType == nAddSkillType)
							{
								bExistSkillType = true;
								break;
							}
						}
					}
				}

				//Added only if there is no same skill type.
				if (bExistSkillType == false)
					nErrorCode = TotalLevelSystem_Error_None;
				else
					nErrorCode = TotalLevelSystem_Error_SameSkillType;
			}

		}
	}
	
	return nErrorCode;
}

void CDnTotalLevelSkillSystem::AddTotalLevelSkill(int slotIndex, DnSkillHandle hSkill, bool isInitialize/* = false*/)
{
	DnSkillHandle hActivateSkill;

	
	std::map<int, DnSkillHandle>::iterator findIter = m_ActiveTotalLevelSkillList.find(slotIndex);
	if (findIter != m_ActiveTotalLevelSkillList.end())
		hActivateSkill = findIter->second;

	//Remove existing skill and
	if (hActivateSkill)
		m_hActor->RemoveSkill(hActivateSkill->GetClassID());

	if (hSkill)
		m_hActor->OnAddSkill(hSkill, isInitialize);

	//If existing slot info exists, replace the skill, and
	if (findIter != m_ActiveTotalLevelSkillList.end())
	{
		//If there is a new skill to register, change skill; otherwise remove info
		if (hSkill)
			findIter->second = hSkill;
		else
			findIter->second.Identity();
	}
	else
	{
		m_ActiveTotalLevelSkillList.insert(std::make_pair(slotIndex, hSkill));
	}
}

DnSkillHandle CDnTotalLevelSkillSystem::GetActivateTotalLevelSkill(int slotIndex)
{
	DnSkillHandle hActivateSkill;

	std::map<int, DnSkillHandle>::iterator findIter = m_ActiveTotalLevelSkillList.find(slotIndex);
	if (findIter != m_ActiveTotalLevelSkillList.end())
		hActivateSkill = findIter->second;

	return hActivateSkill;
}

void CDnTotalLevelSkillSystem::RemoveTotallevelSkill(int slotIndex)
{
	DnSkillHandle hActivateSkill = GetActivateTotalLevelSkill(slotIndex);
	if (hActivateSkill)
	{
		m_hActor->RemoveSkill(hActivateSkill->GetClassID());

		//Initialize skill in the actual list.
		RemoveTotalLevelSkillFromList(slotIndex);
	}
}

void CDnTotalLevelSkillSystem::RemoveTotalLevelSkillFromList(int slotIndex)
{
	std::map<int, DnSkillHandle>::iterator findIter = m_ActiveTotalLevelSkillList.find(slotIndex);
	if (findIter != m_ActiveTotalLevelSkillList.end())
		findIter->second.Identity();
}

void CDnTotalLevelSkillSystem::ActivateTotalLevelSkillSlot(int slotIndex, bool bActivate)
{
	if (bActivate == false)
	{
		//If the slot becomes inactive, remove any existing skill.
		DnSkillHandle hActivateSkill = GetActivateTotalLevelSkill(slotIndex);
		if (hActivateSkill)
			RemoveTotallevelSkill(slotIndex);
	}

	std::map<int, bool>::iterator iter = m_TotalLevelSkillSlotActivate.find(slotIndex);
	if (iter != m_TotalLevelSkillSlotActivate.end())
		iter->second = bActivate;
	else
		m_TotalLevelSkillSlotActivate.insert(std::make_pair(slotIndex, bActivate));
}

void CDnTotalLevelSkillSystem::SetTotalLevel(int totalLevel)
{
	if (m_TotalLevel == totalLevel)
		return;

	m_TotalLevel = totalLevel;

	UpdateTotalLevel();
}

void CDnTotalLevelSkillSystem::UpdateTotalLevel()
{
	int nCharLevel = m_hActor ? m_hActor->GetLevel() : -1;

	//Update slot activation info
	std::map<int, _TotalLevelSkillSlotInfo>::iterator iter = m_TotalLevelSlotInfoList.begin();
	std::map<int, _TotalLevelSkillSlotInfo>::iterator endIter = m_TotalLevelSlotInfoList.end();
	for (; iter != endIter; ++iter)
	{
		_TotalLevelSkillSlotInfo& _info = iter->second;

		if (_info.m_isCashSlot == false)
		{
			bool bActivate = nCharLevel != -1 ? nCharLevel >= _info.m_LevelLimit : false;

			ActivateTotalLevelSkillSlot(_info.m_SlotIndex, bActivate);
		}
	}
}

void CDnTotalLevelSkillSystem::ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate)
{
	//Update slot activation info
	std::map<int, _TotalLevelSkillSlotInfo>::iterator iter = m_TotalLevelSlotInfoList.begin();
	std::map<int, _TotalLevelSkillSlotInfo>::iterator endIter = m_TotalLevelSlotInfoList.end();
	for (; iter != endIter; ++iter)
	{
		_TotalLevelSkillSlotInfo& _info = iter->second;

		if (_info.m_SlotIndex == nSlotIndex && _info.m_isCashSlot == true)
		{
			ActivateTotalLevelSkillSlot(_info.m_SlotIndex, bActivate);
			_info.m_ExpireDate = tExpireDate;
		}
	}
}

void CDnTotalLevelSkillSystem::Process(LOCAL_TIME localTime, float fDelta)
{
	vector<DnSkillHandle>::iterator iter = m_vlTotalLevelSkillList.begin();
	vector<DnSkillHandle>::iterator endIter = m_vlTotalLevelSkillList.end();

	for (; iter != endIter; ++iter)
	{
		DnSkillHandle hSkill = (*iter);
		if( hSkill)
			hSkill->Process( localTime, fDelta );
	}
}

int CDnTotalLevelSkillSystem::FindEmptySlot()
{
	int nSlotIndex = -1;

	std::map<int, DnSkillHandle>::iterator iter = m_ActiveTotalLevelSkillList.begin();
	std::map<int, DnSkillHandle>::iterator endIter = m_ActiveTotalLevelSkillList.end();

	for (; iter != endIter; ++iter)
	{
		if (!iter->second)
		{
			int slotIndex = iter->first;

			bool bSlotActivate = false;
			std::map<int, bool>::iterator slotActivateIter = m_TotalLevelSkillSlotActivate.find(slotIndex);
			if (slotActivateIter != m_TotalLevelSkillSlotActivate.end())
			{
				bSlotActivate = slotActivateIter->second;
			}

			if (bSlotActivate == false)
				continue;

			nSlotIndex = iter->first;
			break;
		}
	}

	return nSlotIndex;
}

int CDnTotalLevelSkillSystem::FindEmptyCashSlot(int nSkillID)
{
	int nSelectedSlotIndex = -1;

	//Skip if skill info is invalid.
	_TotalLevelSkillInfo* pSkillInfo = GetTotalLevelSkillInfo(nSkillID);
	if (pSkillInfo == NULL)
		return nSelectedSlotIndex;

	std::map<int, _TotalLevelSkillSlotInfo>::iterator iter = m_TotalLevelSlotInfoList.begin();
	std::map<int, _TotalLevelSkillSlotInfo>::iterator endIter = m_TotalLevelSlotInfoList.end();
	for (; iter != endIter; ++iter)
	{
		//Check only the cash slot.
		if (iter->second.m_isCashSlot == true)
		{
			int slotIndex = iter->first;

			//Skip if the cash slot is not active.
			if (IsActivateSlot(slotIndex) == false)
				continue;

			std::map<int, DnSkillHandle>::iterator findSkillIter = m_ActiveTotalLevelSkillList.find(slotIndex);
			if (findSkillIter != m_ActiveTotalLevelSkillList.end())
			{
				//If there is no skill in the slot, select this slot.
				if (!findSkillIter->second)
				{
					nSelectedSlotIndex = slotIndex;
					break;
				}
				else
				{
					//Select if same skill type, do not select if different skill type.
					_TotalLevelSkillInfo* pOldSkillInfo = GetTotalLevelSkillInfo(findSkillIter->second->GetClassID());
					if (pOldSkillInfo && pSkillInfo->m_SkillType == pOldSkillInfo->m_SkillType)
					{
						nSelectedSlotIndex = slotIndex;
						break;
					}
				}
			}
		}
	}

	return nSelectedSlotIndex;
}

int CDnTotalLevelSkillSystem::FindEmptyNormalSlot(int nSkillID)
{
	int nSelectedSlotIndex = -1;

	//Skip if skill info is invalid.
	_TotalLevelSkillInfo* pSkillInfo = GetTotalLevelSkillInfo(nSkillID);
	if (pSkillInfo == NULL)
		return nSelectedSlotIndex;


	std::map<int, int> normalEmptySlotList;

	//1. Find a slot with the same skill type.
	std::map<int, _TotalLevelSkillSlotInfo>::iterator iter = m_TotalLevelSlotInfoList.begin();
	std::map<int, _TotalLevelSkillSlotInfo>::iterator endIter = m_TotalLevelSlotInfoList.end();
	for (; iter != endIter; ++iter)
	{
		//Check normal slots only.
		if (iter->second.m_isCashSlot == false)
		{
			int slotIndex = iter->first;

			//Skip if the slot is not active.
			if (IsActivateSlot(slotIndex) == false)
				continue;

			std::map<int, DnSkillHandle>::iterator findSkillIter = m_ActiveTotalLevelSkillList.find(slotIndex);
			if (findSkillIter != m_ActiveTotalLevelSkillList.end())
			{
				//When the skill exists
				if (findSkillIter->second)
				{
					//Select if same skill type, do not select if different skill type.
					int nOldSkillID = findSkillIter->second->GetClassID();
					_TotalLevelSkillInfo* pOldSkillInfo = GetTotalLevelSkillInfo(nOldSkillID);
					if (pOldSkillInfo && 
						pSkillInfo->m_SkillType == pOldSkillInfo->m_SkillType)
					{
						if (nSkillID != nOldSkillID)
						{
							nSelectedSlotIndex = slotIndex;
							break;
						}
						else
						{
							//If a skill of the same type already exists..
							nSelectedSlotIndex = -2;
						}
					}
				}
				//If the skill does not exist, add it to an empty slot.
				else
				{
					normalEmptySlotList.insert(std::make_pair(slotIndex, slotIndex));
				}
			}
		}
	}

	//If a slot with the same skill ID exists and there is no empty slot, set nSelectedSlotIndex to -2 to prevent printing an error message.
	if (nSelectedSlotIndex == -2 && normalEmptySlotList.empty())
	{
		return nSelectedSlotIndex;
	}

	//If no skill of the same type was found above, find an empty slot.
	if (nSelectedSlotIndex == -1)
	{
		if (normalEmptySlotList.empty() == false)
		{
			std::map<int, int>::iterator emtpySlotIter = normalEmptySlotList.begin();
			if (emtpySlotIter != normalEmptySlotList.end())
				nSelectedSlotIndex = emtpySlotIter->first;
		}
	}

	return nSelectedSlotIndex;
}

int CDnTotalLevelSkillSystem::FindEmptySlot(int nSkillID)
{
	int nSlotIndex = -1;

	//1. Check cash slot.
	nSlotIndex = FindEmptyCashSlot(nSkillID);
	
	if (nSlotIndex == -1)
		nSlotIndex = FindEmptyNormalSlot(nSkillID);
	
	return nSlotIndex;
}

CDnTotalLevelSkillSystem::_TotalLevelSkillSlotInfo* CDnTotalLevelSkillSystem::GetSlotInfo(int nSlotIndex)
{
	_TotalLevelSkillSlotInfo* pInfo = NULL;

	std::map<int, _TotalLevelSkillSlotInfo>::iterator findIter = m_TotalLevelSlotInfoList.find(nSlotIndex);
	if (findIter != m_TotalLevelSlotInfoList.end())
		pInfo = &(findIter->second);

	return pInfo;
}

bool CDnTotalLevelSkillSystem::IsActivateSlot(int nSlotIndex)
{
	std::map<int, bool>::iterator findIter = m_TotalLevelSkillSlotActivate.find(nSlotIndex);
	if (findIter != m_TotalLevelSkillSlotActivate.end())
		return findIter->second;
	else
		return false;
}

#endif // PRE_ADD_TOTAL_LEVEL_SKILL