#include "StdAfx.h"
#include "DnItemCompoundMixDlg.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnItemCompoundMixDlg::CDnItemCompoundMixDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pGold(NULL)
, m_pSilver(NULL)
, m_pBronze(NULL)
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
, m_pProb(NULL)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		m_pItem[i] = NULL;
		m_pItemSlotButton[i] = NULL;
	}
}

CDnItemCompoundMixDlg::~CDnItemCompoundMixDlg(void)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		SAFE_DELETE(m_pItem[i]);
	}
}

void CDnItemCompoundMixDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompoundMixDlg.ui" ).c_str(), bShow );
}

void CDnItemCompoundMixDlg::InitialUpdate()
{
	m_pGold = GetControl<CEtUIStatic>("ID_GOLD"); 
	m_pSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_dwColorGold = m_pGold->GetTextColor();
	m_dwColorSilver = m_pSilver->GetTextColor();
	m_dwColorBronze = m_pBronze->GetTextColor();

	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");

	m_pProb = GetControl<CEtUIStatic>("ID_PROBABILITY");

	char szName[32];
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		sprintf_s(szName, _countof(szName), "%s%d", "ID_ITEM_SLOT", i);
		m_pItemSlotButton[i] = GetControl<CDnItemSlotButton>(szName);
		m_pItemSlotButton[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
	}

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnItemCompoundMixDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_SmartMove.MoveCursor();
		EnableButton( true );
	}
	else
	{
		m_SmartMove.ReturnCursor();

		for( int i = 0; i < NUM_NEED_ITEM; ++i )
			m_pItemSlotButton[i]->ResetSlot();

		m_pGold->SetIntToText( 0 );
		m_pSilver->SetIntToText( 0 );
		m_pBronze->SetIntToText( 0 );
		m_pGold->SetTextColor( m_dwColorGold );
		m_pSilver->SetTextColor( m_dwColorSilver );
		m_pBronze->SetTextColor( m_dwColorBronze );
	}
}

void CDnItemCompoundMixDlg::SetPrice( int nPrice )
{
	int nGold = nPrice/10000;
	int nSilver = (nPrice%10000)/100;
	int nBronze = nPrice%100;

	m_pGold->SetIntToText( nGold );
	m_pSilver->SetIntToText( nSilver );
	m_pBronze->SetIntToText( nBronze );
	if( nPrice > GetItemTask().GetCoin() )
	{
		if( nPrice >= 10000 ) m_pGold->SetTextColor( 0xFFFF0000, true );
		if( nPrice >= 100 ) m_pSilver->SetTextColor( 0xFFFF0000, true );
		m_pBronze->SetTextColor( 0xFFFF0000, true );
	}
}


void CDnItemCompoundMixDlg::SetItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount )
{
	if( nItemID == 0 )
		return;

	SAFE_DELETE( m_pItem[nSlotIndex] );

	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
	m_pItem[nSlotIndex] = GetItemTask().CreateItem( itemInfo );
	// Set default quantity to 1 and render actual count separately.
	m_pItemSlotButton[nSlotIndex]->SetItem(m_pItem[nSlotIndex], CDnSlotButton::ITEM_ORIGINAL_COUNT);
	m_pItemSlotButton[nSlotIndex]->SetJewelCount( nNeedItemCount, nCurItemCount );

	if( nCurItemCount < nNeedItemCount )
		m_pItemSlotButton[nSlotIndex]->SetRegist( true );
	else
		m_pItemSlotButton[nSlotIndex]->SetRegist( false );
}

void CDnItemCompoundMixDlg::SetCompoundInfo( int *pItemID, int *pItemCount )
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		if( pItemID[i] > 0 && pItemCount[i] > 0 )
		{
			// Get required item ID and quantity,
			int nItemID = pItemID[i];
			int nItemCount = pItemCount[i];

			// Check item types and counts in inventory.
			int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( nItemID );
			SetItemSlot( i, nItemID, nItemCount, nCurItemCount );
		}
	}
}

void CDnItemCompoundMixDlg::SetProb( int nProb )
{
	if( nProb != 0 )
	{
		WCHAR wszProb[8];
		swprintf_s(wszProb, _countof(wszProb), L"%d%%", nProb );
		m_pProb->SetText( wszProb );
		m_pProb->Show( true );
	}
}

void CDnItemCompoundMixDlg::SetOK( bool bOK )
{
	if( bOK )
		m_pButtonOK->Enable( true );
	else
		m_pButtonOK->Enable( false );
}

void CDnItemCompoundMixDlg::EnableButton( bool bEnable )
{
	m_pButtonOK->Enable( bEnable );
	m_pButtonCancel->Enable( bEnable );
}