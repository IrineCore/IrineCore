#include "StdAfx.h"
#include "DnCashShopRecvGiftItemDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnCashShopRecvGiftItemDlg::CDnCashShopRecvGiftItemDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
, m_giftDBID( 0 )
, m_pNameStatic( NULL )
, m_pDateStatic( NULL )
, m_pRestStatic( NULL )
{
}

void CDnCashShopRecvGiftItemDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("CSPresent_Flist.ui").c_str(), bShow);
}

void CDnCashShopRecvGiftItemDlg::InitialUpdate()
{
	m_pNameStatic = GetControl<CEtUIStatic>("ID_STATIC_NAME");
	m_pDateStatic = GetControl<CEtUIStatic>("ID_STATIC0");
	m_pRestStatic = GetControl<CEtUIStatic>("ID_STATIC1");
}

void CDnCashShopRecvGiftItemDlg::SetInfo(const SCashShopRecvGiftBasicInfo& info)
{
	Clear();

	m_giftDBID = info.giftDBID;

	m_pNameStatic->SetText(info.name.c_str());
	m_pDateStatic->SetText(info.recvDate.c_str());

// 	std::wstring restDayStr;
// 	restDayStr = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4640), info.rest); // UISTRING : %d days
//	m_pRestStatic->SetText(restDayStr.c_str());
	m_pRestStatic->SetText(info.expireDate.c_str());
}

void CDnCashShopRecvGiftItemDlg::Clear()
{
	m_pNameStatic->ClearText();
	m_pDateStatic->ClearText();
	m_pRestStatic->ClearText();
}

// void CDnCashShopRecvGiftItemDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
// {
// 	SetCmdControlName(pControl->GetControlName());
// 
// 	if (nCommand == )
// 		if (IsCmdControl("ID_COMBOBOX_ABILITY") && m_pAbilityComboBox->IsOpenedDropDownBox() == false)
// 		{
// 		}
// 
// 		CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
// 	}
// }