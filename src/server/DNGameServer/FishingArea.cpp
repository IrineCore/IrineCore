
#include "stdafx.h"
#include "FishingArea.h"
#include "DnActor.h"
#include "DNUserSession.h"
#include "DNFarmUserSession.h"
#include "SecondarySkill.h"
#include "SecondarySkillRepository.h"
#include "DNGameDataManager.h"


CFishingArea::CFishingArea(CDNFarmGameRoom * pFarmGameRoom, int nIdx, SOBB * pOBB, TFishingPointTableData * pFishingPoint)
:m_pFarmGameRoom(pFarmGameRoom), m_nFishingAreaIdx(nIdx), m_OBB(pOBB?*pOBB:SOBB())
{
	_ASSERT(pFishingPoint);
#ifdef PRE_ADD_CASHFISHINGITEM
	memset(&m_FishingMeritInfo, 0, sizeof(TFishingMeritInfo));
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
	m_FishingPoint = *pFishingPoint;
}

CFishingArea::~CFishingArea()
{
}

#ifdef PRE_ADD_CASHFISHINGITEM
int CFishingArea::CheckFishingRequirement(CDNFarmUserSession * pSession, TFishingToolInfo &Tool, TFishingMeritInfo &Info)
{
	if (!pSession->GetActorHandle())
		return ERROR_FISHING_FAIL;

	//Is in fishing area?
	if (CheckInside(*pSession->GetActorHandle()->GetPosition()) == false)
		return ERROR_FISHING_FAIL_ISNOT_FISHINGAREA;

	SecondarySkill::Grade::eType eFishingGrade;
	int nFishingLevel;
	if (pSession->GetFishingSecondarySkillInfo(eFishingGrade, nFishingLevel) == false)
		return ERROR_FISHING_FAIL;

	//Has fishing skill?
	CSecondarySkill * pSecondarySkill = pSession->GetSecondarySkillRepository()->Get(SecondarySkill::SubType::FishingSkill);
	if (pSecondarySkill == NULL)
		return ERROR_FISHING_FAIL;

	int nRet = pSession->CheckFishingRequirement(m_FishingPoint.nRequireItemType1, m_FishingPoint.nRequireItemType2, Tool, Info);
	if (nRet != ERROR_NONE)
		return nRet;

	m_FishingMeritInfo = Info;

	return CheckFishingAreaRequirement(eFishingGrade, nFishingLevel);
}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
int CFishingArea::CheckFishingRequirement(CDNFarmUserSession * pSession, int nRodInvenIndex, int nBaitInvenIndex, INT64 &nBaitSerial)
{
	if (!pSession->GetActorHandle())
		return ERROR_FISHING_FAIL;

	//Is in fishing area?
	if (CheckInside(*pSession->GetActorHandle()->GetPosition()) == false)
		return ERROR_FISHING_FAIL_ISNOT_FISHINGAREA;

	SecondarySkill::Grade::eType eFishingGrade;
	int nFishingLevel;
	if (pSession->GetFishingSecondarySkillInfo(eFishingGrade, nFishingLevel) == false)
		return ERROR_FISHING_FAIL;

	//Has fishing skill?
	CSecondarySkill * pSecondarySkill = pSession->GetSecondarySkillRepository()->Get(SecondarySkill::SubType::FishingSkill);
	if (pSecondarySkill == NULL)
		return ERROR_FISHING_FAIL;

	int nRet = pSession->CheckFishingRequirement(m_FishingPoint.nRequireItemType1, nRodInvenIndex, m_FishingPoint.nRequireItemType2, nBaitInvenIndex, nBaitSerial);
	if (nRet != ERROR_NONE)
		return nRet;

	return CheckFishingAreaRequirement(eFishingGrade, nFishingLevel);
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

bool CFishingArea::CheckInside(EtVector3 &vPoint)
{
	return m_OBB.IsInside(vPoint);
}

int CFishingArea::CheckFishingAreaRequirement(int nFishingGrade, int nFishingLevel)
{
	if (m_FishingPoint.nSecondarySkillClass > nFishingGrade)
		return ERROR_FISHING_INSUFFICIENCY_FISHINGSKILL_LEVEL;
	if (m_FishingPoint.nSecondarySkillLevel > nFishingLevel)
		return ERROR_FISHING_INSUFFICIENCY_FISHINGSKILL_GRADE;
	return ERROR_NONE;
}

bool CFishingArea::GetFishingPattern(int &nPatternID, TFishingTableData &Fishing)
{
	int nRand = _roomrand(m_pFarmGameRoom)%100;
	int nAmountProbability = 0;

	for (int i = 0; i < Fishing::Max::FISHINGPATTERNMAX; i++)
	{
		if (m_FishingPoint.Pattern[i].nProbabillity + nAmountProbability >= nRand)
		{
			TFishingTableData * pPattern = g_pDataManager->GetFishingTableData(m_FishingPoint.Pattern[i].nPatternID);
			if (pPattern)
			{
				nPatternID = m_FishingPoint.Pattern[i].nPatternID;
				Fishing = *pPattern;

#ifdef PRE_ADD_CASHFISHINGITEM
				Fishing.nAutoMaxTime = Fishing.nMaxTime;
				if (m_FishingMeritInfo.nMeritReduceFishingTime > 0 || m_FishingMeritInfo.nMeritSuccessRate)
				{
					Fishing.nSuccessProbability += m_FishingMeritInfo.nMeritSuccessRate;
					Fishing.nAutoMaxTime = (Fishing.nMaxTime - (int)(((float)m_FishingMeritInfo.nMeritReduceFishingTime * 0.01f) * (float)Fishing.nMaxTime));
				}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
				return true;
			}
		}

		nAmountProbability += m_FishingPoint.Pattern[i].nProbabillity;
	}
	return false;
}

