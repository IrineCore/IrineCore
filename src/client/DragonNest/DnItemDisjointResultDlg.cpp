#include "StdAfx.h"
#include "DnItemDisjointResultDlg.h"
#include "DnItemTask.h"
#include "DnItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//////////////////////////////////////////////////////////////////////////
// CDnItemDisjointResultListItemDlg

CDnItemDisjointResultListItemDlg::CDnItemDisjointResultListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pItem( NULL )
, m_pSlotButton( NULL )
, m_pStaticName( NULL )
, m_pStaticCount( NULL )
{
}

CDnItemDisjointResultListItemDlg::~CDnItemDisjointResultListItemDlg()
{
	SAFE_DELETE( m_pItem );
}

void CDnItemDisjointResultListItemDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemDisjointListDlg.ui" ).c_str(), bShow );
}

void CDnItemDisjointResultListItemDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
	m_pSlotButton = GetControl<CDnItemSlotButton>( "ID_BT_ITEM" );
	if( m_pSlotButton )
		m_pSlotButton->Show( false );
}

void CDnItemDisjointResultListItemDlg::SetResultItem( int nItemID, int nCount )
{
	SAFE_DELETE( m_pItem );
	if( nItemID <= 0 || !CDnItemTask::IsActive() )
		return;

	TItemInfo itemInfo;
	if( !CDnItem::MakeItemInfo( nItemID, max( 1, nCount ), itemInfo ) )
		return;

	m_pItem = GetItemTask().CreateItem( itemInfo );
	if( !m_pItem ) return;

	if( m_pSlotButton )
	{
		m_pSlotButton->SetItem( m_pItem, max( 1, nCount ) );
		m_pSlotButton->Show( true );
	}
	if( m_pStaticName )
		m_pStaticName->SetText( m_pItem->GetName() );
	if( m_pStaticCount )
	{
		wchar_t wszCount[32] = {0,};
		swprintf_s( wszCount, _countof(wszCount), L"%d", max( 1, nCount ) );
		m_pStaticCount->SetText( wszCount );
	}
}

void CDnItemDisjointResultListItemDlg::SetResultItem( const TItem &item )
{
	SetResultItem( item.nItemID, item.wCount );
}

//////////////////////////////////////////////////////////////////////////
// CDnItemDisjointResultDlg

CDnItemDisjointResultDlg::CDnItemDisjointResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pListBoxEx( NULL )
{
}

CDnItemDisjointResultDlg::~CDnItemDisjointResultDlg()
{
	ClearResultList();
}

void CDnItemDisjointResultDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemDisjointResultDlg.ui" ).c_str(), bShow );
}

void CDnItemDisjointResultDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );
}

void CDnItemDisjointResultDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
		ClearResultList();

	CDnCustomDlg::Show( bShow );
}

void CDnItemDisjointResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnItemDisjointResultDlg::ClearResultList()
{
	if( m_pListBoxEx )
		m_pListBoxEx->RemoveAllItems();
}

void CDnItemDisjointResultDlg::AddResultItem( int nItemID, int nCount )
{
	if( !m_pListBoxEx || nItemID <= 0 )
		return;

	CDnItemDisjointResultListItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnItemDisjointResultListItemDlg>();
	if( pItemDlg )
		pItemDlg->SetResultItem( nItemID, nCount );
}

void CDnItemDisjointResultDlg::SetResultItems( const std::vector<TItem> &vecItems )
{
	ClearResultList();
	if( !m_pListBoxEx ) return;

	for( size_t i = 0; i < vecItems.size(); ++i )
	{
		if( vecItems[i].nItemID > 0 )
			AddResultItem( vecItems[i].nItemID, vecItems[i].wCount );
	}
}
