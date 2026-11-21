#include "stdafx.h"
#include "DNCommonDef.h"

#include "DnCostumeMixDataMgr.h"
#ifdef _CLIENT
#include "DnItem.h"
#include "DnParts.h"
#include "DnCommonUtil.h"
#endif // _CLIENT

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCostumeMixDataMgr::CDnCostumeMixDataMgr()
{
#ifdef _CLIENT
	m_CurrentClassId = -1;
#endif

	m_bEnableCostumeMix = true;
	m_bEnableCostumeDesignMix = true;
}

CDnCostumeMixDataMgr::MIX_RESULT_LIST& CDnCostumeMixDataMgr::GetResultItemList(int classType, int equipType)
{
	std::map<int, MIXPARTS_RESULT_LIST>::iterator resIter = m_CostumeResultList.find(classType);
	if (resIter != m_CostumeResultList.end())
	{
		MIXPARTS_RESULT_LIST& mixList = (*resIter).second;
		MIXPARTS_RESULT_LIST::iterator partsResIter = mixList.find(equipType);
		if (partsResIter != mixList.end())
			return (*partsResIter).second;

		MIX_RESULT_LIST curList;
		mixList.insert(std::make_pair(equipType, curList));

		MIXPARTS_RESULT_LIST::iterator retIter = mixList.find(equipType);
		return (*retIter).second;
	}

	MIXPARTS_RESULT_LIST resultList;
	MIX_RESULT_LIST itemList;
	resultList.insert(std::make_pair(equipType, itemList));
	m_CostumeResultList.insert(std::make_pair(classType, resultList));

	std::map<int, MIXPARTS_RESULT_LIST>::iterator tmpIter = m_CostumeResultList.find(classType);
	MIXPARTS_RESULT_LIST& tmpList = (*tmpIter).second;
	MIXPARTS_RESULT_LIST::iterator retIter = tmpList.find(equipType);

	return (*retIter).second;

}

const CDnCostumeMixDataMgr::MIX_RESULT_LIST* CDnCostumeMixDataMgr::GetMixResultItemList(int classType, int equipType) const
{
	std::map<int, MIXPARTS_RESULT_LIST>::const_iterator resIter = m_CostumeResultList.find(classType);
	if (resIter != m_CostumeResultList.end())
	{
		const MIXPARTS_RESULT_LIST& mixList = (*resIter).second;
		MIXPARTS_RESULT_LIST::const_iterator partsResIter = mixList.find(equipType);
		if (partsResIter != mixList.end())
			return &((*partsResIter).second);
	}

	return NULL;
}

bool CDnCostumeMixDataMgr::LoadData()
{
	if (IsEnableCostumeMix() == false)
		return true;

	DNTableFileFormat* pItemDropTable = GetDNTable(CDnTableDB::TITEMDROP);
	if (pItemDropTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"ItemDropTable.ext failed\r\n");
		return false;
	}

	DNTableFileFormat* pCosMixSox = GetDNTable(CDnTableDB::TCOSMIX);
	if (pCosMixSox == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"CosmixTable.ext.ext failed\r\n");
		return false;
	}


#ifndef _CLIENT
	int nPrevMinScore = 0;
#endif
	int id = 0;
	for (; id < pCosMixSox->GetItemCount(); ++id)
	{
		int nTableID		= pCosMixSox->GetItemID(id);
		int nClassType		= pCosMixSox->GetFieldFromLablePtr(nTableID, "_Class")->GetInteger();
		nClassType++; // note by kalliste : class type differs between table value and source define value
		int nEquipType		= pCosMixSox->GetFieldFromLablePtr(nTableID, "_EquipType")->GetInteger();
		int nDropID			= pCosMixSox->GetFieldFromLablePtr(nTableID, "_DropID")->GetInteger();

#ifndef _CLIENT
		TCostumeMix mixData;
		mixData.nMinScore	= pCosMixSox->GetFieldFromLablePtr(nTableID, "_MinScore")->GetInteger();
		mixData.nDropID		= nDropID;
		m_CosMixDropList.insert(std::make_pair(std::make_pair(nClassType, nEquipType), mixData));
#endif

		MIX_RESULT_LIST& addItemList = GetResultItemList(nClassType, nEquipType);
		std::set<int> tempItemDropGroupList;
		FillList(addItemList, tempItemDropGroupList, pItemDropTable, nDropID, 0);
	}

	DNTableFileFormat* pAbilitySox = GetDNTable(CDnTableDB::TCOSMIXABILITY);
	if (pAbilitySox == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"CosMixAbilityTable.ext failed\r\n");
		return false;
	}


#ifdef _CLIENT
	DNTableFileFormat* pPotenSox = GetDNTable(CDnTableDB::TPOTENTIAL);
	if (pPotenSox == NULL)
	{
		_ASSERT(0);
		return false;
	}

	int presentPotenId = -1;
	for (id = 0; id < pAbilitySox->GetItemCount(); ++id)
	{
		int nTableID	= pAbilitySox->GetItemID(id);
		int rank		= pAbilitySox->GetFieldFromLablePtr(nTableID, "_Rank")->GetInteger();
		int partsId		= pAbilitySox->GetFieldFromLablePtr(nTableID, "_Parts")->GetInteger();
		if (rank == ITEMRANK_C)
		{
			presentPotenId = pAbilitySox->GetFieldFromLablePtr(nTableID, "_PotentialID")->GetInteger();
			if (presentPotenId < 0)
			{
				_ASSERT(0);
				continue;
			}

			std::vector<int> nVecItemID;
			pPotenSox->GetItemIDListFromField("_PotentialID", presentPotenId, nVecItemID);

			std::string str;
			std::vector<SAddAbility> addAbilityUnitList;
			int i = 1;

			std::vector<int>::const_iterator iter = nVecItemID.begin();
			for (; iter != nVecItemID.end(); ++iter, ++i)
			{
				int itemId = *iter;
				int nStateType = pPotenSox->GetFieldFromLablePtr(itemId, "_State1")->GetInteger();
				if (nStateType == -1) break;
				char *szValue = pPotenSox->GetFieldFromLablePtr(itemId, "_State1Value")->GetString();

				SAddAbility ability;
				ability.abilityNum = nStateType;
				ability.abilityValue = (float)atof(szValue);
				ability.potenOffset = i;

				addAbilityUnitList.push_back(ability);
			}

			m_CostumeAddAbility.insert(std::make_pair(partsId, addAbilityUnitList));
		}
	}
#else // _CLIENT
	for (id = 0; id < pAbilitySox->GetItemCount(); ++id)
	{
		int nTableID	= pAbilitySox->GetItemID(id);

		int rank		= pAbilitySox->GetFieldFromLablePtr(nTableID, "_Rank")->GetInteger();
		int potentialID	= pAbilitySox->GetFieldFromLablePtr(nTableID, "_PotentialID")->GetInteger();
		int partsId		= pAbilitySox->GetFieldFromLablePtr(nTableID, "_Parts")->GetInteger();
		m_PotentialList.insert(std::make_pair(std::pair<int, int>(partsId, rank), potentialID));
	}

	DNTableFileFormat* pGachaInfoTable = GetDNTable(CDnTableDB::TGACHAINFO);
	if (pGachaInfoTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"gachatable.ext failed\r\n");
		return false;
	}


	for (id = 0; id < pGachaInfoTable->GetItemCount(); ++id)
	{
		int nTableID = pGachaInfoTable->GetItemID(id);
		int nEnableDesignMix = pGachaInfoTable->GetFieldFromLablePtr(nTableID, "_UseCompound")->GetInteger();

		m_CosDesignMixEnableMap.insert(std::make_pair(nTableID, nEnableDesignMix));
	}
#endif // _CLIENT

	return true;
}

bool CDnCostumeMixDataMgr::FillList(CDnCostumeMixDataMgr::MIX_RESULT_LIST& addItemList, std::set<int>& tempGroupList, DNTableFileFormat* pItemDropSox, int nDropID, int depthCounter)
{
	int dropIdIdx = 1;
	for (; dropIdIdx < MAXITEMDROPFIELD + 1; ++dropIdIdx)
	{
		std::string groupField = FormatA("_IsGroup%d", dropIdIdx);
		std::string itemFieldString = FormatA("_Item%dIndex", dropIdIdx);

		DNTableCell* pItemField = pItemDropSox->GetFieldFromLablePtr(nDropID, itemFieldString.c_str());
		DNTableCell* pGroupField = pItemDropSox->GetFieldFromLablePtr(nDropID, groupField.c_str());
		if (pItemField == NULL || pGroupField == NULL)
		{
			_ASSERT(0);
			g_Log.Log(LogType::_ERROR, L"[COSTUMEMIX_ERROR] dropId:%d - dropItemField:_Item%dIndex or dropGroupField NOT FOUND\r\n", nDropID, dropIdIdx);
			return false;
		}

		int itemId = pItemField->GetInteger();
		bool bGroup = pGroupField->GetInteger() ? true : false;

		if (itemId != ITEMCLSID_NONE)
		{
			if (bGroup)
			{
				std::set<int>::iterator tgIter = std::find(tempGroupList.begin(), tempGroupList.end(), itemId);
				if (tgIter != tempGroupList.end())
					continue;

				if (depthCounter + 1 > ITEMDROP_DEPTH)
				{
					g_Log.Log(LogType::_ERROR, L"[COSTUMEMIX_ERROR] itemId(%d) depthCounter(%d) is over the ITEMDROP_DEPTH(%d)\r\n", itemId, depthCounter, ITEMDROP_DEPTH);
					continue;
				}

				tempGroupList.insert(itemId);
				FillList(addItemList, tempGroupList, pItemDropSox, itemId, depthCounter + 1);
			}
			else
			{
				DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TITEM);
				if (pSox == NULL)
					return false;

				DNTableFileFormat* pGachaInfoTable = GetDNTable(CDnTableDB::TGACHAINFO);
				if (pGachaInfoTable == NULL)
					return false;

				if (pSox->IsExistItem(itemId))
				{
					// todo by kalliste : Check whether to validate _ComposeType in item table
#ifdef _CLIENT
					eItemRank itemRank = (eItemRank)(pSox->GetFieldFromLablePtr(itemId, "_Rank" )->GetInteger());
					if (itemRank >= ITEMRANK_D && itemRank < ITEMRANK_B)
					{
						SResultItemInfo info;
						info.itemId = itemId;
						const DNTableCell* pField = pGachaInfoTable->GetFieldFromLablePtr(itemId, "_sort");
						if (pField == NULL)
						{
							//_ASSERT(0);
							//g_Log.Log(LogType::_ERROR, L"[COSTUMEMIX_ERROR] ItemId:%d item NOT FOUND in TCOSMIXSORT\r\n", itemId);
							continue;
						}

						info.showPriority = pField->GetInteger();
						addItemList.insert(info);
					}
#else // _CLIENT
					eItemRank itemRank = (eItemRank)(pSox->GetFieldFromLablePtr(itemId, "_Rank" )->GetInteger());
					if (itemRank >= ITEMRANK_D && itemRank < ITEMRANK_B)
					{
						SResultItemInfo info;
						info.itemId = itemId;
						const DNTableCell* pField = pGachaInfoTable->GetFieldFromLablePtr(itemId, "_sort");
						if (pField == NULL)
						{
							//_ASSERT(0);
							//g_Log.Log(LogType::_ERROR, L"[COSTUMEMIX_ERROR] ItemId:%d item NOT FOUND in TCOSMIXSORT\r\n", itemId);
							continue;
						}

						info.showPriority = pField->GetInteger();
						addItemList.insert(info);
					}
#endif
				}
				else
				{
					//_ASSERT(0);
					g_Log.Log(LogType::_ERROR, L"[COSTUMEMIX_ERROR] ItemId:%d item NOT FOUND in ITEMTABLE\r\n", itemId);
					return false;
				}
			}
		}
	}

	return true;
}

#ifdef _CLIENT
void CDnCostumeMixDataMgr::SetCurrentClassId(int classId)
{
	if (CommonUtil::IsValidCharacterClassId(classId) == false)
	{
		m_CurrentClassId = -1;
		return;
	}

	m_CurrentClassId = classId;
}

const std::vector<CDnCostumeMixDataMgr::SAddAbility>* CDnCostumeMixDataMgr::GetMixAbilityList(int partsId) const
{
	std::map<int, std::vector<SAddAbility> >::const_iterator iter = m_CostumeAddAbility.find(partsId);
	if (iter != m_CostumeAddAbility.end())
	{
		const std::vector<SAddAbility>& abilityList = (*iter).second;
		return &abilityList;
	}

	return NULL;
}
#else
int CDnCostumeMixDataMgr::GetMixDropTableID(int classType, int equipType, int weight) const
{
	int result = -1;
	std::multimap<std::pair<int, int>, TCostumeMix>::const_iterator iter = m_CosMixDropList.lower_bound(std::pair<int, int>(classType, equipType));
	for (; iter != m_CosMixDropList.upper_bound(std::pair<int, int>(classType, equipType)); ++iter)
	{
		const TCostumeMix& mixData = (*iter).second;
		if (weight >= mixData.nMinScore)
			result = mixData.nDropID;
	}

	return result;
}

int	CDnCostumeMixDataMgr::GetPotentialID(int partsId, int rank) const
{
	std::map<std::pair<int, int>, int>::const_iterator iter = m_PotentialList.find(std::pair<int, int>(partsId, rank));
	if (iter != m_PotentialList.end())
		return (*iter).second;

	return -1;
}
#endif // _CLIENT

#ifdef _CLIENT
bool CDnCostumeMixDataMgr::IsEnableCostumeDesignMixStuff(const CDnItem* pStuff) const
#else
bool CDnCostumeMixDataMgr::IsEnableCostumeDesignMixStuff(const TItem* pStuff) const
#endif
{
	if (pStuff == NULL)
		return false;

#ifdef _CLIENT
	DNTableFileFormat* pGachaSox = GetDNTable( CDnTableDB::TGACHAINFO );
	if (pGachaSox == NULL)
		return false;

	DNTableCell* pField = pGachaSox->GetFieldFromLablePtr(pStuff->GetClassID(), "_UseCompound");
	if (pField == NULL)
		return false;

	bool bEnableStuff = (pField->GetInteger()) ? true : false;
	if (bEnableStuff == false || pStuff->IsEternityItem() == false || pStuff->GetLookItemID() != 0)
		return false;

	return true;
#else
	std::map<int, int>::const_iterator iter = m_CosDesignMixEnableMap.find(pStuff->nItemID);
	if (iter != m_CosDesignMixEnableMap.end())
	{
		int enableStuff = (*iter).second;
		return (enableStuff > 0 && pStuff->bEternity && pStuff->nLookItemID == 0);
	}

	return false;
#endif
}

#ifdef _CLIENT
bool CDnCostumeMixDataMgr::IsEnableCostumeMixStuff(const CDnParts* pStuff) const
{
	if (pStuff == NULL)
		return false;

	bool bRet = true;
	if (pStuff->IsEnableCostumeMixItem() == false)
		return false;

	if (pStuff->GetLookItemID() != 0)
		return false;

	int classId = GetCurrentClassId();
	const CDnCostumeMixDataMgr::MIX_RESULT_LIST* pItemList = GetMixResultItemList(classId, pStuff->GetPartsType());
	if (pItemList && pItemList->size() <= 0)
		bRet = false;

	return bRet;
}
#endif