#include "StdAfx.h"
#include "DnTradeMarket.h"
#include "TradeSendPacket.h"
#include "DnMarketTabDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"

#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM
#include "DnItemUpgradeDlg.h"
#include "DnMarketDirectBuyDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnTradeMarket::CDnTradeMarket(void)		
{
	m_pMarketDialog = NULL;
	m_bRequestWait = false;
}

CDnTradeMarket::~CDnTradeMarket(void)
{
}

void CDnTradeMarket::ServerMessageBox( int nRetCode )
{
	GetInterface().ServerMessageBox( nRetCode );
}

void CDnTradeMarket::RequestMarketBuyList( int nPageNum, char cMinLevel, char cMaxLevel, char *cItemGradeArray, char cJob, short nMainType, char cDetailType, WCHAR *pwszSearch1, WCHAR *pwszSearch2, WCHAR *pwszSearch3, char cSortType, int nExchangeItemID, bool bLowJobGroup, WCHAR *pwszSearch, char cPayMethodCode )
{
	if( m_bRequestWait ) return;
	SendMarketList( nPageNum, cMinLevel, cMaxLevel, cItemGradeArray, cJob, nMainType, cDetailType,  pwszSearch1, pwszSearch2, pwszSearch3, cSortType, nExchangeItemID, bLowJobGroup, pwszSearch, cPayMethodCode );
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketSellList()
{
	if( m_bRequestWait ) return;
	SendMyMarketList();
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketCalculationList()
{
	if( m_bRequestWait ) return;
	SendMarketCalculationList();
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketBuy( int nMarketDBID )
{
	if( m_bRequestWait ) return;
	ASSERT( nMarketDBID > 0 );
	SendMarketBuy( nMarketDBID, false );
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketRegister( INT64 biInvenSerial, short wCount, int nPrice, char cPeriodIndex, char cInvenType, BYTE cInvenIndex, bool bPremium, char cPayMethodCode )
{
	if( m_bRequestWait ) return;
	ASSERT( wCount > 0 );
	SendMarketRegister( biInvenSerial, wCount, nPrice, cPeriodIndex, cInvenType, cInvenIndex, bPremium, cPayMethodCode );
	m_bRequestWait = false;
}

void CDnTradeMarket::RequestMarketInterrupt( int nMarketDBID )
{
	if( m_bRequestWait ) return;
	ASSERT( nMarketDBID > 0 );
	SendMarketInterrupt( nMarketDBID );
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketCalculation( int nMarketDBID )
{
	if( m_bRequestWait ) return;
	ASSERT( nMarketDBID > 0 );
	SendMarketCalculation( nMarketDBID );
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketCalculationAll()
{
	if( m_bRequestWait ) return;
	SendMarketCalculationAll();
	m_bRequestWait = true;

	m_pMarketDialog->EnableMarketAccountControl( "ID_BUTTON_ALLACCOUNT", false );
}

void CDnTradeMarket::RequestMarketPetalBalance()
{
	if( m_bRequestWait ) return;
	SendMarketPetalBalance();
	m_bRequestWait = true;
}

void CDnTradeMarket::RequestMarketPrice( int nMarketDBID, int nItemID, BYTE cLevel, BYTE cOption )
{
	if( m_bRequestWait ) return;
	SendMarketPrice( nMarketDBID, nItemID, cLevel, cOption );
	m_bRequestWait = true;
}


// OnRecv

void CDnTradeMarket::OnRecvMarketBuyList( SCMarketList *pPacket )
{
	if( !CheckValidPacket( pPacket ) )  {
		m_pMarketDialog->UpdateBuyList( std::vector< TMarketInfo >(), -1);
	}

	GetInterface().OpenMarketDialog();
	
	std::vector< TMarketInfo > BuyList;
	for( int i = 0; i < pPacket->cMarketCount; i++) {
		BuyList.push_back( pPacket->MarketInfo[i] );
	}
	m_pMarketDialog->UpdateBuyList( BuyList, pPacket->nMarketTotalCount);
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketSellList( SCMyMarketList *pPacket )
{
	if( !CheckValidPacket( pPacket ) )  return;
	
	std::vector< TMyMarketInfo > SellList;
	for( int i = 0; i < pPacket->cMarketCount; i++) {
		SellList.push_back( pPacket->MarketInfo[i] );
	}

	m_pMarketDialog->SetSellCount( pPacket->wSellingCount, pPacket->wClosingCount, pPacket->wWeeklyRegisterCount, pPacket->wRegisterItemCount, pPacket->bPremiumTrade );
	m_pMarketDialog->UpdateSellList( SellList );
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketCalculationList( SCMarketCalculationList *pPacket )
{
	if( CheckValidPacket( pPacket ) ) {
		std::vector< TMarketCalculationInfo > CalculationList;
		for( int i = 0; i < pPacket->cMarketCount; i++) {
			CalculationList.push_back( pPacket->MarketInfo[i] );
		}
		m_pMarketDialog->UpdateCalculationList( CalculationList );
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketRegist( SCMarketResult *pPacket )
{
	if( ERROR_NONE != pPacket->nRet )
	{
		if( ERROR_GENERIC_LEVELLIMIT == pPacket->nRet )
		{
			WCHAR wszMessage[512];
			int iWeight = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::LimitLevel_Market);
			swprintf_s( wszMessage, _countof(wszMessage), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3630 ), iWeight );	// Purchase and registration are available after level %d.
			GetInterface().MessageBox( wszMessage );
		}
		else
			GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
		RequestMarketSellList();

	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketInterrupt( SCMarketInterrupt *pPacket )
{
	if( CheckValidPacketShort( pPacket ) )  {
		m_pMarketDialog->RemoveSellItem( pPacket->nMarketDBID, pPacket->wRegisterItemCount );

	}
	else {
		m_pMarketDialog->RemoveSellItem( -1, -1 );
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketBuy( SCMarketBuyResult *pPacket )
{
	if( ERROR_NONE != pPacket->nRet )
	{
		if( ERROR_GENERIC_LEVELLIMIT == pPacket->nRet )
		{
			WCHAR wszMessage[512];
			int iWeight = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::LimitLevel_Market);
			swprintf_s( wszMessage, _countof(wszMessage), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3630 ), iWeight );	// Purchase and registration are available after level %d.

			GetInterface().MessageBox( wszMessage );
		}
		else
			GetInterface().ServerMessageBox(pPacket->nRet);

		m_pMarketDialog->RemoveBuyItem( -1 );
	}
	else
	{
		m_pMarketDialog->RemoveBuyItem( pPacket->nMarketDBID );	

#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
		if(pPacket->bMini)
		{
			TCHAR* pName = GetInterface().GetDirectBuyUpgradeItem()->GetPrevSelectItem()->GetName();
			std::wstring wszMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4073 /*Purchased item %s.*/) , pName);
			GetInterface().MessageBox( wszMsg.c_str(), MB_OK );
			((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->SetMiniMarketFlag(pPacket->bMini);
			((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->RefreshUpgradeItemButton();

			GetInterface().GetDirectBuyUpgradeItem()->Show(false);
		}
#endif
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketCalculation( SCMarketResult *pPacket )
{
	if( CheckValidPacket( pPacket ) )  {
		m_pMarketDialog->RemoveCalculationItem( pPacket->nMarketDBID );	
	}
	else {
		m_pMarketDialog->RemoveCalculationItem( -1 );	
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketCalculationAll( SCMarketCalculationAll *pPacket )
{
	if( CheckValidPacket( pPacket ) )  {
		m_pMarketDialog->RemoveAllCalculationItem();	
	}
	m_bRequestWait = false;
	m_pMarketDialog->EnableMarketAccountControl( "ID_BUTTON_ALLACCOUNT", true );
}

void CDnTradeMarket::OnRecvMarketCalcNotify( SCNotifyMarket *pPacket )
{
	m_bRequestWait = false;
	if( !CDnInterface::IsActive() ) {
		return;
	}
	if( pPacket->nItemID > 0 ) {
		std::wstring wszMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4071 ), CDnItem::GetItemFullName( pPacket->nItemID ).c_str() );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg.c_str(), false );
	}
	if( pPacket->wCalculationCount > 0 ) {
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4072 ), false );
	}
}

void CDnTradeMarket::OnRecvMarketPetalBalance( SCMarketPetalBalance * pPacket )
{
	if( CheckValidPacket( pPacket ) )  {
		m_pMarketDialog->UpdatePetalBalance( pPacket->nPetalBalance );
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM) && defined(PRE_ADD_PETALTRADE)
		GetInterface().GetDirectBuyUpgradeItem()->SetMyPetal(pPacket->nPetalBalance);
#endif // PRE_ADD_DIRECT_BUY_UPGRADEITEM
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::OnRecvMarketPrice( SCMarketPrice * pPacket )
{
	if( CheckValidPacket( pPacket ) )  {
		std::vector< TMarketPrice > vecPrice;
		vecPrice.push_back( pPacket->ItemPrices[0] );
		vecPrice.push_back( pPacket->ItemPrices[1] );

		m_pMarketDialog->UpdateMarketPrice( pPacket->nMarketDBID, vecPrice );
	}
	m_bRequestWait = false;
}

#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
void CDnTradeMarket::OnRecvMarketMiniList( SCMarketMiniList *pPacket )
{
	if( CheckValidPacket(pPacket) )
	{
		GetInterface().SetDirectBuyUpgradeItemDlg(pPacket);
	}
	m_bRequestWait = false;
}

void CDnTradeMarket::RequestMarketMiniBuy( int nMarketDBID )
{
	if( m_bRequestWait ) return;
	ASSERT( nMarketDBID > 0 );
	SendMarketBuy( nMarketDBID, true );
	m_bRequestWait = true;
}
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)