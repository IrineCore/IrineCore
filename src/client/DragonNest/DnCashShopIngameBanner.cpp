#include "Stdafx.h"

#ifdef PRE_ADD_CASHSHOP_RENEWAL

#include "DnTableDB.h"
#include "DnItemTask.h"
#include "DnCashShopTask.h"

#include "DnInterface.h"
#include "DnCashShopRenewalDlg.h"
#include "DnCashShopIngameBanner.h"


CDnCashShopIngameBanner::CDnCashShopIngameBanner(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
:CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
,m_AccumulateTime(0.0f)
,m_crrShowItem(0)
,m_pItemSlot(NULL)
,m_pStaticName(NULL)
,m_pStaticPrice(NULL)
,m_pCheckBoxShow(NULL)
,m_pStaticBoard(NULL)
,m_pStaticShow(NULL)
{

}

CDnCashShopIngameBanner::~CDnCashShopIngameBanner()
{	
	int size = (int)m_vItems.size();
	for( int i=0; i<size; ++i )
		delete m_vItems[ i ];
		//delete m_vItems[ i ].pItem;
	m_vItems.clear();
}

void CDnCashShopIngameBanner::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CS_SumInfoDlg.ui" ).c_str(), bShow );
}


void CDnCashShopIngameBanner::InitialUpdate()
{
	// Control.
	m_pItemSlot = GetControl<CDnItemSlotButton>("ID_BT_ITEM");
	m_pStaticBoard = GetControl<CEtUIStatic>("ID_STATIC_BOARD");
	m_pStaticName = GetControl<CEtUIStatic>("ID_TEXT_NAME");
	m_pStaticPrice = GetControl<CEtUITextBox>("ID_TEXTBOX_PRICE");
	m_pCheckBoxShow = GetControl<CEtUICheckBox>("ID_CHECKBOX_SHOW");
	m_pCheckBoxShow->SetChecked( true );
	
	m_pStaticShow = GetControl<CEtUIStatic>("ID_TEXT_SHOW");
	m_pStaticShow->Show( false );


	// Select items to display on the banner.
	int jobClass = 1;
	if( CDnActor::s_hLocalActor )
		jobClass = CDnActor::s_hLocalActor->GetClassID();

	DNTableFileFormat * pTableItem = GetDNTable(CDnTableDB::TITEM);
	DNTableFileFormat * pTableCashLimit = GetDNTable(CDnTableDB::TCASHLIMIT);
	int sizeCashLimit = pTableCashLimit->GetItemCount();


	DNTableFileFormat * pTableCashCommodity = GetDNTable(CDnTableDB::TCASHCOMMODITY);
	DNTableCell * pCell = NULL;
	if( pTableCashCommodity )
	{
		int size = pTableCashCommodity->GetItemCount();
		for ( int i=0; i<size; ++i )
		{
			int nID = pTableCashCommodity->GetItemID( i );
			
			pCell = pTableCashCommodity->GetFieldFromLablePtr(nID, "_OnSale");
			if( !pCell || ( pCell && pCell->GetBool() == false )  )
				continue;
			
			pCell = pTableCashCommodity->GetFieldFromLablePtr(nID, "_ShowBanner");
			if( !pCell )
				continue;
			float fShowTime = (float)pCell->GetInteger();
			if( fShowTime < 1.0f )
				continue;
			

			pCell = pTableCashCommodity->GetFieldFromLablePtr(nID, "_ItemID01");
			if( !pCell )
				continue;

			int itemID = pCell->GetInteger();
			
			// ItemTable.	
			pCell = pTableItem->GetFieldFromLablePtr( itemID, "_NeedJobClass");
			if( !pCell )
				continue;

			bool bNeedJob = false;
			std::vector<std::string> tokens;	
			TokenizeA( std::string(pCell->GetString()), tokens, std::string(";") );
			int cnt = (int)tokens.size();
			for( int k=0; k<cnt; ++k )
			{
				int needJob = ::atoi( tokens[k].c_str() );
				if( needJob == 0 || needJob == jobClass )
				{
					bNeedJob = true;
					break;
				}
			}

			if( !bNeedJob )
				continue;
		
			// Create item.
			TItemInfo info;
			if( CDnItem::MakeItemInfo( itemID, 1, info ) )
			{
				CDnItem * pItem = CDnItemTask::GetInstance().CreateItem( info );
				if( pItem )
				{
					// SN.
					pCell = pTableCashCommodity->GetFieldFromLablePtr(nID, "_SN");
					if( !pCell )
						continue;
					int _SN = pCell->GetInteger();
					pItem->SetSerialID( _SN );

					//if( GetCashShopTask().IsActive() && GetCashShopTask().GetCashLimitItemMaxCount( _SN ) > 0 && GetCashShopTask().GetCashLimitItemRemainCount( _SN ) == 0 )					
					//if( GetCashShopTask().IsActive() && GetCashShopTask().GetCashLimitItemMaxCount( _SN ) > 0 && GetCashShopTask().GetCashLimitItemRemainCount( _SN ) > 0 )
					//{
						//SBannerItem bannerItem( pItem, fShowTime);
						//m_vItems.push_back( bannerItem ); 
						SBannerItem * pBItem = new SBannerItem( pItem, fShowTime );
						m_vItems.push_back( pBItem );
					//}
				}
			}

			//pCell = pTableCashCommodity->GetFieldFromLablePtr(nID, "_SN");
			//if( !pCell )
			//	continue;
			//int serialID = pCell->GetInteger();

			//// CashLimit Table.
			//for( int k=0; k<sizeCashLimit; ++k )
			//{
			//	int _id = pTableCashLimit->GetItemID( k );

			//	pCell = pTableItem->GetFieldFromLablePtr( _id, "_SN" );
			//	if( !pCell )
			//		continue;

			//	 if( pCell && pCell->GetInteger() == serialID  )
			//	 {
			//		 // Start.
			//		 pCell = pTableItem->GetFieldFromLablePtr( _id, "_StartDate" );
			//		 if( !pCell )
			//			 continue;
			//		 std::string strStart( pCell->GetString() );

			//		 // End.
			//		 pCell = pTableItem->GetFieldFromLablePtr( _id, "_EndDate" );
			//		 if( !pCell )
			//			 continue;
			//		 std::string strEnd( pCell->GetString() );



			//		 break;
			//	 }
			//}

		}

	}

	// Set Slot.
	if( !m_vItems.empty() )
	{
		CDnItem * pItem = m_vItems[ m_crrShowItem ]->pItem;
		m_pItemSlot->SetItem( pItem, 1 );	
	}

}


void CDnCashShopIngameBanner::Show( bool bShow )
{
//	if( m_bShow == bShow )
//		return;

	CDnCustomDlg::Show( bShow );

	//if( bShow )
	//{		
	//}
	//else
	//{	
	//}

}


void CDnCashShopIngameBanner::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName(pControl->GetControlName());

	if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl( "ID_CHECKBOX_SHOW" ) )
		{
			if( m_pCheckBoxShow )
			{
				// Turn on.
				bool bShow = false;
				if( m_pCheckBoxShow->IsChecked() )
					bShow = true;

				if( m_pItemSlot )
					m_pItemSlot->Show( bShow );
				if( m_pStaticName )
					m_pStaticName->Show( bShow );
				if( m_pStaticBoard )
					m_pStaticBoard->Show( bShow );
				if( m_pStaticPrice )
					m_pStaticPrice->Show( bShow );

				if( m_pStaticShow )
					m_pStaticShow->Show( !bShow );
			}
		}
	}

	CEtUIDialog::ProcessCommand(nCommand, bTriggeredByUser, pControl, uMsg);
}


void CDnCashShopIngameBanner::Process( float dTime )
{
	if( !IsShow() )
		return;

	
	UpdateBanner( dTime );


	CEtUIDialog::Process( dTime );
}


void CDnCashShopIngameBanner::UpdateBanner( float dTime )
{
	if( m_vItems.empty() || !m_pCheckBoxShow->IsChecked() )
		return;

	//SBannerItem & bannerItem = m_vItems[ m_crrShowItem ];
	SBannerItem * bannerItem = m_vItems[ m_crrShowItem ];

	m_AccumulateTime += dTime;

	if( bannerItem->bShow && bannerItem->fShowtime <= m_AccumulateTime )
	{
		m_AccumulateTime = 0.0f;
		++m_crrShowItem;

		if( m_crrShowItem >= (int)m_vItems.size() )
			m_crrShowItem = 0;

		m_pItemSlot->SetItem( bannerItem->pItem, 1 );
		m_pStaticName->SetText( bannerItem->pItem->GetName() );

		SCashShopItemInfo * pCashShopInfo = NULL;
		if( GetCashShopTask().IsActive()  )
		{
			pCashShopInfo = GetCashShopTask().GetItemInfo( (CASHITEM_SN)bannerItem->pItem->GetSerialID() );
			if( pCashShopInfo )
			{
				m_pStaticPrice->ClearText();
				memset( m_strTemp, 0, sizeof(wchar_t)*256 );
				if( pCashShopInfo->priceFix > 0 )
					swprintf_s( m_strTemp, 256, L"%d->%d", pCashShopInfo->price, pCashShopInfo->priceFix );
				else
					swprintf_s( m_strTemp, 256, L"%d", pCashShopInfo->price );
				m_pStaticPrice->SetText( m_strTemp );				
			}
		}

	}

}



bool CDnCashShopIngameBanner::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONUP:
		{
			float fMouseX, fMouseY;
			GetScreenMouseMovePoints( fMouseX, fMouseY );
			fMouseX -= GetXCoord();
			fMouseY -= GetYCoord();
			SUICoord uiCoord;

			if( m_pCheckBoxShow && m_pCheckBoxShow->GetUICoord().IsInside( fMouseX, fMouseY ) == false )
			{
				if( m_pItemSlot && m_pItemSlot->GetUICoord().IsInside( fMouseX, fMouseY ) )
				{
					// ... CashShop Open.

					if (CDnCashShopTask::IsActive())
					{
						MIInventoryItem * pItem = m_pItemSlot->GetItem();
						if( pItem )
						{
							static_cast< CDnCashShopRenewalDlg * >( GetInterface().GetCashShopDlg() )->SetShowIngameBannerItem( std::wstring(static_cast< CDnItem * >( pItem )->GetName()) );
							CDnCashShopTask::GetInstance().RequestCashShopOpen();						
						}
					}
				}

			}
			
		}
		break;
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}


void CDnCashShopIngameBanner::UpdateIngameBannerItem()
{
	//std::vector< SBannerItem * > m_vItems;
	SBannerItem * pBItem = NULL;
	int size = (int)m_vItems.size();
	for( int i=0; i<size; ++i )
	{
		pBItem = m_vItems[ i ];
#ifdef PRE_ADD_LIMITED_CASHITEM
		if( GetCashShopTask().IsActive() && 
			GetCashShopTask().GetCashLimitItemMaxCount( pBItem->pItem->GetSerialID() ) > 0 && 
			GetCashShopTask().GetCashLimitItemRemainCount( pBItem->pItem->GetSerialID() ) == 0 )
		{
			pBItem->bShow = false;	
		}
#endif
	}
}

#endif // PRE_ADD_CASHSHOP_RENEWAL