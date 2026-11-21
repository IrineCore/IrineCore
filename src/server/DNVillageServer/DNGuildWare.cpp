#include "stdafx.h"
#include "DNGuildWare.h"
#include "DNGuildVillage.h"

#include "DNUserSession.h"

#include "DNGameDataManager.h"
#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNAuthManager.h"

#include "TimeSet.h"

extern TVillageConfig g_Config;
extern CDNDBConnectionManager* g_pDBConnectionManager;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////						PUBLIC FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDNGuildWare::CDNGuildWare()
{
	Reset();
}

CDNGuildWare::~CDNGuildWare()
{
	Reset();
}

void CDNGuildWare::Open(CDNGuildVillage* pGuildVillage, const TAGetGuildWareInfo* pPacket)
{
	// Get DB object
	if(!m_pDBCon)
		m_pDBCon = g_pDBConnectionManager->GetDBConnection( m_cDBThreadID );
	
	// Manage parent object
	m_GuildOwner = pGuildVillage;

	// Save warehouse item list (save by serial)
	m_MapWareItem.clear();
	
	INT64 nSerial = 0;
	for(int i=0; i<pPacket->cItemListCount; i++)
	{
		nSerial = pPacket->ItemList[i].Item.nSerial;
		m_MapWareItem [nSerial] = pPacket->ItemList[i];
	}

	// Initialize slot version
	UpdateVersionDate();

	// Request the latest wear slot list from each village server via master.
	g_pMasterConnection->SendGuildWareInfo(m_GuildOwner->GetUID());
}

void CDNGuildWare::ReportWareSlot()
{
	// Save warehouse list to DB.
	if(m_pDBCon)
	{
		TGuildWareSlot WareSlotList[GUILD_WAREHOUSE_MAX];
		BYTE cSlotListCount = GetSlotList(WareSlotList);
		m_pDBCon->QuerySetGuildWareInfo(m_cDBThreadID, m_GuildOwner->GetUID().nDBID, g_Config.nWorldSetID, cSlotListCount, WareSlotList);
	}
	else
		_DANGER_POINT();
}

// FROM USER
// Guild item
bool CDNGuildWare::OnRecvMoveGuildItem(CDNUserSession* pUserSession, CSMoveGuildItem* pPacket)
{
	int nRet = ERROR_NONE;

	switch(pPacket->cMoveType)
	{
	case MoveType_GuildWare:
		nRet = ItemInWare(pUserSession, pPacket); 
		break;

	case MoveType_InvenToGuildWare:
		nRet = ItemInvenToWare(pUserSession, pPacket);
		break;

	case MoveType_GuildWareToInven:
		nRet = ItemWareToInven(pUserSession, pPacket);
		break;

	default:
		_DANGER_POINT();
		break;
	}

	// Error handling
	if(nRet != ERROR_NONE)
	{
		pUserSession->SendMoveGuildItem(pPacket->cMoveType, 0, 0, NULL, NULL, nRet);
		return false;
	}
	
	return true;
}

// Guild coin 
bool CDNGuildWare::OnRecvMoveGuildCoin(CDNUserSession* pUserSession, CSMoveGuildCoin* pMove)
{
	int nRet = ERROR_NONE;

	switch(pMove->cMoveType)
	{
	case MoveType_InvenToGuildWare:
		nRet = CoinInvenToWare(pUserSession, pMove);
		break;

	case MoveType_GuildWareToInven:
		nRet = CoinWareToInven(pUserSession, pMove);
		break;

	default:
		_DANGER_POINT();
		break;
	}

	if(nRet != ERROR_NONE)
	{
		pUserSession->SendMoveGuildCoin(pMove->cMoveType, 0, 0, 0, 0, nRet);
		return false;
	}

	return true;
}

// Guild warehouse history
void CDNGuildWare::OnRecvGetWareHistory(CDNUserSession* pUserSession, CSGetGuildWareHistory* pPacket)
{
	// Get history list from DB
	if(m_pDBCon)
		m_pDBCon->QueryGetGuildWareHistory(m_cDBThreadID, pUserSession->GetAccountDBID(), pUserSession->GetCharacterDBID(), m_GuildOwner->GetUID().nDBID, g_Config.nWorldSetID, pPacket->nIndex);
}

//FROM MASTER
bool CDNGuildWare::OnRecvGuildWareInfo(const MAGuildWareInfo *pPacket)
{
	MAGuildWareInfoResult result;
	memset(&result, 0, sizeof(result));

	result.nFromManagedID = pPacket->nManagedID;
	SaveWareResultPacket(result);

	g_pMasterConnection->SendGuildWareInfoResult(result);
	return true;
}

bool CDNGuildWare::OnRecvGuildWareInfoResult(MAGuildWareInfoResult *pPacket)
{
	UpdateSlotList(pPacket->VersionDate, pPacket->cSlotListCount, pPacket->WareSlotList);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////						PRIVATE FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDNGuildWare::Reset()
{
	m_MapWareItem.clear();
	m_pDBCon = NULL;
	m_GuildOwner = NULL;
	m_VersionDate = 0;
	m_cDBThreadID = 0;
}

BYTE CDNGuildWare::GetSlotList(TGuildWareSlot* pSlot)
{
	BYTE cListCount = 0;

	// Initialize
	memset(pSlot, 0x00, sizeof(TGuildWareSlot)*GUILD_WAREHOUSE_MAX);

	ScopeLock<CSyncLock> AutoLock(m_WareLock);

	TMapGuildWareItem::const_iterator con_iter = m_MapWareItem.begin();

	for(; con_iter != m_MapWareItem.end(); con_iter++)
	{
		pSlot[cListCount].cSlotIndex = con_iter->second.cSlotIndex;
		pSlot[cListCount].nSerial = con_iter->second.Item.nSerial;
		
		cListCount++;
		if(cListCount >= GUILD_WAREHOUSE_MAX)
			break;
	}

	return cListCount;
}

void CDNGuildWare::UpdateWareSlot(BYTE cCount, TGuildWareSlot* List)
{
	if (cCount >  GUILD_WAREHOUSE_MAX)
		return;

	TMapGuildWareItem::iterator iter = m_MapWareItem.begin();

	for(; iter != m_MapWareItem.end(); iter++)
	{
		for(BYTE i=0; i<cCount; i++)
		{
			if(iter->second.Item.nSerial == List[i].nSerial)
			{
				iter->second.cSlotIndex = List[i].cSlotIndex;
			}
		}
	}
}

size_t CDNGuildWare::GetWareItemCount()
{
	return m_MapWareItem.size();
}

__time64_t CDNGuildWare::GetVersionDate()
{
	return m_VersionDate;
}

void CDNGuildWare::UpdateVersionDate()
{
	m_TimeSet.Reset();
	m_VersionDate = m_TimeSet.GetTimeT64_LC();
}

void CDNGuildWare::SetVersionDate(__time64_t tVersion)
{
	m_VersionDate = tVersion;
}

BYTE CDNGuildWare::GetWareList(TItemInfo* pItemInfo)
{
	BYTE cListCount = 0;
	memset(pItemInfo, 0x00, sizeof(TItemInfo)*GUILD_WAREHOUSE_MAX);

	ScopeLock<CSyncLock> AutoLock(m_WareLock);

	TMapGuildWareItem::const_iterator con_iter = m_MapWareItem.begin();

	for(; con_iter != m_MapWareItem.end(); con_iter++)
	{
		if(cListCount >= GUILD_WAREHOUSE_MAX)
		{
			if (m_GuildOwner)
				g_Log.Log(LogType::_ERROR, L"[World:%d][DBID:%d] GetWareList - OverFlow\r\n", m_GuildOwner->GetUID().nWorldID, m_GuildOwner->GetUID().nDBID);
			break;
		}

		pItemInfo[cListCount] = con_iter->second;
		cListCount++;
	}

	return cListCount;
}

const TItem* CDNGuildWare::GetWareItem(INT64 biItemSerial)
{
	if(biItemSerial <= 0) 
		return NULL;

	if(m_MapWareItem.empty()) 
		return NULL;

	ScopeLock<CSyncLock> AutoLock(m_WareLock);

	TMapGuildWareItem::iterator iter = m_MapWareItem.find(biItemSerial);
	if(iter == m_MapWareItem.end())
		return NULL;

	return &(iter->second.Item);
}

const TItemInfo* CDNGuildWare::GetWareItemInfo(INT64 biItemSerial)
{
	if(biItemSerial <= 0) 
		return NULL;

	if(m_MapWareItem.empty()) 
		return NULL;

	ScopeLock<CSyncLock> AutoLock(m_WareLock);

	TMapGuildWareItem::iterator iter = m_MapWareItem.find(biItemSerial);
	if(iter == m_MapWareItem.end())
		return NULL;

	return &(iter->second);
}

int CDNGuildWare::AddWareItemInfo(TItemInfo& AddItem)
{
	ScopeLock<CSyncLock> AutoLock(m_WareLock);
	INT64 nSerial = AddItem.Item.nSerial;
	m_MapWareItem [nSerial] = AddItem;

	if (m_MapWareItem.size() > GUILD_WAREHOUSE_MAX)
	{
		if (m_GuildOwner)
			g_Log.Log(LogType::_ERROR, L"[World:%d][DBID:%d] AddWareItemInfo - OverFlow\r\n", m_GuildOwner->GetUID().nWorldID, m_GuildOwner->GetUID().nDBID);
	}

	return ERROR_NONE;
}

void CDNGuildWare::UpdateWareInfo(TItemInfo& ItemInfo)
{
	INT64 nUpdateSerial = ItemInfo.Item.nSerial;
	const TItemInfo* WareInfo = GetWareItemInfo(nUpdateSerial);

	if(WareInfo)
	{
		if(ItemInfo.Item.wCount <= 0)
		{
			// Delete info
			RemoveWareItem(nUpdateSerial);
		}
		else
		{
			// Update
			ScopeLock<CSyncLock> AutoLock(m_WareLock);
			*const_cast<TItemInfo*>(WareInfo) = ItemInfo;
		}
	}
	else
		AddWareItemInfo(ItemInfo);
}

int CDNGuildWare::RemoveWareItem(INT64 nSerial)
{
	ScopeLock<CSyncLock> AutoLock(m_WareLock);

	m_MapWareItem.erase(nSerial);

	return ERROR_NONE;
}

#ifdef PRE_ADD_JOINGUILD_SUPPORT
void CDNGuildWare::BroadCastRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession, bool bNoUpdate)
{
	UpdateVersionDate(); // Update warehouse version.

	// Notify other guild members excluding self and update personal warehouse version.
	m_GuildOwner->SendRefreshGuildItem(cType, biSrcSerial, biDestSerial, pSrcInfo, pDestInfo, pSession, bNoUpdate);

	g_pMasterConnection->SendRefreshGuildItem(m_GuildOwner->GetUID(), cType, biSrcSerial, biDestSerial, pSrcInfo, pDestInfo, GetVersionDate());
}
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
void CDNGuildWare::BroadCastRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession)
{
	UpdateVersionDate(); // Update warehouse version.

	// Notify other guild members excluding self and update personal warehouse version.
	m_GuildOwner->SendRefreshGuildItem(cType, biSrcSerial, biDestSerial, pSrcInfo, pDestInfo, pSession);

	g_pMasterConnection->SendRefreshGuildItem(m_GuildOwner->GetUID(), cType, biSrcSerial, biDestSerial, pSrcInfo, pDestInfo, GetVersionDate());
}
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT

void CDNGuildWare::BroadCastRefreshGuildCoin(INT64 biTotalCoin, CDNUserSession* pSession)
{
	m_GuildOwner->SendRefreshGuildCoin(biTotalCoin, pSession);

	g_pMasterConnection->SendRefreshGuildCoin(m_GuildOwner->GetUID(), biTotalCoin);
}

// Move item only within warehouse
int CDNGuildWare::ItemInWare(CDNUserSession* pUserSession, CSMoveGuildItem* pPacket)
{
	if(!m_pDBCon) return ERROR_DB;

	if(!pUserSession)
		return ERROR_GENERIC_INVALIDREQUEST;

	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem)
		return ERROR_ITEM_FAIL;

	if(pPacket->wCount <= 0)
		return ERROR_ITEM_NOTFOUND;

	if(pPacket->cDestIndex == pPacket->cSrcIndex)
		return ERROR_ITEM_INDEX_UNMATCH;

	int nGstorage = m_GuildOwner->GetInfo()->wGuildWareSize;

	if( (pPacket->cSrcIndex >= nGstorage) ) 
		return ERROR_ITEM_INDEX_UNMATCH;

	if( (pPacket->cDestIndex >= nGstorage) )
		return ERROR_ITEM_INDEX_UNMATCH;

	const TItemInfo* pSrcInfo = GetWareItemInfo(pPacket->biSrcItemSerial);
	if(!pSrcInfo) 
		return ERROR_ITEM_NOTFOUND;

	const TItem* pSrcItem = &pSrcInfo->Item;
	if(!pSrcItem)
		return ERROR_ITEM_NOTFOUND;

	// Error if a value greater than what is owned arrives!
	if(pPacket->wCount > pSrcItem->wCount)
		return ERROR_ITEM_OVERFLOW;

	// Inspect serial code
	if(pSrcItem->nSerial != pPacket->biSrcItemSerial) 
		return ERROR_ITEM_NOTFOUND;

	// Check max overlap count
	int nSrcWareOverlapCount = g_pDataManager->GetItemOverlapCount(pSrcItem->nItemID);
	if(nSrcWareOverlapCount <= 0) return ERROR_ITEM_FAIL;

	// Check guild info once more!
	const TGuildUID GuildUID = pUserSession->GetGuildUID();
	if(!GuildUID.IsSet())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	if(GuildUID != m_GuildOwner->GetUID())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Check guild member
	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());
	if(!pGuildMember)
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Check item storage permission
	if(!m_GuildOwner->CheckGuildInfoAuth(static_cast<eGuildRoleType>(pGuildMember->btGuildRole), static_cast<eGuildAuthType>(GUILDAUTH_TYPE_TAKEITEM)))
		return ERROR_GUILD_HAS_NO_AUTHORITY;

	const TItemInfo* pDestInfo = GetWareItemInfo(pPacket->biDestItemSerial);
	const TItem* pDestItem = NULL;
	if(pDestInfo)
		pDestItem = &pDestInfo->Item;

	if(pDestItem)
	{
		if( CDNUserItem::bIsDifferentItem( pSrcItem, pDestItem) )
			return pItem->NothingMoveItemInGuildWare(this, pPacket);

		// Check if source item can merge with target item
		int nDestWareOverlapCount = g_pDataManager->GetItemOverlapCount(pDestItem->nItemID);
		if(nDestWareOverlapCount <= 1)
			return pItem->NothingMoveItemInGuildWare(this, pPacket);
		if(pDestItem->wCount == nDestWareOverlapCount)
			return pItem->NothingMoveItemInGuildWare(this, pPacket);

		//Packet to send to DB
		TQMoveItemInGuildWare Move;
		memset(&Move, 0, sizeof(TQMoveItemInGuildWare));

		Move.nCharacterDBID		= pUserSession->GetCharacterDBID();
		Move.nGuildDBID			= GuildUID.nDBID;

		Move.SrcItem.cSlotIndex = pPacket->cSrcIndex;
		Move.SrcItem.nItemID	= pSrcItem->nItemID;
		Move.SrcItem.biSerial	= pSrcItem->nSerial;

		Move.DestItem.cSlotIndex = pPacket->cDestIndex;
		Move.DestItem.nItemID	= pDestItem->nItemID;
		Move.DestItem.biSerial	= pDestItem->nSerial;

		Move.nItemID			= pSrcItem->nItemID;
		Move.biItemSerial		= pSrcItem->nSerial;
		Move.wItemCount			= pSrcItem->wCount;
		Move.nMapIndex			= pUserSession->GetMapIndex();

		// Warehouse has no split. Only need to handle merge

		// When exceeding MAX value
		if(pDestItem->wCount + pPacket->wCount > nDestWareOverlapCount)
		{
			int nGapCount = nDestWareOverlapCount - pDestItem->wCount;
			// Force-split source item and send.
			Move.wSplitItemCount = nGapCount;
			Move.biNewItemSerial = MakeItemSerial();

			Move.SrcItem.wCount = pSrcItem->wCount - nGapCount;
			Move.DestItem.wCount = pDestItem->wCount + nGapCount;
		}
		else{
			if(pSrcItem->wCount == pPacket->wCount){	// If equal
				Move.SrcItem.wCount = 0;
				Move.SrcItem.biNewSerial = 0;
			}
			else{
				Move.SrcItem.wCount = pSrcItem->wCount - pPacket->wCount;
			}

			Move.DestItem.wCount = pDestItem->wCount + pPacket->wCount;
		}

		Move.biMergeTargetItemSerial = pDestItem->nSerial;
		Move.wMergeItemCount = pDestItem->wCount;

		m_pDBCon->QueryMoveItemInGuildWare(m_cDBThreadID, pUserSession->GetAccountDBID(), g_Config.nWorldSetID, &Move);
	}
	else
	{
		TItemInfo SrcInfo ={0,};
		SrcInfo.cSlotIndex = pPacket->cSrcIndex;
		SrcInfo.Item.nSerial = pPacket->biSrcItemSerial;

		TItemInfo DestInfo;
		DestInfo.cSlotIndex = pPacket->cDestIndex;
		DestInfo.Item = *pSrcItem;

		RemoveWareItem(pPacket->biSrcItemSerial);
		AddWareItemInfo(DestInfo);

		pUserSession->SendMoveGuildItem(MoveType_GuildWare, pPacket->biSrcItemSerial, pPacket->biDestItemSerial, &SrcInfo, &DestInfo, ERROR_NONE);
		BroadCastRefreshGuildItem(MoveType_GuildWare, pPacket->biSrcItemSerial, pPacket->biDestItemSerial, &SrcInfo, &DestInfo, pUserSession);
	}
	
	return ERROR_NONE;
}

// Move item from Inven -> Warehouse
int CDNGuildWare::ItemInvenToWare(CDNUserSession* pUserSession, CSMoveGuildItem* pPacket)
{
	if(!m_pDBCon) return ERROR_DB;

	if(!pUserSession)
		return ERROR_GENERIC_INVALIDREQUEST;

	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem)
		return ERROR_ITEM_FAIL;

	if(pPacket->wCount <= 0)
		return ERROR_ITEM_NOTFOUND;

	int nGstorage = m_GuildOwner->GetInfo()->wGuildWareSize;

	if((pPacket->cDestIndex >= nGstorage)) 
		return ERROR_ITEM_INDEX_UNMATCH;

	if (pItem->CheckRangeInventoryIndex(pPacket->cSrcIndex) == false)
		return ERROR_ITEM_INDEX_UNMATCH;

	// Inspect inven item
	const TItem* pInven = pItem->GetInventory(pPacket->cSrcIndex);
	if(!pInven) 
		return ERROR_ITEM_NOTFOUND;

	// Check overflow, serial, and soulbound
	int nRet = pItem->IsTradeEnableItem(ITEMPOSITION_INVEN, pPacket->cSrcIndex, pPacket->biSrcItemSerial, pPacket->wCount);
	if(nRet != ERROR_NONE)
		return nRet;

	// Check max overlap count
	int nInvenOverlapCount = g_pDataManager->GetItemOverlapCount(pInven->nItemID);
	if(nInvenOverlapCount <= 0) 
		return ERROR_ITEM_FAIL;

	// Check guild info once more!
	const TGuildUID GuildUID = pUserSession->GetGuildUID();
	if(!GuildUID.IsSet())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	if(GuildUID != m_GuildOwner->GetUID())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Check guild member
	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());
	if(!pGuildMember)
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Check item storage permission
	if(!m_GuildOwner->CheckGuildInfoAuth(static_cast<eGuildRoleType>(pGuildMember->btGuildRole), static_cast<eGuildAuthType>(GUILDAUTH_TYPE_STOREITEM)))
		return ERROR_GUILD_HAS_NO_AUTHORITY;

	// Packet to send to DB
	TQMoveInvenToGuildWare Move;
	memset(&Move, 0, sizeof(Move));

	Move.nCharacterDBID		= pUserSession->GetCharacterDBID();
	Move.nGuildDBID			= GuildUID.nDBID;

	Move.InvenItem.cSlotIndex = pPacket->cSrcIndex;
	Move.InvenItem.nItemID	= pInven->nItemID;
	Move.InvenItem.biSerial	= pInven->nSerial;

	Move.GuildWareItem.cSlotIndex = pPacket->cDestIndex;

	Move.nItemID			= pInven->nItemID;
	Move.biItemSerial		= pInven->nSerial;
	Move.wItemCount			= pInven->wCount;
	Move.wMovingItemCount	= pPacket->wCount;
	Move.nMapIndex			= pUserSession->GetMapIndex();

	// Check if inven item came from Split
	if(pInven->wCount > pPacket->wCount)
		Move.biNewItemSerial = MakeItemSerial();
	else
		Move.biNewItemSerial = 0;

	if(pInven->wCount == pPacket->wCount){	// If equal
		Move.InvenItem.wCount = 0;
		Move.InvenItem.biNewSerial = 0;
	}
	else{
		Move.InvenItem.wCount = pInven->wCount - pPacket->wCount;
	}

	// Warehouse item
	const TItem* pWare = GetWareItem(pPacket->biDestItemSerial);
	// If warehouse item exists, attempt Merge!
	if(pWare)
	{
		// Check if inven item can Merge with warehouse item
		if( CDNUserItem::bIsDifferentItem( pInven, pWare ) )
			return pItem->NothingMoveInvenToGuildWare(this, pPacket);

		// Check if inven item can Merge with warehouse item
		int nWareOverlapCount = g_pDataManager->GetItemOverlapCount(pWare->nItemID);
		if(pWare->wCount == nWareOverlapCount)
			return pItem->NothingMoveInvenToGuildWare(this, pPacket);

		Move.GuildWareItem.nItemID = pWare->nItemID;
		Move.GuildWareItem.biSerial	= pWare->nSerial;
		Move.GuildWareItem.wCount = pWare->wCount;

		// When exceeding MAX value
		if(pWare->wCount + pPacket->wCount > nWareOverlapCount)
		{
			// Force split inven item and send.
			int nGapCount = nWareOverlapCount - pWare->wCount;
			Move.wMovingItemCount = nGapCount;
			Move.biNewItemSerial = MakeItemSerial();

			Move.InvenItem.wCount = pInven->wCount - nGapCount;
			Move.GuildWareItem.wCount = pWare->wCount + nGapCount;
		}
		else{
			Move.GuildWareItem.wCount = pWare->wCount + pPacket->wCount;
		}

		Move.biMergeTargetItemSerial = pWare->nSerial;
		Move.wMergeItemCount = pWare->wCount;
	}
	else{
		Move.GuildWareItem.nItemID = pInven->nItemID;
		Move.GuildWareItem.biNewSerial = (Move.biNewItemSerial > 0) ? Move.biNewItemSerial : pInven->nSerial;
		Move.GuildWareItem.wCount = pPacket->wCount;
	}

	m_pDBCon->QueryMoveInvenToGuildWare(m_cDBThreadID, pUserSession->GetAccountDBID(), g_Config.nWorldSetID, &Move);
	return ERROR_NONE;
}

// Move item from warehouse -> inventory
int CDNGuildWare::ItemWareToInven(CDNUserSession* pUserSession, CSMoveGuildItem* pPacket)
{
	if(!m_pDBCon) return ERROR_DB;
	if(!pUserSession) return ERROR_GENERIC_INVALIDREQUEST;

	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem) return ERROR_ITEM_FAIL;

	if(pPacket->wCount <= 0)
		return ERROR_ITEM_NOTFOUND;

	int nGstorage = m_GuildOwner->GetInfo()->wGuildWareSize;

	if((pPacket->cSrcIndex >= nGstorage))
		return ERROR_ITEM_INDEX_UNMATCH;
	
	if (pItem->CheckRangeInventoryIndex(pPacket->cDestIndex) == false)
		return ERROR_ITEM_INDEX_UNMATCH;

	// Inspect warehouse item
	const TItem* pWare = GetWareItem(pPacket->biSrcItemSerial);
	if(!pWare)
		return ERROR_ITEM_NOTFOUND;

	// Error if a value greater than what is owned arrives!
	if(pPacket->wCount > pWare->wCount)
		return ERROR_ITEM_OVERFLOW;

	// Inspect serial code
	if(pWare->nSerial != pPacket->biSrcItemSerial) 
		return ERROR_ITEM_NOTFOUND;

	// Check max overlap count
	int nInvenOverlapCount = g_pDataManager->GetItemOverlapCount(pWare->nItemID);
	if(nInvenOverlapCount <= 0) 
		return ERROR_ITEM_FAIL;

	// Check guild info once more!
	const TGuildUID GuildUID = pUserSession->GetGuildUID();
	if(!GuildUID.IsSet())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	if(GuildUID != m_GuildOwner->GetUID())
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Verify guild member (withdraw item)
	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());
	if(!pGuildMember)
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Check permission to withdraw item
	if(!m_GuildOwner->CheckGuildInfoAuth(static_cast<eGuildRoleType>(pGuildMember->btGuildRole), static_cast<eGuildAuthType>(GUILDAUTH_TYPE_TAKEITEM)))
		return ERROR_GUILD_HAS_NO_AUTHORITY;

#if defined( PRE_ADD_CHANGEGUILDROLE )
	// Get max available usage count (check only non-guild-master and non-submaster members)
	if(pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER &&
		pGuildMember->btGuildRole != GUILDROLE_TYPE_SUBMASTER)
#else
	// Get max available usage count (check only non-guild-master members)
	if(pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER)
#endif
	{
		int nMaxItemCount = m_GuildOwner->GetInfo()->nRoleMaxItem[pGuildMember->btGuildRole];

		// Check max value
		if(nMaxItemCount == -1)
			return ERROR_MAX_TAKE_GUILDITEM;

		// Create a temporary withdraw count.
		USHORT uTakeCount = pGuildMember->TakeWareItemCount + 1;

		// Verify allowed usage count
		if(uTakeCount > nMaxItemCount)
			return ERROR_MAX_TAKE_GUILDITEM;
	}

	// Build packet to send to DB
	TQMoveGuildWareToInven Move;
	memset(&Move, 0, sizeof(Move));

	Move.nCharacterDBID		= pUserSession->GetCharacterDBID();
	Move.nGuildDBID			= GuildUID.nDBID;

	Move.GuildWareItem.cSlotIndex = pPacket->cSrcIndex;
	Move.GuildWareItem.nItemID = pWare->nItemID;
	Move.GuildWareItem.biSerial	= pWare->nSerial;

	Move.InvenItem.cSlotIndex = pPacket->cDestIndex;

	Move.nItemID			= pWare->nItemID;
	Move.biItemSerial		= pWare->nSerial;
	Move.wItemCount			= pWare->wCount;
	Move.wMovingItemCount	= pPacket->wCount;
	Move.nMapIndex			= pUserSession->GetMapIndex();

#if defined( PRE_ADD_CHANGEGUILDROLE )
	if( pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER &&
		pGuildMember->btGuildRole != GUILDROLE_TYPE_SUBMASTER )
#else
	if(pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER)
#endif
		Move.nMaxDailyTakeItemCount = m_GuildOwner->GetInfo()->nRoleMaxItem[pGuildMember->btGuildRole];
	else
	{
		Move.nMaxDailyTakeItemCount = -1;
	}
	// Check if warehouse item was split
	if(pWare->wCount > pPacket->wCount)
		Move.biNewItemSerial = MakeItemSerial();
	else
		Move.biNewItemSerial = 0;

	if(pWare->wCount == pPacket->wCount){	// If equal
		Move.GuildWareItem.wCount = 0;
		Move.GuildWareItem.biNewSerial = 0;
	}
	else{
		Move.GuildWareItem.wCount = pWare->wCount - pPacket->wCount;
	}

	// Inven item
	const TItem* pInven = pItem->GetInventory(pPacket->cDestIndex);
	if(pInven)
	{
		// Check if warehouse item can be merged with inventory item
		if( CDNUserItem::bIsDifferentItem( pWare, pInven ) == true )
			return pItem->NothingMoveGuildWareToInven(this, pPacket, pGuildMember->TakeWareItemCount);

		// Check MAX value
		int nInvenOverlapCount = g_pDataManager->GetItemOverlapCount(pInven->nItemID);
		if(pInven->wCount == nInvenOverlapCount)
			return pItem->NothingMoveGuildWareToInven(this, pPacket, pGuildMember->TakeWareItemCount);

		Move.InvenItem.nItemID	= pInven->nItemID;
		Move.InvenItem.biSerial	= pInven->nSerial;
		Move.InvenItem.wCount	= pInven->wCount;
		
		if(pInven->wCount + pPacket->wCount > nInvenOverlapCount)
		{
			// Force-split warehouse item and send.
			int nGapCount = nInvenOverlapCount - pInven->wCount;
			Move.wMovingItemCount = nGapCount;
			Move.biNewItemSerial = MakeItemSerial();

			Move.GuildWareItem.wCount = pWare->wCount - nGapCount;
			Move.InvenItem.wCount = pInven->wCount + nGapCount;
		}
		else{
			Move.InvenItem.wCount = pInven->wCount + pPacket->wCount;
		}

		Move.biMergeTargetItemSerial = pInven->nSerial;
		Move.wMergeItemCount = pInven->wCount;
	}
	else{
		Move.InvenItem.nItemID = pWare->nItemID;
		Move.InvenItem.biNewSerial = (Move.biNewItemSerial > 0) ? Move.biNewItemSerial : pWare->nSerial;
		Move.InvenItem.wCount = pPacket->wCount;
	}

	m_pDBCon->QueryMoveGuildWareToInven(m_cDBThreadID, pUserSession->GetAccountDBID(), g_Config.nWorldSetID, &Move);
	return ERROR_NONE;
}

// FROM DB Server
// Move item between guild warehouses
int CDNGuildWare::OnMoveItemInGuildWare(CDNUserSession* pUserSession, TAMoveItemInGuildWare* pResult)
{
	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem)
		return ERROR_ITEM_FAIL;

	return pItem->InGuildWare(this, pResult);
}

// Result of moving item from Inven -> Guild warehouse
int CDNGuildWare::OnMoveInvenToGuildWare(CDNUserSession* pUserSession, TAMoveInvenToGuildWare* pResult)
{
	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem)
		return ERROR_ITEM_FAIL;

	return pItem->FromInvenToGuildWare(this, pResult);
}

// Move item from guild storage -> inventory
int CDNGuildWare::OnMoveGuildWareToInven(CDNUserSession* pUserSession, TAMoveGuildWareToInven* pResult)
{
	CDNUserItem* pItem = pUserSession->GetItem();
	if(!pItem)
		return ERROR_ITEM_FAIL;

	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());
	if(pGuildMember)
	{
		pGuildMember->TakeWareItemCount = pResult->nDailyTakeItemCount;
		pGuildMember->LastWareUseDate = pResult->tUseDate;
	}

	return pItem->FromGuildWareToInven(this, pResult);
}

// Move coin from Inven -> Warehouse
int CDNGuildWare::CoinInvenToWare(CDNUserSession* pUserSession, CSMoveGuildCoin* pPacket)
{
	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());
	if (!pGuildMember)
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	// Coin value range check
	if(pPacket->nMoveCoin <= 0) 
		return ERROR_ITEM_INSUFFICIENCY_MONEY;

	if(pPacket->nMoveCoin > GUILDWARE_COINMAX)
		return ERROR_ITEM_INVENTOWARE02;

	// Check inven gold
	if(!pUserSession->CheckEnoughCoin(pPacket->nMoveCoin))
		return ERROR_ITEM_INVENTOWARE01;

	// No space to put in warehouse.
	if(pPacket->nMoveCoin + GetWarehouseCoin() > GUILDWARE_COINMAX)
		return ERROR_ITEM_INVENTOWARE02;

	TQGuildWareHouseCoin Packet;

	Packet.cMoveType	= pPacket->cMoveType;
	Packet.bInOutFlag	= false; // Deposit
	Packet.nMoveCoin	= pPacket->nMoveCoin;
	Packet.nGuildDBID   = pUserSession->GetGuildUID().nDBID;
	Packet.nCharacterDBID = pUserSession->GetCharacterDBID();
	Packet.nMapIndex	= pUserSession->GetMapIndex();
	_strcpy(Packet.szIP, _countof(Packet.szIP), pUserSession->GetIp(), (int)strlen(pUserSession->GetIp()));

	if(m_pDBCon)
		m_pDBCon->QueryGuildWarehouseCoin(m_cDBThreadID, pUserSession->GetAccountDBID(), g_Config.nWorldSetID, &Packet);

	return ERROR_NONE;
}

// Move coin from warehouse -> inventory
int CDNGuildWare::CoinWareToInven(CDNUserSession* pUserSession, CSMoveGuildCoin* pPacket)
{

	// Coin value range check
	if((pPacket->nMoveCoin <= 0) ||(pPacket->nMoveCoin > GUILDWARE_COINMAX)) 
		return ERROR_ITEM_INVENTOWARE01;

	if(pPacket->nMoveCoin > GetWarehouseCoin())			// No money in warehouse
		return ERROR_ITEM_WARETOINVEN01;	

	if(!pUserSession->CheckMaxCoin(pPacket->nMoveCoin))	// No space in inven
		return ERROR_ITEM_WARETOINVEN02;


	// Verify guild member (withdrawal)
	TGuildMember* pGuildMember = m_GuildOwner->GetMemberInfo(pUserSession->GetCharacterDBID());

	if(pGuildMember)
	{
		// Check if user has withdrawal permission
		if(!m_GuildOwner->CheckGuildInfoAuth(static_cast<eGuildRoleType>(pGuildMember->btGuildRole), static_cast<eGuildAuthType>(GUILDAUTH_TYPE_WITHDRAW)))
			return ERROR_GUILD_HAS_NO_AUTHORITY;

#if defined( PRE_ADD_CHANGEGUILDROLE )
		// Get max available usage count (check only non-guild-master and non-submaster members)
		if( pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER &&
			pGuildMember->btGuildRole != GUILDROLE_TYPE_SUBMASTER )
#else
		// Get max available usage count (check only non-guild-master members)
		if(pGuildMember->btGuildRole != GUILDROLE_TYPE_MASTER)
#endif
		{
			// Max withdrawal amount
			int nMaxCoinCount = m_GuildOwner->GetInfo()->nRoleMaxCoin[pGuildMember->btGuildRole];

			// Calculate the temporary withdrawal amount.
			INT64 nTotalCoin = pGuildMember->WithdrawCoin + pPacket->nMoveCoin;

			// Verify withdrawal amount is allowed
			if(nTotalCoin > nMaxCoinCount)
				return ERROR_MAX_WITHDRAW_GUILDCOIN;
		}
	}
	else
		return ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;

	TQGuildWareHouseCoin Packet;

	Packet.cMoveType	= pPacket->cMoveType;
	Packet.bInOutFlag	= true; // Withdraw
	Packet.nMoveCoin	= (int)pPacket->nMoveCoin;
	Packet.nGuildDBID   = pUserSession->GetGuildUID().nDBID;
	Packet.nCharacterDBID = pUserSession->GetCharacterDBID();
	Packet.nMapIndex	= pUserSession->GetMapIndex();
	_strcpy(Packet.szIP, _countof(Packet.szIP), pUserSession->GetIp(), (int)strlen(pUserSession->GetIp()));

	if(m_pDBCon)
		m_pDBCon->QueryGuildWarehouseCoin(m_cDBThreadID, pUserSession->GetAccountDBID(), g_Config.nWorldSetID, &Packet);

	return ERROR_NONE;
}

// Guild coin
int CDNGuildWare::OnModGuildCoin(CDNUserSession* pUserSession, TAGuildWareHouseCoin* pResult)
{
	if(!pUserSession)
		return ERROR_GENERIC_INVALIDREQUEST;

	INT64 nWithdrawCoin = 0;
	// Deposit result
	if(pResult->cMoveType == MoveType_InvenToGuildWare)
	{
		pUserSession->DelCoin(pResult->nMoveCoin, DBDNWorldDef::CoinChangeCode::DoNotDBSave, pUserSession->GetGuildUID().nDBID);
	}
	else if(pResult->cMoveType == MoveType_GuildWareToInven)	// Withdraw
	{
		pUserSession->AddCoin(pResult->nMoveCoin, DBDNWorldDef::CoinChangeCode::DoNotDBSave, pUserSession->GetGuildUID().nDBID);
		nWithdrawCoin = m_GuildOwner->UpdateMemberCoin(pUserSession->GetCharacterDBID(), pResult->nMoveCoin, pResult->tUseDate);
	}
	else
		return ERROR_GENERIC_INVALIDREQUEST;

	SetWarehouseCoin(pResult->biTotalCoin);

	pUserSession->SendMoveGuildCoin(pResult->cMoveType, pResult->nMoveCoin, pUserSession->GetCoin(), GetWarehouseCoin(), nWithdrawCoin, ERROR_NONE);
	BroadCastRefreshGuildCoin(GetWarehouseCoin(), pUserSession);
	
	return ERROR_NONE;
}

#ifdef PRE_ADD_JOINGUILD_SUPPORT
int	CDNGuildWare::OnAddGuildCoin(CDNUserSession* pUserSession, INT64 nAddCoin)
{
	if(!pUserSession)
		return ERROR_GENERIC_INVALIDREQUEST;

	SetWarehouseCoin(GetWarehouseCoin() + nAddCoin);

	BroadCastRefreshGuildCoin(GetWarehouseCoin(), pUserSession);
	pUserSession->SendRefreshGuildCoin(GetWarehouseCoin());
	return ERROR_NONE;
}

int CDNGuildWare::OnAddInGuildWare(CDNUserSession * pUserSession, DBPacket::TTakeAttachInfo * pItem)
{
	TItemInfo DestInfo;
	for (int i = 0; i < MAILATTACHITEMMAX; i++)
	{
		if (pItem->TakeItem[i].Item.nItemID <= 0) continue;
		memset(&DestInfo, 0, sizeof(TItemInfo));
		DestInfo = pItem->TakeItem[i];

		AddWareItemInfo(DestInfo);
		BroadCastRefreshGuildItem(MoveType_MailToGuildWare, 0, DestInfo.Item.nSerial, NULL, &DestInfo, pUserSession, true);
	}
	return ERROR_NONE;
}
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT

void CDNGuildWare::SetWarehouseCoin(INT64 biTotalCoin)
{
	if(biTotalCoin < 0 || biTotalCoin > GUILDWARE_COINMAX)
		return;

	m_GuildOwner->SetInfo()->ui64GuildMoney = biTotalCoin;
}

void CDNGuildWare::ChangeWareSize(CDNUserBase* pUserObject, short wSize)
{
	if(m_pDBCon)
		m_pDBCon->QueryChangeGuildWareSize(m_cDBThreadID, pUserObject->GetAccountDBID(), pUserObject->GetCharacterDBID(), m_GuildOwner->GetUID().nDBID, g_Config.nWorldSetID, wSize);
}

INT64 CDNGuildWare::GetWarehouseCoin()
{
	if(m_GuildOwner->GetInfo()->ui64GuildMoney <=0)
		m_GuildOwner->SetInfo()->ui64GuildMoney = 0;

	if(m_GuildOwner->GetInfo()->ui64GuildMoney >= GUILDWARE_COINMAX) 
		m_GuildOwner->SetInfo()->ui64GuildMoney = GUILDWARE_COINMAX;

	return m_GuildOwner->GetInfo()->ui64GuildMoney;
}

void CDNGuildWare::UpdateSlotList(__time64_t tVersion, BYTE cCount, TGuildWareSlot* List)
{
	// Check if it is the latest version.
	if(m_VersionDate > tVersion)
		return;

	// Save slot list
	UpdateWareSlot(cCount, List);

	// Version update
	m_VersionDate = tVersion;
}

void CDNGuildWare::SaveWareResultPacket(MAGuildWareInfoResult& result)
{
	result.GuildUID = m_GuildOwner->GetUID();

	// Copy slot list
	result.cSlotListCount = GetSlotList(result.WareSlotList);

	// Version
	result.VersionDate = m_VersionDate;
}

INT64 CDNGuildWare::MakeItemSerial()
{
	return MakeSerial( static_cast<short>(g_pAuthManager->GetServerID()));
}