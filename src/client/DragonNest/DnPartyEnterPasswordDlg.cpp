#include "StdAfx.h"
#include "DnPartyEnterPasswordDlg.h"
#include "TaskManager.h"
#include "DnPartyTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
CDnPartyEnterPasswordDlg::CDnPartyEnterPasswordDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnPartyEnterDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pEditBoxPassWord(NULL)
{
}
#else 
CDnPartyEnterPasswordDlg::CDnPartyEnterPasswordDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnPartyEnterDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pEditBoxPassWord(NULL)
{
}
#endif 

CDnPartyEnterPasswordDlg::~CDnPartyEnterPasswordDlg(void)
{
}

void CDnPartyEnterPasswordDlg::Initialize( bool bShow, ePartyType type )
{
	m_Type = type;
	std::string resourcePath = "PartyPassDlg.ui";
	if (type == _RAID_PARTY_8)
		resourcePath = "PartyRaidPassDlg.ui";
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName(resourcePath.c_str()).c_str(), bShow );
}

void CDnPartyEnterPasswordDlg::InitialUpdate()
{
	CDnPartyEnterDlg::InitialUpdate();

	m_pEditBoxPassWord = GetControl<CEtUIEditBox>("ID_EDITBOX_PASS");
}

void CDnPartyEnterPasswordDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CDnPartyEnterDlg::Show( bShow );

	if( bShow )
	{
		m_pEditBoxPassWord->ClearText();
		RequestFocus(m_pEditBoxPassWord);
	}
}

#ifdef PRE_PARTY_DB
int CDnPartyEnterPasswordDlg::GetPassword()
{
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	// If password is entered as 0 
	return m_pEditBoxPassWord->GetTextToInt() == 0 ? Party::Constants::INCORRECT_PASSWORD : m_pEditBoxPassWord->GetTextToInt(); 
#else
	return m_pEditBoxPassWord->GetTextToInt();
#endif 
}
#else
LPCWSTR CDnPartyEnterPasswordDlg::GetPassword()
{
	return m_pEditBoxPassWord->GetText();
}
#endif

void CDnPartyEnterPasswordDlg::OnPartyJoinFailed()
{
	RequestFocus(m_pEditBoxPassWord);
}

void CDnPartyEnterPasswordDlg::Process( float fElapsedTime )
{
	if (IsShow())
	{
		CDnPartyTask *pPartyTask  = static_cast<CDnPartyTask*>(CTaskManager::GetInstancePtr()->GetTask( "PartyTask" ));
		if (pPartyTask == NULL)
			return;

		m_pConfirmBtn->Enable(m_pEditBoxPassWord->GetTextLength() == pPartyTask->GetMaxPasswordLength());
	}

	CDnPartyEnterDlg::Process(fElapsedTime);
}

bool CDnPartyEnterPasswordDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			Show(false);
			return true;
		}
	}

	return CDnPartyEnterDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}