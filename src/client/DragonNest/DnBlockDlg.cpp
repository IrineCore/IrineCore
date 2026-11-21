#include "StdAfx.h"
#include "DnBlockDlg.h"
#include "DnBlockAddDlg.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBlockListDlg::CDnBlockListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pButtonInfo( NULL )
	, m_pStaticName( NULL )
{
}

CDnBlockListDlg::~CDnBlockListDlg()
{
}

void CDnBlockListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "BlockListDlg.ui" ).c_str(), bShow );
}

void CDnBlockListDlg::InitialUpdate()
{
	m_pButtonInfo = GetControl<CEtUIButton>( "ID_BT_INFO" );
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	if( m_pButtonInfo ) m_pButtonInfo->Show( false );
}

void CDnBlockListDlg::SetInfo( const std::wstring &wszName )
{
	if( m_pStaticName ) m_pStaticName->SetText( wszName.c_str() );
}

LPCWSTR CDnBlockListDlg::GetName() const
{
	return m_pStaticName ? m_pStaticName->GetText() : L"";
}

CDnBlockDlg::CDnBlockDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_pNameList(NULL)
	, m_pButtonAdd(NULL)
	, m_pButtonDel(NULL)
	, m_pBlockAddDlg( NULL )
{
}

CDnBlockDlg::~CDnBlockDlg()
{
	SAFE_DELETE(m_pBlockAddDlg);
}

void CDnBlockDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "BlockDlg.ui" ).c_str(), bShow );

	if( CDnIsolateTask::IsActive() )
		GetIsolateTask().SetDlg( this );
	else
		CDebugSet::ToLogFile( "CDnBlockDlg::Initialize, task must be created before dialog is created." );
}

void CDnBlockDlg::InitialUpdate()
{
	m_pNameList		= GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");
	m_pButtonAdd	= GetControl<CEtUIButton>("ID_BUTTON_ADD");
	m_pButtonDel	= GetControl<CEtUIButton>("ID_BUTTON_DEL");

	m_pBlockAddDlg	= new CDnBlockAddDlg( UI_TYPE_CHILD, this, CHILD_BLOCK_ADD, this );
	m_pBlockAddDlg->Initialize( false );
}

CDnBlockListDlg* CDnBlockDlg::GetSelectedBlockItem()
{
	if( m_pNameList ) {
		int nSel = m_pNameList->GetSelectedIndex();
		if( nSel != -1 )
			return m_pNameList->GetItem<CDnBlockListDlg>( nSel );
	}
	return NULL;
}

void CDnBlockDlg::Reset()
{
	m_pNameList->RemoveAllItems();
}

void CDnBlockDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if (bShow)
	{
		Reset();
		if (GetIsolateTask().ReqIsolateList() == false)
			RefreshList();
	}
	else
	{
		ShowChildDialog(m_pBlockAddDlg, false);
	}

	CEtUIDialog::Show( bShow );
}

void CDnBlockDlg::Process( float fElapsedTime )
{
	if (m_bShow)
	{
		bool bEnable = false;
		if ( GetInterface().IsPVP() || GetInterface().IsFarm() )
			bEnable = true;

		m_pButtonAdd->Enable(bEnable == false);
	}

	CEtUIDialog::Process(fElapsedTime);
}

void CDnBlockDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_ADD") )
		{
			if (GetIsolateTask().GetBlackListCount() + 1 > ISOLATELISTMAX)
			{
				std::wstring str;
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4213), ISOLATELISTMAX); // UISTRING : You can register up to %d blocked users.
				GetInterface().MessageBox(str.c_str(), MB_OK);
				return;
			}

			m_pBlockAddDlg->Show(true);
			return;
		}

		if( IsCmdControl("ID_BUTTON_DEL") )
		{
			if (m_pNameList->GetSize() <= 0)
			{
				GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4214), MB_OK); // UISTRING : No users to delete
			}
			else
			{
				CDnBlockListDlg *pItem = GetSelectedBlockItem();
				if( pItem )
					GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 1961), MB_YESNO, MESSAGEBOX_BLOCK_DELETE, this); // UISTRING : Are you sure you want to delete?
				else
					GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4215), MB_OK); // UISTRING : Please select a user to delete
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnBlockDlg::RefreshList()
{
	Reset();

	const std::list<std::wstring>& blackList = GetIsolateTask().GetBlackList();
	std::list<std::wstring>::const_iterator iter = blackList.begin();
	for (; iter != blackList.end(); ++iter)
	{
		const std::wstring& userName = *iter;
		CDnBlockListDlg *pItem = m_pNameList->AddItem<CDnBlockListDlg>();
		if( pItem ) pItem->SetInfo( userName );
	}
}

void CDnBlockDlg::ShowAddDlg(bool bShow)
{
	if (m_pBlockAddDlg == NULL)
		_ASSERT(0);

	ShowChildDialog(m_pBlockAddDlg, bShow);
}

void CDnBlockDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case CHILD_BLOCK_ADD:
		{
			if (nCommand == EVENT_BUTTON_CLICKED)
			{
				if (IsCmdControl("ID_CANCEL"))
					ShowChildDialog(m_pBlockAddDlg, false);
			}
		}
		break;

	case MESSAGEBOX_BLOCK_DELETE:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if (IsCmdControl("ID_YES"))
				{
					CDnBlockListDlg *pItem = GetSelectedBlockItem();
					if( pItem && pItem->GetName() )
						GetIsolateTask().ReqIsolateDel(pItem->GetName());
				}
			}
		}
		break;

	case MESSAGEBOX_BLOCK_SERVER_ERROR_MSG:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( IsCmdControl("ID_OK" ) )
				{
					if (m_pBlockAddDlg && m_pBlockAddDlg->IsShow())
						m_pBlockAddDlg->ReturnFocus();
				}
			}
		}
		break;
	}
}

bool CDnBlockDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_DELETE )
		{
			CDnBlockListDlg *pItem = GetSelectedBlockItem();
			if( pItem )
			{
				GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1961 ), MB_YESNO, MESSAGEBOX_BLOCK_DELETE, this); // UISTRING : Are you sure you want to delete?
			}
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnBlockDlg::HandleServerErrorCode(int nRet)
{
// 	if (nRet == 6) // User does not exist.
// 	{
// 		if (m_pBlockAddDlg)
// 			m_pBlockAddDlg->ReturnFocus();
// 	}
}
