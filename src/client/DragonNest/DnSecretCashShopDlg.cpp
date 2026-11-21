#include "StdAfx.h"
#include "DnSecretCashShopDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnCashShopDefine.h"
#include "DnCashShopTask.h"
#include "CashShopSendPacket.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnSecretCashShopAlarmButtonDlg
//-----------------------------------------------------------------------------
CDnSecretCashShopAlarmButtonDlg::CDnSecretCashShopAlarmButtonDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pBtnSecret( NULL )
	, m_pStaticHidden( NULL )
	, m_pTextureFace( NULL )
	, m_pStaticTextHidden( NULL )
	, m_nHiddenShopID( 0 )
{
}

CDnSecretCashShopAlarmButtonDlg::~CDnSecretCashShopAlarmButtonDlg()
{
	SAFE_RELEASE_SPTR( m_hFaceTexture );
}

void CDnSecretCashShopAlarmButtonDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CS_SecretBTDlg.ui" ).c_str(), bShow );
}

void CDnSecretCashShopAlarmButtonDlg::InitialUpdate()
{
	m_pBtnSecret = GetControl<CEtUIButton>( "ID_BT_SECRET" );
	m_pStaticHidden = GetControl<CEtUIStatic>( "ID_STATIC_HIDDEN" );
	m_pTextureFace = GetControl<CEtUITextureControl>( "ID_TEXTUREL_FACE" );
	m_pStaticTextHidden = GetControl<CEtUIStatic>( "ID_TEXT_HIDDEN" );
}

void CDnSecretCashShopAlarmButtonDlg::SetAlarmInfo( const WCHAR *wszText, const char *szFaceTexture )
{
	if( m_pStaticTextHidden && wszText )
		m_pStaticTextHidden->SetText( wszText );

	if( m_pTextureFace && szFaceTexture && szFaceTexture[0] != '\0' )
	{
		SAFE_RELEASE_SPTR( m_hFaceTexture );
		m_hFaceTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( szFaceTexture ).c_str(), RT_TEXTURE );
		if( m_hFaceTexture )
			m_pTextureFace->SetTexture( m_hFaceTexture, 0, 0, m_hFaceTexture->Width(), m_hFaceTexture->Height() );
	}
}

void CDnSecretCashShopAlarmButtonDlg::SetHiddenShop( int nHiddenShopID )
{
	m_nHiddenShopID = nHiddenShopID;
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::THIDDENCASHSHOPREAL );
	if( !pSox || !pSox->IsExistItem( nHiddenShopID ) )
		return;

	DNTableCell *pImgCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_ButtonImgID" );
	int nButtonImgID = pImgCell ? pImgCell->GetInteger() : 0;
	if( m_pTextureFace && nButtonImgID > 0 )
	{
		if( !m_hFaceTexture )
			m_hFaceTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( "CS_SecretButton.dds" ).c_str(), RT_TEXTURE );
		if( m_hFaceTexture )
		{
			int nOffset = ( ( nButtonImgID - 1 ) / 6 ) * 32 + ( nButtonImgID - 1 ) * 80;
			int nX = nOffset % 512;
			int nY = ( nOffset / 512 ) * 80;
			m_pTextureFace->SetTexture( m_hFaceTexture, nX, nY, 80, 80 );
			m_pTextureFace->Show( true );
		}
	}
	if( m_pStaticHidden ) m_pStaticHidden->Show( true );
	if( m_pStaticTextHidden ) m_pStaticTextHidden->Show( true );
}

void CDnSecretCashShopAlarmButtonDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && IsCmdControl( "ID_BT_SECRET" ) )
	{
		if( m_nHiddenShopID > 0 && GetInterface().GetSecretCashShopMsgDlg() )
		{
			GetInterface().GetSecretCashShopMsgDlg()->SetHiddenShop( m_nHiddenShopID );
			GetInterface().GetSecretCashShopMsgDlg()->Show( true );
		}
		else if( GetInterface().GetSecretCashShopBuyDlg() )
		{
			GetInterface().GetSecretCashShopBuyDlg()->Show( true );
		}
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnSecretCashShopMessageDlg
//-----------------------------------------------------------------------------
CDnSecretCashShopMessageDlg::CDnSecretCashShopMessageDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pTextBoxAsk( NULL )
	, m_pTextureNpc( NULL )
	, m_nHiddenShopID( 0 )
{
}

CDnSecretCashShopMessageDlg::~CDnSecretCashShopMessageDlg()
{
	SAFE_RELEASE_SPTR( m_hNpcTexture );
}

void CDnSecretCashShopMessageDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CS_SecretMessageDlg.ui" ).c_str(), bShow );
}

void CDnSecretCashShopMessageDlg::InitialUpdate()
{
	m_pTextBoxAsk = GetControl<CEtUITextBox>( "ID_TEXTBOX_ASK" );
	m_pTextureNpc = GetControl<CEtUITextureControl>( "ID_TEXTUREL_NPC" );
}

void CDnSecretCashShopMessageDlg::SetMessage( const WCHAR *wszMessage, const char *szNpcTexture )
{
	if( m_pTextBoxAsk )
	{
		m_pTextBoxAsk->ClearText();
		if( wszMessage )
			m_pTextBoxAsk->AddText( wszMessage );
	}
	if( m_pTextureNpc && szNpcTexture && szNpcTexture[0] != '\0' )
	{
		SAFE_RELEASE_SPTR( m_hNpcTexture );
		m_hNpcTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( szNpcTexture ).c_str(), RT_TEXTURE );
		if( m_hNpcTexture )
			m_pTextureNpc->SetTexture( m_hNpcTexture, 0, 0, 180, 180 );
	}
}

void CDnSecretCashShopMessageDlg::SetHiddenShop( int nHiddenShopID )
{
	m_nHiddenShopID = nHiddenShopID;
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::THIDDENCASHSHOPREAL );
	if( !pSox || !pSox->IsExistItem( nHiddenShopID ) )
		return;

	DNTableCell *pImgCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_FirstNPCImg" );
	const char *szNpcImg = pImgCell ? pImgCell->GetString() : NULL;
	DNTableCell *pTalkCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_FirstTalkID" );
	int nTalkID = pTalkCell ? pTalkCell->GetInteger() : 0;
	const WCHAR *wszTalk = ( nTalkID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTalkID ) : L"";
	SetMessage( wszTalk, szNpcImg );
}

void CDnSecretCashShopMessageDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_nHiddenShopID > 0 )
				SendSecretCashShopOpen( m_nHiddenShopID );
			Show( false );
			return;
		}
		if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnSecretCashShopBuyDlg
//-----------------------------------------------------------------------------
CDnSecretCashShopBuyDlg::CDnSecretCashShopBuyDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnPrev( NULL )
	, m_pBtnNext( NULL )
	, m_pCheckCash( NULL )
	, m_pCheckCredit( NULL )
	, m_pCheckReserve( NULL )
	, m_pTextBoxAsk( NULL )
	, m_pTextureNpc( NULL )
	, m_pStaticCash( NULL )
	, m_pStaticReserve( NULL )
	, m_pStaticPage( NULL )
	, m_pStaticInfo( NULL )
	, m_pStaticSelectedName( NULL )
	, m_pStaticSelectedCount( NULL )
	, m_pStaticSelectedPeriod( NULL )
	, m_nHiddenShopID( 0 )
	, m_nPurchaseCount( -1 )
	, m_nCurPage( 0 )
	, m_nMaxPage( 0 )
	, m_nSelectedIdx( -1 )
{
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		m_pSlotItems[i] = NULL;
		m_pStaticSelect[i] = NULL;
		m_pTextBoxPrice[i] = NULL;
		m_pStaticDscRate[i] = NULL;
		m_pStaticItemCount[i] = NULL;
		m_pStaticItemName[i] = NULL;
		m_pStaticItemPeriod[i] = NULL;
		m_pPreviewItems[i] = NULL;
	}
}

CDnSecretCashShopBuyDlg::~CDnSecretCashShopBuyDlg()
{
	ClearPreviewItems();
	SAFE_RELEASE_SPTR( m_hNpcTexture );
}

void CDnSecretCashShopBuyDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CS_Secret_USADlg.ui" ).c_str(), bShow );
}

void CDnSecretCashShopBuyDlg::InitialUpdate()
{
	m_pBtnPrev = GetControl<CEtUIButton>( "ID_BT_PREV" );
	m_pBtnNext = GetControl<CEtUIButton>( "ID_BT_NEXT" );
	m_pCheckCash = GetControl<CEtUICheckBox>( "ID_CHECKBOX_CASH" );
	m_pCheckCredit = GetControl<CEtUICheckBox>( "ID_CHECKBOX_CREDIT" );
	m_pCheckReserve = GetControl<CEtUICheckBox>( "ID_CHECKBOX_RESERVE" );
	m_pTextBoxAsk = GetControl<CEtUITextBox>( "ID_TEXTBOX_ASK" );
	m_pTextureNpc = GetControl<CEtUITextureControl>( "ID_TEXTUREL_NPC" );
	m_pStaticCash = GetControl<CEtUIStatic>( "ID_TEXT_CASH" );
	m_pStaticReserve = GetControl<CEtUIStatic>( "ID_TEXT_RESERVE" );
	m_pStaticPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );
	m_pStaticInfo = GetControl<CEtUIStatic>( "ID_TEXT_INFO" );
	m_pStaticSelectedName = GetControl<CEtUIStatic>( "ID_TEXT_ITEMNAME" );
	m_pStaticSelectedCount = GetControl<CEtUIStatic>( "ID_TEXT_ITEMCOUNT" );
	m_pStaticSelectedPeriod = GetControl<CEtUIStatic>( "ID_TEXT_ITEMPERIOD" );

	char szName[64] = { 0 };
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_ITEM%d", i );
		m_pSlotItems[i] = GetControl<CDnItemSlotButton>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_STATIC_SELECT%d", i );
		m_pStaticSelect[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXTBOX_PRICE%d", i );
		m_pTextBoxPrice[i] = GetControl<CEtUITextBox>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXT_DSCRATE%d", i );
		m_pStaticDscRate[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXT_ITEMCOUNT%d", i );
		m_pStaticItemCount[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXT_ITEMNAME%d", i );
		m_pStaticItemName[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_TEXT_ITEMPERIOD%d", i );
		m_pStaticItemPeriod[i] = GetControl<CEtUIStatic>( szName );
	}

	if( m_pCheckCash )
		m_pCheckCash->SetChecked( true, false );
}

void CDnSecretCashShopBuyDlg::ClearPreviewItems()
{
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( m_pSlotItems[i] ) m_pSlotItems[i]->ResetSlot();
		SAFE_DELETE( m_pPreviewItems[i] );
	}
}

void CDnSecretCashShopBuyDlg::SetSecretItems( const std::vector<SSecretShopItem> &vecItems )
{
	m_vecItems = vecItems;
	m_nCurPage = 0;
	m_nMaxPage = m_vecItems.empty() ? 0 : ( (int)m_vecItems.size() - 1 ) / SLOT_PER_PAGE;
	m_nSelectedIdx = m_vecItems.empty() ? -1 : 0;
	RefreshPage();

	if( CDnInterface::IsActive() && GetInterface().GetSecretCashShopAlarmBtnDlg() )
		GetInterface().GetSecretCashShopAlarmBtnDlg()->Show( !m_vecItems.empty() );
}

void CDnSecretCashShopBuyDlg::SetHiddenShop( int nHiddenShopID )
{
	m_nHiddenShopID = nHiddenShopID;
	m_vecItems.clear();
	m_nPurchaseCount = -1;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::THIDDENCASHSHOPREAL );
	if( !pSox || !pSox->IsExistItem( nHiddenShopID ) )
		return;

	if( m_pTextureNpc )
	{
		SAFE_RELEASE_SPTR( m_hNpcTexture );
		DNTableCell *pImgCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_SecondNPCImg" );
		const char *szNpcImg = pImgCell ? pImgCell->GetString() : NULL;
		if( szNpcImg && szNpcImg[0] != '\0' )
		{
			m_hNpcTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( szNpcImg ).c_str(), RT_TEXTURE );
			if( m_hNpcTexture )
			{
				m_pTextureNpc->SetTexture( m_hNpcTexture, 0, 0, 180, 300 );
				m_pTextureNpc->Show( true );
			}
		}
	}

	if( m_pTextBoxAsk )
	{
		m_pTextBoxAsk->ClearText();
		DNTableCell *pTalkCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_SecondTalkID" );
		int nTalkID = pTalkCell ? pTalkCell->GetInteger() : 0;
		if( nTalkID > 0 )
			m_pTextBoxAsk->AddText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nTalkID ) );
	}

	DNTableCell *pPurchaseCell = pSox->GetFieldFromLablePtr( nHiddenShopID, "_PurchaseCount" );
	if( pPurchaseCell )
		m_nPurchaseCount = pPurchaseCell->GetInteger();

	char szLabel[32] = { 0 };
	for( int i = 1; i <= 10; ++i )
	{
		sprintf_s( szLabel, sizeof( szLabel ), "_SN_%02d", i );
		DNTableCell *pSnCell = pSox->GetFieldFromLablePtr( nHiddenShopID, szLabel );
		if( !pSnCell ) continue;
		int nSN = pSnCell->GetInteger();
		if( nSN <= 0 || !CDnCashShopTask::IsActive() ) continue;

		const SCashShopItemInfo *pInfo = GetCashShopTask().GetItemInfo( nSN );
		if( !pInfo ) continue;

		SSecretShopItem item;
		item.nSN = nSN;
		item.nItemID = !pInfo->linkIdList.empty() ? pInfo->linkIdList[0] : pInfo->presentItemId;
		item.nCount = ( pInfo->count > 0 ) ? pInfo->count : 1;
		item.nPeriod = pInfo->period;
		item.nPrice = pInfo->price;
		item.nDiscountRate = ( pInfo->priceFix > pInfo->price && pInfo->priceFix > 0 )
			? ( ( pInfo->priceFix - pInfo->price ) * 100 / pInfo->priceFix ) : 0;
		m_vecItems.push_back( item );
	}

	m_nCurPage = 0;
	m_nMaxPage = m_vecItems.empty() ? 0 : ( (int)m_vecItems.size() - 1 ) / SLOT_PER_PAGE;
	m_nSelectedIdx = m_vecItems.empty() ? -1 : 0;
	RefreshPage();
}

void CDnSecretCashShopBuyDlg::SelectSlot( int nSlotIdx )
{
	int nIdx = m_nCurPage * SLOT_PER_PAGE + nSlotIdx;
	if( nIdx < 0 || nIdx >= (int)m_vecItems.size() )
		return;

	m_nSelectedIdx = nIdx;
	UpdateSelectedInfo();
}

void CDnSecretCashShopBuyDlg::UpdateSelectedInfo()
{
	int nStart = m_nCurPage * SLOT_PER_PAGE;
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( m_pStaticSelect[i] )
			m_pStaticSelect[i]->Show( ( nStart + i ) == m_nSelectedIdx );
	}

	if( m_nSelectedIdx < 0 || m_nSelectedIdx >= (int)m_vecItems.size() )
	{
		if( m_pStaticSelectedName ) m_pStaticSelectedName->SetText( L"" );
		if( m_pStaticSelectedCount ) m_pStaticSelectedCount->SetText( L"" );
		if( m_pStaticSelectedPeriod ) m_pStaticSelectedPeriod->SetText( L"" );
		return;
	}

	const SSecretShopItem &entry = m_vecItems[m_nSelectedIdx];
	WCHAR wszBuf[64] = { 0 };

	if( m_pStaticSelectedName )
	{
		int nSlotOffset = m_nSelectedIdx - nStart;
		if( nSlotOffset >= 0 && nSlotOffset < SLOT_PER_PAGE && m_pPreviewItems[nSlotOffset] )
			m_pStaticSelectedName->SetText( m_pPreviewItems[nSlotOffset]->GetName() );
		else
			m_pStaticSelectedName->SetText( CDnItem::GetItemFullName( entry.nItemID ).c_str() );
	}
	if( m_pStaticSelectedCount )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", entry.nCount );
		m_pStaticSelectedCount->SetText( wszBuf );
	}
	if( m_pStaticSelectedPeriod )
	{
		if( entry.nPeriod > 0 )
			swprintf_s( wszBuf, _countof( wszBuf ), L"%d", entry.nPeriod );
		else
			wcscpy_s( wszBuf, _countof( wszBuf ), L"-" );
		m_pStaticSelectedPeriod->SetText( wszBuf );
	}
}

void CDnSecretCashShopBuyDlg::UpdateBalanceDisplay()
{
	if( !CDnCashShopTask::IsActive() )
		return;

	WCHAR wszBuf[64] = { 0 };
	if( m_pStaticCash )
	{
		int nCashAmount = GetCashShopTask().GetUserCash();
#ifdef PRE_ADD_CASHSHOP_CREDIT
		if( m_pCheckCredit && m_pCheckCredit->IsChecked() )
			nCashAmount = GetCashShopTask().GetUserCredit();
		else if( m_pCheckCash && m_pCheckCash->IsChecked() )
			nCashAmount = GetCashShopTask().GetUserPrepaid();
#endif
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", nCashAmount );
		m_pStaticCash->SetText( wszBuf );
	}
	if( m_pStaticReserve )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", GetCashShopTask().GetUserReserve() );
		m_pStaticReserve->SetText( wszBuf );
	}
}

void CDnSecretCashShopBuyDlg::RefreshPage()
{
	ClearPreviewItems();
	int nStart = m_nCurPage * SLOT_PER_PAGE;
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( m_pStaticItemName[i] ) m_pStaticItemName[i]->SetText( L"" );
		if( m_pStaticItemCount[i] ) m_pStaticItemCount[i]->SetText( L"" );
		if( m_pStaticItemPeriod[i] ) m_pStaticItemPeriod[i]->SetText( L"" );
		if( m_pStaticDscRate[i] ) m_pStaticDscRate[i]->SetText( L"" );
		if( m_pTextBoxPrice[i] ) m_pTextBoxPrice[i]->ClearText();

		int nIdx = nStart + i;
		if( m_pStaticSelect[i] ) m_pStaticSelect[i]->Show( nIdx == m_nSelectedIdx );
		if( nIdx < 0 || nIdx >= (int)m_vecItems.size() )
			continue;

		const SSecretShopItem &entry = m_vecItems[nIdx];
		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( entry.nItemID, entry.nCount, itemInfo ) )
		{
			m_pPreviewItems[i] = GetItemTask().CreateItem( itemInfo );
			if( m_pPreviewItems[i] && m_pSlotItems[i] )
				m_pSlotItems[i]->SetItem( m_pPreviewItems[i], entry.nCount );
			if( m_pPreviewItems[i] && m_pStaticItemName[i] )
				m_pStaticItemName[i]->SetText( m_pPreviewItems[i]->GetName() );
		}

		WCHAR wszBuf[64] = { 0 };
		if( m_pStaticItemCount[i] )
		{
			swprintf_s( wszBuf, _countof( wszBuf ), L"%d", entry.nCount );
			m_pStaticItemCount[i]->SetText( wszBuf );
		}
		if( m_pStaticItemPeriod[i] )
		{
			if( entry.nPeriod > 0 )
				swprintf_s( wszBuf, _countof( wszBuf ), L"%d", entry.nPeriod );
			else
				wcscpy_s( wszBuf, _countof( wszBuf ), L"-" );
			m_pStaticItemPeriod[i]->SetText( wszBuf );
		}
		if( m_pStaticDscRate[i] )
		{
			swprintf_s( wszBuf, _countof( wszBuf ), L"%d%%", entry.nDiscountRate );
			m_pStaticDscRate[i]->SetText( wszBuf );
		}
		if( m_pTextBoxPrice[i] )
		{
			swprintf_s( wszBuf, _countof( wszBuf ), L"%d", entry.nPrice );
			m_pTextBoxPrice[i]->AddText( wszBuf );
		}
	}

	if( m_pStaticPage )
	{
		WCHAR wszPage[32] = { 0 };
		swprintf_s( wszPage, _countof( wszPage ), L"%d / %d", m_nCurPage + 1, m_nMaxPage + 1 );
		m_pStaticPage->SetText( wszPage );
	}

	if( m_pCheckCash && ( !m_pCheckCash->IsChecked() )
		&& ( !m_pCheckCredit || !m_pCheckCredit->IsChecked() )
		&& ( !m_pCheckReserve || !m_pCheckReserve->IsChecked() ) )
	{
		m_pCheckCash->SetChecked( true, false );
	}

	UpdateSelectedInfo();
	UpdateBalanceDisplay();
}

void CDnSecretCashShopBuyDlg::Show( bool bShow )
{
	if( bShow )
		RefreshPage();
	CEtUIDialog::Show( bShow );
}

void CDnSecretCashShopBuyDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl( "ID_CHECKBOX_CASH" ) )
		{
			if( m_pCheckCash ) m_pCheckCash->SetChecked( true, false );
			if( m_pCheckCredit ) m_pCheckCredit->SetChecked( false, false );
			if( m_pCheckReserve ) m_pCheckReserve->SetChecked( false, false );
			UpdateBalanceDisplay();
			return;
		}
		else if( IsCmdControl( "ID_CHECKBOX_CREDIT" ) )
		{
			if( m_pCheckCash ) m_pCheckCash->SetChecked( false, false );
			if( m_pCheckCredit ) m_pCheckCredit->SetChecked( true, false );
			if( m_pCheckReserve ) m_pCheckReserve->SetChecked( false, false );
			UpdateBalanceDisplay();
			return;
		}
		else if( IsCmdControl( "ID_CHECKBOX_RESERVE" ) )
		{
			if( m_pCheckCash ) m_pCheckCash->SetChecked( false, false );
			if( m_pCheckCredit ) m_pCheckCredit->SetChecked( false, false );
			if( m_pCheckReserve ) m_pCheckReserve->SetChecked( true, false );
			UpdateBalanceDisplay();
			return;
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_ITEM0" ) )
		{
			SelectSlot( 0 );
			return;
		}
		else if( IsCmdControl( "ID_ITEM1" ) )
		{
			SelectSlot( 1 );
			return;
		}
		else if( IsCmdControl( "ID_BT_PREV" ) )
		{
			if( m_nCurPage > 0 ) --m_nCurPage;
			RefreshPage();
			return;
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			if( m_nCurPage < m_nMaxPage ) ++m_nCurPage;
			RefreshPage();
			return;
		}
		else if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_nSelectedIdx < 0 || m_nSelectedIdx >= (int)m_vecItems.size() )
			{
				if( GetInterface().GetSecretCashShopMsgDlg() )
				{
					GetInterface().GetSecretCashShopMsgDlg()->SetMessage( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4748 ) );
					GetInterface().GetSecretCashShopMsgDlg()->Show( true );
				}
				return;
			}

			char cPaymentRules = Cash::PaymentRules::Prepaid;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
			eBuyMode buyMode = eBuyMode::CASHSHOP_BUY_CASH;
#endif
			if( m_pCheckReserve && m_pCheckReserve->IsChecked() )
			{
				cPaymentRules = Cash::PaymentRules::Petal;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
				buyMode = eBuyMode::CASHSHOP_BUY_RESERVE;
#endif
			}
			else if( m_pCheckCredit && m_pCheckCredit->IsChecked() )
			{
				cPaymentRules = Cash::PaymentRules::Credit;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
				buyMode = eBuyMode::CASHSHOP_BUY_CREDIT;
#endif
			}

#if defined(PRE_ADD_CASHSHOP_CREDIT)
			if( CDnCashShopTask::IsActive() )
				GetCashShopTask().SetBuyMode( buyMode );
#endif

			const SSecretShopItem &entry = m_vecItems[m_nSelectedIdx];
			std::vector<TCashShopInfo> buyList;
			TCashShopInfo info;
			memset( &info, 0, sizeof( info ) );
			info.nItemID = entry.nItemID;
			info.nItemSN = entry.nSN;
			info.cSlotIndex = 0;
			info.nOptionIndex = 0;
			buyList.push_back( info );

#ifdef PRE_ADD_CASH_REFUND
			SendCashShopBuy( eCashUnit_BuyItemNow, cPaymentRules, buyList, true );
#else
			SendCashShopBuy( eCashUnit_BuyItemNow, cPaymentRules, buyList );
#endif
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) )
		{
			SendSecretCashShopClose();
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
