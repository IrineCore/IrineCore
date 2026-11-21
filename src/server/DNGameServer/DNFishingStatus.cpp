
#include "stdafx.h"
#include "DNFishingStatus.h"
#include "FishingArea.h"
#include "DNUserSession.h"
#include "SecondarySkillRepository.h"
#include "DNFarmGameRoom.h"
#include "NpcReputationProcessor.h"
#include "DNGameDataManager.h"
#include "DNMissionSystem.h"

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "DnTotalLevelSkillBlows.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

CDNFishingStatus::CDNFishingStatus(CDNUserSession * pSession, CFishingArea * pFishingArea)
{
	_ASSERT(pSession);
	_ASSERT(pFishingArea);

	m_pSession = pSession;
	m_pFishingArea = pFishingArea;	
	
	m_eFishingStatus = Fishing::Status::FISHING_STATUS_NONE;
	m_eCastType = Fishing::Cast::CASTNONE;
	m_eControlType = Fishing::Control::CONTROL_NONE;
	m_nStartingTime = 0;
	m_nReduceTime = 0;
	m_nFishingGauge = 0;
	m_nCheckTick = 0;
#ifdef PRE_ADD_CASHFISHINGITEM
	memset(&m_FishingToolInfo, 0, sizeof(TFishingMeritInfo));
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	m_nBaitIndex = -1;
	m_biBaitSerial = 0;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
	m_nLastSyncTick = 0;	

	m_nFishingPatternID = -1;
	memset(&m_FishingPattern, 0, sizeof(TFishingTableData));
}

CDNFishingStatus::~CDNFishingStatus()
{
}

void CDNFishingStatus::DoUpdate(DWORD nCurTick)
{
	//Basically time-based item granting; duration can be shortened by user control
	if (IsFishing() == false)
		return;					//Skip if not currently fishing

	//Currently fishing.
	if (m_nStartingTime > 0)
	{
		ULONG nTimeLimit = 0;
		GetTimeLimit(nTimeLimit);

		if (nCurTick >= nTimeLimit)
		{
			SendFishingEnd();
			//CheckAndRewardItem(nCurTick);
		}
		else if (m_eCastType == Fishing::Cast::eCast::CASTMANUAL)
		{
			UpdateRemainTime(nCurTick);
			if (m_nLastSyncTick <= 0 || m_nLastSyncTick + Fishing::FISHINGSYNCTERM < nCurTick)
			{
				m_pSession->SendFishingSync(m_nReduceTime, m_nFishingGauge);
				m_nLastSyncTick = nCurTick;
			}
		}
	}
}

bool CDNFishingStatus::IsFishingReward()
{
	return ( Fishing::Status::FISHING_STATUS_FISHING == m_eFishingStatus || Fishing::Status::FISHING_STATUS_REWARD == m_eFishingStatus )? true : false;
}

bool CDNFishingStatus::IsFishing()
{
	return Fishing::Status::FISHING_STATUS_FISHING == m_eFishingStatus ? true : false;
}

bool CDNFishingStatus::SetFishingPattern(CFishingArea * pFishingArea)
{
	//check fishing status - need to verify if currently fishing.
	if (Fishing::Status::FISHING_STATUS_FISHING == m_eFishingStatus)
	{
		_ASSERT(0);						//Need to check if this happens. Pattern reset is impossible during fishing.
		return false;
	}

	if (pFishingArea == NULL)
		return false;

	m_pFishingArea = pFishingArea;
	if (m_pFishingArea->GetFishingPattern(m_nFishingPatternID, m_FishingPattern))
	{
		m_eFishingStatus = Fishing::Status::FISHING_STATUS_READY;
		return true;
	}
	return false;
}

#ifdef PRE_ADD_CASHFISHINGITEM
bool CDNFishingStatus::StartFishing(Fishing::Cast::eCast eCastType, TFishingMeritInfo &FishingMerit)
{
	if (Fishing::Status::FISHING_STATUS_READY != m_eFishingStatus && Fishing::Status::FISHING_STATUS_IDLE != m_eFishingStatus)
		return false;

	m_eFishingStatus = Fishing::Status::FISHING_STATUS_FISHING;
	m_eCastType = eCastType;
	m_nStartingTime = timeGetTime();
	m_nCheckTick = 0;
	m_nFishingGauge = 0;
	m_nLastSyncTick = 0;
	m_FishingToolInfo = FishingMerit;
	m_nReduceTime = 0;	
	m_pSession->SendFishingPattern(m_nFishingPatternID, m_FishingPattern.nAutoMaxTime);
	return true;
}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
bool CDNFishingStatus::StartFishing(Fishing::Cast::eCast eCastType, int nBaitIndex, INT64 biBaitSerial)
{
	if (Fishing::Status::FISHING_STATUS_READY != m_eFishingStatus && Fishing::Status::FISHING_STATUS_IDLE != m_eFishingStatus)
		return false;

	m_eFishingStatus = Fishing::Status::FISHING_STATUS_FISHING;
	m_eCastType = eCastType;
	m_nStartingTime = timeGetTime();
	m_nCheckTick = 0;
	m_nFishingGauge = 0;
	m_nLastSyncTick = 0;
	m_nBaitIndex = nBaitIndex;
	m_biBaitSerial = biBaitSerial;
	m_nReduceTime = 0;	
	m_pSession->SendFishingPattern(m_nFishingPatternID);
	return true;
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

bool CDNFishingStatus::StopFishing()
{
	m_eFishingStatus = Fishing::Status::FISHING_STATUS_IDLE;
	m_eControlType = Fishing::Control::CONTROL_NONE;
	m_nStartingTime = 0;
	m_nCheckTick = 0;
	m_nFishingGauge = 0;
#ifdef PRE_ADD_CASHFISHINGITEM
	memset(&m_FishingToolInfo, 0, sizeof(TFishingMeritInfo));
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	m_nBaitIndex = -1;
	m_biBaitSerial = 0;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
	m_nLastSyncTick = 0;
	m_nReduceTime = 0;

	m_pSession->SendFishingStopFishingResult(ERROR_NONE);
	return true;
}

void CDNFishingStatus::FishingControl(Fishing::Control::eControl eControl)
{
	if (m_eFishingStatus == Fishing::Status::FISHING_STATUS_FISHING)
	{
		m_eControlType = eControl;
		return;
	}
	_DANGER_POINT();
}

void CDNFishingStatus::CheckAndRewardItem()
{
	if (m_eFishingStatus != Fishing::Status::FISHING_STATUS_REWARD)
	{
		_DANGER_POINT();
		return;
	}

	int nRetCode = -1;
	int nCatchItemID = 0;
	int nCatchItemCount	= 0;
	int nLevel = 1;
	int nDepth = ITEMDROP_DEPTH;
	bool bCheck = true;

#ifdef PRE_ADD_CASHFISHINGITEM
	if (m_FishingToolInfo.bUseCashBait)
	{
		if (m_pSession->GetItem()->DeleteCashInventoryBySerial(m_FishingToolInfo.biBaitSerial, 1) == false)
			bCheck = false;
	}
	else
	{
		if (m_pSession->GetItem()->DeleteItemByUse(ITEMPOSITION_INVEN, m_FishingToolInfo.nBaitIndex, m_FishingToolInfo.biBaitSerial) == false)
			bCheck = false;
	}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	if (m_pSession->GetItem()->DeleteItemByUse(ITEMPOSITION_INVEN, m_nBaitIndex, m_biBaitSerial) == false)
		bCheck = false;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

	if (((_roomrand(m_pSession->GetGameRoom()) % 100) <= m_FishingPattern.nSuccessProbability) && bCheck)
	{
		//Success
#if defined( PRE_ADD_FARM_DOWNSCALE )
		CDNUserItem::CalcDropItems( m_pSession, m_FishingPattern.nSuccessDropTableID, nCatchItemID, nCatchItemCount, nLevel, nDepth );
#else
		m_pSession->GetItem()->CalcDropItems(m_FishingPattern.nSuccessDropTableID, nCatchItemID, nCatchItemCount, nLevel, nDepth );
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )

		TItem ResultItem;
		if(CDNUserItem::MakeItemStruct(nCatchItemID, ResultItem))
		{
			//Insert the item immediately.
			nRetCode = m_pSession->GetItem()->CreateInvenWholeItem(ResultItem, DBDNWorldDef::AddMaterializedItem::FishingReward, 0, CREATEINVEN_ETC);
			TItemData *pItemData = g_pDataManager->GetItemData(ResultItem.nItemID);
			if (pItemData)
				m_pSession->GetEventSystem()->OnEvent( EventSystem::OnItemGain, 2, EventSystem::ItemType, pItemData->nType, EventSystem::ItemCount, m_pSession->GetItem()->GetInventoryItemCountByType(pItemData->nType));
		}
	}

	if (bCheck)	//If the item was not deducted, no secondary skill EXP either. Let's help each other out, okay?
	{
		int nAddPoint = (nRetCode == ERROR_NONE) ? m_FishingPattern.nSkillPointSuccess : m_FishingPattern.nSkillPointFailure;
		CNpcReputationProcessor::CheckAndCalcUnionBenefit( m_pSession, TStoreBenefitData::FishProficiencyUp, nAddPoint );

		if (m_pSession->GetSecondarySkillRepository()->AddExp(SecondarySkill::SubType::eType::FishingSkill, nAddPoint) == false)
			_DANGER_POINT();

	}
	else
		nRetCode = ERROR_FISHING_FAIL;

	if (nRetCode == -1)
		nRetCode = ERROR_FISHING_FAIL;

	SendFishingResult(nCatchItemID, nRetCode);
	StopFishing();	
}

#ifdef PRE_ADD_CASHFISHINGITEM
int CDNFishingStatus::GetRodItemID()
{
	return m_FishingToolInfo.nRodItemID;
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

bool CDNFishingStatus::GetTimeLimit(ULONG &nLimitTime)
{
	if (Fishing::Status::FISHING_STATUS_FISHING != m_eFishingStatus || m_nStartingTime <= 0)
		return false;
	
#ifdef PRE_ADD_CASHFISHINGITEM
	int nMaxTime = m_eCastType == Fishing::Cast::eCast::CASTAUTO ? m_FishingPattern.nAutoMaxTime : m_FishingPattern.nMaxTime;	
	int nReduce = m_eCastType == Fishing::Cast::eCast::CASTMANUAL ? m_nReduceTime : 0;

	nLimitTime = (nMaxTime * 1000) + m_nStartingTime - nReduce;
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	int nReduce = m_eCastType == Fishing::Cast::eCast::CASTMANUAL ? m_nReduceTime : 0;
	nLimitTime = (m_FishingPattern.nMaxTime * 1000) + m_nStartingTime - nReduce;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	//Decrease auto-fishing time by n seconds..
	DnActorHandle hActor = m_pSession->GetActorHandle();
	float incTimeValue = 0.0f;
	if (hActor && hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_264))
	{
		DNVector(DnBlowHandle) vlBlows;
		hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_264, vlBlows);
		{
			int nCount = (int)vlBlows.size();
			for (int i = 0; i < nCount; ++i)
			{
				DnBlowHandle hBlow = vlBlows[i];
				if (hBlow && hBlow->IsEnd() == false)
				{
					incTimeValue += hBlow->GetFloatValue() * 1000.0f;
				}
			}
		}
	}	
	nLimitTime -= (int)incTimeValue;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	return true;
}

void CDNFishingStatus::UpdateRemainTime(DWORD nCurTick)
{
	if (m_nCheckTick <= 0)
	{
		m_nCheckTick = nCurTick;
		return;
	}

	int nElapsedTick = nCurTick - m_nCheckTick;
	if (nElapsedTick <= 0)
		return;

	if (m_nCheckTick <= 0 || nElapsedTick > Fishing::FISHINGCHECKPERIODTICK)
	{
		int nTempGauge = nElapsedTick * ((m_eControlType == Fishing::Control::CONTROL_NONE) ? 1 : -1);
		m_nFishingGauge += (nTempGauge * m_FishingPattern.nPullingTargetSpeed);
		m_nFishingGauge = Fishing::FISHINGMAXGAUGE < m_nFishingGauge ? m_nFishingGauge - Fishing::FISHINGMAXGAUGE : m_nFishingGauge;
		//printf("m_nFishingGauge : %d [nTempGauge:%d] Control : %d\n", m_nFishingGauge, nTempGauge, m_eControlType);

		if ((m_FishingPattern.nPullingTargetMin * 1000) <= m_nFishingGauge && (m_FishingPattern.nPullingTargetMax *1000) >= m_nFishingGauge)
		{
			//If matched
			if (m_eControlType == Fishing::Control::CONTROL_PULLING)
			{
				m_nReduceTime += (nElapsedTick * (m_FishingPattern.nPullingTargetUpTime / 100));
				//printf("was matched reduce tick [%d]\n", m_nReduceTime);
			}
		}
		m_nCheckTick = nCurTick;
	}
}

void CDNFishingStatus::SendFishingEnd()
{
	m_eFishingStatus = Fishing::Status::FISHING_STATUS_REWARD;
	m_pSession->SendFishingEnd();
}

void CDNFishingStatus::SendFishingResult(int nChatchedItemID, int nRetCode)
{
	SCFishingReward packet;
	memset(&packet, 0, sizeof(SCFishingReward));

	packet.nSessionID = m_pSession->GetSessionID();
	packet.nRewardItemID = nChatchedItemID;
	packet.nRetCode = nRetCode;

	if (nRetCode == ERROR_NONE || nRetCode == ERROR_FISHING_FAIL)
	{
		//Broadcast to all users (sync caught fish) only when there is no error; otherwise send only to the user.
		CDNGameRoom * pGameRoom = m_pSession->GetGameRoom();
		if (pGameRoom)
			static_cast<CDNFarmGameRoom*>(pGameRoom)->BroadCast(SC_FISHING, eFishing::SC_FISHINGREWARD, (char*)&packet, sizeof(SCFishingReward));
	}
	else
	{
		if (m_pSession)
			m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGREWARD, (char*)&packet, sizeof(SCFishingReward));
	}
}
