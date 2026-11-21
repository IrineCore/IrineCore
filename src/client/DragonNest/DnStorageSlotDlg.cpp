#include "StdAfx.h"
#include "DnStorageSlotDlg.h"
#include "DnItemTask.h"
#include "DnInterfaceDlgID.h"
#include "DnInterface.h"
#include "DnMessageBox.h"
#include "DnInvenTabDlg.h"
#include "DnMainMenuDlg.h"
#ifdef PRE_ADD_INVEN_EXTENSION
#include "DnStorageExtensionDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnStorageSlotDlg::CDnStorageSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pDragButton(NULL)
	, m_pPressedButton(NULL)
#ifdef PRE_ADD_INVEN_EXTENSION
	, m_pStorageExtensionDlg(NULL)
	, m_nUseItemCnt(0)
#endif
{
}

CDnStorageSlotDlg::~CDnStorageSlotDlg(void)
{
#ifdef PRE_ADD_INVEN_EXTENSION
	SAFE_DELETE(m_pStorageExtensionDlg);
#endif
}

void CDnStorageSlotDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StorageSlotDlg.ui" ).c_str(), bShow );
}

#ifdef PRE_ADD_INVEN_EXTENSION
void CDnStorageSlotDlg::InitialUpdate()
{
	m_pStorageExtensionDlg = new CDnStorageExtensionDlg( UI_TYPE_CHILD, this, STORAGE_EXTENSION_DIALOG, NULL );
	m_pStorageExtensionDlg->Initialize( false );
}
#endif

void CDnStorageSlotDlg::InitCustomControl( CEtUIControl *pControl )
{
	CDnItemSlotButton *pItemSlotButton(NULL);
	pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);

	pItemSlotButton->SetSlotType( ST_STORAGE_PLAYER );
	pItemSlotButton->SetSlotIndex( GetSlotStartIndex() + (int)m_vecSlotButton.size() );
	m_vecSlotButton.push_back( pItemSlotButton );
}

#ifdef PRE_ADD_INVEN_EXTENSION
void CDnStorageSlotDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
	CheckShowExtensionDlg( bShow );
}

void CDnStorageSlotDlg::CheckShowExtensionDlg( bool bShow )
{
#ifdef PRE_ADD_INSTANT_CASH_BUY
	bool bShowDlg = ((m_nUseItemCnt < ITEM_MAX) && bShow);
#else // PRE_ADD_INSTANT_CASH_BUY
	bool bShowDlg = ((m_nUseItemCnt < ITEM_MAX) && (CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage) && bShow);
#endif // PRE_ADD_INSTANT_CASH_BUY
	ShowChildDialog( m_pStorageExtensionDlg, bShowDlg );
}
#endif

void CDnStorageSlotDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_ITEM" ) )
		{
			if( GetItemTask().IsRequestWait() )
				return;

			m_pDragButton = (CDnItemSlotButton*)drag::GetControl();
			m_pPressedButton = (CDnItemSlotButton*)pControl;

			if( m_pPressedButton->GetItemState() == ITEMSLOT_DISABLE ) 
				return;

			if( m_pDragButton == NULL )
			{
				CDnItem *pItem = static_cast<CDnItem *>(m_pPressedButton->GetItem());
				if( !pItem ) return;

				if( uMsg == WM_RBUTTONUP )
				{
					// Find an appropriate slot in the inventory window.
					// Since it does not check whether the current slot is closed,
					// If it ever becomes UnUsable while containing items in the future, this routine will also need to be modified (currently inventory expansion is not time-limited, so this cannot happen).
					int nInsertableSlotIndex = -1;
					std::vector<CDnItem*> vecItem;
					GetItemTask().GetCharInventory().ScanItemFromID( pItem->GetClassID(), &vecItem );
					for( int i = 0; i < (int)vecItem.size(); ++i )
					{
						if( ( vecItem[i]->IsSoulbBound() == pItem->IsSoulbBound() && vecItem[i]->GetSealCount() == pItem->GetSealCount() )  
							&& (vecItem[i]->GetOverlapCount() + pItem->GetOverlapCount() <= pItem->GetMaxOverlapCount()) )
						{
							nInsertableSlotIndex = vecItem[i]->GetSlotIndex();
							break;
						}
					}

					if( nInsertableSlotIndex == -1 )
					{
						if( !GetInterface().IsEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG ) )
						{
							GetInterface().MessageBox( 1925, MB_OK );
							return;
						}
						nInsertableSlotIndex = GetInterface().GetEmptySlot( CDnMainMenuDlg::INVENTORY_DIALOG );
					}

					GetItemTask().RequestMoveItem(	MoveType_WareToInven, 
													m_pPressedButton->GetSlotIndex(), 
													pItem->GetSerialID(),
													nInsertableSlotIndex,
													pItem->GetOverlapCount() );
					return;
				}

				CDnInventoryDlg::SetSelectAmount( pItem->GetOverlapCount() );

				m_pPressedButton->EnableSplitMode(pItem->GetOverlapCount());
				drag::SetControl(m_pPressedButton);

				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );
			}
			else
			{
				CDnItem *pItem = static_cast<CDnItem *>(m_pDragButton->GetItem());
				if( !pItem ) return;

				if( pItem->GetItemType() == ITEMTYPE_QUEST || pItem->IsCashItem() )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3609 ), MB_OK ); // UISTRING : This item cannot be moved.

					//m_pDragButton->DisableSplitMode(true);
					drag::Command(UI_DRAG_CMD_CANCEL);
					drag::ReleaseControl();
					return;
				}

				if(  m_pDragButton != m_pPressedButton )
				{
					RequestMoveItem( m_pDragButton, m_pPressedButton );
				}
				else
				{
					// Sound handling when moving in place.
					CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );
					drag::Command(UI_DRAG_CMD_CANCEL);
					drag::ReleaseControl();
				}
			}

			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnStorageSlotDlg::SetItem( MIInventoryItem *pItem )
{
	int nSlotIndex = pItem->GetSlotIndex()-GetSlotStartIndex();
	ASSERT( nSlotIndex>=0&&"CDnStoreSlotDlg::SetItem" );
	ASSERT( nSlotIndex<(int)m_vecSlotButton.size()&&"CDnStoreSlotDlg::SetItem" );
	m_vecSlotButton[nSlotIndex]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
}

void CDnStorageSlotDlg::ResetSlot( int nSlotIndex )
{
	nSlotIndex -= GetSlotStartIndex();
	ASSERT( nSlotIndex>=0&&"CDnStoreSlotDlg::ResetSlot" );
	ASSERT( nSlotIndex<(int)m_vecSlotButton.size()&&"CDnStoreSlotDlg::ResetSlot" );
	m_vecSlotButton[ nSlotIndex ]->ResetSlot();
	m_vecSlotButton[ nSlotIndex ]->OnRefreshTooltip();
}

void CDnStorageSlotDlg::SetUseItemCnt( DWORD dwItemCnt )
{
	ASSERT( dwItemCnt <= ITEM_MAX );
	if( dwItemCnt > ITEM_MAX )
		return;

	CDnItemSlotButton *pButton(NULL);
	DWORD dwVecSize = (DWORD)m_vecSlotButton.size();
	DWORD i(0);

	for( ; i<dwItemCnt; i++ )
	{
		pButton = m_vecSlotButton[i];
		if( pButton )
		{
			pButton->SetItemState( ITEMSLOT_ENABLE );
		}
	}

	for( ; i<dwVecSize; i++ )
	{
		pButton = m_vecSlotButton[i];
		if( pButton )
		{
			pButton->SetItemState( ITEMSLOT_DISABLE );
		}
	}

#ifdef PRE_ADD_INVEN_EXTENSION
	m_nUseItemCnt = dwItemCnt;
#endif
}

int CDnStorageSlotDlg::GetSlotStartIndex()
{
	return GetDialogID() * ITEM_MAX;
}

void CDnStorageSlotDlg::RequestMoveItem( CDnSlotButton *pDragButton, CDnItemSlotButton *pPressedButton )
{
	ASSERT(pDragButton&&pPressedButton);

	switch( pDragButton->GetSlotType() )
	{
	case ST_STORAGE_PLAYER:
		{
			CDnItem *pItem = (CDnItem*)pDragButton->GetItem();
			GetItemTask().RequestMoveItem( MoveType_Ware, pDragButton->GetSlotIndex(), pItem->GetSerialID(), pPressedButton->GetSlotIndex(), CDnInventoryDlg::GetCurrentAmount() );
		}
		break;
	case ST_INVENTORY:
		{
			CDnItem *pItem = static_cast<CDnItem *>(m_pDragButton->GetItem());
			if( pItem )
			{
				GetItemTask().RequestMoveItem( MoveType_InvenToWare, m_pDragButton->GetSlotIndex(), pItem->GetSerialID(), m_pPressedButton->GetSlotIndex(), pItem->GetOverlapCount() );
			}
		}
		break;
	}
}

bool CDnStorageSlotDlg::IsEmptySlot()
{
	if( GetEmptySlot() == -1 )
	{
		return false;
	}

	return true;
}

int CDnStorageSlotDlg::GetEmptySlot()
{
	CDnItemSlotButton *pButton(NULL);

	DWORD dwVecSize = (DWORD)m_vecSlotButton.size();
	for( DWORD i=0; i<dwVecSize; i++ )
	{
		pButton = m_vecSlotButton[i];
		if( !pButton ) continue;

		if( (pButton->GetItemState() == ITEMSLOT_ENABLE) && pButton->IsEmptySlot() )
		{
			return pButton->GetSlotIndex();
		}
	}

	return -1;
}