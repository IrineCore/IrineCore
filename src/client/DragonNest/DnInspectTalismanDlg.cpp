#include "StdAfx.h"

#if defined(PRE_ADD_TALISMAN_SYSTEM)

#include "DnInspectTalismanDlg.h"
#include "DnInspectPlayerDlg.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "DnTalisman.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//////////////////////////////////////////////////////////////////////////
// CDnInspectTalismanDlg

CDnInspectTalismanDlg::CDnInspectTalismanDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pInspectTalismanSlotDlg( NULL )
, m_pInspectTalismanInfoDlg( NULL )
, m_pBtnOption( NULL )
, m_pListBoxTalisman( NULL )
, m_pScrollBar( NULL )
, m_pTextOption( NULL )
, m_nSlotCount( 0 )
, m_nScrollMax( 40 )
, m_dwSessionID( 0 )
{
}

CDnInspectTalismanDlg::~CDnInspectTalismanDlg()
{
	SAFE_DELETE( m_pInspectTalismanSlotDlg );
	SAFE_DELETE( m_pInspectTalismanInfoDlg );
}

void CDnInspectTalismanDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlayerTalismanDlg.ui" ).c_str(), bShow );
}

void CDnInspectTalismanDlg::InitialUpdate()
{
	m_pInspectTalismanInfoDlg = new CDnInspectTalismanInfoDlg( UI_TYPE_CHILD, this );
	m_pInspectTalismanInfoDlg->Initialize( false );

	m_pInspectTalismanSlotDlg = new CDnInspectTalismanSlotDlg( UI_TYPE_SELF, NULL );
	m_pInspectTalismanSlotDlg->SetParentInspectDlg( this );
	m_pInspectTalismanSlotDlg->Initialize( false );

	CEtUIButton *pComposeBtn = GetControl<CEtUIButton>( "ID_BT_COMPOSE" );
	if( pComposeBtn )
		pComposeBtn->Enable( false );

	m_pTextOption = GetControl<CEtUIStatic>( "ID_TEXT_OPTION" );
	m_pBtnOption = GetControl<CEtUIButton>( "ID_BT_OPTION" );
	m_pListBoxTalisman = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_TALISMAN" );
	if( m_pListBoxTalisman )
	{
		m_pScrollBar = m_pListBoxTalisman->GetScrollBar();
		if( m_pScrollBar )
		{
			m_pScrollBar->SetPageSize( 25 );
			m_pScrollBar->SetTrackRange( 0, m_nScrollMax );
		}
	}

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( pSox )
		m_nSlotCount = pSox->GetItemCount();
}

void CDnInspectTalismanDlg::UpdateListPosition()
{
	if( !m_pInspectTalismanSlotDlg || !m_pListBoxTalisman )
		return;

	SUICoord listCoord;
	m_pListBoxTalisman->GetUICoord( listCoord );

	float fContentHeight = m_pInspectTalismanSlotDlg->GetContentHeight();
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

	m_pInspectTalismanSlotDlg->SetPosition(
		GetXCoord() + listCoord.fX,
		GetYCoord() + listCoord.fY + CDnCharTalismanListDlg::TALISMAN_LIST_TOP_PADDING - fScrollOffset );
}

void CDnInspectTalismanDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CDnCustomDlg::Show( bShow );

	if( m_pInspectTalismanSlotDlg )
	{
		m_pInspectTalismanSlotDlg->Show( bShow );
		UpdateListPosition();
		if( bShow )
		{
			m_pInspectTalismanSlotDlg->SetEfficiencyTextShow( true );
			RefreshTalisman( m_dwSessionID );
		}
		else
		{
			if( m_pInspectTalismanInfoDlg && m_pInspectTalismanInfoDlg->IsShow() )
				m_pInspectTalismanInfoDlg->Show( false );
			m_pInspectTalismanSlotDlg->HideTooltip();
		}
	}
}

void CDnInspectTalismanDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( IsShow() && m_pInspectTalismanSlotDlg )
	{
		UpdateListPosition();
		m_pInspectTalismanSlotDlg->Process( fElapsedTime );
	}
}

void CDnInspectTalismanDlg::Render( float fElapsedTime )
{
	CDnCustomDlg::Render( fElapsedTime );

	if( !IsShow() || !m_pInspectTalismanSlotDlg || !m_pListBoxTalisman )
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

	m_pInspectTalismanSlotDlg->Render( fElapsedTime );

	CEtSprite::GetInstance().Flush();
	GetEtDevice()->SetScissorRect( &OriginalRect );
	GetEtDevice()->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
}

bool CDnInspectTalismanDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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

	if( m_pInspectTalismanSlotDlg )
	{
		UpdateListPosition();
		m_pInspectTalismanSlotDlg->SetHWnd( hWnd );
		if( m_pInspectTalismanSlotDlg->MsgProc( hWnd, uMsg, wParam, lParam ) )
			return true;
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnInspectTalismanDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED && IsCmdControl( "ID_BT_OPTION" ) )
	{
		if( m_pInspectTalismanInfoDlg )
		{
			if( m_pInspectTalismanInfoDlg->IsShow() )
			{
				m_pInspectTalismanInfoDlg->Show( false );
			}
			else
			{
				CDnInspectPlayerDlg *pParentInspect = dynamic_cast<CDnInspectPlayerDlg*>( GetParentDialog() );
				DWORD dwSession = pParentInspect ? pParentInspect->GetSessionID() : m_dwSessionID;
				DnActorHandle hActor = CDnActor::FindActorFromUniqueID( dwSession );
				CDnPlayerActor *pPlayer = hActor ? dynamic_cast<CDnPlayerActor*>( hActor.GetPointer() ) : NULL;
				if( pPlayer )
				{
					m_pInspectTalismanInfoDlg->SetText( pPlayer );
					m_pInspectTalismanInfoDlg->Show( true );
				}
			}
		}
		return;
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnInspectTalismanDlg::RefreshTalisman( DWORD dwSessionID )
{
	m_dwSessionID = dwSessionID;
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( dwSessionID );
	CDnPlayerActor *pPlayer = hActor ? dynamic_cast<CDnPlayerActor*>( hActor.GetPointer() ) : NULL;

	if( m_pInspectTalismanSlotDlg )
		m_pInspectTalismanSlotDlg->RefreshInspectTalisman( pPlayer );
	if( m_pInspectTalismanInfoDlg && m_pInspectTalismanInfoDlg->IsShow() && pPlayer )
		m_pInspectTalismanInfoDlg->SetText( pPlayer );
}

//////////////////////////////////////////////////////////////////////////
// CDnInspectTalismanSlotDlg

CDnInspectTalismanSlotDlg::CDnInspectTalismanSlotDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCharTalismanListDlg( dialogType, pParentDialog, nID, pCallback )
, m_pParentInspectDlg( NULL )
{
}

CDnInspectTalismanSlotDlg::~CDnInspectTalismanSlotDlg()
{
}

void CDnInspectTalismanSlotDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlayerTalismanListDlg.ui" ).c_str(), bShow );
}

void CDnInspectTalismanSlotDlg::InitialUpdate()
{
	SetAllignType( AT_HORI_LEFT_FIXED, AT_VERT_TOP );

	InitTalismanSlot();
	if( !m_pParentInspectDlg && m_pParentDialog )
		m_pParentInspectDlg = dynamic_cast<CDnInspectTalismanDlg*>( m_pParentDialog );

	// Inspect slots are read-only view slots
	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		if( m_vecSlotInfo[i].pSlotButton )
			m_vecSlotInfo[i].pSlotButton->SetSlotType( ST_INSPECT );
		if( m_vecSlotInfo[i].pStaticCover )
			m_vecSlotInfo[i].pStaticCover->Show( false );
		if( m_vecSlotInfo[i].pStaticCashCover )
			m_vecSlotInfo[i].pStaticCashCover->Show( false );
	}
}

void CDnInspectTalismanSlotDlg::Show( bool bShow )
{
	CDnCustomDlg::Show( bShow );
}

bool CDnInspectTalismanSlotDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !m_pParentInspectDlg || !m_pParentInspectDlg->IsShow() )
		return false;

	if( m_pParentInspectDlg->GetListBox() )
	{
		if( uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP )
		{
			POINT MousePoint;
			MousePoint.x = short( LOWORD( lParam ) );
			MousePoint.y = short( HIWORD( lParam ) );

			float fScreenX = 0.0f, fScreenY = 0.0f;
			m_pParentInspectDlg->PointToFloat( MousePoint, fScreenX, fScreenY );

			SUICoord listCoord;
			m_pParentInspectDlg->GetListBox()->GetUICoord( listCoord );
			if( !listCoord.IsInside( fScreenX, fScreenY ) )
				return false;
		}
	}

	return CDnCustomDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnInspectTalismanSlotDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnInspectTalismanSlotDlg::RefreshInspectTalisman( CDnPlayerActor *pActor )
{
	for( size_t i = 0; i < m_vecSlotInfo.size(); ++i )
	{
		if( !m_vecSlotInfo[i].pSlotButton ) continue;
		DnTalismanHandle hTalisman = pActor ? pActor->GetTalisman( (int)i ) : CDnTalisman::Identity();
		if( hTalisman )
			m_vecSlotInfo[i].pSlotButton->SetItem( hTalisman.GetPointer(), CDnSlotButton::ITEM_ORIGINAL_COUNT );
		else
			m_vecSlotInfo[i].pSlotButton->ResetSlot();
		m_vecSlotInfo[i].pSlotButton->OnRefreshTooltip();
	}
}

//////////////////////////////////////////////////////////////////////////
// CDnInspectTalismanInfoDlg

CDnInspectTalismanInfoDlg::CDnInspectTalismanInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCharTalismanInfoDlg( dialogType, pParentDialog, nID, pCallback )
{
}

CDnInspectTalismanInfoDlg::~CDnInspectTalismanInfoDlg()
{
}

void CDnInspectTalismanInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PlayerTalismanInfoDlg.ui" ).c_str(), bShow );
}

void CDnInspectTalismanInfoDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
		SetRenderPriority( this, true );

	CDnCharTalismanInfoDlg::Show( bShow );
}

bool CDnInspectTalismanInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	return CDnCharTalismanInfoDlg::MsgProc( hWnd, uMsg, wParam, lParam );
}

#endif // PRE_ADD_TALISMAN_SYSTEM
