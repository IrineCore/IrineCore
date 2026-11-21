#include "StdAfx.h"
#include "DnCharPetPreviewDlg.h"
#include "DnLocalPlayerActor.h"



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCharPetPreviewDlg::CDnCharPetPreviewDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pButtonRotateLeft( NULL )
, m_pButtonRotateRight( NULL )
, m_pButtonRotateFront( NULL )
, m_pButtonRotateBack( NULL )
, m_pButtonAvatarViewArea( NULL )
, m_fAvatarViewMouseX( 0.0f )
, m_fAvatarViewMouseY( 0.0f )
{
}

CDnCharPetPreviewDlg::~CDnCharPetPreviewDlg()
{
}

void CDnCharPetPreviewDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharPetPreviewDlg.ui" ).c_str(), bShow );
}

void CDnCharPetPreviewDlg::InitialUpdate()
{
	m_pButtonRotateLeft = GetControl<CEtUIButton>( "ID_ROTATE_LEFT" );
	m_pButtonRotateRight = GetControl<CEtUIButton>( "ID_ROTATE_RIGHT" );
	m_pButtonRotateFront = GetControl<CEtUIButton>( "ID_BUTTON_FRONT" );
	m_pButtonRotateBack = GetControl<CEtUIButton>( "ID_BUTTON_BACK" );
	m_pButtonAvatarViewArea = GetControl<CEtUIButton>( "ID_BUTTON_PREVIEW" );

	SUICoord charViewCoord;
	if( m_pButtonAvatarViewArea->GetProperty() != NULL )
		charViewCoord = m_pButtonAvatarViewArea->GetProperty()->UICoord;

	const int nTextureSize = 512;
	SCameraInfo CameraInfo;
	CameraInfo.Target = CT_RENDERTARGET_NO_GENERATE_BACKBUFFER;
	CameraInfo.fNear = 10.f;
	CameraInfo.Type = CT_ORTHOGONAL;
	m_RenderAvatar.Initialize( nTextureSize, nTextureSize, CameraInfo, 96, 72, FMT_A8R8G8B8, true, true );
	m_RenderAvatar.CalcUVCoord( charViewCoord.fWidth, charViewCoord.fHeight );
	m_RenderAvatar.SetCameraYPos( 30.0f );
}

void CDnCharPetPreviewDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		CDnPlayerActor *pDnPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if( pDnPlayerActor == NULL ) 
			return;

		if( m_RenderAvatar.GetActorID() )
			m_RenderAvatar.ResetActor();

		if( pDnPlayerActor && pDnPlayerActor->IsSummonPet() && pDnPlayerActor->GetSummonPet() )
			m_RenderAvatar.SetActor( pDnPlayerActor->GetSummonPet() );

		m_RenderAvatar.AddRotateYaw( 25.f );
	}
	else
	{
		m_RenderAvatar.ResetActor();
	}
}

void CDnCharPetPreviewDlg::Process( float fDelta )
{
	CEtUIDialog::Process( fDelta );

	if( !IsShow() ) return;

	m_RenderAvatar.Process( fDelta );

	static float fRotScale = 200.f;
	
	if( m_pButtonRotateLeft->IsPressed() )
		m_RenderAvatar.AddRotateYaw( fDelta * fRotScale );
	else if( m_pButtonRotateRight->IsPressed() )
		m_RenderAvatar.AddRotateYaw( -fDelta * fRotScale );

	if( m_pButtonAvatarViewArea->IsPressed() )
	{
		float fX = m_pButtonAvatarViewArea->GetMouseCoord().fX;
		float fY = m_pButtonAvatarViewArea->GetMouseCoord().fY;
		if( m_fAvatarViewMouseX != 0.f && m_fAvatarViewMouseY != 0.f )
		{
			static float fMoveScale = 500.f;
			float fAddAngle = fMoveScale * sqrtf( (m_fAvatarViewMouseX-fX)*(m_fAvatarViewMouseX-fX)+(m_fAvatarViewMouseY-fY)*(m_fAvatarViewMouseY-fY) ) *  (((m_fAvatarViewMouseX-fX)>0.f)? 1.f : -1.f) ;
			m_RenderAvatar.AddRotateYaw( fAddAngle );
		}
		m_fAvatarViewMouseX = fX;
		m_fAvatarViewMouseY = fY;

		focus::SetFocus( m_pButtonAvatarViewArea );
	}
	else if( m_pButtonAvatarViewArea->IsRightPressed() )
	{
		float fX = m_pButtonAvatarViewArea->GetMouseCoord().fX;
		float fY = m_pButtonAvatarViewArea->GetMouseCoord().fY;

		if( m_fAvatarViewMouseX != 0.f && m_fAvatarViewMouseY != 0.f ) {
			float fDistX = m_fAvatarViewMouseX - fX;
			float fDistY = m_fAvatarViewMouseY - fY;
			m_RenderAvatar.Panning( fDistX * GetScreenWidth(), fDistY * GetScreenHeight() );
		}

		m_fAvatarViewMouseX = fX;
		m_fAvatarViewMouseY = fY;

		focus::SetFocus( m_pButtonAvatarViewArea );
	}
	else
	{
		m_fAvatarViewMouseX = 0.f;
		m_fAvatarViewMouseY = 0.f;
	}

	if( m_RenderAvatar.IsFrontView() )
	{
		m_pButtonRotateFront->Show( false );
		m_pButtonRotateBack->Show( true );
	}
	else 
	{
		m_pButtonRotateFront->Show( true );
		m_pButtonRotateBack->Show( false );
	}
}

void CDnCharPetPreviewDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );

	if( IsShow() && m_RenderAvatar.GetActorID() )
	{
		SUICoord charViewCoord;
		if( m_pButtonAvatarViewArea->GetProperty() != NULL )
			charViewCoord = m_pButtonAvatarViewArea->GetProperty()->UICoord;
		DrawSprite( m_RenderAvatar.GetRTT(), m_RenderAvatar.GetUVCoord(), 0xFFFFFFFF, charViewCoord );
	}
}

void CDnCharPetPreviewDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_PREVIEW"))	
			focus::ReleaseControl();

		if( IsCmdControl("ID_BUTTON_FRONT") )
			m_RenderAvatar.SetFrontView();

		if( IsCmdControl("ID_BUTTON_BACK") )
			m_RenderAvatar.SetRearView();

		if( IsCmdControl("ID_BUTTON_ZOOMIN") )
			m_RenderAvatar.ZoomIn();

		if( IsCmdControl("ID_BUTTON_ZOOMOUT") )
			m_RenderAvatar.ZoomOut();
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnCharPetPreviewDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	switch( uMsg )
	{
		case WM_MOUSEWHEEL:
		{
			if( IsMouseInDlg() )
			{
				UINT uLines;
				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;

				if( nScrollAmount < 0 )
					m_RenderAvatar.ZoomOut();
				else if( nScrollAmount > 0 )
					m_RenderAvatar.ZoomIn();
			}
		}
		break;
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnCharPetPreviewDlg::SetAvatarAction( const char* szActionName )
{
	if( m_RenderAvatar.GetActor() )
		m_RenderAvatar.GetActor()->SetActionQueue( szActionName );
}

void CDnCharPetPreviewDlg::RefreshPetEquip()
{
	CDnPlayerActor *pDnPlayerActor = static_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	if( pDnPlayerActor == NULL ) 
		return;

	if( !m_RenderAvatar.GetActor() )
		return;

	if( pDnPlayerActor && pDnPlayerActor->IsSummonPet() && pDnPlayerActor->GetSummonPet() )
		m_RenderAvatar.RefreshEquip( pDnPlayerActor->GetSummonPet() );
}
