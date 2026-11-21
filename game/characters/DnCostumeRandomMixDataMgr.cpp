#include "stdafx.h"
#include "DnCostumeRandomMixDataMgr.h"

#ifdef PRE_ADD_COSRANDMIX

#include "DnCommonUtil.h"

//#if defined (_SERVER) || defined (_CLIENT)
#include "DNCommonDef.h"
//#endif

#ifdef _SERVER
#include "DNGameDataManager.h"
#elif _CLIENT
#include "DnItemSlotButton.h"
#include "DnItem.h"
#include "DnItemTask.h"
#endif

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
#include <boost/tuple/tuple_comparison.hpp>
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCostumeRandomMixDataMgr::CDnCostumeRandomMixDataMgr()
	: m_bEnableCosRandomMix(true)
	, m_nCosMixProbabilityResultCount(21)
{
#ifdef _CLIENT
	#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	m_OpenType = CostumeMix::RandomMix::OpenFail;
	#endif
	#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	m_nMixerEnablePartsType = CostumeMix::RandomMix::MixerCanMixPartsAll;
	#endif
#endif
}

bool CDnCostumeRandomMixDataMgr::LoadData()
{
	if (IsEnableCosRandomMix() == false)
		return true;

#if defined (_SERVER) || defined (_CLIENT)
	DNTableFileFormat* pCosRandMixTable = GetDNTable(CDnTableDB::TCOSMIXINFO);
	if (pCosRandMixTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"costumemixinfo.ext failed\r\n");
		return false;
	}

	DNTableFileFormat* pItemTable = GetDNTable(CDnTableDB::TITEM);
	if (pItemTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"itemtable.ext failed\r\n");
		return false;
	}

	DNTableFileFormat* pPartsTable = GetDNTable(CDnTableDB::TPARTS);
	if (pPartsTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"partstable.ext failed\r\n");
		return false;
	}

	DNTableFileFormat* pCosRandMixProbabilityTable = GetDNTable(CDnTableDB::TCOSRANDOMMIXINFO);
	if (pCosRandMixProbabilityTable == NULL)
	{
		g_Log.Log( LogType::_FILELOG, L"costumemixtable.ext failed\r\n");
		return false;
	}
#else
	DNTableFileFormat* pCosRandMixTable = LoadExtFile("costumemixinfo.ext");
	if (pCosRandMixTable == NULL)
	{
#ifdef _TABLEVERIFIER
		EXPECT_FALSE(true) << "CANNOT load costumemixinfo.ext";
#endif
		return false;
	}

	DNTableFileFormat* pItemTable = LoadExtFile("itemtable.ext");
	if (pItemTable == NULL)
	{
#ifdef _TABLEVERIFIER
		EXPECT_FALSE(true) << "CANNOT load itemtable.ext";
#endif
		return false;
	}

	DNTableFileFormat* pPartsTable = LoadExtFile("partstable.ext");
	if (pPartsTable == NULL)
	{
#ifdef _TABLEVERIFIER
		EXPECT_FALSE(true) << "CANNOT load partstable.ext";
#endif
		return false;
	}

	DNTableFileFormat* pCosRandMixProbabilityTable = LoadExtFile("costumemixtable.ext");
	if (pCosRandMixProbabilityTable == NULL)
	{
#ifdef _TABLEVERIFIER
		EXPECT_FALSE(true) << "CANNOT load costumemixtable.ext";
#endif
		return false;
	}
#endif

	m_CosRandomMixResultList.clear();
#ifdef PRE_FIX_68828
	m_CosRandomMixResultListForRef.clear();
#endif

	int i = 0;
	for (; i < pCosRandMixTable->GetItemCount(); ++i)
	{
		int nItemID = pCosRandMixTable->GetItemID(i);
		std::vector<int> enableJobList;
		CommonUtil::GetItemJobUsable(enableJobList, nItemID, pItemTable);

		DNTableCell* pPartsCell = pPartsTable->GetFieldFromLablePtr(nItemID, "_Parts");
		if (pPartsCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO %d item in parts table\n", nItemID);
			continue;
		}
		int nPartsId = pPartsCell->GetInteger();

		DNTableCell* pRankCell = pCosRandMixTable->GetFieldFromLablePtr(nItemID, "_MixRank");
		if (pRankCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixRank : %d item\n", nItemID);
			continue;
		}
		int nRankId = pRankCell->GetInteger();

		DNTableCell* pWorldMsgCell = pCosRandMixTable->GetFieldFromLablePtr(nItemID, "_MixMsg");
		if (pWorldMsgCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixMsg : %d item\n", nItemID);
			continue;
		}
		bool bShowWorldMsg = (pWorldMsgCell->GetInteger() > 0) ? true : false;

		DNTableCell* pEnableCell = pCosRandMixTable->GetFieldFromLablePtr(nItemID, "_MixUseCompound");
		if (pEnableCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixUseCompound : %d item\n", nItemID);
			continue;
		}

		bool bEnableMix = (pEnableCell->GetInteger() == TRUE) ? true : false;
		if (bEnableMix)
		{
			SRandomMixEnableData data;
			data.nRank = nRankId;
			data.bShowWorldMsg = bShowWorldMsg;
			m_CosRandomMixEnableList.insert(std::make_pair(nItemID, data));
		}

		DNTableCell* pResultCell = pCosRandMixTable->GetFieldFromLablePtr(nItemID, "_MixResultAble");
		if (pResultCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixResultAble : %d item\n", nItemID);
			continue;
		}

		bool bResultEnable = (pResultCell->GetInteger() == TRUE) ? true : false;
		if (bResultEnable == false)
			continue;

#if defined (PRE_FIX_68828) && defined (_SERVER)
		SRandomMixEnableData resultData;
		resultData.nRank = nRankId;
		resultData.bShowWorldMsg = bShowWorldMsg;
		m_CosRandomMixResultListForRef.insert(std::make_pair(nItemID, resultData));
#endif

		std::vector<int>::const_iterator jobIter = enableJobList.begin();
		for (; jobIter != enableJobList.end(); ++jobIter)
		{
			const int& nJobId = (*jobIter);
			COSTUME_RANDOM_MIX_LIST::iterator iter = m_CosRandomMixResultList.find(std::make_pair(nJobId, nPartsId));
			if (iter != m_CosRandomMixResultList.end())
			{
				COSTUME_RANDOM_MIX_RANKLIST& rankList = (*iter).second;
				COSTUME_RANDOM_MIX_RANKLIST::iterator rankListIter = rankList.find(nRankId);
				if (rankListIter != rankList.end())
				{
					std::vector<ITEMCLSID>& rankItemList = (*rankListIter).second;
					rankItemList.push_back(nItemID);
				}
				else
				{
					std::vector<ITEMCLSID> rankItemList;
					MakeRankList(rankList, rankItemList, nRankId, nItemID);
				}
			}
			else
			{
				std::vector<ITEMCLSID> rankItemList;
				COSTUME_RANDOM_MIX_RANKLIST rankList;
				MakeRankList(rankList, rankItemList, nRankId, nItemID);

				m_CosRandomMixResultList.insert(std::make_pair(std::make_pair(nJobId, nPartsId), rankList));
			}
		}
	}

	m_CosRandomMixProbabilityList.clear();
	for (i = 0; i < pCosRandMixProbabilityTable->GetItemCount(); ++i)
	{
		int nItemID = pCosRandMixProbabilityTable->GetItemID(i);
		DNTableCell* pRank1Cell = pCosRandMixProbabilityTable->GetFieldFromLablePtr(nItemID, "_MixRank1");
		if (pRank1Cell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixRank1 field\n");
			continue;
		}
		int nMixRank1 = pRank1Cell->GetInteger();

		DNTableCell* pRank2Cell = pCosRandMixProbabilityTable->GetFieldFromLablePtr(nItemID, "_MixRank2");
		if (pRank2Cell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixRank2 field\n");
			continue;
		}
		int nMixRank2 = pRank2Cell->GetInteger();

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
		DNTableCell* pMixTypeCell = pCosRandMixProbabilityTable->GetFieldFromLablePtr(nItemID, "_MixType");
		if (pMixTypeCell == NULL)
		{
			OutputDebug("[RANDMIX] There is NO _MixType field\n");
			continue;
		}
		int nMixType = pMixTypeCell->GetInteger();
		COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(boost::make_tuple(nMixType, nMixRank1, nMixRank2));
#else
		COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(std::make_pair(nMixRank1, nMixRank2));
#endif
		if (iter != m_CosRandomMixProbabilityList.end())
		{
			OutputDebug("[RANDMIX] Duplicate Costume Random Mix Probability List KEY (_MixRank1:%d, _MixRank2:%d)\n", nMixRank1, nMixRank2);
			continue;
		}

		std::vector<float> probList;
		int j = 0;
		std::string fieldName;
		float fRateSum = 0.f;
		for (; j < m_nCosMixProbabilityResultCount; ++j)
		{
			fieldName = FormatA("_ResultRate%d", j);
			DNTableCell* pCell = pCosRandMixProbabilityTable->GetFieldFromLablePtr(nItemID, fieldName.c_str());
			if (pCell == NULL)
			{
				if (fRateSum >= 100.1f || fRateSum < 99.9f)
				{
					OutputDebug("[RANDMIX] Costume Random Mix Probability SUM Error: rowId:%d value:%f\n", nItemID, fRateSum);
#if defined (_SERVER) || defined (_CLIENT)
					g_Log.Log(LogType::_ERROR, L"[RANDMIX] Costume Random Mix Probability SUM Error: rowId:%d value:%f\n", nItemID, fRateSum);
#endif
				}
				return false;
			}

			float fCurProb = pCell->GetFloat();
			probList.push_back(fCurProb);
			fRateSum += fCurProb;
		}

		if (fRateSum >= 100.1f || fRateSum < 99.9f)
		{
			OutputDebug("[RANDMIX] Costume Random Mix Probability SUM Error: rowId:%d value:%f\n", nItemID, fRateSum);
#if defined (_SERVER) || defined (_CLIENT)
			g_Log.Log(LogType::_ERROR, L"[RANDMIX] Costume Random Mix Probability SUM Error: rowId:%d value:%f\n", nItemID, fRateSum);
#endif
			return false;
		}

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
		boost::tuple<int, int, int> keyTuple = boost::make_tuple(nMixType, nMixRank1, nMixRank2);
		m_CosRandomMixProbabilityList.insert(std::make_pair(keyTuple, probList));
#else
		std::pair<int, int> keyPair = std::make_pair(nMixRank1, nMixRank2);
		m_CosRandomMixProbabilityList.insert(std::make_pair(keyPair, probList));
#endif

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
		DNTableCell* pFeeCell = pCosRandMixProbabilityTable->GetFieldFromLablePtr(nItemID, "_CompoundFee");
		if (pFeeCell == NULL)
		{
			OutputDebug("[RANDMIX] Costume Random Mix NO Fee Error: rowId:%d\n", nItemID);
#if defined (_SERVER) || defined (_CLIENT)
			g_Log.Log(LogType::_ERROR, L"[RANDMIX] Costume Random Mix NO Fee Error: rowId:%d\n", nItemID);
#endif
			continue;
		}

		INT64 compoundFee = pFeeCell->GetInteger();
#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
		m_CosRandomMixFeeList.insert(std::make_pair(keyTuple, compoundFee));
#else
		m_CosRandomMixFeeList.insert(std::make_pair(keyPair, compoundFee));
#endif // PRE_ADD_COSRANDMIX_MIXERTYPE

#endif
	}

#ifdef _SERVER
	m_Random.srand(timeGetTime());
#endif

	return true;
}

void CDnCostumeRandomMixDataMgr::MakeRankList(COSTUME_RANDOM_MIX_RANKLIST& rankResultList, std::vector<ITEMCLSID>& itemResultList, const int& rank, const ITEMCLSID& itemId)
{
	itemResultList.push_back(itemId);
	rankResultList.insert(std::make_pair(rank, itemResultList));
}

#ifdef _CLIENT
bool CDnCostumeRandomMixDataMgr::IsEnableCostumeRandomMixStuff(const CDnItem* pStuff) const
{
	if (pStuff == NULL || pStuff->IsEternityItem() == false)
		return false;

	std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixEnableList.find(pStuff->GetClassID());

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	if (iter == m_CosRandomMixEnableList.end())
		return false;

	int nMixerEnableParts = GetMixerEnablePartsType();
	if (nMixerEnableParts == CostumeMix::RandomMix::MixerCanMixPartsAll)
		return true;

	int nCurPartsType = (int)CDnParts::GetPartsType(pStuff->GetClassID());
	if (nMixerEnableParts == CostumeMix::RandomMix::MixerCanMixBasicParts)
	{
		return (nCurPartsType >= 0 && nCurPartsType < CDnParts::OverlapPartsTypeEnum_Amount);
	}
	else
	{
		return (nMixerEnableParts == nCurPartsType);
	}
#else
	return (iter != m_CosRandomMixEnableList.end());
#endif
}

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
void CDnCostumeRandomMixDataMgr::SetMixerEnablePartsType(int nPartsType)
{
	m_nMixerEnablePartsType = nPartsType;
}
#endif

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY

bool CDnCostumeRandomMixDataMgr::MakePreviewItemList(std::vector<SRandomProgressData>& itemList, const CDnParts::PartsTypeEnum& partsType) const
{
	itemList.clear();

	if (CDnItemTask::IsActive() && CDnActor::s_hLocalActor)
	{
		typedef SRandomProgressData SRPData;

		int classId = ((CDnPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->GetClassID();
		const COSTUME_RANDOM_MIX_RANKLIST* pCommonItemRankList = GetRandomMixRankList(0, (int)partsType);
		const COSTUME_RANDOM_MIX_RANKLIST* pRankList = GetRandomMixRankList(classId, (int)partsType);
		if (pRankList == NULL && pCommonItemRankList == NULL)
			return false;

		if (pRankList)
			DoMakePreviewItemList(itemList, *pRankList);
		if (pCommonItemRankList)
			DoMakePreviewItemList(itemList, *pCommonItemRankList);
	}

	return true;
}

void CDnCostumeRandomMixDataMgr::DoMakePreviewItemList(std::vector<SRandomProgressData>& itemList, const COSTUME_RANDOM_MIX_RANKLIST& srcList) const
{
	COSTUME_RANDOM_MIX_RANKLIST::const_iterator iter = srcList.begin();
	for (; iter != srcList.end(); ++iter)
	{
		const std::vector<ITEMCLSID>& subList = (*iter).second;
		std::vector<ITEMCLSID>::const_iterator subIter = subList.begin();
		for (; subIter != subList.end(); ++subIter)
		{
			SRandomProgressData data;
			data.progressItem = (*subIter);
			itemList.push_back(data);
		}
	}
}

#ifdef _CLIENT
void CDnCostumeRandomMixDataMgr::OnStartMixDlg(CostumeMix::RandomMix::eOpenType type)
{
	m_OpenType = type;
}
#endif

#else // PRE_ADD_COSRANDMIX_ACCESSORY

								bool CDnCostumeRandomMixDataMgr::MakePreviewItemList(std::vector<SRandomProgressData>& itemList, const CDnParts::PartsTypeEnum& partsType, const std::vector<ITEMCLSID>& stuffItemIDs) const
								{
									itemList.clear();

									if (CDnItemTask::IsActive())
									{
										typedef SRandomProgressData SRPData;

										int classId = ((CDnPlayerActor*)CDnActor::s_hLocalActor.GetPointer())->GetClassID();
										const CDnCostumeRandomMixDataMgr::COSTUME_RANDOM_MIX_RANKLIST* pRankList = GetRandomMixRankList(classId, (int)partsType);
										if (pRankList == NULL)
											return false;

										if (stuffItemIDs.size() != MAX_COSTUME_RANDOMMIX_STUFF)
										{
											return false;
										}
										int i = 0;
										int nStuffRank[MAX_COSTUME_RANDOMMIX_STUFF] = {-1, };
										for (; i < MAX_COSTUME_RANDOMMIX_STUFF; ++i)
											nStuffRank[i] = GetRandomMixStuffItemRank(stuffItemIDs[i]);

						#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
										const std::vector<float>* pProbList = GetRandomMixProbableList(partsType, nStuffRank[0], nStuffRank[1]);
						#else
										const std::vector<float>* pProbList = GetRandomMixProbableList(nStuffRank[0], nStuffRank[1]);
						#endif
										if (pProbList == NULL)
											return false;

										float fSum = 0.f;
										CDnCostumeRandomMixDataMgr::COSTUME_RANDOM_MIX_RANKLIST::const_iterator iter = pRankList->begin();
										for (; iter != pRankList->end(); ++iter)
										{
											int nRank = (*iter).first;
											if (nRank < 0 || nRank > (int)pProbList->size())
												continue;

											SRPData data;
											data.fShowRatio = (*pProbList)[nRank];
											data.progressItemList = (*iter).second;

											fSum += data.fShowRatio;
											itemList.push_back(data);
										}

										if (fSum < 99.9f || fSum > 100.1f)
										{
											int nLargestIndex = 0;
											float fLargest = 0.f, fOtherSum = 0.f;
											int i = 0;
											for (; i < (int)itemList.size(); ++i)
											{
												SRPData& cur = itemList[i];
												if (cur.fShowRatio > fLargest)
												{
													fLargest = cur.fShowRatio;
													nLargestIndex = i;
												}
												else
												{
													fOtherSum += cur.fShowRatio;
												}
											}

											if (nLargestIndex >= 0)
												itemList[nLargestIndex].fShowRatio = (100.f - fOtherSum);
										}
									}

									return true;
								}

#endif // PRE_ADD_COSRANDMIX_ACCESSORY

#endif // _CLIENT

#ifdef _SERVER
bool CDnCostumeRandomMixDataMgr::IsEnableCostumeRandomMixStuff(const std::vector<TItem>& stuffs, int userClassId) const
{
	if (stuffs.empty() || (int)stuffs.size() != MAX_COSTUME_RANDOMMIX_STUFF)
		return false;

	int nPartsType = 0;
	int i = 0;
	for (; i < MAX_COSTUME_RANDOMMIX_STUFF; ++i)
	{
		const TItem& stuff = stuffs[i];

		if (stuff.bEternity == false)
			return false;

		std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixEnableList.find(stuff.nItemID);

		if (iter == m_CosRandomMixEnableList.end())
			return false;

		if (stuff.wCount != 1)
			return false;	// Single item only

		TItemData *pItemData = g_pDataManager->GetItemData(stuff.nItemID);
		if (pItemData == NULL)
			return false;

		if (g_pDataManager->IsItemNeedJob(stuff.nItemID, userClassId) == false)
			return false;

		TPartData *pPartData = g_pDataManager->GetPartData(pItemData->nItemID);
		if (!pPartData)
			return false;

		if (i != 0 && nPartsType != pPartData->nParts)
			return false;

		nPartsType = pPartData->nParts;
	}

	int stuffRank0 = GetRandomMixStuffItemRank(stuffs[0].nItemID);
	int stuffRank1 = GetRandomMixStuffItemRank(stuffs[1].nItemID);

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	const std::vector<float>* pCurRankProbList = GetRandomMixProbableList(nPartsType, stuffRank0, stuffRank1);
#else
	const std::vector<float>* pCurRankProbList = GetRandomMixProbableList(stuffRank0, stuffRank1);
#endif
	if (pCurRankProbList == NULL)
		return false;

	return true;
}

int CDnCostumeRandomMixDataMgr::DoRandomMix(const INT64* pStuffSerials, int stuffCount, int userClassId, const CDNUserItem& userItem, const INT64& mixerCharacterDBID, eError& lastError)
{
	if (pStuffSerials == NULL || stuffCount != MAX_COSTUME_RANDOMMIX_STUFF)
	{
		g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : ILLEGAL STUFF_ITEM - NULL or NOT_ENOUGH_STUFFCOUNT(%d)\r\n", mixerCharacterDBID, stuffCount);
		return 0;
	}

	std::vector<TItem> stuffIDs;
	int i = 0;
	for (; i < stuffCount; ++i)
	{
		const TItem* pItem = userItem.GetCashInventory(pStuffSerials[i]);
		if (pItem == NULL)
		{
			g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : ILLEGAL STUFF_ITEM - THERE IS NO STUFF ITEM IN CASH INVENTORY! SERIAL:(%I64d)\r\n", mixerCharacterDBID, pStuffSerials[i]);
			return 0;
		}

		stuffIDs.push_back(*pItem);
	}

	return DoRandomMix(stuffIDs, userClassId, userItem, mixerCharacterDBID, lastError);
}

int CDnCostumeRandomMixDataMgr::DoRandomMix(const std::vector<TItem>& stuffItemIDs, int userClassId, const CDNUserItem& userItem, const INT64& mixerCharacterDBID, eError& lastError)
{
	std::vector<int> nStuffsRankList;
	int nPartsType = -1;
	GetStuffsRankAndPartsType(nStuffsRankList, nPartsType, stuffItemIDs, userItem, mixerCharacterDBID);
	int stuffRankListSize = (int)nStuffsRankList.size();
	if (stuffRankListSize <= 0 || stuffRankListSize > MAX_COSTUME_RANDOMMIX_STUFF)
		return 0;

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	const std::vector<float>* pCurRankProbList = GetRandomMixProbableList(nPartsType, nStuffsRankList[0], nStuffsRankList[1]);
#else
	const std::vector<float>* pCurRankProbList = GetRandomMixProbableList(nStuffsRankList[0], nStuffsRankList[1]);
#endif
	if (pCurRankProbList == NULL)
	{
		g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : NO PROBABILITY(%d, %d)\r\n", mixerCharacterDBID, nStuffsRankList[0], nStuffsRankList[1]);
		return 0;
	}

	const std::vector<float>& curRankProbList = *pCurRankProbList;
	float rankValue = m_Random.rand_float(0.f, 100.f);

	int resultRank = -1;
	float fRangeAccumulator = 0.f;
	int i = 0;
	for (i = 0; i < (int)curRankProbList.size(); ++i)
	{
		fRangeAccumulator += curRankProbList[i];
		if (rankValue <= fRangeAccumulator)
		{
			resultRank = i;
			break;
		}
	}

	bool bExistClassRank = false;
	COSTUME_RANDOM_MIX_RANKLIST::const_iterator rankIter;
	COSTUME_RANDOM_MIX_LIST::const_iterator resultIter = m_CosRandomMixResultList.find(std::make_pair(userClassId, nPartsType));
	if (resultIter != m_CosRandomMixResultList.end())
	{
		const COSTUME_RANDOM_MIX_RANKLIST& curRankList = (*resultIter).second;
		rankIter = curRankList.find(resultRank);
		if (rankIter != curRankList.end())
			bExistClassRank = true;
	}

	bool bExistCommonRank = false;
	COSTUME_RANDOM_MIX_RANKLIST::const_iterator commonRankIter;
	COSTUME_RANDOM_MIX_LIST::const_iterator commonResultIter = m_CosRandomMixResultList.find(std::make_pair(0, nPartsType));
	if (commonResultIter != m_CosRandomMixResultList.end())
	{
		const COSTUME_RANDOM_MIX_RANKLIST& curCommonRankList = (*commonResultIter).second;
		commonRankIter = curCommonRankList.find(resultRank);
		if (commonRankIter != curCommonRankList.end())
			bExistCommonRank = true;
	}

	if (bExistClassRank == false && bExistCommonRank == false)
	{
		g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : NO RESULT LIST(userClassId(%d), partsType(%d))\r\n", mixerCharacterDBID, userClassId, nPartsType);
		return 0;
	}

	int resultItemId = 0;
	if (bExistCommonRank && bExistClassRank == false)
	{
		const std::vector<int>& commonRankList = (*commonRankIter).second;
		int wholeSize = (int)commonRankList.size();
		if (wholeSize < 1)
		{
			g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : RESULT CALCULATION ERROR\r\n", mixerCharacterDBID);
			return 0;
		}
		int nNumberDrew = m_Random.rand(1, wholeSize);
		nNumberDrew--;
		resultItemId = commonRankList[nNumberDrew];
	}
	else if (bExistClassRank && bExistCommonRank == false)
	{
		const std::vector<int>& rankList = (*rankIter).second;
		int wholeSize = (int)rankList.size();
		if (wholeSize < 1)
		{
			g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : RESULT CALCULATION ERROR\r\n", mixerCharacterDBID);
			return 0;
		}
		int nNumberDrew = m_Random.rand(1, wholeSize);
		nNumberDrew--;
		resultItemId = rankList[nNumberDrew];
	}
	else if (bExistCommonRank && bExistClassRank)
	{
		const std::vector<int>& rankList = (*rankIter).second;
		const std::vector<int>& commonRankList = (*commonRankIter).second;

		int wholeSize = (int)rankList.size() + (int)commonRankList.size();
		if (wholeSize < 1)
		{
			g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : RESULT CALCULATION ERROR\r\n", mixerCharacterDBID);
			return 0;
		}

		int nNumberDrew = m_Random.rand(1, wholeSize);

		if (nNumberDrew > (int)rankList.size())
		{
			int nCommonItemIndexDrew = (nNumberDrew - (int)rankList.size()) - 1;
			if (nCommonItemIndexDrew < 0 || nCommonItemIndexDrew >= (int)commonRankList.size())
			{
				g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : RESULT CALCULATION ERROR\r\n", mixerCharacterDBID);
				return 0;
			}

			resultItemId = commonRankList[nCommonItemIndexDrew];
		}
		else
		{
			int nItemIndexDrew = nNumberDrew - 1;
			if (nItemIndexDrew < 0 || nItemIndexDrew >= (int)rankList.size())
			{
				g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : RESULT CALCULATION ERROR\r\n", mixerCharacterDBID);
				return 0;
			}

			resultItemId = rankList[nItemIndexDrew];
		}
	}

	return resultItemId;
}

void CDnCostumeRandomMixDataMgr::GetStuffsRankAndPartsType(std::vector<int>& nStuffsRankList, int& nPartsType, const std::vector<TItem>& stuffItemIDs, const CDNUserItem& userItem, const INT64& mixerCharacterDBID) const
{

	if (MAX_COSTUME_RANDOMMIX_STUFF != (int)stuffItemIDs.size())
	{
		g_Log.Log(LogType::_ERROR, L"[CDBID:%I64d] COSRANDOMMIX : GetStuffsRankAndPartsType() MAX_COSTUME_RANDOMMIX_STUFF != (int)nStuffItemIDs.size()\r\n", mixerCharacterDBID);
		return;
	}

	int i = 0;
	nPartsType = -1;
	for (; i < (int)stuffItemIDs.size(); ++i)
	{
		int nStuffItemId = stuffItemIDs[i].nItemID;
		std::map<int, SRandomMixEnableData>::const_iterator enableListIter = m_CosRandomMixEnableList.find(nStuffItemId);
		if (enableListIter == m_CosRandomMixEnableList.end())
		{
			g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : ILLEGAL STUFF_ITEM_ID(%d)\r\n", mixerCharacterDBID, nStuffItemId);
			return;
		}

		if (nPartsType == -1)
		{
			const TPartData *pPartData = g_pDataManager->GetPartData(nStuffItemId);
			if (!pPartData)
			{
				g_Log.Log(LogType::_FILELOG, L"[CDBID:%I64d] COSRANDOMMIX : STUFF ITEM IS NOT PARTSITEM(%d)\r\n", mixerCharacterDBID, nStuffItemId);
				return;
			}
			nPartsType = pPartData->nParts;
		}

		const SRandomMixEnableData& data = (*enableListIter).second;
		nStuffsRankList.push_back(data.nRank);
	}
}

#ifdef PRE_FIX_68828
bool CDnCostumeRandomMixDataMgr::IsWorldMessagingItem(int nItemId) const
{
	std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixResultListForRef.find(nItemId);
	if (iter != m_CosRandomMixResultListForRef.end())
	{
		const SRandomMixEnableData& data = (*iter).second;
		return data.bShowWorldMsg;
	}

	return false;
}
#endif

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
const std::vector<float>* CDnCostumeRandomMixDataMgr::GetRandomMixProbableList(int stuffPartsType, int stuffRank1, int stuffRank2) const
{
	int stuffPartsTypeForProbableList = stuffPartsType;
	if (stuffPartsTypeForProbableList < 0)
		return NULL;

	if (IsBasicCashPartsType(stuffPartsTypeForProbableList))
		stuffPartsTypeForProbableList = CASHEQUIPMIN;

	COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(boost::make_tuple(stuffPartsTypeForProbableList, stuffRank1, stuffRank2));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	iter = m_CosRandomMixProbabilityList.find(boost::make_tuple(stuffPartsTypeForProbableList, stuffRank2, stuffRank1));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	return NULL;
}
#else // PRE_ADD_COSRANDMIX_MIXERTYPE
const std::vector<float>* CDnCostumeRandomMixDataMgr::GetRandomMixProbableList(int stuffRank1, int stuffRank2) const
{
	COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(std::make_pair(stuffRank1, stuffRank2));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	iter = m_CosRandomMixProbabilityList.find(std::make_pair(stuffRank2, stuffRank1));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	return NULL;
}
#endif // PRE_ADD_COSRANDMIX_MIXERTYPE

#endif // _SERVER

const CDnCostumeRandomMixDataMgr::COSTUME_RANDOM_MIX_RANKLIST* CDnCostumeRandomMixDataMgr::GetRandomMixRankList(int userClassId, int partsId) const
{
	const COSTUME_RANDOM_MIX_LIST::const_iterator rankListIter = m_CosRandomMixResultList.find(std::make_pair(userClassId, partsId));
	if (rankListIter == m_CosRandomMixResultList.end())
		return NULL;

	return &((*rankListIter).second);
}

int CDnCostumeRandomMixDataMgr::GetRandomMixStuffItemRank(ITEMCLSID itemId) const
{
	std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixEnableList.find(itemId);
	if (iter != m_CosRandomMixEnableList.end())
	{
		const SRandomMixEnableData& data = (*iter).second;
		return data.nRank;
	}

	return -1;
}

#ifdef PRE_FIX_68828
int CDnCostumeRandomMixDataMgr::GetRandomMixResultItemRank(ITEMCLSID itemId) const
{
	std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixResultListForRef.find(itemId);
	if (iter != m_CosRandomMixResultListForRef.end())
	{
		const SRandomMixEnableData& data = (*iter).second;
		return data.nRank;
	}

	return -1;
}
#endif

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY

INT64 CDnCostumeRandomMixDataMgr::GetRandomMixFee(const std::vector<int>& stuffs) const
{
	if (stuffs.size() != MAX_COSTUME_RANDOMMIX_STUFF)
		return -1;

	int nRank[MAX_COSTUME_RANDOMMIX_STUFF] = {0,};
	int i = 0;
#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	int nPartsType = -1, nLastPartsType = -1;
#endif
	for (; i < MAX_COSTUME_RANDOMMIX_STUFF; ++i)
	{
		int nStuffItemId = stuffs[i];
		std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixEnableList.find(nStuffItemId);
		if (iter == m_CosRandomMixEnableList.end())
			return -1;

#if defined (PRE_ADD_COSRANDMIX_MIXERTYPE) && !defined (_TABLEVERIFIER)
		nPartsType = GetPartsType(nStuffItemId);
		if (i != 0 && nPartsType != nLastPartsType)
			return -1;

		nLastPartsType = nPartsType;
#endif
		const SRandomMixEnableData& data = (*iter).second;
		nRank[i] = data.nRank;
	}

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
	if (nPartsType < 0)
		return -1;

#if !defined (_TABLEVERIFIER)
	if (IsBasicCashPartsType(nPartsType))
		nPartsType = CASHEQUIPMIN;
#endif

	COSRANDMIX_FEE_LIST::const_iterator feeIter = m_CosRandomMixFeeList.find(boost::make_tuple(nPartsType, nRank[0], nRank[1]));
#else
	COSRANDMIX_FEE_LIST::const_iterator feeIter = m_CosRandomMixFeeList.find(std::make_pair(nRank[0], nRank[1]));
#endif
	if (feeIter != m_CosRandomMixFeeList.end())
	{
		return (*feeIter).second;
	}
	else
	{
#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
		COSRANDMIX_FEE_LIST::const_iterator convertFeeIter = m_CosRandomMixFeeList.find(boost::make_tuple(nPartsType, nRank[1], nRank[0]));
#else
		COSRANDMIX_FEE_LIST::const_iterator convertFeeIter = m_CosRandomMixFeeList.find(std::make_pair(nRank[1], nRank[0]));
#endif
		if (convertFeeIter != m_CosRandomMixFeeList.end())
			return (*convertFeeIter).second;
	}

	return -1;
}

#else // PRE_ADD_COSRANDMIX_ACCESSORY

#ifdef PRE_ADD_COSRANDMIX_MIXERTYPE
#ifndef _SERVER
const std::vector<float>* CDnCostumeRandomMixDataMgr::GetRandomMixProbableList(int mixType, int stuffRank1, int stuffRank2) const
{
	if (mixType < 0)
		return NULL;

	COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(boost::make_tuple(mixType, stuffRank1, stuffRank2));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	iter = m_CosRandomMixProbabilityList.find(boost::make_tuple(mixType, stuffRank2, stuffRank1));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	return NULL;
}
#endif // _SERVER
#else
const std::vector<float>* CDnCostumeRandomMixDataMgr::GetRandomMixProbableList(int stuffRank1, int stuffRank2) const
{
	COSRANDMIX_PROBABLE_LIST::const_iterator iter = m_CosRandomMixProbabilityList.find(std::make_pair(stuffRank1, stuffRank2));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);

	iter = m_CosRandomMixProbabilityList.find(std::make_pair(stuffRank2, stuffRank1));
	if (iter != m_CosRandomMixProbabilityList.end())
		return &((*iter).second);
	
	return NULL;
}
#endif // PRE_ADD_COSRANDMIX_MIXERTYPE

#endif // PRE_ADD_COSRANDMIX_ACCESSORY

#if defined (PRE_ADD_COSRANDMIX_MIXERTYPE) && !defined (_TABLEVERIFIER)
int CDnCostumeRandomMixDataMgr::GetPartsType(int nItemId) const
{
#ifdef _SERVER
	const TPartData *pPartData = g_pDataManager->GetPartData(nItemId);
	if (!pPartData)
		return -1;

	return pPartData->nParts;
#else
	return (int)CDnParts::GetPartsType(nItemId);
#endif
}

bool CDnCostumeRandomMixDataMgr::IsBasicCashPartsType(int nPartsType) const
{
#ifdef _SERVER
	return (nPartsType >= CASHEQUIPMIN && nPartsType <= CASHEQUIP_FOOT);
#else
	return (nPartsType >= CDnParts::CashHelmet && nPartsType <= CDnParts::CashFoot);
#endif
}
#endif // PRE_ADD_COSRANDMIX_MIXERTYPE

#ifdef PRE_FIX_68828
#else
#if defined(_SERVER)
bool CDnCostumeRandomMixDataMgr::IsWorldMessagingItem(int nItemId) const
{
	std::map<int, SRandomMixEnableData>::const_iterator iter = m_CosRandomMixEnableList.find(nItemId);
	if (iter != m_CosRandomMixEnableList.end())
	{
		const SRandomMixEnableData& data = (*iter).second;
		return data.bShowWorldMsg;
	}

	return false;
}
#endif	// #if defined(_SERVER)
#endif

#endif // PRE_ADD_COSRANDMIX