#include "StdAfx.h"
#include "DnPotentialJewelCleanDlg.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "ItemSendPacket.h"
#include "DnPotentialJewel.h"
#include "DnTableDB.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#endif
#include "DnCommonTask.h"
#include "TaskManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL

CDnPotentialJewelCleanDlg::CDnPotentialJewelCleanDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
{
	m_pItem = NULL;
	m_pItemSlot = NULL;
	m_pStaticText = NULL;
	m_pButtonApply = NULL;
	m_pButtonQuickSlot = NULL;

	m_pPotentialItem = NULL;
	m_nSoundIdx = -1;
}

CDnPotentialJewelCleanDlg::~CDnPotentialJewelCleanDlg()
{
	SAFE_DELETE( m_pItem );
	CEtSoundEngine::GetInstance().RemoveSound( m_nSoundIdx );
}

void CDnPotentialJewelCleanDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenItemPower_CleanDlg.ui" ).c_str(), bShow );

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10041 );
	if( strlen( szFileName ) > 0 )
		m_nSoundIdx = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
}

void CDnPotentialJewelCleanDlg::InitialUpdate()
{
	m_pItemSlot = GetControl<CDnItemSlotButton>("ID_ITEM");
	m_pStaticText = GetControl<CEtUIStatic>("ID_TEXT_ASK");
	m_pButtonApply = GetControl<CEtUIButton>("ID_OK");
}

void CDnPotentialJewelCleanDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CDnCustomDlg::Show( bShow );

	// Transition to None state on server.	
	CDnCommonTask *pTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask( "CommonTask" ));
	if( pTask )			
		pTask->SendWindowState( bShow == true ? WINDOW_BLIND : WINDOW_NONE );

	if( bShow )
	{
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg && pInvenDlg->IsShow() && !drag::IsValid() )
			pInvenDlg->ShowTab( ST_INVENTORY );

		WCHAR wszText[256] = {0,};

		int nStringIndex = 0;
		if( m_pPotentialItem->GetCategoryType() == ITEM_CLEAN_CATEGORY_WEAPON )
			nStringIndex = 7928;
		else if( m_pPotentialItem->GetCategoryType() == ITEM_CLEAN_CATEGORY_ARMOR )
			nStringIndex = 7927;
		else if( m_pPotentialItem->GetCategoryType() == ITEM_CLEAN_CATEGORY_ACCESSORY )
			nStringIndex = 7929;

		if( nStringIndex == 0)
			swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8053), m_pPotentialItem->GetName() );
		else
			swprintf_s( wszText, _countof(wszText), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringIndex), m_pPotentialItem->GetName() );

		m_pStaticText->SetText( wszText );

		SetRenderPriority( this, true );
	}
	else
	{
		m_pItemSlot->ResetSlot();
		m_pStaticText->SetText( L"" );

		m_pButtonApply->Enable( false );

		if( m_pButtonQuickSlot )
		{
			m_pButtonQuickSlot->SetRegist( false );
			m_pButtonQuickSlot = NULL;
		}
	}
}

void CDnPotentialJewelCleanDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) ) {
			if( !m_pButtonQuickSlot || !m_pPotentialItem ) return;
			SendPotentialJewel( m_pButtonQuickSlot->GetSlotIndex(), m_pItem->GetSerialID(), ( m_pPotentialItem->IsCashItem() == true ) ? m_pPotentialItem->GetSerialID() : m_pPotentialItem->GetSlotIndex() );
			m_pButtonApply->Enable( false );
		}
		else if( IsCmdControl( "ID_CANCEL" ) ) {
			Show(false);
			return;
		}
		else if( IsCmdControl("ID_CLOSE"))
		{
			Show(false);
			return;
		}
		else if( IsCmdControl("ID_ITEM") )
		{
			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			CDnSlotButton *pPressedButton = ( CDnItemSlotButton * )pControl;

			// Remove via right-click
			if( uMsg == WM_RBUTTONUP )
			{
				SAFE_DELETE(m_pItem);
				pPressedButton->ResetSlot();
				if( m_pButtonQuickSlot )
				{
					m_pButtonQuickSlot->SetRegist( false );
					m_pButtonQuickSlot = NULL;
				}
				CheckPotentialItem();
				return;
			}

			if( pDragButton )
			{
				drag::ReleaseControl();

				if( pDragButton->GetItemType() != MIInventoryItem::Item )
					return;

				CDnItem *pItem = static_cast<CDnItem *>(pDragButton->GetItem());
				if( pItem == NULL || IsPotentialableItem( pDragButton ) == false )
				{
					pDragButton->DisableSplitMode( true );
					return;
				}

				CDnItem *pPressedItem = static_cast<CDnItem *>(pPressedButton->GetItem());
				if( pPressedItem )
				{
					SAFE_DELETE( m_pItem );
					pPressedButton->ResetSlot();
					m_pButtonQuickSlot->SetRegist( false );
					m_pButtonQuickSlot = NULL;
				}

				TItemInfo itemInfo;
				pItem->GetTItemInfo( itemInfo );
				m_pItem = GetItemTask().CreateItem( itemInfo );
				pPressedButton->SetItem( m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );

				CheckPotentialItem();
				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

				m_pButtonQuickSlot = (CDnQuickSlotButton *)pDragButton;
				m_pButtonQuickSlot->SetRegist( true );
			}
			return;
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_POTENTIAL);
		}
#endif
	}
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

}

void CDnPotentialJewelCleanDlg::SetPotentialItem( CDnQuickSlotButton *pPressedButton )
{

	CDnItem *pItem = static_cast<CDnItem *>(pPressedButton->GetItem());

	// Skip if re-registering an already listed item.
	if( m_pButtonQuickSlot && (pItem == m_pButtonQuickSlot->GetItem()) )
	{
		return;
	}

	// Check item type
	switch( pItem->GetItemType() ) {
		case ITEMTYPE_WEAPON:
		case ITEMTYPE_PARTS:
			break;
		default:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
			return;
	}
	if (pItem->GetTypeParam(0) == 0) {
		GetInterface().MessageBox(8051, MB_OK);
		return;
	}
	if( ((CDnPotentialJewel*)m_pPotentialItem)->IsCanApplySealedItem() == false )
	{
		if( pItem->IsSoulbBound() == false )
		{
			GetInterface().MessageBox(8051, MB_OK );
			return;
		}
	}

	CDnItem *pOriginItem = static_cast<CDnItem *>(m_pItemSlot->GetItem());
	if( pOriginItem )
	{
		SAFE_DELETE( m_pItem );
		m_pItemSlot->ResetSlot();
		m_pButtonQuickSlot->SetRegist( false );
		m_pButtonQuickSlot = NULL;
	}

	TItemInfo itemInfo;
	pItem->GetTItemInfo( itemInfo );
	m_pItem = GetItemTask().CreateItem( itemInfo );
	m_pItemSlot->SetItem( m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );

	CheckPotentialItem();
	CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	m_pButtonQuickSlot = (CDnQuickSlotButton *)pPressedButton;
	m_pButtonQuickSlot->SetRegist( true );
}


bool CDnPotentialJewelCleanDlg::IsPotentialableItem( CDnSlotButton *pDragButton )
{
	if( pDragButton->GetSlotType() != ST_INVENTORY )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8051), MB_OK );
		return false;
	}

	CDnItem *pItem = static_cast<CDnItem *>(pDragButton->GetItem());

	// Skip if re-registering an already listed item.
	if( m_pButtonQuickSlot && (pItem == m_pButtonQuickSlot->GetItem()) )
	{
		return false;
	}

	// Check item type
	switch( pItem->GetItemType() ) {
		case ITEMTYPE_WEAPON:
		case ITEMTYPE_PARTS:
			break;
		default:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK );
			return false;
	}
	if( pItem->GetTypeParam(0) == 0 ) {
		GetInterface().MessageBox(8051, MB_OK );
		return false;
	}

	int nApplyApplicableValue = ((CDnPotentialJewel*)m_pPotentialItem)->GetApplyApplicableValue();
	int nCurApplicableValue = pItem->GetApplicableValue();
	if( ( nCurApplicableValue & nApplyApplicableValue ) != nCurApplicableValue ) {
		GetInterface().MessageBox(8051, MB_OK );
		return false;
	}
	bool bErase = ((CDnPotentialJewel*)m_pPotentialItem)->IsErasable();
	if( !bErase && pItem->GetPotentialIndex() > 0 ) {
		GetInterface().MessageBox(8051, MB_OK );
		return false;
	}
	if( bErase && pItem->GetPotentialIndex() <= 0 ) {
		GetInterface().MessageBox(8051, MB_OK );
		return false;
	}
	if( ((CDnPotentialJewel*)m_pPotentialItem)->IsCanApplySealedItem() == false )
	{
		if( pItem->IsSoulbBound() == false )
		{
			GetInterface().MessageBox(8051, MB_OK );
			return false;
		}
	}

	return true;
}

void CDnPotentialJewelCleanDlg::CheckPotentialItem()
{
	if( m_pItemSlot && !m_pItemSlot->IsEmptySlot() && m_pItem )
	{
		m_pButtonApply->Enable( true );
	}
	else
	{
		m_pButtonApply->Enable( false );
	}
}

void CDnPotentialJewelCleanDlg::SetPotentialItem( CDnItem *pItem )
{
	m_pPotentialItem = pItem;
	if( m_pPotentialItem && m_pPotentialItem->GetItemType() != ITEMTYPE_POTENTIAL_JEWEL ) {
		m_pPotentialItem = NULL;
		return;
	}
}


void CDnPotentialJewelCleanDlg::OnRecvPotentialItem( int nResult, int nSlotIndex )
{
	if( nResult == ERROR_NONE )
	{
		CDnItem *pItem = GetItemTask().GetCharInventory().GetItem( nSlotIndex );
		if( !pItem || !m_pItem || pItem->GetClassID() != m_pItem->GetClassID() )
			return;

		if (m_nSoundIdx >= 0)
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIdx, false );
	}

	SAFE_DELETE(m_pItem);
	m_pItemSlot->ResetSlot();

	if( m_pButtonQuickSlot )
	{
		m_pButtonQuickSlot->SetRegist( false );
		m_pButtonQuickSlot = NULL;
	}

	// Seems easier to just close the window
	Show( false );
}
#endif // // PRE_MOD_POTENTIAL_JEWEL_RENEWAL