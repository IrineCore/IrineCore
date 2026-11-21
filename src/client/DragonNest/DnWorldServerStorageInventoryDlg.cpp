#include "StdAfx.h"

#ifdef PRE_ADD_SERVER_WAREHOUSE

#include "DnWorldServerStorageInventoryDlg.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnCommonUtil.h"
#include "DnItemSlotButton.h"
#include "DnWorldServerStorageInventory.h"
#include "DnWorldServerStorageDlg.h"
#include "DnWorldServerStorageConfirmDlg.h"
#include "DnWorldServerStorageChargeConfirmDlg.h"
#include "DnWorldServerStorageDrawConfirmDlg.h"
#include "DnMainMenuDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnWorldServerStorageInventoryDlg::CDnWorldServerStorageInventoryDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_InvenType(Storage::WorldServerStorage::eNormal)
{
	m_pWorldServerStorageDlg = NULL;
	m_pCountStatic = NULL;
	m_pNoticeStatic = NULL;
	m_pChargeItemSlot = NULL;
	m_pChargeItemSlotStatic = NULL;
	m_pNoticeOnNoChargeStatic = NULL;
	m_ChargeType = Storage::WorldServerStorage::eFree;
	m_pChargeItem = NULL;

	m_pConfirmDlg = NULL;
	m_pSplitConfirmDlg = NULL;
	m_pChargeConfirmDlg = NULL;
	m_pChargeSplitConfirmDlg = NULL;
	m_pDrawConfirmDlg = NULL;
	m_pDrawSplitConfirmDlg = NULL;
}

CDnWorldServerStorageInventoryDlg::~CDnWorldServerStorageInventoryDlg(void)
{
	SAFE_DELETE(m_pChargeItem);
	SAFE_DELETE(m_pConfirmDlg);
	SAFE_DELETE(m_pSplitConfirmDlg);

	SAFE_DELETE(m_pChargeConfirmDlg);
	SAFE_DELETE(m_pChargeSplitConfirmDlg);

	SAFE_DELETE(m_pDrawConfirmDlg);
	SAFE_DELETE(m_pDrawSplitConfirmDlg);

	SAFE_DELETE(m_pWorldServerStorageDlg);
}

void CDnWorldServerStorageInventoryDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "StorageServerDlg.ui" ).c_str(), bShow );
}

void CDnWorldServerStorageInventoryDlg::InitialUpdate()
{
	m_pConfirmDlg = new CDnWorldServerStorageConfirmDlg(UI_TYPE_MODAL, NULL, FREE_CONFIRM_DLG, this);
	m_pConfirmDlg->Initialize(false);

	m_pSplitConfirmDlg = new CDnWorldServerStorageConfirmExDlg(UI_TYPE_MODAL, NULL, FREE_CONFIRM_EX_DLG, this);
	m_pSplitConfirmDlg->Initialize(false);

	m_pChargeConfirmDlg = new CDnWorldServerStorageChargeConfirmDlg(UI_TYPE_MODAL, NULL, CHARGE_CONFIRM_DLG, this);
	m_pChargeConfirmDlg->Initialize(false);

	m_pChargeSplitConfirmDlg = new CDnWorldServerStorageChargeConfirmExDlg(UI_TYPE_MODAL, NULL, CHARGE_CONFIRM_EX_DLG, this);
	m_pChargeSplitConfirmDlg->Initialize(false);

	m_pDrawConfirmDlg = new CDnWorldServerStorageDrawConfirmDlg(UI_TYPE_MODAL, NULL, DRAW_CONFIRM_DLG, this);
	m_pDrawConfirmDlg->Initialize(false);

	m_pDrawSplitConfirmDlg = new CDnWorldServerStorageDrawConfirmExDlg(UI_TYPE_MODAL, NULL, DRAW_CONFIRM_EX_DLG, this);
	m_pDrawSplitConfirmDlg->Initialize(false);

	OnInitialUpdate();

	m_pCountStatic = GetControl<CEtUIStatic>("ID_TEXT_COUNT");
	m_pCountStatic->Show(false);
	m_pNoticeStatic = GetControl<CEtUIStatic>("ID_TEXT_NOTICE");
	m_pNoticeStatic->Show(false);
	m_pChargeItemSlot = GetControl<CDnItemSlotButton>("ID_BT_ITEM");
	m_pChargeItemSlot->Show(false);
	m_pChargeItemSlotStatic = GetControl<CEtUIStatic>("ID_STATIC_SLOT");
	m_pChargeItemSlotStatic->Show(false);
	m_pNoticeOnNoChargeStatic = GetControl<CEtUIStatic>("ID_TEXT_NORMAL");
	m_pNoticeOnNoChargeStatic->Show(false);
}

void CDnWorldServerStorageInventoryDlg::OnInitialUpdate()
{
	m_InvenType = Storage::WorldServerStorage::eNormal;

	m_pWorldServerStorageDlg = new CDnWorldServerStorageDlg(UI_TYPE_CHILD, this);
	m_pWorldServerStorageDlg->Initialize( true );
	m_pWorldServerStorageDlg->SetSlotType(ST_STORAGE_WORLDSERVER_NORMAL);

	GetItemTask().GetWorldServerStorageInventory().SetInventoryDialog(this);
}

void CDnWorldServerStorageInventoryDlg::UpdateCoin()
{
	Storage::WorldServerStorage::eChargeType type = Storage::WorldServerStorage::eFree;
	int storageType = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_PAYMENT);
	if( storageType == 1) // Paid coin method
	{
		int nChargeItemID = StorageUIDef::eChargeCoinItemID;
		int nChargeItemAllCount = GetItemTask().FindItemCountFromItemType( ITEMTYPE_SERVERWARE_COIN );

		if (nChargeItemID > 0)
		{
			SAFE_DELETE(m_pChargeItem);
			TItemInfo itemInfo;
			if (CDnItem::MakeItemInfo(nChargeItemID, 1, itemInfo) == false)
				return;
			m_pChargeItem = GetItemTask().CreateItem(itemInfo);
			if (m_pChargeItem)
			{
				m_pChargeItem->SetInfoItem(true);
				m_pChargeItemSlot->SetItem(m_pChargeItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
			}
		}

		std::wstring str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1628), nChargeItemAllCount);
		m_pCountStatic->SetText(str.c_str());
	}
}

void CDnWorldServerStorageInventoryDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (bShow)
	{
		Storage::WorldServerStorage::eChargeType type = Storage::WorldServerStorage::eFree;
		int storageType = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_PAYMENT);
		switch (storageType)
		{
		case 0: type = Storage::WorldServerStorage::eFree; break;
		case 1: type = Storage::WorldServerStorage::eFeeCharge; break;
		default: 
			{
				_ASSERT(0);
			}
			break;
		}

		SetChargeType(type);

		m_pNoticeOnNoChargeStatic->Show(type == Storage::WorldServerStorage::eFree);
		m_pNoticeStatic->Show(type == Storage::WorldServerStorage::eFeeCharge);
		m_pCountStatic->Show(type == Storage::WorldServerStorage::eFeeCharge);
		m_pChargeItemSlot->Show(type == Storage::WorldServerStorage::eFeeCharge);
		m_pChargeItemSlotStatic->Show(type == Storage::WorldServerStorage::eFeeCharge);

		int nChargeItemID = 0;
		int nChargeItemAllCount = 0;
		UpdateCoin();
	}

	CEtUIDialog::Show(bShow);
}

void CDnWorldServerStorageInventoryDlg::SortInventory()
{
	GetItemTask().GetWorldServerStorageInventory().SortCashInventory();
}

void CDnWorldServerStorageInventoryDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BT_SORT"))
		{
			SortInventory();
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnWorldServerStorageInventoryDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process(fElapsedTime);
}

void CDnWorldServerStorageInventoryDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		switch (nID)
		{
		case FREE_CONFIRM_DLG: OnConfirmUIMsg(nID, m_pConfirmDlg); break;
		case FREE_CONFIRM_EX_DLG: OnConfirmUIMsg(nID, m_pSplitConfirmDlg); break;
		case CHARGE_CONFIRM_DLG: OnConfirmUIMsg(nID, m_pChargeConfirmDlg); break;
		case CHARGE_CONFIRM_EX_DLG: OnConfirmUIMsg(nID, m_pChargeSplitConfirmDlg); break;
		case DRAW_CONFIRM_DLG: OnConfirmUIMsg(nID, m_pDrawConfirmDlg); break;
		case DRAW_CONFIRM_EX_DLG: OnConfirmUIMsg(nID, m_pDrawSplitConfirmDlg); break;
		default:
			{
				_ASSERT(0);
				return;
			}
		}
	}
}

template <typename T>
void CDnWorldServerStorageInventoryDlg::OnConfirmUIMsg(int nID, T* pConfirmDlg, bool bForceOk)
{
	if (pConfirmDlg == NULL)
	{
		_ASSERT("WSS");
		return;

	}
	eItemMoveType moveType = MoveType_InvenToServerWare;
	if (pConfirmDlg->IsFromCashInventory() || pConfirmDlg->IsFromCashStorage())
	{
		if (nID == FREE_CONFIRM_EX_DLG || nID == FREE_CONFIRM_DLG ||
			nID == CHARGE_CONFIRM_DLG || nID == CHARGE_CONFIRM_EX_DLG)
		{
			moveType = MoveType_CashToServerWare;
		}
		else if (nID == DRAW_CONFIRM_DLG || nID == DRAW_CONFIRM_EX_DLG)
		{
			moveType = MoveType_ServerWareToCash;
		}
		else
		{
			_ASSERT("WSS");
			return;
		}
	}
	else
	{
		if (nID == FREE_CONFIRM_EX_DLG || nID == FREE_CONFIRM_DLG ||
			nID == CHARGE_CONFIRM_DLG || nID == CHARGE_CONFIRM_EX_DLG)
		{
			moveType = MoveType_InvenToServerWare;
		}
		else if (nID == DRAW_CONFIRM_DLG || nID == DRAW_CONFIRM_EX_DLG)
		{
			moveType = MoveType_ServerWareToInven;
		}
		else
		{
			_ASSERT("WSS");
			return;
		}
	}

	if (IsCmdControl("ID_OK") || bForceOk == true)
	{
		if (pConfirmDlg == NULL)
			return;

		CDnItem* pItem = pConfirmDlg->GetItem();
		if (pItem)
		{
			int nItemCount = pConfirmDlg->GetRealItemCount();
			if (nItemCount <= 0)
			{
				pConfirmDlg->Show(false);
				return;
			}
			if( moveType == MoveType_InvenToServerWare || moveType == MoveType_CashToServerWare )
			{
				if( m_ChargeType == Storage::WorldServerStorage::eFeeCharge )
				{
					int nReqCount = 1;
					if (pConfirmDlg->GetItem()->GetItemType() == ITEMTYPE_SERVERWARE_COIN)
					{
						nReqCount += pConfirmDlg->GetSlotItemCount();
					}

					if (HasCoinItem(nReqCount) == false)
					{
						pConfirmDlg->Show(false);
						GetInterface().MessageBox(1617); // UISTRING : A Server Storage Pass is required to use Server Storage.
						return;
					}
				}
				else if( m_ChargeType == Storage::WorldServerStorage::eFree )
				{
					if (HasEnoughMoney() == false)
					{
						GetInterface().MessageBox(1615); // UISTRING : Not enough fee to use Server Storage.
						return;
					}
				}
			}			
			RequestMoveItem(moveType, pConfirmDlg->GetItem(), nItemCount);
		}

		pConfirmDlg->Show(false);
	}
	else if (IsCmdControl("ID_CANCEL"))
	{
		pConfirmDlg->Show(false);
	}
}

void CDnWorldServerStorageInventoryDlg::SetItem(MIInventoryItem* pItem)
{
	ASSERT(pItem&&"CDnWorldServerStorageInventoryDlg::SetItem, pItem is NULL!");

	if (m_pWorldServerStorageDlg)
		m_pWorldServerStorageDlg->SetItem(pItem);
}

void CDnWorldServerStorageInventoryDlg::ResetSlot(MIInventoryItem* pItem)
{
	ASSERT(pItem&&"CDnWorldServerStorageInventoryDlg::SetItem, pItem is NULL!");
	if (m_pWorldServerStorageDlg)
		m_pWorldServerStorageDlg->ResetSlot( pItem->GetSlotIndex() );
}

void CDnWorldServerStorageInventoryDlg::SetUseItemCnt(DWORD dwItemCnt)
{
	ASSERT(!(dwItemCnt%ITEM_X)&&"CDnGuildStorageMainDlg::SetUseItemCnt");
	if( dwItemCnt % ITEM_X )
		return;

	if (m_pWorldServerStorageDlg)
		m_pWorldServerStorageDlg->SetUseItemCnt(dwItemCnt);
}

bool CDnWorldServerStorageInventoryDlg::IsEmptySlot() const
{
	return true;
}

int CDnWorldServerStorageInventoryDlg::GetEmptySlot() const
{
	if (!m_pWorldServerStorageDlg)
		return -1;
	return m_pWorldServerStorageDlg->GetEmptySlot();
}

bool CDnWorldServerStorageInventoryDlg::HasEnoughMoney() const
{
	if (m_ChargeType != Storage::WorldServerStorage::eFree)
		return true;

	TAX_TYPE nMoney = CommonUtil::GetFixedTax(TAX_WSTORAGE);
	if (CDnItemTask::IsActive())
		return (CDnItemTask::GetInstance().GetCoin() >= nMoney);

	return false;
}

bool CDnWorldServerStorageInventoryDlg::HasCoinItem(int nReqCount) const
{
	if (m_ChargeType != Storage::WorldServerStorage::eFeeCharge)
		return true;

	int nChargeItemAllCount = GetItemTask().FindItemCountFromItemType( ITEMTYPE_SERVERWARE_COIN );
	if( nChargeItemAllCount >= nReqCount )
		return true;
	return false;
}

void CDnWorldServerStorageInventoryDlg::MoveItemToStorage(CDnQuickSlotButton* pFromSlot, bool bForceMove)
{
	if (pFromSlot == NULL)
	{
		_ASSERT(0);
		return;
	}

	MIInventoryItem* pInvenItem = pFromSlot->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	int limitLevel = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_IN_LEVELLIMIT);
	if (CDnActor::s_hLocalActor)
	{
		if (CDnActor::s_hLocalActor->GetLevel() < limitLevel)
		{
			std::wstring str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1624), limitLevel);
			GetInterface().MessageBox(str.c_str()); // UISTRING : Must be Level %d or higher to deposit items into Server Storage.
			return;
		}
	}

	bool bNoEmptySlot = false;
	if (pFromSlot->GetSlotType() == ST_INVENTORY_CASH)
	{
		CDnWorldServerStorageInventory& cashWorldServerStorage = CDnItemTask::GetInstance().GetWorldServerStorageCashInventory();
		if (cashWorldServerStorage.FindFirstEmptyIndex() >= cashWorldServerStorage.GetUsableSlotCount())
			bNoEmptySlot = true;
	}
	else
	{
		CDnWorldServerStorageInventory& worldServerStorage = CDnItemTask::GetInstance().GetWorldServerStorageInventory();
		if (worldServerStorage.FindFirstEmptyIndex() >= worldServerStorage.GetUsableSlotCount())
			bNoEmptySlot = true;
	}

	if (bNoEmptySlot)
	{
		GetInterface().MessageBox(1614); // UISTRING : Not enough empty slots in Server Storage.
		return;
	}

	CDnItem* pItem = static_cast<CDnItem *>(pInvenItem);
	if (pItem == NULL)
		return;

	if (pItem->IsEternityItem() == false)
	{
		GetInterface().MessageBox(4082); // UISTRING : Timed items cannot be traded.
		return;
	}

	bool bRenderCountMinusOne = false;
	if (m_ChargeType == Storage::WorldServerStorage::eFeeCharge)
	{
		if (pItem->GetItemType() == ITEMTYPE_SERVERWARE_COIN)
		{
			std::vector<CDnItem*> coinItems;
			GetItemTask().FindItemFromItemType(ITEMTYPE_SERVERWARE_COIN, ITEM_SLOT_TYPE::ST_INVENTORY_CASH, coinItems);
			if (coinItems.empty() == false)
			{
				if (coinItems.size() == 1)
				{
					if (pItem->GetOverlapCount() <= 1)
					{
						GetInterface().MessageBox(1617); // UISTRING : A Server Storage Pass is required to use Server Storage.
						return;
					}

					bRenderCountMinusOne = true;
				}
			}
		}
	}

	if (m_ChargeType == Storage::WorldServerStorage::eFree)
	{
		if (bForceMove == false)
		{
			if (pFromSlot->GetSlotType() != ST_INVENTORY_CASH && pFromSlot->GetRenderCount() > 1)
			{
				m_pSplitConfirmDlg->SetItemWithSlotButton(pFromSlot, false);
				m_pSplitConfirmDlg->Show(true);
			}
			else
			{
				m_pConfirmDlg->SetItemWithSlotButton(pFromSlot, false);
				m_pConfirmDlg->Show(true);
			}

			return;
		}
		else
		{
			if (HasEnoughMoney() == false)
			{
				GetInterface().MessageBox(1615); // UISTRING : Not enough fee to use Server Storage.
				return;
			}

			ITEM_SLOT_TYPE slotType = pFromSlot->GetSlotType();

			eItemMoveType moveType = MoveType_InvenToServerWare;
			if (slotType == ST_INVENTORY)
			{
				moveType = MoveType_InvenToServerWare;
			}
			else if (slotType == ST_INVENTORY_CASH)
			{
				moveType = MoveType_CashToServerWare;
			}
			else
			{
				_ASSERT(0);
				return;
			}

			// todo by kalliste : calculation sound?
			RequestMoveItem(moveType, pItem, pItem->GetOverlapCount());
		}
	}
	else if (m_ChargeType == Storage::WorldServerStorage::eFeeCharge)
	{
		if (pFromSlot->GetSlotType() != ST_INVENTORY_CASH && pFromSlot->GetRenderCount() > 1)
		{
			m_pChargeSplitConfirmDlg->SetItemWithSlotButton(pFromSlot, bRenderCountMinusOne);
			m_pChargeSplitConfirmDlg->Show(true);
		}
		else
		{
			m_pChargeConfirmDlg->SetItemWithSlotButton(pFromSlot, bRenderCountMinusOne);
			m_pChargeConfirmDlg->Show(true);
		}

		return;
	}
}

void CDnWorldServerStorageInventoryDlg::MoveItemToInventory(CDnItemSlotButton* pFromSlot, UINT uMsg)
{
	if (pFromSlot == NULL)
	{
		_ASSERT(0);
		return;
	}

	MIInventoryItem* pInvenItem = pFromSlot->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	int limitLevel = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_OUT_LEVELLIMIT);
	if (CDnActor::s_hLocalActor)
	{
		if (CDnActor::s_hLocalActor->GetLevel() < limitLevel)
		{
			std::wstring str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1625), limitLevel);
			GetInterface().MessageBox(str.c_str()); // UISTRING : Must be Level %d or higher to withdraw items from Server Storage.
			return;
		}
	}

	bool bNoEmptySlot = false;
	if (pFromSlot->GetSlotType() != ST_STORAGE_WORLDSERVER_CASH)
	{
		CDnInventory& inven = CDnItemTask::GetInstance().GetCharInventory();
		if (inven.FindFirstEmptyIndex() >= inven.GetUsableSlotCount())
			bNoEmptySlot = true;
	}

	if (bNoEmptySlot)
	{
		GetInterface().MessageBox(1602); // UISTRING : No empty slots in inventory.
		return;
	}

	CDnItem* pItem = static_cast<CDnItem *>(pInvenItem);
	if (pItem == NULL)
		return;

	if (pFromSlot->GetSlotType() != ST_STORAGE_WORLDSERVER_CASH && pFromSlot->GetRenderCount() > 1)
	{
		m_pDrawSplitConfirmDlg->SetItemWithSlotButton(pFromSlot, false);
		if (uMsg == (WM_RBUTTONUP | VK_SHIFT))
		{
			m_pDrawSplitConfirmDlg->SetItemCountForce(pFromSlot->GetRenderCount());
			OnConfirmUIMsg(DRAW_CONFIRM_DLG, m_pDrawSplitConfirmDlg, true);
		}
		else
		{
			m_pDrawSplitConfirmDlg->Show(true);
		}
	}
	else
	{
		m_pDrawConfirmDlg->SetItemWithSlotButton(pFromSlot, false);
		bool bIsDrawPopUp = false;
		if (int(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_POPUP)) == 1)
		{
			if (uMsg != (WM_RBUTTONUP | VK_SHIFT))
				bIsDrawPopUp = true;
		}

		if (bIsDrawPopUp == false)
		{
			OnConfirmUIMsg(DRAW_CONFIRM_DLG, m_pDrawConfirmDlg, true);
		}
		else
		{
			m_pDrawConfirmDlg->Show(true);
		}
	}

	return;
}

void CDnWorldServerStorageInventoryDlg::RequestMoveItem(eItemMoveType moveType, CDnItem* pItem, int itemCount)
{
	DWORD emptySlotType = -1;
	int nInsertableSlotIndex = -1;

	if (moveType == MoveType_InvenToServerWare || moveType == MoveType_CashToServerWare)
	{
		emptySlotType = CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG;
	}
	else if (moveType == MoveType_ServerWareToInven || moveType == MoveType_ServerWareToCash)
	{
		emptySlotType = CDnMainMenuDlg::INVENTORY_DIALOG;

		if (moveType == MoveType_ServerWareToInven)
		{
			std::vector<CDnItem*> vecItem;
			GetItemTask().GetCharInventory().ScanItemFromID(pItem->GetClassID(), &vecItem);
			for (int i = 0; i < (int)vecItem.size(); ++i)
			{
				if ((vecItem[i]->IsSoulbBound() == pItem->IsSoulbBound() && vecItem[i]->GetSealCount() == pItem->GetSealCount())
					&& (vecItem[i]->GetOverlapCount() + itemCount <= pItem->GetMaxOverlapCount()))
				{
					nInsertableSlotIndex = vecItem[i]->GetSlotIndex();
					break;
				}
			}

			if( nInsertableSlotIndex == -1 )
			{
				if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) )
				{
					GetInterface().MessageBox( 1925, MB_OK );
					return;
				}

				nInsertableSlotIndex = GetInterface().GetEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG );
			}
		}
	}
	else
	{
		_ASSERT("WSS");
		return;
	}

	if (nInsertableSlotIndex == -1)
		nInsertableSlotIndex = GetInterface().GetEmptySlot(emptySlotType);

	CDnItemTask::GetInstance().RequestMoveItem(moveType,
		pItem->GetSlotIndex(),
		pItem->GetSerialID(),
		nInsertableSlotIndex,
		itemCount);
}

//////////////////////////////////////////////////////////////////////////

CDnWorldServerStorageCashInventoryDlg::CDnWorldServerStorageCashInventoryDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
:CDnWorldServerStorageInventoryDlg(dialogType, pParentDialog, nID, pCallback)
{
}

void CDnWorldServerStorageCashInventoryDlg::OnInitialUpdate()
{
	m_InvenType = Storage::WorldServerStorage::eCash;
	m_pWorldServerStorageDlg = new CDnWorldServerStorageDlg(UI_TYPE_CHILD, this);
	m_pWorldServerStorageDlg->Initialize( true );
	m_pWorldServerStorageDlg->SetSlotType(ST_STORAGE_WORLDSERVER_CASH);

	GetItemTask().GetWorldServerStorageCashInventory().SetInventoryDialog(this);
}

void CDnWorldServerStorageCashInventoryDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	CDnWorldServerStorageInventoryDlg::Show(bShow);
}

void CDnWorldServerStorageCashInventoryDlg::SortInventory()
{
	GetItemTask().GetWorldServerStorageCashInventory().SortCashInventory();
}

#endif // PRE_ADD_ACCOUNT_STORAGE