#include "StdAfx.h"

#ifdef PRE_ADD_CHOICECUBE

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
#include "DnChaosCubeStuffDlg.h"
#include "DnChaosCubeDlg.h"
#include "DnChoiceCubeDlg.h"

// Construct.
CDnChoiceCubeDlg::CDnChoiceCubeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor ),
m_pItemChaos(NULL), // Chaos item.
m_nChaosSerial(0),
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


	m_pStaticLightResult = NULL;
	m_pStaticPageResult = NULL;
	m_pSlotBtnSelectedResult = NULL; // Selected result slot.

	m_pBtnMaterial = NULL; // Material.
	m_pBtnProduct = NULL;  // Result.
	m_pBtnConfirm = NULL;  // Confirm.

	m_pWheelRegionStuff = NULL; // Material slot wheel area.
	m_pWheelRegionResult = NULL; // Result slot wheel area.

	m_crrPageResult = 0;
	m_maxPageResult = 0;

	m_SelectedSlotIndexResult = -1;
	m_fAlphaColorResult = 1.0f;

	m_fAlphaColorResult = 1.0f;
	m_signLightResult = 1;  
}


// ReleaseDlg()
void CDnChoiceCubeDlg::ReleaseDlg()
{
	m_vSlotBtnMaterials.clear(); // Material slots.
	m_vStaticSelect.clear(); // Material slot selection Static.
	m_vNeedStuffCount.clear(); // Number of materials required for mix.
	m_vStuffItemIDs.clear(); // Material item.


	m_vSlotBtnMaterialsResult.clear(); // Material slots.
	m_vStaticSelectResult.clear(); // Material slot selection Static.

	m_vResultItems.clear(); // Result items.

	m_vPeriodItems.clear(); // Static control for displaying time-limited duration in result slot.

}


// Initialize()
void CDnChoiceCubeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("ChoiceBoxDlg.ui").c_str(), bShow );
}


// InitialUpdate()
void CDnChoiceCubeDlg::InitialUpdate()
{

	//--- Material slots ---//
	m_pSlotBtnSelected = GetControl<CDnItemSlotButton>("ID_ITEM0"); // Selected material slot.
	m_pStaticLight = GetControl<CEtUIStatic>("ID_STATIC_LIGHT0");    // Sparkle.
		
	m_pStaticPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE0"); // Page/MaxPage.

	// Material slots.
	char buf[64] = {0,};
	m_vSlotBtnMaterials.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_SAUCEITEM%d", i );
		m_vSlotBtnMaterials[ i ] = GetControl<CDnItemSlotButton>( buf );
	}

	// Material slot selection Static.
	m_vStaticSelect.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_SAUCESELECT%d", i );
		m_vStaticSelect[ i ] = GetControl<CEtUIStatic>( buf );
	}	


	//--- Result slots ---//
	m_pSlotBtnSelectedResult = GetControl<CDnItemSlotButton>("ID_ITEM1"); // Selected result slot.
	m_pStaticLightResult = GetControl<CEtUIStatic>("ID_STATIC_LIGHT1"); // ID_STATIC_LIGHT_RESULT
	m_pStaticPageResult = GetControl<CEtUIStatic>("ID_TEXT_PAGE1"); // Page/MaxPage.
	
		
	// Slots.	
	m_vSlotBtnMaterialsResult.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_RESULTITEM%d", i );
		m_vSlotBtnMaterialsResult[ i ] = GetControl<CDnItemSlotButton>( buf );
	}

	// Slot selection Static.
	m_vStaticSelectResult.resize( m_CountPerPage );	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_RESULTSELECT%d", i );
		m_vStaticSelectResult[ i ] = GetControl<CEtUIStatic>( buf );
	}	

	// Static control for displaying time-limited duration in result slot.
	m_vPeriodItems.resize( m_CountPerPage ); 
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_STATIC_TIME%d", i );
		m_vPeriodItems[ i ] = GetControl<CEtUIStatic>( buf );
	}

	m_pBtnMaterial = GetControl<CEtUIButton>("ID_BT_LIST0"); // Material.
	m_pBtnProduct = GetControl<CEtUIButton>("ID_BT_LIST1");    // Result.


	m_pStaticTitle = GetControl<CEtUIStatic>("ID_TEXT_TITLE"); // Title.
	m_pStaticHelp = GetControl<CEtUIStatic>("ID_TEXT_ASK0"); // Ask. 

	m_pBtnConfirm = GetControl<CEtUIButton>("ID_OK"); // Confirm.

	m_pWheelRegionStuff = GetControl<CEtUIStatic>("ID_STATIC7");  // Material slot wheel area.
	m_pWheelRegionResult = GetControl<CEtUIStatic>("ID_STATIC8"); // Result slot wheel area.
}


// Show()
void CDnChoiceCubeDlg::Show( bool bShow )
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


void CDnChoiceCubeDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
	
	// Play glow animation if a selected material item exists.
	if( m_pSlotBtnSelected->GetItem() || m_pSlotBtnSelectedResult->GetItem() )
	{		
		D3DXCOLOR color( m_pStaticLight->GetTextureColor() );

		if( m_fAlphaColor <= 0.3f )
			m_signLight = 1;
		if( m_fAlphaColor >= 1.0f )
			m_signLight = -1;

		m_fAlphaColor += fElapsedTime * m_signLight;
		color.a = m_fAlphaColor;

		// Material slot.
		if( m_pSlotBtnSelected->GetItem() )
			m_pStaticLight->SetTextureColor( color );

		// Result slot.
		if( m_pSlotBtnSelectedResult->GetItem() && m_pStaticLightResult->IsShow() )
			m_pStaticLightResult->SetTextureColor( color );
	}

}


bool CDnChoiceCubeDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_MOUSEWHEEL:
		{			
			if( IsMouseInDlg() )
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				float fMouseX, fMouseY;
				ScreenToClient( hWnd, &MousePoint );
				PointToFloat( MousePoint, fMouseX, fMouseY );
				lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;

				// Material slot area.
				SUICoord coord;
				coord = m_pWheelRegionStuff->GetUICoord();
				if( coord.IsInside( fMouseX, fMouseY ) )				
				{
					if( nScrollAmount > 0 )
					{
						if( m_crrPage > 0 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_PRIOR0"), 0 );
					}
					else if( nScrollAmount < 0 )
					{
						if( m_crrPage < m_maxPage )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT0"), 0 );
					}
				}

				// Result slot area.
				else
				{
					coord = m_pWheelRegionResult->GetUICoord();
					if( coord.IsInside( fMouseX, fMouseY ) )					
					{
						if( nScrollAmount > 0 )
						{
							if( m_crrPageResult > 0 )
								ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_PRIOR1"), 0 );
						}
						else if( nScrollAmount < 0 )
						{
							if( m_crrPageResult < m_maxPageResult )
								ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT1"), 0 );
						}
					}
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
void CDnChoiceCubeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{

		if( strstr( pControl->GetControlName(), "ID_SAUCEITEM" ) )
		{
			// Select material item.
			SelectionStuffItem( pControl->GetControlName() );			
		}

		if( strstr( pControl->GetControlName(), "ID_RESULTITEM" ) )
		{
			// Select result item.
			SelectionResultItem( pControl->GetControlName() );
		}

		// Previous.
		else if( IsCmdControl("ID_BT_PRIOR0") )
		{
			--m_crrPage;
			if( m_crrPage < 0 )
				m_crrPage = m_maxPage;

			// Refresh material slot.
			RefreshStuffItemSlot();
		}

		// Next.
		else if( IsCmdControl("ID_BT_NEXT0") )
		{
			++m_crrPage;
			if( m_crrPage > m_maxPage )
				m_crrPage = 0;

			// Refresh material slot.
			RefreshStuffItemSlot();
		}


		// Previous result slot.
		else if( IsCmdControl("ID_BT_PRIOR1") )
		{
			--m_crrPageResult;
			if( m_crrPageResult < 0 )
				m_crrPageResult = m_maxPageResult;
			
			// Update result slot.
			RefreshResultItemSlot();
		}

		// Next result slot.
		else if( IsCmdControl("ID_BT_NEXT1") )
		{
			++m_crrPageResult;
			if( m_crrPageResult > m_maxPageResult )
				m_crrPageResult = 0;

			// Update result slot.
			RefreshResultItemSlot();
		}



		// Open material window.
		else if( IsCmdControl("ID_BT_LIST0") )
		{
			CDnChaosCubeStuffDlg * pStuffDlg = GetInterface().GetChaosCubeStuffDlg();
			if( pStuffDlg )
			{
				pStuffDlg->SetDlgMode( CDnChaosCubeDlg::EChildDlgMode::STUFF_CHOICE, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
				pStuffDlg->Show( true );
			}
		}

		// Open result window.
		else if( IsCmdControl("ID_BT_LIST1") )
		{
			CDnChaosCubeStuffDlg * pStuffDlg = GetInterface().GetChaosCubeStuffDlg();
			if( pStuffDlg )
			{
				pStuffDlg->SetDlgMode( CDnChaosCubeDlg::EChildDlgMode::PRODUCT_CHOICE, m_pItemChaos->GetTypeParam(0), m_bSelfItemList );
				pStuffDlg->Show( true );
			}
		}

		// Close.
		else if( IsCmdControl("ID_BT_CLOSE") || IsCmdControl("ID_CANCEL") )
			Show( false );

		// Start mix.
		else if( IsCmdControl("ID_OK") )
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
					GetInterface().MessageBox( 7040, MB_YESNO, 1, this ); // "The selected material will be consumed and the selected item will be obtained. Really operate the Choice Cube?"
				}
			}					
		}

	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnChoiceCubeDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
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
void CDnChoiceCubeDlg::CloseDlg()
{
	m_crrPage = 0;
	m_crrPageResult = 0;

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
		m_vStaticSelect[ i ]->Show( false ); 
	

	// Initialize result slot.
	m_pSlotBtnSelectedResult->ResetSlot();
	size = (int)m_vSlotBtnMaterialsResult.size();
	for( int i=0; i<size; ++i )
		m_vSlotBtnMaterialsResult[ i ]->ResetSlot();

	// Hide - Result slot selection Static.
	size = (int)m_vStaticSelectResult.size();
	for( int i=0; i<size; ++i )
		m_vStaticSelectResult[ i ]->Show( false ); 
	

	// Hide glow on result selection slot.
	m_pStaticLightResult->Show( false );

	// Close ChildDlg.
	//m_pChildDlg->Show( false );
	CDnChaosCubeStuffDlg * pDlg = GetInterface().GetChaosCubeStuffDlg();
	if( pDlg )
		pDlg->Show( false );
}


void CDnChoiceCubeDlg::OpenDlg()
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


	// Result slots.
	m_vResultItems.clear();
	pTable = GetDNTable( CDnTableDB::TCHAOSCUBERESULT );
	if( !pTable )
		return;

	size = pTable->GetItemCount();
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
					CDnItem * pNewItem = NULL;
					int itemID = pCell->GetInteger();

					// Count.
					pCell = pTable->GetFieldFromLablePtr( id, "_Count" );
					if( pCell )
					{
						pNewItem = CDnItem::CreateItem( itemID, 0 );
						pNewItem->SetOverlapCount( pCell->GetInteger() );

						// Time-limited.
						pCell = pTable->GetFieldFromLablePtr( id, "_Period" );
						if( pCell )
						{
							SResultItem aNewItem;
							aNewItem.pItem = pNewItem;
							aNewItem.nPeriod = pCell->GetInteger();		

							// Gold.
							pCell = pTable->GetFieldFromLablePtr( id, "_Gold" );
							if( pCell )
								aNewItem.nGold = (INT64)( pCell->GetInteger() );

							m_vResultItems.push_back( aNewItem );
						}						

					}
				}
			}
		}
	}


	// Refresh item list.
	UpdateItemList(); 

}


// Refresh item list.
void CDnChoiceCubeDlg::UpdateItemList()
{	

	// Material slot -------------//
	if( !m_vStuffItemIDs.empty() )
	{			
		m_pStaticHelp->SetTextColor( textcolor::WHITE );
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7019 ) ); // "Please select materials to place into the Chaos Cube."

		m_pItemChaos = GetItemTask().GetCashInventory().FindItemFromSerialID( m_nChaosSerial );

		// No item.
		if( m_pItemChaos == NULL )
		{
			Show( false );
			return;
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



	// Result slot -----------//
	//m_crrPageResult = 0;
	if( m_vResultItems.empty() )
	{
		m_crrPageResult = m_maxPageResult = 0;		
	}
	else
	{
		int size = (int)m_vResultItems.size();
		if( size >= m_CountPerPage )
			--size;
		m_maxPageResult = size / m_CountPerPage;
	}
	
	// Update result slot.
	RefreshResultItemSlot();

}


// Register material in material slot.
void CDnChoiceCubeDlg::RefreshStuffItemSlot()
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
	wchar_t str[128];
	swprintf_s( str, 10, L"%d / %d", m_crrPage+1, m_maxPage+1 );
	m_pStaticPage->SetText( str );

}

// Select material item.
void CDnChoiceCubeDlg::SelectionStuffItem( const char * strCtlName )
{
	// Selection indicator.
	std::string str( strCtlName );
	std::string strNumber = str.substr( strlen("ID_SAUCEITEM"), 1 );
	int idx = atoi( strNumber.c_str() );
	if( idx >= 0 && idx < m_CountPerPage )
	{
		// Hide selection indicator for everything except the selected part.
		for( int i=0; i<m_CountPerPage; ++i )
			m_vStaticSelect[ i ]->Show( false );

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

					// Whether result item is selected - 
					// 7034 : "Please select the item you want to acquire from the result list."
					// 7035 : "Please check the result item and click the Confirm button."
					m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 
						m_pSlotBtnSelectedResult->GetItem() == NULL ? 7034 : 7035 ) );					

					// Selected material slot index.
					m_SelectedSlotIndex = idx;

					if( m_pSlotBtnSelectedResult->GetItem() != NULL )
					{
						// Compare if material item and result item are the same.
						if( CompareStuffnResult() )
							m_pBtnConfirm->Enable( true ); // Enable Confirm button.						
					}
					
				}
			}			
		}		
	}
}

// Register result into result slot.
void CDnChoiceCubeDlg::RefreshResultItemSlot()
{
	// Result slots -------------//


	// Hide selection indicator.
	for( int i=0; i<m_CountPerPage; ++i )
		m_vStaticSelectResult[ i ]->Show( false );

	// Initialize slot.
	int size = (int)m_vSlotBtnMaterialsResult.size();
	for( int i=0; i<size; ++i )
	{
		m_vSlotBtnMaterialsResult[ i ]->ResetSlot();
		m_vSlotBtnMaterialsResult[ i ]->SetSlotType( ST_ITEM_NONE );
	}

	if( !m_vResultItems.empty() )
	{
		CDnItem * pResultItem = NULL;
		int slotIndex = 0;
		int itemSize = (int)m_vResultItems.size();

		int size = (int)m_vResultItems.size();
		int nBegin = m_crrPageResult * m_CountPerPage;
		for( int i=nBegin; i<nBegin+m_CountPerPage; ++i, ++slotIndex )
		{
			if( i < itemSize )
			{
				SResultItem & rResultItem = m_vResultItems[ i ];				
				m_vSlotBtnMaterialsResult[ slotIndex ]->SetItem( rResultItem.pItem, rResultItem.pItem->GetOverlapCount() );

				// In the case of a time-limited item - 
				m_vPeriodItems[ slotIndex ]->Show( ( rResultItem.nPeriod > 0 ? true : false ) ); //Display icon.
				if( rResultItem.nPeriod )
				{
					std::wstring strTooltip = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4706 ), rResultItem.nPeriod );
					m_vSlotBtnMaterialsResult[ slotIndex ]->SetTooltipText( strTooltip.c_str() );
				}
				else
				{
					m_vSlotBtnMaterialsResult[ slotIndex ]->ClearTooltipText();
				}

				// For gold items, display gold amount in the tooltip.
				if( rResultItem.pItem->GetClassID() == 1073750029 )
				{
					// Gold acquired - do not output units with 0 (e.g., 1 gold 0 silver 1 copper -> 1 gold 1 copper).
					if( rResultItem.nGold > 0 )
					{
						std::wstring str;
						GoldToString( str, rResultItem.nGold,
							GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1766 ),
							GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1781 ),		
							GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1783 ) );
						m_vSlotBtnMaterialsResult[ slotIndex ]->SetTooltipText( str.c_str() );	
					}
					else
					{
						m_vSlotBtnMaterialsResult[ slotIndex ]->ClearTooltipText();
					}
				}
			}
		}

	}

	// Display page.	
	wchar_t str[128] = {0,};
	swprintf_s( str, 10, L"%d / %d", m_crrPageResult+1, m_maxPageResult+1 );
	m_pStaticPageResult->SetText( str );	

}


// Select result item.
void CDnChoiceCubeDlg::SelectionResultItem( const char * strCtlName )
{
	// Selection indicator.
	std::string str( strCtlName );
	std::string strNumber = str.substr( strlen("ID_RESULTITEM"), 1 );
	int idx = atoi( strNumber.c_str() );
	if( idx >= 0 && idx < m_CountPerPage )
	{
		// Hide selection indicator for everything except the selected part.
		for( int i=0; i<m_CountPerPage; ++i )
			m_vStaticSelectResult[ i ]->Show( false );

		MIInventoryItem * pSlotItem = m_vSlotBtnMaterialsResult[ idx ]->GetItem();
		if( !pSlotItem )
			return;

		CDnItem * pSelectedItem = static_cast< CDnItem * >( pSlotItem );
		if( pSelectedItem )
		{
			m_SelectedSlotIndexResult = -1;					
			
			m_vStaticSelectResult[ idx ]->Show( true );	

			// Register the selected item to the selected material slot.	
			m_pSlotBtnSelectedResult->SetItem( pSelectedItem, pSelectedItem->GetOverlapCount() );		

			// Reset glow.
			m_fAlphaColorResult = 1.0f;

			//m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7020 ) ); // "Check the selected materials and click Confirm. The specified number of materials will be transformed into a new item."

			// Selected material slot index.
			m_SelectedSlotIndexResult = idx;

			if( m_pSlotBtnSelected->GetItem() != NULL )
			{
				m_pStaticLightResult->Show( true );

				// Compare if material item and result item are the same.
				if( CompareStuffnResult() )
					m_pBtnConfirm->Enable( true ); // Enable Confirm button.				
			}
		}
	}

}


// Start mix.
void CDnChoiceCubeDlg::MixStart()
{
	CDnItem * pSelectedItem = static_cast< CDnItem * >( m_pSlotBtnSelected->GetItem() );
	CDnItem * pSelectedItemResult = static_cast< CDnItem * >( m_pSlotBtnSelectedResult->GetItem() );
	if( !pSelectedItem || !pSelectedItemResult )
		return;

	CSChaosCubeRequest packet;
	memset( packet.ChaosItem, 0, sizeof(TChaosItem) * MAX_CHAOSCUBE_STUFF );
	
	if( m_pItemChaos->IsCashItem() )
		packet.cCubeInvenType = ITEMPOSITION_CASHINVEN;
	else
		packet.cCubeInvenType = ITEMPOSITION_INVEN;	
	packet.sCubeInvenIndex = m_pItemChaos->GetSlotIndex();
	packet.biCubeInvenSerial = m_pItemChaos->GetSerialID();
	packet.nChoiceItemID = pSelectedItemResult->GetClassID();

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


// Chaos item.
void CDnChoiceCubeDlg::SetChaoseItem( class CDnItem * pItem )
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
void CDnChoiceCubeDlg::MixComplete()
{
	m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7019 ) ); // "Please select materials to place into the Chaos Cube."
}


// Result item received.
void CDnChoiceCubeDlg::RecvResultItemComplet()
{
	// Material.
	if( m_SelectedSlotIndex != -1 )
	{
		char buf[10];
		std::string str( "ID_SAUCEITEM" );
		str.append( itoa(m_SelectedSlotIndex,buf,10) );

		SelectionStuffItem( str.c_str() );
	}

	// Result.
	if( m_SelectedSlotIndexResult != -1 )
	{
		char buf[10];
		std::string str( "ID_RESULTITEM" );
		str.append( itoa(m_SelectedSlotIndexResult,buf,10) );

		SelectionResultItem( str.c_str() );
	}

}

// Mix start response.
void CDnChoiceCubeDlg::RecvMixStart( bool bEnableBtn )
{
	m_pBtnConfirm->Enable( bEnableBtn );
}


int CDnChoiceCubeDlg::GetResultItemID()
{	
	CDnItem * pR = static_cast< CDnItem * >( m_pSlotBtnSelectedResult->GetItem() );
	if( pR )
		return pR->GetClassID();

	return 0;
}


// Compare if material item and result item are the same.
bool CDnChoiceCubeDlg::CompareStuffnResult()
{
	int pStuffID = m_pSlotBtnSelected->GetItem()->GetClassID();
	int pResultID = m_pSlotBtnSelectedResult->GetItem()->GetClassID();
	
	bool bSame = ( pStuffID == pResultID );
	int typeParam2 = m_pItemChaos->GetTypeParam( 1 );

	bool bRes = true;

	// Same item.
	if( bSame )
	{
		bRes = (typeParam2 == 0 || typeParam2 == 1) ? true : false; // True if same item is allowed, otherwise false.
	}

	// Process message & remove result slot.
	if( !bRes )
	{
		m_pSlotBtnSelectedResult->ResetSlot();
		m_vStaticSelectResult[ m_SelectedSlotIndexResult ]->Show( false ); // Hide selected slot indicator.
		m_pStaticLightResult->Show( false );
		GetInterface().MessageBox( 7041, MB_OK ); // "In this cube, the material and result cannot be selected as the same item."
	}
	else
	{
		m_pStaticLightResult->Show( true );
	}

	return bRes;
}


// Whether item has an expiration period (return value: duration).
int CDnChoiceCubeDlg::GetPeriodItemByID( int nItemID )
{
	int size = (int)m_vResultItems.size();
	for( int i=0; i<size; ++i )
	{
		SResultItem & rItem = m_vResultItems[ i ];
		if( rItem.pItem->GetClassID() == nItemID )
			return rItem.nPeriod;
	}

	return 0;
}


#endif
