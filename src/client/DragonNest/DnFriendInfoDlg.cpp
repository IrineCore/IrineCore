#include "StdAfx.h"
#include "DnFriendInfoDlg.h"
#include "DnTextInputDlg.h"
#include "DnFriendTask.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnFriendInfoDlg::CDnFriendInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pComboBoxGroup(NULL)
	, m_pStaticName(NULL)
	, m_pStaticClass(NULL)
	, m_pStaticUserLevel(NULL)
// 	, m_pStaticWorldLevel(NULL)
	, m_pStaticMap(NULL)
	//, m_pStaticParty(NULL)
	, m_pButtonOK(NULL)
	, m_pButtonCancel(NULL)
// 	, m_pTextBoxMemo(NULL)
// 	, m_pButtonEdit(NULL)
// 	, m_pButtonDelete(NULL)
	, m_pTextInputDlg(NULL)
	, m_biFrinedCharDBID(0)
{
}

CDnFriendInfoDlg::~CDnFriendInfoDlg(void)
{
	SAFE_DELETE( m_pTextInputDlg );
}

void CDnFriendInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "FriendInfoDlg.ui" ).c_str(), bShow );
}

void CDnFriendInfoDlg::InitialUpdate()
{
	m_pComboBoxGroup = GetControl<CEtUIComboBox>("ID_COMBOBOX_GROUP");
	m_pStaticName = GetControl<CEtUIStatic>("ID_STATIC_NAME");
	m_pStaticClass = GetControl<CEtUIStatic>("ID_STATIC_CLASS");
	m_pStaticUserLevel = GetControl<CEtUIStatic>("ID_STATIC_USERLEVEL");
	//m_pStaticWorldLevel = GetControl<CEtUIStatic>("ID_STATIC_WORLDLEVEL");
	m_pStaticMap = GetControl<CEtUIStatic>("ID_STATIC_MAP");
	//m_pStaticParty = GetControl<CEtUIStatic>("ID_STATIC_PARTY");
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");
	//m_pTextBoxMemo = GetControl<CEtUITextBox>("ID_TEXTBOX_MEMO");
	//m_pButtonEdit = GetControl<CEtUIButton>("ID_BUTTON_EDIT");
	//m_pButtonDelete = GetControl<CEtUIButton>("ID_BUTTON_DELETE");
	CEtUIStatic* pTestSTatic = static_cast<CEtUIStatic*>(GetControl("ID_STATIC3"));

	m_pTextInputDlg = new CDnTextInputDlg( UI_TYPE_CHILD, this, -1, this );
	m_pTextInputDlg->Initialize( false );
}

void CDnFriendInfoDlg::UpdateFriendInfo()
{
	int nGroupID(-1);
	m_pComboBoxGroup->GetSelectedValue( nGroupID );
	GetFriendTask().RequestFriendUpdate( m_biFrinedCharDBID, nGroupID );//, m_pTextBoxMemo->GetText() );
}

void CDnFriendInfoDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process(fElapsedTime);
}

void CDnFriendInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK" ) )
			UpdateFriendInfo();

// 		if( IsCmdControl("ID_BUTTON_EDIT") )
// 		{
// 			m_pTextInputDlg->SetText( m_pTextBoxMemo->GetText() );
// 			ShowChildDialog( m_pTextInputDlg, true );
// 			return;
// 		}

// 		if( IsCmdControl("ID_BUTTON_DELETE") )
// 		{
// 			m_pTextBoxMemo->ClearText();
// 			return;
// 		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnFriendInfoDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
	}
	else
	{
	}
}

void CDnFriendInfoDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK" ) )
		{
			std::wstring strTemp( m_pTextInputDlg->GetText() );
			if( !strTemp.empty() )
			{
// 				m_pTextBoxMemo->ClearText();
// 				m_pTextBoxMemo->AddText( strTemp.c_str() );
			}
		}
	}
}

void CDnFriendInfoDlg::GetLocationText(tstring& result, const TCommunityLocation& info)
{
	result = _T("");
	switch(info.cServerLocation)
	{
	case CDnFriendTask::_LOCATION_NONE:			
		{
			result = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4305 );//	UISTRING : Offline
		}
		return;

	case CDnFriendTask::_LOCATION_MOVE:
		{
			result = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4306 );	// UISTRING : Moving
		}
		return;

	case CDnFriendTask::_LOCATION_LOGIN:
		{
			result = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4309 );	// UISTRING : Logging in
		}
		return;

	case CDnFriendTask::_LOCATION_VILLAGE:
		{
			std::wstring mapName;
			GetFriendTask().GetMapName(mapName, info.nMapIdx);
			if (mapName.empty() || info.nChannelID == 0 || info.nChannelID == -1)
				_ASSERT(0);
			result = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4307 ), mapName.c_str(), info.nChannelID);	// UISTRING : %s (Channel: %d)
		}
		return;

	case CDnFriendTask::_LOCATION_GAME:
		{
			GetFriendTask().GetMapName(result, info.nMapIdx);
			if (result.empty())
				_ASSERT(0);
		}
		return;

	default:
		{
			result = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4308 );	// UISTRING : No info
		}
		return;
	}
}

void CDnFriendInfoDlg::RefreshFriendInfo( INT64 biCharDBID )
{
	if( biCharDBID <= 0 )
		return;

	m_biFrinedCharDBID = biCharDBID;

	CDnFriendTask::SFriendInfo *pFriendInfo = GetFriendTask().GetFriendInfo( m_biFrinedCharDBID );
	if( !pFriendInfo )
		return;

	m_pStaticName->SetText( pFriendInfo->wszFriendName );
	m_pStaticClass->SetText( DN_INTERFACE::STRING::GetJobString( pFriendInfo->nJob ) );
	m_pStaticUserLevel->SetIntToText( pFriendInfo->nLevel );
// 	m_pStaticWorldLevel->SetIntToText( pFriendInfo->nActLevel );
	// Note : Display online/offline status and current character location using location info.
	//
	//m_pStaticMap->SetIntToText( pFriendInfo->cLocation );
	tstring locationText;
	GetLocationText(locationText, pFriendInfo->Location);
	m_pStaticMap->SetText(locationText.c_str());
	//m_pStaticParty->SetText( pFriendInfo->bIsParty ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 632 ) : GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 93 ) );

	//SetFriendMemo( pFriendInfo->wszFriendMemo.c_str() );
	SetFriendGroup( pFriendInfo->nGroupDBID );
}

void CDnFriendInfoDlg::SetFriendMemo( const wchar_t *wszMemo )
{
// 	m_pTextBoxMemo->ClearText();
// 	m_pTextBoxMemo->AddText( wszMemo );
}

void CDnFriendInfoDlg::SetFriendGroup( int nGroupID )
{
	m_pComboBoxGroup->RemoveAllItems();
	m_pComboBoxGroup->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1346/*No group*/ ), NULL, 0, true );

	int nSelectIndex(0);

	const CDnFriendTask::FRIENDGROUPINFO_VEC &friendGroupInfo = GetFriendTask().GetFriendGroupInfoList();
	for( int i=0; i<(int)friendGroupInfo.size(); ++i )
	{
		m_pComboBoxGroup->AddItem( friendGroupInfo[i].wszGroupName.c_str(), NULL, friendGroupInfo[i].nGroupDBID, true );

		if( friendGroupInfo[i].nGroupDBID == nGroupID )
		{
			nSelectIndex = i+1;
		}
	}

	m_pComboBoxGroup->SetSelectedByIndex( nSelectIndex );
}

bool CDnFriendInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
		else if( wParam == VK_RETURN )
		{
			UpdateFriendInfo();
			Show(false);
			return true;
		}
	}

	return CEtUIDialog::MsgProc(hWnd, uMsg, wParam, lParam);
}
