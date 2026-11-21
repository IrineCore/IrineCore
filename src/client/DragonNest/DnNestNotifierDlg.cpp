#include "StdAfx.h"
#include "DnNestNotifierDlg.h"
#include "DnNestInfoTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnNestNotifierListItemDlg
//-----------------------------------------------------------------------------
CDnNestNotifierListItemDlg::CDnNestNotifierListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticName( NULL )
	, m_pStaticCount( NULL )
{
}

CDnNestNotifierListItemDlg::~CDnNestNotifierListItemDlg()
{
}

void CDnNestNotifierListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyCreateInfoListDlg.ui" ).c_str(), bShow );
}

void CDnNestNotifierListItemDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
}

void CDnNestNotifierListItemDlg::SetNestInfo( const WCHAR *wszNestName, int nRemainCount, int nMaxCount )
{
	if( m_pStaticName && wszNestName )
		m_pStaticName->SetText( wszNestName );
	if( m_pStaticCount )
	{
		WCHAR wszBuf[64] = { 0 };
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d / %d", nRemainCount, nMaxCount );
		m_pStaticCount->SetText( wszBuf );
	}
}

//-----------------------------------------------------------------------------
// CDnNestNotifierDlg
//-----------------------------------------------------------------------------
CDnNestNotifierDlg::CDnNestNotifierDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pListBoxEx( NULL )
	, m_pStaticNotice( NULL )
{
}

CDnNestNotifierDlg::~CDnNestNotifierDlg()
{
	ClearNestList();
}

void CDnNestNotifierDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyCreateInfoDlg.ui" ).c_str(), bShow );
}

void CDnNestNotifierDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );
	m_pStaticNotice = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
}

void CDnNestNotifierDlg::ClearNestList()
{
	if( m_pListBoxEx )
		m_pListBoxEx->RemoveAllItems();
}

void CDnNestNotifierDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
		RefreshNestList();

	CDnCustomDlg::Show( bShow );
}

void CDnNestNotifierDlg::RefreshNestList()
{
	ClearNestList();
	if( !CDnNestInfoTask::IsActive() )
		return;

	CDnNestInfoTask &nestTask = CDnNestInfoTask::GetInstance();
	DWORD dwCount = nestTask.GetNestDungeonCount();
	for( DWORD i = 0; i < dwCount; ++i )
	{
		CDnNestInfoTask::NestDungeonInfoStruct *pInfo = nestTask.GetNestDungeonInfo( i );
		if( !pInfo )
			continue;
		int nRemain = pInfo->nMaxTryCount - pInfo->nTryCount;
		if( nRemain < 0 ) nRemain = 0;
		AddNestItem( pInfo->szMapName.c_str(), nRemain, pInfo->nMaxTryCount );
	}
}

void CDnNestNotifierDlg::AddNestItem( const WCHAR *wszNestName, int nRemainCount, int nMaxCount )
{
	if( !m_pListBoxEx )
		return;
	CDnNestNotifierListItemDlg *pItem = m_pListBoxEx->AddItem<CDnNestNotifierListItemDlg>();
	if( pItem )
		pItem->SetNestInfo( wszNestName, nRemainCount, nMaxCount );
}

void CDnNestNotifierDlg::SetNoticeText( const WCHAR *wszNotice )
{
	if( m_pStaticNotice && wszNotice )
		m_pStaticNotice->SetText( wszNotice );
}

void CDnNestNotifierDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
