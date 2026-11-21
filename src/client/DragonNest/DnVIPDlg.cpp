#include "StdAfx.h"

#ifdef PRE_ADD_VIP

#include "DnVIPDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#include "DnInterfaceString.h"
#include "SystemSendPacket.h"
#include "DnAuthTask.h"
#include "DnSecurityNUMPADCheckDlg.h"
#include "TaskManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnVIPDlg::CDnVIPDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback) : CEtUIDialog(dialogType, pParentDialog, nID, pCallback, true),
																												 m_pBtnAutoPayCancel( NULL ),
																												 m_pBtnClose( NULL ),
																												 m_pStaticAutoPay( NULL ),
																												 m_pTextBoxContents( NULL )

{
}

void CDnVIPDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("VIPServiceDlg.ui").c_str(), bShow);
}

void CDnVIPDlg::InitialUpdate()
{
	m_pBtnAutoPayCancel = GetControl<CEtUIButton>("ID_BUTTON_AUTOPAYCANCEL");
	m_pBtnClose			= GetControl<CEtUIButton>("ID_BUTTON_CLOSE");
	m_pTextBoxContents	= GetControl<CEtUITextBox>("ID_TEXTBOX_CONTENTS");
	m_pStaticAutoPay	= GetControl<CEtUIStatic>("ID_STATIC_AUTOPAY");
	if (m_pTextBoxContents)
		m_pTextBoxContents->EnableWorkBreak(true);
}

void CDnVIPDlg::SetInfo(bool bAutoPay, const __time64_t& expireDate, DWORD expireDateColor, const std::wstring& contents)
{
	if (m_pBtnAutoPayCancel == NULL || m_pTextBoxContents == NULL || m_pTextBoxContents == NULL || m_pStaticAutoPay == NULL)
	{
		CDnInterface::GetInstance().MessageBox(L"VIP UI Data Loading Error : Please Re-install.");
		return;
	}

	m_pBtnAutoPayCancel->Enable(bAutoPay);
	m_pTextBoxContents->ClearText();
	m_pTextBoxContents->SetText(contents.c_str());

	if (bAutoPay)
	{
		m_pStaticAutoPay->SetText(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020023));	// UISTRING : Auto-billing service is currently active.
	}
	else
	{
		if (expireDate != 0)
		{
			std::wstring temp, dayText;

			m_pStaticAutoPay->SetTextColor(expireDateColor);
			DN_INTERFACE::STRING::GetDayText(dayText, expireDate);
			temp = FormatW(L"%s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020014), dayText.c_str()); // 2020014
			m_pStaticAutoPay->SetText(temp.c_str());
		}
	}
}

void CDnVIPDlg::Show(bool bShow)
{
	if (m_bShow == bShow)
		return;

	CEtUIDialog::Show(bShow);
}

void CDnVIPDlg::Process(float fElapsedTime)
{
	CEtUIDialog::Process(fElapsedTime);
}

void CDnVIPDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	SetCmdControlName(pControl->GetControlName());

	if (nCommand == EVENT_BUTTON_CLICKED)
	{
		CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
		if (!pMainMenuDlg) return;

		if (IsCmdControl("ID_BUTTON_CLOSE"))
		{
			pMainMenuDlg->ShowSystemDialog( true );
			return;
		}

		if (IsCmdControl("ID_BUTTON_AUTOPAYCANCEL"))
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020005), MB_YESNO, eMSGBOX_AUTOPAYCANCEL, this); // UISTRING : Are you sure you want to cancel auto-billing?
			return;
		}
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}

void CDnVIPDlg::OnUICallbackProc(int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg)
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case eMSGBOX_AUTOPAYCANCEL:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
				if (IsCmdControl("ID_YES"))
				{
					CDnAuthTask *pAuthTask = (CDnAuthTask *)CTaskManager::GetInstance().GetTask("AuthTask");
					if (pAuthTask)
					{
						if (pAuthTask->GetSecondAuthPW())
							GetInterface().OpenSecurityCheckDlg(CDnInterface::InterfaceTypeEnum::Login, CDnAuthTask::Validate_VIP);
						else
							SendVIPAutomaticPay(false);
					}
					else
					{
						return;
					}

					CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
					if (!pMainMenuDlg) return;

					pMainMenuDlg->ShowSystemDialog( true );
					return;
				}
			}
		}
	}
}

#endif // PRE_ADD_VIP