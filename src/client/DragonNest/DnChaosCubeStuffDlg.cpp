#include "StdAfx.h"


#ifdef PRE_ADD_CHAOSCUBE

#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnMainFrame.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnChaosCubeDlg.h"
#include "DnChaosCubeStuffDlg.h"

// Construct.
CDnChaosCubeStuffDlg::CDnChaosCubeStuffDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor ),
m_DlgMode( CDnChaosCubeDlg::EChildDlgMode::NONE ),
m_CountPerPage(5), // Number of slots per page.
m_crrPage(0), // Current page.
m_maxPage(0),  // Total pages.
m_nStuffItemID(-1), // Material item ID.
m_nChaosNum(0)
{
	m_pStaticTitle  = NULL; // Title bar.
	m_pStaticHelp = NULL;  // Help.
	m_pStaticPage = NULL;  // Page/MaxPage.

	m_pBtnPrev = NULL;     // Previous.
	m_pBtnNext = NULL;     // Next.
}


// ReleaseDlg()
void CDnChaosCubeStuffDlg::ReleaseDlg()
{
	m_mapGold.clear(); // Result gold amount.
	m_vItems.clear(); // Item list.
	m_vSlotBtns.clear();   // Item slots.
	m_vSlotStatics.clear();//Item slot Static.
}


// Initialize()
void CDnChaosCubeStuffDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("MixBoxListDlg.ui").c_str(), bShow );
}


// InitialUpdate()
void CDnChaosCubeStuffDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>("ID_TEXT_TITLE");
	m_pStaticHelp  = GetControl<CEtUIStatic>("ID_TEXT_ASK");
	m_pStaticPage  = GetControl<CEtUIStatic>("ID_TEXT_PAGE");

	// Material slots.
	char buf[64] = {0,};
	m_vSlotBtns.resize( m_CountPerPage );
	m_vSlotStatics.resize( m_CountPerPage );
	for( int i=0; i<m_CountPerPage; ++i )
	{
		sprintf_s( buf, 64, "ID_ITEM%d", i );
		m_vSlotBtns[ i ] = GetControl<CDnItemSlotButton>( buf );

		sprintf_s( buf, 64, "ID_TEXT_NAME%d", i );
		m_vSlotStatics[ i ] = GetControl<CEtUIStatic>( buf );
	}

}


// Show()
void CDnChaosCubeStuffDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	// Open.
	if( bShow )
	{
		// Material window.
		if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF )
			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7028 ) ); // "Chaos Cube Material List"

		// Result window.
		else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT )
			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7027 ) ); // "Chaos Cube Result"

#ifdef PRE_ADD_CHOICECUBE
		// Material_Choice window.
		else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF_CHOICE )
			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7028 ) ); // "Choice Cube Material List"

		// Result_Choice window.
		else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT_CHOICE )
			m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7027 ) ); // "Choice Cube Result"
#endif

		// Refresh item slot.
		RefreshList();
	}

	// Close.
	else
	{
		m_crrPage = 0;
	}
}


bool CDnChaosCubeStuffDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
void CDnChaosCubeStuffDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		// Previous.
		if( IsCmdControl("ID_BT_PRIOR") )
		{
			--m_crrPage;
			if( m_crrPage < 0 )
				m_crrPage = m_maxPage;

			// Refresh item slot.
			RefreshList();
		}

		// Next.
		else if( IsCmdControl("ID_BT_NEXT") )
		{
			++m_crrPage;
			if( m_crrPage > m_maxPage )
				m_crrPage = 0;

			// Refresh item slot.
			RefreshList();
		}

		else if( IsCmdControl("ID_BT_CLOSE") || IsCmdControl("ID_BT_CANCEL") )
			Show( false );
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}



// Get material items corresponding to the mix box number.	
//void CDnChaosCubeStuffDlg::LoadItems( int nChaosNum, int nItemID, bool bSelf )
void CDnChaosCubeStuffDlg::LoadItems( int nChaosNum, bool bSelf )
{
	m_vItems.clear();

	DNTableFileFormat * pTable = NULL;

	char * strCellName = "_ChaoscubeNum";

	// Table.
	if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF )		// Material
		pTable = GetDNTable( CDnTableDB::TCHAOSCUBESTUFF );
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT ) // Result.
		pTable = GetDNTable( CDnTableDB::TCHAOSCUBERESULT );

#ifdef PRE_ADD_CHOICECUBE
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF_CHOICE ) // Material_Choice.
		pTable = GetDNTable( CDnTableDB::TCHAOSCUBESTUFF );
	
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT_CHOICE ) // Result_Choice.
		pTable = GetDNTable( CDnTableDB::TCHAOSCUBERESULT );

#endif


	if( !pTable )
		return;	

	CDnItem * pItem = NULL;
	DNTableCell * pCell = NULL;

	int size = pTable->GetItemCount();
	for( int i=0; i<size; ++i )
	{
		int id = pTable->GetItemID( i );
		DNTableCell * pCell = pTable->GetFieldFromLablePtr( id, "_ChaoscubeNum" );
		if( pCell && ( pCell->GetInteger() != nChaosNum ) )
			continue;
			
		pCell = pTable->GetFieldFromLablePtr( id, "_ItemID" );
		if( pCell )
		{	
			/*// Whether an item identical to the material item is loaded.
			if( bSelf == false && pCell->GetInteger() == nItemID )
				continue;*/

			TItemInfo info;
			if( CDnItem::MakeItemInfo( pCell->GetInteger(), 1, info ) )
			{
				pItem = CDnItemTask::GetInstance().CreateItem( info );
				if( pItem )
				{
					pCell = pTable->GetFieldFromLablePtr( id, "_Count" );
					if( pCell )
					{
						pItem->SetOverlapCount( pCell->GetInteger() );
						m_vItems.push_back( pItem );
					}
				}
			}
		}

		// Result gold amount.
#ifdef PRE_ADD_CHOICECUBE
		if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT || 
			m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT_CHOICE ) // Result.			
#else
		if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT ) // Result.
#endif	
		{
			pCell = pTable->GetFieldFromLablePtr( id, "_Gold" );
			if( pCell )
				m_mapGold.insert( std::map< int, INT64 >::value_type( i, (INT64)pCell->GetInteger() ) );
		}


	}

	// MaxPage.
	if( m_vItems.empty() )
		m_maxPage = 0;
	else
	{
		int stuffSize = (int)m_vItems.size();
		if( stuffSize >= m_CountPerPage )
			--stuffSize;
		m_maxPage = stuffSize / m_CountPerPage;
	}		

}


// Material window or Result window.
//void CDnChaosCubeStuffDlg::SetDlgMode( int mode, int nItemID, int nChaosNum, bool bSelf )
void CDnChaosCubeStuffDlg::SetDlgMode( int mode, int nChaosNum, bool bSelf )
{ 		
	//if(	m_DlgMode == mode && m_nChaosNum == nChaosNum )
	//	return;

	//m_nStuffItemID = nItemID; // Material item ID.

	m_DlgMode = mode;
	m_nChaosNum = nChaosNum;

	// Load.
	//LoadItems( nChaosNum, nItemID, bSelf );
	LoadItems( nChaosNum, bSelf );

	// Material window.
	if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF )
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7031 ) ); // "One of the following items is required to operate this Chaos Cube."

	// Result window.
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT )		
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, bSelf ? 7025 : 7026 ) ); // "One of the items in the list below appears in this Chaos Cube.", "One of the items in the list below, excluding the inserted material, appears in this Chaos Cube."

#ifdef PRE_ADD_CHOICECUBE
	// Material_Choice window.
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::STUFF_CHOICE )
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7038 ) ); // "In this Choice Cube, you can select one item from the list below."

	// Result_Choice window.
	else if( m_DlgMode == CDnChaosCubeDlg::EChildDlgMode::PRODUCT_CHOICE )
		m_pStaticHelp->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7039 ) ); // "In this Choice Cube, you can select one item from the list below excluding the inserted material."
#endif

}


// Handle item list paging.
void CDnChaosCubeStuffDlg::RefreshList()
{
	// Initialize item slots.
	int size = (int)m_vSlotBtns.size();
	for( int i=0; i<size; ++i )
	{
		m_vSlotBtns[ i ]->ResetSlot();	
		m_vSlotStatics[ i ]->SetText( std::wstring(L"") );
	}
		
	size = m_crrPage * m_CountPerPage;		

	// Register items for the current page into slots.	
	for( int i=0; i<m_CountPerPage; ++i )
	{
		unsigned int index = size + i;
		if( index >= m_vItems.size() )
			break;

		CDnItem * pItem = m_vItems[ index ];
		// Register to slot.
		m_vSlotBtns[ i ]->SetItem( pItem, pItem->GetOverlapCount() );

		// When it is gold.
		if( pItem->GetClassID() == 1073750029 ) 
		{
			std::map< int, INT64 >::iterator it = m_mapGold.find( i );
			if( it != m_mapGold.end() )
			{
				std::wstring str;
				GoldToString( str, (*it).second,
					GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1766 ),
					GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1781 ),		
					GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1783 ) );
				m_vSlotStatics[ i ]->SetText( str );
			}
		}
		// In case of a non-gold item.
		else
			m_vSlotStatics[ i ]->SetText( pItem->GetName() );
	}

	// Display page.
	wchar_t str[10];
	swprintf_s( str, 10, L"%d / %d", m_crrPage+1, m_maxPage+1 );
	m_pStaticPage->SetText( str );

}

#endif