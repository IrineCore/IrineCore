#include "StdAfx.h"
#include "DnMessageBox.h"
#include "DnNpcDlg.h"
#include "DnAuthTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMessageBox::CDnMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pButtonOK(NULL)
	, m_pButtonCancel(NULL)
	, m_pButtonRetry(NULL)
	, m_pButtonYes(NULL)
	, m_pButtonNo(NULL)
	, m_pButtonAuthPw(NULL)
	, m_pButtonWaterBottle(NULL)
	, m_pButtonWater(NULL)
	, m_pStaticTitle(NULL)
	, m_BtnType(MB_OK)
	, m_bTopMost(false)
	, m_bSmartMoveNoButton(false)
	, m_bNoOverwrite(false)
	, m_bIgnoreEnterHotkey(false)
	, m_pStaticMessage(NULL)
{
}

CDnMessageBox::~CDnMessageBox(void)
{
}

void CDnMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MessageBox.ui" ).c_str(), bShow );
}

void CDnMessageBox::InitialUpdate()
{
	m_pButtonOK = GetControl<CEtUIButton>("ID_OK");
	m_pButtonCancel = GetControl<CEtUIButton>("ID_CANCEL");
	m_pButtonRetry = GetControl<CEtUIButton>("ID_RETRY");
	m_pButtonYes = GetControl<CEtUIButton>("ID_YES");
	m_pButtonNo = GetControl<CEtUIButton>("ID_NO");
	m_pButtonAuthPw = GetControl<CEtUIButton>("ID_BTN_AUTH_PW");
	m_pButtonWaterBottle = GetControl<CEtUIButton>("ID_BT_BUCKET");
	m_pButtonWater = GetControl<CEtUIButton>("ID_BT_USEPOT");
	m_pStaticMessage = GetControl<CEtUIStatic>("ID_MESSAGE");
	m_pStaticTitle = GetControl<CEtUIStatic>("ID_TITLE");

	m_pButtonOK->GetUICoord( m_ButtonCoord[0] );
	m_pButtonCancel->GetUICoord( m_ButtonCoord[1] );
	m_pButtonYes->GetUICoord( m_ButtonCoord[2] );

	m_strTitle = m_pStaticTitle->GetText();
	m_dwTitleColor = m_pStaticTitle->GetTextColor();
}

void CDnMessageBox::SetMessageBox( CDnInterface::InterfaceTypeEnum emType, LPCWSTR pwszMessage, UINT uType, int nID, CEtUICallback *pCall, bool bTopMost, bool bSmartMoveNoButton, bool bNoBtn, bool bNoOverwrite )
{
	if( IsShow() && m_bNoOverwrite )
		return;

	m_bIgnoreEnterHotkey = m_bSmartMoveNoButton = bSmartMoveNoButton;
	m_bNoOverwrite = bNoOverwrite;
	m_bTopMost = bTopMost;
	m_emInterfaceType = emType;
	m_nDialogID = nID;
	m_pCallback = pCall;
	m_pStaticMessage->SetText( pwszMessage );

	ShowControl( false );

	m_vecButtonControl.clear();

	if (bNoBtn)
	{
		m_BtnType = uType;
		return;
	}

	switch( uType )
	{
	case MB_OK:
		m_vecButtonControl.push_back( m_pButtonOK );
		break;
	case MB_OKCANCEL:
		m_vecButtonControl.push_back( m_pButtonOK );
		m_vecButtonControl.push_back( m_pButtonCancel );
		break;
	case MB_RETRYCANCEL:
		m_vecButtonControl.push_back( m_pButtonRetry );
		m_vecButtonControl.push_back( m_pButtonCancel );
		break;
	case MB_YESNO:
		m_vecButtonControl.push_back( m_pButtonYes );
		m_vecButtonControl.push_back( m_pButtonNo );
		break;
	case MB_YESNOCANCEL:
		m_vecButtonControl.push_back( m_pButtonYes );
		m_vecButtonControl.push_back( m_pButtonNo );
		m_vecButtonControl.push_back( m_pButtonCancel );
		break;
	case MB_CANCELTRYCONTINUE: 
		m_vecButtonControl.push_back( m_pButtonAuthPw);
		m_vecButtonControl.push_back( m_pButtonYes );
		m_vecButtonControl.push_back( m_pButtonNo );
		break; 
	case MB_ABORTRETRYIGNORE:	//For watering
		m_vecButtonControl.push_back( m_pButtonWaterBottle);
		m_vecButtonControl.push_back( m_pButtonWater );
		m_vecButtonControl.push_back( m_pButtonCancel );
		break;



	default:
		ASSERT( 0&&"CDnMessageBox::SetMessageBox" );
		break;
	}

	switch( uType )
	{
	case MB_OK:
		m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[2] );
		break;
	case MB_OKCANCEL:
	case MB_RETRYCANCEL:
	case MB_YESNO:
		m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[1] );
		m_vecButtonControl[1]->SetUICoord( m_ButtonCoord[2] );
		break;
	case MB_YESNOCANCEL:
	case MB_CANCELTRYCONTINUE:
	case MB_ABORTRETRYIGNORE:
		m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[0] );
		m_vecButtonControl[1]->SetUICoord( m_ButtonCoord[1] );
		m_vecButtonControl[2]->SetUICoord( m_ButtonCoord[2] );
		break;
	default:
		ASSERT( 0&&"CDnMessageBox::SetMessageBox" );
		break;
	}

	m_BtnType = uType;

	for( int i = 0; i < ( int )m_vecButtonControl.size(); i++ )
	{
		m_vecButtonControl[ i ]->Show( true );
	}
}

void CDnMessageBox::Show( bool bShow )
{ 
	if( bShow == m_bShow )
		return;

	if( bShow )
	{
		if (m_vecButtonControl.size() > 0)
		{
			if( m_bSmartMoveNoButton && m_BtnType != MB_OK && m_vecButtonControl.size() > 1 && m_vecButtonControl[1] ) 
			{
				if( m_BtnType == MB_CANCELTRYCONTINUE )	//Request focus handling specifically for the 2nd password create button
					m_SmartMove.SetControl( m_vecButtonControl[1] );
				else 
					m_SmartMove.SetControl( m_vecButtonControl[1] );

				m_bSmartMoveNoButton = false;	// Handle as a one-time setting.
			}
			else 
			{
				if( m_BtnType == MB_CANCELTRYCONTINUE )
				{
					m_SmartMove.SetControl( m_vecButtonControl[1] );
				}
				else 
				{
					m_SmartMove.SetControl( m_vecButtonControl[0] );
				}

			}
			m_SmartMove.MoveCursor();

			// Pausing modal window focus handling for now due to insufficient understanding of the focus system.
			//std::vector<CEtUIControl*> pControlVec = focus::GetPrevControl();
			//int nSize = pControlVec.size();
			//
			//focus::PushControl(this);
			//focus::SetFocus(m_vecButtonControl[0]);
		}
	}
	else
	{
		//focus::PopControl(this);

		// If it is an item destruction confirmation dialog, directly send the No callback when Show(false) is called.
		if( m_nDialogID == MESSAGEBOX_23 )
			CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonNo, 0 );

		if (m_vecButtonControl.size() > 0)
			m_SmartMove.ReturnCursor();

		SetDialogID(-1);
		SetCallback(NULL);
		SetTitle( m_strTitle.c_str(), m_dwTitleColor );
		m_bNoOverwrite = false;	// Initialize when window closes
	}

	CEtUIDialog::Show( bShow );
}

void CDnMessageBox::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show(false);
	}
}

bool CDnMessageBox::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	// Likely ESC key handling code used instead of hotkey settings.
	// Because of this code, pressing ESC clears Show, and
	// As a result, parent's ProcessCommand is not called for unshown controls, and
	// Consequently, m_pTrashItemButton = NULL; in CDnInvenTabDlg::OnUICallbackProc is not called, so
	// This led to a bug where the item destruction message box would not appear again after pressing ESC once.
	//
	// Could fix using an observer or similar, but
	// Because increasing complexity before fully understanding the code could cause other issues,
	// Handle this by commenting out the code below.

//	if( uMsg == WM_KEYDOWN )
//	{
//		if( wParam == VK_ESCAPE )
//		{
////			CEtUIDialog::ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl("ID_BUTTON_CLOSE"), 0 );
//			Show(false);
//			return true;
//		}
//	}

// 	if (uMsg == WM_KEYDOWN)
// 	{
// 		if (wParam == VK_RETURN)
// 		{
// 			if (m_BtnType == MB_OK)
// 			{
// 				Show(false);
// 				return true;
// 			}
// 		}
// 	}

	if( m_bIgnoreEnterHotkey )
	{
		if( uMsg == WM_KEYDOWN )
		{
			if( m_BtnType == MB_YESNO )
			{
				if( m_pButtonYes->GetHotKey() == wParam )
				{
					ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pButtonNo, 0 );
					return true;
				}
			}
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnMessageBox::ShowControl( bool bShow )
{
	m_pButtonOK->Show(bShow);
	m_pButtonCancel->Show(bShow);
	m_pButtonRetry->Show(bShow);
	m_pButtonYes->Show(bShow);
	m_pButtonNo->Show(bShow);
	m_pButtonAuthPw->Show(bShow);
	m_pButtonWater->Show(bShow);
	m_pButtonWaterBottle->Show(bShow);
}

void CDnMessageBox::SetTitle( LPCWSTR wszTitle, DWORD dwColor )
{
	m_pStaticTitle->SetText( wszTitle );
	m_pStaticTitle->SetTextColor( dwColor );
}


//////////////////////////////////////////////////////////////////////////

CDnMessageBoxTextBox::CDnMessageBoxTextBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnMessageBox( dialogType, pParentDialog, nID, pCallback )
, m_pTextMessage( NULL )
{
}

CDnMessageBoxTextBox::~CDnMessageBoxTextBox(void)
{
}

void CDnMessageBoxTextBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MessageBox_TextBox.ui" ).c_str(), bShow );
}

void CDnMessageBoxTextBox::InitialUpdate()
{
	CDnMessageBox::InitialUpdate();
	m_pTextMessage = GetControl<CEtUITextBox>( "ID_TEXTBOX_MESSAGE" );
}

void CDnMessageBoxTextBox::SetMessageBox( CDnInterface::InterfaceTypeEnum emType, LPCWSTR pwszMessage, UINT uType, int nID, CEtUICallback *pCall, bool bTopMost, bool bSmartMoveNoButton, bool bNoBtn, bool bNoOverwrite )
{
	if( IsShow() && m_bNoOverwrite )
		return;

	m_bIgnoreEnterHotkey = m_bSmartMoveNoButton = bSmartMoveNoButton;
	m_bNoOverwrite = bNoOverwrite;
	m_bTopMost = bTopMost;
	m_emInterfaceType = emType;
	m_nDialogID = nID;
	m_pCallback = pCall;
	m_pTextMessage->AddText( pwszMessage );

	ShowControl( false );

	m_vecButtonControl.clear();

	if (bNoBtn)
	{
		m_BtnType = uType;
		return;
	}

	switch( uType )
	{
		case MB_OK:
			m_vecButtonControl.push_back( m_pButtonOK );
			break;
		case MB_OKCANCEL:
			m_vecButtonControl.push_back( m_pButtonOK );
			m_vecButtonControl.push_back( m_pButtonCancel );
			break;
		case MB_RETRYCANCEL:
			m_vecButtonControl.push_back( m_pButtonRetry );
			m_vecButtonControl.push_back( m_pButtonCancel );
			break;
		case MB_YESNO:
			m_vecButtonControl.push_back( m_pButtonYes );
			m_vecButtonControl.push_back( m_pButtonNo );
			break;
		case MB_YESNOCANCEL:
			m_vecButtonControl.push_back( m_pButtonYes );
			m_vecButtonControl.push_back( m_pButtonNo );
			m_vecButtonControl.push_back( m_pButtonCancel );
			break;
		case MB_CANCELTRYCONTINUE: 
			m_vecButtonControl.push_back( m_pButtonAuthPw);
			m_vecButtonControl.push_back( m_pButtonYes );
			m_vecButtonControl.push_back( m_pButtonNo );
			break; 
		case MB_ABORTRETRYIGNORE:	//For watering
			m_vecButtonControl.push_back( m_pButtonWaterBottle);
			m_vecButtonControl.push_back( m_pButtonWater );
			m_vecButtonControl.push_back( m_pButtonCancel );
			break;
		default:
			ASSERT( 0&&"CDnMessageBox::SetMessageBox" );
			break;
	}

	switch( uType )
	{
		case MB_OK:
			m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[2] );
			break;
		case MB_OKCANCEL:
		case MB_RETRYCANCEL:
		case MB_YESNO:
			m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[1] );
			m_vecButtonControl[1]->SetUICoord( m_ButtonCoord[2] );
			break;
		case MB_YESNOCANCEL:
		case MB_CANCELTRYCONTINUE:
		case MB_ABORTRETRYIGNORE:
			m_vecButtonControl[0]->SetUICoord( m_ButtonCoord[0] );
			m_vecButtonControl[1]->SetUICoord( m_ButtonCoord[1] );
			m_vecButtonControl[2]->SetUICoord( m_ButtonCoord[2] );
			break;
		default:
			ASSERT( 0&&"CDnMessageBox::SetMessageBox" );
			break;
	}

	m_BtnType = uType;

	for( int i = 0; i < ( int )m_vecButtonControl.size(); i++ )
	{
		m_vecButtonControl[ i ]->Show( true );
	}
}

void CDnMessageBoxTextBox::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CDnMessageBox::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_TEXTBOX_SELECTION )
	{
		if( IsShow() && IsCmdControl( "ID_TEXTBOX_MESSAGE" ) )
		{
			CWord selectedWord = m_pTextMessage->GetSelectedWordData();
			if( selectedWord.m_strWordWithTag.empty() == false )
				EtInterface::GetNameLinkMgr().TranslateText( std::wstring(), selectedWord.m_strWordWithTag.c_str(), this );
		}
	}
}

#include <exdisp.h>
CComQIPtr<IWebBrowser2> g_pWebBrowser;
HWND					g_hwndWebBrowser;

void CDnMessageBoxTextBox::OnOpenHyperLink( const std::wstring& argString )
{
	HRESULT hr;
	hr = CoCreateInstance( CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&g_pWebBrowser );

	if( FAILED(hr) )
		return;

	SHANDLE_PTR hBrowserWnd = 0;
	g_pWebBrowser->get_HWND( &hBrowserWnd );
	g_hwndWebBrowser = reinterpret_cast<HWND>(hBrowserWnd);

	CComVariant vUrl( argString.c_str() ), vEmpty;
	VARIANT vFlags;
	V_VT(&vFlags) = VT_I4;
	V_I4(&vFlags) = navNoReadFromCache;
	hr = g_pWebBrowser->Navigate2( &vUrl, &vFlags, &vEmpty, &vEmpty, &vEmpty );

	if( SUCCEEDED(hr) )
	{
		g_pWebBrowser->put_Visible( VARIANT_TRUE );
		ShowWindow( (HWND)g_hwndWebBrowser, SW_SHOW );
		SetForegroundWindow( (HWND)g_hwndWebBrowser );
		SetActiveWindow( (HWND)g_hwndWebBrowser );
		InvalidateRect( g_hwndWebBrowser, NULL, true );
	}
}