#include "Stdafx.h"

#ifdef PRE_ADD_AUTOUNPACK


#include "DnItemTask.h"
#include "ItemSendPacket.h"

#include "DnInterface.h"
#include "DnMainMenuDlg.h"

#include "DnInvenTabDlg.h"
#include "DnInvenDlg.h"
#include "DnInvenCashDlg.h"
#include "DnInvenSlotDlg.h"

#include "DnCharmItemKeyDlg.h"

#include "DnAutoUnPackDlg.h"

#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#include "DnCashShopTask.h"
#endif // PRE_ADD_INSTANT_CASH_BUY


CDnAutoUnPackDlg::CDnAutoUnPackDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor ),
m_bUnPackIng( false ),
m_pInvenTabDlg(NULL),
m_pResigtedItem(NULL),
m_ItemClassID(0),
m_pCtrItemSlot( NULL ),
m_pStaticKeyCount( NULL ),
m_invenType(0),
m_ItemSerial(0)
{	
}


void CDnAutoUnPackDlg::ReleaseDlg()
{
	m_bUnPackIng = false;
	m_ItemClassID = 0;
	m_pResigtedItem = NULL;
}


void CDnAutoUnPackDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("InvenBoxOpenDlg.ui").c_str(), bShow );
}


void CDnAutoUnPackDlg::InitialUpdate()
{
	m_pCtrItemSlot = GetControl<CDnItemSlotButton>( "ID_ITEM" );
	m_pStaticKeyCount = GetControl<CEtUIStatic>( "ID_TEXT_KEY" );
	m_pStaticKeyCount->SetText( std::wstring(L"") );
}


void CDnAutoUnPackDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
#ifdef PRE_ADD_INSTANT_CASH_BUY
		int nShowType = GetCashShopTask().GetInstantCashBuyEnableType( INSTANT_BUY_AUTOUNPACK );
		if( nShowType == INSTANT_BUY_HIDE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( false );
		}
		else if( nShowType == INSTANT_BUY_SHOW )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( true );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
		else if( nShowType == INSTANT_BUY_DISABLE )
		{
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Show( true );
			GetControl<CEtUIButton>( "UI_BUTTON_BUYSHORTCUT" )->Enable( false );
			GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" )->Show( true );
		}
#endif // PRE_ADD_INSTANT_CASH_BUY	
	}
	else
	{
		EngUnpack();
		SendAutoUnpackClose();
	}	
}


void CDnAutoUnPackDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK") )
		{
			// Check if inventory slots remain.
			int needEmptyCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::CharmItemEmptySlotCount);
			
			// Check normal inventory tab count.
			int nCurEmptySlotCount = 0;
			CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			if( pInvenDlg ) 
			{
				nCurEmptySlotCount = pInvenDlg->GetEmptySlotCount();

				// Insufficient inventory slots.
				if( nCurEmptySlotCount < needEmptyCount )
				{
					wchar_t strMsg[256] = {0,};
					swprintf_s( strMsg, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7052 ), needEmptyCount );
					GetInterface().MessageBox( strMsg, MB_OK );	
				}
				else
				{
					m_bUnPackIng = true;

					// Send open packet.
					if( m_pResigtedItem && m_ItemSerial !=0 )
					{								
						GetItemTask().RequestAutoUnpackItem( m_ItemSerial, true );
					}
					/*else
					{
					//.. Please place an item.
					}*/				
				}
			}
		}
#ifdef PRE_ADD_INSTANT_CASH_BUY
		else if( IsCmdControl( "UI_BUTTON_BUYSHORTCUT" ) )
		{
			GetInterface().ShowInstantCashShopBuyDlg( true, INSTANT_BUY_AUTOUNPACK, this );
		}
#endif // PRE_ADD_INSTANT_CASH_BUY
		if( IsCmdControl("ID_CANCEL") || IsCmdControl("ID_CLOSE") )
		{
			m_bUnPackIng = false;
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


// Register item for auto-open.
// : Find and register all items matching the argument in the current inventory tab.
bool CDnAutoUnPackDlg::RegisterItem( CDnItem * pItem )
{	 
	// Check item type.
	if( pItem && pItem->GetItemType() != ITEMTYPE_CHARM )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8051 ), MB_OK );
		return false;
	}

	bool bResult = true;

	m_pResigtedItem = NULL; // Registered item for auto-open.

	// All SlotDlgs belonging to one inventory tab - must find the argument item across all SlotDlgs.
	if( m_pInvenTabDlg == NULL )
		m_pInvenTabDlg = static_cast<CDnInvenTabDlg * >( GetInterface().GetMainMenuDialog()->GetMainDialog( CDnMainMenuDlg::INVENTORY_DIALOG ) );
	
	std::vector< CDnInvenSlotDlg * > vSlotDlg;
	CDnInvenTabDlg::eInvenTabType invenType = m_pInvenTabDlg->GetEnableInvenType();

	// Store the inventory type where the item is located.
	if( pItem )
		m_invenType = (int)invenType; 
	
	// Keep opening previously registered item.
	else
		invenType = (CDnInvenTabDlg::eInvenTabType)m_invenType;
	

	// [Cash Tab] uses a different class.
	if( invenType == CDnInvenTabDlg::eInvenTabType::eCASH ) 
	{
		CDnInvenCashDlg * pInvenCashDlg = m_pInvenTabDlg->GetCashInvenDlg();		
		vSlotDlg = pInvenCashDlg->GetInvenSlotDlgList();
	}

	// [Tabs other than Cash Tab].
	else 
	{
		// Matched tab types because the tab type used in InvenTabDlg differs from the one used in InvenDlg.
		ITEM_SLOT_TYPE invenDlgSlotType = ITEM_SLOT_TYPE::ST_ITEM_NONE;
		switch( invenType )
		{
		case CDnInvenTabDlg::eInvenTabType::eCOMMON :	
			invenDlgSlotType = ITEM_SLOT_TYPE::ST_INVENTORY; break;

		case CDnInvenTabDlg::eInvenTabType::eQUEST :	
			invenDlgSlotType = ITEM_SLOT_TYPE::ST_INVENTORY_QUEST; break;
		}

		CDnInvenDlg * pInvenDlg = m_pInvenTabDlg->GetInvenDlg( invenDlgSlotType );
		vSlotDlg = pInvenDlg->GetInvenSlotDlgList();		
	}
	
	if( !vSlotDlg.empty() )
	{
		int nTotCount = 0; // Total count of the item.
		CDnInvenSlotDlg * pSlotDlg = NULL;
		int size = (int)vSlotDlg.size();
		for( int i=0; i<size; ++i )
		{
			pSlotDlg = vSlotDlg[ i ];
			if( pSlotDlg == NULL )
				continue;

			// When registering an item, find and register all identical items in the slot window matching that slot button's item.
			const std::vector< CDnQuickSlotButton* > & vSlotButtons = pSlotDlg->GetSlotButtons();
			CDnQuickSlotButton * pSlotBtn = NULL;
			CDnItem * pSlotItem = NULL;

			int nItemID = pItem ? pItem->GetClassID() : m_ItemClassID;
			size_t size = vSlotButtons.size();
			for( size_t i=0; i<size; ++i )
			{
				pSlotBtn = vSlotButtons.at( i );
				if( !pSlotBtn || !pSlotBtn->GetItem() )
					continue;

				pSlotItem = static_cast< CDnItem * >( pSlotBtn->GetItem() );
				if( nItemID == pSlotItem->GetClassID() )
				{
					// pItem is non-null when selected from an inventory slot, and NULL when auto-selected by auto-open. 
					// When NULL, set to an existing item in inventory slots so the cash serialnumber is present.
					if( pItem == NULL )
						pItem = pSlotItem;

					nTotCount += pSlotItem->GetOverlapCount(); // Total count of identical items in the slot.
				}
			}
		}
		
		// Set item on SlotButton.
		if( nTotCount > 0 )
		{
			m_pResigtedItem = pItem;
			m_ItemClassID = pItem->GetClassID();
			m_pCtrItemSlot->SetItem( pItem, nTotCount );	

			// Display key count when a key is required.
			SetKeyItemCount( pItem );

			bResult = true;
		}	

		// All consumed, stop auto-open.
		else
		{
			bResult = false;
		}
		
	}

	return bResult;
}


// Start auto-open.
void CDnAutoUnPackDlg::UnPacking()
{	
	if( m_pResigtedItem )
		GetInterface().ShowCharmItemKeyDialog( true, m_pResigtedItem );
}


// Open request result (received before actual open)
void CDnAutoUnPackDlg::OnRecvRequestCharmItem( char cInvenType, short sInvenIndex, INT64 biInvenSerial )
{
	GetInterface().GetCharmItemKeyDlg()->OnRecvRequestCharmItem( cInvenType, sInvenIndex, biInvenSerial );
}

// Restore previous state.
void CDnAutoUnPackDlg::PreStateRestore()
{
	if( m_ItemClassID == 0 )
		return;

	// Send pItem as NULL when auto-selected during auto-open.
	// When NULL, find and set the item matching m_ItemClassID from inventory slots. Needed to pass server integrity check.
	m_pResigtedItem = NULL;

	// Refresh ItemSlotButton.
	RegisterItem( m_pResigtedItem );	
}

void CDnAutoUnPackDlg::ReadyUnPack()
{
	if( m_ItemClassID == 0 )
		return;

	// Send pItem as NULL when auto-selected during auto-open.
	// When NULL, find and set the item matching m_ItemClassID from inventory slots. Needed to pass server integrity check.
	m_pResigtedItem = NULL;

	// Refresh ItemSlotButton.
	if( RegisterItem( m_pResigtedItem ) )
	{
		// Start auto-open.
		UnPacking();
	}

	// Stop auto-open.
	else
	{
		Show( false );
	}
}

// Set KeyItem count.
void CDnAutoUnPackDlg::SetKeyItemCount( CDnItem * pItem )
{
	if( !pItem ) 
		return;
	if( !CDnItemTask::IsActive() ) 
		return;
	
	int nTypeParam = pItem->GetTypeParam( 1 );

	// Key required.
	if( nTypeParam == 1 || nTypeParam == 2 )
	{
		
		std::vector<CDnItem*> vecKeyItem;
		std::vector<int> vecKeyID;
		pItem->GetCharmItemKeyID( vecKeyID );

		
		for( int i = 0; i < (int)vecKeyID.size(); ++i )
		{
			GetItemTask().GetCharInventory().ScanItemFromID( vecKeyID[i], &vecKeyItem );
			GetItemTask().GetCashInventory().ScanItemFromID( vecKeyID[i], &vecKeyItem );
		}

		// Set key count.		
		wchar_t buf[64] = {0,};
		int nKeyCnt = 0;
		if( !vecKeyItem.empty() )
		{
			int size = (int)vecKeyItem.size();
			for( int i=0; i<size; ++i )
				nKeyCnt += vecKeyItem[ i ]->GetOverlapCount();
		}			
		swprintf_s( buf, 64, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4874), nKeyCnt );
		m_pStaticKeyCount->SetText( std::wstring(buf) );

		vecKeyItem.clear();
		vecKeyID.clear();
	}

	// Key not required.
	else
	{
		m_pStaticKeyCount->SetText( std::wstring(L"") );
	}

}


// Set open delay time % per item.
void CDnAutoUnPackDlg::SetUnPackTime( int nTime )
{	
	if( nTime <= 0 )
		nTime = 100;

	m_fUnPackTimePerOwn = (float)nTime / 100.0f;
}


// Stop auto-open.
void CDnAutoUnPackDlg::EngUnpack()
{
	m_ItemSerial = 0;
	m_invenType = 0;
	m_bUnPackIng = false;
	m_ItemClassID = 0;
	m_pResigtedItem = NULL;
	m_pCtrItemSlot->ResetSlot();
	m_pStaticKeyCount->SetText( std::wstring(L"") );
}

#ifdef PRE_ADD_INSTANT_CASH_BUY
void CDnAutoUnPackDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	if( nID == INSTANT_CASHSHOP_BUY_DIALOG )
	{
		CDnItem* pItem = static_cast<CDnItem*>( m_pCtrItemSlot->GetItem() );
		if( pItem )
			SetKeyItemCount( pItem );
	}
}
#endif // PRE_ADD_INSTANT_CASH_BUY

#endif