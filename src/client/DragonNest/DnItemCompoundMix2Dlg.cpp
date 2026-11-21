#include "StdAfx.h"
#include "DnItemCompoundMix2Dlg.h"
#include "DnItemTask.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnItemCompoundMix2Dlg::CDnItemCompoundMix2Dlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
, m_pProb(NULL)
, m_pDescription(NULL)
, m_pName(NULL)
, m_pStaticSelect(NULL)
, m_nSelectSlotIndex(-1)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		m_pItem[i] = NULL;
		m_pItemSlotButton[i] = NULL;
	}
}

CDnItemCompoundMix2Dlg::~CDnItemCompoundMix2Dlg(void)
{
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		SAFE_DELETE(m_pItem[i]);
	}
}

void CDnItemCompoundMix2Dlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemCompoundOptionMixDlg.ui" ).c_str(), bShow );
}

void CDnItemCompoundMix2Dlg::InitialUpdate()
{
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");

	m_pProb = GetControl<CEtUIStatic>("ID_PROBABILITY");
	m_pDescription = GetControl<CEtUIStatic>("ID_TEXT_OPTION");
	m_pName = GetControl<CEtUIStatic>("ID_TEXT_NAME");
	m_pStaticSelect = GetControl<CEtUIStatic>("ID_STATIC_SELECT");

	char szName[32];
	for( int i = 0; i < NUM_NEED_ITEM; ++i )
	{
		sprintf_s(szName, _countof(szName), "%s%d", "ID_ITEM_SLOT", i);
		m_pItemSlotButton[i] = GetControl<CDnItemSlotButton>(szName);
		m_pItemSlotButton[i]->SetSlotType( ST_INVENTORY );
	}

	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_OK") );
}

void CDnItemCompoundMix2Dlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_SmartMove.MoveCursor();
		EnableButton( true );
	}
	else
	{
		m_SmartMove.ReturnCursor();

		for( int i = 0; i < NUM_NEED_ITEM; ++i )
			m_pItemSlotButton[i]->ResetSlot();
	}
}

void CDnItemCompoundMix2Dlg::SetItemSlot( int nSlotIndex, CDnItem *pItem, char nOptionIndex/* = -1*/ )
{
	if( pItem == NULL ) return;

	SAFE_DELETE( m_pItem[nSlotIndex] );

	TItemInfo itemInfo;
	pItem->GetTItemInfo( itemInfo );

	//If option values are configured, apply them when creating the item.
	if (nOptionIndex != -1)
		itemInfo.Item.cOption = nOptionIndex;

	m_pItem[nSlotIndex] = GetItemTask().CreateItem( itemInfo );
	// Set default quantity to 1 and render actual count separately.
	m_pItemSlotButton[nSlotIndex]->SetItem(m_pItem[nSlotIndex], CDnSlotButton::INVALID_SPLIT_COUNT);

	m_pItemSlotButton[nSlotIndex]->SetRegist( true );
}

void CDnItemCompoundMix2Dlg::SetCompoundInfo( std::vector<CDnItem *> &pVecList, char nOptionIndex/* = -1*/ )
{
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		if( i >= NUM_NEED_ITEM ) break;
		SetItemSlot( i, pVecList[i], nOptionIndex );
	}
	m_nSelectSlotIndex = ( pVecList.empty() ) ? -1 : 0;
	RefreshSelectItemSlot();
}

void CDnItemCompoundMix2Dlg::RefreshSelectItemSlot()
{
	m_pName->SetText( L"" );
	m_pStaticSelect->Show( false );
	for( int i=0; i<NUM_NEED_ITEM; i++ ) {
		m_pItemSlotButton[i]->SetRegist( true );
		if( m_pItemSlotButton[i]->GetItem() == NULL ) continue;
		if( i == m_nSelectSlotIndex ) {
			// Select
			char szControl[32];
			SUICoord Coord;
			int nArray[5] = { 1, 3, 4, 5, 6 };
			sprintf_s( szControl, "ID_STATIC%d", nArray[i] );
			CEtUIStatic *pStatic = GetControl<CEtUIStatic>(szControl);
			pStatic->GetUICoord( Coord );

			m_pStaticSelect->SetUICoord( Coord );
			m_pStaticSelect->Show( true );
			m_pItemSlotButton[i]->SetRegist( false );

			// Name
			CDnItem *pItem = (CDnItem*)m_pItemSlotButton[i]->GetItem();
			wchar_t szTemp[256]={0};
			wchar_t szTempSub1[256]={0};	// Secondary

			if( pItem->GetEnchantLevel() != 0 )
				swprintf_s( szTempSub1, 256, L"+%d ", pItem->GetEnchantLevel() );
#ifndef _FINAL_BUILD
			if( GetAsyncKeyState( VK_LSHIFT ) )
				swprintf_s( szTemp, 256, L"%s[ID:%d]", szTempSub1, pItem->GetClassID() );
			else
				swprintf_s( szTemp, 256, L"%s%s", szTempSub1, pItem->GetName() );
#else
			swprintf_s( szTemp, 256, L"%s%s", szTempSub1, pItem->GetName() );
#endif

			m_pName->SetTextColor( DN_INTERFACE::STRING::ITEM::RANK_2_COLOR( pItem->GetItemRank() ) );
			m_pName->SetText( szTemp );
		}
	}
}

void CDnItemCompoundMix2Dlg::SetProb( int nProb )
{
	if( nProb != 0 )
	{
		WCHAR wszProb[8];
		swprintf_s(wszProb, _countof(wszProb), L"%d%%", nProb );
		m_pProb->SetText( wszProb );
		m_pProb->Show( true );
	}
}

void CDnItemCompoundMix2Dlg::SetOK( bool bOK )
{
	if( bOK )
		m_pButtonOK->Enable( true );
	else
		m_pButtonOK->Enable( false );
}

void CDnItemCompoundMix2Dlg::EnableButton( bool bEnable )
{
	m_pButtonOK->Enable( bEnable );
	m_pButtonCancel->Enable( bEnable );
}

void CDnItemCompoundMix2Dlg::SetDescription( const WCHAR *wszStr )
{
	m_pDescription->SetText( wszStr );
}

void CDnItemCompoundMix2Dlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( strstr( pControl->GetControlName(), "ID_ITEM_SLOT" ) ) {
			int nSlotIndex;
			sscanf_s( pControl->GetControlName(), "ID_ITEM_SLOT%d", &nSlotIndex );

			//#53608 Ignore clicks on empty slots.
			if( m_pItemSlotButton[nSlotIndex]->GetItem() == NULL )
				return;

			m_nSelectSlotIndex = nSlotIndex;
			if( m_nSelectSlotIndex < 0 || m_nSelectSlotIndex >= NUM_NEED_ITEM )
			{
				m_nSelectSlotIndex = -1;
				return;
			}
			
			RefreshSelectItemSlot();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnItemCompoundMix2Dlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

INT64 CDnItemCompoundMix2Dlg::GetSelectItemSerialID()
{
	if( m_nSelectSlotIndex < 0 || m_nSelectSlotIndex >= NUM_NEED_ITEM ) return 0;
	if( m_pItemSlotButton[m_nSelectSlotIndex]->GetItem() == NULL ) return 0;

	return ((CDnItem*)m_pItemSlotButton[m_nSelectSlotIndex]->GetItem())->GetSerialID();
}