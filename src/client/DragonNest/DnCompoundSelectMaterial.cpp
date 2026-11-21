#include "StdAfx.h"

#if defined(PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL)

#include "DnCompoundSelectMaterial.h"
#include "DnItemTask.h"
#include "DnItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCompoundSelectMaterial::CDnCompoundSelectMaterial( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pStaticTitle( NULL )
, m_pStaticPage( NULL )
, m_pStaticBg( NULL )
, m_pTargetSlot( NULL )
, m_pBtnOK( NULL )
, m_pBtnPrior( NULL )
, m_pBtnNext( NULL )
, m_pTargetItem( NULL )
, m_nSelectedIndex( -1 )
, m_nMaxPage( 1 )
, m_nCurPage( 1 )
, m_nSelectMode( MODE_NORMAL )
{
	memset( m_pMaterialSlots, 0, sizeof(m_pMaterialSlots) );
	memset( m_pSelectFrames, 0, sizeof(m_pSelectFrames) );
}

CDnCompoundSelectMaterial::~CDnCompoundSelectMaterial()
{
	SAFE_DELETE( m_pTargetItem );
	ClearMaterials();
}

void CDnCompoundSelectMaterial::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenItemCompoundDlg.ui" ).c_str(), bShow );
}

void CDnCompoundSelectMaterial::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
	m_pStaticPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );
	m_pStaticBg = GetControl<CEtUIStatic>( "ID_STATIC6" );

	m_pTargetSlot = GetControl<CDnItemSlotButton>( "ID_ITEM0" );
	if( m_pTargetSlot )
		m_pTargetSlot->SetSlotType( ST_ITEM_COMPOUND );

	char szBuf[32] = {0,};
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		sprintf_s( szBuf, _countof(szBuf), "ID_ITEM%d", i + 1 );
		m_pMaterialSlots[i] = GetControl<CDnItemSlotButton>( szBuf );
		if( m_pMaterialSlots[i] )
		{
			m_pMaterialSlots[i]->SetSlotType( ST_ITEM_COMPOUND_JEWEL );
			m_pMaterialSlots[i]->SetSlotIndex( i );
		}

		sprintf_s( szBuf, _countof(szBuf), "ID_SELECT%d", i + 1 );
		m_pSelectFrames[i] = GetControl<CEtUIStatic>( szBuf );
		if( m_pSelectFrames[i] )
			m_pSelectFrames[i]->Show( false );
	}

	m_pBtnOK = GetControl<CEtUIButton>( "ID_BT_OK" );
	if( m_pBtnOK )
	{
		m_pBtnOK->Enable( false );
		m_SmartMove.SetControl( m_pBtnOK );
	}
	m_pBtnPrior = GetControl<CEtUIButton>( "ID_BT_PRIOR" );
	m_pBtnNext = GetControl<CEtUIButton>( "ID_BT_NEXT" );
}

void CDnCompoundSelectMaterial::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		UpdatePageText();
		UpdateTitle();
		RefreshMaterialSlots();
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
		m_nCurPage = 1;
		m_nMaxPage = 1;
		SAFE_DELETE( m_pTargetItem );
		if( m_pTargetSlot ) m_pTargetSlot->ResetSlot();
		ClearMaterials();
		for( int i = 0; i < SLOT_PER_PAGE; ++i )
		{
			if( m_pSelectFrames[i] ) m_pSelectFrames[i]->Show( false );
		}
		m_nSelectedIndex = -1;
		m_nSelectMode = MODE_NORMAL;
		if( m_pBtnOK ) m_pBtnOK->Enable( false );
	}

	CDnCustomDlg::Show( bShow );
}

void CDnCompoundSelectMaterial::ClearMaterials()
{
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( m_pMaterialSlots[i] )
			m_pMaterialSlots[i]->ResetSlot();
	}
	SAFE_DELETE_PVEC( m_vecMaterialItems );
}

void CDnCompoundSelectMaterial::UpdateTitle()
{
	if( !m_pStaticTitle ) return;

	int nStringID = 0x83a;
	if( m_nSelectMode == MODE_SUFFIX ) nStringID = 0x83c;
	else if( m_nSelectMode == MODE_UPGRADE ) nStringID = 0x83d;

	m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringID ) );
}

void CDnCompoundSelectMaterial::UpdatePageText()
{
	if( !m_pStaticPage ) return;
	wchar_t wszPage[32] = {0,};
	swprintf_s( wszPage, _countof(wszPage), L"%d/%d", m_nCurPage, max( 1, m_nMaxPage ) );
	m_pStaticPage->SetText( wszPage );
}

void CDnCompoundSelectMaterial::SetTargetItem( CDnItem *pItem )
{
	SAFE_DELETE( m_pTargetItem );
	if( m_pTargetSlot ) m_pTargetSlot->ResetSlot();
	if( !pItem || !CDnItemTask::IsActive() ) return;

	TItemInfo itemInfo;
	pItem->GetTItemInfo( itemInfo );
	m_pTargetItem = GetItemTask().CreateItem( itemInfo );
	if( m_pTargetItem && m_pTargetSlot )
	{
		m_pTargetItem->SetSlotIndex( itemInfo.cSlotIndex );
		m_pTargetSlot->SetItem( m_pTargetItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
	}
}

void CDnCompoundSelectMaterial::AddMaterialItem( CDnItem *pItem )
{
	if( !pItem || !CDnItemTask::IsActive() ) return;

	TItemInfo itemInfo;
	pItem->GetTItemInfo( itemInfo );
	CDnItem *pCopy = GetItemTask().CreateItem( itemInfo );
	if( pCopy )
	{
		pCopy->SetSlotIndex( itemInfo.cSlotIndex );
		m_vecMaterialItems.push_back( pCopy );
		m_nMaxPage = max( 1, ( (int)m_vecMaterialItems.size() + SLOT_PER_PAGE - 1 ) / SLOT_PER_PAGE );
	}
}

void CDnCompoundSelectMaterial::RefreshMaterialSlots()
{
	int nStart = ( m_nCurPage - 1 ) * SLOT_PER_PAGE;
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( !m_pMaterialSlots[i] ) continue;
		m_pMaterialSlots[i]->ResetSlot();

		int nIdx = nStart + i;
		if( nIdx >= 0 && nIdx < (int)m_vecMaterialItems.size() && m_vecMaterialItems[nIdx] )
		{
			m_pMaterialSlots[i]->SetItem( m_vecMaterialItems[nIdx], CDnSlotButton::ITEM_ORIGINAL_COUNT );
		}
	}

	if( m_vecMaterialItems.size() == 1 && m_pMaterialSlots[0] && !m_pMaterialSlots[0]->IsEmptySlot() )
	{
		m_nSelectedIndex = 0;
		if( m_pBtnOK ) m_pBtnOK->Enable( true );
	}

	UpdateSelectFrame();
}

void CDnCompoundSelectMaterial::UpdateSelectFrame()
{
	for( int i = 0; i < SLOT_PER_PAGE; ++i )
	{
		if( m_pSelectFrames[i] )
			m_pSelectFrames[i]->Show( false );
	}

	int nLocal = m_nSelectedIndex - ( m_nCurPage - 1 ) * SLOT_PER_PAGE;
	if( nLocal >= 0 && nLocal < SLOT_PER_PAGE && m_pMaterialSlots[nLocal] && !m_pMaterialSlots[nLocal]->IsEmptySlot() )
	{
		if( m_pSelectFrames[nLocal] )
			m_pSelectFrames[nLocal]->Show( true );
	}
}

bool CDnCompoundSelectMaterial::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bRet = CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );

	if( uMsg == WM_MOUSEWHEEL && m_pStaticBg )
	{
		SUICoord bgCoord;
		m_pStaticBg->GetUICoord( bgCoord );
		float fMouseX = 0.0f, fMouseY = 0.0f;
		GetScreenMouseMovePoints( fMouseX, fMouseY );
		if( bgCoord.IsInside( fMouseX - GetXCoord(), fMouseY - GetYCoord() ) )
		{
			int nDelta = GET_WHEEL_DELTA_WPARAM( wParam );
			if( nDelta > 0 && m_pBtnPrior )
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pBtnPrior, 0 );
			else if( nDelta < 0 && m_pBtnNext )
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pBtnNext, 0 );
			return true;
		}
	}

	return bRet;
}

void CDnCompoundSelectMaterial::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_PRIOR" ) )
		{
			if( m_nCurPage > 1 )
			{
				--m_nCurPage;
				UpdatePageText();
				RefreshMaterialSlots();
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			if( m_nCurPage < m_nMaxPage )
			{
				++m_nCurPage;
				UpdatePageText();
				RefreshMaterialSlots();
			}
			return;
		}
		else if( strstr( pControl->GetControlName(), "ID_ITEM" ) && !IsCmdControl( "ID_ITEM0" ) )
		{
			int nSlotNum = 0;
			if( sscanf_s( pControl->GetControlName(), "ID_ITEM%d", &nSlotNum ) == 1 && nSlotNum >= 1 && nSlotNum <= SLOT_PER_PAGE )
			{
				int nLocal = nSlotNum - 1;
				if( m_pMaterialSlots[nLocal] && !m_pMaterialSlots[nLocal]->IsEmptySlot() )
				{
					m_nSelectedIndex = ( m_nCurPage - 1 ) * SLOT_PER_PAGE + nLocal;
					UpdateSelectFrame();
					if( m_pBtnOK ) m_pBtnOK->Enable( true );
				}
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

CDnItem *CDnCompoundSelectMaterial::GetSelectedItem() const
{
	if( m_nSelectedIndex < 0 || m_nSelectedIndex >= (int)m_vecMaterialItems.size() )
		return NULL;
	return m_vecMaterialItems[m_nSelectedIndex];
}

INT64 CDnCompoundSelectMaterial::GetSelectedSerialID() const
{
	CDnItem *pItem = GetSelectedItem();
	return pItem ? pItem->GetSerialID() : 0;
}

#endif // PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
