#include "StdAfx.h"
#include "DnInstantCashShopBuyDlg.h"
#include "DnTableDB.h"
#include "DnCashShopTask.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "CashShopSendPacket.h"
#ifdef PRE_MOD_PETAL_WRITE
#include "DnInterfaceString.h"
#endif // PRE_MOD_PETAL_WRITE


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_ADD_INSTANT_CASH_BUY

CDnInstantCashShopBuyDlg::CDnInstantCashShopBuyDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pItemSlotBtn0( NULL )
, m_pItemSlotBtn1( NULL )
, m_pItem0( NULL )
, m_pItem1( NULL )
, m_pStaticSelect0( NULL )
, m_pStaticSelect1( NULL )
, m_pStaticItemName0( NULL )
, m_pStaticItemName1( NULL )
, m_pStaticItemPeriod0( NULL )
, m_pStaticItemPeriod1( NULL )
, m_pStaticItemCount0( NULL )
, m_pStaticItemCount1( NULL )
, m_pStaticReserve0( NULL )
, m_pStaticReserve1( NULL )
, m_pStaticCash0( NULL )
, m_pStaticCash1( NULL )
, m_pStaticPage( NULL )
, m_pStaticSelectItem( NULL )
, m_pStaticSelectItemCount( NULL )
, m_pStaticSelectItemPeriod( NULL )
, m_pStaticSelectCash( NULL )
, m_pStaticSelectReserve( NULL )
, m_pStaticSelectRemain( NULL )
#ifdef PRE_ADD_CASH_REFUND
, m_pCheckBoxMoveCashInven( NULL )
#endif // PRE_ADD_CASH_REFUND
, m_pCheckBoxBuyReserve( NULL )
#ifdef PRE_ADD_CASHSHOP_CREDIT
, m_pCheckBoxBuyCash( NULL )
, m_pCheckBoxBuyCredit( NULL )
#endif // PRE_ADD_CASHSHOP_CREDIT
, m_pPrevButton( NULL )
, m_pNextButton( NULL )
, m_pOKButton( NULL )
, m_nTotalPage( 0 )
, m_nCurrentPage( 0 )
, m_nSelectedItemIndex( -1 )
, m_pParentCallback( NULL )
, m_pInstantCashShopBuyMessageBox( NULL )
, m_eBuyType( INSTANT_BUY_NONE )
, m_bBuying( false )
, m_nCloseWindow( 0 )
#if defined(PRE_ADD_CASHSHOP_CREDIT)
, m_eBuyMode( CASHSHOP_BUY_NONE )
#endif
{
}

CDnInstantCashShopBuyDlg::~CDnInstantCashShopBuyDlg(void)
{
	m_vecItemSNList.clear();
	SAFE_DELETE( m_pItem0 );
	SAFE_DELETE( m_pItem1 );
	SAFE_DELETE( m_pInstantCashShopBuyMessageBox );
}

void CDnInstantCashShopBuyDlg::Initialize( bool bShow )
{
#if defined(PRE_ADD_CASH_REFUND)
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MiniCashshop_KORDlg.ui" ).c_str(), bShow );
#elif defined(PRE_ADD_CASHSHOP_CREDIT)
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MiniCashshop_USADlg.ui" ).c_str(), bShow );
#else
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MiniCashshopDlg.ui" ).c_str(), bShow );
#endif
}

void CDnInstantCashShopBuyDlg::InitialUpdate()
{
	m_pItemSlotBtn0 = GetControl<CDnItemSlotButton>( "ID_ITEM" );
	m_pItemSlotBtn0->SetSlotType( ST_CASHINSTANCE_ITEM );
	m_pItemSlotBtn1 = GetControl<CDnItemSlotButton>( "ID_BUTTON0" );
	m_pItemSlotBtn1->SetSlotType( ST_CASHINSTANCE_ITEM );
	m_pStaticSelect0 = GetControl<CEtUIStatic>( "ID_STATIC_SELECT0" );
	m_pStaticSelect0->Show( false );
	m_pStaticSelect1 = GetControl<CEtUIStatic>( "ID_STATIC_SELECT1" );
	m_pStaticSelect1->Show( false );
	m_pStaticItemName0 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMNAME0" );
	m_pStaticItemName1 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMNAME1" );
	m_pStaticItemPeriod0 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMPERIOD0" );
	m_pStaticItemPeriod1 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMPERIOD1" );
	m_pStaticItemCount0 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMCOUNT0" );
	m_pStaticItemCount1 = GetControl<CEtUIStatic>( "ID_TEXT_ITEMCOUNT1" );
	m_pStaticReserve0 = GetControl<CEtUIStatic>( "ID_TEXT_RESERVE0" );
	m_pStaticReserve1 = GetControl<CEtUIStatic>( "ID_TEXT_RESERVE1" );
	m_pStaticCash0 = GetControl<CEtUIStatic>( "ID_TEXT_CASH0" );
	m_pStaticCash1 = GetControl<CEtUIStatic>( "ID_TEXT_CASH1" );
	m_pStaticPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );
	m_pStaticSelectItem = GetControl<CEtUIStatic>( "ID_TEXT_ITEMNAME" );
	m_pStaticSelectItemCount = GetControl<CEtUIStatic>( "ID_TEXT_ITEMCOUNT" );
	m_pStaticSelectItemPeriod = GetControl<CEtUIStatic>( "ID_TEXT_ITEMPERIOD" );
	m_pStaticSelectCash = GetControl<CEtUIStatic>( "ID_TEXT_CASH" );
	m_pStaticSelectReserve = GetControl<CEtUIStatic>( "ID_TEXT_RESERVE" );
	m_pStaticSelectRemain = GetControl<CEtUIStatic>( "ID_TEXT_INFO" );
#ifdef PRE_ADD_CASH_REFUND
	m_pCheckBoxMoveCashInven = GetControl<CEtUICheckBox>( "ID_CHECKBOX_SENDCI" );
#endif // PRE_ADD_CASH_REFUND
	m_pCheckBoxBuyReserve = GetControl<CEtUICheckBox>( "ID_CHECKBOX_RESERVE" );
	m_pCheckBoxBuyReserve->Enable( false );
#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_pCheckBoxBuyCash = GetControl<CEtUICheckBox>( "ID_CHECKBOX_CASH" );
	m_pCheckBoxBuyCredit = GetControl<CEtUICheckBox>( "ID_CHECKBOX_CREDIT" );
#endif // PRE_ADD_CASHSHOP_CREDIT
	m_pPrevButton = GetControl<CEtUIButton>( "ID_BT_PREV" );
	m_pNextButton = GetControl<CEtUIButton>( "ID_BT_NEXT" );
	m_pOKButton = GetControl<CEtUIButton>( "ID_BT_OK" );
	m_pOKButton->Enable( false );

	m_pInstantCashShopBuyMessageBox = new CDnInstantCashShopBuyMessageBox( UI_TYPE_MODAL );
	m_pInstantCashShopBuyMessageBox->Initialize( false );
}

void CDnInstantCashShopBuyDlg::EnableUI( bool bEnable )
{
	m_pPrevButton->Enable( bEnable );
	m_pNextButton->Enable( bEnable );
	m_pOKButton->Enable( bEnable );

	CEtUIButton* pCancelButton = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
	if( pCancelButton )
		pCancelButton->Enable( bEnable );
#ifdef PRE_ADD_CASH_REFUND
	m_pCheckBoxMoveCashInven->Enable( bEnable );
#endif // PRE_ADD_CASH_REFUND
	m_pCheckBoxBuyReserve->Enable( bEnable );
#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_pCheckBoxBuyCash->Enable( bEnable );
	m_pCheckBoxBuyCredit->Enable( bEnable );
#endif // PRE_ADD_CASHSHOP_CREDIT
}

bool CDnInstantCashShopBuyDlg::SetBuyType( eInstantCashShopBuyType eBuyType, CEtUICallback* pParentCallback )
{
	m_pParentCallback = pParentCallback;

	ResetData();

	CDnCashShopTask::GetInstance().RequestCashShopBalanceInquiry();

	DNTableFileFormat* pCashBuyShortcut = GetDNTable( CDnTableDB::TCASHBUYSHORTCUT );
	if( pCashBuyShortcut == NULL )
		return false;

	for( int i=0; i<pCashBuyShortcut->GetItemCount(); i++ )
	{
		int nItemIndex = pCashBuyShortcut->GetItemID( i );
		int nType = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_Type" )->GetInteger();

		if( nType == eBuyType )
		{
			m_eBuyType = eBuyType;
			char szLabel[64];

			for( int j=0; j<10; j++ )
			{
				sprintf_s( szLabel, "_SN%02d", j+1 );
				CASHITEM_SN sn = (CASHITEM_SN)pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, szLabel )->GetInteger();
				if( sn > 0 )
				{
					bool bEnableItem = true;
					const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( sn );
					if( pInfo )
					{
						// Requested to exclude SN if it is a costume or a package containing a costume
						if( pInfo->type == eCSType_Costume || pInfo->bLimit )
						{
							bEnableItem = false;
						}
						else if( pInfo->type == eCSType_ePackage )
						{
							const std::vector<CASHITEM_SN>* pPackageItemList = CDnCashShopTask::GetInstance().GetInPackageItemList( sn );
							if (pPackageItemList != NULL)
							{
								std::vector<CASHITEM_SN>::const_iterator iter = pPackageItemList->begin();
								for (;iter != pPackageItemList->end(); ++iter)
								{
									CASHITEM_SN curSn = *iter;
									const SCashShopItemInfo* pCurInfo = GetCashShopTask().GetItemInfo( curSn );
									if( pCurInfo && pCurInfo->type == eCSType_ePackage )
									{
										bEnableItem = false;
										break;
									}
								}
							}
						}
					}
					else
					{
						bEnableItem = false;
					}

					if( bEnableItem )
						m_vecItemSNList.push_back( sn );
				}
			}
		}
	}

	int nSNListCount = static_cast<int>( m_vecItemSNList.size() );

	if( nSNListCount == 0 ) return false;

	m_nTotalPage = nSNListCount / 2;
	m_nTotalPage += nSNListCount % 2 > 0 ? 1 : 0;

	SetItemPage( m_nCurrentPage );

	if( m_nTotalPage == 1 )
	{
		m_pPrevButton->Enable( false );
		m_pNextButton->Enable( false );
	}

#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_pCheckBoxBuyReserve->Enable( false );
	m_pCheckBoxBuyCash->Enable( false );
	m_pCheckBoxBuyCredit->Enable( false );
#endif // PRE_ADD_CASHSHOP_CREDIT

	return true;
}

void CDnInstantCashShopBuyDlg::ResetData()
{
	m_vecItemSNList.clear();
	m_nTotalPage = 0;
	m_nCurrentPage = 0;
	m_nSelectedItemIndex = -1;

	m_pStaticPage->ClearText();
	m_pStaticSelectItem->ClearText();
	m_pStaticSelectCash->ClearText();
	m_pStaticSelectReserve->ClearText();
	m_pStaticSelectRemain->ClearText();
#ifdef PRE_ADD_CASH_REFUND
	m_pCheckBoxMoveCashInven->SetChecked( false );
#endif // PRE_ADD_CASH_REFUND
	m_pCheckBoxBuyReserve->SetChecked( false );
#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_pCheckBoxBuyCash->SetChecked( false );
	m_pCheckBoxBuyCredit->SetChecked( false );
#endif // PRE_ADD_CASHSHOP_CREDIT
	m_eBuyType = INSTANT_BUY_NONE;
	m_bBuying = false;
	m_nCloseWindow = 0;

	ResetUIList();
}

void CDnInstantCashShopBuyDlg::ResetUIList()
{
	m_pStaticSelect0->Show( false );
	m_pStaticSelect1->Show( false );
	m_pStaticItemName0->ClearText();
	m_pStaticItemName1->ClearText();
	m_pStaticItemPeriod0->ClearText();
	m_pStaticItemPeriod1->ClearText();
	m_pStaticItemCount0->ClearText();
	m_pStaticItemCount1->ClearText();
	m_pStaticReserve0->ClearText();
	m_pStaticReserve1->ClearText();
	m_pStaticCash0->ClearText();
	m_pStaticCash1->ClearText();

	SAFE_DELETE( m_pItem0 );
	SAFE_DELETE( m_pItem1 );
	m_pItemSlotBtn0->ResetSlot();
	m_pItemSlotBtn1->ResetSlot();

	m_pPrevButton->Enable( true );
	m_pNextButton->Enable( true );

	EnableUI( true );
	m_pCheckBoxBuyReserve->Enable( false );
	m_pCheckBoxBuyReserve->SetChecked( false );
	m_pOKButton->Enable( false );
	m_nSelectedItemIndex = -1;

	m_pStaticSelectItem->ClearText();
	m_pStaticSelectItemCount->ClearText();
	m_pStaticSelectItemPeriod->ClearText();
	m_pStaticSelectCash->ClearText();
	m_pStaticSelectReserve->ClearText();
	m_pStaticSelectRemain->ClearText();
}

void CDnInstantCashShopBuyDlg::SetItemPage( int nCurrentPage )
{
	if( nCurrentPage > m_nTotalPage )
		return;

	ResetUIList();


	WCHAR wszStr[64];
	swprintf_s( wszStr, L"%d / %d", m_nCurrentPage + 1, m_nTotalPage );
	m_pStaticPage->SetText( wszStr );

	int nIndex = nCurrentPage * 2;
	int nNextIndex = nIndex + 1;
	int nSNListCount = static_cast<int>( m_vecItemSNList.size() );

	std::wstring str;
	if( nIndex < nSNListCount )
	{
		const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( m_vecItemSNList[nIndex] );
		if( pInfo )
		{
			if( pInfo->presentItemId != ITEMCLSID_NONE )
			{
				SAFE_DELETE( m_pItem0 );
				TItemInfo itemInfo;
				if( CDnItem::MakeItemInfo( pInfo->presentItemId, pInfo->count, itemInfo ) )
				{
					m_pItem0 = GetItemTask().CreateItem( itemInfo );
					if( m_pItem0 )
					{
						m_pItem0->SetCashItemSN( pInfo->sn );
						m_pItemSlotBtn0->SetItem( m_pItem0, pInfo->count );
					}
				}
			}

			m_pStaticItemName0->SetText( pInfo->nameString );
			str = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4625), pInfo->count );	// UISTRING : %d ea
			m_pStaticItemCount0->SetText( str.c_str() );
			if( pInfo->period > 0 )
			{
				str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4613 ), pInfo->period ); // UISTRING : (%d days)
				m_pStaticItemPeriod0->SetText( str.c_str() );
			}
			if( pInfo->bReserveOffer )
			{
#ifdef PRE_MOD_PETAL_WRITE
				str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030005 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->ReserveAmount ).c_str() ); // UISTRING : (Earn %s Petals)
#else // PRE_MOD_PETAL_WRITE
				str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4658 ), pInfo->ReserveAmount ); // UISTRING : (Earn %d Petals)
#endif // PRE_MOD_PETAL_WRITE
				m_pStaticReserve0->SetText( str.c_str() );
			}

#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030001 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->price ).c_str() );	// UISTRING : %s Cash
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4642 ), pInfo->price );	// UISTRING : %d Cash
#endif // PRE_MOD_PETAL_WRITE

			m_pStaticCash0->SetText( str.c_str() );
		}
	}

	if( nNextIndex < nSNListCount )
	{
		const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( m_vecItemSNList[nNextIndex] );
		if( pInfo )
		{
			if( pInfo->presentItemId != ITEMCLSID_NONE )
			{
				SAFE_DELETE( m_pItem1 );
				TItemInfo itemInfo;
				if( CDnItem::MakeItemInfo( pInfo->presentItemId, pInfo->count, itemInfo ) )
				{
					m_pItem1 = GetItemTask().CreateItem( itemInfo );
					if( m_pItem1 )
					{
						m_pItem1->SetCashItemSN( pInfo->sn );
						m_pItemSlotBtn1->SetItem( m_pItem1, pInfo->count );
					}
				}
			}

			m_pStaticItemName1->SetText( pInfo->nameString );
			str = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4625), pInfo->count );	// UISTRING : %d ea
			m_pStaticItemCount1->SetText( str.c_str() );
			if( pInfo->period > 0 )
			{
				str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4613 ), pInfo->period ); // UISTRING : (%d days)
				m_pStaticItemPeriod1->SetText( str.c_str() );
			}
			if( pInfo->bReserveOffer )
			{
#ifdef PRE_MOD_PETAL_WRITE
				str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030005 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->ReserveAmount ).c_str() ); // UISTRING : (Earn %s Petals)
#else // PRE_MOD_PETAL_WRITE
				str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4658 ), pInfo->ReserveAmount ); // UISTRING : (Earn %d Petals)
#endif // PRE_MOD_PETAL_WRITE
				m_pStaticReserve1->SetText( str.c_str() );
			}

#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030001 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->price ).c_str() );	// UISTRING : %s Cash
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4642 ), pInfo->price );	// UISTRING : %d Cash
#endif // PRE_MOD_PETAL_WRITE

			m_pStaticCash1->SetText( str.c_str() );
		}
	}

	// Selected UI
	if( m_nSelectedItemIndex == nIndex )
	{
		m_pStaticSelect0->Show( true );
		m_pStaticSelect1->Show( false );

	}
	else if( m_nSelectedItemIndex == nNextIndex )
	{
		m_pStaticSelect0->Show( false );
		m_pStaticSelect1->Show( true );
	}
}

void CDnInstantCashShopBuyDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) )
		{
			m_pInstantCashShopBuyMessageBox->SetMsgBox( MESSAGEBOX_CONFIRM, this );
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
		}
		else if( IsCmdControl( "ID_BT_PREV" ) )
		{
			PrevPage();
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			NextPage();
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( m_nSelectedItemIndex >= 0 && m_nSelectedItemIndex < static_cast<int>( m_vecItemSNList.size() ) )
		{
			int nSelectedSN = m_vecItemSNList[m_nSelectedItemIndex];

			const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( nSelectedSN );
			if( pInfo )
			{
#ifdef PRE_ADD_CASHSHOP_CREDIT
				if( IsCmdControl( "ID_CHECKBOX_CASH" ) )
				{
					if( m_pCheckBoxBuyCash->IsChecked() )
					{
						m_pCheckBoxBuyReserve->SetChecked( false );
						m_pCheckBoxBuyCredit->SetChecked( false );
						m_eBuyMode = CASHSHOP_BUY_CASH;
					}
					UpdateBuyInfo();
				}
				else if( IsCmdControl( "ID_CHECKBOX_RESERVE" ) )
				{
					if( m_pCheckBoxBuyReserve->IsChecked() )
					{
						m_pCheckBoxBuyCash->SetChecked( false );
						m_pCheckBoxBuyCredit->SetChecked( false );
						m_eBuyMode = CASHSHOP_BUY_RESERVE;
					}
					UpdateBuyInfo();
				}
				else if( IsCmdControl( "ID_CHECKBOX_CREDIT" ) )
				{
					if( m_pCheckBoxBuyCredit->IsChecked() )
					{
						if( !pInfo->bCreditAble )
						{
							WCHAR wzStr[1024] = {0,};
							swprintf_s( wzStr, _countof(wzStr), L"%s\n%s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4839), pInfo->nameString.c_str() );
							GetInterface().MessageBox( wzStr, MB_OK, MESSAGEBOX_CREDIT_ERROR, this );
						}

						m_pCheckBoxBuyCash->SetChecked( false );
						m_pCheckBoxBuyReserve->SetChecked( false );
						m_eBuyMode = CASHSHOP_BUY_CREDIT;
					}
					UpdateBuyInfo();
				}
				else
				{
					m_eBuyMode = CASHSHOP_BUY_NONE;
				}
#else // PRE_ADD_CASHSHOP_CREDIT
				if( IsCmdControl( "ID_CHECKBOX_RESERVE" ) )
				{
					UpdateBuyInfo();
#ifdef PRE_ADD_CASH_REFUND
					if( m_pCheckBoxBuyReserve->IsChecked() )
					{
						m_pCheckBoxMoveCashInven->SetChecked( true );
						m_pCheckBoxMoveCashInven->Enable( false );
					}
					else
					{
						m_pCheckBoxMoveCashInven->SetChecked( false );
						m_pCheckBoxMoveCashInven->Enable( true );
					}
#endif // PRE_ADD_CASH_REFUND
				}
#endif // PRE_ADD_CASHSHOP_CREDIT
#ifdef PRE_ADD_CASH_REFUND
				if( IsCmdControl( "ID_CHECKBOX_SENDCI" ) )
				{
					UpdateBuyInfo();
				}
#endif // PRE_ADD_CASH_REFUND
			}
		}
	}
}

void CDnInstantCashShopBuyDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
		case MESSAGEBOX_CONFIRM:
			{
				if( strcmp( pControl->GetControlName(), "ID_BT_OK" ) == 0 ) 
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4774 ), MB_OK, MESSAGEBOX_REQUEST_BUY, this );
					m_bBuying = true;
					EnableUI( false );

					int nSelectedSN = m_vecItemSNList[m_nSelectedItemIndex];

					const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( nSelectedSN );
					if( CDnCashShopTask::IsActive() && pInfo )
					{
						int abilityCount = GetCashShopTask().GetValidAbilityCount( pInfo->abilityList );

						ITEMCLSID ability;
						if( abilityCount == 1 )
							ability = pInfo->abilityList[0];
						else
							ability = ITEMCLSID_NONE;
					
						std::vector<TCashShopInfo> infoList;
						TCashShopInfo info;
						info.nItemID = ability;
						info.nItemSN = pInfo->sn;
						info.cSlotIndex = pInfo->id;
						infoList.push_back( info );

						char cPaymentRules = Cash::PaymentRules::None;

						if( m_pCheckBoxBuyReserve->IsChecked() )
							cPaymentRules = Cash::PaymentRules::Petal;

#if defined(PRE_ADD_CASHSHOP_CREDIT)
						GetCashShopTask().SetBuyMode( m_eBuyMode );
#endif
#ifdef PRE_ADD_CASH_REFUND
						SendCashShopBuy( eCashUnit_BuyInstant + m_eBuyType, cPaymentRules, infoList, true );
#else // PRE_ADD_CASH_REFUND
						SendCashShopBuy( eCashUnit_BuyInstant + m_eBuyType, cPaymentRules, infoList );
#endif // PRE_ADD_CASH_REFUND
					}
				}
			}
			break;
		case MESSAGEBOX_RECIEVE_RESULT:
			{
				if( m_pParentCallback )
				{
					m_pParentCallback->OnUICallbackProc( INSTANT_CASHSHOP_BUY_DIALOG, m_nCloseWindow, NULL );
				}

				Show( false );
			}
			break;
#ifdef PRE_ADD_CASHSHOP_CREDIT
		case MESSAGEBOX_CREDIT_ERROR:
			{
				if( strcmp( pControl->GetControlName(), "ID_OK" ) == 0 )
				{
					m_pCheckBoxBuyCredit->SetChecked( false );
				}
			}
			break;
#endif // PRE_ADD_CASHSHOP_CREDIT
	}
}

bool CDnInstantCashShopBuyDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( !m_bBuying )
	{
		if( uMsg == WM_LBUTTONDOWN )
		{
			POINT MousePoint;
			float fMouseX, fMouseY;

			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );
			PointToFloat( MousePoint, fMouseX, fMouseY );

			int nSelectedItemIndex = -1;
			bool bSelect = false;
			SUICoord uiCoordsSelect;

			m_pStaticSelect0->GetUICoord( uiCoordsSelect );
			if( uiCoordsSelect.IsInside( fMouseX, fMouseY ) )
			{
				nSelectedItemIndex = m_nCurrentPage * 2;
				if( nSelectedItemIndex >= 0 && nSelectedItemIndex < static_cast<int>( m_vecItemSNList.size() ) )
				{
					bSelect = true;
					m_pStaticSelect0->Show( true );
					m_pStaticSelect1->Show( false );
				}
			}
			
			if( !bSelect )
			{
				m_pStaticSelect1->GetUICoord( uiCoordsSelect );
				if( uiCoordsSelect.IsInside( fMouseX, fMouseY ) )
				{
					nSelectedItemIndex = ( m_nCurrentPage * 2 ) + 1;
					if( nSelectedItemIndex >= 0 && nSelectedItemIndex < static_cast<int>( m_vecItemSNList.size() ) )
					{
						bSelect = true;
						m_pStaticSelect0->Show( false );
						m_pStaticSelect1->Show( true );
					}
				}
			}

			if( bSelect )
				SetSelectedItemInfo( nSelectedItemIndex );
		}
		else if( uMsg == WM_MOUSEWHEEL )
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
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnInstantCashShopBuyDlg::PrevPage()
{
	if( m_nTotalPage == 1 )
		return;

	m_nCurrentPage--;

	if( m_nCurrentPage < 0 )
	{
		m_nCurrentPage = m_nTotalPage - 1;
	}

	SetItemPage( m_nCurrentPage );
}

void CDnInstantCashShopBuyDlg::NextPage()
{
	if( m_nTotalPage == 1 )
		return;

	m_nCurrentPage++;

	if( m_nCurrentPage >= m_nTotalPage )
	{
		m_nCurrentPage = 0;
	}

	SetItemPage( m_nCurrentPage );
}

void CDnInstantCashShopBuyDlg::SetSelectedItemInfo( int nSelectedItemIndex )
{
	if( nSelectedItemIndex < 0 || nSelectedItemIndex >= static_cast<int>( m_vecItemSNList.size() ) )
		return;

	if( m_nSelectedItemIndex == nSelectedItemIndex )
		return;

	m_nSelectedItemIndex = nSelectedItemIndex;
#ifdef PRE_ADD_CASH_REFUND
	m_pCheckBoxMoveCashInven->SetChecked( false );
#endif // PRE_ADD_CASH_REFUND
	m_pCheckBoxBuyReserve->Enable( true );
	m_pCheckBoxBuyReserve->SetChecked( false );
#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_pCheckBoxBuyCash->SetChecked( false );
	m_pCheckBoxBuyCredit->SetChecked( false );
	m_pCheckBoxBuyReserve->Enable( true );
	m_pCheckBoxBuyCash->Enable( true );
	m_pCheckBoxBuyCredit->Enable( true );
#endif // PRE_ADD_CASHSHOP_CREDIT
	m_pStaticSelectItemPeriod->ClearText();

	int nSelectedSN = m_vecItemSNList[m_nSelectedItemIndex];

	const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( nSelectedSN );
	if( pInfo )
	{
		std::wstring str;
		m_pStaticSelectItem->SetText( pInfo->nameString.c_str() );
		str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4625 ), pInfo->count ); // UISTRING : %d ea
		m_pStaticSelectItemCount->SetText( str.c_str() );
		if( pInfo->period > 0 )
		{
			str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4613 ), pInfo->period ); // UISTRING : (%d days)
			m_pStaticSelectItemPeriod->SetText( str.c_str() );
		}

		if( pInfo->bReserveUsable == false )
			m_pCheckBoxBuyReserve->Enable( false );

		if( pInfo->bReserveOffer )
		{
#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030005 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->ReserveAmount ).c_str() ); // UISTRING : (Earn %s Petals)
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4658 ), pInfo->ReserveAmount ); // UISTRING : (Earn %d Petals)
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectReserve->SetText( str.c_str() );
		}
		else
		{
			m_pStaticSelectReserve->ClearText();
		}

#ifdef PRE_MOD_PETAL_WRITE
		str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030001 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->price ).c_str() );	// UISTRING : %s Cash
#else // PRE_MOD_PETAL_WRITE
		str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4642 ), pInfo->price );	// UISTRING : %d Cash
#endif // PRE_MOD_PETAL_WRITE
		m_pStaticSelectCash->SetText( str.c_str() );
		UpdateBuyInfo();
	}
}

void CDnInstantCashShopBuyDlg::UpdateBuyInfo()
{
	if( m_nSelectedItemIndex < 0 || m_nSelectedItemIndex >= static_cast<int>( m_vecItemSNList.size() ) )
		return;

	int nSelectedSN = m_vecItemSNList[m_nSelectedItemIndex];

	const SCashShopItemInfo* pInfo = CDnCashShopTask::GetInstance().GetItemInfo( nSelectedSN );
	if( pInfo )
	{
		std::wstring str;
		int userReserve = GetCashShopTask().GetUserReserve();
#ifdef PRE_ADD_CASHSHOP_CREDIT
		int userCredit	= GetCashShopTask().GetUserCredit();
		int userCash	= GetCashShopTask().GetUserPrepaid();
#else // PRE_ADD_CASHSHOP_CREDIT
		int userCash	= GetCashShopTask().GetUserCash();
#endif // PRE_ADD_CASHSHOP_CREDIT

		bool bEnableBuy = false;
		if( m_pCheckBoxBuyReserve->IsChecked() )
		{
			int rest = userReserve - pInfo->price;
#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030007 ), DN_INTERFACE::UTIL::GetAddCommaString( rest ).c_str() );	// UISTRING : Remaining Points : %s Petal
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4708 ), userReserve - pInfo->price );	// UISTRING : Remaining Points : %d Petal
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectRemain->SetTextColor( (userReserve - pInfo->price < 0) ? 0xffff0000 : 0xffffffff );
			m_pStaticSelectRemain->SetText( str.c_str() );

#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030002 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->price ).c_str() );	// UISTRING : %s Petal
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4647 ), pInfo->price );	// UISTRING : %d Petal
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectCash->SetText( str.c_str() );
			if( rest >= 0 )
				bEnableBuy = true;
		}
#ifdef PRE_ADD_CASHSHOP_CREDIT
		else if( m_pCheckBoxBuyCash->IsChecked() )
		{
			int rest = userCash - pInfo->price;
			m_pStaticSelectRemain->SetTextColor( (rest < 0) ? 0xffff0000 : 0xffffffff );
#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030006 ), DN_INTERFACE::UTIL::GetAddCommaString( rest ).c_str() );		// UISTRING : Remaining Cash : %s Cash
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4707 ), rest );		// UISTRING : Remaining Cash : %d Cash
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectRemain->SetText( str.c_str() );
			if( rest >= 0 )
				bEnableBuy = true;
		}
		else if( m_pCheckBoxBuyCredit->IsChecked() )
		{
			int rest = userCredit - pInfo->price;
			m_pStaticSelectRemain->SetTextColor( (rest < 0) ? 0xffff0000 : 0xffffffff );
#ifdef PRE_MOD_PETAL_WRITE
			std::wstring strFormat = FormatW( L"%s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4844), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2030001) );	// UISTRING : Remaining NxCredit : %s NX
			str = FormatW( strFormat.c_str(), DN_INTERFACE::UTIL::GetAddCommaString( rest ).c_str() );
#else // PRE_MOD_PETAL_WRITE
			std::wstring strFormat = FormatW( L"%s %s", GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4844), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4845) );		// UISTRING : Remaining NxCredit : %d NX
			str = FormatW( strFormat.c_str(), rest );
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectRemain->SetText( str.c_str() );
			if( rest >= 0 )
				bEnableBuy = true;
		}
		else
			m_pStaticSelectRemain->SetText( L"" );
#else // PRE_ADD_CASHSHOP_CREDIT
		else
		{
			int rest = userCash - pInfo->price;
			m_pStaticSelectRemain->SetTextColor((rest < 0) ? 0xffff0000 : 0xffffffff);
#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030006 ), DN_INTERFACE::UTIL::GetAddCommaString( rest ).c_str() );		// UISTRING : Remaining Cash : %s Cash
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4707 ), rest );		// UISTRING : Remaining Cash : %d Cash
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectRemain->SetText( str.c_str() );

#ifdef PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030001 ), DN_INTERFACE::UTIL::GetAddCommaString( pInfo->price ).c_str() );	// UISTRING : %s Cash
#else // PRE_MOD_PETAL_WRITE
			str = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4642 ), pInfo->price );	// UISTRING : %d Cash
#endif // PRE_MOD_PETAL_WRITE
			m_pStaticSelectCash->SetText( str.c_str() );
			if( rest >= 0 )
				bEnableBuy = true;

		}
#endif // PRE_ADD_CASHSHOP_CREDIT

#ifdef PRE_ADD_CASH_REFUND
		if( !m_pCheckBoxMoveCashInven->IsChecked() )
			bEnableBuy = false;
#endif // PRE_ADD_CASH_REFUND

		m_pOKButton->Enable( bEnableBuy );

	}
}

void CDnInstantCashShopBuyDlg::OnRecvInstantBuyResult( SCCashShopBuy* pPacket )
{
	if( pPacket == NULL )
		return;

	switch( pPacket->nRet )
	{
	case ERROR_NONE:
		{
			DNTableFileFormat* pCashBuyShortcut = GetDNTable( CDnTableDB::TCASHBUYSHORTCUT );
			if( pCashBuyShortcut == NULL )
				return;

			for( int i=0; i<pCashBuyShortcut->GetItemCount(); i++ )
			{
				int nItemIndex = pCashBuyShortcut->GetItemID( i );
				int nType = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_Type" )->GetInteger();

				if( nType == m_eBuyType )
				{
					m_nCloseWindow = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_Window" )->GetInteger();
					break;
				}
			}

			// m_nCloseWindow -> 1: close window, 0: refresh window immediately
			if( m_nCloseWindow == 0 )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4700 ), MB_OK, MESSAGEBOX_RECIEVE_RESULT, this );
			}
			else if( m_nCloseWindow == 1 )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4908 ), MB_OK, MESSAGEBOX_RECIEVE_RESULT, this );
			}

			m_bBuying = false;
			EnableUI( true );
		}
		break;
	case 558:
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4839 )); // UISTRING : Your cart contains an item that cannot be purchased with Nx Credit
			Show( false );
		}
		break;
	default:
		{
			std::wstring str;
			GetInterface().GetServerMessage( str, pPacket->nRet );

			if( pPacket->cCount > 0 )
			{
				int i = 0;
				for( ; i < pPacket->cCount; ++i )
				{
					if( i == 0 )
						str.append( L"\n" );

					const SCashShopItemInfo* pItemInfo = CDnCashShopTask::GetInstance().GetItemInfo( pPacket->BuyList[i].nItemSN );
					if( pItemInfo )
					{
						std::wstring temp;
						temp = FormatW( L"%s / ", pItemInfo->nameString.c_str() );
						str.append( temp.c_str() );
					}
				}
			}

			GetInterface().MessageBox( str.c_str(), MB_OK );
			Show( false );
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
// CDnInstantCashShopBuyMessageBox


CDnInstantCashShopBuyMessageBox::CDnInstantCashShopBuyMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
{
}

CDnInstantCashShopBuyMessageBox::~CDnInstantCashShopBuyMessageBox(void)
{
}

void CDnInstantCashShopBuyMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize(CEtResourceMng::GetInstance().GetFullName( "MiniCashshopMessageBox.ui" ).c_str(), bShow);
}

void CDnInstantCashShopBuyMessageBox::InitialUpdate()
{
}

void CDnInstantCashShopBuyMessageBox::SetMsgBox( int nID, CEtUICallback* pCallBack )
{
	SetDialogID( nID );
	SetCallback( pCallBack );
	Show( true );
}

void CDnInstantCashShopBuyMessageBox::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg)
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show( false );
	}
}

#endif // PRE_ADD_INSTANT_CASH_BUY