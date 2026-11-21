#include "StdAfx.h"
#include "DnMailSendConfirmDlg.h"
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMailSendConfirmDlg::CDnMailSendConfirmDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallBack )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallBack ),
	m_pButtonCancel( NULL ),
	m_pButtonSend( NULL ),
	m_pCoinB( NULL ),
	m_pCoinG( NULL ),
	m_pCoinS( NULL ),
	m_pStaticConfirm( NULL ),
	m_pStaticQuick( NULL )
{

}

void CDnMailSendConfirmDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("MailSendConfirm.ui").c_str(), bShow);
}

void CDnMailSendConfirmDlg::InitialUpdate()
{
	m_pStaticConfirm	= GetControl<CEtUIStatic>("ID_STATIC2");
	m_pStaticQuick		= GetControl<CEtUIStatic>("ID_QUICK");

	m_pCoinG			= GetControl<CEtUIStatic>("ID_GOLD");
	m_pCoinS			= GetControl<CEtUIStatic>("ID_SILVER");
	m_pCoinB			= GetControl<CEtUIStatic>("ID_BRONZE");

	m_pButtonSend		= GetControl<CEtUIButton>("ID_SEND");
	m_pButtonCancel		= GetControl<CEtUIButton>("ID_CANCEL");

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_SEND") );
}

void CDnMailSendConfirmDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	if (bShow)
		m_SmartMove.MoveCursor();
	else
		m_SmartMove.ReturnCursor();

	CEtUIDialog::Show(bShow);
}

void CDnMailSendConfirmDlg::Show(bool bShow, bool bPremium, const std::wstring& receiverName)
{
	if (bShow)
	{
		std::wstring str;
		if (bPremium)
			str = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114039); // UISTRING : Premium

		m_pStaticQuick->SetText(str.c_str());

		str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1992), receiverName.c_str()); // UISTRING : Send mail to %s?
		m_pStaticConfirm->SetText(str.c_str());
	}

	Show(bShow);
}

void CDnMailSendConfirmDlg::SetTax(const TAX_TYPE& amount)
{
	CommonUtil::MONEY money(amount);

	m_pCoinG->SetInt64ToText(money.GetG());
	m_pCoinS->SetInt64ToText(money.GetS());
	m_pCoinB->SetInt64ToText(money.GetB());
}