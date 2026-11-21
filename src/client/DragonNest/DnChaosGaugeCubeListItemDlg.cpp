#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosGaugeCubeListItemDlg.h"
#include "DnItem.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChaosGaugeCubeListItemDlg::CDnChaosGaugeCubeListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pItem( NULL )
	, m_pItemSlotButton( NULL )
	, m_pStaticName( NULL )
	, m_pStaticCount( NULL )
	, m_nItemID( 0 )
	, m_nOwnedCount( 0 )
	, m_nGaugeValue( 1 )
{
}

CDnChaosGaugeCubeListItemDlg::~CDnChaosGaugeCubeListItemDlg()
{
	SAFE_DELETE( m_pItem );
}

void CDnChaosGaugeCubeListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MixGaugListDlg.ui" ).c_str(), bShow );
}

void CDnChaosGaugeCubeListItemDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
	if( !m_pItemSlotButton )
		m_pItemSlotButton = GetControl<CDnItemSlotButton>( "ID_BT_ITEM" );
}

void CDnChaosGaugeCubeListItemDlg::InitCustomControl( CEtUIControl *pControl )
{
	if( pControl && strcmp( pControl->GetControlName(), "ID_BT_ITEM" ) == 0 )
		m_pItemSlotButton = static_cast<CDnItemSlotButton*>( pControl );
}

void CDnChaosGaugeCubeListItemDlg::SetInfo( int nItemID, int nOwnedCount, int nGaugeValue )
{
	m_nItemID = nItemID;
	m_nOwnedCount = nOwnedCount;
	m_nGaugeValue = ( nGaugeValue > 0 ) ? nGaugeValue : 1;

	SAFE_DELETE( m_pItem );

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, ( nOwnedCount > 0 ) ? nOwnedCount : 1, itemInfo ) )
	{
		m_pItem = GetItemTask().CreateItem( itemInfo );
	}

	if( m_pItem )
	{
		if( m_pItemSlotButton )
		{
			m_pItemSlotButton->SetItem( m_pItem, ( nOwnedCount > 0 ) ? nOwnedCount : 1 );
			m_pItemSlotButton->SetRegist( nOwnedCount <= 0 );
		}
		if( m_pStaticName )
			m_pStaticName->SetText( m_pItem->GetName() );
	}

	if( m_pStaticCount )
	{
		WCHAR wszBuf[64] = { 0 };
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", m_nOwnedCount );
		m_pStaticCount->SetText( wszBuf );
	}
}

#endif // PRE_ADD_CHAOSCUBE
