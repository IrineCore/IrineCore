#include "StdAfx.h"
#include <Richedit.h>
#include "DnDebugTask.h"
#include "PerfCheck.h"
#include "DnMainFrame.h"
#include "resource.h"
#include "DnActor.h"
#include "MATransAction.h"
#include "DnVillageTask.h"
#include "TaskManager.h"
#include "VelocityFunc.h"
#include "DnWorld.h"
#include "EtActionCoreMng.h"
#include "VillageSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

bool CDnDebugTask::s_bDialogMode = false;
CDnDebugTask::CDnDebugTask()
: CInputReceiver( true )
{
	m_hLibrary = LoadLibrary( _T("RichEd20.dll") );
	s_pThis = this;
	m_hWnd = CreateDialog( CDnMainFrame::GetInstance().GetHInst(), MAKEINTRESOURCE(IDD_DEBUGDLG), 
		CDnMainFrame::GetInstance().GetHWnd(), DialogProc );
	ShowWindow( m_hWnd, SW_HIDE );
	SendMessage( GetDlgItem( m_hWnd, IDC_EDIT1 ), EM_SETEVENTMASK, 0, ENM_CHANGE );
	m_bShow = false;
	m_nCursorPos = 0;
	m_nVertScrollPos = 0;
	m_bScrollLock = false;
	m_nTotalLineCount = 0;
	m_nStartLength = 0;
	m_nEndLength = 0;
	m_nMaxLineCount = 0;

	m_fVertVelocity = 0.05f;
	m_fVertVelocityTemp = 0.0f;
	memset( m_szParam, 0, sizeof( m_szParam ) );
}

CDnDebugTask::~CDnDebugTask()
{
	if( m_hLibrary )
		FreeLibrary( m_hLibrary );

	ShowWindow( m_hWnd, SW_HIDE );
	DestroyWindow( m_hWnd );
	SetOutputDebugFuncPtr( _OutputDebug );
}

CDnDebugTask *CDnDebugTask::s_pThis = NULL;

enum DebugCmdLineIndexEnum {
	HELP,
	CLS,
	OUTPUT,
	PROFILE,
	MEM,
	CHANGE_ENVI,
	CHANGE_DEBUG_MODE,
	SHOW_COLL,
	RELOAD_ACTION,
	DebugCmdLineIndexEnum_END,
};

CDnDebugTask::CmdLineDefineStruct g_DebugCmdLineDefineList[] = {
	HELP, _T("help"), _T("Help"),
	CLS, _T("cls"), _T("Clear screen"),
	OUTPUT, _T("output"), _T("(0/1), (on/off), Hide debug info"),
	PROFILE, _T("profile"), _T("Print accumulated profile values"),
	MEM, _T("mem"), _T("Print accumulated memory check values"),
	CHANGE_ENVI, _T("ce"), _T("(envi name) Change environment"),
	CHANGE_DEBUG_MODE, _T("dmode"), _T("(0:EngineConsole/1:WindowConsole), Debug task type"), 
	SHOW_COLL, _T("showcollision"), _T("(0:Hide/1:Show), Show/Hide collision box"), 
	RELOAD_ACTION, _T("reloadaction"), _T("Reload actions"),
	DebugCmdLineIndexEnum_END, NULL, NULL,
};


void CDnDebugTask::CommandDebug( DebugCmdLineIndexEnum Index )
{
	switch( Index ) {
		case HELP:
			OutputScript( "\nCommand List\n\n" );
			for( DWORD i=0; i<DebugCmdLineIndexEnum_END; i++ ) {
				OutputScript( " %s : %s\n", g_DebugCmdLineDefineList[i].szString, g_DebugCmdLineDefineList[i].szDescription );
			}
			break;
		case CLS:
			{
				HWND hWnd = GetDlgItem( m_hWnd, IDC_EDIT1 );
				SETTEXTEX TextEx;
				TextEx.flags = ST_DEFAULT;
				TextEx.codepage = CP_ACP;
				SendMessage( hWnd, EM_SETTEXTEX, (WPARAM)&TextEx, (LPARAM)"" );
				m_szAccumulationString.clear();
				m_nVertScrollPos = 0;
			}
			break;
		case OUTPUT:
			if( !_tcscmp( m_szParam, _T("on") ) || !_tcscmp( m_szParam, _T("1") ) ) SetOutputDebugFuncPtr( CDnDebugTask::OutputScriptString );
			else if( !_tcscmp( m_szParam, _T("off") ) || !_tcscmp( m_szParam, _T("0") ) ) SetOutputDebugFuncPtr( _OutputDebug );
			break;
		case PROFILE:
			ProfilePrint( CDnDebugTask::OutputScriptString );
			break;
		case MEM:
			ProfileMemPrint( CDnDebugTask::OutputScriptString );
			break;
		case CHANGE_ENVI:
			{
				char szTemp[128], szEnviName[128];
				WideCharToMultiByte( CP_ACP, 0, m_szParam, -1, szTemp, 128, NULL, NULL );
				sprintf_s( szEnviName, "Envi\\%s.env", szTemp );

				bool bFind;
				CFileNameString szFullName = CEtResourceMng::GetInstance().GetFullName( szEnviName, &bFind );
				if( bFind )
					CDnWorld::GetInstance().InitializeEnviroment( szFullName.c_str(), CDnCamera::GetActiveCamera() );
				else {
					OutputScript( "Can't find environment file. [ %s ]", m_szParam );
				}
			}
			break;
		case CHANGE_DEBUG_MODE:
			{
				bool bPrevMode = s_bDialogMode;
				if( !_tcscmp( m_szParam, _T("1") ) ) s_bDialogMode = true;
				else if( !_tcscmp( m_szParam, _T("0") ) ) s_bDialogMode = false;
				if( bPrevMode == true && s_bDialogMode == false ) {
					ShowWindow( m_hWnd, SW_HIDE );
				}
				else if( bPrevMode == false && s_bDialogMode == true ) {
					ShowWindow( m_hWnd, SW_SHOW );
				}
			}
			break;
		case SHOW_COLL:
			{
				bool bShow=false;
				if( !_tcscmp( m_szParam, _T("1") ) ) bShow = true;
				else if( !_tcscmp( m_szParam, _T("0") ) ) bShow = false;

				CDnActor::SetAllMonsterDebugDrawing(bShow, bShow);
			}
			break;
		case RELOAD_ACTION:
			{
//				CEtActionCoreMng::GetInstance().Reload();
//				SendChatMsg( CHATTYPE_NORMAL, _T("/reloadaction") );
			}
			break;
	}
}

void CDnDebugTask::Show( bool bShow )
{
	m_bShow = bShow;
	if( s_bDialogMode )
		ShowWindow( m_hWnd, ( bShow == true ) ? SW_SHOW : SW_HIDE );
	else {
		m_fVertVelocity = 0.03f;
		m_fVertVelocityTemp = 0.f;
	}
}

void CDnDebugTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !s_bDialogMode ) {
		float fWidth = (float)CEtDevice::GetInstance().Width();
		float fHeight = (float)CEtDevice::GetInstance().Height();
		if( m_fVertVelocity > 0.f ) {
			m_fVertVelocityTemp += CalcMovement( m_fVertVelocity, fDelta, 0.1f, -0.1f, -0.076f );
			if( m_fVertVelocityTemp >= 0.6f ) {
				m_fVertVelocity = 0.f;
				m_fVertVelocityTemp = 0.6f;
			}
			CalcAccumulationValue();
		}
		else m_fVertVelocityTemp = 0.6f;

		EtVector2 vTemp[4] = { EtVector2( 0.f, 0.f ), EtVector2( 1.f, 0.f ), EtVector2( 1.f, m_fVertVelocityTemp ), EtVector2( 0.f, m_fVertVelocityTemp ) };
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[1], vTemp[2], 0x80404040 );
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[2], vTemp[3], 0x80404040 );

		EternityEngine::DrawLine2D( EtVector2( 1.f / fWidth * 10.f, m_fVertVelocityTemp - ( 1.f / fHeight * 20.f ) ), EtVector2( 1.f - ( 1.f / fWidth * 10.f ), m_fVertVelocityTemp - ( 1.f / fHeight * 20.f ) ), 0xFFFFFFFF );

		char szTemp[65535] = { 0, };

		for( int i=0; i<m_nCursorPos; i++ ) szTemp[i] = ' ';
		szTemp[m_nCursorPos] = '_';
		szTemp[m_nCursorPos+1] = 0;

		EternityEngine::DrawText2D( EtVector2( 1.f / fWidth * 15.f, m_fVertVelocityTemp - ( 1.f / fHeight * 18.f ) ), szTemp, 0xFFFFFFFF );

		memset( szTemp, 0, sizeof(szTemp) );
		WideCharToMultiByte( CP_ACP, 0, m_szCurString.c_str() , -1, szTemp, 65535, NULL, NULL );
		EternityEngine::DrawText2D( EtVector2( 1.f / fWidth * 15.f, m_fVertVelocityTemp - ( 1.f / fHeight * 18.f ) ), szTemp, 0xFFFFFFFF );

		memset( szTemp, 0, sizeof(szTemp) );
		WideCharToMultiByte( CP_ACP, 0, m_szAccumulationString.c_str() + m_nStartLength, -1, szTemp, m_nEndLength - m_nStartLength + 1, NULL, NULL );
		EternityEngine::DrawText2D( EtVector2( 1.f / fWidth * 15.f, ( 1.f / fHeight * 10.f ) ), szTemp, 0xFFFFFFFF );

		// Draw scroll.
		vTemp[0] = EtVector2( 1.f - ( 1.f / fWidth * 10.f ), ( 1.f / fHeight * 10.f ) );
		vTemp[1] = EtVector2( 1.f - ( 1.f / fWidth * 3.f ), ( 1.f / fHeight * 10.f ) );
		vTemp[2] = EtVector2( 1.f - ( 1.f / fWidth * 3.f ), m_fVertVelocityTemp - ( 1.f / fHeight * 30.f ) );
		vTemp[3] = EtVector2( 1.f - ( 1.f / fWidth * 10.f ), m_fVertVelocityTemp - ( 1.f / fHeight * 30.f ) );
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[1], vTemp[2], 0xBBCCCCCC );
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[2], vTemp[3], 0xBBCCCCCC );


		float fSize = m_fVertVelocityTemp - ( 1.f / fHeight * 30.f ) - ( 1.f / fHeight * 10.f ) - ( 1.f / fHeight * 20.f );
		float fTemp = fSize / ( m_nTotalLineCount - m_nMaxLineCount ) * m_nVertScrollPos;

		vTemp[0] = EtVector2( 1.f - ( 1.f / fWidth * 10.f ), ( 1.f / fHeight * 10.f ) + fTemp );
		vTemp[1] = EtVector2( 1.f - ( 1.f / fWidth * 3.f ), ( 1.f / fHeight * 10.f ) + fTemp );
		vTemp[2] = EtVector2( 1.f - ( 1.f / fWidth * 3.f ), ( 1.f / fHeight * 10.f ) + fTemp + ( 1.f / fHeight * 20.f ) );
		vTemp[3] = EtVector2( 1.f - ( 1.f / fWidth * 10.f ), ( 1.f / fHeight * 10.f ) + fTemp + ( 1.f / fHeight * 20.f ) );
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[1], vTemp[2], 0xFFEEFF22 );
		EternityEngine::DrawTriangle2D( vTemp[0], vTemp[2], vTemp[3], 0xFFEEFF22 );
	}
}

INT_PTR CALLBACK CDnDebugTask::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch( uMsg ) {
		case WM_COMMAND:
			switch( LOWORD(wParam) ) {
				case IDC_EDIT1:
					switch( HIWORD(wParam) ) {
						case 1024: // How should I handle the mask? -_-;
							{
								int nLength = GetWindowTextLength( GetDlgItem( hwndDlg, IDC_EDIT1 ) ) + 1;
								s_pThis->m_szString.resize(nLength-1);

								GetWindowText( GetDlgItem( hwndDlg, IDC_EDIT1 ), &s_pThis->m_szString[0], nLength );
							}
							break;
					}
					break;
				case IDC_EDIT2:
					{
						static int nPrevSize = 0;
						static TCHAR szPrevTemp[256] = { 0, };
						switch( HIWORD(wParam) ) {
							case EN_CHANGE:
								{
									TCHAR szTemp[256] = { 0, };
									int nLength = GetWindowTextLength( GetDlgItem( hwndDlg, IDC_EDIT2 ) );
									GetWindowText( GetDlgItem( hwndDlg, IDC_EDIT2 ), szTemp, nLength + 1 );

									if( _tcscmp( szPrevTemp, szTemp ) == NULL && nPrevSize == nLength ) {	// Return
										s_pThis->CheckDebugCommand( szTemp );
										SetWindowText( GetDlgItem( hwndDlg, IDC_EDIT2 ), _T("") );
									}
									else if( szTemp[nLength-1] == 9 ) {	// Tab
										s_pThis->OutputScript( "Tab!!!\n" );
									}

									_tcscpy_s( szPrevTemp, szTemp );
									nPrevSize = nLength;
								}
								break;
						}
					}
					break;
			}
			break;
			/*
		case WM_SETFOCUS:
			//CGlobalValue::GetInstance().ShowCursor( true );
			g_Cursor.SetCursor( CDnMouseCursor::NORMAL );
			break;
		case WM_KILLFOCUS:
			//CGlobalValue::GetInstance().ShowCursor( false );
			g_Cursor.SetCursor( CDnMouseCursor::NONE );
			break;
			*/
	}
	return FALSE;
}

void CDnDebugTask::OutputScript( char *szString, ... )
{
	if( !IsActive() ) return;
	TCHAR szBuf[65535], szTemp[65535];
	TCHAR szStringT[65535];
	va_list arg;

	va_start( arg, szString );

	MultiByteToWideChar( CP_ACP, 0, szString, -1, szStringT, 65535 );
	_vstprintf_s( szBuf, szStringT, arg );
	int nLength = (int)_tcslen( szBuf );
	int nPos = 0;
	for( int i=0; i<nLength; i++ ) {
		if( szBuf[i] == '\n' ) {
			szTemp[nPos] = '\r';
			++nPos;
		}
		szTemp[nPos] = szBuf[i];
		++nPos;
	}
	szTemp[nPos] = 0;

	m_szString = szTemp;

	if( m_bShow == true ) {
		if( s_bDialogMode ) {
			HWND hWnd = GetDlgItem( m_hWnd, IDC_EDIT1 );

			if( IsDlgButtonChecked( m_hWnd, IDC_CHECK1 ) == BST_UNCHECKED ) {
				SendMessage( hWnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)m_szString.c_str() );
			}
			else {
				int nMin, nMax;

				SendMessage( hWnd, EM_GETSEL, (WPARAM)&nMin, (LPARAM)&nMax );

				SETTEXTEX TextEx;
				TextEx.flags = ST_DEFAULT;
				TextEx.codepage = CP_ACP;
				SendMessage( hWnd, EM_SETTEXTEX, (WPARAM)&TextEx, (LPARAM)m_szString.c_str() );

				SendMessage( hWnd, EM_SETSEL, nMin, nMax );
				SendMessage( hWnd, EM_SCROLLCARET, 0, 0 );
			}
		}
		else {
			m_szAccumulationString += m_szString;

			CalcAccumulationValue();
		}
	}

	va_end( arg );
}

void CDnDebugTask::CalcAccumulationValue()
{
	float fWidth = (float)CEtDevice::GetInstance().Width();
	float fHeight = (float)CEtDevice::GetInstance().Height();

	m_nStartLength = m_nEndLength = 0;
	m_nMaxLineCount = (int)( ( fHeight * m_fVertVelocityTemp ) - ( 1.f / fHeight * 18.f ) - ( 1.f / fHeight * 10.f ) ) / 17;
	if( m_bScrollLock ) {
		int nLength = (int)m_szAccumulationString.size();
		int nLineCount = 0;
		for( int i=0; i<nLength; i++ ) {
			if( m_szAccumulationString[i] == '\r' ) {
				nLineCount++;
				if( nLineCount == m_nVertScrollPos ) {
					m_nStartLength = i + 2;
				}
				if( nLineCount == m_nVertScrollPos + m_nMaxLineCount ) {
					m_nEndLength = i;
				}
			}
		}
		if( m_nEndLength == 0 ) m_nEndLength = nLength;
		m_nTotalLineCount = nLineCount;
	}
	else {
		int nLength = (int)m_szAccumulationString.size();
		int nLineCount = 0;
		for( int i=nLength-1; i>=0; i-- ) {
			if( m_szAccumulationString[i] == '\r' ) {
				nLineCount++;
				if( nLineCount == m_nMaxLineCount + 1 ) {
					m_nStartLength = i + 2;
				}
			}
		}
		m_nEndLength = nLength;
		m_nTotalLineCount = nLineCount;
		m_nVertScrollPos = m_nTotalLineCount - m_nMaxLineCount;
		if( m_nVertScrollPos < 0 ) m_nVertScrollPos = 0;
	}
}

void CDnDebugTask::OutputScriptString( const char *szString, ... )
{
	if( !s_pThis ) return;
	char szBuf[1024];
	va_list arg;

	va_start( arg, szString );

	vsprintf_s( szBuf, szString, arg );
	s_pThis->OutputScript( szBuf );
}


void CDnDebugTask::CheckDebugCommand( TCHAR *szStr )
{
	TCHAR szCommand[64] = { 0, };
	int nLength = (int)_tcslen(szStr);
	for( int i=0; i<nLength; i++ ) {
		if( szStr[i] == ' ' ) break;
		szCommand[i] = szStr[i];
	}
	for( DWORD i=0; i<(int)DebugCmdLineIndexEnum_END; i++ ) {
		if( _tcsicmp( szCommand, g_DebugCmdLineDefineList[i].szString ) == NULL ) {
			memset( m_szParam, 0, sizeof(m_szParam) );
			int nCommandLength = (int)_tcslen(szCommand) + 1;
			for( int j=nCommandLength; j<nLength; j++ ) {
				m_szParam[j-nCommandLength] = szStr[j];
			}
//			memcpy( m_szParam, szStr + _tcslen(szCommand), _tcslen(szStr) - _tcslen(szCommand) );
			CommandDebug( g_DebugCmdLineDefineList[i].Index );
			return;
		}
	}
	// Unknown command
	if( nLength > 0 ) {
		OutputScript( "'%s' is unable command.\n", szCommand );
	}
}



BYTE g_DIK2VK[256]
= {
	-1,
	VK_ESCAPE,	// DIK_ESCAPE          0x01
	0x31,		// DIK_1               0x02
	0x32,		// DIK_2               0x03
	0x33,		// DIK_3               0x04
	0x34,		// DIK_4               0x05
	0x35,		// DIK_5               0x06
	0x36,		// DIK_6               0x07
	0x37,		// DIK_7               0x08
	0x38,		// DIK_8               0x09
	0x39,		// DIK_9               0x0A
	0x30,		// DIK_0               0x0B
	VK_OEM_MINUS,		// DIK_MINUS           0x0C
	VK_OEM_PLUS,	// DIK_EQUALS          0x0D
	VK_BACK,		// DIK_BACK            0x0E
	VK_TAB,	// DIK_TAB             0x0F
	0x51,	// DIK_Q               0x10
	0x57,	// DIK_W               0x11
	0x45,	// DIK_E               0x12
	0x52,	// DIK_R               0x13
	0x54,	// DIK_T               0x14
	0x59,	// DIK_Y               0x15
	0x55,	// DIK_U               0x16
	0x49,	// DIK_I               0x17
	0x4F,	// DIK_O               0x18
	0x50,	// DIK_P               0x19
	-1,		// DIK_LBRACKET        0x1A
	-1,		// DIK_RBRACKET        0x1B
	VK_RETURN,		// DIK_RETURN          0x1C
	VK_LCONTROL,	// DIK_LCONTROL        0x1D
	0x41,	// DIK_A               0x1E
	0x53,	// DIK_S               0x1F
	0x44,	// DIK_D               0x20
	0x46,	// DIK_F               0x21
	0x47,	// DIK_G               0x22
	0x48,	// DIK_H               0x23
	0x4A,	// DIK_J               0x24
	0x4B,	// DIK_K               0x25
	0x4C,	// DIK_L               0x26
	0x27,	// DIK_SEMICOLON       0x27
	0x28,	// DIK_APOSTROPHE      0x28
	-1,		// DIK_GRAVE           0x29
	VK_LSHIFT,		// DIK_LSHIFT          0x2A
	VK_OEM_5,		// DIK_BACKSLASH       0x2B
	0x5A,	// DIK_Z               0x2C
	0x58,	// DIK_X               0x2D
	0x43,	// DIK_C               0x2E
	0x56,	// DIK_V               0x2F
	0x42,	// DIK_B               0x30
	0x4E,	// DIK_N               0x31
	0x4D,	// DIK_M               0x32
	VK_OEM_COMMA,	// DIK_COMMA           0x33
	VK_OEM_PERIOD,	// DIK_PERIOD          0x34
	VK_OEM_2,	// DIK_SLASH           0x35
	VK_RSHIFT,	// DIK_RSHIFT          0x36
	VK_MULTIPLY,	// DIK_MULTIPLY        0x37
	VK_LMENU,	// DIK_LMENU           0x38
	VK_SPACE,	// DIK_SPACE           0x39
	VK_CAPITAL,	// DIK_CAPITAL         0x3A
	VK_F1,		// DIK_F1              0x3B
	VK_F2,		// DIK_F2              0x3C
	VK_F3,		// DIK_F3              0x3D
	VK_F4,		// DIK_F4              0x3E
	VK_F5,		// DIK_F5              0x3F
	VK_F6,		// DIK_F6              0x40
	VK_F7,		// DIK_F7              0x41
	VK_F8,		// DIK_F8              0x42
	VK_F9,		// DIK_F9              0x43
	VK_F10,		// DIK_F10             0x44
	VK_NUMLOCK,	// DIK_NUMLOCK         0x45
	VK_SCROLL,	// DIK_SCROLL          0x46
	VK_NUMPAD7,	// DIK_NUMPAD7         0x47
	VK_NUMPAD8,	// DIK_NUMPAD8         0x48
	VK_NUMPAD9,	// DIK_NUMPAD9         0x49
	VK_SUBTRACT,// DIK_SUBTRACT        0x4A
	VK_NUMPAD4,	// DIK_NUMPAD4         0x4B
	VK_NUMPAD5,	// DIK_NUMPAD5         0x4C
	VK_NUMPAD6,	// DIK_NUMPAD6         0x4D
	VK_ADD,		// DIK_ADD             0x4E
	VK_NUMPAD1,	// DIK_NUMPAD1         0x4F
	VK_NUMPAD2,	// DIK_NUMPAD2         0x50
	VK_NUMPAD3,	// DIK_NUMPAD3         0x51
	VK_NUMPAD0,	// DIK_NUMPAD0         0x52
	VK_DECIMAL,	// DIK_DECIMAL         0x53
	-1,	// DIK_OEM_102         0x56
	VK_F11,	// DIK_F11             0x57
	VK_F12,	// DIK_F12             0x58
	VK_F13,	// DIK_F13             0x64
	VK_F14,	// DIK_F14             0x65
	VK_F15,	// DIK_F15             0x66
	-1,		// DIK_KANA            0x70
	-1,		// DIK_ABNT_C1         0x73
	VK_CONVERT,	// DIK_CONVERT         0x79
	VK_NONCONVERT,	// DIK_NOCONVERT       0x7B
	-1,		// DIK_YEN             0x7D
	-1,	// DIK_ABNT_C2         0x7E
	VK_OEM_NEC_EQUAL,	// DIK_NUMPADEQUALS    0x8D
	-1,	// DIK_PREVTRACK       0x90
	-1,	// DIK_AT              0x91
	VK_OEM_1,	// DIK_COLON           0x92
	-1,		// DIK_UNDERLINE       0x93
	-1,		// DIK_KANJI           0x94
	-1,		// DIK_STOP            0x95
	-1,		// DIK_AX              0x96
	-1,		// DIK_UNLABELED       0x97
	-1,	// DIK_NEXTTRACK       0x99
	VK_RETURN,	// DIK_NUMPADENTER     0x9C
	VK_RCONTROL,	// DIK_RCONTROL        0x9D
	-1,	// DIK_MUTE            0xA0
	-1,	// DIK_CALCULATOR      0xA1
	-1,	// DIK_PLAYPAUSE       0xA2
	-1,	// DIK_MEDIASTOP       0xA4
	-1,	// DIK_VOLUMEDOWN      0xAE
	-1,	// DIK_VOLUMEUP        0xB0
	-1,	// DIK_WEBHOME         0xB2
	VK_OEM_COMMA,// DIK_NUMPADCOMMA     0xB3
	VK_DIVIDE,	// DIK_DIVIDE          0xB5
	-1,	// DIK_SYSRQ           0xB7
	VK_RMENU,		// DIK_RMENU           0xB8
	VK_PAUSE,	// DIK_PAUSE           0xC5
	VK_HOME,	// DIK_HOME            0xC7
	VK_UP,	// DIK_UP              0xC8
	VK_PRIOR,	// DIK_PRIOR           0xC9
	VK_LEFT,	// DIK_LEFT            0xCB
	VK_RIGHT,	// DIK_RIGHT           0xCD
	VK_END,	// DIK_END             0xCF
	VK_DOWN,	// DIK_DOWN            0xD0
	VK_NEXT,	// DIK_NEXT            0xD1
	VK_INSERT,	// DIK_INSERT          0xD2
	VK_DELETE,	// DIK_DELETE          0xD3
	VK_LWIN,	// DIK_LWIN            0xDB
	VK_RWIN,	// DIK_RWIN            0xDC
	VK_APPS,	// DIK_APPS            0xDD
	-1,	// DIK_POWER           0xDE
	-1,	// DIK_SLEEP           0xDF
	-1,	// DIK_WAKE            0xE3
	-1,	// DIK_WEBSEARCH       0xE5
	-1,		// DIK_WEBFAVORITES    0xE6
	-1,		// DIK_WEBREFRESH      0xE7
	-1,		// DIK_WEBSTOP         0xE8
	-1,		// DIK_WEBFORWARD      0xE9
	-1,		// DIK_WEBBACK         0xEA
	-1,		// DIK_MYCOMPUTER      0xEB
	-1,		// DIK_MAIL            0xEC
	-1,		// DIK_MEDIASELECT     0xED
};


void CDnDebugTask::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState == IR_KEY_DOWN ) {
		for( DWORD i=0; i<GetEventKeyCount(); i++ ) {
			BYTE cKey = GetEventKey(i);
			if( ( cKey >= DIK_1 && cKey <= DIK_EQUALS ) || 
				( cKey >= DIK_Q && cKey <= DIK_RBRACKET ) ||
				( cKey >= DIK_A && cKey <= DIK_APOSTROPHE ) ||
				( cKey >= DIK_BACKSLASH && cKey <= DIK_SLASH ) ||
				( cKey == DIK_SPACE ) ) {
					if( g_DIK2VK[cKey] >= 0x41 && g_DIK2VK[cKey] <= 0x58 && !(  GetKeyState( VK_CAPITAL ) & 0x0001 ) ) {
						m_szCurString.insert( m_szCurString.begin() + m_nCursorPos, g_DIK2VK[cKey] + 32 );
					}
					else m_szCurString.insert( m_szCurString.begin() + m_nCursorPos, g_DIK2VK[cKey] );
					m_nCursorPos++;
			}
			if( cKey == DIK_BACK ) {
				if( m_nCursorPos > 0 ) {
					m_nCursorPos--;
					m_szCurString.erase( m_szCurString.begin() + m_nCursorPos );
				}
			}
			if( cKey == DIK_LEFT ) {
				m_nCursorPos--;
				if( m_nCursorPos < 0 ) m_nCursorPos = 0;
			}
			if( cKey == DIK_RIGHT ) {
				m_nCursorPos++;
				if( m_nCursorPos >= (int)m_szCurString.size() ) m_nCursorPos = (int)m_szCurString.size();
			}
			if( cKey == DIK_UP ) {
				m_nVertScrollPos--;
				if( m_nVertScrollPos < 0 ) m_nVertScrollPos = 0;
				CalcAccumulationValue();
			}
			if( cKey == DIK_DOWN ) {
				m_nVertScrollPos++;
				int nTemp = 0;
				if( m_nTotalLineCount < m_nMaxLineCount ) nTemp = 0;
				else nTemp = m_nTotalLineCount - m_nMaxLineCount;
				if( m_nVertScrollPos > nTemp ) m_nVertScrollPos = nTemp;
				CalcAccumulationValue();
			}
			if( cKey == DIK_PGUP ) {
				m_nVertScrollPos -= m_nMaxLineCount;
				if( m_nVertScrollPos < 0 ) m_nVertScrollPos = 0;
				CalcAccumulationValue();
			}
			if( cKey == DIK_PGDN ) {
				m_nVertScrollPos += m_nMaxLineCount;
				int nTemp = 0;
				if( m_nTotalLineCount < m_nMaxLineCount ) nTemp = 0;
				else nTemp = m_nTotalLineCount - m_nMaxLineCount;
				if( m_nVertScrollPos > nTemp ) m_nVertScrollPos = nTemp;
				CalcAccumulationValue();
			}

			if( cKey == DIK_RETURN || cKey == DIK_NUMPADENTER ) {
				m_nCursorPos = 0;
				s_pThis->CheckDebugCommand( (TCHAR*)m_szCurString.c_str() );
				m_szCurString.clear();
			}
		}
	}
	if( IsPushKey( DIK_SCROLL ) ) {
		m_bScrollLock = !m_bScrollLock;
	}
}