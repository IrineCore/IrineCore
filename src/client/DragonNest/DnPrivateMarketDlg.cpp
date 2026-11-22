#include "StdAfx.h"
#include "DnPrivateMarketDlg.h"
#include "DnItem.h"
#include "DnInterface.h"
#include "DnTradeTask.h"
#include "DnMoneyInputDlg.h"
#include "DnMainMenuDlg.h"
#include "DnItemTask.h"
#include "DnInterfaceString.h"
#include "DnLocalPlayerActor.h"
#include "DnStoreConfirmExDlg.h"
#include "DnCommonUtil.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPrivateMarketDlg::CDnPrivateMarketDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticGold_You(NULL)
	, m_pStaticSilver_You(NULL)
	, m_pStaticBronze_You(NULL)
	, m_pStaticGold_Me(NULL)
	, m_pStaticSilver_Me(NULL)
	, m_pStaticBronze_Me(NULL)
	, m_pButtonRegist(NULL)
	, m_pButtonOK(NULL)
	, m_pButtonCancel(NULL)
	, m_pButtonInputMoney(NULL)
	, m_pMoneyInputDlg(NULL)
	, m_pStaticNotice(NULL)
	, m_pStaticBalloonMe(NULL)
	, m_pStaticChatMe(NULL)
	, m_pStaticBalloonYou(NULL)
	, m_pStaticChatYou(NULL)
	, m_pStaticMe(NULL)
	, m_pStaticYou(NULL)
	, m_fBalloonMeShowTime(0.f)
	, m_fBalloonYouShowTime(0.f)
	, m_bReadyMe(false)
	, m_bReadyYou(false)
	, m_bComfirm(false)
	, m_pSplitConfirmExDlg(NULL)
	, m_pStaticGold_Tax(NULL)
	, m_pStaticSilver_Tax(NULL)
	, m_pStaticBronze_Tax(NULL)
	, m_nTradeMoney(0)
	, m_nTaxMoney(0)	
{
	SecureZeroMemory(m_strYourName, sizeof(m_strYourName));
}

CDnPrivateMarketDlg::~CDnPrivateMarketDlg(void)
{
	SAFE_DELETE( m_pMoneyInputDlg );
	SAFE_DELETE_PVEC( m_vecItem_Me );
	SAFE_DELETE(m_pSplitConfirmExDlg);
}

void CDnPrivateMarketDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PrivateMarketDlg.ui" ).c_str(), bShow );

	if( CDnTradeTask::IsActive() )
	{
		GetTradeTask().GetTradePrivateMarket().SetPrivateMarketDialog( this );
	}
	else
	{
		CDebugSet::ToLogFile( "CDnMailDlg::Initialize, Trade task must be created before private market dialog is created." );
	}
}

void CDnPrivateMarketDlg::InitialUpdate()
{
	m_pStaticGold_You = GetControl<CEtUIStatic>("ID_GOLD_YOU");
	m_pStaticSilver_You = GetControl<CEtUIStatic>("ID_SILVER_YOU");
	m_pStaticBronze_You = GetControl<CEtUIStatic>("ID_BRONZE_YOU");
	m_pStaticGold_Me = GetControl<CEtUIStatic>("ID_GOLD_ME");
	m_pStaticSilver_Me = GetControl<CEtUIStatic>("ID_SILVER_ME");
	m_pStaticBronze_Me = GetControl<CEtUIStatic>("ID_BRONZE_ME");

	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");
	m_pButtonRegist = GetControl<CEtUIButton>("ID_BUTTON_REGIST");
	m_pButtonInputMoney = GetControl<CEtUIButton>("ID_BUTTON_INPUTMONEY");

	m_pStaticBalloonMe = GetControl<CEtUIStatic>("ID_BALLOON_ME");
	m_pStaticChatMe = GetControl<CEtUIStatic>("ID_CHAT_ME");
	m_pStaticBalloonYou = GetControl<CEtUIStatic>("ID_BALLOON_YOU");
	m_pStaticChatYou = GetControl<CEtUIStatic>("ID_CHAT_YOU");

	m_pStaticMe = GetControl<CEtUIStatic>("ID_STATIC_ME");
	m_pStaticYou = GetControl<CEtUIStatic>("ID_STATIC_YOU");

	m_pStaticGold_Tax = GetControl<CEtUIStatic>("ID_GOLD_TAX");
	m_pStaticSilver_Tax = GetControl<CEtUIStatic>("ID_SILVER_TAX");
	m_pStaticBronze_Tax = GetControl<CEtUIStatic>("ID_BRONZE_TAX");

	m_pStaticBalloonMe->Show( false );
	m_pStaticChatMe->Show( false );
	m_pStaticBalloonYou->Show( false );
	m_pStaticChatYou->Show( false );
	
	m_pStaticNotice = GetControl<CEtUIStatic>("ID_STATIC_NOTICE");

	char szTemp[32]={0};
	
	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		sprintf_s( szTemp, 32, "ID_ITEMSLOT_YOU_%02d", i );
		CDnItemSlotButton *pItemSlotButtonYou = GetControl<CDnItemSlotButton>(szTemp);
		pItemSlotButtonYou->SetSlotType( ST_EXCHANGE );
		m_vecItemSlotButton_You.push_back( pItemSlotButtonYou );

		sprintf_s( szTemp, 32, "ID_ITEMSLOT_ME_%02d", i );
		CDnItemSlotButton *pItemSlotButton = GetControl<CDnItemSlotButton>(szTemp);
		pItemSlotButton->SetSlotIndex( i );
		pItemSlotButton->SetSlotType( ST_EXCHANGE );
		m_vecItemSlotButton_Me.push_back( pItemSlotButton );
		m_vecItem_Me.push_back(NULL);
	}

	m_vecQuickSlotButton.resize(PRIVATE_MARKET_ITEMSLOT_MAX);

	m_pMoneyInputDlg = new CDnMoneyInputDlg( UI_TYPE_MODAL, NULL, MONEY_INPUT_DIALOG, this );
	m_pMoneyInputDlg->Initialize( false );

	m_pSplitConfirmExDlg = new CDnStoreConfirmExDlg( UI_TYPE_MODAL, NULL, ITEM_ATTACH_CONFIRM_DIALOG, this );
	m_pSplitConfirmExDlg->Initialize( false );

	m_strYourName[0] = '\0';
}

void CDnPrivateMarketDlg::OnCancelTrade()
{
	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		if( m_vecQuickSlotButton[i] )
			m_vecQuickSlotButton[i]->DisableSplitMode(true);
	}

	drag::ReleaseControl();
}

void CDnPrivateMarketDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		OnResetControl();
		CDnLocalPlayerActor::LockInput( true );

		if( drag::IsValid() )
		{
			drag::Command(UI_DRAG_CMD_CANCEL);
			drag::ReleaseControl();
		}

		GetInterface().CloseItemUnsealDialog();
		GetInterface().ShowItemSealDialog( false );
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		GetInterface().ShowItemPotentialTransferDialog( false );
#endif
	}
	else
	{
		if (GetInterface().IsOpenBlind() == false)
			CDnLocalPlayerActor::LockInput( false );

		// Restore quickslot color changes when the window closes.
		for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
		{
			if( m_vecQuickSlotButton[i] )
			{
				m_vecQuickSlotButton[i]->SetRegist( false );
				m_vecQuickSlotButton[i]->DisableSplitMode( true );
				m_vecQuickSlotButton[i] = NULL;
			}
		}

		// If the other party cancels while the money input dialog is open, must close it manually.
		if( m_pMoneyInputDlg->IsShow() )
			m_pMoneyInputDlg->Show( false );

		if( m_pSplitConfirmExDlg->IsShow() )
		{
			CDnSlotButton* pFromBtn = m_pSplitConfirmExDlg->GetFromSlot();
			if (pFromBtn)
			{
				pFromBtn->SetRegist(false);
				pFromBtn->DisableSplitMode(true);
			}
			m_pSplitConfirmExDlg->Show( false );
		}
	}

	CEtUIDialog::Show( bShow );
//	CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

bool CDnPrivateMarketDlg::CheckReversionItemTradable(const CDnItem* pItem)
{
	if (pItem->GetReversion() == CDnItem::Belong && pItem->IsSoulbBound())
	{
		eItemTypeEnum type = pItem->GetItemType();
		if (type == ITEMTYPE_WEAPON || 
			type == ITEMTYPE_PARTS ||
			type == ITEMTYPE_GLYPH ||
#if defined(PRE_ADD_TALISMAN_SYSTEM)
			type == ITEMTYPE_TALISMAN ||
#endif
			type == ITEMTYPE_POTENTIAL_JEWEL)
		{
			if (pItem->GetSealCount() > 0)
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3626 ), MB_OK ); // UISTRING : Re-sealable bound items must be sealed with a Sealing Stamp to trade.
				return false;
			}
		}

		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4035 ), MB_OK ); // UISTRING : This item cannot be traded
		return false;
	}
	else if (pItem->GetReversion() == CDnItem::GuildBelong)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4035 ), MB_OK ); // UISTRING : This item cannot be traded
		return false;
	}

	return true;
}

bool CDnPrivateMarketDlg::IsTradableItem(CDnSlotButton* pSlotBtn, CDnItem* pItem)
{
	if( pSlotBtn->GetSlotType() != ST_INVENTORY )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3610 ), MB_OK );
		return false;
	}

	// Skip if re-registering an already listed item.
	if( CheckItemDuplication( pItem ) )
		return false;

	if( pItem->GetItemType() == ITEMTYPE_QUEST )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
		return false;
	}

	if (CheckReversionItemTradable(pItem) == false)
		return false;

	return true;
}

void CDnPrivateMarketDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_INPUTMONEY" ) )
		{
			m_pMoneyInputDlg->Show( true );
			return;
		}

		if( IsCmdControl( "ID_BUTTON_REGIST" ) )
		{
			// Check coins. Insufficient fee message 4036
			if( m_nTradeMoney + m_nTaxMoney > GetItemTask().GetCoin() ) {
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4036 ), MB_OK );
				return;
			}

			OnClickButtonRegist_One();
			return;
		}

		if( IsCmdControl( "ID_BUTTON_OK" ) )
		{
			OnClickButtonConfirm_One();
			return;
		}

		if( strstr( pControl->GetControlName(), "ID_ITEMSLOT_ME" ) )
		{
			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			CDnSlotButton *pPressedButton = ( CDnItemSlotButton * )pControl;

			if( ((CDnItemSlotButton*)pPressedButton)->GetItemState() == ITEMSLOT_DISABLE )
			{
				if( pDragButton )
				{
					pDragButton->DisableSplitMode(true);
					drag::ReleaseControl();
				}
				return;
			}

			if( uMsg == WM_RBUTTONUP )
			{
				SAFE_DELETE(m_vecItem_Me[pPressedButton->GetSlotIndex()]);
				pPressedButton->ResetSlot();
				GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketDeleteItem( pPressedButton->GetSlotIndex() );

				if( m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] )
				{
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist( false );
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->DisableSplitMode(true);
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = NULL;
				}
				RefreshTax();
				return;
			}

			if( pDragButton )
			{
				//pDragButton->DisableSplitMode(false);
				//drag::ReleaseControl();

				CDnItem *pItem = static_cast<CDnItem *>(pDragButton->GetItem());
				if (pItem == NULL || IsTradableItem(pDragButton, pItem) == false)
				{
					drag::Command(UI_DRAG_CMD_CANCEL);
					drag::ReleaseControl();
					return;
				}

				// If placing another item on a slot that already has one,
				CDnItem *pPressedItem = static_cast<CDnItem *>(pPressedButton->GetItem());
				if( pPressedItem )
				{
					// Remove the item from the trade window.
					SAFE_DELETE(m_vecItem_Me[pPressedButton->GetSlotIndex()]);
					pPressedButton->ResetSlot();
					GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketDeleteItem( pPressedButton->GetSlotIndex() );
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist( false );
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->DisableSplitMode(true);;
					m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = NULL;
				}

				// When registering an item to trade, instead of passing the inventory item pointer directly as before,
				//pPressedButton->SetItem( pItem );
				// Create and insert a temporary item like this.
				TItemInfo itemInfo;
				pItem->GetTItemInfo(itemInfo);
				CDnItem *pNewMyItem = GetItemTask().CreateItem( itemInfo );
				m_vecItem_Me[pPressedButton->GetSlotIndex()] = pNewMyItem;
				pPressedButton->SetItem(pNewMyItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

				GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketAddItem( pPressedButton->GetSlotIndex(), pItem->GetSlotIndex(), pItem->GetOverlapCount(), pItem->GetSerialID() );
				SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3612 ) );

				m_vecQuickSlotButton[pPressedButton->GetSlotIndex()] = (CDnQuickSlotButton *)pDragButton;
				m_vecQuickSlotButton[pPressedButton->GetSlotIndex()]->SetRegist( true );

				drag::ReleaseControl();
				RefreshTax();
			}

			return;
		}

		if( IsCmdControl("ID_CLOSE_DIALOG") || IsCmdControl("ID_BUTTON_CANCEL") )
		{
			for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
			{
				if( m_vecQuickSlotButton[i] )
					m_vecQuickSlotButton[i]->DisableSplitMode(true);
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

DWORD CDnPrivateMarketDlg::GetGoldCoinColor( INT64 nGold )
{
	DWORD dwColor = D3DCOLOR_ARGB(255,255,174,18);

	if( nGold >= 100000 )
	{
		dwColor = D3DCOLOR_ARGB(255,255,209,47);
	}
	else if( nGold >= 10000 )
	{
		dwColor = D3DCOLOR_ARGB(255,168,134,255);
	}
	else if( nGold >= 1000 )
	{
		dwColor = D3DCOLOR_ARGB(255,255,145,72);
	}
	else if( nGold >= 100 )
	{
		dwColor = D3DCOLOR_ARGB(255,121,146,254);
	}
	else if( nGold >= 10 )
	{
		dwColor = D3DCOLOR_ARGB(255,136,171,7);
	}

	return dwColor;
}

void CDnPrivateMarketDlg::OnUpdateMoney_Other( INT64 nMoney )
{
	INT64 nGold = nMoney/10000;
	INT64 nSilver = (nMoney%10000)/100;
	INT64 nBronze = nMoney%100;
	std::wstring strString;

	m_pStaticGold_You->SetTextColor( GetGoldCoinColor( nGold ) );
	m_pStaticGold_You->SetInt64ToText( nGold );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nGold, strString );
	m_pStaticGold_You->SetTooltipText( strString.c_str() );

	m_pStaticSilver_You->SetInt64ToText( nSilver );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nSilver, strString );
	m_pStaticSilver_You->SetTooltipText( strString.c_str() );

	m_pStaticBronze_You->SetInt64ToText( nBronze );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nBronze, strString );
	m_pStaticBronze_You->SetTooltipText( strString.c_str() );

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3608 ) );

	if( m_bReadyMe )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3622 ) );
}

void CDnPrivateMarketDlg::OnUpdateMoney_One( INT64 nMoney )
{
	INT64 nCoin = GetItemTask().GetCoin();
	if( nCoin < nMoney ) nMoney = nCoin;

	m_nTradeMoney = nMoney;
	INT64 nGold = nMoney/10000;
	INT64 nSilver = (nMoney%10000)/100;
	INT64 nBronze = nMoney%100;
	std::wstring strString;

	m_pStaticGold_Me->SetTextColor( GetGoldCoinColor( nGold ) );
	m_pStaticGold_Me->SetInt64ToText( nGold );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nGold, strString );
	m_pStaticGold_Me->SetTooltipText( strString.c_str() );

	m_pStaticSilver_Me->SetInt64ToText( nSilver );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nSilver, strString );
	m_pStaticSilver_Me->SetTooltipText( strString.c_str() );

	m_pStaticBronze_Me->SetInt64ToText( nBronze );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nBronze, strString );
	m_pStaticBronze_Me->SetTooltipText( strString.c_str() );

	// Note : Send amount to server.
	//
	GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketCoin( nMoney );

	RefreshTax();

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3612 ) );
}

void CDnPrivateMarketDlg::OnAddItem_Other( int nIndex, CDnItem *pItem )
{
	ASSERT( pItem );
	ASSERT( (nIndex>=0) && (nIndex<PRIVATE_MARKET_ITEMSLOT_MAX) );
	m_vecItemSlotButton_You[nIndex]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

	if( !m_bReadyMe )
		SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3613 ) );
	else {
		SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3622 ), true, 0xFFFF5522 );
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3622 ) );
	}
}

void CDnPrivateMarketDlg::OnDelItem_Other( int nIndex )
{
	ASSERT( (nIndex>=0) && (nIndex<PRIVATE_MARKET_ITEMSLOT_MAX) );
	m_vecItemSlotButton_You[nIndex]->ResetSlot();

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3614 ) );

	if( m_bReadyMe )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3622 ) );
}

void CDnPrivateMarketDlg::OnClickButtonRegist_Other()
{
	m_bReadyYou = true;

	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		m_vecItemSlotButton_You[i]->SetItemState( ITEMSLOT_DISABLE );
	}

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3615 ) );
}

void CDnPrivateMarketDlg::OnClickButtonRegist_One()
{
	m_pButtonInputMoney->Enable(false);
	m_pButtonRegist->Enable(false);
	m_bReadyMe = true;

	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		m_vecItemSlotButton_Me[i]->SetItemState( ITEMSLOT_DISABLE );
	}

	GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketConfirm( EXCHANGE_REGIST );

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3616 ) );
}

//void CDnPrivateMarketDlg::OnClickButtonCancel_Other()
//{
//	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
//	{
//		m_vecItemSlotButton_You[i]->SetItemState( ITEMSLOT_ENABLE );
//	}
//
//	SetNotice( L"The other party clicked the [Cancel] button." );
//}

//void CDnPrivateMarketDlg::OnClickButtonCancel_One()
//{
//	m_pButtonInputMoney->Enable(true);
//	m_pButtonOK->Enable(true);
//	m_pButtonRegist->Enable(false);
//
//	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
//	{
//		m_vecItemSlotButton_Me[i]->SetItemState( ITEMSLOT_ENABLE );
//	}
//
//	// Note : Send cancel state to server.
//	//
//	GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketConfirm( false );
//
//	SetNotice( L"Clicked the [Cancel] button." );
//}

void CDnPrivateMarketDlg::OnResetControl()
{
	m_pStaticGold_You->ClearText();
	m_pStaticGold_You->ClearTooltipText();
	m_pStaticSilver_You->ClearText();
	m_pStaticSilver_You->ClearTooltipText();
	m_pStaticBronze_You->ClearText();
	m_pStaticBronze_You->ClearTooltipText();

	m_pStaticGold_Me->ClearText();
	m_pStaticGold_Me->ClearTooltipText();
	m_pStaticSilver_Me->ClearText();
	m_pStaticSilver_Me->ClearTooltipText();
	m_pStaticBronze_Me->ClearText();
	m_pStaticBronze_Me->ClearTooltipText();

	m_nTradeMoney = 0;
	m_nTaxMoney = 0;
	m_pStaticGold_Tax->ClearText();
	m_pStaticSilver_Tax->ClearText();
	m_pStaticBronze_Tax->ClearText();

	m_pButtonInputMoney->Enable(true);
	m_pButtonRegist->Enable( true );
	m_pButtonOK->Enable( false );

	m_bReadyMe = false;
	m_bReadyYou = false;
	m_bComfirm = false;

	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		m_vecItemSlotButton_You[i]->SetItemState( ITEMSLOT_ENABLE );
		m_vecItemSlotButton_You[i]->ResetSlot();

		m_vecItemSlotButton_Me[i]->SetItemState( ITEMSLOT_ENABLE );
		m_vecItemSlotButton_Me[i]->ResetSlot();
		SAFE_DELETE(m_vecItem_Me[i]);
	}

	m_pStaticBalloonMe->Show( false );
	m_pStaticChatMe->Show( false );
	m_pStaticBalloonYou->Show( false );
	m_pStaticChatYou->Show( false );

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3617 ) );
}

bool CDnPrivateMarketDlg::CheckItemDuplication( CDnItem *pItem )
{
	for( int i=0; i<PRIVATE_MARKET_ITEMSLOT_MAX; i++ )
	{
		if( m_vecQuickSlotButton[i] && m_vecQuickSlotButton[i]->GetItem() == pItem )
		{
			SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3618 ) );
			return true;
		}
	}

	return false;
}

void CDnPrivateMarketDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if (nID == MONEY_INPUT_DIALOG)
		{
			if( IsCmdControl( "ID_BUTTON_OK" ) )
			{
				OnUpdateMoney_One( m_pMoneyInputDlg->GetMoney() );
			}

			m_pMoneyInputDlg->Show( false );
			return;
		}
		else if (nID == ITEM_ATTACH_CONFIRM_DIALOG)
		{
			if (IsCmdControl("ID_OK"))
			{
				CDnItem *pItem = m_pSplitConfirmExDlg->GetItem();
				if (pItem)
				{
					int nCount = m_pSplitConfirmExDlg->GetItemCount();

					if (m_pSplitConfirmExDlg->GetConfirmType() == CDnInterface::PRIVATE_TRADE)
					{
						if (nCount == 0)
						{
							if (drag::IsValid())
							{
								CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
								if (pDragButton->GetSlotType() == ST_INVENTORY)
									drag::ReleaseControl();
							}

							GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3624), MB_OK ); // UISTRING : Cannot trade 0 items.

							return;
						}
						else if (nCount < 0)
						{
							CDnSlotButton* pFromSlot = m_pSplitConfirmExDlg->GetFromSlot();
							if (pFromSlot != NULL)
							{
								pFromSlot->SetRegist(false);
								pFromSlot->DisableSplitMode(true);
							}
							m_pSplitConfirmExDlg->Show(false);
							return;
						}

						CDnSlotButton* pFromBtn = m_pSplitConfirmExDlg->GetFromSlot();
						if (pFromBtn && pFromBtn->GetSlotType() == ST_INVENTORY)
						{
							CDnQuickSlotButton* pPressedBtn = static_cast<CDnQuickSlotButton*>(pFromBtn);
							//pPressedBtn->EnableSplitMode(nCount);
							DoAddItemToMyTradeList(pPressedBtn, nCount);
						}
						else
						{
							_ASSERT(0);
						}
					}

					m_pSplitConfirmExDlg->Show(false);
				}
				else
				{
					_ASSERT(0);
				}
			}
			else if (IsCmdControl("ID_CANCEL"))
			{
				CDnSlotButton* pFromBtn = m_pSplitConfirmExDlg->GetFromSlot();
				if (pFromBtn)
				{
					pFromBtn->SetRegist(false);
					pFromBtn->DisableSplitMode(true);
				}

				m_pSplitConfirmExDlg->Show(false);
			}
		}
	}
}

void CDnPrivateMarketDlg::SetNotice( const wchar_t *wszNotice, bool bUseTempColor, DWORD dwTempColor )
{
	if( !m_pStaticNotice ) return;

	static DWORD dwOldColor = m_pStaticNotice->GetTextColor();

	if( bUseTempColor )
		m_pStaticNotice->SetTextColor( dwTempColor );
	else
		m_pStaticNotice->SetTextColor( dwOldColor );

	m_pStaticNotice->SetTextWithEllipsis( wszNotice, L"..." );
}

void CDnPrivateMarketDlg::AddChatMe( const wchar_t *wszChat )
{
	m_fBalloonMeShowTime = 0.0f;
	std::wstring chat;
	EtInterface::GetNameLinkMgr().TranslateText(chat, wszChat);
	m_pStaticChatMe->SetTextWithEllipsis( chat.c_str(), L"..." );
	m_pStaticBalloonMe->Show( true );
	m_pStaticChatMe->Show( true );
}

void CDnPrivateMarketDlg::AddChatYou( const wchar_t *wszChat )
{
	m_fBalloonYouShowTime = 0.0f;
	std::wstring chat;
	EtInterface::GetNameLinkMgr().TranslateText(chat, wszChat);
	m_pStaticChatYou->SetTextWithEllipsis( chat.c_str(), L"..." );
	m_pStaticBalloonYou->Show( true );
	m_pStaticChatYou->Show( true );
}

void CDnPrivateMarketDlg::SetNameMe( const wchar_t *wszName )
{
	wchar_t wszTemp[256]={0};
	swprintf_s( wszTemp, 256, L"%s%s", wszName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4115 ) );
	m_pStaticMe->SetText( wszTemp );
}

void CDnPrivateMarketDlg::SetNameYou( const wchar_t *wszName )
{
	wchar_t wszTemp[256]={0};
	swprintf_s( wszTemp, 256, L"%s%s", wszName, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4115 ) );
	m_pStaticYou->SetText( wszTemp );

	swprintf_s( m_strYourName, NAMELENMAX, wszName );
}

void CDnPrivateMarketDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( m_fBalloonMeShowTime >= 5.0f )
	{
		m_pStaticBalloonMe->Show( false );
		m_pStaticChatMe->Show( false );
	}
	else
	{
		m_fBalloonMeShowTime += fElapsedTime;
	}

	if( m_fBalloonYouShowTime >= 5.0f )
	{
		m_pStaticBalloonYou->Show( false );
		m_pStaticChatYou->Show( false );
	}
	else
	{
		m_fBalloonYouShowTime += fElapsedTime;
	}

	if( m_bReadyMe && m_bReadyYou )
	{
		if( !m_bComfirm )
		{
			m_pButtonOK->Enable( true );
		}
	}
}

void CDnPrivateMarketDlg::OnClickButtonConfirm_Other()
{
	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3619 ) );
}

void CDnPrivateMarketDlg::OnClickButtonConfirm_One()
{
	m_bComfirm = true;
	m_pButtonOK->Enable( false );
	GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketConfirm( EXCHANGE_CONFIRM );

	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3620 ) );
}

void CDnPrivateMarketDlg::DoAddItemToMyTradeList(CDnQuickSlotButton* pFromBtn, int itemCount)
{
	if (pFromBtn == NULL)
	{
		_ASSERT(0);
		return;
	}

	CDnSlotButton* pEmptyBtn = GetEmptySlotMe();
	if (pEmptyBtn == NULL)
	{
		_ASSERT(0);
		return;
	}

	MIInventoryItem* pInvenItem = pFromBtn->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	CDnItem* pItem = static_cast<CDnItem*>(pInvenItem);
	TItemInfo itemInfo;
	pItem->GetTItemInfo(itemInfo);
	CDnItem *pNewMyItem = GetItemTask().CreateItem( itemInfo );
	m_vecItem_Me[GetEmptySlot()] = pNewMyItem;
	pEmptyBtn->SetItem(pNewMyItem, itemCount);

	CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	GetTradeTask().GetTradePrivateMarket().RequestPrivateMarketAddItem(pEmptyBtn->GetSlotIndex(), pItem->GetSlotIndex(), pEmptyBtn->GetRenderCount(), pItem->GetSerialID());
	SetNotice( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3612 ) );

	m_vecQuickSlotButton[pEmptyBtn->GetSlotIndex()] = pFromBtn;
	m_vecQuickSlotButton[pEmptyBtn->GetSlotIndex()]->SetRegist(true);
	m_vecQuickSlotButton[pEmptyBtn->GetSlotIndex()]->EnableSplitMode(itemCount);

	RefreshTax();
}

void CDnPrivateMarketDlg::AddItemToMyTradeList(CDnQuickSlotButton *pPressedButton, bool bItemSplit)
{
	CDnItem *pItem = static_cast<CDnItem *>(pPressedButton->GetItem());

	// Check if there is space in the trade window
	if( !IsEmptySlot() )
	{
		GetInterface().MessageBox( 3621, MB_OK );
		return;
	}

#ifdef PRE_ADD_ONESTOP_TRADECHECK
	if (pItem && CDnTradeTask::IsActive() && CDnTradeTask::GetInstance().IsTradable(CDnTradeTask::eTRADECHECK_PRIVATE, *pItem) == false)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4035 ), MB_OK );	// UISTRING : This item cannot be traded
		return;
	}
#endif

	// Skip if re-registering an already listed item.
	if( CheckItemDuplication( pItem ) )
		return;

	// Check item type
	if( pItem->GetItemType() == ITEMTYPE_QUEST )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
		return;
	}

	if (CheckReversionItemTradable(pItem) == false)
		return;

	if (pPressedButton->GetRenderCount() > 1 && bItemSplit)
	{
		pPressedButton->SetRegist(true);

		m_pSplitConfirmExDlg->SetItem(pPressedButton, CDnInterface::PRIVATE_TRADE);
		m_pSplitConfirmExDlg->Show(true);

		return;
	}

	DoAddItemToMyTradeList(pPressedButton, pPressedButton->GetRenderCount());
}

bool CDnPrivateMarketDlg::IsEmptySlot()
{
	if( GetEmptySlot() == -1 )
	{
		return false;
	}

	return true;
}

int CDnPrivateMarketDlg::GetEmptySlot()
{
	CDnItemSlotButton *pButton(NULL);

	DWORD dwVecSize = (DWORD)m_vecItemSlotButton_Me.size();
	for( DWORD i=0; i<dwVecSize; i++ )
	{
		pButton = m_vecItemSlotButton_Me[i];
		if( !pButton ) continue;

		// If item state changed to DISABLE after clicking register, do not retrieve the slot index.
		if( (pButton->GetItemState() == ITEMSLOT_ENABLE) && pButton->IsEmptySlot() )
		{
			return pButton->GetSlotIndex();
		}
	}

	return -1;
}

CDnSlotButton* CDnPrivateMarketDlg::GetEmptySlotMe()
{
	int slotNum = GetEmptySlot();
	if (slotNum < 0 || slotNum >= (int)m_vecItemSlotButton_Me.size())
	{
		_ASSERT(0);
		return NULL;
	}

	return m_vecItemSlotButton_Me[slotNum];
}

void CDnPrivateMarketDlg::RefreshTax()
{
	// Accumulate money first.
	INT64 nTotalAmount = m_nTradeMoney;

	std::vector<CDnItemSlotButton*>::const_iterator iter = m_vecItemSlotButton_Me.begin();
	for (; iter != m_vecItemSlotButton_Me.end(); ++iter)
	{
		const CDnItemSlotButton* curSlot = *iter;
		if (curSlot)
		{
			const CDnItem *pItem = static_cast<const CDnItem*>(curSlot->GetItem());
			if( pItem )
				nTotalAmount += (pItem->GetItemAmount() * curSlot->GetRenderCount());
		}
	}

	// Calculate
	m_nTaxMoney = CommonUtil::CalcTax(TAX_DEAL, CDnActor::s_hLocalActor->GetLevel(), nTotalAmount);
	INT64 nGold = m_nTaxMoney/10000;
	INT64 nSilver = (m_nTaxMoney%10000)/100;
	INT64 nBronze = m_nTaxMoney%100;
	m_pStaticGold_Tax->SetInt64ToText( nGold );
	m_pStaticSilver_Tax->SetInt64ToText( nSilver );
	m_pStaticBronze_Tax->SetInt64ToText( nBronze );
}