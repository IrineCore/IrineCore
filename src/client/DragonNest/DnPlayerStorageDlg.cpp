#include "StdAfx.h"

#include "DnPlayerStorageDlg.h"
#include "DnStorageDlg.h"
#include "DnSlotButton.h"
#include "DnItemTask.h"
#include "DnInterfaceDlgID.h"
#include "DnInterface.h"
#include "DnInterfaceString.h"
#include "DnMoneyInputDlg.h"
#include "DnStoreConfirmExDlg.h"
#include "DnMainMenuDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnPlayerStorageDlg::CDnPlayerStorageDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pStoreCommonDlg(NULL)
, m_pStaticGold(NULL)
, m_pStaticSilver(NULL)
, m_pStaticBronze(NULL)
, m_pButtonMoneyIn(NULL)
, m_pButtonMoneyOut(NULL)
, m_pMoneyInputDlg(NULL)
, m_pSplitConfirmExDlg(NULL)
, m_iRemoteItemID( 0 )
{
	m_emMoneyType = MONEY_CONFIRM_IN;
}

CDnPlayerStorageDlg::~CDnPlayerStorageDlg(void)
{
	SAFE_DELETE( m_pMoneyInputDlg );
	SAFE_DELETE( m_pSplitConfirmExDlg );
	SAFE_DELETE( m_pStoreCommonDlg );
}

void CDnPlayerStorageDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StoragePrivateDlg.ui" ).c_str(), bShow );
}

void CDnPlayerStorageDlg::InitialUpdate()
{
	m_pStoreCommonDlg = new CDnStorageDlg( UI_TYPE_CHILD, this );
	m_pStoreCommonDlg->Initialize( true );
#if defined( PRE_PERIOD_INVENTORY )
	m_pStoreCommonDlg->EnablePeriodStorage( false );
#endif	// #if defined( PRE_PERIOD_INVENTORY )

	GetItemTask().GetStorageInventory().SetInventoryDialog( this );

	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");

	m_pButtonMoneyIn = GetControl<CEtUIButton>("ID_MONEY_IN");
	m_pButtonMoneyOut = GetControl<CEtUIButton>("ID_MONEY_OUT");

	m_pMoneyInputDlg = new CDnMoneyInputDlg( UI_TYPE_MODAL, NULL, MONEY_INPUT_DIALOG, this );
	m_pMoneyInputDlg->Initialize( false );

	m_pSplitConfirmExDlg = new CDnStoreConfirmExDlg( UI_TYPE_MODAL, NULL, ITEM_ATTACH_CONFIRM_DIALOG, this );
	m_pSplitConfirmExDlg->Initialize( false );
}

void CDnPlayerStorageDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_MONEY_IN" ) )
		{
			// Storage
			m_emMoneyType = MONEY_CONFIRM_IN;
			m_pMoneyInputDlg->SetInfo( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1605 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1607 ) );
			m_pMoneyInputDlg->SetMaxMoney( -1 );
			m_pMoneyInputDlg->Show( true );
			return;
		}
		else if( IsCmdControl("ID_MONEY_OUT") )
		{
			// Withdraw
			m_emMoneyType = MONEY_CONFIRM_OUT;
			m_pMoneyInputDlg->SetInfo( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1606 ), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1608 ) );
			m_pMoneyInputDlg->SetMaxMoney( GetItemTask().GetCoinStorage() );
			m_pMoneyInputDlg->Show( true );
			return;
		}
		else if( IsCmdControl("ID_BT_SORT") )
		{
#if defined( PRE_PERIOD_INVENTORY )
			if( true == m_pStoreCommonDlg->IsPeriodStorageEnable() )
				GetItemTask().RequestSortPeriodStorage();
			else
				GetItemTask().RequestSortStorage();
#else
			GetItemTask().RequestSortStorage();
#endif	// #if defined( PRE_PERIOD_INVENTORY )
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPlayerStorageDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
	//	CDnMouseCursor::GetInstance().ShowCursor( bShow );

	if( bShow )
	{
		// Do not show gold withdrawal/deposit if opened with a remote storage item.
		bool bEnableMoneyInOut = true;
		if( 0 < m_iRemoteItemID )
		{
			bEnableMoneyInOut = false;
		}

		m_pButtonMoneyIn->Enable( bEnableMoneyInOut );
		m_pButtonMoneyOut->Enable( bEnableMoneyInOut );
	}
	else
	{
		if( drag::IsValid() )
		{
			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
			if( pDragButton->GetSlotType() == ST_STORAGE_PLAYER )
			{
				drag::Command(UI_DRAG_CMD_CANCEL);
				drag::ReleaseControl();
			}
		}
	}
#ifdef PRE_ADD_INVEN_EXTENSION
	// Because it is not a tab dialog, force this call so the child dialog's Show-Hide is invoked and the storage purchase button updates properly.
	if( m_pStoreCommonDlg ) m_pStoreCommonDlg->Show( bShow );
#endif
}

void CDnPlayerStorageDlg::SetUseItemCnt( DWORD dwItemCnt )
{
	ASSERT(!(dwItemCnt%ITEM_X)&&"CDnStoreTabDlg::SetUseItemCnt");
	if( dwItemCnt % ITEM_X )
		return;

	// Note: All tabs are usable in the cash inventory.
	//		Tab availability can only be set for normal inventory.
	//
	if( m_pStoreCommonDlg )
	{
		m_pStoreCommonDlg->SetUseItemCnt( dwItemCnt );
	}

}

void CDnPlayerStorageDlg::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnPlayerStorageDlg::SetItem, pItem is NULL!");
	//CDnStorageDlg *pStorageDlg = static_cast<CDnStorageDlg*>(m_groupTabDialog.GetDialog(m_pCommonTabButton->GetTabID()));
	//pStorageDlg->SetItem( pItem );

	m_pStoreCommonDlg->SetItem( pItem );
}

void CDnPlayerStorageDlg::ResetSlot( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnPlayerStorageDlg::SetItem, pItem is NULL!");
	//CDnStorageDlg *pStorageDlg = static_cast<CDnStorageDlg*>(m_groupTabDialog.GetDialog(m_pCommonTabButton->GetTabID()));
	//pStorageDlg->ResetSlot( pItem->GetSlotIndex() );
	m_pStoreCommonDlg->ResetSlot( pItem->GetSlotIndex() );
}

bool CDnPlayerStorageDlg::IsEmptySlot()
{
	if( !m_pStoreCommonDlg )
	{
		return false;
	}

	return m_pStoreCommonDlg->IsEmptySlot();
}

int CDnPlayerStorageDlg::GetEmptySlot()
{
	if( !m_pStoreCommonDlg )
	{
		return -1;
	}

	return m_pStoreCommonDlg->GetEmptySlot();
}

void CDnPlayerStorageDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
		if( !CDnItemTask::IsActive() ) return;
		INT64 nCoin = GetItemTask().GetCoinStorage();
		INT64 nGold = nCoin/10000;
		INT64 nSilver = (nCoin%10000)/100;
		INT64 nBronze = nCoin%100;
		std::wstring strString;

		m_pStaticGold->SetInt64ToText( nGold );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nGold, strString );
		m_pStaticGold->SetTooltipText( strString.c_str() );

		m_pStaticSilver->SetInt64ToText( nSilver );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nSilver, strString );
		m_pStaticSilver->SetTooltipText( strString.c_str() );

		m_pStaticBronze->SetInt64ToText( nBronze );
		DN_INTERFACE::UTIL::GetValue_2_String( (int)nBronze, strString );
		m_pStaticBronze->SetTooltipText( strString.c_str() );
	}
}

void CDnPlayerStorageDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		switch( nID ) 
		{
		case MONEY_INPUT_DIALOG:
			{
				if( IsCmdControl("ID_BUTTON_OK") )
				{
					if( m_emMoneyType == MONEY_CONFIRM_IN )
						GetItemTask().RequestInvenToWare( m_pMoneyInputDlg->GetMoney() );
					else
						GetItemTask().RequestWareToInven( m_pMoneyInputDlg->GetMoney() );
					m_pMoneyInputDlg->Show( false );
				}
				else if( IsCmdControl("ID_BUTTON_CANCEL") )
				{
					m_pMoneyInputDlg->Show( false );
				}
			}
			break;

		case ITEM_ATTACH_CONFIRM_DIALOG:
			{
				if (IsCmdControl("ID_OK"))
				{
					CDnItem* pItem = m_pSplitConfirmExDlg->GetItem();
					if (pItem)
					{
						int nCount = m_pSplitConfirmExDlg->GetItemCount();

						if (nCount <= 0)
						{
							m_pSplitConfirmExDlg->Show(false);
							return;
						}

						CDnInterface::emSTORE_CONFIRM_TYPE confirmType = m_pSplitConfirmExDlg->GetConfirmType();
						if (confirmType == CDnInterface::STORAGE_FROM_INVENTORY)
							RequestMoveItem(MoveType_InvenToWare, pItem, nCount);
					}

					m_pSplitConfirmExDlg->Show(false);
				}
				else if (IsCmdControl("ID_CANCEL"))
				{
					m_pSplitConfirmExDlg->Show(false);
				}
			}
			break;
		}
	}
}

void CDnPlayerStorageDlg::MoveItemToStorage(CDnQuickSlotButton* pFromSlot, bool bItemSplit)
{
	if (pFromSlot == NULL)
	{
		_ASSERT(0);
		return;
	}

	MIInventoryItem* pInvenItem = pFromSlot->GetItem();
	if (pInvenItem == NULL || pInvenItem->GetType() != MIInventoryItem::Item)
	{
		_ASSERT(0);
		return;
	}

	CDnItem* pItem = static_cast<CDnItem *>(pInvenItem);
	if (pItem == NULL)
		return;

	if (pFromSlot->GetRenderCount() > 1 && bItemSplit)
	{
		m_pSplitConfirmExDlg->SetItem(pFromSlot, CDnInterface::STORAGE_FROM_INVENTORY);
		m_pSplitConfirmExDlg->Show(true);
		return;
	}

	RequestMoveItem(MoveType_InvenToWare, pItem, pItem->GetOverlapCount());
}

void CDnPlayerStorageDlg::RequestMoveItem(eItemMoveType moveType, CDnItem* pItem, int itemCount)
{
	DWORD emptySlotType = -1;

	if (moveType == MoveType_InvenToWare)		
	{
		// Interacting with this dialog itself means the tab is already selected.
		// Player and guild storage merged.
		emptySlotType = CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG;
		
	}
	else if (moveType == MoveType_WareToInven)	emptySlotType = CDnMainMenuDlg::INVENTORY_DIALOG;
	else
	{
		_ASSERT(0);
		return;
	}

	int destIdx = -1;

	// When moving from inventory to storage, find an appropriate slot for stackable items (MoveType_WareToInven seems unused here).
	if (moveType == MoveType_InvenToWare)
	{
		// Find an appropriate slot in the inventory window.
		// Since it does not check whether the current slot is closed,
		// If it ever becomes UnUsable while containing items in the future, this routine will also need to be modified (currently inventory expansion is not time-limited, so this cannot happen).
		int nInsertableSlotIndex = -1;
		std::vector<CDnItem*> vecItem;
		GetItemTask().GetStorageInventory().ScanItemFromID( pItem->GetClassID(), &vecItem );
		for( int i = 0; i < (int)vecItem.size(); ++i )
		{
			if ( ( vecItem[i]->IsSoulbBound() == pItem->IsSoulbBound() && vecItem[i]->GetSealCount() == pItem->GetSealCount() ) 
				&& (( vecItem[i]->GetOverlapCount() + itemCount ) <= pItem->GetMaxOverlapCount()) )
			{
				nInsertableSlotIndex = vecItem[i]->GetSlotIndex();
				break;
			}
		}

		if (nInsertableSlotIndex == -1)
		{
			if (!GetInterface().IsEmptySlot(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG) )
			{
				GetInterface().MessageBox(1609, MB_OK);
				return;
			}
			nInsertableSlotIndex = GetInterface().GetEmptySlot(emptySlotType);
		}
		destIdx = nInsertableSlotIndex;
	}
	else if (moveType == MoveType_WareToInven)
	{
		destIdx = GetInterface().GetEmptySlot(emptySlotType);
	}

	CDnItemTask::GetInstance().RequestMoveItem(moveType,
		pItem->GetSlotIndex(),
		pItem->GetSerialID(),
		destIdx,
		itemCount);
}

void CDnPlayerStorageDlg::SetRemoteItemID( int iRemoteItemID )
{
	m_iRemoteItemID = iRemoteItemID;
	if( 0 < m_iRemoteItemID )
	{
		GetInterface().GetMainMenuDialog()->SetIgnoreShowFunc( true );
		GetInterface().OpenBlind();
		CDnLocalPlayerActor::StopAllPartyPlayer();
		CDnLocalPlayerActor::LockInput( true );
	}
}

#if defined( PRE_PERIOD_INVENTORY )
void CDnPlayerStorageDlg::EnablePeriodStorage( const bool bEnable, const __time64_t tTime )
{
	m_pStoreCommonDlg->EnablePeriodStorage( bEnable, tTime );
}
#endif	//	#if defined( PRE_PERIOD_INVENTORY )