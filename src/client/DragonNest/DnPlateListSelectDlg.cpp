#include "StdAfx.h"
#include "DnPlateListSelectDlg.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnPlateListDlg.h"
#include "DnPlateItemSlotDlg.h"
#include "DnPlateMixViewDlg.h"
#include "DnPlateMainDlg.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPlateListSelectDlg::CDnPlateListSelectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pPlateListDlg(NULL)
, m_pButtonNext(NULL)
, m_pButtonPrev(NULL)
, m_pStaticPage(NULL)
, m_pStaticGold(NULL)
, m_pStaticSilver(NULL)
, m_pStaticBronze(NULL)
, m_dwColorBronze(0)
, m_dwColorGold(0)
, m_dwColorSilver(0)
, m_nCurSelect(0)
{
	memset( m_pNeedRune, NULL, sizeof(m_pNeedRune) );
	memset( m_pPlateInfo, 0, sizeof(m_pPlateInfo));
	memset( m_RuneInfo, 0, sizeof(m_RuneInfo));
	memset( m_pNeedRuneSlotButton, 0, sizeof(m_pNeedRuneSlotButton));
	memset( m_pPlateInfo, 0, sizeof(m_pPlateInfo));
}

CDnPlateListSelectDlg::~CDnPlateListSelectDlg(void)
{
	SAFE_DELETE( m_pPlateListDlg );

	for( int itr = 0; itr < SLOT_NUM; ++itr )
		SAFE_DELETE(m_pNeedRune[itr]);
}

void CDnPlateListSelectDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlateListSelectDlg.ui" ).c_str(), bShow );
}

void CDnPlateListSelectDlg::InitialUpdate()
{
	char szControlName[32];

	for( int i = 0; i < SLOT_NUM; ++i )
	{
		sprintf_s(szControlName, _countof(szControlName), "%s%d", "ID_ITEM_SLOT", i);
		m_pNeedRuneSlotButton[i] = GetControl<CDnItemSlotButton>(szControlName);
		m_pNeedRuneSlotButton[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
	}

	m_pStaticGold = GetControl<CEtUIStatic>( "ID_GOLD" );
	m_pStaticSilver = GetControl<CEtUIStatic>( "ID_SILVER" );
	m_pStaticBronze = GetControl<CEtUIStatic>( "ID_BRONZE" );
	m_dwColorGold = m_pStaticGold->GetTextColor();
	m_dwColorSilver = m_pStaticSilver->GetTextColor();
	m_dwColorBronze = m_pStaticBronze->GetTextColor();

	m_pPlateListDlg = new CDnPlateListDlg( UI_TYPE_CHILD, this );
	m_pPlateListDlg->Initialize( false );
}

void CDnPlateListSelectDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	ShowChildDialog(m_pPlateListDlg, bShow);
	if( bShow )
	{
	}
	else
	{
	}

	CEtUIDialog::Show( bShow );
}

void CDnPlateListSelectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnPlateListSelectDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		{
			
		}
		break;
	case WM_LBUTTONDBLCLK:
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

// Set required runes for the sentence corresponding to the index.
void CDnPlateListSelectDlg::RefreshPlateList()
{
	for( int itr = 0; itr < SLOT_NUM; ++itr )
	{
		SAFE_DELETE(m_pNeedRune[itr]);
		m_pNeedRuneSlotButton[itr]->ResetSlot();
	}
	m_pStaticGold->SetText( L"" );
	m_pStaticSilver->SetText( L"" );
	m_pStaticBronze->SetText( L"" );
	m_pStaticGold->SetTextColor( m_dwColorGold );
	m_pStaticSilver->SetTextColor( m_dwColorSilver );
	m_pStaticBronze->SetTextColor( m_dwColorBronze );

	int nPlateItemID = ((CDnPlateMainDlg *)m_pParentDialog)->GetPlateItemID();
	const CDnItemCompounder::S_PLATE_INFO* pPlateInfo = GetItemTask().GetPlateInfoByItemID( nPlateItemID );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEMCOMPOUND );
	if( !pSox ) return;

	for( int nIndex = 0; nIndex < NUM_MAX_PLATE_COMPOUND_VARI; ++nIndex )
	{
		//Save the index of the final sentence.
		m_pPlateInfo[nIndex] = pPlateInfo->aiCompoundTableID[nIndex];

		if( 0 != pPlateInfo->aiCompoundTableID[nIndex] )
		{
			for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
			{
				// Find Crest info and store required Rune info for the Crest.
				if( pSox->GetFieldFromLablePtr(pSox->GetItemID(itr), "_SuccessItemID1")->GetInteger() == pPlateInfo->aiCompoundTableID[nIndex] )
				{
					int nNum = pSox->GetItemID(itr);
					m_RuneInfo[nIndex].m_nID[0] = pSox->GetFieldFromLablePtr( nNum, "_Slot1Id" )->GetInteger();
					m_RuneInfo[nIndex].m_nNum[0] = pSox->GetFieldFromLablePtr( nNum, "_Slot1Num" )->GetInteger();

					m_RuneInfo[nIndex].m_nID[1] = pSox->GetFieldFromLablePtr( nNum, "_Slot2Id" )->GetInteger();
					m_RuneInfo[nIndex].m_nNum[1] = pSox->GetFieldFromLablePtr( nNum, "_Slot2Num" )->GetInteger();

					m_RuneInfo[nIndex].m_nID[2] = pSox->GetFieldFromLablePtr( nNum, "_Slot3Id" )->GetInteger();
					m_RuneInfo[nIndex].m_nNum[2] = pSox->GetFieldFromLablePtr( nNum, "_Slot3Num" )->GetInteger();

					m_RuneInfo[nIndex].m_nID[3] = pSox->GetFieldFromLablePtr( nNum, "_Slot4Id" )->GetInteger();
					m_RuneInfo[nIndex].m_nNum[3] = pSox->GetFieldFromLablePtr( nNum, "_Slot4Num" )->GetInteger();

					m_RuneInfo[nIndex].m_nID[4] = pSox->GetFieldFromLablePtr( nNum, "_Slot5Id" )->GetInteger();
					m_RuneInfo[nIndex].m_nNum[4] = pSox->GetFieldFromLablePtr( nNum, "_Slot5Num" )->GetInteger();
					break;
				}
			}
		}
	}

	m_pPlateListDlg->RefreshPlateList();
}

int CDnPlateListSelectDlg::GetSelectedCompoundID()
{
	return m_pPlateListDlg->GetSelectedCompoundID();
}

void CDnPlateListSelectDlg::SelectPlate( int nIndex )
{
	m_nCurSelect = nIndex;

	TItemInfo itemInfo;
	m_vNeedItemInfo.clear();
	for( int itr = 0; itr < SLOT_NUM; ++itr )
	{
		SAFE_DELETE( m_pNeedRune[itr] );
		m_pNeedRuneSlotButton[itr]->ResetSlot();

		if( CDnItem::MakeItemInfo( m_RuneInfo[nIndex].m_nID[itr], 1, itemInfo ) == false ) continue;

		int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( m_RuneInfo[nIndex].m_nID[itr] );

		m_pNeedRune[itr] = GetItemTask().CreateItem( itemInfo );
		m_pNeedRuneSlotButton[itr]->SetItem(m_pNeedRune[itr], CDnSlotButton::ITEM_ORIGINAL_COUNT);
		m_pNeedRuneSlotButton[itr]->SetJewelCount( m_RuneInfo[nIndex].m_nNum[itr], nCurItemCount );

		CDnItemTask::ItemCompoundInfo CompoundInfo;
		CompoundInfo.iItemID = m_RuneInfo[nIndex].m_nID[itr];
		CompoundInfo.iUseCount = m_RuneInfo[nIndex].m_nNum[itr];
		m_vNeedItemInfo.push_back(CompoundInfo);

		if( nCurItemCount < m_RuneInfo[nIndex].m_nNum[itr] )
			m_pNeedRuneSlotButton[itr]->SetRegist( true );
		else
			m_pNeedRuneSlotButton[itr]->SetRegist( false );
	}

	int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	int nPrice = (int)CompoundInfo.iCost;
	int nGold = nPrice/10000;
	int nSilver = (nPrice%10000)/100;
	int nBronze = nPrice%100;
	m_pStaticGold->SetIntToText( nGold );
	m_pStaticSilver->SetIntToText( nSilver );
	m_pStaticBronze->SetIntToText( nBronze );
	if( nPrice > GetItemTask().GetCoin() )
	{
		if( nPrice >= 10000 ) m_pStaticGold->SetTextColor( 0xFFFF0000, true );
		if( nPrice >= 100 ) m_pStaticSilver->SetTextColor( 0xFFFF0000, true );
		m_pStaticBronze->SetTextColor( 0xFFFF0000, true );
	}
	else
	{
		m_pStaticGold->SetTextColor( m_dwColorGold );
		m_pStaticSilver->SetTextColor( m_dwColorSilver );
		m_pStaticBronze->SetTextColor( m_dwColorBronze );
	}
}

bool CDnPlateListSelectDlg::EnoughRune()
{
	for( int itr = 0; itr < (int)m_vNeedItemInfo.size(); ++itr )
	{
		if( m_RuneInfo[m_nCurSelect].m_nNum[itr] > GetItemTask().GetCharInventory().GetItemCount( m_vNeedItemInfo[itr].iItemID ) )
			return false;
	}

	return true;
}

bool CDnPlateListSelectDlg::EnoughCoin()
{
	int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	return GetItemTask().GetCoin() >= CompoundInfo.iCost;
}

/*
CDnPlateListSelectDlg::CDnPlateListSelectDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_emStep(stepBase)
, m_pItem(NULL)
, m_pItemSlotButton(NULL)
, m_pStaticFix(NULL)
, m_pButtonOK(NULL)
, m_pButtonCancel(NULL)
, m_pStaticBack(NULL)
, m_pStaticGold(NULL)
, m_pStaticSilver(NULL)
, m_pStaticBronze(NULL)
, m_pQuickSlotButton(NULL)
, m_pPlateListDlg(NULL)
, m_pPlateMixViewDlg(NULL)
{
}

CDnPlateListSelectDlg::~CDnPlateListSelectDlg(void)
{
	SAFE_DELETE( m_pPlateListDlg );
	SAFE_DELETE( m_pPlateMixViewDlg );
	SAFE_DELETE( m_pItem );
}

void CDnPlateListSelectDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlateListSelectDlg.ui" ).c_str(), bShow );
}

void CDnPlateListSelectDlg::InitialUpdate()
{
	m_pItemSlotButton = GetControl<CDnItemSlotButton>("ID_ITEM_PLATE");
	m_pStaticFix = GetControl<CEtUIStatic>("ID_STATIC_FIX");
	m_pStaticFix->Show( false );

	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");

	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");

	m_pStaticBack = GetControl<CEtUIStatic>("ID_STATIC_BACK");

	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");

	m_pPlateListDlg = new CDnPlateListDlg( UI_TYPE_CHILD, this );
	m_pPlateListDlg->Initialize( false );

	for( int i = 0; i < 5; ++i )
	{
		CDnPlateItemSlotDlg *pDlg;
		pDlg = new CDnPlateItemSlotDlg( UI_TYPE_CHILD, this );
		pDlg->Init( i+1 );
		m_SlotDlgGroup.AddDialog( i, pDlg );
	}

	m_pPlateMixViewDlg = new CDnPlateMixViewDlg( UI_TYPE_CHILD, this );
	m_pPlateMixViewDlg->Initialize( false );
}

void CDnPlateListSelectDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{

	}
	else
	{
		ChangeStep( stepBase );
		GetInterface().CloseBlind();
	}

	CEtUIDialog::Show( bShow );
}

bool CDnPlateListSelectDlg::IsEnableRegisterPlateItem(CDnSlotButton* pDragButton, CDnItem* pItem) const
{
	if( pDragButton->GetSlotType() != ST_INVENTORY )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3610 ), MB_OK );
		return false;
	}

	// Skip if re-registering an already listed item.
	if( m_pQuickSlotButton && (pItem == m_pQuickSlotButton->GetItem()) )
	{
		return false;
	}

	// Check item type
	if( pItem->GetItemType() == CDnItem::Quest )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
		return false;
	}

	// Check if it is a plate item.
	if( pItem->GetItemType() != CDnItem::Plate )
	{
		GetInterface().MessageBox( 9008, MB_OK );
		return false;
	}

	return true;
}

void CDnPlateListSelectDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_OK") )
		{
			if( m_emStep == stepListSelect )
			{
				int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
				CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
				GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );
				if( GetItemTask().GetCoin() < CompoundInfo.iCost )
				{
					// When entering via double-click with insufficient fee, a message will appear even though the button is not enabled.
					// This does not seem great, so just do nothing and move on.
					//GetInterface().MessageBox( 4036, MB_OK );
					return;
				}
				
				ChangeStep( stepSlotItem );
			}
			else if( m_emStep == stepSlotItem )
			{
				if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) )
				{
					GetInterface().MessageBox( 7012, MB_OK );
					return;
				}

				// Must process after the 'Do you want to combine?' message.
				GetInterface().MessageBox( 9009, MB_YESNO, COMPOUND_CONFIRM_DIALOG, this );
				return;
			}
			return;
		}
		else if( IsCmdControl("ID_BUTTON_CANCEL") )
		{
			ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl("ID_CLOSE_DIALOG"), 0 );
			return;
		}
		else if( IsCmdControl("ID_ITEM_PLATE") )
		{
			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			CDnSlotButton *pPressedButton = ( CDnItemSlotButton * )pControl;

			// Remove via right-click
			if( uMsg == WM_RBUTTONUP )
			{
				ChangeStep( stepBase );
				return;
			}

			if( pDragButton )
			{
				drag::ReleaseControl();

				CDnItem *pItem = static_cast<CDnItem *>(pDragButton->GetItem());
				if (pItem == NULL || IsEnableRegisterPlateItem(pDragButton, pItem) == false)
				{
					pDragButton->EnableSplitMode(true);
					return;
				}

				CDnItem *pPressedItem = static_cast<CDnItem *>(pPressedButton->GetItem());

				// If placing another item on a slot that already has one,
				if( pPressedItem )
				{
					// Remove the item from the window.
					SAFE_DELETE(m_pItem);
					pPressedButton->ResetSlot();
					m_pQuickSlotButton->SetRegist( false );
					m_pQuickSlotButton = NULL;
				}

				// When registering a plate, instead of passing the inventory item pointer directly like before,
				//pPressedButton->SetItem( pItem );
				// Create and insert a temporary item like this.
				TItemInfo itemInfo;
				pItem->GetTItemInfo(itemInfo);
				m_pItem = GetItemTask().CreateItem( itemInfo );
				pPressedButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

				m_pQuickSlotButton = (CDnQuickSlotButton *)pDragButton;
				m_pQuickSlotButton->SetRegist( true );

				ChangeStep( stepPlateRegist );
			}

			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPlateListSelectDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case COMPOUND_CONFIRM_DIALOG:
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( IsCmdControl("ID_YES") )
			{
				// After sending registration request to server, show cutscene movie upon receiving the corresponding packet.
OutputDebug( "Send plate craft packet\n" );
				GetItemTask().RequestEmblemCompoundOpen( static_cast<CDnItem*>(m_pItemSlotButton->GetItem()), m_pPlateListDlg->GetSelectedCompoundID(), m_vNeedItemInfo );

				// Temporary
				//int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
				//CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
				//GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );
				//GetInterface().OpenMovieProcessDlg(
				//	"PlateCombine.avi", CompoundInfo.fTimeRequired / 1000.0f, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9011 ), COMPOUND_MOVIE_DIALOG, this );
			}
		}
		break;
	case COMPOUND_MOVIE_DIALOG:
		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			// Invisible button. Indicates that processing has finished.
			if( IsCmdControl("ID_PROCESS_COMPLETE") )
			{
				// Send completion packet.
				// Send...
				GetItemTask().RequestEmblemCompound( static_cast<CDnItem*>(m_pItemSlotButton->GetItem()), m_pPlateListDlg->GetSelectedCompoundID(), m_vNeedItemInfo );
OutputDebug( "Send crest jewel combination complete packet\n" );

			}
			else if( IsCmdControl("ID_BUTTON_CANCEL") )
			{
				// Send cancel packet.
				// Send...
OutputDebug( "Send crest jewel combination cancel packet\n" );
				GetItemTask().RequestEmblemCompoundCancel();
			}
		}
		break;
	}
}

void CDnPlateListSelectDlg::OnRecvEmblemCompoundOpen()
{
	// After obtaining the appropriate duration according to the item to combine
	int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
	GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

	GetInterface().OpenMovieProcessDlg(
		"PlateCombine.avi", CompoundInfo.fTimeRequired / 1000.0f, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8013 ), COMPOUND_MOVIE_DIALOG, this );

	// Instead of calling directly, when substituting 'this' on calls from elsewhere,
	// GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLATE_LIST_SELECT_DIALOG)
}

void CDnPlateListSelectDlg::OnRecvEmblemCompound()
{
	// Whether Crest Jewel combination succeeds or fails, return to default step.
	ChangeStep( stepBase );
}

void CDnPlateListSelectDlg::ChangeStep( emPlateStep emStep )
{
	m_emStep = emStep;

	if( m_emStep == stepBase )
	{
		SAFE_DELETE(m_pItem);
		m_pItemSlotButton->ResetSlot();
		m_pStaticFix->Show( false );

		if( m_pQuickSlotButton )
		{
			m_pQuickSlotButton->SetRegist( false );
			m_pQuickSlotButton = NULL;
		}

		m_pStaticBack->Show( true );

		m_pStaticGold->SetIntToText(0);
		m_pStaticSilver->SetIntToText(0);
		m_pStaticBronze->SetIntToText(0);

		m_pButtonOK->Enable( false );
		m_pButtonOK->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 74 ) );
		m_pButtonCancel->Enable( true );

		ClosePlateList();
		m_SlotDlgGroup.CloseAllDialog();
		ClosePlateMixView();
	}
	else if( m_emStep == stepPlateRegist )
	{
		m_pStaticFix->Show( true );
		m_pStaticBack->Show( false );

		// Reset fee
		m_pStaticGold->SetIntToText(0);
		m_pStaticSilver->SetIntToText(0);
		m_pStaticBronze->SetIntToText(0);

		m_pButtonOK->Enable( false );
		m_pButtonOK->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 74 ) );

		RefreshPlateList();
		m_SlotDlgGroup.CloseAllDialog();
		ClosePlateMixView();
	}
	else if( m_emStep == stepListSelect )
	{
		int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

		// Retrieve and display the fee. Since this won't exceed int range, force-cast it.
		int nPrice = (int)CompoundInfo.iCost;
		int nGold = nPrice/10000;
		int nSilver = (nPrice%10000)/100;
		int nBronze = nPrice%100;
		m_pStaticGold->SetIntToText( nGold );
		m_pStaticSilver->SetIntToText( nSilver );
		m_pStaticBronze->SetIntToText( nBronze );

		if( GetItemTask().GetCoin() >= CompoundInfo.iCost )
			m_pButtonOK->Enable( true );
		else
			m_pButtonOK->Enable( false );
		m_pButtonOK->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 74 ) );
	}
	else if( m_emStep == stepSlotItem )
	{
		int nCompoundID = m_pPlateListDlg->GetSelectedCompoundID();
		CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;
		GetItemTask().GetCompoundInfo( nCompoundID, &CompoundInfo );

		m_pButtonOK->Enable( false );
		m_pButtonOK->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9003 ) );

		// Should normally use NUM_MAX_NEED_ITEM, but crest plates use at most 5.
		int nNumNeedItem;
		for(nNumNeedItem = 0; nNumNeedItem < 5; ++nNumNeedItem)
			if( CompoundInfo.aiItemID[nNumNeedItem] == 0 ) break;

		// Show the appropriate dialog according to the number of required slots.
ASSERT(nNumNeedItem&&"If needed slot count for combination is 0, isn't that invalid data.");

		// Show the item slot dialog according to the number of required slots.
		ClosePlateList();
		m_SlotDlgGroup.ShowDialog(nNumNeedItem-1, true);
		ShowPlateMixView();

		int nProb = (int)(CompoundInfo.fSuccessPossibility * 100.0f);
		if( CompoundInfo.fSuccessPossibility * 100.0f - (float)nProb > 0.999 ) nProb += 1;
		int nResultItemID = 0;
		if( CompoundInfo.aiSuccessTypeOrCount[0] == -1 && CompoundInfo.aiSuccessItemID[0] > 0 )
			nResultItemID = CompoundInfo.aiSuccessItemID[0];
		m_pPlateMixViewDlg->SetInfo( nProb, nResultItemID );
		bool bResult = ((CDnPlateItemSlotDlg *)m_SlotDlgGroup.GetDialog(nNumNeedItem-1))->SetCompoundInfo(CompoundInfo.aiItemID, CompoundInfo.aiItemCount);

		// Set up the required item info struct to send to the server at the item slot stage.
		m_vNeedItemInfo.clear();
		for( int i = 0; i < nNumNeedItem; ++i )
		{
			CDnItemTask::ItemCompoundInfo Info;
			Info.iItemID = CompoundInfo.aiItemID[ i ];
			Info.iUseCount = CompoundInfo.aiItemCount[ i ];
			m_vNeedItemInfo.push_back(Info);
		}

		if( bResult )
			m_pButtonOK->Enable( true );
		else
			m_pButtonOK->Enable( false );
	}
}

void CDnPlateListSelectDlg::RefreshPlateList()
{
	if( m_pPlateListDlg )
	{
		if( !m_pPlateListDlg->IsShow() ) ShowChildDialog( m_pPlateListDlg, true );
	}
	m_pPlateListDlg->RefreshPlateList();
}

void CDnPlateListSelectDlg::ClosePlateList()
{
	if( m_pPlateListDlg )
	{
		ShowChildDialog( m_pPlateListDlg, false );
	}
}

void CDnPlateListSelectDlg::ShowPlateMixView()
{
	if( m_pPlateMixViewDlg )
	{
		if( !m_pPlateMixViewDlg->IsShow() ) ShowChildDialog( m_pPlateMixViewDlg, true );
	}
}

void CDnPlateListSelectDlg::ClosePlateMixView()
{
	if( m_pPlateMixViewDlg )
	{
		ShowChildDialog( m_pPlateMixViewDlg, false );
	}
}

void CDnPlateListSelectDlg::SetPlateItem( CDnQuickSlotButton *pPressedButton )
{
	CDnItem *pItem = static_cast<CDnItem *>(pPressedButton->GetItem());

	// Skip if re-registering an already listed item.
	if( m_pQuickSlotButton && (pItem == m_pQuickSlotButton->GetItem()) )
	{
		return;
	}

	// Check item type
	if( pItem->GetItemType() == CDnItem::Quest )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
		return;
	}

	// Check if it is a plate item.
	if( pItem->GetItemType() != CDnItem::Plate )
	{
		GetInterface().MessageBox( 9008, MB_OK );
		return;
	}

	// Re-registering is better than saying 'Only one slot, cannot register more'.
	CDnItem *pOriginItem = static_cast<CDnItem *>(m_pItemSlotButton->GetItem());
	if( pOriginItem )
	{
		// Remove the item from the window.
		SAFE_DELETE(m_pItem);
		m_pItemSlotButton->ResetSlot();
		m_pStaticFix->Show( false );
		m_pQuickSlotButton->SetRegist( false );
		m_pQuickSlotButton = NULL;
	}

	// When registering a plate, instead of passing the inventory item pointer directly like before,
	//pPressedButton->SetItem( pItem );
	// Create and insert a temporary item like this.
	TItemInfo itemInfo;
	pItem->GetTItemInfo(itemInfo);
	m_pItem = GetItemTask().CreateItem(itemInfo);
	m_pItemSlotButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

	CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	m_pQuickSlotButton = (CDnQuickSlotButton *)pPressedButton;
	m_pQuickSlotButton->SetRegist( true );

	ChangeStep( stepPlateRegist );
}

int CDnPlateListSelectDlg::GetPlateItemID()
{
	if( m_emStep != stepBase )
	{
		CDnItem *pPlateItem = static_cast<CDnItem *>(m_pItemSlotButton->GetItem());
		return pPlateItem->GetClassID();
	}
	else return 0;
}*/