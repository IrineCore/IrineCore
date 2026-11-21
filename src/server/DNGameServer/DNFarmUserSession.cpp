
#include "stdafx.h"
#include "DNFarmUserSession.h"
#include "DNDBConnection.h"
#include "SecondarySkill.h"
#include "SecondarySkillRepository.h"
#include "DNGameDataManager.h"
#include "DNFishingStatus.h"

CDNFarmUserSession::CDNFarmUserSession( UINT uiUID, CDNRUDPGameServer* pServer, CDNGameRoom* pRoom )
:CDNBreakIntoUserSession( uiUID, pServer, pRoom )
{
	m_iFarmActiveFieldCount		= 0;
	m_bIsNeedUpdateFieldList	= false;
	m_pFishingStatus = NULL;
}

CDNFarmUserSession::~CDNFarmUserSession()
{
	if (m_pFishingStatus)
		SAFE_DELETE(m_pFishingStatus);
}

//int testTeam=0;
void CDNFarmUserSession::PreInitializeUser(WCHAR * wszAccountName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bTutorial, bool bAdult, char cPCBangGrade, TMemberVoiceInfo * pInfo /* = NULL*/)
{

	CDNUserSession::PreInitializeUser( wszAccountName, nAccountDBID, nSessionID, biCharacterDBID, iTeam, nWorldID, nVillageID, bTutorial, bAdult, cPCBangGrade, pInfo );
	// Force farm to team 0.
}

void CDNFarmUserSession::OnDBRecvFarm(int nSubCmd, char *pData)
{
	switch( nSubCmd )
	{
		case QUERY_GETLIST_HARVESTDEPOTITEM:
		{
			const TAGetListHarvestDepotItem* pPacket = reinterpret_cast<const TAGetListHarvestDepotItem*>(pData);

			LoadFarmWareHouseList( pPacket );
			SendFarmWareHouseList( pPacket->nRetCode, pPacket->biLastUniqueID, m_MapFarmWareHouse );
			break;
		}
		case QUERY_GETFIELDCOUNT_BYCHARACTER:
		{
			const TAGetFieldCountByCharacter* pPacket = reinterpret_cast<const TAGetFieldCountByCharacter*>(pData);

			if( pPacket->nRetCode == ERROR_NONE )
			{
				m_bIsNeedUpdateFieldList	= true;
				m_iFarmActiveFieldCount		= pPacket->iFieldCount;
				if( pPacket->bIsSend == true )
					SendFarmFieldCountInfo( pPacket->iFieldCount );
			}
			else
			{
				_ASSERT(0);
			}
			break;
		}
		case QUERY_GETLIST_FIELD_BYCHARACTER:
		{
			const TAGetListFieldByCharacter* pPacket = reinterpret_cast<TAGetListFieldByCharacter*>(pData);

			if( pPacket->nRetCode == ERROR_NONE )
			{
				SendFarmFieldList( pPacket->Fields, pPacket->cCount );
			}
			else
			{
				_ASSERT(0);
			}
			break;
		}
	}

	CDNUserBase::OnDBRecvFarm( nSubCmd, pData );
}

void CDNFarmUserSession::DoUpdate( DWORD dwCurTick )
{
	CDNUserSession::DoUpdate( dwCurTick );

	// Handle farm-related Update.
	// Update to process after SESSION_STATE_GAME_PLAY
	if( GetState() == SESSION_STATE_GAME_PLAY )
	{
		if( m_bIsNeedUpdateFieldList == true )
		{
			m_bIsNeedUpdateFieldList = false;
			GetDBConnection()->QueryGetListFieldByCharacter( this );
		}

		if (m_pFishingStatus)
			m_pFishingStatus->DoUpdate(dwCurTick);
	}
}

void CDNFarmUserSession::LoadFarmWareHouseList( const TAGetListHarvestDepotItem* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	for( int i=0 ; i<pPacket->cCount ; ++i )
		m_MapFarmWareHouse.insert( std::make_pair(pPacket->Items[i].biUniqueID,pPacket->Items[i]) );
}

const TItem* CDNFarmUserSession::GetFarmWareHouseItem( INT64 biUniqueID )
{
	std::map<INT64, TFarmWareHouseItem>::iterator itor = m_MapFarmWareHouse.find( biUniqueID );
	if( itor == m_MapFarmWareHouse.end() )
		return NULL;

	return &(*itor).second;
}

void CDNFarmUserSession::DeleteFarmWareHouseItem( INT64 biUniqueID )
{
	m_MapFarmWareHouse.erase( biUniqueID );
}

INT64 CDNFarmUserSession::GetLastWareHouseItemUniqueID()
{
	if( m_MapFarmWareHouse.empty() )
		return 0;

	std::map<INT64, TFarmWareHouseItem>::iterator itor = m_MapFarmWareHouse.begin();
	std::advance( itor, m_MapFarmWareHouse.size()-1 );

	return (*itor).first;
}

bool CDNFarmUserSession::CheckActiveFieldCount()
{
	CSecondarySkill* pSecondarySkill = GetSecondarySkillRepository()->Get( SecondarySkill::SubType::CultivationSkill );
	if( pSecondarySkill == NULL )
		return false;
	TSecondarySkillLevelTableData* pTableData = g_pDataManager->GetSecondarySkillLevelTableData( pSecondarySkill->GetSkillID(), pSecondarySkill->GetGrade(), pSecondarySkill->GetLevel() );
	if( pTableData == NULL )
		return false;
	if( GetFarmActiveFieldCount() >= pTableData->iParam )
		return false;

	return true;
}

bool CDNFarmUserSession::GetFishingSecondarySkillInfo(SecondarySkill::Grade::eType &nSkillGrade, int &nLevel)
{
	//Has fishing skill?
	CSecondarySkill * pSecondarySkill = GetSecondarySkillRepository()->Get(SecondarySkill::SubType::FishingSkill);
	if (pSecondarySkill == NULL)
		return false;

	nSkillGrade = pSecondarySkill->GetGrade();
	nLevel = pSecondarySkill->GetLevel();
	return true;
}

#ifdef PRE_ADD_CASHFISHINGITEM
int CDNFarmUserSession::CheckFishingRequirement(int nRequirementType1, int nRequirementType2, TFishingToolInfo &Tool, TFishingMeritInfo &Info)
{
	if (GetItem()->FindBlankInventorySlotCount() <= 0)
		return ERROR_FISHING_INSUFFICIENCY_INVENTORY;

	bool bUseCashRod = Tool.biCashRodSerial > 0 ? true : false;
	Info.bUseCashBait = Tool.biCashBaitSerial > 0 ? true : false;
	if (Info.bUseCashBait == true && Tool.nFishingBaitInvenIdx >= 0)
		Info.bUseCashBait = false;

	const TItem * pRod;
	const TItem * pBait;
	if (nRequirementType1 > 0)
	{
		pRod = bUseCashRod ? GetItem()->GetCashInventory(Tool.biCashRodSerial) : GetItem()->GetInventory(Tool.nFishingRodInvenIdx);
		if (pRod == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;

		TItemData * pRodItemData = g_pDataManager->GetItemData(pRod->nItemID);
		if (pRodItemData == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;

		Info.nRodItemID = pRod->nItemID;

		if (bUseCashRod)
		{
			if (GetItem()->IsExpired(*pRod) == true)
				return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;

			Info.nMeritReduceFishingTime = pRodItemData->nTypeParam[0];
		}

		if (pRodItemData->nType != nRequirementType1)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;
	}

	if (nRequirementType2 > 0)
	{
		pBait = Info.bUseCashBait ? GetItem()->GetCashInventory(Tool.biCashBaitSerial) : GetItem()->GetInventory(Tool.nFishingBaitInvenIdx);
		if (pBait == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;

		TItemData * pBaitItemData = g_pDataManager->GetItemData(pBait->nItemID);
		if (pBaitItemData == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;		

		if (Info.bUseCashBait)
			Info.nMeritSuccessRate = pBaitItemData->nTypeParam[0];
		else
			Info.nBaitIndex = Tool.nFishingBaitInvenIdx;

		if (pBaitItemData->nType != nRequirementType2)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;

		Info.biBaitSerial = pBait->nSerial;
	}

	return ERROR_NONE;
}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
int CDNFarmUserSession::CheckFishingRequirement(int nRequirementType1, int nUsingInvenIndex1, int nRequirementType2, int nUsingInvenIndex2, INT64 &nBaitSerial)
{
	//Whether there is empty space in inven
	if (GetItem()->FindBlankInventorySlotCount() <= 0)
		return ERROR_FISHING_INSUFFICIENCY_INVENTORY;

	const TItem * pRod;
	const TItem * pBait;
	if (nRequirementType1 > 0)
	{
		pRod = GetItem()->GetInventory(nUsingInvenIndex1);
		if (pRod == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;

		TItemData * pRodItemData = g_pDataManager->GetItemData(pRod->nItemID);
		if (pRodItemData == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;

		if (pRodItemData->nType != nRequirementType1)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_ROD;
	}

	if (nRequirementType2 > 0)
	{
		pBait = GetItem()->GetInventory(nUsingInvenIndex2);
		if (pBait == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;

		TItemData * pBaitItemData = g_pDataManager->GetItemData(pBait->nItemID);
		if (pBaitItemData == NULL)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;		

		if (pBaitItemData->nType != nRequirementType2)
			return ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT;

		nBaitSerial = pBait->nSerial;
	}

	return ERROR_NONE;
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

bool CDNFarmUserSession::SetFishingStatus(CFishingArea * pFishingArea)
{
	if (pFishingArea == NULL)
		return false;

	if (m_pFishingStatus == NULL)
		m_pFishingStatus = new CDNFishingStatus(this, pFishingArea);		//Allocate if not already allocated.

	if (m_pFishingStatus == NULL)
		return false;

	return m_pFishingStatus->SetFishingPattern(pFishingArea);
}

bool CDNFarmUserSession::GetIsFishing()
{
	return m_pFishingStatus == NULL ? false : m_pFishingStatus->IsFishing();
}

bool CDNFarmUserSession::GetIsFishingReward()
{
	return m_pFishingStatus == NULL ? false : m_pFishingStatus->IsFishingReward();
}

#ifdef PRE_ADD_CASHFISHINGITEM
int CDNFarmUserSession::GetFishingRodItemID()
{
	return m_pFishingStatus->GetRodItemID();
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

