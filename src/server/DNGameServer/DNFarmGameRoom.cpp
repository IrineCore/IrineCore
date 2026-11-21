
#include "stdafx.h"
#include "DNFarmGameRoom.h"
#include "DnFarmGameTask.h"
#include "DNDBConnection.h"
#include "DNUserSession.h"
#include "DNMasterConnectionManager.h"
#include "GrowingArea.h"
#include "GrowingAreaStateHarvesting.h"
#include "GrowingAreaStateGrowing.h"
#include "GrowingAreaStatePlanting.h"
#include "SecondarySkill.h"
#include "SecondarySkillRepository.h"
#include "DNGameDataManager.h"
#include "DnPartyTask.h"
#include "DNFarmUserSession.h"
#include "NpcReputationProcessor.h"


CDNFarmGameRoom::CDNFarmGameRoom(CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket)
	:CDNGameRoom(pServer, iRoomID, pPacket)
{
	m_nReportedUserCount = 0;
#if defined( PRE_PARTY_DB )
	m_PartyStructData = pPacket->PartyData;
	m_PartyStructData.iTargetMapIndex = pPacket->nMapIndex;
#else
	m_nFarmMaxUser = pPacket->cMemberMax;
	m_nTargetMapIdx = pPacket->nMapIndex;
#endif
	m_bFarmStartFlag = pPacket->bStart;
	m_bReportedFarmStartFlag = m_bFarmStartFlag;
#if defined( PRE_ADD_FARM_DOWNSCALE )
	m_iAttr = pPacket->iFarmAttr;
#elif defined( PRE_ADD_VIP_FARM )
	m_Attr = pPacket->FarmAttr;
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	m_bSendFarmDataLoadedFail = false;
}

CDNFarmGameRoom::~CDNFarmGameRoom()
{

}

void CDNFarmGameRoom::OnDBMessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen)
{
	switch (iMainCmd)
	{
	case MAINCMD_FARM:
	{
		OnDBRecvFarm(iSubCmd, pData, iLen);
		break;
	}
	}

	CDNGameRoom::OnDBMessageProcess(iMainCmd, iSubCmd, pData, iLen);
}

void CDNFarmGameRoom::OnDBRecvFarm(int nSubCmd, char * pData, int nLen)
{
	switch (nSubCmd)
	{
	case QUERY_GETLIST_FIELD:
	{
		if (m_GameState != _GAME_STATE_FARM_LOAD2PLAY)
		{
			g_Log.Log(LogType::_FARM, L"QUERY_GETLIST_FIELD Fail\n");
			return;					//This must not happen.
		}

		//Starts from this timing.
		TAGetListField* pPacket = reinterpret_cast<TAGetListField*>(pData);

		// Task is created in InitRoomState.
		static_cast<CDnFarmGameTask*>(GetGameTask())->InitializeSeed(pPacket);

		//
		m_bFarmStartFlag = true;
		m_GameState = _GAME_STATE_PLAY;
		GetGameTask()->SetSyncComplete(true);
		CDnPartyTask::GetInstance(this).SetSyncComplete(true);

		//Notify master that loading is finished and ready to accept users
#if defined( PRE_PARTY_DB )
		bool bSend = g_pMasterConnectionManager->SendFarmDataLoaded(m_iWorldID, m_pGameServer->GetServerID(), GetRoomID(), GetFarmIndex(), m_PartyStructData.iTargetMapIndex, GetFarmMaxUser());
#else
		bool bSend = g_pMasterConnectionManager->SendFarmDataLoaded(m_iWorldID, m_pGameServer->GetServerID(), GetRoomID(), GetFarmIndex(), m_nTargetMapIdx, GetFarmMaxUser());
#endif
		if (bSend == false)
			m_bSendFarmDataLoadedFail = true;
		break;
	}
	case QUERY_GETLIST_FIELD_FORCHARACTER:
	{
		if (GetGameTask())
		{
			static_cast<CDnFarmGameTask*>(GetGameTask())->InitializePrivateGrowingArea(reinterpret_cast<TAGetListFieldForCharacter*>(pData));
		}
		break;
	}
	case QUERY_GET_FIELDITEMCOUNT:
	{
		TAGetFieldItemCount* pPacket = reinterpret_cast<TAGetFieldItemCount*>(pData);
		if (pPacket->nRetCode == ERROR_NONE)
		{
			CDNUserSession * pSession = GetUserSession(pPacket->nSessionID);
			CGrowingArea* pArea = GetFarmGameTask()->GetGrowingArea(pPacket->iAreaIndex, pSession);
			if (pArea == NULL)
				break;

			if (pArea->GetState() == Farm::AreaState::PLANTING)
			{
				TFarmCultivateTableData* pData = g_pDataManager->GetFarmCultivateTableData(pArea->GetPlantSeed()->GetSeedItemID());
				if (pData == NULL)
					break;
				if (pData->iOverlap <= pPacket->iItemCount)
				{
					static_cast<CFarmAreaStatePlanting*>(pArea->GetStatePtr())->SetLastError(ERROR_FARM_CANT_PLANT_OVERLAPCOUNT);
					pArea->ChangeState(Farm::AreaState::NONE);
					break;
				}
				else
				{
					static_cast<CFarmAreaStatePlanting*>(pArea->GetStatePtr())->SetCheckOverlapCountFlag(true);
				}
			}

		}
		break;
	}
	case QUERY_ADD_FIELD:
	case QUERY_ADD_FIELD_FORCHARACTER:
	{
		TAAddField* pPacket = reinterpret_cast<TAAddField*>(pData);
		CDNUserSession* pSession = GetUserSessionByCharDBID(pPacket->biCharacterDBID);
		CGrowingArea* pArea = GetFarmGameTask()->GetGrowingArea(pPacket->nFieldIndex, pSession);
		if (pArea == NULL)
			break;

		if (pPacket->nRetCode == ERROR_NONE)
		{
			CDNUserSession* pSession = GetUserSessionByCharDBID(pPacket->biCharacterDBID);
			if (pSession)
			{
				// Remove seed item
				// Since it was already removed from DB, set LogCode to 0 to avoid sending a DB query.
				if (pSession->GetItem()->DeleteInventoryBySlot(pPacket->cInvenIndex, 1, pPacket->biSeedItemSerial, DBDNWorldDef::UseItem::DoNotDBSave) == false)
				{
					// Item mismatch between DB and server; disconnect user since DB must be trusted.
					pSession->DetachConnection(L"PlantSeed SeedItem Sync Failed!");
				}

				// Remove attached item
				for (int i = 0; i<pPacket->cAttachCount; ++i)
				{
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
					if (!pPacket->AttachItems[i].bRemoveItem)
						continue;
#endif
					if (pSession->GetItem()->DeleteCashInventoryBySerial(pPacket->AttachItems[i].biSerial, pPacket->AttachItems[i].cCount, false) == false)
					{
						// Item mismatch between DB and server; disconnect user since DB must be trusted.
						pSession->DetachConnection(L"PlantSeed AttachItem Sync Failed!");
					}
				}

#if defined( PRE_ADD_VIP_FARM )
#if defined( PRE_ADD_FARM_DOWNSCALE )
				if (GetAttr()&Farm::Attr::Vip)
#else
				if (GetAttr() == Farm::Attr::Vip)
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
				{
					if (pArea->bIsPrivateArea() == true)
					{
						GetDBConnection()->QueryAddFieldForCharacterAttachment(GetDBThreadID(), pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), GetRoomID(),
							pArea->GetIndex(), Farm::Common::VIP_GROWING_BOOST_ITEMID, 0, 0, 0, pSession->GetMapIndex(), pSession->GetIpW(), true);
					}
					else
					{
						GetDBConnection()->QueryAddFieldAttachment(GetDBThreadID(), pSession, GetFarmIndex(), GetRoomID(), pArea->GetIndex(), Farm::Common::VIP_GROWING_BOOST_ITEMID, 0, 0, 0, true);
					}
				}
#endif // #if defined( PRE_ADD_VIP_FARM )

				// Secondary skill proficiency
				CSecondarySkill* pSkill = pSession->GetSecondarySkillRepository()->Get(SecondarySkill::SubType::CultivationSkill);
				if (pSkill)
				{
					TFarmCultivateTableData* pTableData = g_pDataManager->GetFarmCultivateTableData(pArea->GetPlantSeed()->GetSeedItemID());
					if (pTableData)
					{
						int nAddPoint = pTableData->iSkillPointPlant;
						CNpcReputationProcessor::CheckAndCalcUnionBenefit(pSession, TStoreBenefitData::CultivateProficiencyUp, nAddPoint);
						pSession->GetSecondarySkillRepository()->AddExp(pSkill->GetSkillID(), nAddPoint);
					}
					else
					{
						_ASSERT(0);
					}
				}
				else
				{
					_ASSERT(0);
				}
			}

			pArea->ChangeState(Farm::AreaState::GROWING);
		}
		else
		{
			if (pArea->GetState() == Farm::AreaState::PLANTING)
				static_cast<CFarmAreaStatePlanting*>(pArea->GetStatePtr())->SetLastError(pPacket->nRetCode);
			pArea->ChangeState(Farm::AreaState::NONE);
		}

		break;
	}
	case QUERY_DEL_FIELD:
	{
		const TADelField* pPacket = reinterpret_cast<TADelField*>(pData);

		// DB and seed info are out of sync. What to do here??? Just log a warning for now.
		if (pPacket->nRetCode != ERROR_NONE)
		{
			g_Log.Log(LogType::_FARM, L"QUERY_DEL_FIELD Failed! Ret=%d", pPacket->nRetCode);
		}
		break;
	}
	case QUERY_DEL_FIELD_FORCHARACTER:
	{
		const TADelFieldForCharacter* pPacket = reinterpret_cast<TADelFieldForCharacter*>(pData);

		// DB and seed info are out of sync. What to do here??? Just log a warning for now.
		if (pPacket->nRetCode != ERROR_NONE)
		{
			g_Log.Log(LogType::_FARM, L"QUERY_DEL_FIELD_FORCHARACTER Failed! Ret=%d", pPacket->nRetCode);
		}
		break;
	}
	case QUERY_HARVEST:
	case QUERY_HARVEST_FORCHARACTER:
	{
		TAHarvest* pPacket = reinterpret_cast<TAHarvest*>(pData);

		CDNUserSession* pSession = GetUserSessionByCharDBID(pPacket->biCharacterDBID);
		CGrowingArea* pArea = GetFarmGameTask()->GetGrowingArea(pPacket->nFieldIndex, pSession);
		if (pArea == NULL)
			break;

		if (pPacket->nRetCode == ERROR_NONE)
		{
			CDNUserSession* pSession = NULL;
			if (pArea->GetState() == Farm::AreaState::HARVESTING)
			{
				pSession = GetUserSessionByCharDBID(static_cast<CFarmAreaStateHarvesting*>(pArea->GetStatePtr())->GetHarvestingCharacterDBID());
			}

			pArea->ChangeState(Farm::AreaState::HARVESTED, pSession);
		}
		else
		{
			// No farm to harvest.
			if (pPacket->nRetCode == 103283)
			{
				pArea->ChangeState(Farm::AreaState::NONE);
			}
			else
			{
				pArea->ChangeState(Farm::AreaState::COMPLETED);
			}
		}
		break;
	}
	case QUERY_ADD_FIELD_ATTACHMENT:
	case QUERY_ADD_FIELD_FORCHARACTER_ATTACHMENT:
	{
		const TAAddFieldAttachment* pPacket = reinterpret_cast<TAAddFieldAttachment*>(pData);
		CDNUserSession* pSession = GetUserSessionByCharDBID(pPacket->biCharacterDBID);
		CGrowingArea* pArea = GetFarmGameTask()->GetGrowingArea(pPacket->nFieldIndex, pSession);
		if (pArea == NULL)
			break;

		if (pPacket->nRetCode == ERROR_NONE)
		{
			CDNUserSession* pSession = GetUserSessionByCharDBID(pPacket->biCharacterDBID);
			if (pSession)
			{
#if defined( PRE_ADD_VIP_FARM )
				// Remove attached item
				// Since it was already removed from DB, set LogCode to 0 to avoid sending a DB query.
				bool bOK = false;
				if (pPacket->bVirtualAttach == true)
				{
					bOK = true;
				}
				else
				{
					switch (pPacket->cInvenType)
					{
					case ITEMPOSITION_INVEN:
					{
						bOK = pSession->GetItem()->DeleteInventoryBySlot(pPacket->cInvenIndex, 1, pPacket->biAttachItemSerial, DBDNWorldDef::UseItem::DoNotDBSave);
						break;
					}
					case ITEMPOSITION_CASHINVEN:
					{
						bOK = pSession->GetItem()->DeleteCashInventoryBySerial(pPacket->biAttachItemSerial, 1, false);
						break;
					}
					}
				}
#else
				// Remove attached item
				// Since it was already removed from DB, set LogCode to 0 to avoid sending a DB query.
				bool bOK = false;
				switch (pPacket->cInvenType)
				{
				case ITEMPOSITION_INVEN:
				{
					bOK = pSession->GetItem()->DeleteInventoryBySlot(pPacket->cInvenIndex, 1, pPacket->biAttachItemSerial, DBDNWorldDef::UseItem::DoNotDBSave);
					break;
				}
				case ITEMPOSITION_CASHINVEN:
				{
					bOK = pSession->GetItem()->DeleteCashInventoryBySerial(pPacket->biAttachItemSerial, 1, false);
					break;
				}
				}
#endif // #if defined( PRE_ADD_VIP_FARM )

				if (bOK == false)
				{
					// Item mismatch between DB and server; disconnect user since DB must be trusted.
					pSession->DetachConnection(L"AddField AttachItem Sync Failed!");
					break;
				}

				const TItemData* pItemData = g_pDataManager->GetItemData(pPacket->iAttachItemID);
				int iAddWaterPoint = 0;
				// If water~
				if (pItemData && pItemData->nType == ITEMTYPE_WATER)
				{
					// Secondary skill proficiency
					CSecondarySkill* pSkill = pSession->GetSecondarySkillRepository()->Get(SecondarySkill::SubType::CultivationSkill);
					if (pSkill)
					{
						TFarmCultivateTableData* pTableData = g_pDataManager->GetFarmCultivateTableData(pArea->GetPlantSeed()->GetSeedItemID());
						if (pTableData)
						{
							iAddWaterPoint = pTableData->iSkillPointWater;
							if (iAddWaterPoint > 0)
							{
								CNpcReputationProcessor::CheckAndCalcUnionBenefit(pSession, TStoreBenefitData::CultivateProficiencyUp, iAddWaterPoint);
								pSession->GetSecondarySkillRepository()->AddExp(pSkill->GetSkillID(), iAddWaterPoint);
							}
						}
					}
				}

				if (iAddWaterPoint > 0)
				{
					if (pArea->GetOwnerCharacterDBID() != pSession->GetCharacterDBID())
						g_pMasterConnectionManager->SendFarmSyncAddWater(GetWorldSetID(), pArea->GetOwnerCharacterDBID(), pSession->GetCharacterName(), iAddWaterPoint);
				}
			}

			// AttachItem
			pArea->GetPlantSeed()->AttachItem(pPacket->iAttachItemID);
			// Send Area info
			pArea->SendAreaInfo();
			// Sync FieldList
			g_pMasterConnectionManager->SendFarmSync(GetWorldSetID(), pArea->GetPlantSeed()->GetOwnerCharacterDBID(), Farm::ServerSyncType::FIELDLIST);
		}

#if defined( PRE_ADD_VIP_FARM )
		if (pArea->GetState() == Farm::AreaState::GROWING && pPacket->iAttachItemID != Farm::Common::VIP_GROWING_BOOST_ITEMID)
		{
			CFarmAreaStateGrowing* pGrowingState = static_cast<CFarmAreaStateGrowing*>(pArea->GetStatePtr());
			pGrowingState->CompleteAddWater(pPacket->nRetCode);
		}
#else
		if (pArea->GetState() == Farm::AreaState::GROWING)
		{
			CFarmAreaStateGrowing* pGrowingState = static_cast<CFarmAreaStateGrowing*>(pArea->GetStatePtr());
			pGrowingState->CompleteAddWater(pPacket->nRetCode);
		}
#endif // #if defined( PRE_ADD_VIP_FARM )

		break;
	}
	}
}

void CDNFarmGameRoom::QueryGetListField()
{
	//If farm, must fetch initialization data from DB.
	CDNDBConnection * pDBCon = NULL;
	BYTE cThreadID = 0;
	CopyDBConnectionInfo(pDBCon, cThreadID);

	if (pDBCon)
	{
		pDBCon->QueryGetListField(cThreadID, m_iWorldID, GetRoomID(), GetFarmIndex());
		m_GameState = _GAME_STATE_FARM_LOAD2PLAY;
		m_iNextGameState = GetGameTick() + GOGO_SING_TO_FARMPLAY_FOR_WAIT_TIME_LIMIT;
	}
	else
	{	//If it fails here, delete the room immediately.
		_DANGER_POINT();
		m_GameState = _GAME_STATE_DESTROYED;
	}
}

void CDNFarmGameRoom::FarmUpdate()
{
	//No need to do it too frequently.
	if (m_nReportedUserCount != GetUserCount() || m_bReportedFarmStartFlag != m_bFarmStartFlag)
	{
		m_nReportedUserCount = GetUserCount();
		m_bReportedFarmStartFlag = m_bFarmStartFlag;
		//Report to Master if user count mismatches.
		if (g_pMasterConnectionManager->SendFarmUserCount(GetWorldSetID(), GetFarmIndex(), GetRoomID(), GetUserCount(), m_bFarmStartFlag) == false)
			_DANGER_POINT();		//Huh?
	}

	if (m_bSendFarmDataLoadedFail)
	{
		m_bSendFarmDataLoadedFail = false;
#if defined( PRE_PARTY_DB )
		bool bSend = g_pMasterConnectionManager->SendFarmDataLoaded(m_iWorldID, m_pGameServer->GetServerID(), GetRoomID(), GetFarmIndex(), m_PartyStructData.iTargetMapIndex, GetFarmMaxUser());
#else
		bool bSend = g_pMasterConnectionManager->SendFarmDataLoaded(m_iWorldID, m_pGameServer->GetServerID(), GetRoomID(), GetFarmIndex(), m_nTargetMapIdx, GetFarmMaxUser());
#endif
		if (bSend == false)
			m_bSendFarmDataLoadedFail = true;
	}
}

void CDNFarmGameRoom::OnInitGameRoomUser()
{
	// Initialize GameRoom
	CDNGameRoom::OnInitGameRoomUser();

	// Initialize farm info
}

void CDNFarmGameRoom::OnSync2Sync(CDNUserSession* pBreakIntoSession)
{
	if (pBreakIntoSession == NULL)
		return;

	static_cast<CDNFarmUserSession*>(pBreakIntoSession)->SendFarmFieldCountInfo(static_cast<CDNFarmUserSession*>(pBreakIntoSession)->GetFarmActiveFieldCount());
	static_cast<CDnFarmGameTask*>(GetGameTask())->SyncArea(pBreakIntoSession);
	static_cast<CDnFarmGameTask*>(GetGameTask())->SyncFishing(pBreakIntoSession);
}

void CDNFarmGameRoom::SetFarmActivation(bool bActivation)
{
	m_bFarmStartFlag = bActivation;
	if (m_bFarmStartFlag)
	{
		if (m_GameState == _GAME_STATE_FARM_PAUSE)		//If the current state is Pause
		{
			m_GameState = _GAME_STATE_PLAY;				//Switch to play state.
			CDnGameTask* pGameTask = GetGameTask();
			if (pGameTask && pGameTask->m_pFrameSync)
			{
				LOCAL_TIME LocalTime = pGameTask->m_pFrameSync->GetMSTime();
				LOCAL_TIME PrevTime = pGameTask->GetPrevLocalTime();

				g_Log.Log(LogType::_FARM, L"SetFarmActivation() TimeGap:%d", LocalTime - PrevTime);

				pGameTask->SetPrevLocalTime(LocalTime);
			}
		}
	}
}

void CDNFarmGameRoom::UpdateCharacterName(MAChangeCharacterName* pPacket)
{
	if (GetFarmGameTask()->UpdateCharacterName(pPacket))
	{
		SCChangeOwnerName TxPacket;
		memset(&TxPacket, 0, sizeof(TxPacket));

		_wcscpy(TxPacket.wszOriginName, _countof(TxPacket.wszOriginName), pPacket->wszOriginName, (int)wcslen(pPacket->wszOriginName));
		_wcscpy(TxPacket.wszCharacterName, _countof(TxPacket.wszCharacterName), pPacket->wszCharacterName, (int)wcslen(pPacket->wszCharacterName));

		BroadCast(SC_FARM, eFarm::SC_CHANGE_OWNERNAME, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
	}
}

void CDNFarmGameRoom::BroadCast(int iMainCmd, int iSubCmd, char* pData, int iLen)
{
	for (UINT i = 0; i<GetUserCount(); ++i)
	{
		CDNUserSession* pSession = GetUserData(i);
		if (pSession == NULL || pSession->GetState() != SESSION_STATE_GAME_PLAY)
			continue;

		pSession->AddSendData(iMainCmd, iSubCmd, pData, iLen);
	}
}

void CDNFarmGameRoom::OnLeaveUser(const UINT uiSessionID)
{
	if (GetGameTask())
		GetGameTask()->OnLeaveUser(uiSessionID);
}

void CDNFarmGameRoom::OnStartGuildWarFinal()
{
	// Return if not the winning guild farm
	if (!(GetAttr()&Farm::Attr::GuildChampion))
		return;

	static_cast<CDnFarmGameTask*>(GetGameTask())->OnStartGuildWarFinal();
}
