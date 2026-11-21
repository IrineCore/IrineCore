#include "StdAfx.h"
#include "DnCashShopPayItemDlg.h"
#include "DnCashShopTask.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnCashShopPayDlg.h"
#ifdef PRE_MOD_PETAL_WRITE
#include "DnInterfaceString.h"
#endif // PRE_MOD_PETAL_WRITE


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnCashShopPayItemDlg::CDnCashShopPayItemDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_pItemSlotBG( NULL )
, m_pNameStatic( NULL )
, m_pSlotBtn( NULL )
, m_pBase( NULL )
, m_pCountStatic( NULL )
, m_pPriceStatic( NULL )
, m_pReserveStatic( NULL )
, m_pAbilityComboBox( NULL )
, m_pPeriodComboBox( NULL )
, m_pItem( NULL )
, m_Index( 0 )
, m_Mode( eCashUnit_None )
{
}

CDnCashShopPayItemDlg::~CDnCashShopPayItemDlg(void)
{
	SAFE_DELETE(m_pItem);
}

void CDnCashShopPayItemDlg::Initialize(bool bShow)
{
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("CS_CSList.ui").c_str(), bShow);
#else
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("CSList.ui").c_str(), bShow);
#endif // PRE_ADD_CASHSHOP_RENEWAL
}

void CDnCashShopPayItemDlg::InitialUpdate()
{
	m_pBase				= GetControl<CEtUIStatic>("ID_STATIC0");
	m_pNameStatic		= GetControl<CEtUIStatic>("ID_STATIC_NAME");

	m_pItemSlotBG		= GetControl<CEtUIStatic>("ID_STATIC1");

	m_pCountStatic		= GetControl<CEtUIStatic>("ID_STATIC_COUNT");
	m_pPriceStatic		= GetControl<CEtUIStatic>("ID_STATIC_PRICE");
	m_pReserveStatic	= GetControl<CEtUIStatic>("ID_STATIC_RESERVE");

	m_pAbilityComboBox	= GetControl<CEtUIComboBox>("ID_COMBOBOX_ABILITY");
	m_pAbilityComboBox->SetEditMode(false);
	m_pPeriodComboBox	= GetControl<CEtUIComboBox>("ID_COMBOBOX_PERIOD");
	m_pPeriodComboBox->SetEditMode(false);
}

void CDnCashShopPayItemDlg::InitCustomControl(CEtUIControl *pControl)
{
	if (pControl == NULL || !strstr(pControl->GetControlName(), "ID_BUTTON_ITEM"))
		return;

	m_pSlotBtn = static_cast<CDnItemSlotButton*>(pControl);
	m_pSlotBtn->SetSlotType(ST_INVENTORY);
	m_pSlotBtn->SetSlotIndex(0);
}

void CDnCashShopPayItemDlg::SetCostumeMode()
{
	m_pCountStatic->Show(false);
	m_pPriceStatic->Show(false);
	m_pReserveStatic->Show(false);
	m_pAbilityComboBox->Show(true);
	m_pPeriodComboBox->Show(true);
}

void CDnCashShopPayItemDlg::SetNormalMode()
{
	m_pCountStatic->Show(true);
	m_pPriceStatic->Show(true);
	m_pReserveStatic->Show(true);
	m_pAbilityComboBox->Show(false);
	m_pPeriodComboBox->Show(false);
}

void CDnCashShopPayItemDlg::SetInfo(eCashUnitType mode, const SCashShopItemInfo& data, const SCashShopCartItemInfo& cartData)
{
	m_pNameStatic->SetText(data.nameString.c_str());

	//m_Index = cartData.index;
	m_Mode = mode;

	if (data.presentItemId != ITEMCLSID_NONE)
	{
		SAFE_DELETE(m_pItem);
		TItemInfo itemInfo;
		if (CDnItem::MakeItemInfo(data.presentItemId, data.count, itemInfo))
		{
			m_pItem = GetItemTask().CreateItem(itemInfo);
			if (m_pItem)
				m_pSlotBtn->SetItem(m_pItem, m_pItem->GetOverlapCount());
		}
	}

#ifdef PRE_ADD_VIP
	if (data.type == eCSType_Costume || data.type == eCSType_Term)
#else
	if (data.type == eCSType_Costume)
#endif
	{
		const std::vector<ITEMCLSID>& itemIdList = data.abilityList;

		int i = 0;
		for (; i < (int)itemIdList.size(); ++i)
		{
			ITEMCLSID id = itemIdList[i];

			std::wstring strAbility = GetCashShopTask().GetAbilityString( id );
			if( strAbility.length() > 0 )
				m_pAbilityComboBox->AddItem( strAbility.c_str(), NULL, id, true );
		}

		if (cartData.ability != ITEMCLSID_NONE)
			m_pAbilityComboBox->SetSelectedByValue(cartData.ability);
		else
			m_pAbilityComboBox->SetSelectedByIndex(0);
		SComboBoxItem* pItem = m_pAbilityComboBox->GetSelectedItem();
		if (pItem)
			GetCashShopTask().SetCartItemAbility(m_Index, pItem->nValue);

		const std::vector<CASHITEM_SN>& itemSnList = data.linkIdList;

		i = 0;
		for (; i < (int)itemSnList.size(); ++i)
		{
			CASHITEM_SN curSN = itemSnList[i];
			const SCashShopItemInfo* pItemInfo = GetCashShopTask().GetItemInfo(curSN);
			if (pItemInfo)
			{
				std::wstring str;
				str = (pItemInfo->period < 0) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4705 ) : FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4706 ), pItemInfo->period);	// UISTRING : Until end of service / Use for %d days
				m_pPeriodComboBox->AddItem(str.c_str(), NULL, curSN);
			}
		}

		m_pPeriodComboBox->SetSelectedByValue(data.sn);

		SetCostumeMode();
	}
	else if (data.type == eCSType_Function)
	{
		std::wstring str;
		str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4625), data.count);	// UISTRING : %d ea
		m_pCountStatic->SetText(str.c_str());

#ifdef PRE_ADD_PETALSHOP
#ifdef PRE_MOD_PETAL_WRITE
		int nUIStringIndex = 2030002;	// UISTRING : %s Petal
#else // PRE_MOD_PETAL_WRITE
		int nUIStringIndex = 4647;	// UISTRING : %d Petal
#endif // PRE_MOD_PETAL_WRITE
#else // PRE_ADD_PETALSHOP
#ifdef PRE_MOD_PETAL_WRITE
		int nUIStringIndex = 2030001;	// UISTRING : %s Cash
#else // PRE_MOD_PETAL_WRITE
		int nUIStringIndex = 4642;	// UISTRING : %d Cash
#endif // PRE_MOD_PETAL_WRITE
#endif // PRE_ADD_PETALSHOP

#ifdef PRE_MOD_PETAL_WRITE
		str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nUIStringIndex ), DN_INTERFACE::UTIL::GetAddCommaString( data.price ).c_str() );
#else // PRE_MOD_PETAL_WRITE
		str = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringIndex), data.price );
#endif // PRE_MOD_PETAL_WRITE
		
		m_pPriceStatic->SetText(str.c_str());

		SetNormalMode();
	}
}

void CDnCashShopPayItemDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if ((nCommand == EVENT_COMBOBOX_SELECTION_CHANGED) && bTriggeredByUser)
	{
		if (IsCmdControl("ID_COMBOBOX_ABILITY") && m_pAbilityComboBox->IsOpenedDropDownBox() == false)
		{
			SComboBoxItem* pItem = m_pAbilityComboBox->GetSelectedItem();
			if (pItem)
			{
				if (m_Mode == eCashUnit_Cart)
					GetCashShopTask().SetCartItemAbility(m_Index, pItem->nValue);
				else if (m_Mode == eCashUnit_BuyItemNow)
					GetCashShopTask().SetBuyItemNowAbility(pItem->nValue);
				else if (m_Mode == eCashUnit_PreviewCart)
					GetCashShopTask().SetPreviewCartItemAbility(m_Index, pItem->nValue);
			}
		}
		else if (IsCmdControl("ID_COMBOBOX_PERIOD") && m_pAbilityComboBox->IsOpenedDropDownBox() == false)
		{
			SComboBoxItem* pItem = m_pPeriodComboBox->GetSelectedItem();
			if (pItem)
			{
				GetCashShopTask().SelectCartItemSN(m_Mode, m_Index, pItem->nValue);
			}
		}
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}