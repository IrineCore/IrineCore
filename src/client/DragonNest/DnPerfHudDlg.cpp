#include "StdAfx.h"
#include <mmsystem.h>
#ifdef PlaySound
#undef PlaySound
#endif
#include "DnPerfHudDlg.h"
#include "DnBridgeTask.h"
#include "DnMouseCursor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

namespace
{
	bool s_bPerfHudUserMoved = false;
	float s_fPerfHudSavedOffsetX = 0.0f;
	float s_fPerfHudSavedOffsetY = 0.0f;
}

CDnPerfHudDlg::CDnPerfHudDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticFps( NULL )
	, m_pStaticDivider( NULL )
	, m_pStaticPing( NULL )
	, m_dwFpsLastTick( 0 )
	, m_nFpsFrameCount( 0 )
	, m_nDisplayFps( 0 )
	, m_nLastDisplayedFps( -1 )
	, m_nLastDisplayedPing( -2 )
	, m_bDragging( false )
{
	m_ptLastMouse.x = 0;
	m_ptLastMouse.y = 0;
}

CDnPerfHudDlg::~CDnPerfHudDlg( void )
{
}

void CDnPerfHudDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PerfHudDlg.ui" ).c_str(), bShow );
}

void CDnPerfHudDlg::InitialUpdate()
{
	m_pStaticFps = GetControl< CEtUIStatic >( "ID_STATIC_FPS" );
	m_pStaticDivider = GetControl< CEtUIStatic >( "ID_STATIC_DIVIDER" );
	m_pStaticPing = GetControl< CEtUIStatic >( "ID_STATIC_PING" );

	if( m_pStaticDivider )
	{
		m_pStaticDivider->SetText( L"|" );
		m_pStaticDivider->SetTextColor( 0xFF7B8FA6 );
	}

	if( s_bPerfHudUserMoved )
	{
		MoveDialog( s_fPerfHudSavedOffsetX, s_fPerfHudSavedOffsetY );
	}

	RefreshPerfText( true );
}

void CDnPerfHudDlg::RefreshPerfText( bool bForce )
{
	int nPingMs = CDnBridgeTask::IsActive() ? CDnBridgeTask::GetInstance().GetClientPingMs() : -1;

	if( !bForce && m_nDisplayFps == m_nLastDisplayedFps && nPingMs == m_nLastDisplayedPing )
		return;

	m_nLastDisplayedFps = m_nDisplayFps;
	m_nLastDisplayedPing = nPingMs;

	if( m_pStaticFps )
	{
		WCHAR wszFps[32] = { 0, };
		swprintf_s( wszFps, _countof( wszFps ), L"FPS: %d", m_nDisplayFps );
		DWORD dwFpsColor = 0xFF5CE66E;
		if( m_nDisplayFps < 30 )
			dwFpsColor = 0xFFFF5E5E;
		else if( m_nDisplayFps < 50 )
			dwFpsColor = 0xFFFFD257;

		m_pStaticFps->SetText( wszFps );
		m_pStaticFps->SetTextColor( dwFpsColor );
	}

	if( m_pStaticPing )
	{
		WCHAR wszPing[32] = { 0, };
		DWORD dwPingColor = 0xFFB0BEC5;
		if( nPingMs >= 0 )
		{
			swprintf_s( wszPing, _countof( wszPing ), L"PING: %dms", nPingMs );
			if( nPingMs <= 60 )
				dwPingColor = 0xFF5CE66E;
			else if( nPingMs <= 140 )
				dwPingColor = 0xFFFFD257;
			else
				dwPingColor = 0xFFFF5E5E;
		}
		else
		{
			wcscpy_s( wszPing, _countof( wszPing ), L"PING: --" );
		}

		m_pStaticPing->SetText( wszPing );
		m_pStaticPing->SetTextColor( dwPingColor );
	}
}

void CDnPerfHudDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	RefreshPerfText( false );
}

void CDnPerfHudDlg::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	DWORD dwNow = timeGetTime();
	if( m_dwFpsLastTick == 0 )
	{
		m_dwFpsLastTick = dwNow;
		m_nFpsFrameCount = 0;
	}
	++m_nFpsFrameCount;
	DWORD dwElapsed = dwNow - m_dwFpsLastTick;
	if( dwElapsed >= 350 )
	{
		m_nDisplayFps = (int)( ( m_nFpsFrameCount * 1000 + ( dwElapsed / 2 ) ) / dwElapsed );
		m_nFpsFrameCount = 0;
		m_dwFpsLastTick = dwNow;
		RefreshPerfText( false );
	}

	CEtUIDialog::Render( fElapsedTime );
}

bool CDnPerfHudDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_LBUTTONDOWN )
	{
		if( !CDnMouseCursor::GetInstance().IsShowCursor() )
			return false;

		POINT pt;
		pt.x = short( LOWORD( lParam ) );
		pt.y = short( HIWORD( lParam ) );
		float fLocalX = 0.0f, fLocalY = 0.0f;
		PointToFloat( pt, fLocalX, fLocalY );

		if( fLocalX >= 0.0f && fLocalX <= m_DlgInfo.DlgCoord.fWidth &&
			fLocalY >= 0.0f && fLocalY <= m_DlgInfo.DlgCoord.fHeight )
		{
			m_bDragging = true;
			m_ptLastMouse = pt;
			return true;
		}
	}
	else if( uMsg == WM_MOUSEMOVE )
	{
		if( m_bDragging )
		{
			if( !CDnMouseCursor::GetInstance().IsShowCursor() )
			{
				m_bDragging = false;
				return false;
			}

			POINT pt;
			pt.x = short( LOWORD( lParam ) );
			pt.y = short( HIWORD( lParam ) );

			float fScreenW = GetScreenWidth();
			float fScreenH = GetScreenHeight();
			if( fScreenW > 0.0f && fScreenH > 0.0f )
			{
				float fDX = ( pt.x - m_ptLastMouse.x ) / fScreenW;
				float fDY = ( pt.y - m_ptLastMouse.y ) / fScreenH;

				float fCurX = GetXCoord();
				float fCurY = GetYCoord();
				float fMaxX = ( 1.0f + GetScreenWidthBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fWidth;
				float fMaxY = ( 1.0f + GetScreenHeightBorderSize() * 2.0f ) - m_DlgInfo.DlgCoord.fHeight;

				if( fCurX + fDX < 0.0f ) fDX = -fCurX;
				if( fCurX + fDX > fMaxX ) fDX = fMaxX - fCurX;
				if( fCurY + fDY < 0.0f ) fDY = -fCurY;
				if( fCurY + fDY > fMaxY ) fDY = fMaxY - fCurY;

				MoveDialog( fDX, fDY );
				s_fPerfHudSavedOffsetX += fDX;
				s_fPerfHudSavedOffsetY += fDY;
				s_bPerfHudUserMoved = true;
			}

			m_ptLastMouse = pt;
			return true;
		}
	}
	else if( uMsg == WM_LBUTTONUP )
	{
		if( m_bDragging )
		{
			m_bDragging = false;
			return true;
		}
	}

	return false;
}
