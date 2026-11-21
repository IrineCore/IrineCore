#include "StdAfx.h"
#include "DnWorldServerStorageConfirmDlg.h"
#include "DnItem.h"
#include "DnInterfaceString.h"
#include "DnItemTask.h"
#include "DnCommonTask.h"
#include "DnMoneyInputDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#define MAX_WSTORAGE_FEE_CHAR_COUNT 16

CDnWorldServerStorageConfirmDlg::CDnWorldServerStorageConfirmDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
	: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback)
	, m_pItemSlotButton(NULL)
	, m_pItemName(NULL)
	, m_pOKButton(NULL)
	, m_pCancelButton(NULL)
	, m_pGold(NULL)
	, m_pSilver(NULL)
	, m_pCopper(NULL)
{
	m_dwColorGold = m_dwColorSilver = m_dwColorCopper = textcolor::WHITE;
	m_bRenderCountMinusOne = false;
}

CDnWorldServerStorageConfirmDlg::~CDnWorldServerStorageConfirmDlg(void)
{
}

void CDnWorldServerStorageConfirmDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName( "StorageMessageBoxDlg.ui" ).c_str(), bShow);
}

void CDnWorldServerStorageConfirmDlg::InitialUpdate()
{
	m_pItemName = GetControl<CEtUIStatic>("ID_TEXT_NAME"); 

	m_pOKButton = GetControl<CEtUIButton>("ID_OK");
	m_pCancelButton = GetControl<CEtUIButton>("ID_CANCEL");

	m_pGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pCopper = GetControl<CEtUIStatic>("ID_BRONZE");

	m_dwColorGold = m_pGold->GetTextColor();
	m_dwColorSilver = m_pSilver->GetTextColor();
	m_dwColorCopper = m_pCopper->GetTextColor();

	m_SmartMove.SetControl(m_pCancelButton);
}

void CDnWorldServerStorageConfirmDlg::InitCustomControl(CEtUIControl *pControl)
{
	m_pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);
}

void CDnWorldServerStorageConfirmDlg::Show(bool bShow)
{ 
	if (bShow == m_bShow)
		return;

	if (bShow)
	{
		UpdateFee();

		m_SmartMove.MoveCursor();
	}
	else
	{
		Reset();
	}

	CDnCustomDlg::Show(bShow);
}

void CDnWorldServerStorageConfirmDlg::UpdateFee()
{
	wchar_t wszMoney[MAX_WSTORAGE_FEE_CHAR_COUNT] = {0, };
	INT64 nMoneyUnit = 0;
	TAX_TYPE nMoney = CommonUtil::GetFixedTax(TAX_WSTORAGE);

	if (CDnItemTask::IsActive())
	{
		if (CDnItemTask::GetInstance().GetCoin() < nMoney)
		{
			m_pGold->SetTextColor(textcolor::RED);
			m_pSilver->SetTextColor(textcolor::RED);
			m_pCopper->SetTextColor(textcolor::RED);
		}
		else
		{
			m_pGold->SetTextColor(m_dwColorGold);
			m_pSilver->SetTextColor(m_dwColorSilver);
			m_pCopper->SetTextColor(m_dwColorCopper);
		}
	}

	CDnMoneyControl::GetStrMoneyG(nMoney, wszMoney, &nMoneyUnit);
	m_pGold->SetInt64ToText(nMoneyUnit);
	CDnMoneyControl::GetStrMoneyS(nMoney, wszMoney, &nMoneyUnit);
	m_pSilver->SetInt64ToText(nMoneyUnit);
	CDnMoneyControl::GetStrMoneyC(nMoney, wszMoney, &nMoneyUnit);
	m_pCopper->SetInt64ToText(nMoneyUnit);
}

CDnItem* CDnWorldServerStorageConfirmDlg::GetItem()
{
	return static_cast<CDnItem*>(m_pItemSlotButton->GetItem());
}

bool CDnWorldServerStorageConfirmDlg::SetItem(CDnItem* pItem, int slotCount)
{
	if (!pItem)
		return false;

	m_pItemSlotButton->SetItem(pItem, slotCount);
	m_pItemName->SetTextColor(DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(pItem->GetItemRank()));
	m_pItemName->SetText(pItem->GetName());

	return true;
}

void CDnWorldServerStorageConfirmDlg::SetItemWithSlotButton(CDnSlotButton* pFromSlot, bool bRenderCountMinusOne)
{
	if (pFromSlot == NULL)
		return;

	m_bRenderCountMinusOne = bRenderCountMinusOne;

	MIInventoryItem* pInvenItem = pFromSlot->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	CDnItem* pItem = static_cast<CDnItem*>(pInvenItem);
	int slotCount = pFromSlot->GetRenderCount();
	if (slotCount > 0)
		slotCount = bRenderCountMinusOne ? (slotCount - 1) : slotCount;
	SetItem(pItem, slotCount);

	m_pFromSlotBtnCache = pFromSlot;
}

void CDnWorldServerStorageConfirmDlg::EnableButtons(bool bEnable)
{
	if (m_pOKButton == NULL || m_pCancelButton == NULL)
		return;
	m_pOKButton->Enable(bEnable);
	m_pCancelButton->Enable(bEnable);
}

void CDnWorldServerStorageConfirmDlg::Reset()
{
	m_SmartMove.ReturnCursor();

	m_pItemSlotButton->ResetSlot();
	m_pFromSlotBtnCache = NULL;
}

int CDnWorldServerStorageConfirmDlg::GetSlotItemCount() const
{
	return 1;
}

int CDnWorldServerStorageConfirmDlg::GetRealItemCount() const
{
	int slotItemCount = GetSlotItemCount();
	if (IsRenderCountMinusOne())
		return slotItemCount + 1;

	return slotItemCount;
}

bool CDnWorldServerStorageConfirmDlg::IsFromCashInventory() const
{
	if (m_pFromSlotBtnCache == NULL)
	{
		_ASSERT("WSS");
		return false;
	}

	ITEM_SLOT_TYPE slotType = m_pFromSlotBtnCache->GetSlotType();
	return (slotType == ST_INVENTORY_CASH);
}

bool CDnWorldServerStorageConfirmDlg::IsFromCashStorage() const
{
	if (m_pFromSlotBtnCache == NULL)
	{
		_ASSERT("WSS");
		return false;
	}

	ITEM_SLOT_TYPE slotType = m_pFromSlotBtnCache->GetSlotType();
	return (slotType == ST_STORAGE_WORLDSERVER_CASH);
}

void CDnWorldServerStorageConfirmDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BT_CLOSE"))
			Show(false);
	}
	CDnCustomDlg::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}


//////////////////////////////////////////////////////////////////////////

CDnWorldServerStorageConfirmExDlg::CDnWorldServerStorageConfirmExDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CDnWorldServerStorageConfirmDlg(dialogType, pParentDialog, nID, pCallback)
, m_pItemUp(NULL)
, m_pItemDown(NULL)
, m_pButtonMin(NULL)
, m_pButtonMax(NULL)
, m_pEditBoxItemCount(NULL)
{
}

void CDnWorldServerStorageConfirmExDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName( "StorageMessageBoxEXDlg.ui" ).c_str(), bShow);
}

void CDnWorldServerStorageConfirmExDlg::InitialUpdate()
{
	CDnWorldServerStorageConfirmDlg::InitialUpdate();

	m_pEditBoxItemCount = GetControl<CEtUIEditBox>("ID_EDITBOX_COUNT");
	m_pItemUp = GetControl<CEtUIButton>("ID_ITEM_UP");
	m_pItemDown = GetControl<CEtUIButton>("ID_ITEM_DOWN");

	m_pButtonMin = GetControl<CEtUIButton>("ID_BT_MINI");
	m_pButtonMax = GetControl<CEtUIButton>("ID_BT_MAX");
}

void CDnWorldServerStorageConfirmExDlg::UpdateAmount()
{
	CDnItem *pItem = static_cast<CDnItem*>(m_pItemSlotButton->GetItem());
	if (!pItem)
		return;

	int nCount = m_pEditBoxItemCount->GetTextToInt();
	int nOverCount(0);
	int nAmount(0);

	bool bPresentCost = true;
	nOverCount = pItem->GetOverlapCount();
	nAmount = pItem->GetItemAmount();

	if (nCount > nOverCount)
	{
		m_pEditBoxItemCount->SetIntToText(nOverCount);
	}
	else if (nCount >= 0 && nCount < 1)
	{
		m_pEditBoxItemCount->SetIntToText(1);
	}
}

bool CDnWorldServerStorageConfirmExDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!IsShow())
		return false;

	if (uMsg == WM_MOUSEWHEEL)
	{
		POINT MousePoint;
		MousePoint.x = short(LOWORD(lParam));
		MousePoint.y = short(HIWORD(lParam));

		float fMouseX, fMouseY;
		PointToFloat(MousePoint, fMouseX, fMouseY);

		if (IsMouseInDlg())
		{
			UINT uLines;
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0);
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			if (nScrollAmount > 0)
			{
				ProcessCommand(EVENT_BUTTON_CLICKED, false, m_pItemUp, 0);
				return true;
			}
			else if (nScrollAmount < 0)
			{
				ProcessCommand(EVENT_BUTTON_CLICKED, false, m_pItemDown, 0);
				return true;
			}
		}
	}

	// To make up/down keys work even when focus is on the edit box (quantity input), UI tool hotkeys won't work.
	// So handle it directly in MsgProc.
	if( uMsg == WM_KEYDOWN )
	{
		if (wParam == VK_UP)
		{
			ProcessCommand(EVENT_BUTTON_CLICKED, false, m_pItemUp, 0);
			return true;
		}
		else if (wParam == VK_DOWN)
		{
			ProcessCommand(EVENT_BUTTON_CLICKED, false, m_pItemDown, 0);
			return true;
		}
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

void CDnWorldServerStorageConfirmExDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_EDITBOX_CHANGE)
	{
		if (IsCmdControl("ID_EDITBOX_COUNT"))
		{
			UpdateAmount();
			m_pOKButton->Enable(m_pEditBoxItemCount->GetTextToInt() > 0);
			return;
		}
	}

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_ITEM_UP"))
		{
			m_pEditBoxItemCount->SetIntToText(m_pEditBoxItemCount->GetTextToInt() + 1);
			UpdateAmount();
			m_pOKButton->Enable(m_pEditBoxItemCount->GetTextToInt() > 0);
			return;
		}

		if (IsCmdControl("ID_ITEM_DOWN"))
		{
			int nCount = m_pEditBoxItemCount->GetTextToInt() - 1;
			nCount = max(nCount, 1);
			m_pEditBoxItemCount->SetIntToText(nCount);
			UpdateAmount();
			m_pOKButton->Enable(m_pEditBoxItemCount->GetTextToInt() > 0);
			return;
		}

		if (IsCmdControl("ID_BT_MINI"))
		{
			m_pEditBoxItemCount->SetIntToText(1);
			UpdateAmount();
			m_pOKButton->Enable(m_pEditBoxItemCount->GetTextToInt() > 0);
			return;
		}

		if (IsCmdControl("ID_BT_MAX"))
		{
			m_pEditBoxItemCount->SetIntToText(99999);
			UpdateAmount();
			m_pOKButton->Enable(m_pEditBoxItemCount->GetTextToInt() > 0);
			return;
		}
	}

	// If you enter 0 and press Enter, a message box saying you cannot purchase 0 items pops up while focus remains on the edit box,
	// Since focus remains on the edit box here, hotkeys on the message box dialog do not work.
	// So for now, make it lose focus like this.
	if( nCommand == EVENT_EDITBOX_STRING )
	{
		RequestFocus( GetControl<CEtUIButton>("ID_OK") );
	}

	CDnWorldServerStorageConfirmDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnWorldServerStorageConfirmExDlg::SetItem(CDnItem* pItem, int slotCount)
{
	if (!pItem)
		return false;

	m_pItemSlotButton->SetItem(pItem, slotCount);
	m_pItemName->SetTextColor(DN_INTERFACE::STRING::ITEM::RANK_2_COLOR(pItem->GetItemRank()));
	m_pItemName->SetText(pItem->GetName());

	m_pEditBoxItemCount->SetIntToText(1);
	return true;
}

int CDnWorldServerStorageConfirmExDlg::GetSlotItemCount() const
{
	return m_pEditBoxItemCount->GetTextToInt();
}

void CDnWorldServerStorageConfirmExDlg::EnableButtons(bool bEnable)
{
	if (m_pButtonMin == NULL || m_pButtonMax == NULL)
		return;
	m_pOKButton->Enable(bEnable);
	m_pCancelButton->Enable(bEnable);
	m_pButtonMin->Enable(bEnable);
	m_pButtonMax->Enable(bEnable);
}

void CDnWorldServerStorageConfirmExDlg::Reset()
{
	CDnWorldServerStorageConfirmDlg::Reset();

	m_pEditBoxItemCount->ClearText();
}

void CDnWorldServerStorageConfirmExDlg::Show(bool bShow)
{ 
	if (bShow == m_bShow)
		return;

	if (bShow)
	{
		UpdateFee();

		m_SmartMove.MoveCursor();
	}
	else
	{
		Reset();
	}

	CDnCustomDlg::Show(bShow);
}

void CDnWorldServerStorageConfirmExDlg::SetItemCountForce(int count)
{
	m_pEditBoxItemCount->SetIntToText(count);
}
