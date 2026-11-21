#include "stdafx.h"
#include "DnLifeStorageDlg.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInventoryDlg.h"
#include "DnLifeSkillPlantTask.h"
#include "SyncTimer.h"


const float BUTTON_REFRESH_TIME = 2.0f;
const int	ONE_DAY_TIME = 86400;

CDnLifeStorageDlg::CDnLifeStorageDlg(UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pDragButton( NULL )
, m_pPressedButton( NULL )
, m_pButtonPrev( NULL )
, m_pButtonNext( NULL )
, m_iSelectPage(-1)
, m_bNextPage( true )
{
	
}

CDnLifeStorageDlg::~CDnLifeStorageDlg()
{
	ResetAllStorage();
}

void CDnLifeStorageDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "LifeStorageDlg.ui" ).c_str(), bShow );
}

void CDnLifeStorageDlg::InitCustomControl(CEtUIControl *pControl)
{
	CDnItemSlotButton *pItemSlotButton(NULL);
	pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);

	pItemSlotButton->SetSlotType( ST_STORAGE_FARM );
	pItemSlotButton->SetSlotIndex( (int)m_vecSlotButton.size() );
	m_vecSlotButton.push_back( pItemSlotButton );
}

void CDnLifeStorageDlg::InitialUpdate()
{
	m_pButtonPrev = GetControl<CEtUIButton>("ID_BT_PRIOR");
	m_pButtonNext = GetControl<CEtUIButton>("ID_BT_NEXT");
}

void CDnLifeStorageDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CDnCustomDlg::Show( bShow );

	if( !bShow )
	{
		ResetAllStorage();

		if( drag::IsValid() )
		{
			CDnSlotButton *pDragButton = static_cast<CDnSlotButton*>(drag::GetControl());
			if( pDragButton->GetSlotType() == ST_STORAGE_PLAYER )
			{
				drag::Command(UI_DRAG_CMD_CANCEL);
				drag::ReleaseControl();
			}
		}

		GetInterface().CloseBlind();
	}
	else
	{
		GetLifeSkillPlantTask().RequestWareHouseList();
		UpdateSelectPage();
	}
}

void CDnLifeStorageDlg::Process( float fElapsedTime )
{
	/*
	if( m_bButtonNext )
	{
		m_fNextDelta -= fElapsedTime;
		if( m_fNextDelta < 0.f )
		{
			CONTROL( Button, ID_BT_NEXT )->Enable( true );
			m_bButtonNext = false;
		}
	}
	if( m_bButtonPrev )
	{
		m_fPrevDelta -= fElapsedTime;
		if( m_fPrevDelta < 0.f )
		{
			CONTROL( Button, ID_BT_PRIOR )->Enable( true );
			m_bButtonPrev = false;
		}
	}
	*/

	CEtUIDialog::Process( fElapsedTime );
}

void CDnLifeStorageDlg::ProcessCommand(UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_BT_PRIOR" ) )
		{
			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			if( pDragButton )
			{
				pDragButton->DisableSplitMode(false);
				drag::ReleaseControl();
			}

			if( 0 >= m_iSelectPage )
				return;
			ChangePage(false);

			/*
			m_bButtonPrev = true;
			m_fPrevDelta = BUTTON_REFRESH_TIME;
			CONTROL( Button, ID_BT_PRIOR )->Enable( false );
			*/
			m_pButtonPrev->Enable( false );
			m_pButtonPrev->SetDisableTime( BUTTON_REFRESH_TIME );
			m_pButtonPrev->Enable( true );
		}
		else if( strstr( pControl->GetControlName(), "ID_BT_NEXT" ) )
		{
			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			if( pDragButton )
			{
				pDragButton->DisableSplitMode(false);
				drag::ReleaseControl();
			}

			if( m_bNextPage && (m_iSelectPage + 1) == (BYTE)m_vItemsPerPage.size() )
				GetLifeSkillPlantTask().RequestWareHouseList();
			else
			{
				if( (m_iSelectPage + 1) != (BYTE)m_vItemsPerPage.size() )
					ChangePage(true);
			}
			/*
			m_bButtonNext = true;
			m_fNextDelta = BUTTON_REFRESH_TIME;
			CONTROL( Button, ID_BT_NEXT )->Enable( false );
			*/
			m_pButtonNext->Enable( false );
			m_pButtonNext->SetDisableTime( BUTTON_REFRESH_TIME );
			m_pButtonNext->Enable( true );
		}
		else if( strstr( pControl->GetControlName(), "ID_ITEM" ) )
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
					int nIndex= m_pPressedButton->GetSlotIndex();
					RequestTakeWareHouseItem( nIndex );
					return;
				}
				drag::SetControl(m_pPressedButton);

				CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );
			}
			else
			{
				m_pDragButton->DisableSplitMode(true);
				drag::ReleaseControl();
			}

			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnLifeStorageDlg::ResetSlot()
{
	for( int itr = 0 ; itr < (int)m_vecSlotButton.size(); ++itr )
		m_vecSlotButton[itr]->ResetSlot();
}

void CDnLifeStorageDlg::ResetAllStorage()
{
	for( int itr = 0 ; itr < (int)m_vItemsPerPage.size(); ++itr )
	{
		for( STORAGE_MAP_ITOR itor = m_vItemsPerPage[itr].begin(); itor != m_vItemsPerPage[itr].end(); ++itor )
			SAFE_DELETE( itor->second );
	}
	
	m_vItemsPerPage.clear();
	m_iSelectPage = -1;
	m_bNextPage = true;
}

void CDnLifeStorageDlg::RefreshLifeStorage(SCFarmWareHouseList * pData)
{
	if( !IsShow() )	//Ignore packets that arrive late after closing.
		return;

	if( Farm::Max::HARVESTDEPOT_COUNT > pData->cCount )
	{
		m_bNextPage = false;

		if( 0 == pData->cCount )
			return;
	}

	ResetSlot();

	STORAGE_MAP mStorage;

	for( BYTE itr = 0; itr < pData->cCount; ++itr )
	{
		SStorageSlotInfo * sItemInfo = new SStorageSlotInfo;

		sItemInfo->m_pItem = GetItemTask().CreateItem( pData->Items[itr] );

		__time64_t tRemainTime =  CSyncTimer::GetInstance().GetCurTime() - pData->Items[itr].tMaterializeDate;

		sItemInfo->m_nExpireTime = Farm::Max::WAREHOUSE_DEFAULT_KEEPING_DAY - (int)(tRemainTime / ONE_DAY_TIME);

		mStorage.insert( make_pair(pData->Items[itr].biUniqueID, sItemInfo) );

		m_vecSlotButton[itr]->SetItem( sItemInfo->m_pItem, 0 );
		m_vecSlotButton[itr]->SetExpire( sItemInfo->m_nExpireTime );
	}

	m_vItemsPerPage.push_back( mStorage );

	++m_iSelectPage;
	UpdateSelectPage();
}

void CDnLifeStorageDlg::ChangePage( bool bNext )
{
	ResetSlot();

	if( bNext )
		++m_iSelectPage;
	else 
		--m_iSelectPage;

	BYTE cCount = 0;
	CDnItem * pItem;
	for( STORAGE_MAP_ITOR itor = m_vItemsPerPage[m_iSelectPage].begin(); itor != m_vItemsPerPage[m_iSelectPage].end(); ++itor )
	{
		pItem = itor->second->m_pItem;
		if( pItem )
		{
			m_vecSlotButton[cCount]->SetItem( pItem, 0 );
			m_vecSlotButton[cCount]->SetExpire( itor->second->m_nExpireTime );
		}
		++cCount;
	}
	UpdateSelectPage();
}

void CDnLifeStorageDlg::UpdateSelectPage()
{
	WCHAR szString[256];

	int nCurPage = m_iSelectPage + 1;
	if( nCurPage < 1 ) nCurPage = 1;
	int nMaxPage = ( ( GetLifeSkillPlantTask().GetWareHouseItemCount() - 1 ) / Farm::Max::HARVESTDEPOT_COUNT ) + 1;
	if( nMaxPage < 1 ) nMaxPage = 1;

	wsprintf( szString, L"%d / %d", nCurPage, nMaxPage );
	CONTROL( Static, ID_BT_PAGE )->SetText( szString );

	m_pButtonPrev->Enable( false );
	m_pButtonNext->Enable( false );

	if( nCurPage > 1 ) m_pButtonPrev->Enable( true );
	if( nCurPage < nMaxPage ) m_pButtonNext->Enable( true );

}

void CDnLifeStorageDlg::RequestTakeWareHouseItem( int nSlotIndex )
{
	STORAGE_MAP_ITOR itor = m_vItemsPerPage[m_iSelectPage].begin();
	for( int itr = 0; itr < nSlotIndex; ++itr )
		++itor;

	if( itor == m_vItemsPerPage[m_iSelectPage].end() )
		return;

	GetLifeSkillPlantTask().RequestTakeWareHouseItem( itor->first );
}

void CDnLifeStorageDlg::OnRecvTakeWareHouseItem(INT64 biUniqueID)
{
	if( -1 == m_iSelectPage )
		return;

	BYTE cCount = 0;
	CDnItem * pItem;
	for( STORAGE_MAP_ITOR itor = m_vItemsPerPage[m_iSelectPage].begin(); itor != m_vItemsPerPage[m_iSelectPage].end(); ++itor )
	{
		pItem = itor->second->m_pItem;
		if( pItem && biUniqueID == (*itor).first )
		{
			SAFE_DELETE( pItem );
			(*itor).second->m_pItem = NULL;
			m_vecSlotButton[cCount]->ResetSlot();
		}
		++cCount;
	}
}
