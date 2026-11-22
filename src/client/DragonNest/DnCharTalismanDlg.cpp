#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnCharTalismanDlg.h"
#include "DnCharTalismanInfoDlg.h"
#include "DnCharTalismanListDlg.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "DnMainMenuDlg.h"
#include "EtSoundEngine.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#endif
#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#include "DnCashShopTask.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCharTalismanDlg::CDnCharTalismanDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pCharTalismanInfoDlg( NULL )
, m_pCharTalismanListDlg( NULL )
, m_pBtnOK( NULL )
, m_pBtnInfo( NULL )
, m_pBtnCompose( NULL )
, m_pListBoxTalisman( NULL )
, m_pScrollBar( NULL )
, m_pTextOption( NULL )
, m_pBtnBuyShortcut( NULL )
, m_pStaticBuyShortcut( NULL )
, m_nSlotCount( 0 )
, m_nScrollMax( 40 )
, m_nEquipSoundIndex( -1 )
{
}

CDnCharTalismanDlg::~CDnCharTalismanDlg()
{
	if( m_nEquipSoundIndex != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nEquipSoundIndex );
	SAFE_DELETE( m_pCharTalismanInfoDlg );
	SAFE_DELETE( m_pCharTalismanListDlg );
}

void CDnCharTalismanDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharTalismanDlg.ui" ).c_str(), bShow );
}

void CDnCharTalismanDlg::InitialUpdate()
{
	m_pCharTalismanListDlg = new CDnCharTalismanListDlg( UI_TYPE_SELF, NULL );
	m_pCharTalismanListDlg->SetParentTalismanDlg( this );
	m_pCharTalismanListDlg->Initialize( false );

	m_pCharTalismanInfoDlg = new CDnCharTalismanInfoDlg( UI_TYPE_CHILD, this );
	m_pCharTalismanInfoDlg->Initialize( false );

	m_pBtnOK = GetControl<CEtUIButton>( "ID_BT_OK" );
	m_pBtnInfo = GetControl<CEtUIButton>( "ID_BT_INFO" );
	m_pBtnCompose = GetControl<CEtUIButton>( "ID_BT_COMPOSE" );
	m_pListBoxTalisman = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_TALISMAN" );
	m_pTextOption = GetControl<CEtUIStatic>( "ID_TEXT_OPTION" );

	if( m_pTextOption )
	{
		m_pTextOption->SetTextColor( 0xffffd700 );
		m_pTextOption->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8368 ) );
	}

	if( m_pListBoxTalisman )
	{
		m_pScrollBar = m_pListBoxTalisman->GetScrollBar();
		if( m_pScrollBar )
		{
			m_pScrollBar->SetPageSize( 25 );
			m_pScrollBar->SetTrackRange( 0, m_nScrollMax );
		}
	}

	CEtUIButton *pHelpBtn = GetControl<CEtUIButton>( "ID_BT_SMALLHELP" );
	if( pHelpBtn )
		pHelpBtn->Enable( true );

	m_pStaticBuyShortcut = GetControl<CEtUIStatic>( "ID_STATIC_BUYSHORTCUT" );
	if( m_pStaticBuyShortcut )
		m_pStaticBuyShortcut->Show( false );

	m_pBtnBuyShortcut = GetControl<CEtUIButton>( "UI_BT_BUYSHORTCUT" );
	if( m_pBtnBuyShortcut )
		m_pBtnBuyShortcut->Show( false );

	if( m_pBtnCompose )
		m_pBtnCompose->Enable( false );

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( pSox )
		m_nSlotCount = pSox->GetItemCount();
}

void CDnCharTalismanDlg::UpdateListPosition()
{
	if( !m_pCharTalismanListDlg || !m_pListBoxTalisman )
		return;

	SUICoord listCoord;
	m_pListBoxTalisman->GetUICoord( listCoord );

	float fContentHeight = m_pCharTalismanListDlg->GetContentHeight();
	float fOverflow = fContentHeight - listCoord.fHeight;
	float fScrollOffset = 0.0f;

	if( m_pScrollBar )
	{
		if( fOverflow > 0.0f && m_nScrollMax > 0 )
		{
			int nPageSize = (int)( m_nScrollMax * ( listCoord.fHeight / fContentHeight ) );
			if( nPageSize < 1 ) nPageSize = 1;
			if( nPageSize >= m_nScrollMax ) nPageSize = m_nScrollMax - 1;

			m_pScrollBar->Enable( true );
			m_pScrollBar->SetTrackRange( 0, m_nScrollMax );
			m_pScrollBar->SetPageSize( nPageSize );

			int nMaxPos = m_nScrollMax - nPageSize;
			if( nMaxPos > 0 )
				fScrollOffset = fOverflow * ( (float)m_pScrollBar->GetTrackPos() / (float)nMaxPos );
		}
		else
		{
			m_pScrollBar->SetTrackRange( 0, 1 );
			m_pScrollBar->SetPageSize( 1 );
			m_pScrollBar->SetTrackPos( 0 );
			m_pScrollBar->Enable( false );
		}
	}

	m_pCharTalismanListDlg->SetPosition(
		GetXCoord() + listCoord.fX,
		GetYCoord() + listCoord.fY + CDnCharTalismanListDlg::TALISMAN_LIST_TOP_PADDING - fScrollOffset );
}

void CDnCharTalismanDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CDnCustomDlg::Show( bShow );

	if( m_pCharTalismanListDlg )
	{
		m_pCharTalismanListDlg->Show( bShow );
		UpdateListPosition();
		if( bShow )
		{
			m_pCharTalismanListDlg->SetEfficiencyTextShow( true );
			RefreshTalisman();
		}
		else
		{
			m_pCharTalismanListDlg->HideTooltip();
			if( m_pCharTalismanInfoDlg )
				m_pCharTalismanInfoDlg->Show( false );
		}
	}
}

void CDnCharTalismanDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( IsShow() && m_pCharTalismanListDlg )
	{
		UpdateListPosition();
		m_pCharTalismanListDlg->Process( fElapsedTime );
	}
}

void CDnCharTalismanDlg::Render( float fElapsedTime )
{
	CDnCustomDlg::Render( fElapsedTime );

	if( !IsShow() || !m_pCharTalismanListDlg || !m_pListBoxTalisman )
		return;

	UpdateListPosition();

	RECT OriginalRect;
	RECT ScissorRect;
	GetEtDevice()->GetScissorRect( &OriginalRect );

	SUICoord listCoord;
	m_pListBoxTalisman->GetUICoord( listCoord );

	int nDevW = GetEtDevice()->Width();
	int nDevH = GetEtDevice()->Height();
	ScissorRect.left   = int( ( listCoord.fX + GetXCoord() ) / GetScreenWidthRatio() * nDevW );
	ScissorRect.top    = int( ( listCoord.fY + GetYCoord() ) / GetScreenHeightRatio() * nDevH );
	ScissorRect.right  = ScissorRect.left + int( listCoord.fWidth / GetScreenWidthRatio() * nDevW );
	ScissorRect.bottom = ScissorRect.top  + int( listCoord.fHeight / GetScreenHeightRatio() * nDevH );

	if( ScissorRect.left < 0 ) ScissorRect.left = 0;
	if( ScissorRect.top < 0 ) ScissorRect.top = 0;
	if( ScissorRect.right > nDevW ) ScissorRect.right = nDevW;
	if( ScissorRect.bottom > nDevH ) ScissorRect.bottom = nDevH;
	if( ScissorRect.right <= ScissorRect.left || ScissorRect.bottom <= ScissorRect.top )
		return;

	CEtSprite::GetInstance().Flush();
	GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	GetEtDevice()->SetScissorRect( &ScissorRect );

	m_pCharTalismanListDlg->Render( fElapsedTime );

	CEtSprite::GetInstance().Flush();
	GetEtDevice()->SetScissorRect( &OriginalRect );
	GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
}

bool CDnCharTalismanDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_MOUSEWHEEL && m_pScrollBar && m_pScrollBar->IsEnable() && m_pListBoxTalisman )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		float fMouseX = 0.0f, fMouseY = 0.0f;
		PointToFloat( MousePoint, fMouseX, fMouseY );
		SUICoord listCoord;
		m_pListBoxTalisman->GetUICoord( listCoord );
		if( listCoord.IsInside( fMouseX, fMouseY ) )
		{
			int nDelta = GET_WHEEL_DELTA_WPARAM( wParam );
			if( nDelta != 0 )
			{
				m_pScrollBar->Scroll( ( nDelta > 0 ) ? -3 : 3 );
				UpdateListPosition();
				return true;
			}
		}
	}

	if( m_pCharTalismanListDlg )
	{
		UpdateListPosition();
		m_pCharTalismanListDlg->SetHWnd( hWnd );
		if( m_pCharTalismanListDlg->MsgProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharTalismanDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_INFO" ) )
		{
			if( m_pCharTalismanInfoDlg )
			{
				if( m_pCharTalismanInfoDlg->IsShow() )
				{
					m_pCharTalismanInfoDlg->Show( false );
				}
				else
				{
					m_pCharTalismanInfoDlg->SetText();
					m_pCharTalismanInfoDlg->Show( true );
				}
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			CDnCharStatusDlg *pCharStatusDlg = dynamic_cast<CDnCharStatusDlg*>( GetParentDialog() );
			if( pCharStatusDlg && pCharStatusDlg->GetCallBack() )
				pCharStatusDlg->GetCallBack()->OnUICallbackProc( pCharStatusDlg->GetDialogID(), EVENT_BUTTON_CLICKED, pControl );
			return;
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl( "ID_BT_SMALLHELP" ) )
		{
			CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
			if( pMainMenuDlg )
			{
				CDnSystemDlg *pSystemDlg = pMainMenuDlg->GetSystemDlg();
				if( pSystemDlg )
					pSystemDlg->ShowChoiceHelpDlg( HELP_SHORTCUT_CHAR_SET_PLATE );
			}
			return;
		}
#endif
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnCharTalismanDlg::EquipTalismanFromInven( int nInvenIndex, MIInventoryItem *pItem, CDnSlotButton *pSlotButton )
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->EquipTalismanFromInven( nInvenIndex, pItem, pSlotButton );
}

MIInventoryItem *CDnCharTalismanDlg::GetEquipTalisman( const CDnItem *pItem ) const
{
	if( !m_pCharTalismanListDlg ) return NULL;
	return m_pCharTalismanListDlg->GetEquipTalisman( pItem );
}

void CDnCharTalismanDlg::SetSlotMagneticMode( bool bMagnetic )
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->SetSlotMagneticMode( bMagnetic );
}

void CDnCharTalismanDlg::SetTalismanItem( int nSlotIndex, CDnItem *pItem )
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->SetTalismanItem( nSlotIndex, pItem );
	if( m_pCharTalismanInfoDlg && m_pCharTalismanInfoDlg->IsShow() )
		m_pCharTalismanInfoDlg->SetText();
}

void CDnCharTalismanDlg::ResetTalismanSlot( int nSlotIndex )
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->ResetTalismanSlot( nSlotIndex );
	if( m_pCharTalismanInfoDlg && m_pCharTalismanInfoDlg->IsShow() )
		m_pCharTalismanInfoDlg->SetText();
}

void CDnCharTalismanDlg::RefreshTalisman()
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->RefreshTalisman();
	if( m_pCharTalismanInfoDlg && m_pCharTalismanInfoDlg->IsShow() )
		m_pCharTalismanInfoDlg->SetText();
}

void CDnCharTalismanDlg::RefreshOpenSlot()
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->RefreshOpenSlot();
}

void CDnCharTalismanDlg::RefreshCashSlot()
{
	if( m_pCharTalismanListDlg )
		m_pCharTalismanListDlg->RefreshCashSlot();
}

CDnCharStatusDlg::eRetWearable CDnCharTalismanDlg::GetWearableTalismanEquipType( const MIInventoryItem *pItem, int &nSlotIndex, std::wstring *strErrorMsg )
{
	if( !m_pCharTalismanListDlg ) return CDnCharStatusDlg::eWEAR_UNABLE;
	return m_pCharTalismanListDlg->GetWearableTalismanEquipType( pItem, nSlotIndex, strErrorMsg );
}

#endif // PRE_ADD_TALISMAN_SYSTEM
