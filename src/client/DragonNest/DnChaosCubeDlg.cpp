#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DNPacket.h"
#include "DnMainFrame.h"
#include "DnItemTask.h"
#include "DnCharInventory.h"
#include "DnCashInventory.h"
#include "DnPetInventory.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnChaosCubeResultDlg.h"
#include "DnChaosCubeProgressDlg.h"
#include "DnChaosCubeStuffDlg.h"
#include "DnChaosCubeDlg.h"


// Construct.
CDnChaosCubeDlg::CDnChaosCubeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor ),
//m_pChaosCubeProgressDlg(NULL),
//m_pChaosCubeResultDlg(NULL),
//m_pChildDlg(NULL),
m_pItemChaos(NULL), // Chaos item.
m_CountPerPage(5), // Number of slots per page.
m_crrPage(0), // Current page.
m_maxPage(0), // Total pages.
m_signLight(1), // Sign for selected material slot glow effect.
m_fAlphaColor(1.0f),
m_bSelfItemList(true), // Whether the same item as the material item is loaded.
m_SelectedSlotIndex(-1) // Selected material slot index.
{
	m_pSlotBtnSelected = NULL; // Selected material slot.
	m_pStaticLight = NULL; // Sparkle.
	m_pStaticTitle = NULL; // Title bar.
	m_pStaticHelp = NULL;  // Help.
	m_pStaticPage = NULL;  // Page/MaxPage.

	m_pBtnPrev = NULL;     // Previous.
	m_pBtnNext = NULL;     // Next.
	m_pBtnMaterial = NULL; // Material.
	m_pBtnProduct = NULL;  // Result.
	m_pBtnConfirm = NULL;  // Confirm.
}


// ReleaseDlg()
void CDnChaosCubeDlg::ReleaseDlg()
{
	m_vSlotBtnMaterials.clear(); // Material slots.
	m_vStaticSelect.clear(); // Material slot selection Static.
	m_vNeedStuffCount.clear(); // Number of materials required for mix.
	m_vStuffItemIDs.clear(); // Material item.
	
//	SAFE_DELETE( m_pChildDlg );
//	SAFE_DELETE( m_pChaosCubeProgressDlg );
//	SAFE_DELETE( m_pChaosCubeResultDlg );
}


// Initialize()
void CDnChaosCubeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("MixBoxDlg.ui").c_str(), bShow );
}


// InitialUpdate()
void CDnChaosCubeDlg::InitialUpdate()
{
	//// Dialog - Material Dlg or Result.
	//m_pChildDlg = new CDnChaosCubeStuffDlg( UI_TYPE_CHILD_MODAL, this );  
	//m_pChildDlg->Initialize( false );

	//// Dialog - Progress window.	
	//m_pChaosCubeProgressDlg = new CDnChaosCubeProgressDlg( UI_TYPE_CHILD_MODAL, this );
	//m_pChaosCubeProgressDlg->Initialize( false );

	//// Dialog - Acquired result.
	//m_pChaosCubeResultDlg = new CDnChaosCubeResultDlg( UI_TYPE_CHILD_MODAL, this );
	//m_pChaosCubeResultDlg->Initialize( false );

	m_pSlotBtnSelected = GetControl<CDnItemSlotButton>("ID_ITEM0"); // Selected material slot.
	m_pStaticLight = GetControl<CEtUIStatic>("ID_STATIC_LIGHT");    // Sparkle.

	m_pStaticPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE"); // Page/MaxPage.

	// Material slots.
	char buf[64] = {0,};
	m_vSlotBtnMaterials.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_ITEM%d", i+1 );
		m_vSlotBtnMaterials[ i ] = GetControl<CDnItemSlotButton>( buf );
	}

	// Material slot selection Static.
	m_vStaticSelect.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_SELECT%d", i+1 );
		m_vStaticSelect[ i ] = GetControl<CEtUIStatic>( buf );
	}	
	
	m_pBtnMaterial = GetControl<CEtUIButton>("ID_BT_ITEMLIST"); // Material.
	m_pBtnProduct = GetControl<CEtUIButton>("ID_BT_RESULT");    // Result.


	m_pStaticTitle = GetControl<CEtUIStatic>("ID_TEXT_TITLE"); // Title.
	m_pStaticHelp = GetControl<CEtUIStatic>("ID_TEXT_ASK"); // Ask. 

	m_pBtnConfirm = GetControl<CEtUIButton>("ID_BT_OK"); // Confirm.
}


// Show()
void CDnChaosCubeDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	// Open.
	if( bShow )
	{
		m_pBtnConfirm->Enable( false ); // Disable Confirm button.
		OpenDlg();
	}
	
	// Close.
	else
	{
		// Clean up when Dlg closes.
		CloseDlg();		
	}
}


void CDnChaosCubeDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	// Play glow animation if a selected material item exists.
	if( m_pSlotBtnSelected->GetItem() )
	{		
		D3DXCOLOR color( m_pStaticLight->GetTextureColor() );
		
		if( m_fAlphaColor <= 0.3f )
			m_signLight = 1;
		if( m_fAlphaColor >= 1.0f )
			m_signLight = -1;

		m_fAlphaColor += fElapsedTime * m_signLight;
		color.a = m_fAlphaColor;
		
		m_pStaticLight->SetTextureColor( color );
	}

}


bool CDnChaosCubeDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_MOUSEWHEEL:
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			ScreenToClient( hWnd, &MousePoint );
			lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

			if( IsMouseInDlg() )
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
				if( nScrollAmount > 0 )
				{
					if( m_crrPage > 0 )
						ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_PRIOR"), 0 );
				}
				else if( nScrollAmount < 0 )
				{
					if( m_crrPage < m_maxPage )
						ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT"), 0 );
				}
				CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );

				return true;
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}


// ProcessCommand()
void CDnChaosCubeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
			
		if( strstr( pControl->GetControlName(), "ID_ITEM" ) )
		{
			// Select material item.
			SelectionStuffItem( pControl->GetControlName() );			
		}

		// Previous.
		else if( IsCmdControl("ID_BT_PRIOR") )
		{
			--m_crrPage;
			if( m_crrPage < 0 )
				m_crrPage = m_maxPage;

			// Refresh material slot.
			RefreshStuffItemSlot();
		}

		// Next.
		else if( IsCmdControl("ID_BT_NEXT") )
		{
			++m_crrPage;
			if( m_crrPage > m_maxPage )
				m_crrPage = 0;

			// Refresh material slot.
			RefreshStuffItemSlot();
		}

		// Open material window.
		else if( IsCmdControl("ID_BT_ITEMLIST") )
		{
			//if( m_pSlotBtnSelected->GetItem() )
			//{
			//	m_pChildDlg->SetDlgMode( EChildDlgMode::STUFF, m_pSlotBtnSelected->GetItem()->GetClassID(), m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
			//	m_pChildDlg->Show( true );			
			//}
//			m_pChildDlg->SetDlgMode( EChildDlgMode::STUFF, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
//			m_pChildDlg->Show( true );			
			GetInterface().GetChaosCubeStuffDlg()->SetDlgMode( EChildDlgMode::STUFF, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
			GetInterface().GetChaosCubeStuffDlg()->Show( true );
		}
	
		// Open result window.
		else if( IsCmdControl("ID_BT_RESULT") )
		{
			//if( m_pSlotBtnSelected->GetItem() )
			//{
			//	m_pChildDlg->SetDlgMode( EChildDlgMode::PRODUCT, m_pSlotBtnSelected->GetItem()->GetClassID(), m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
			//	m_pChildDlg->Show( true );			
			//}
			//m_pChildDlg->SetDlgMode( EChildDlgMode::PRODUCT, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
			//m_pChildDlg->Show( true );
			GetInterface().GetChaosCubeStuffDlg()->SetDlgMode( EChildDlgMode::PRODUCT, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
			GetInterface().GetChaosCubeStuffDlg()->Show( true );
		}

		// Close.
		else if( IsCmdControl("ID_BT_CLOSE") || IsCmdControl("ID_BT_CANCEL") )
			Show( false );

		// Start mix.
		else if( IsCmdControl("ID_BT_OK") )
		{
			// Check if inventory slots remain.
			int needEmptyCount = (int)CGlobalWeightIntTable::GetInstance().GetValue(CGlobalWeightIntTable::ChaosCubeEmptySlotCount);

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
					GetInterface().MessageBox( 7032, MB_YESNO, 1, this ); // "The selected material will be consumed and a new item will be obtained. Really operate the Chaos Cube?"			
				}
			}					
		}
		
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnChaosCubeDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nID == 1 )
	{
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( IsCmdControl( "ID_YES" ) )
			{
				m_pBtnConfirm->Enable( false );
				MixStart(); // Start mix.
			}
		}
	}

}


// Clean up when Dlg closes.
void CDnChaosCubeDlg::CloseDlg()
{
	m_crrPage = 0;

	// Initialize material selection slot.
	m_pSlotBtnSelected->ResetSlot();
	
	// Initialize material slot.
	int size = (int)m_vSlotBtnMaterials.size();
	for( int i=0; i<size; ++i )
		m_vSlotBtnMaterials[ i ]->ResetSlot();

	// Material items searched from inventory.
	std::map< int, std::vector< CDnItem * > >::iterator it = m_mapStuffItems.begin();
	for( ; it != m_mapStuffItems.end(); ++it )
		(*it).second.clear();
	m_mapStuffItems.clear();	


	// Hide - Material slot selection Static.
	size = (int)m_vStaticSelect.size();
	for( int i=0; i<size; ++i )
	{
		m_vStaticSelect[ i ]->Show( false ); 
	}

	// Close ChildDlg.
	//m_pChildDlg->Show( false );
	CDnChaosCubeStuffDlg * pDlg = GetInterface().GetChaosCubeStuffDlg();
	if( pDlg )
		pDlg->Show( false );
}


void CDnChaosCubeDlg::OpenDlg()
{
	if( !m_pItemChaos )
		return;

	m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7019 ) ); // "Please select materials to place into the Chaos Cube."

	int mixNumber = m_pItemChaos->GetTypeParam( 0 );
	int mixType = m_pItemChaos->GetTypeParam( 1 );

	switch( mixType )
	{
	case 0 :
		m_bSelfItemList = true;
		m_pBtnMaterial->Enable( true );
		m_pBtnProduct->Enable( true );
		break;
	
	case 1:
		m_bSelfItemList = true;
		m_pBtnMaterial->Enable( true );
		m_pBtnProduct->Enable( false );
		break;
	
	case 2:
		m_bSelfItemList = false;
		m_pBtnMaterial->Enable( true );
		m_pBtnProduct->Enable( true );
		break;

	case 3:
		m_bSelfItemList = false;
		m_pBtnMaterial->Enable( true );
		m_pBtnProduct->Enable( false );
		break;
	}


	// Get material items corresponding to the mix box number.	
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TCHAOSCUBESTUFF );
	if( !pTable )
		return;
	
	m_vStuffItemIDs.clear();

	int size = pTable->GetItemCount();
	for( int i=0; i<size; ++i )
	{
		int id = pTable->GetItemID( i );
		DNTableCell * pCell = pTable->GetFieldFromLablePtr( id, "_ChaoscubeNum" );
		if( pCell )
		{	
			if( mixNumber == pCell->GetInteger() )
			{
				pCell = pTable->GetFieldFromLablePtr( id, "_ItemID" );

				if( pCell )
				{
					int itemID = pCell->GetInteger();
					m_vStuffItemIDs.push_back( itemID );

					pCell = pTable->GetFieldFromLablePtr( id, "_Count" );
					if( pCell )
						m_vNeedStuffCount.insert( std::map< int, int >::value_type( itemID, pCell->GetInteger() ) );
				}
			}
		}
	}
	
	// Refresh item list.
	UpdateItemList(); 
	
}


// Refresh item list.
void CDnChaosCubeDlg::UpdateItemList()
{	
	if( !m_vStuffItemIDs.empty() )
	{			
		m_pStaticHelp->SetTextColor( textcolor::WHITE );
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7019 ) ); // "Please select materials to place into the Chaos Cube."

		m_pItemChaos = GetItemTask().GetCashInventory().FindItemFromSerialID( m_nChaosSerial );

		// No item.
		if( m_pItemChaos == NULL )
		{
			m_pItemChaos = GetItemTask().GetCharInventory().FindItemFromSerialID( m_nChaosSerial );
			if( m_pItemChaos == NULL )
			{
				Show( false );
				return;
			}
		}
				
		
		m_mapStuffItems.clear();

		CDnItemTask & pItemTask = GetItemTask();

		const int size = 3;
		CDnInventory *arrInven[ size ] = { &pItemTask.GetCharInventory(), &pItemTask.GetCashInventory(), &pItemTask.GetPetInventory() };
		CDnInventory * pInven;

		int stuffItemCnt = 0;
		int stuffItemCount = (int)m_vStuffItemIDs.size();
		for( int i=0; i<stuffItemCount; ++i )
		{		
			// Search materials in inventory.
			std::vector< CDnItem * > vResult;
			for( int k=0; k<size; ++k )
			{
				pInven = arrInven[ k ];
				int tot = pInven->ScanItemFromID( m_vStuffItemIDs[ i ], &vResult );

				// Found.
				if( !vResult.empty() )
				{
					++stuffItemCnt;
					m_mapStuffItems.insert( std::map< int, std::vector< CDnItem * > >::value_type( m_vStuffItemIDs[ i ], vResult ) ); // Store material items in map.
					break;
				}
			}
		}
		
		if( stuffItemCnt >= m_CountPerPage )
			--stuffItemCnt;
		m_maxPage = stuffItemCnt / m_CountPerPage;
	}

	

	// Refresh material slot.
	RefreshStuffItemSlot();
}


// Register material in material slot.
void CDnChaosCubeDlg::RefreshStuffItemSlot()
{
	// Hide material selection indicator.
	for( int i=0; i<m_CountPerPage; ++i )
		m_vStaticSelect[ i ]->Show( false );

	// Initialize material slot.
	int size = (int)m_vSlotBtnMaterials.size();
	for( int i=0; i<size; ++i )
	{
		m_vSlotBtnMaterials[ i ]->ResetSlot();
		m_vSlotBtnMaterials[ i ]->SetSlotType( ST_ITEM_NONE );
	}

	if( m_mapStuffItems.empty() )
	{
		m_pStaticHelp->SetTextColor( textcolor::RED );
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7033 ) ); // "You do not have any materials that can be placed into this Chaos Cube."
		return;
	}

	// Register materials for the current page into slots.	
	int i = 0;
	size = m_crrPage * m_CountPerPage;
	std::map< int, std::vector< CDnItem * > >::iterator it = m_mapStuffItems.begin();
	while( i < size )
	{
		++it;
		++i;

		if( it == m_mapStuffItems.end() )
		{			
			return;
		}		
	}
		
	for( i=0; i<m_CountPerPage; ++i )
	{
		std::vector< CDnItem * > & vItems = (*it).second;

		// Count.
		int tot = 0;
		for( int k=0; k<(int)vItems.size(); ++k )
			tot += vItems[ k ]->GetOverlapCount();

		// Register to slot.
		m_vSlotBtnMaterials[ i ]->SetItem( vItems[0], tot );
		
		// Change count font color to RED when material count is insufficient.		
		std::map< int, int >::iterator itNC = m_vNeedStuffCount.find( vItems[0]->GetClassID() );
		if( itNC != m_vNeedStuffCount.end() )
		{
			if( tot < (*itNC).second )
			{
				m_vSlotBtnMaterials[ i ]->SetSlotType( ST_SET_FONTCOLOR );
				m_vSlotBtnMaterials[ i ]->SetFontColor( D3DCOLOR_RGBA(255,0,0,255) );
			}
		}	

		++it;
		if( it == m_mapStuffItems.end() )
			break;
	}

	// Display page.
	wchar_t str[10];
	swprintf_s( str, 10, L"%d / %d", m_crrPage+1, m_maxPage+1 );
	m_pStaticPage->SetText( str );
}


// Select material item.
void CDnChaosCubeDlg::SelectionStuffItem( const char * strCtlName )
{
	// Selection indicator.
	std::string str( strCtlName );
	std::string strNumber = str.substr( strlen("ID_ITEM"), 1 );
	int idx = atoi( strNumber.c_str() );
	if( idx > 0 && idx <= m_CountPerPage )
	{
		// Hide selection indicator for everything except the selected part.
		for( int i=0; i<m_CountPerPage; ++i )
			m_vStaticSelect[ i ]->Show( false );
	
		// Show selection indicator if a material item exists in the selected part.
		--idx;

		MIInventoryItem * pSlotItem = m_vSlotBtnMaterials[ idx ]->GetItem();
		if( !pSlotItem )
			return;

		CDnItem * pSelectedItem = static_cast< CDnItem * >( pSlotItem );
		if( pSelectedItem )
		{
			m_SelectedSlotIndex = -1;			

			// Check required material count.
			int totItemCount = m_vSlotBtnMaterials[ idx ]->GetRenderCount(); // Inventory count.

			std::map< int, int >::iterator it = m_vNeedStuffCount.find( pSelectedItem->GetClassID() );
			if( it != m_vNeedStuffCount.end() )
			{
				int needCount = (*it).second; // Required material count.

				// Insufficient materials.
				if( totItemCount < needCount )
				{
					m_pBtnConfirm->Enable( false ); // Enable Confirm button.
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7022) ); // "Materials below the minimum quantity cannot be placed into the Chaos Cube."
				}

				// OK.
				else
				{
					m_vStaticSelect[ idx ]->Show( true );	

					// Register the selected item to the selected material slot.	
					m_pSlotBtnSelected->SetItem( pSelectedItem, needCount );		

					// Reset glow.
					m_fAlphaColor = 1.0f;

					m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7020 ) ); // "Check the selected materials and click Confirm. The designated amount of materials will transform into a new item."

					// Selected material slot index.
					m_SelectedSlotIndex = idx;

					m_pBtnConfirm->Enable( true ); // Enable Confirm button.
				}
			}			
		}		
	}
}


// Start mix.
void CDnChaosCubeDlg::MixStart()
{
	CDnItem * pSelectedItem = static_cast< CDnItem * >( m_pSlotBtnSelected->GetItem() );
	if( !pSelectedItem )
		return;

	CSChaosCubeRequest packet;
	memset( packet.ChaosItem, 0, sizeof(TChaosItem) * MAX_CHAOSCUBE_STUFF );

	if( m_pItemChaos->IsCashItem() )
		packet.cCubeInvenType = ITEMPOSITION_CASHINVEN;
	else
		packet.cCubeInvenType = ITEMPOSITION_INVEN;	
	packet.sCubeInvenIndex = m_pItemChaos->GetSlotIndex();
	packet.biCubeInvenSerial = m_pItemChaos->GetSerialID();

	
	// Required material count.
	std::map< int, int >::iterator it = m_vNeedStuffCount.find( pSelectedItem->GetClassID() );
	if( it != m_vNeedStuffCount.end() )
	{
		int needCount = (*it).second; // Required material count.
		packet.nCount = 0;

		std::map< int, std::vector< CDnItem * > >::iterator mapIt = m_mapStuffItems.find( pSelectedItem->GetClassID() );
		if( mapIt != m_mapStuffItems.end() )
		{		
			// Inventory type - Mount.
			CDnItem * pItem = GetItemTask().FindItem( pSelectedItem->GetClassID(), ST_INVENTORY_VEHICLE ); 
			if( pItem )
				packet.cInvenType = ITEMPOSITION_VEHICLE;
			else
			{
				// Inventory - Cash.
				if( pSelectedItem->IsCashItem() )
					packet.cInvenType = ITEMPOSITION_CASHINVEN; 
				else
				{
					// Inventory - Normal.
					pItem = GetItemTask().FindItem( pSelectedItem->GetClassID(), ST_INVENTORY ); 
					if( pItem )
						packet.cInvenType = ITEMPOSITION_INVEN;
					else
					{
						// Inventory type error.
						ASSERT( 0 && "Error - Inven Type." );
						return;
					}
				}
			}		

			std::vector< CDnItem * > & rVec = (*mapIt).second;
			int size = (int)rVec.size();
			for( int i=0; i<size; ++i )
			{
				++packet.nCount;

				CDnItem * stuffItem = rVec[ i ];				
							
				packet.ChaosItem[ i ].biInvenSerial = stuffItem->GetSerialID();
				packet.ChaosItem[ i ].sInvenIndex = stuffItem->GetSlotIndex();

				int count = stuffItem->GetOverlapCount();
				
				needCount -= count;

				// Need more, continue with the next item.
				if( needCount > 0 )
				{
					packet.ChaosItem[ i ].wCount = count;
				}

				else if( needCount == 0 )
				{
					packet.ChaosItem[ i ].wCount = count;
					break;	
				}
				
				else if( needCount < 0 )
				{
					packet.ChaosItem[ i ].wCount = needCount + count;
					break;
				}
			}// for.

			GetItemTask().RequestChaosCube( packet );
		}
	}

}	


// Open ProgressDlg.
//void CDnChaosCubeDlg::OpenProgressDlg( bool bShow, SCChaosCubeRequest * pData )
//{
//	m_pChaosCubeProgressDlg->SetRequestData( pData );
//	m_pChaosCubeProgressDlg->Show( bShow );
//}

//// Open ResultDlg;
//void CDnChaosCubeDlg::OpenResultDlg( bool bShow, int nItemID, int nCount )
//{
//	m_pChaosCubeResultDlg->SetResultItem( nItemID, nCount );
//	m_pChaosCubeResultDlg->Show( bShow );
//}


// Chaos item.
void CDnChaosCubeDlg::SetChaoseItem( class CDnItem * pItem )
{
	m_pItemChaos = pItem;

	if( m_pItemChaos == NULL )
		return;

	m_nChaosSerial = m_pItemChaos->GetSerialID();

	memcpy_s( m_pItemChaos, sizeof(CDnItem), pItem, sizeof(CDnItem) );		
	m_nChaosSerial = m_pItemChaos->GetSerialID();


	//
	//m_pStaticTitle = GetControl<CEtUIStatic>("ID_TEXT_TITLE"); // Title.
	m_pStaticTitle->SetText( m_pItemChaos->GetName() );
	
}


// Mix complete.
void CDnChaosCubeDlg::MixComplete()
{
	m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7019 ) ); // "Please select materials to place into the Chaos Cube."
}


// Result item received.
void CDnChaosCubeDlg::RecvResultItemComplet()
{
	if( m_SelectedSlotIndex != -1 )
	{
		char buf[10];
		std::string str( "ID_ITEM" );
		str.append( itoa(m_SelectedSlotIndex+1,buf,10) );

		SelectionStuffItem( str.c_str() );
	}
	
}

// Mix start response.
void CDnChaosCubeDlg::RecvMixStart( bool bEnableBtn )
{
	m_pBtnConfirm->Enable( bEnableBtn );
}


#endif
