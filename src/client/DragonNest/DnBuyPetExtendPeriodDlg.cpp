#include "StdAfx.h"
#include "DnBuyPetExtendPeriodDlg.h"
#include "DnVehicleTask.h"
#include "DnPetTask.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnInventory.h"
#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#include "DnCashShopTask.h"
#endif // PRE_ADD_INSTANT_CASH_BUY


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef PRE_ADD_PET_EXTEND_PERIOD

//////////////////////////////////////////////////////////////////////////
// class CDnBuyConfirmPetExtendPeriodDlg

CDnBuyConfirmPetExtendPeriodDlg::CDnBuyConfirmPetExtendPeriodDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
{
}

CDnBuyConfirmPetExtendPeriodDlg::~CDnBuyConfirmPetExtendPeriodDlg()
{
}

void CDnBuyConfirmPetExtendPeriodDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();
}

void CDnBuyConfirmPetExtendPeriodDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPetMessageDlg.ui" ).c_str(), bShow );
}

void CDnBuyConfirmPetExtendPeriodDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnBuyConfirmPetExtendPeriodDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) 
		{
			Show( false );
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnBuyConfirmPetExtendPeriodDlg::SetBuyInfo( int nPetLevel, std::wstring strPetName, std::wstring strPetExtendPeriodItemName )
{
	CEtUIStatic* pStatic = GetControl<CEtUIStatic>( "ID_TEXT_LV" );
	pStatic->SetText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 728 ), nPetLevel ) );
	pStatic = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	pStatic->SetText( strPetName );
	pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ITEMNAME" );
	pStatic->SetText( strPetExtendPeriodItemName );
}

//////////////////////////////////////////////////////////////////////////
// class CDnBuyPetExtendPeriodDlg

CDnBuyPetExtendPeriodDlg::CDnBuyPetExtendPeriodDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_nPetSerialID( 0 )
, m_pDnBuyConfirmPetExtendPeriodDlg( NULL )
, m_pPrevButton( NULL )
, m_pNextButton( NULL )
, m_pStaticPage( NULL )
, m_nTotalPage( 0 )
, m_nCurrentPage( 0 )
, m_nSelectedItemIndex( 0 )
{
	for( int i=0; i<5; i++ )
	{
		m_pPetExtendPeriodItemSlot[i] = NULL;
		m_pSelectStatic[i] = NULL;
	}
}

CDnBuyPetExtendPeriodDlg::~CDnBuyPetExtendPeriodDlg()
{
	SAFE_DELETE( m_pDnBuyConfirmPetExtendPeriodDlg );
}

void CDnBuyPetExtendPeriodDlg::InitialUpdate()
{
	CEtUIDialog::InitialUpdate();

	m_pDnBuyConfirmPetExtendPeriodDlg = new CDnBuyConfirmPetExtendPeriodDlg( UI_TYPE_MODAL );
	m_pDnBuyConfirmPetExtendPeriodDlg->Initialize( false );

	char szLabel[32];
	for( int i=0; i<5; i++ )
	{
		sprintf_s( szLabel, 32, "ID_BT_ITEM%d", i );
		m_pPetExtendPeriodItemSlot[i] = GetControl<CDnItemSlotButton>( szLabel );
		sprintf_s( szLabel, 32, "ID_STATIC_SELECT%d", i );
		m_pSelectStatic[i] = GetControl<CEtUIStatic>( szLabel );
	}
	
	m_pPrevButton = GetControl<CEtUIButton>( "ID_BT_PRIOR" );
	m_pNextButton = GetControl<CEtUIButton>( "ID_BT_NEXT" );
	m_pStaticPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );
}

void CDnBuyPetExtendPeriodDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPetCashMessageDlg.ui" ).c_str(), bShow );
}

void CDnBuyPetExtendPeriodDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
		if( pButton )
			pButton->Enable( true );
		pButton = GetControl<CEtUIButton>( "ID_BT_CLOSE" );
		if( pButton )
			pButton->Enable( true );

		RefreshPetExtendPeriodItemList();
#ifdef PRE_ADD_INSTANT_CASH_BUY
		int nShowType = GetCashShopTask().GetInstantCashBuyEnableType( INSTANT_BUY_PETEXTEND );
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
		m_pDnBuyConfirmPetExtendPeriodDlg->Show( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnBuyPetExtendPeriodDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) ) 
		{
			if( m_nSelectedItemIndex > -1 && m_nSelectedItemIndex < static_cast<int>( m_vecpPetExtendPeriodItem.size() ) )
			{
				TVehicleCompact* pPetCompact = GetVehicleTask().GetVehicleInfoFromSerial( m_nPetSerialID );
				if( pPetCompact == NULL ) return;

				m_pDnBuyConfirmPetExtendPeriodDlg->SetBuyInfo( GetPetTask().GetPetLevel( pPetCompact->Vehicle[Pet::Slot::Body].nSerial ), 
																pPetCompact->wszNickName, m_vecpPetExtendPeriodItem[m_nSelectedItemIndex]->GetName() );
				m_pDnBuyConfirmPetExtendPeriodDlg->SetDialogID( BUYCONFIRM_PETEXTENDPERIODITEM );
				m_pDnBuyConfirmPetExtendPeriodDlg->SetCallback( this );
				m_pDnBuyConfirmPetExtendPeriodDlg->Show( true );

				CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_OK" );
				if( pButton )
					pButton->Enable( false );
				pButton = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
				if( pButton )
					pButton->Enable( false );
				pButton = GetControl<CEtUIButton>( "ID_BT_CLOSE" );
				if( pButton )
					pButton->Enable( false );
			}
			else
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9275 ), MB_OK ); // UISTRING : Please select a Horn of Life to use.
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_PRIOR" ) )
		{
			PrevPage();
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			NextPage();
		}
#ifdef PRE_ADD_INSTANT_CASH_BUY
		else if( IsCmdControl( "UI_BUTTON_BUYSHORTCUT" ) )
		{
			GetInterface().ShowInstantCashShopBuyDlg( true, INSTANT_BUY_PETEXTEND, this );
			return;
		}
#endif // PRE_ADD_INSTANT_CASH_BUY
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

static bool SortbyExtendPeriod( CDnItem *s1, CDnItem *s2 )
{
	if( s1 == NULL || s2 == NULL ) return false;
	if( s1->IsEternityItem() || s2->IsEternityItem() ) return false;

	if( s1->GetExpireDate() < s2->GetExpireDate() ) return true;
	else if( s1->GetExpireDate() > s2->GetExpireDate() ) return false;

	return false;
}

void CDnBuyPetExtendPeriodDlg::RefreshPetExtendPeriodItemList()
{
	m_vecpPetExtendPeriodItem.clear();
	CDnItem* pItem = NULL;
	CDnInventory::INVENTORY_MAP_ITER iter = GetItemTask().GetCashInventory().GetInventoryItemList().begin();

	for( ; iter != GetItemTask().GetCashInventory().GetInventoryItemList().end(); iter++ )
	{
		pItem = iter->second;
		if( !pItem )
			continue;

		if( pItem->GetItemType() == ITEMTYPE_PET_EXPIRE )
			m_vecpPetExtendPeriodItem.push_back( pItem );
	}

	std::sort( m_vecpPetExtendPeriodItem.begin(), m_vecpPetExtendPeriodItem.end(), SortbyExtendPeriod );

	int nPetExtendPeriodItemCount = static_cast<int>( m_vecpPetExtendPeriodItem.size() );

	CEtUIStatic* pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK1" );
	if( nPetExtendPeriodItemCount > 0 )
	{
		pStatic->SetTextColor(textcolor::WHITE);
		pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9275 ) );
	}
	else
	{
		pStatic->SetTextColor(textcolor::RED);
		pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9276 ) );
	}

	m_nTotalPage = nPetExtendPeriodItemCount / 5;
	m_nTotalPage += nPetExtendPeriodItemCount % 5 > 0 ? 1 : 0;
	m_nCurrentPage = 0;

	if( m_nTotalPage > 0 )
	{
		SetItemPage( m_nCurrentPage );
	}
	else
	{
		for( int i=0; i<5; i++ )
		{
			if( m_pPetExtendPeriodItemSlot[i] )
			{
				m_pPetExtendPeriodItemSlot[i]->Show( false );
				m_pPetExtendPeriodItemSlot[i]->ResetSlot();
			}
			if( m_pSelectStatic[i] )
				m_pSelectStatic[i]->Show( false );
		}

		CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_OK" );
		pButton->Enable( false );
	}

	m_pPrevButton->Enable( true );
	m_pNextButton->Enable( true );
	if( m_nTotalPage <= 1 )
	{
		m_pPrevButton->Enable( false );
		m_pNextButton->Enable( false );
	}
}

void CDnBuyPetExtendPeriodDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
#ifdef PRE_ADD_INSTANT_CASH_BUY
	if( pControl == NULL && nID == INSTANT_CASHSHOP_BUY_DIALOG )
	{
		RefreshPetExtendPeriodItemList();
		if( nCommand == 1 )
			Show( false );

		return;
	}
#endif // PRE_ADD_INSTANT_CASH_BUY
	if( pControl && nID == BUYCONFIRM_PETEXTENDPERIODITEM )
	{
		if( strcmp( pControl->GetControlName(), "ID_BT_OK" ) == 0 ) 
		{
			if( m_nSelectedItemIndex > -1 && m_nSelectedItemIndex < static_cast<int>( m_vecpPetExtendPeriodItem.size() ) )
			{
				DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
				if( !pSox ) return;
				if( !pSox->IsExistItem( m_vecpPetExtendPeriodItem[m_nSelectedItemIndex]->GetClassID() ) ) return;

				int nExtendPeriod = pSox->GetFieldFromLablePtr( m_vecpPetExtendPeriodItem[m_nSelectedItemIndex]->GetClassID(), "_TypeParam1" )->GetInteger();

				GetPetTask().SendPetExtendPeriod( m_vecpPetExtendPeriodItem[m_nSelectedItemIndex]->GetSerialID(), m_nPetSerialID, nExtendPeriod );
			}
		}
		else if( strcmp( pControl->GetControlName(), "ID_BT_CANCEL" ) == 0 || strcmp( pControl->GetControlName(), "ID_BT_CLOSE" ) == 0 )
		{
			CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_OK" );
			if( pButton )
				pButton->Enable( true );
			pButton = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
			if( pButton )
				pButton->Enable( true );
			pButton = GetControl<CEtUIButton>( "ID_BT_CLOSE" );
			if( pButton )
				pButton->Enable( true );
		}
	}
}

bool CDnBuyPetExtendPeriodDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_LBUTTONDOWN )
	{
		POINT MousePoint;
		float fMouseX, fMouseY;

		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		SUICoord uiCoordsSelect;
		int nSelectIndex = -1;
		for( int i=0; i<5; i++ )
		{
			if( m_pPetExtendPeriodItemSlot[i] && m_pPetExtendPeriodItemSlot[i]->IsShow() )
			{
				 m_pPetExtendPeriodItemSlot[i]->GetUICoord( uiCoordsSelect );
				 if( uiCoordsSelect.IsInside( fMouseX, fMouseY ) )
				 {
					 nSelectIndex = i;
					 m_nSelectedItemIndex = i + ( m_nCurrentPage * 5 );
					 CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_OK" );
					 pButton->Enable( true );
					 break;
				 }
			}
		}
		if( nSelectIndex > -1 )
		{
			for( int i=0; i<5; i++ )
			{
				if( i == nSelectIndex )
					m_pSelectStatic[i]->Show( true );
				else
					m_pSelectStatic[i]->Show( false );
			}
		}
	}
	if( uMsg == WM_MOUSEWHEEL )
	{
		if( IsMouseInDlg() )
		{
			UINT uLines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			if( nScrollAmount > 0 )
			{
				PrevPage();
			}
			else if( nScrollAmount < 0 )
			{
				NextPage();
			}
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnBuyPetExtendPeriodDlg::PrevPage()
{
	if( m_nTotalPage <= 1 )
		return;

	m_nCurrentPage--;

	if( m_nCurrentPage < 0 )
	{
		m_nCurrentPage = m_nTotalPage - 1;
	}

	SetItemPage( m_nCurrentPage );
}

void CDnBuyPetExtendPeriodDlg::NextPage()
{
	if( m_nTotalPage <= 1 )
		return;

	m_nCurrentPage++;

	if( m_nCurrentPage >= m_nTotalPage )
	{
		m_nCurrentPage = 0;
	}

	SetItemPage( m_nCurrentPage );
}

void CDnBuyPetExtendPeriodDlg::SetItemPage( int nCurrentPage )
{
	if( nCurrentPage > m_nTotalPage )
		return;

	WCHAR wszStr[64];
	swprintf_s( wszStr, L"%d / %d", m_nCurrentPage + 1, m_nTotalPage );
	m_pStaticPage->SetText( wszStr );

	CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BT_OK" );
	pButton->Enable( false );
	m_nSelectedItemIndex = -1;

	int nIndex = nCurrentPage * 5;
	int nNextIndex = nIndex + 1;
	int nPetExtendPeriodItemCount = static_cast<int>( m_vecpPetExtendPeriodItem.size() );

	std::wstring str;
	if( nIndex < nPetExtendPeriodItemCount )
	{
		for( int i=0; i<5; i++ )
		{
			if( m_pPetExtendPeriodItemSlot[i] )
			{
				m_pPetExtendPeriodItemSlot[i]->Show( false );
				m_pPetExtendPeriodItemSlot[i]->ResetSlot();
			}
			if( m_pSelectStatic[i] )
				m_pSelectStatic[i]->Show( false );
		}

		int nSlotIndex = 0;
		for( int i=nIndex; i<static_cast<int>( m_vecpPetExtendPeriodItem.size() ) && i<nIndex+5; i++, nSlotIndex++ )
		{
			if( m_pPetExtendPeriodItemSlot[nSlotIndex] )
			{
				m_pPetExtendPeriodItemSlot[nSlotIndex]->Show( true );
				m_pPetExtendPeriodItemSlot[nSlotIndex]->SetItem( m_vecpPetExtendPeriodItem[i], -1 );
			}
		}
	}
}

#endif // PRE_ADD_PET_EXTEND_PERIOD