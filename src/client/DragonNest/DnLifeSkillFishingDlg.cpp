#include "StdAfx.h"
#include "DnLifeSkillFishingDlg.h"
#include "DnLifeSkillFishingTask.h"
#include "DnInterface.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnLifeSkillFishingDlg::CDnLifeSkillFishingDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pStaticState( NULL )
, m_pControlBar( NULL )
, m_pProgressStrength( NULL )
, m_pProgressPower( NULL )
, m_pButtonControlGuide( NULL )
, m_pButtonAutoFishing( NULL )
, m_pButtonCancel( NULL )
, m_pGuideBoard( NULL )
, m_pGuideMouse( NULL )
, m_pGuideMouseLeftButton( NULL )
, m_pGuideMouseRightButton( NULL )
, m_pGuideLeftChatBalloon( NULL )
, m_pGuideLeftChatBalloonTail( NULL )
, m_pGuideRightChatBalloon( NULL )
, m_pGuideRightChatBalloonTail( NULL )
, m_pButtonCloseGuide( NULL )
, m_bShowGuide( false )
, m_bAutoFishing( false )
, m_bSpeedUp( false )
, m_bDoFishingEnd( false )
, m_bMouseLButtonDown( false )
, m_bRecieveResult( false )
, m_fElapsedStrengthTime( 0.0f )
, m_fElapsedStrengthTemp( 0.0f )
, m_fElapsedPowerTime( 0.0f )
{
}

CDnLifeSkillFishingDlg::~CDnLifeSkillFishingDlg()
{
}

void CDnLifeSkillFishingDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "LifeFishingDlg.ui" ).c_str(), bShow );
}

void CDnLifeSkillFishingDlg::InitialUpdate()
{
	m_pStaticState = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pProgressStrength = GetControl<CEtUIProgressBar>( "ID_PRB_TIME" );
	m_pProgressPower = GetControl<CDnFishingProgressBar>( "ID_PRB_CONTROL" );
	m_pControlBar = GetControl<CEtUIStatic>( "ID_CONTROLBAR" );
	m_pButtonControlGuide = GetControl<CEtUIButton>( "ID_BT_HELP" );
	m_pButtonAutoFishing = GetControl<CEtUIButton>( "ID_BT_AUTO" );
	m_pButtonCancel = GetControl<CEtUIButton>( "ID_BT_CANCEL" );

	m_pGuideBoard = GetControl<CEtUIStatic>( "ID_BOARD_HELP" );
	m_pGuideMouse = GetControl<CEtUIStatic>( "ID_MOUSE" );
	m_pGuideMouseLeftButton = GetControl<CEtUIStatic>( "ID_MOUSE_LEFT" );
	m_pGuideMouseRightButton = GetControl<CEtUIStatic>( "ID_MOUSE_RIGHT" );
	m_pGuideLeftChatBalloon = GetControl<CEtUIStatic>( "ID_LEFT_CHAT" );
	m_pGuideLeftChatBalloonTail = GetControl<CEtUIStatic>( "ID_LEFT_TAIL" );
	m_pGuideRightChatBalloon = GetControl<CEtUIStatic>( "ID_RIGHT_CHAT" );
	m_pGuideRightChatBalloonTail = GetControl<CEtUIStatic>( "ID_RIGHT_TAIL" );
	m_pButtonCloseGuide = GetControl<CEtUIButton>( "ID_BT_CLOSE" );

	m_pStaticState->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7404 ) );
	m_pButtonControlGuide->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7406 ) );
	m_pButtonAutoFishing->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7407 ) );
	m_pButtonCancel->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10 ) );

	m_pProgressStrength->GetUICoord( m_ProgressStrengthUiCoord );
	
	ShowGuide( m_bShowGuide );
}

void CDnLifeSkillFishingDlg::SetShowGuide( bool bShow )
{
	m_bShowGuide = bShow;
	ShowGuide( bShow );
}

void CDnLifeSkillFishingDlg::ShowGuide( bool bShow )
{
	m_pGuideBoard->Show( bShow );
	m_pGuideMouse->Show( bShow );
	m_pGuideMouseLeftButton->Show( bShow );
	m_pGuideMouseLeftButton->SetBlink( false );
	m_pGuideMouseRightButton->Show( bShow );
	m_pGuideMouseRightButton->SetBlink( false );
	m_pGuideLeftChatBalloon->Show( false );
	m_pGuideLeftChatBalloonTail->Show( false );
	m_pGuideRightChatBalloon->Show( false );
	m_pGuideRightChatBalloonTail->Show( false );
	m_pButtonCloseGuide->Show( bShow );
}

void CDnLifeSkillFishingDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
		GetInterface().LockMainMenu( true );
	else
	{
		GetInterface().LockMainMenu( false );

		// Initialize fishing info
		ResetFishingInfo();
		m_bAutoFishing = false;
		m_bMouseLButtonDown = false;

		// Initialize UI
		m_bShowGuide = false;
		ShowGuide( false );

		m_pStaticState->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7404 ) );
		m_pButtonAutoFishing->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7407 ) );
		m_pButtonControlGuide->Enable( true );
		m_pProgressPower->Show( true );
		m_pControlBar->Show( true );
		m_pProgressStrength->SetUICoord( m_ProgressStrengthUiCoord );
	}

	CEtUIDialog::Show( bShow );
}

void CDnLifeSkillFishingDlg::ResetFishingInfo()
{
	memset( &m_FishingInfo, 0, sizeof(stFishingInfo) );
	m_fElapsedStrengthTime = 0.0f;
	m_fElapsedStrengthTemp = 0.0f;
	m_fElapsedPowerTime = 0.0f;
	m_bSpeedUp = false;
	m_bDoFishingEnd = false;
	m_pProgressStrength->SetProgress( 0.0f );
	m_pProgressPower->SetProgress( 0.0f );
}

void CDnLifeSkillFishingDlg::SetFishingInfo( stFishingInfo& finshinginfo )
{
	// Initialize
	ResetFishingInfo();

	// Set fishing info
	m_FishingInfo.m_fStrengthTime = finshinginfo.m_fStrengthTime;
	m_FishingInfo.m_fPowerSpeedTime = finshinginfo.m_fPowerSpeedTime;
	m_FishingInfo.m_fPowerTime = finshinginfo.m_fPowerTime;
	m_FishingInfo.m_fSpeedUpStartTime = finshinginfo.m_fSpeedUpStartTime;
	m_FishingInfo.m_fSpeedUpEndTime = finshinginfo.m_fSpeedUpEndTime;
	m_FishingInfo.m_fSpeedUpFactor = finshinginfo.m_fSpeedUpFactor;

	// Set up UI
	float fSpeedUpStart = ( m_FishingInfo.m_fSpeedUpStartTime / m_FishingInfo.m_fPowerTime ) * 100.0f;
	float fSpeedUpEnd = ( m_FishingInfo.m_fSpeedUpEndTime / m_FishingInfo.m_fPowerTime ) * 100.0f;
	m_pProgressPower->SetSpeedUpSection( fSpeedUpStart, fSpeedUpEnd );

	SUICoord UiCoordProgress;
	m_pProgressPower->GetUICoord( UiCoordProgress );
	float fLengthPerTime = UiCoordProgress.fWidth / finshinginfo.m_fPowerTime;

	SUICoord UiCoord;
	m_pControlBar->GetUICoord( UiCoord );
	float fSpeedUpStartPos = UiCoordProgress.fX + ( fLengthPerTime * finshinginfo.m_fSpeedUpStartTime ) - 0.005f;			// UI adjustment
	m_pControlBar->SetPosition( fSpeedUpStartPos, UiCoord.fY );
	float fSpeedUpWidth = fLengthPerTime * ( finshinginfo.m_fSpeedUpEndTime - finshinginfo.m_fSpeedUpStartTime ) + 0.01f;	// UI adjustment
	m_pControlBar->SetSize( fSpeedUpWidth, UiCoord.fHeight );
}

void CDnLifeSkillFishingDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_HELP" ) )
		{
			m_bShowGuide = !m_bShowGuide;
			ShowGuide( m_bShowGuide );
		}
		else if( IsCmdControl( "ID_BT_AUTO" ) )
		{
			m_bAutoFishing = !m_bAutoFishing;
			if( m_bAutoFishing )
			{
				m_pStaticState->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7405 ) );
				m_pButtonAutoFishing->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7408 ) );
				m_pButtonControlGuide->Enable( false );
				ShowGuide( false );
				m_pProgressPower->Show( false );
				m_pControlBar->Show( false );
				m_bShowGuide = false;

				SUICoord ProgressStrengthUiCoord = m_ProgressStrengthUiCoord;
				ProgressStrengthUiCoord.fY += 0.01f;	// Move HP progress bar to center during auto-fishing
				m_pProgressStrength->SetUICoord( ProgressStrengthUiCoord );

				ResetFishingInfo();
				GetLifeSkillFishingTask().SendFishingCancel( true );
				GetLifeSkillFishingTask().SendFishingReady();
			}
			else
			{
				m_pStaticState->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7404 ) );
				m_pButtonAutoFishing->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7407 ) );
				m_pButtonControlGuide->Enable( true );
				m_pProgressPower->Show( true );
				m_pControlBar->Show( true );
				m_pProgressStrength->SetUICoord( m_ProgressStrengthUiCoord );

				ResetFishingInfo();
				GetLifeSkillFishingTask().SendFishingCancel( true );
				GetLifeSkillFishingTask().SendFishingReady();
			}
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) )
		{
			Show( false );
			GetLifeSkillFishingTask().SendFishingCancel( false );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7493 ), false );
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) )
		{
			m_bShowGuide = false;
			ShowGuide( false );
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_FISHING);
		}
#endif
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnLifeSkillFishingDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process(fElapsedTime);

	if( !CDnLifeSkillFishingTask::IsActive() ) return;

//	if( GetLifeSkillFishingTask().GetFishingState() == eFishingState::STATE_FISHING )
//		m_pButtonAutoFishing->Enable( true );
//	else
//		m_pButtonAutoFishing->Enable( false );

	if( !IsShow() || !IsValidFishingState() )
		return;

	// Check speed-up
	if( m_bMouseLButtonDown )
	{
		if( m_fElapsedPowerTime >= m_FishingInfo.m_fSpeedUpStartTime && m_fElapsedPowerTime <= m_FishingInfo.m_fSpeedUpEndTime )
			m_bSpeedUp = true;
		else
			m_bSpeedUp = false;
	}
	else
		m_bSpeedUp = false;

	// Toggle speech bubble
	if( m_bShowGuide )
	{
		if( m_fElapsedPowerTime >= m_FishingInfo.m_fSpeedUpStartTime )
			ToggleLeftChatBalloon( true );
		else
			ToggleLeftChatBalloon( false );

		if( m_fElapsedStrengthTime >= m_FishingInfo.m_fStrengthTime )
		{
			ToggleLeftChatBalloon( false );
			ToggleRightChatBalloon( true );
		}
		else
			ToggleRightChatBalloon( false );
	}

	// Progress process
	if( m_fElapsedStrengthTime < m_FishingInfo.m_fStrengthTime )
	{
		if( m_bSpeedUp && !m_bAutoFishing )
			m_fElapsedStrengthTime += ( fElapsedTime * m_FishingInfo.m_fSpeedUpFactor );
		else
			m_fElapsedStrengthTime += fElapsedTime;

		m_fElapsedStrengthTemp += fElapsedTime;

		m_pProgressStrength->SetProgress( m_fElapsedStrengthTime / m_FishingInfo.m_fStrengthTime * 100.0f );
	}
	else
	{
		m_pProgressStrength->SetProgress( 100.0f );
		if( m_bAutoFishing && GetLifeSkillFishingTask().IsRecieveFishingEnd() )
		{
			ResetFishingInfo();
			GetLifeSkillFishingTask().DoFishingEnd();
		}
	}

	if( !m_bAutoFishing )
	{
		if( !m_bRecieveResult && !m_bDoFishingEnd )
		{
			if( !m_bMouseLButtonDown )
				m_fElapsedPowerTime += fElapsedTime;
			else
				m_fElapsedPowerTime -= fElapsedTime;	
			
			if( m_fElapsedPowerTime > m_FishingInfo.m_fPowerTime )	// Restart from the beginning on timeout
				m_fElapsedPowerTime = 0.0f;

			if( m_fElapsedPowerTime < 0.0f )	// Clamp to 0.0f if gauge drains completely while holding mouse button
				m_fElapsedPowerTime = 0.0f;
		}

		m_pProgressPower->SetProgress( m_fElapsedPowerTime / m_FishingInfo.m_fPowerTime * 100.0f );
	}
}

void CDnLifeSkillFishingDlg::ToggleLeftChatBalloon( bool bShow )
{
	if( m_pGuideLeftChatBalloon->IsShow() == bShow )
		return;

	m_pGuideMouseLeftButton->SetBlink( bShow );
	m_pGuideLeftChatBalloon->Show( bShow );
	m_pGuideLeftChatBalloonTail->Show( bShow );
}

void CDnLifeSkillFishingDlg::ToggleRightChatBalloon( bool bShow )
{
	if( m_pGuideRightChatBalloon->IsShow() == bShow )
		return;

	m_pGuideMouseRightButton->SetBlink( bShow );
	m_pGuideRightChatBalloon->Show( bShow );
	m_pGuideRightChatBalloonTail->Show( bShow );
}

bool CDnLifeSkillFishingDlg::IsValidFishingState()
{
	if( m_FishingInfo.m_fStrengthTime <= 0.0f || m_FishingInfo.m_fPowerTime <= 0.0f )
		return false;
	
	if( m_FishingInfo.m_fSpeedUpFactor < 1.0f )
		return false;

	if( m_FishingInfo.m_fSpeedUpStartTime < 0.0f || m_FishingInfo.m_fSpeedUpEndTime > m_FishingInfo.m_fPowerTime )
		return false;

	return true;
}

void CDnLifeSkillFishingDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
}

bool CDnLifeSkillFishingDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	// Prevent fishing action inside button area on WN_LBUTTONDOWN & UP
	POINT MousePoint;
	float fMouseX, fMouseY;
	MousePoint.x = short( LOWORD( lParam ) );
	MousePoint.y = short( HIWORD( lParam ) );
	PointToFloat( MousePoint, fMouseX, fMouseY );

	if( uMsg == WM_LBUTTONDOWN || ( !m_bMouseLButtonDown && uMsg == WM_LBUTTONUP ) )
	{
		if( m_pButtonControlGuide->IsInside( fMouseX, fMouseY ) || m_pButtonAutoFishing->IsInside( fMouseX, fMouseY ) || m_pButtonCancel->IsInside( fMouseX, fMouseY ) )
			return bRet;
	}

	if( !m_bAutoFishing )
	{
		switch( uMsg )
		{
			case WM_LBUTTONDOWN:
				{
					if( !m_bMouseLButtonDown )
					{
						m_bMouseLButtonDown = true;
						GetLifeSkillFishingTask().SendPullingRodElapsedTime( true );
					}
				}
				break;
			case WM_LBUTTONUP:
				{
					if( !m_bRecieveResult && m_bMouseLButtonDown )
					{
						m_bMouseLButtonDown = false;
						GetLifeSkillFishingTask().SendPullingRodElapsedTime( false );
					}
				}
				break;
			case WM_RBUTTONDOWN:
				{
					if( !m_bDoFishingEnd && m_bRecieveResult )
					{
						if( m_fElapsedStrengthTime >= m_FishingInfo.m_fStrengthTime )
						{
							GetLifeSkillFishingTask().DoFishingEnd();
							m_bDoFishingEnd = true;
						}
					}
				}
				break;
		}
	}

	if( uMsg == WM_MBUTTONDOWN )
		CDnMouseCursor::GetInstance().ShowCursor( false, true );
	else if( uMsg == WM_MBUTTONUP )
		CDnMouseCursor::GetInstance().ShowCursor( true, true );

	return bRet;
}

void CDnLifeSkillFishingDlg::FishingTimeSync( int nReduceTick, int nFishingGauge )
{
	if( nReduceTick > 0 )
	{
//		OutputDebug( "Fishing Sync nReduceTick: %.4f, %.4f\n", m_fElapsedStrengthTime, m_fElapsedStrengthTemp + static_cast<float>( nReduceTick ) / 1000.0f );
		m_fElapsedStrengthTime = m_fElapsedStrengthTemp + static_cast<float>( nReduceTick ) / 1000.0f;
	}

//	OutputDebug( "Fishing Sync nFishingGauge: %.4f, %.4f\n", m_fElapsedPowerTime, ( static_cast<float>( nFishingGauge ) / 1000.0f ) / m_FishingInfo.m_fPowerSpeedTime );
	m_fElapsedPowerTime = ( static_cast<float>( nFishingGauge ) / 1000.0f ) / m_FishingInfo.m_fPowerSpeedTime;
}

void CDnLifeSkillFishingDlg::RecieveResult( bool bRecieveResult )
{
	m_bRecieveResult = bRecieveResult;

	if( m_pButtonCancel )
		m_pButtonCancel->Enable( !bRecieveResult );

	if( m_pButtonAutoFishing )
		m_pButtonAutoFishing->Enable( !bRecieveResult );

	if( bRecieveResult )
		m_fElapsedPowerTime = 0.0f;
}
