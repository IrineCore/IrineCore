#include "StdAfx.h"
#include "DnCashShopMessageBox.h"
#include "DnCashShopTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCashShopMessageBox::CDnCashShopMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pOKBtn(NULL)
	, m_pCancelBtn(NULL)
{
	int i = 0;
	for (; i < eMax; ++i)
	{
		m_pText[i] = NULL;
	}
}

CDnCashShopMessageBox::~CDnCashShopMessageBox(void)
{
}

void CDnCashShopMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName("CSMessageBox.ui").c_str(), bShow);
}

void CDnCashShopMessageBox::InitialUpdate()
{
	m_pOKBtn		= GetControl<CEtUIButton>("ID_YES");
	m_pCancelBtn	= GetControl<CEtUIButton>("ID_NO");

	int i = 0;
	std::string ctrlName;
	for (; i < eMax; ++i)
	{
		ctrlName = FormatA("ID_MESSAGE%d", i);
		m_pText[i] = GetControl<CEtUIStatic>(ctrlName.c_str());
	}
//	m_SmartMove.SetControl( GetControl<CEtUIButton>( "ID_YES" ) );
}

void CDnCashShopMessageBox::SetMsgBox(int nID, CEtUICallback* pCallBack)
{
	SetDialogID(nID);
	SetCallback(pCallBack);
	Show(true);
}

void CDnCashShopMessageBox::SetMsgBoxText(eCSMsgBoxTextType type, const std::wstring& msg)
{
	m_pText[type]->SetText(msg.c_str());
}

void CDnCashShopMessageBox::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show(false);
	}
}
/*
void CDnCashShopMessageBox::Show( bool bShow )
{
	if( bShow )
		m_SmartMove.MoveCursor();
	else
		m_SmartMove.ReturnCursor();
	CEtUIDialog::Show( bShow );
}
*/