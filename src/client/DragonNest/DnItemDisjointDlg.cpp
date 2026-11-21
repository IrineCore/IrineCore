#include "StdAfx.h"

#include "DnItemDisjointDlg.h"
#include "DnItemDisjointExpectItemInfoDlg.h"
#include "DnItemDisjointResultDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnLocalPlayerActor.h"
#include "DnNPCActor.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif
#include "EtUIListBoxEx.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnItemDisjointDlg::CDnItemDisjointDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
	, m_pListBoxEx(NULL)
	, m_pDisjointResultDlg(NULL)
	, m_pStaticGold(NULL)
	, m_pStaticSilver(NULL)
	, m_pStaticBronze(NULL)
	, m_pButtonDisjoint(NULL)
	, m_pButtonCancel(NULL)
	, m_dwColorGold(0)
	, m_dwColorSilver(0)
	, m_dwColorBronze(0)
{
}

CDnItemDisjointDlg::~CDnItemDisjointDlg(void)
{
	SAFE_DELETE_PVEC(m_vecItem);
	SAFE_DELETE(m_pDisjointResultDlg);
}

void CDnItemDisjointDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("ItemDisjointDlg.ui").c_str(), bShow);

	m_pDisjointResultDlg = new CDnItemDisjointResultDlg(UI_TYPE_MODAL);
	if (m_pDisjointResultDlg)
		m_pDisjointResultDlg->Initialize(false);
}

void CDnItemDisjointDlg::InitialUpdate()
{
	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_dwColorGold = m_pStaticGold->GetTextColor();
	m_dwColorSilver = m_pStaticSilver->GetTextColor();
	m_dwColorBronze = m_pStaticBronze->GetTextColor();

	m_pButtonDisjoint = GetControl<CEtUIButton>("ID_BUTTON_DISJOINT");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");


	char szTemp[32] = { 0 };
	for (int i = 0; i<ITEM_DISJOINT_ITEMSLOT_MAX; i++)
	{
		sprintf_s(szTemp, 32, "ID_ITEM%d", i);
		CDnItemSlotButton *pItemSlotButton = GetControl<CDnItemSlotButton>(szTemp);
		pItemSlotButton->SetSlotIndex(i);
		pItemSlotButton->SetSlotType(ST_ITEM_DISJOINT);
		m_vecSlotButton.push_back(pItemSlotButton);

		sprintf_s(szTemp, 32, "ID_RBT_%d", i);
		CEtUIRadioButton *pRadioButton = GetControl<CEtUIRadioButton>(szTemp);
		if (pRadioButton)
			m_mapRadioButton[i] = pRadioButton;
	}

	m_vecItem.resize(ITEM_DISJOINT_ITEMSLOT_MAX);
	m_vecQuickSlotButton.resize(ITEM_DISJOINT_ITEMSLOT_MAX);

	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");
}

void CDnItemDisjointDlg::Clear(bool bAll)
{
	if (m_pListBoxEx)
		m_pListBoxEx->RemoveAllItems();
	if (bAll)
	{
		//m_pItemCacheList.clear();
	}
}

void CDnItemDisjointDlg::SelectSlot(int nSlotIndex)
{
	std::map<int, CEtUIRadioButton*>::iterator it = m_mapRadioButton.find(nSlotIndex);
	if (it != m_mapRadioButton.end() && it->second)
	{
		it->second->SetChecked(true, true, false);
	}
	else
	{
		RefreshExpectItemList(nSlotIndex);
	}
}

void CDnItemDisjointDlg::RefreshExpectItemList(int nSlotIndex)
{
	if (!m_pListBoxEx)
		return;

	m_pListBoxEx->RemoveAllItems();

	if (nSlotIndex < 0 || nSlotIndex >= (int)m_vecItem.size())
		return;

	CDnItem *pItem = m_vecItem[nSlotIndex];
	if (!pItem)
		return;

	DNTableFileFormat *pItemSox = GetDNTable(CDnTableDB::TITEM);
	DNTableFileFormat *pDisjointInfoSox = GetDNTable(CDnTableDB::TDISJOINTORINFO);
	if (!pItemSox || !pDisjointInfoSox)
		return;

	DNTableCell *pDropCell = pItemSox->GetFieldFromLablePtr(pItem->GetClassID(), "_DisjointDrop1");
	if (!pDropCell)
		return;

	int nDisjointDrop = pDropCell->GetInteger();
	int nEnchantLevel = (int)pItem->GetEnchantLevel();

	std::vector<int> vecExpectItemIDs;
	int nRowCount = pDisjointInfoSox->GetItemCount();
	for (int i = 0; i < nRowCount; ++i)
	{
		int nRowID = pDisjointInfoSox->GetItemID(i);
		DNTableCell *pCellDrop = pDisjointInfoSox->GetFieldFromLablePtr(nRowID, "_DisjointDrop");
		DNTableCell *pCellEnchant = pDisjointInfoSox->GetFieldFromLablePtr(nRowID, "_EnchantLevel");
		if (!pCellDrop || !pCellEnchant)
			continue;

		if (pCellDrop->GetInteger() != nDisjointDrop || pCellEnchant->GetInteger() != nEnchantLevel)
			continue;

		char szField[32] = { 0 };
		for (int k = 1; k <= 20; ++k)
		{
			sprintf_s(szField, sizeof(szField), "_Item%dIndex", k);
			DNTableCell *pItemCell = pDisjointInfoSox->GetFieldFromLablePtr(nRowID, szField);
			if (pItemCell && pItemCell->GetInteger() > 0)
				vecExpectItemIDs.push_back(pItemCell->GetInteger());
		}
		break;
	}

	if (vecExpectItemIDs.empty())
		return;

	int nRows = ((int)vecExpectItemIDs.size() - 1) / CDnItemDisjointExpectItemInfoDlg::EXPECT_SLOT_PER_ROW + 1;
	for (int r = 0; r < nRows; ++r)
	{
		CDnItemDisjointExpectItemInfoDlg *pRowDlg = m_pListBoxEx->AddItem<CDnItemDisjointExpectItemInfoDlg>();
		if (pRowDlg)
			pRowDlg->SetExpectItems(vecExpectItemIDs, r * CDnItemDisjointExpectItemInfoDlg::EXPECT_SLOT_PER_ROW);
	}
}

void CDnItemDisjointDlg::ShowDisjointResult(const std::vector<TItem> &vecItems)
{
	if (!m_pDisjointResultDlg || vecItems.empty())
		return;

	m_pDisjointResultDlg->SetResultItems(vecItems);
	m_pDisjointResultDlg->Show(true);
}

void CDnItemDisjointDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (bShow)
	{
		m_pStaticGold->SetIntToText(0);
		m_pStaticSilver->SetIntToText(0);
		m_pStaticBronze->SetIntToText(0);
		m_pStaticGold->SetTextColor(m_dwColorGold);
		m_pStaticSilver->SetTextColor(m_dwColorSilver);
		m_pStaticBronze->SetTextColor(m_dwColorBronze);

		m_pButtonDisjoint->Enable(false);
		m_pButtonCancel->Enable(true);

		for (int i = 0; i<ITEM_DISJOINT_ITEMSLOT_MAX; i++)
		{
			SAFE_DELETE(m_vecItem[i]);
			m_vecSlotButton[i]->ResetSlot();
		}

		// Play Activate action when disassemble window is shown
		DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
		DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
		if (hNpc)
		{
			hNpc->SetActionQueue("Activate");
		}


		this->Clear(true);
		this->Clear(false);
	}
	else
	{
		// Restore quickslot color changes when the window closes.
		for (int i = 0; i<ITEM_DISJOINT_ITEMSLOT_MAX; i++)
		{
			if (m_vecQuickSlotButton[i])
			{
				m_vecQuickSlotButton[i]->SetRegist(false);
				m_vecQuickSlotButton[i]->DisableSplitMode(true);
				m_vecQuickSlotButton[i] = NULL;
			}
		}

		// Play Disjoint_01 action when disassemble window closes
		DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
		DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
		if (hNpc)
		{
			hNpc->SetActionQueue("Disjoint_01");
		}


		GetInterface().CloseBlind();
	}

	CEtUIDialog::Show(bShow);
}

bool CDnItemDisjointDlg::IsDisjoitableItem(CDnSlotButton* pDragButton, CDnItem* pItem) const
{
	if (pDragButton->GetSlotType() != ST_INVENTORY)
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3610), MB_OK);
		return false;
	}

	// Skip if re-registering an already listed item.
	if (CheckItemDuplication(pItem))
		return false;

	// Check item type
	if (pItem->GetItemType() == ITEMTYPE_QUEST)
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3609), MB_OK);
		return false;
	}

	// Check if item cannot be disassembled.
	if (!pItem->CanDisjoint())
	{
		GetInterface().MessageBox(1740, MB_OK);
		return false;
	}

	int nDisjointLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Disjoint_ItemLimitLevel);
	int nDisjointUserLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Disjoint_LimitUserLevel);

	if (pItem->GetLevelLimit() >= nDisjointLevelLimit && nDisjointUserLevelLimit > CDnActor::s_hLocalActor->GetLevel())
	{
		GetInterface().MessageBox(FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8145), pItem->GetLevelLimit(), pItem->GetLevelLimit()).c_str(), MB_OK);
		return false;
	}

	// Check if it is a bound item in a sealed state
	if (pItem->GetReversion() > CDnItem::NoReversion && !pItem->IsSoulbBound())
	{
		GetInterface().MessageBox(1750, MB_OK);
		return false;
	}

	// Check disassemble type
	DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
	DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
	if (hNpc) {
		// After varying NPC presentation based on the count
		CDnNPCActor *pNPC = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());

		if (!pItem->CanDisjointType(pNPC->GetNpcData().nParam[0])) {
			GetInterface().MessageBox(8100, MB_OK);
			return false;
		}
	}

	return true;
}

void CDnItemDisjointDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON_DISJOINT"))
		{
			// Count the number of registered items
			//
			int nCount = 0;
			CDnItemSlotButton *pButton(NULL);
			DWORD dwVecSize = (DWORD)m_vecSlotButton.size();
			for (DWORD i = 0; i<dwVecSize; i++)
			{
				pButton = m_vecSlotButton[i];
				if (!pButton) continue;
				if (!pButton->IsEmptySlot())
					++nCount;
			}

			// After getting NPC
			DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
			DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
			if (hNpc)
			{
				// After varying NPC presentation based on the count
				CDnNPCActor *pNPC = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());
				char szActionName[16];
				sprintf_s(szActionName, sizeof(szActionName), "disjoint_0%d", nCount);
				pNPC->SetActionQueue(szActionName);

				// Calculate and store the playtime.
				//float fFPS = hNpc->CDnActionBase::GetFPS();
				//CEtActionBase::ActionElementStruct* pActionInfo = hNpc->GetElement( szActionName );
				//float fActionTimeLength = (float)pActionInfo->dwLength / fFPS;
			}

			// Close inventory.
			ProcessCommand(EVENT_BUTTON_CLICKED, false, GetControl("ID_CLOSE_DIALOG"), 0);
			// Request disassembly of the selected items
			for (DWORD i = 0; i<dwVecSize; i++)
			{
				pButton = m_vecSlotButton[i];
				if (!pButton) continue;

				if (!pButton->IsEmptySlot())
				{
					CDnItem *pItem = static_cast<CDnItem *>(pButton->GetItem());
					GetItemTask().RequestItemDisjoint(pItem, nUID);
				}
			}

			// The procedure above is handled temporarily because the design has not been finalized yet.
			// To do this properly, the server should complete item deletion and fee processing upon request, and then
			// Need to drop the item after NPC animation finishes,
			// Doing this would require additional exception handling in many areas such as party member NPC synchronization, so
			// Do not overcomplicate it,
			// Decided to show effect only to self and output items immediately when clicking disassemble.
			//
			// If it changes later, we will have to tear it all apart.

			return;
		}
		else if (IsCmdControl("ID_BUTTON_CANCEL"))
		{
			ProcessCommand(EVENT_BUTTON_CLICKED, false, GetControl("ID_CLOSE_DIALOG"), 0);
			return;
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if (IsCmdControl("ID_BT_SMALLHELP"))
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if (pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_ITEM_DISJOINT);
		}
#endif

		if (strstr(pControl->GetControlName(), "ID_ITEM"))
		{
			CDnSlotButton *pDragButton = (CDnSlotButton *)drag::GetControl();
			CDnSlotButton *pPressedButton = (CDnItemSlotButton *)pControl;

			// Remove via right-click
			if (uMsg == WM_RBUTTONUP)
			{
				SAFE_DELETE(m_vecItem[pPressedButton->GetSlotIndex()]);
				pPressedButton->ResetSlot();
				if (m_vecQuickSlotButton[pPressedButton->GetSlotIndex()])
				{
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist(false);
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->DisableSplitMode(true);
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = NULL;
				}
				CheckItemList();
				if (m_pListBoxEx)
					m_pListBoxEx->RemoveAllItems();
				return;
			}

			if (pDragButton)
			{
				drag::ReleaseControl();

				CDnItem *pItem = static_cast<CDnItem *>(pDragButton->GetItem());
				if (pItem == NULL || IsDisjoitableItem(pDragButton, pItem) == false)
				{
					pDragButton->DisableSplitMode(true);
					return;
				}

				CDnItem *pPressedItem = static_cast<CDnItem *>(pPressedButton->GetItem());

				// If placing another item on a slot that already has one,
				if (pPressedItem)
				{
					// Remove the item from the window.
					SAFE_DELETE(m_vecItem[pPressedButton->GetSlotIndex()]);
					pPressedButton->ResetSlot();
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist(false);
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->DisableSplitMode(true);
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = NULL;
				}

				// When registering items to disassemble, instead of passing inventory item pointers directly like before,
				//pPressedButton->SetItem( pItem );
				// Create and insert a temporary item like this.
				TItemInfo itemInfo;
				pItem->GetTItemInfo(itemInfo);
				CDnItem *pNewItem = GetItemTask().CreateItem(itemInfo);
				m_vecItem[pPressedButton->GetSlotIndex()] = pNewItem;

				pPressedButton->SetItem(pNewItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

				CheckItemList();
				CEtSoundEngine::GetInstance().PlaySound("2D", pItem->GetDragSoundIndex());

				m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = (CDnQuickSlotButton *)pDragButton;
				m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist(true);
				SelectSlot(pPressedButton->GetSlotIndex());
			}
			else
			{
				SelectSlot(pPressedButton->GetSlotIndex());
			}

			return;
		}
	}
	else if (nCommand == EVENT_RADIOBUTTON_CHANGED)
	{
		if (strstr(pControl->GetControlName(), "ID_RBT_"))
		{
			int nSlotIndex = -1;
			if (sscanf_s(pControl->GetControlName(), "ID_RBT_%d", &nSlotIndex) == 1)
			{
				RefreshExpectItemList(nSlotIndex);
			}
		}
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

bool CDnItemDisjointDlg::CheckItemDuplication(CDnItem *pItem) const
{
	for (int i = 0; i<ITEM_DISJOINT_ITEMSLOT_MAX; i++)
	{
		if (m_vecQuickSlotButton[i] && m_vecQuickSlotButton[i]->GetItem() == pItem)
			return true;
	}
	return false;
}

void CDnItemDisjointDlg::AddItemToList(CDnQuickSlotButton *pPressedButton)
{
	CDnItem *pItem = static_cast<CDnItem *>(pPressedButton->GetItem());

	// Check if there is space in the trade window
	if (!IsEmptySlot())
	{
		GetInterface().MessageBox(3621, MB_OK);
		return;
	}

	// Skip if re-registering an already listed item.
	if (CheckItemDuplication(pItem))
		return;

	// Check item type
	if (pItem->GetItemType() == ITEMTYPE_QUEST)
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3609), MB_OK);
		return;
	}

	// Check if item cannot be disassembled.
	if (!pItem->CanDisjoint())
	{
		GetInterface().MessageBox(1740, MB_OK);
		return;
	}

	int nDisjointLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Disjoint_ItemLimitLevel);
	int nDisjointUserLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::Disjoint_LimitUserLevel);

	if (pItem->GetLevelLimit() >= nDisjointLevelLimit && nDisjointUserLevelLimit > CDnActor::s_hLocalActor->GetLevel())
	{
		GetInterface().MessageBox(FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8145), pItem->GetLevelLimit(), pItem->GetLevelLimit()).c_str(), MB_OK);
		return;
	}

	// Check if it is a bound item in a sealed state
	if (pItem->GetReversion() > CDnItem::NoReversion && !pItem->IsSoulbBound())
	{
		GetInterface().MessageBox(1750, MB_OK);
		return;
	}

	// Check disassemble type
	DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
	DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);
	if (hNpc) {
		// After varying NPC presentation based on the count
		CDnNPCActor *pNPC = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());

		if (!pItem->CanDisjointType(pNPC->GetNpcData().nParam[0])) {
			GetInterface().MessageBox(8100, MB_OK);
			return;
		}
	}

	// Get an empty slot
	CDnSlotButton *pEmptyButton = m_vecSlotButton[GetEmptySlot()];

	// When registering items to disassemble, instead of passing inventory item pointers directly like before,
	//pEmptyButton->SetItem( pItem );
	// Create and insert a temporary item like this.
	TItemInfo itemInfo;
	pItem->GetTItemInfo(itemInfo);
	CDnItem *pNewItem = GetItemTask().CreateItem(itemInfo);
	m_vecItem[GetEmptySlot()] = pNewItem;

	pEmptyButton->SetItem(pNewItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

	CheckItemList();
	CEtSoundEngine::GetInstance().PlaySound("2D", pItem->GetDragSoundIndex());

	m_vecQuickSlotButton[pEmptyButton->GetSlotIndex()] = pPressedButton;
	m_vecQuickSlotButton[pEmptyButton->GetSlotIndex()]->SetRegist(true);
	SelectSlot(pEmptyButton->GetSlotIndex());
}

bool CDnItemDisjointDlg::IsEmptySlot()
{
	if (GetEmptySlot() == -1)
	{
		return false;
	}

	return true;
}

int CDnItemDisjointDlg::GetEmptySlot()
{
	CDnItemSlotButton *pButton(NULL);

	DWORD dwVecSize = (DWORD)m_vecSlotButton.size();
	for (DWORD i = 0; i<dwVecSize; i++)
	{
		pButton = m_vecSlotButton[i];
		if (!pButton) continue;

		if (pButton->IsEmptySlot())
			return pButton->GetSlotIndex();
	}

	return -1;
}

void CDnItemDisjointDlg::CheckItemList() const
{
	int nPrice = 0;
	bool bListEmpty = true;

	// First check if nothing is registered in the slot
	CDnItemSlotButton *pButton(NULL);
	DWORD dwVecSize = (DWORD)m_vecSlotButton.size();
	for (DWORD i = 0; i<dwVecSize; i++)
	{
		pButton = m_vecSlotButton[i];
		if (!pButton) continue;

		if (!pButton->IsEmptySlot())
		{
			CDnItem *pItem = static_cast<CDnItem *>(pButton->GetItem());
			nPrice += pItem->GetDisjointCost() * pItem->GetOverlapCount();
			bListEmpty = false;
		}
	}

	if (bListEmpty == true)
	{
		// Do not judge by the summed price.
		m_pButtonDisjoint->Enable(false);
	}
	else
	{
		// Also disable if price is insufficient.
		if (nPrice > GetItemTask().GetCoin())
			m_pButtonDisjoint->Enable(false);
		else
			m_pButtonDisjoint->Enable(true);
	}

	int nGold = nPrice / 10000;
	int nSilver = (nPrice % 10000) / 100;
	int nBronze = nPrice % 100;

	m_pStaticGold->SetIntToText(nGold);
	m_pStaticSilver->SetIntToText(nSilver);
	m_pStaticBronze->SetIntToText(nBronze);
	if (nPrice > GetItemTask().GetCoin())
	{
		if (nPrice >= 10000) m_pStaticGold->SetTextColor(0xFFFF0000, true);
		if (nPrice >= 100) m_pStaticSilver->SetTextColor(0xFFFF0000, true);
		m_pStaticBronze->SetTextColor(0xFFFF0000, true);
	}
	else
	{
		m_pStaticGold->SetTextColor(m_dwColorGold);
		m_pStaticSilver->SetTextColor(m_dwColorSilver);
		m_pStaticBronze->SetTextColor(m_dwColorBronze);
	}
}



void CDnItemDisjointDlg::RemoveItemFromCache(int nItemID, int nItemLevel)
{

}