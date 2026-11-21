#include "StdAfx.h"

#if defined( PRE_ADD_DWC )

#include "DnDwcDlg.h"
#include "DnDWCTask.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnDwcMemberListDlg
//-----------------------------------------------------------------------------
CDnDwcMemberListDlg::CDnDwcMemberListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticTitle( NULL )
	, m_pStaticName( NULL )
	, m_pStaticClass( NULL )
	, m_pStaticVillage( NULL )
	, m_pStaticLv( NULL )
{
}

CDnDwcMemberListDlg::~CDnDwcMemberListDlg()
{
}

void CDnDwcMemberListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PVPListDlg.ui" ).c_str(), bShow );
}

void CDnDwcMemberListDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticClass = GetControl<CEtUIStatic>( "ID_TEXT_CLASS" );
	m_pStaticVillage = GetControl<CEtUIStatic>( "ID_TEXT_VILLAGE" );
	m_pStaticLv = GetControl<CEtUIStatic>( "ID_TEXT_LV" );
}

void CDnDwcMemberListDlg::SetMemberInfo( const TDWCTeamMemberInfo &info )
{
	if( m_pStaticTitle )
	{
		const WCHAR *wszRole = info.bTeamLeader
			? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120214 )
			: GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120215 );
		m_pStaticTitle->SetText( wszRole ? wszRole : L"" );
	}

	if( m_pStaticName )
		m_pStaticName->SetText( info.wszCharacterName );

	if( m_pStaticClass )
		m_pStaticClass->SetText( DN_INTERFACE::STRING::GetJobString( info.cJobCode ) );

	if( m_pStaticLv )
	{
		WCHAR wszLv[32] = { 0 };
		swprintf_s( wszLv, _countof( wszLv ), L"%d", (int)info.cLevel );
		m_pStaticLv->SetText( wszLv );
	}

	if( m_pStaticVillage )
	{
		if( info.cConnectState == 0 )
		{
			m_pStaticVillage->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3514 ) );
		}
		else
		{
			DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
			if( pMapSox && info.nMapIndex > 0 && pMapSox->IsExistItem( info.nMapIndex ) )
			{
				int nMapNameID = pMapSox->GetFieldFromLablePtr( info.nMapIndex, "_MapNameID" )->GetInteger();
				m_pStaticVillage->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMapNameID ) );
			}
			else
			{
				m_pStaticVillage->SetText( L"" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// CDnDwcCreateTeamDlg
//-----------------------------------------------------------------------------
CDnDwcCreateTeamDlg::CDnDwcCreateTeamDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pEditTeamName( NULL )
	, m_pBtnOK( NULL )
{
}

CDnDwcCreateTeamDlg::~CDnDwcCreateTeamDlg()
{
}

void CDnDwcCreateTeamDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PVPTeamCreateDlg.ui" ).c_str(), bShow );
}

void CDnDwcCreateTeamDlg::InitialUpdate()
{
	m_pEditTeamName = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
	m_pBtnOK = GetControl<CEtUIButton>( "ID_BT_OK" );
	if( m_pBtnOK )
		m_pBtnOK->Enable( false );
}

void CDnDwcCreateTeamDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pEditTeamName )
		{
			m_pEditTeamName->ClearText();
			RequestFocus( m_pEditTeamName );
		}
		if( m_pBtnOK )
			m_pBtnOK->Enable( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnDwcCreateTeamDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_EDITBOX_CHANGE || nCommand == EVENT_EDITBOX_IME_CHANGE )
	{
		if( IsCmdControl( "ID_IMEEDITBOX_NAME" ) && m_pEditTeamName && m_pBtnOK )
		{
			std::wstring strName = m_pEditTeamName->GetText();
			m_pBtnOK->Enable( !strName.empty() );
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_pEditTeamName && CDnDWCTask::IsActive() )
			{
				std::wstring strName = m_pEditTeamName->GetText();
				if( !strName.empty() )
					GetDWCTask().RequestCreateTeam( strName.c_str() );
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnDwcAddMemberDlg
//-----------------------------------------------------------------------------
CDnDwcAddMemberDlg::CDnDwcAddMemberDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pEditName( NULL )
	, m_pComboName( NULL )
	, m_pBtnOK( NULL )
{
}

CDnDwcAddMemberDlg::~CDnDwcAddMemberDlg()
{
}

void CDnDwcAddMemberDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PVPTeamAddDlg.ui" ).c_str(), bShow );
}

void CDnDwcAddMemberDlg::InitialUpdate()
{
	m_pEditName = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
	m_pBtnOK = GetControl<CEtUIButton>( "ID_BT_OK" );
	m_pComboName = GetControl<CEtUIComboBox>( "ID_COMBOBOX_NAME" );
	if( m_pComboName )
		m_pComboName->Show( false );
}

void CDnDwcAddMemberDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow && m_pEditName )
	{
		m_pEditName->ClearText();
		RequestFocus( m_pEditName );
	}

	CEtUIDialog::Show( bShow );
}

void CDnDwcAddMemberDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_pEditName && CDnDWCTask::IsActive() )
			{
				std::wstring strName = m_pEditName->GetText();
				if( !strName.empty() )
				{
					GetDWCTask().RequestInviteMember( strName.c_str() );
					Show( false );
				}
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnDwcInviteReqDlg
//-----------------------------------------------------------------------------
CDnDwcInviteReqDlg::CDnDwcInviteReqDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pProgressTime( NULL )
	, m_pStaticTitleName( NULL )
	, m_pStaticName( NULL )
	, m_pBtnOK( NULL )
	, m_pBtnCancel( NULL )
	, m_nTeamID( 0 )
	, m_nFromSessionID( 0 )
	, m_fTotalTime( 15.0f )
	, m_fRemainTime( 0.0f )
{
}

CDnDwcInviteReqDlg::~CDnDwcInviteReqDlg()
{
}

void CDnDwcInviteReqDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AcceptProcessBarRequestDlg.ui" ).c_str(), bShow );
}

void CDnDwcInviteReqDlg::InitialUpdate()
{
	m_pProgressTime = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR_TIME" );
	m_pStaticTitleName = GetControl<CEtUIStatic>( "ID_TEXT_TITLE_NAME" );
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pBtnOK = GetControl<CEtUIButton>( "ID_OK" );
	m_pBtnCancel = GetControl<CEtUIButton>( "ID_CANCEL" );

	CEtUIStatic *pTitle = GetControl<CEtUIStatic>( "ID_TITLE" );
	if( pTitle )
		pTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120226 ) );

	CEtUIStatic *pText1 = GetControl<CEtUIStatic>( "ID_TEXT1" );
	if( pText1 )
		pText1->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120230 ) );
}

void CDnDwcInviteReqDlg::SetInviteInfo( UINT nTeamID, UINT nFromSessionID, const WCHAR *wszTeamName, const WCHAR *wszFromCharName, float fTimeoutSec )
{
	m_nTeamID = nTeamID;
	m_nFromSessionID = nFromSessionID;
	m_fTotalTime = ( fTimeoutSec > 0.0f ) ? fTimeoutSec : 15.0f;
	m_fRemainTime = m_fTotalTime;

	if( m_pStaticTitleName && wszTeamName )
		m_pStaticTitleName->SetText( wszTeamName );
	if( m_pStaticName && wszFromCharName )
		m_pStaticName->SetText( wszFromCharName );
	if( m_pProgressTime )
		m_pProgressTime->SetProgress( 100.0f );
}

void CDnDwcInviteReqDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
}

void CDnDwcInviteReqDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	m_fRemainTime -= fElapsedTime;
	if( m_fRemainTime <= 0.0f )
	{
		m_fRemainTime = 0.0f;
		if( CDnDWCTask::IsActive() )
			GetDWCTask().RequestInviteMemberAck( false );
		Show( false );
		return;
	}

	if( m_pProgressTime && m_fTotalTime > 0.0f )
		m_pProgressTime->SetProgress( ( m_fRemainTime / m_fTotalTime ) * 100.0f );
}

void CDnDwcInviteReqDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			if( CDnDWCTask::IsActive() )
				GetDWCTask().RequestInviteMemberAck( true );
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_CANCEL" ) || IsCmdControl( "ID_BUTTON_CLOSE" ) )
		{
			if( CDnDWCTask::IsActive() )
				GetDWCTask().RequestInviteMemberAck( false );
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnDwcDlg
//-----------------------------------------------------------------------------
CDnDwcDlg::CDnDwcDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnHelp( NULL )
	, m_pListBoxMembers( NULL )
	, m_pBtnInvite( NULL )
	, m_pBtnQuit( NULL )
	, m_pStaticTeamName( NULL )
	, m_pStaticRank( NULL )
	, m_pStaticResult( NULL )
	, m_pStaticCount( NULL )
	, m_pAddMemberDlg( NULL )
{
}

CDnDwcDlg::~CDnDwcDlg()
{
	if( m_pListBoxMembers )
		m_pListBoxMembers->RemoveAllItems();
	SAFE_DELETE( m_pAddMemberDlg );
}

void CDnDwcDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PVPDlg.ui" ).c_str(), bShow );

	m_pAddMemberDlg = new CDnDwcAddMemberDlg( UI_TYPE_MODAL );
	if( m_pAddMemberDlg )
		m_pAddMemberDlg->Initialize( false );
}

void CDnDwcDlg::InitialUpdate()
{
	m_pBtnHelp = GetControl<CEtUIButton>( "ID_BT_SMALLHELP" );
	if( m_pBtnHelp )
		m_pBtnHelp->Enable( false );

	m_pListBoxMembers = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );
	m_pBtnInvite = GetControl<CEtUIButton>( "ID_BT_INVITE" );
	m_pBtnQuit = GetControl<CEtUIButton>( "ID_BT_QUIT" );

	CEtUIStatic *pText0 = GetControl<CEtUIStatic>( "ID_TEXT0" );
	if( pText0 ) pText0->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126095 ) );

	CEtUIStatic *pText1 = GetControl<CEtUIStatic>( "ID_TEXT1" );
	if( pText1 ) pText1->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2331 ) );

	CEtUIStatic *pText2 = GetControl<CEtUIStatic>( "ID_TEXT2" );
	if( pText2 ) pText2->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126020 ) );

	m_pStaticTeamName = GetControl<CEtUIStatic>( "ID_TEXT_TEAMNAME" );
	m_pStaticRank = GetControl<CEtUIStatic>( "ID_TEXT_RANK" );
	m_pStaticResult = GetControl<CEtUIStatic>( "ID_TEXT_RESULT" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
}

void CDnDwcDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( CDnDWCTask::IsActive() )
			GetDWCTask().RequestGetTeamInfo( true );
		RefreshTeamInfo();
	}

	CEtUIDialog::Show( bShow );
}

void CDnDwcDlg::RefreshTeamInfo()
{
	if( !CDnDWCTask::IsActive() )
		return;

	const TDWCTeamInfo &team = GetDWCTask().GetTeamInfo();
	const std::vector<TDWCTeamMemberInfo> &members = GetDWCTask().GetTeamMemberList();

	if( m_pStaticTeamName )
		m_pStaticTeamName->SetText( team.wszTeamName );

	WCHAR wszBuf[128] = { 0 };
	if( m_pStaticRank )
	{
		if( team.nRank > 0 )
			swprintf_s( wszBuf, _countof( wszBuf ), L"%d", team.nRank );
		else
			swprintf_s( wszBuf, _countof( wszBuf ), L"-" );
		m_pStaticRank->SetText( wszBuf );
	}

	if( m_pStaticResult )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d / %d / %d (%d)", team.nTotalWin, team.nTotalLose, team.nTotalDraw, team.nDWCPoint );
		m_pStaticResult->SetText( wszBuf );
	}

	if( m_pStaticCount )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", team.nWeeklyPlayCount );
		m_pStaticCount->SetText( wszBuf );
	}

	if( m_pListBoxMembers )
	{
		m_pListBoxMembers->RemoveAllItems();
		for( size_t i = 0; i < members.size(); ++i )
		{
			CDnDwcMemberListDlg *pRow = m_pListBoxMembers->AddItem<CDnDwcMemberListDlg>();
			if( pRow )
				pRow->SetMemberInfo( members[i] );
		}
	}

	bool bHasTeam = GetDWCTask().HasDWCTeam();
	if( m_pBtnInvite )
		m_pBtnInvite->Enable( bHasTeam && GetDWCTask().IsTeamLeader() && (int)members.size() < DWC::MAX_TEAM_MEMBER );
	if( m_pBtnQuit )
		m_pBtnQuit->Enable( bHasTeam );
}

void CDnDwcDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_INVITE" ) )
		{
			if( m_pAddMemberDlg )
				m_pAddMemberDlg->Show( true );
			return;
		}
		else if( IsCmdControl( "ID_BT_QUIT" ) )
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120233 ), MB_YESNO, MESSAGEBOX_LEAVE_TEAM, this );
			return;
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDwcDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nID == MESSAGEBOX_LEAVE_TEAM && nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_YES" ) && CDnDWCTask::IsActive() )
			GetDWCTask().RequestLeaveTeam();
	}
}

#endif // PRE_ADD_DWC
