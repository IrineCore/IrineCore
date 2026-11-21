#include "stdafx.h"
#include "DnCashShopCouponDlg.h"
#include "DnCashShopTask.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnCashShopCouponDlg::CDnCashShopCouponDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true)
, m_pOkBtn( NULL )
, m_pCancelBtn( NULL )
, m_pNumberInputEditBox( NULL )
, m_pNumberStatic( NULL )
{
}

CDnCashShopCouponDlg::~CDnCashShopCouponDlg()
{
}

void CDnCashShopCouponDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("CSGoodsCoupon.ui").c_str(), bShow);
}

void CDnCashShopCouponDlg::InitialUpdate()
{
	m_pNumberInputEditBox	= GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_INPUT");
	m_pNumberStatic			= GetControl<CEtUIStatic>("ID_STATIC_CNUMBER");
	m_pNumberInputEditBox->SetMaxEditLength(COUPONMAX + 1);
	m_pNumberInputEditBox->SetFilterOption(CEtUIEditBox::eDIGIT | CEtUIEditBox::eALPHABET);

	m_pOkBtn				= GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pCancelBtn			= GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
}

void CDnCashShopCouponDlg::SendCouponNumber()
{
	std::wstring str = m_pNumberInputEditBox->GetText();
	transform(str.begin(), str.end(), str.begin(), toupper);
	GetCashShopTask().RequestCashShopRecvItemByCoupon(str.c_str());
}

void CDnCashShopCouponDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		if (IsCmdControl("ID_BUTTON_OK"))
			SendCouponNumber();

		if (IsCmdControl("ID_BUTTON_CANCEL"))
			Show(false);
	}
	else if (nCommand == EVENT_EDITBOX_CHANGE)
	{
		if (uMsg == 22)	//	CTRL + V
		{
			std::wstring result;

			m_pNumberInputEditBox->ClearText();

			if (OpenClipboard(NULL))
			{
				HANDLE handle;

				handle = GetClipboardData(CF_UNICODETEXT);
				if (handle)
				{
					WCHAR *pwszText;
					pwszText = (WCHAR*)GlobalLock(handle);
					if (pwszText)
					{
						result = pwszText;
						std::wstring::size_type found = result.find_first_of(L"- ");
						while (found != string::npos) 
						{
							result.erase(found, 1);
							found = result.find_first_of(L"- ", found+1);
						}
						result = result.substr(0, COUPONMAX - 1);

						GlobalUnlock( handle );
					}
				}
				CloseClipboard();
			}

			if (result.empty() == false)
			{
				int i = 0;
				for (; i < (int)result.size();++i)
				{
					const wchar_t letter = result.at(i);
					if (iswdigit(letter) == false && iswalpha(letter) == false)
					{
						focus::ReleaseControl();
						GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4579)); // UISTRING : Cannot paste because it contains characters other than letters and numbers.
						return;
					}
				}

				m_pNumberInputEditBox->SetText(result.c_str());
			}
		}
		return;
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnCashShopCouponDlg::Process(float fElapsedTime)
{
	if (m_bShow == false)
		return;

	if (m_bShow)
	{
		m_pOkBtn->Enable(m_pNumberInputEditBox->GetTextLength() > 0);

		std::wstring str = m_pNumberInputEditBox->GetText();
		const std::wstring seperator = L"-";
		if (str.empty() == false)
		{
			transform(str.begin(), str.end(), str.begin(), toupper);
			int wholeLoopCount = (int)str.size() / COUPON_NUMBER_SEPARATE_UNIT;
			int i = 0;
			for (; i < wholeLoopCount; ++i)
			{
				int index = COUPON_NUMBER_SEPARATE_UNIT * (i + 1) + (int)seperator.size() * i;
				if (index >= (int)str.size())
					break;
				else
					str.insert(index, seperator);
			}

			m_pNumberStatic->SetText(str.c_str());
		}
		else
		{
			m_pNumberStatic->ClearText();
		}
	}

	m_pNumberInputEditBox->EnableImeSystem(false);

	CEtUIDialog::Process(fElapsedTime);
}

bool CDnCashShopCouponDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch ((WCHAR)uMsg)
	{
	case WM_KEYDOWN:
		{
			if (focus::IsSameControl(m_pNumberInputEditBox))
			{
				if( wParam == VK_RETURN )
				{
					focus::ReleaseControl();
					SendCouponNumber();
					return true;
				}
				else if (wParam == VK_ESCAPE)
				{
					Show(false);
					return true;
				}
			}
		}
		break;

	default:
		break;
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}

void CDnCashShopCouponDlg::Show(bool bShow)
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if (m_bShow)
	{
		m_pNumberStatic->ClearText();
		m_pNumberInputEditBox->ClearText();
	}
	else
	{
		m_pNumberInputEditBox->EnableImeSystem(true);
	}
};