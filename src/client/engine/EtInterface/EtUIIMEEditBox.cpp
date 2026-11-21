#include "StdAfx.h"
#include "EtUIIMEEditBox.h"
#include "EtUIDialog.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtUIIMEEditBox::CEtUIIMEEditBox( CEtUIDialog *pParent )
	: CEtUIEditBox( pParent ), m_bFirstVisibleOver( false )
{
	m_Property.UIType = UI_CONTROL_IMEEDITBOX;

	m_Property.IMEEditBoxProperty.fIndicatorSize = 0.0f;
	m_dwCompTargetColor = D3DCOLOR_ARGB( 255, 255, 255, 255 );
	m_dwCompTargetBkColor = D3DCOLOR_ARGB( 255, 150, 150, 150 );
	m_dwCompTargetNonColor = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	m_dwCompTargetNonBkColor = D3DCOLOR_ARGB( 255, 150, 150, 150 );

	m_bForceBottomPosition = false;
	m_nTempCompSize = 0;
}

CEtUIIMEEditBox::~CEtUIIMEEditBox(void)
{
}

void CEtUIIMEEditBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIEditBox::Initialize( pProperty );
	UpdateRects();
}

void CEtUIIMEEditBox::SetTemplate( int nTemplateIndex )
{
	CEtUIEditBox::SetTemplate( nTemplateIndex );

	if( m_Property.IMEEditBoxProperty.dwMaxChars == 0 ) 
		m_Property.IMEEditBoxProperty.dwMaxChars = 256;

	SetMaxEditLength( m_Property.IMEEditBoxProperty.dwMaxChars );
}

bool CEtUIIMEEditBox::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() ||  !IsKeyLock() )
	{
		return false;
	}

#if defined(DEBUG) || defined(_DEBUG)
	assert( m_bIMEStaticMsgProcCalled && L"To fix, call CEtUIDialogResourceManager::MsgProc() first" );
#endif

	static LPARAM lAlt = 0x80000000, lCtrl = 0x80000000, lShift = 0x80000000;
	bool bpbTrappedData, *pbTrapped;
	HIMC hImc;

	pbTrapped = &bpbTrappedData;
	*pbTrapped = false;
	if( !s_bEnableImeSystem )
	{
		return CEtUIEditBox::MsgProc( uMsg, wParam, lParam );
	}

	// Reference material.
	// During IME composition, wParam of WM_KEYDOWN is always VK_PROCESSKEY.
	// If the original vk value is needed, it can be obtained via ImmGetVirtualKey, though
	// TranslateMessage must not have been called yet.
	// However, the code below may look like it excludes everything except backspace from the default procedure to prevent IME processing, but
	// Even if we do this, the IME still processes all messages internally (i.e., composition is still happening).
	// Ignoring messages like this does not seem to prevent IME composition.
	//if( m_bFirstVisibleOver )
	//{
	//	if( uMsg == WM_KEYDOWN && wParam == VK_PROCESSKEY )
	//	{
	//		UINT nKey = ImmGetVirtualKey( m_pParent->GetHWnd() );
	//		if( nKey == VK_BACK )
	//			m_bFirstVisibleOver = false;
	//		else
	//			return true;
	//	}
	//}

	switch( uMsg )
	{
	case WM_IME_COMPOSITION:
		{
			// Note : Check here because m_Buffer is not a static variable. 
			//		If at max length, do not compose characters anymore.
			//
			if( m_Buffer.IsMaxEditBoxLength() )
			{
				PostMessage( GetFocus(), WM_IME_ENDCOMPOSITION, 0, 0 );
				break;
			}

			// Note: Prevent the composing string from being too long and pushing the completed string out of view.
			// Otherwise m_nFirstVisible would exceed the string length.
			// Only check when characters other than backspace are added; otherwise, when full, deletion fails and input freezes.
			if( m_bFirstVisibleOver )
			{
				HIMC hImc;
				hImc = _ImmGetContext( m_pParent->GetHWnd() );
				_ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
				_ImmReleaseContext( m_pParent->GetHWnd(), hImc );
				m_bFirstVisibleOver = false;
				break;

				// Tried various approaches to resolve #14215 (keep state instead of resetting as above when m_bFirstVisibleOver is set).
				// 1. First, the reason it resets like this is that transitioning to m_bFirstVisibleOver state itself seems undesirable,
				// In the PlaceCaret check that sets m_bFirstVisibleOver to true, tried calling ImmSetCompositionString to adjust the length of the composing string, but
				// No matter what is set, the function returns FALSE.
				// Even searching Google, many people say it doesn't work, but I couldn't find anyone writing how to fix it.
				// 2. Tried to enforce size limit using ImmProcessKey or ImmLockIMC, but the function call failed.
				// 3. Decided to block the message itself since several functions do not work properly.
				// So tried to ignore WM_IME_COMPOSITION by not passing it to the default procedure, however,
				// Even without going to the default procedure, IME receives messages internally, so getting the composition result returns the string still being composed.
				// That is, failed to control the max composition length even by not sending the message.
				// (If you want to verify IME messages are handled internally even without forwarding to the default procedure,
				//  If return true when m_bFirstVisibleOver, typing continuously adds no more chars to the composition window, however,
				//  After Alt-Tabbing out and back, wm_paint refreshes and all pressed characters appear composed.)
				// 4. After further investigation, blocking messages to the IME was only possible via keyboard hooking.
				// 5. In most cases, length limit is not applied to characters being composed, only when the composition Result arrives.
				//
				// Using keyboard hooking just for length limiting feels excessive
				// Keep the current approach for now until a better improvement is found. 
				// Or just force-output the composed character so far as Result..
			}

			LONG lRet;  
			WCHAR wszCompStr[ MAX_COMPSTRING_SIZE ]={0};

			*pbTrapped = true;
			hImc = _ImmGetContext( m_pParent->GetHWnd() );
			if( hImc == NULL )
			{
				break;
			}

			if( lParam & GCS_CURSORPOS )
			{
				s_nCompCaret = _ImmGetCompositionStringW( hImc, GCS_CURSORPOS, NULL, 0 );
				if( s_nCompCaret < 0 )
				{
					s_nCompCaret = 0;
				}
			}

			if( lParam & GCS_RESULTSTR )
			{
				lRet = _ImmGetCompositionStringW( hImc, GCS_RESULTSTR, wszCompStr, sizeof( wszCompStr ) );
				if( lRet > 0 )
				{
					lRet /= sizeof( WCHAR );
					wszCompStr[ lRet ] = 0;
					TruncateCompString( false, ( int )wcslen( wszCompStr ) );
					s_CompString.SetText( wszCompStr );

					if( CEtUIIME::s_bRESULTSTR_NotSendComp == false )
					{
						SendCompString();
					}
					else
					{
						GetEtDevice()->SetDialogBoxMode(false);
						CEtUIIME::s_bRESULTSTR_NotSendComp = true;
					}

					ResetCompositionString();
				}
			}

			if( lParam & GCS_COMPSTR )
			{
				lRet = _ImmGetCompositionStringW( hImc, GCS_COMPSTR, wszCompStr, sizeof( wszCompStr ) );
				if( lRet > 0 )
				{
					int i;

					lRet /= sizeof( WCHAR );
					wszCompStr[ lRet ] = 0;
					TruncateCompString( false, ( int )wcslen( wszCompStr ) );
					s_CompString.SetText( wszCompStr );
					m_pParent->ProcessCommand(EVENT_EDITBOX_IME_CHANGE, true, this, WM_IME_COMPOSITION);
					
					if( ( GetLanguage() == LANG_CHT ) && ( !GetImeId() ) )
					{
						if( lstrlenW( s_CompString.GetBuffer() ) )
						{
							s_CandList.dwCount = 4;             
							s_CandList.dwSelection = ( DWORD ) - 1;
							for( i = 3; i >= 0; i-- )
							{
								if( i > lstrlenW( s_CompString.GetBuffer() ) - 1 )
								{
									s_CandList.awszCandidate[ i ][ 0 ] = 0;
								}
								else
								{
									s_CandList.awszCandidate[ i ][ 0 ] = s_CompString[ i ];
									s_CandList.awszCandidate[ i ][ 1 ] = 0;
								}
							}
							s_CandList.dwPageSize = MAX_CANDLIST;
							memset( ( void * )s_CompString.GetBuffer(), 0, sizeof( WCHAR ) * 4  );
							s_bShowReadingWindow = true;
							GetReadingWindowOrientation( 0 );
							if( s_bHorizontalReading )
							{
								s_CandList.nReadingError = -1;
								s_wszReadingString[ 0 ] = 0;
								for( i = 0; i < ( int )s_CandList.dwCount; i++ )
								{
									if( s_CandList.dwSelection == i )
									{
										s_CandList.nReadingError = lstrlenW( s_wszReadingString );
									}
									wcscat_s( s_wszReadingString, 32, s_CandList.awszCandidate[ i ] );
								}
							}
						}
						else
						{
							s_CandList.dwCount = 0;
							s_bShowReadingWindow = false;
						}
					}

					if( s_bInsertOnType )
					{
						int nCount;

						SendCompString();
						nCount = lstrlenW( s_CompString.GetBuffer() + s_nCompCaret );
						for( i = 0; i < nCount; i++ )
						{
							SendMessage( m_pParent->GetHWnd(), WM_KEYDOWN, VK_LEFT, 0 );
						}
						SendMessage( m_pParent->GetHWnd(), WM_KEYUP, VK_LEFT, 0 );
					}
				}

				ResetCaretBlink();

				// Note : Adjust string when it exceeds control size during Hangul composition.
				//		Currently tested only with Korean; Chinese and Japanese need to be checked later.
				//
				//		The position must be here.
				PlaceCaret( (int)lRet );
			}

			if( lParam & GCS_COMPATTR )
			{
				lRet = _ImmGetCompositionStringW( hImc, GCS_COMPATTR, s_abyCompStringAttr, sizeof( s_abyCompStringAttr ) );
				if( lRet > 0 )
				{
					s_abyCompStringAttr[lRet] = 0;
				}
			}

			if( CEtUIIME::s_bProcess_GCS_COMPCLAUSE )
			{
				if( lParam & GCS_COMPCLAUSE )
				{
					lRet = _ImmGetCompositionStringW( hImc, GCS_COMPCLAUSE, s_adwCompStringClause, sizeof( s_adwCompStringClause ) );
					s_adwCompStringClause[ lRet / sizeof( DWORD ) ] = 0;
				}
			}

			if( CEtUIIME::s_bRESULTSTR_NotSendComp == true )
			{
				// Adjust composition window position.
				float fX1 = CEtUIIMEEditBox::s_CurrentCaretPos.fX * m_pParent->GetScreenWidth();
				float fX2 = CEtUIIMEEditBox::s_CurrentCaretPos.fY * m_pParent->GetScreenHeight();

				RECT rcWindow;
				GetWindowRect( m_pParent->GetHWnd(), &rcWindow );

				POINT pt;
				pt.x = (long)fX1 - rcWindow.left;
				pt.y = (long)fX2 - rcWindow.top;

				// In fullscreen mode, rendering ignores the window caption and covers the original input box.
				// So force it down by 20 pixels.
				if( !(GetWindowLong( m_pParent->GetHWnd(), GWL_STYLE ) & WS_CAPTION) )
					pt.y += 20;

				ClientToScreen(m_pParent->GetHWnd(), &pt);

				COMPOSITIONFORM cf;
				cf.dwStyle = CFS_POINT;
				cf.ptCurrentPos.x = pt.x;
				cf.ptCurrentPos.y = pt.y;
				ImmSetCompositionWindow(hImc, &cf);
			}

			_ImmReleaseContext( m_pParent->GetHWnd(), hImc );

			if( CEtUIIME::s_bRESULTSTR_NotSendComp == true )
			{
				return CEtUIEditBox::MsgProc( uMsg, wParam, lParam );
			}
		}
		break;

	case WM_IME_ENDCOMPOSITION:
		TruncateCompString();
		ResetCompositionString();
		s_bHideCaret = false;
		s_bShowReadingWindow = false;
		m_pParent->ProcessCommand(EVENT_EDITBOX_IME_CHANGE, true, this, WM_IME_ENDCOMPOSITION);
		break;

	case WM_IME_NOTIFY:
		switch( wParam )
		{
		case IMN_SETCONVERSIONMODE:
		case IMN_SETOPENSTATUS:
			CheckToggleState();
			break;

		case IMN_OPENCANDIDATE:
		case IMN_CHANGECANDIDATE:
			{
				int i, j;
				LPCANDIDATELIST lpCandList;
				DWORD dwLenRequired;

				s_CandList.bShowWindow = true;
				*pbTrapped = true;
				hImc = _ImmGetContext( m_pParent->GetHWnd() );
				if( hImc == NULL )
				{
					break;
				}
				lpCandList = NULL;
				s_bShowReadingWindow = false;
				dwLenRequired = _ImmGetCandidateListW( hImc, 0, NULL, 0 );
				if( dwLenRequired )
				{
					lpCandList = ( LPCANDIDATELIST )HeapAlloc( GetProcessHeap(), 0, dwLenRequired );
					dwLenRequired = _ImmGetCandidateListW( hImc, 0, lpCandList, dwLenRequired );
				}

				if( lpCandList )
				{
					int nPageTopIndex;

					s_CandList.dwSelection = lpCandList->dwSelection;
					s_CandList.dwCount = lpCandList->dwCount;
					nPageTopIndex = 0;
					s_CandList.dwPageSize = min( lpCandList->dwPageSize, MAX_CANDLIST );
					if( GetPrimaryLanguage() == LANG_JAPANESE )
					{
						nPageTopIndex = ( s_CandList.dwSelection / s_CandList.dwPageSize ) * s_CandList.dwPageSize;
					}
					else
					{
						nPageTopIndex = lpCandList->dwPageStart;
					}

					if( GetLanguage() == LANG_CHS && !GetImeId() )
					{
						s_CandList.dwSelection = ( DWORD ) - 1;
					}
					else
					{
						s_CandList.dwSelection = s_CandList.dwSelection - nPageTopIndex;
					}

					memset( s_CandList.awszCandidate, 0, sizeof(s_CandList.awszCandidate) );
					for( i = nPageTopIndex, j = 0; i < ( int )lpCandList->dwCount && j < ( int )s_CandList.dwPageSize; i++, j++ )
					{
						LPWSTR pwsz;
						WCHAR *pwszNewCand;

						pwsz = s_CandList.awszCandidate[ j ];
						*pwsz = ( WCHAR )( L'0' + ( ( j + 1 ) % 10 ) );
						pwsz++;
						if( s_bVerticalCand )
						{
							*pwsz = L' ';
							pwsz++;
						}
						pwszNewCand = ( LPWSTR )( ( BYTE * )lpCandList + lpCandList->dwOffset[ i ] );
						while( *pwszNewCand )
						{
							*pwsz = *pwszNewCand;
							pwsz++;
							pwszNewCand++;
						}
						if( !s_bVerticalCand )
						{
							*pwsz = L' ';
							pwsz++;
						}
						*pwsz = 0;
					}

					s_CandList.dwCount = lpCandList->dwCount - lpCandList->dwPageStart;
					if( s_CandList.dwCount > lpCandList->dwPageSize )
					{
						s_CandList.dwCount = lpCandList->dwPageSize;
					}

					HeapFree( GetProcessHeap(), 0, lpCandList );
					_ImmReleaseContext( m_pParent->GetHWnd(), hImc );

					if( GetPrimaryLanguage() == LANG_KOREAN || GetLanguage() == LANG_CHT && !GetImeId() )
					{
						s_CandList.dwSelection = ( DWORD ) - 1;
					}

					if( !s_bVerticalCand )
					{
						WCHAR wszCand[ 256 ] = L"";

						s_CandList.nFirstSelected = 0;
						s_CandList.nHoriSelectedLen = 0;
						for( i = 0; i < MAX_CANDLIST; i++ )
						{
							WCHAR wszEntry[ 32 ];

							if( s_CandList.awszCandidate[ i ][ 0 ] == L'\0' )
							{
								break;
							}
							swprintf_s( wszEntry, 32, L"%s ", s_CandList.awszCandidate[ i ] );
							if( i == s_CandList.dwSelection )
							{
								s_CandList.nFirstSelected = lstrlenW( wszCand );
								s_CandList.nHoriSelectedLen = lstrlenW( wszEntry ) - 1;
							}
							wcscat_s( wszCand, 256, wszEntry );
						}
						wszCand[ lstrlenW( wszCand ) - 1 ] = L'\0';
						s_CandList.HoriCand.SetText( wszCand );
					}
				}
				break;
			}

		case IMN_CLOSECANDIDATE:
			{
				s_CandList.bShowWindow = false;
				if( !s_bShowReadingWindow )
				{
					s_CandList.dwCount = 0;
					memset( s_CandList.awszCandidate, 0, sizeof( s_CandList.awszCandidate ) );
				}
				*pbTrapped = true;
				break;
			}

		case IMN_PRIVATE:
			{
				DWORD dwId;

				if( !s_CandList.bShowWindow )
				{
					GetPrivateReadingString();
				}

				dwId = GetImeId();
				switch( dwId )
				{
					case IMEID_CHT_VER42:
					case IMEID_CHT_VER43:
					case IMEID_CHT_VER44:
					case IMEID_CHS_VER41:
					case IMEID_CHS_VER42:
						if( ( lParam == 1 ) || ( lParam == 2 ) )
						{
							*pbTrapped = true;
						}
						break;

					case IMEID_CHT_VER50:
					case IMEID_CHT_VER51:
					case IMEID_CHT_VER52:
					case IMEID_CHT_VER60:
					case IMEID_CHS_VER53:
						if( (lParam == 16) || (lParam == 17) || (lParam == 26) || (lParam == 27) || (lParam == 28) )
						{
							*pbTrapped = true;
						}
						break;
				}
			}
			break;

		default:
			*pbTrapped = true;
			break;
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if( !( lAlt & 0x80000000 ) && wParam != VK_MENU && ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_MENU, ( lAlt & 0x01ff0000 ) | 0xC0000001 );
		}   
		else if( !( lCtrl & 0x80000000 ) && wParam != VK_CONTROL && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_CONTROL, ( lCtrl & 0x01ff0000 ) | 0xC0000001 );
		}
		else if( !( lShift & 0x80000000 ) && wParam != VK_SHIFT && ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_SHIFT, ( lShift & 0x01ff0000 ) | 0xC0000001 );
		}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		switch ( wParam )
		{
			case VK_MENU:
				lAlt = lParam;
				break;
			case VK_SHIFT:
				lShift = lParam;
				break;
			case VK_CONTROL:
				lCtrl = lParam;
				break;
		}

	default:
		return CEtUIEditBox::MsgProc( uMsg, wParam, lParam );

	}  

	return *pbTrapped;
}

bool CEtUIIMEEditBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() || !IsKeyLock() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			float fFontHeight = m_pParent->GetFontHeight( m_Template.m_vecElement[5].nFontIndex, m_Template.m_vecElement[5].nFontHeight );
			//ID3DXFont *pFont = m_pParent->GetFont( m_Template.m_vecElement[5].nFontIndex, m_Template.m_vecElement[5].nFontHeight );
			int nCompStrWidth = 0;
			CEtFontMng& FontMng = CEtFontMng::GetInstance();
			SUIElement* pElement = GetElement( 0 );
			nCompStrWidth = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_CompString.GetTextSize() );
			//s_CompString.CPtoX( s_CompString.GetTextSize(), FALSE, &nCompStrWidth );

			if( ( s_fCompStringX <= fX ) && ( s_fCompStringY <= fY ) &&
				( s_fCompStringX + nCompStrWidth / m_pParent->GetScreenWidth() > fX ) && ( s_fCompStringY + fFontHeight > fY ) )
			{
				HIMC hImc;
				int nCharBodyHit, nCharHit;
				int nTrail;

				nCharBodyHit = FontMng.GetCaretFromCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
														   (int)((fX - s_fCompStringX) * m_pParent->GetScreenWidth()), nTrail );
				//s_CompString.XtoCP( ( int )( ( fX - s_fCompStringX ) * m_pParent->GetScreenWidth() ), &nCharBodyHit, &nTrail );
				if( ( nTrail ) && ( nCharBodyHit < s_CompString.GetTextSize() ) )
				{
					nCharHit = nCharBodyHit + 1;
				}
				else
				{
					nCharHit = nCharBodyHit;
				}

				hImc = _ImmGetContext( m_pParent->GetHWnd() );
				if( !hImc )
				{
					return true;
				}

				_ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 );
				_ImmReleaseContext( m_pParent->GetHWnd(), hImc );

				switch( GetPrimaryLanguage() )
				{
				case LANG_JAPANESE:
					if( s_nFirstTargetConv != -1 )
					{
						int nClauseClicked, nClauseSelected, nSendCount;
						BYTE byVirtKey;

						nClauseClicked = 0;
						while( ( int )s_adwCompStringClause[ nClauseClicked + 1 ] <= nCharBodyHit )
						{
							nClauseClicked++;
						}

						nClauseSelected = 0;
						while( ( int )s_adwCompStringClause[ nClauseSelected + 1 ] <= s_nFirstTargetConv )
						{
							nClauseSelected++;
						}

						if( nClauseClicked > nClauseSelected )
						{
							byVirtKey = VK_RIGHT;
						}
						else
						{
							byVirtKey = VK_LEFT;
						}
						nSendCount = abs( nClauseClicked - nClauseSelected );
						while( nSendCount-- > 0 )
						{
							SendKey( byVirtKey );
						}
						return true;
					}
				case LANG_CHINESE:
					{
						BYTE byVirtKey;
						int nSendCount;

						if( nCharHit > s_nCompCaret )
						{
							byVirtKey = VK_RIGHT;
						}
						else
						{
							byVirtKey = VK_LEFT;
						}
						nSendCount = abs( nCharHit - s_nCompCaret );
						while( nSendCount-- > 0 )
						{
							SendKey( byVirtKey );
						}
						break;
					}
				}
				return true;
			}

			if( ( s_CandList.bShowWindow ) && ( s_CandList.CandidateCoord.IsInside( fX, fY ) ) )
			{
				if( s_bVerticalCand )
				{
					int nRow;

					nRow = ( int )( ( fY - s_CandList.CandidateCoord.fY ) / fFontHeight );

					if( nRow < ( int )s_CandList.dwCount )
					{
						switch( GetPrimaryLanguage() )
						{
							case LANG_CHINESE:
							case LANG_KOREAN:
								SendKey( ( BYTE ) ( '0' + nRow + 1 ) );
								break;

							case LANG_JAPANESE:
							{
								BYTE byVirtKey;
								int nNumToHit, nStrike;

								if( nRow > ( int )s_CandList.dwSelection )
								{
									byVirtKey = VK_DOWN;
								}
								else
								{
									byVirtKey = VK_UP;
								}
								nNumToHit = abs( ( int) ( nRow - s_CandList.dwSelection ) );
								for( nStrike = 0; nStrike < nNumToHit; nStrike++ )
								{
									SendKey( byVirtKey );
								}
								SendKey( VK_RIGHT );
								SendKey( VK_LEFT );
								break;
							}
						}
					}
				} 
				else
				{
					int i, nCharHit, nTrail, nCandidate, nEntryStart;

					nCharHit = FontMng.GetCaretFromCaretPos( s_CandList.HoriCand.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
														   (int)((fX - s_CandList.CandidateCoord.fX) * m_pParent->GetScreenWidth()), nTrail );
					//s_CandList.HoriCand.XtoCP( ( int )( ( fX - s_CandList.CandidateCoord.fX ) * m_pParent->GetScreenWidth() ), &nCharHit, &nTrail );

					nCandidate = s_CandList.dwCount - 1;
					nEntryStart = 0;
					for( i = 0; i < ( int )s_CandList.dwCount; i++ )
					{
						if( nCharHit >= nEntryStart )
						{
							nEntryStart += lstrlenW( s_CandList.awszCandidate[ i ] ) + 1;
						} 
						else
						{
							nCandidate = i - 1;
							break;
						}
					}

					switch( GetPrimaryLanguage() )
					{
						case LANG_CHINESE:
						case LANG_KOREAN:
							SendKey( ( BYTE ) ( '0' + nCandidate + 1 ) );
							break;
					}
				}
				return true;
			}
		}
	}

	return CEtUIEditBox::HandleMouse( uMsg, fX, fY, wParam, lParam );
}

void CEtUIIMEEditBox::Focus( bool fFocus )
{
	CEtUIEditBox::Focus( fFocus );

	// For some Chinese IMEs, unless forcibly blocked as below
	// Intercepts all English characters entering the default procedure and pops up the candidate window.
	// 
	// It might not be used in the domestic build because the Hangul/English key toggle doesn't work when unfocused, but
	// In overseas builds, the provided IME must be used, and the code below must be used to prevent the candidate window from appearing without focus.
	//
	// Since EtInterface has no per-country defines, it receives and uses a static bool value set from outside.
	if( CEtUIIME::s_bDisableIMEonKillFocus )
	{
		if( fFocus )
		{
			HIMC hImc;

			if( s_bEnableImeSystem )
			{
				_ImmAssociateContext( m_pParent->GetHWnd(), s_hImcDef );
				CheckToggleState();
			} 
			else
			{
				_ImmAssociateContext( m_pParent->GetHWnd(), NULL );
			}
			hImc = _ImmGetContext( m_pParent->GetHWnd() );
			if( hImc != NULL ) 
			{
				if( !s_bEnableImeSystem )
				{
					s_ImeState = IMEUI_STATE_OFF;
				}
				_ImmReleaseContext( m_pParent->GetHWnd(), hImc );
				CheckToggleState();
			}
		}
		else
		{
			FinalizeString( false );
			_ImmAssociateContext( m_pParent->GetHWnd(), NULL );
		}
	}
}

void CEtUIIMEEditBox::PumpMessage()
{
	MSG msg;

	while( PeekMessageW( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if( !GetMessageW( &msg, NULL, 0, 0 ) )
		{
			PostQuitMessage( (int)msg.wParam );
			return;
		}
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
}

void CEtUIIMEEditBox::UpdateRects()
{
	float fTempWidth;

	fTempWidth = m_Property.UICoord.fWidth;
	if( m_Property.IMEEditBoxProperty.fIndicatorSize > 0.0f )
	{
		m_Property.UICoord.fWidth -= m_Property.IMEEditBoxProperty.fIndicatorSize + m_Property.IMEEditBoxProperty.fBorder * 2;
	}

	CEtUIEditBox::UpdateRects();

	m_IndicatorCoord.SetCoord( m_Property.UICoord.Right(), m_Property.UICoord.fY, m_Property.IMEEditBoxProperty.fIndicatorSize + m_Property.IMEEditBoxProperty.fBorder * 2, m_Property.UICoord.fHeight );
	m_Property.UICoord.fWidth = fTempWidth;
}

void CEtUIIMEEditBox::Show( bool bShow )
{
	CEtUIControl::Show( bShow );
	s_bHideCaret = false;
}

void CEtUIIMEEditBox::Render( float fElapsedTime )
{
	if( !IsShow() )
	{
		return;
	}

/*	if( m_Property.IMEEditBoxProperty.fIndicatorSize <= 0.0f )
	{
		int i;

		for( i = 0; i < 5; i++ )
		{
			SUICoord Coord;

			Coord.SetCoord( 0.0f, 0.0f, 0.1f, 0.1f );
			m_pParent->CalcTextRect( s_aszIndicator[ i ], &m_Template.m_vecElement[ 5 ], Coord );
			m_Property.IMEEditBoxProperty.fIndicatorSize = max( m_Property.IMEEditBoxProperty.fIndicatorSize, Coord.fWidth );
		}
		UpdateRects();
	}*/
	SUIElement *pElement;
	pElement = GetElement(0);

	if( pElement )
	{
		m_fCompositionWidth = 0.0f;
		if( m_bFocus && s_bHideCaret )
		{
			SUICoord CompCoord;
			m_pParent->CalcTextRect( s_CompString.GetBuffer(), pElement, CompCoord );
			m_fCompositionWidth = CompCoord.fWidth;
		}

		CEtUIEditBox::Render( fElapsedTime );

		//ID3DXFont *pFont = m_pParent->GetFont( pElement->nFontIndex, pElement->nFontHeight );
		//s_CompString.SetFont( pFont );
		//s_CandList.HoriCand.SetFont( pFont );
	}

	if( CEtUIIME::s_bDisableIMEonIMEControl )
		return;

	if( m_bFocus )
	{
		RenderIndicator( fElapsedTime );

		if( s_bHideCaret )
			RenderComposition( fElapsedTime );

		if( s_bShowReadingWindow )
		{
			RenderCandidateReadingWindow( fElapsedTime, true );
		}
		else if( s_CandList.bShowWindow )
		{
			RenderCandidateReadingWindow( fElapsedTime, false );
		}
	}
}

void CEtUIIMEEditBox::RenderCandidateReadingWindow( float fElapsedTime, bool bReading )
{
	UINT i, nNumEntries;
	SUICoord Coord;
	DWORD dwTextColor, dwTextBkColor, dwSelTextColor, dwSelBkColor;
	int nX, nXFirst, nXComp;

	nNumEntries = MAX_CANDLIST;
	if( bReading )
	{
		nNumEntries = 4;
	}

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	SUIElement* pElement = GetElement( 0 );
 	nX = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nCaret );
	nXFirst = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
	//m_Buffer.CPtoX( m_nCaret, FALSE, &nX );
	//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

	if( bReading )
	{
		dwTextColor = m_Property.IMEEditBoxProperty.dwReadingTextColor;
		dwTextBkColor = m_Property.IMEEditBoxProperty.dwReadingBackColor;
		dwSelTextColor = m_Property.IMEEditBoxProperty.dwReadingSelTextColor;
		dwSelBkColor = m_Property.IMEEditBoxProperty.dwReadingSelBackColor;
	} 
	else
	{
		dwTextColor = m_Property.IMEEditBoxProperty.dwCandiTextColor;
		dwTextBkColor = m_Property.IMEEditBoxProperty.dwCandiBackColor;
		dwSelTextColor = m_Property.IMEEditBoxProperty.dwCandiSelTextColor;
		dwSelBkColor = m_Property.IMEEditBoxProperty.dwCandiSelBackColor;
	}

	if( ( GetLanguage() == LANG_CHT ) && ( !GetImeId() ) )
	{
		nXComp = 0;
	}
	else
	{
		if( GetPrimaryLanguage() == LANG_JAPANESE )
		{
			nXComp = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_nFirstTargetConv );
			//s_CompString.CPtoX( s_nFirstTargetConv, FALSE, &nXComp );
		}
		else
		{
			nXComp = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_nCompCaret );
			//s_CompString.CPtoX( s_nCompCaret, FALSE, &nXComp );
		}
	}

	//SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	float fWidthRequired = 0.0f;
	float fHeightRequired = 0.0f;
	float fSingleLineHeight = 0.0f;

	if( ( s_bVerticalCand && !bReading ) || ( !s_bHorizontalReading && bReading ) )
	{
		for( i = 0; i < nNumEntries; i++ )
		{
			if( s_CandList.awszCandidate[ i ][ 0 ] == L'\0' )
			{
				break;
			}
			Coord.SetCoord( 0.0f, 0.0f, 0.0f, 0.0f );
			m_pParent->CalcTextRect( s_CandList.awszCandidate[i], pElement, Coord );
			fWidthRequired = max( fWidthRequired, Coord.fWidth );
			fSingleLineHeight = max( fSingleLineHeight, Coord.fHeight );
		}
		fHeightRequired = fSingleLineHeight * nNumEntries;
	} 
	else
	{
		Coord.SetCoord( 0.0f, 0.0f, 0.0f, 0.0f );
		if( bReading )
		{
			m_pParent->CalcTextRect( s_wszReadingString, pElement, Coord );
		}
		else
		{
			m_pParent->CalcTextRect( s_CandList.HoriCand.GetBuffer(), pElement, Coord );
		}
		// Leave some padding so it is not drawn too tightly
		Coord.fWidth += 0.01f;
		Coord.fHeight += 0.01f;
		fWidthRequired = Coord.fWidth;;
		fSingleLineHeight = Coord.fHeight;
		fHeightRequired = Coord.fHeight;
	}

	bool bHasPosition = false;

	Coord.SetCoord( s_fCompStringX + FloatXCoord( nXComp ), s_fCompStringY + m_TextCoord.fHeight, 
		fWidthRequired, fHeightRequired );
	if( Coord.Right() > m_pParent->Width() )
	{
		Coord.fX -= Coord.Right() - m_pParent->Width();
		Coord.fWidth = m_pParent->Width() - Coord.fX;
	}
	if( Coord.Bottom() <= m_pParent->Height() )
	{
		bHasPosition = true;
	}
	else if( m_bForceBottomPosition )
	{
		bHasPosition = true;
	}

	if( !bHasPosition )
	{
		Coord.SetCoord( s_fCompStringX + FloatXCoord( nXComp ), s_fCompStringY - fHeightRequired, fWidthRequired, fHeightRequired );
		if( Coord.Right() > m_pParent->Width() )
		{
			Coord.fX -= Coord.Right() - m_pParent->Width();
			Coord.fWidth = m_pParent->Width();
		}
		if( Coord.fY >= 0.0f )
		{
			bHasPosition = true;
		}
	}

	if( !bHasPosition )
	{
		int nXCompTrail;

		nXCompTrail = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_nCompCaret );
		//s_CompString.CPtoX( s_nCompCaret, TRUE, &nXCompTrail );
		Coord.SetCoord( s_fCompStringX + FloatXCoord( nXCompTrail ), 0.0f, fWidthRequired, fHeightRequired );
		if( Coord.Right() <= m_pParent->Width() )
		{
			bHasPosition = true;
		}
	}

	if( !bHasPosition )
	{
		Coord.SetCoord( s_fCompStringX + FloatXCoord( nXComp ) - fWidthRequired, 0.0f, fWidthRequired, fHeightRequired );
		if( Coord.Right() >= 0.0f )
		{
			bHasPosition = true;
		}
	}

	if( !bHasPosition )
	{
		Coord.fX = 0.0f;
		Coord.fWidth = fWidthRequired;
	}

	if( !bReading )
	{
		s_CandList.CandidateCoord = Coord;
	}

	m_pParent->DrawRect( Coord, dwTextBkColor );
	if( ( s_bVerticalCand && !bReading ) || ( !s_bHorizontalReading && bReading ) )
	{
		for( i = 0; i < nNumEntries; i++ )
		{
			Coord.fHeight = fSingleLineHeight;
			if( s_CandList.dwSelection == i )
			{
				m_pParent->DrawRect( Coord, dwSelBkColor );
				m_pParent->DrawDlgText( s_CandList.awszCandidate[ i ], pElement, dwSelTextColor, Coord );
			} 
			else
			{
				m_pParent->DrawDlgText( s_CandList.awszCandidate[ i ], pElement, dwTextColor, Coord );
			}
			Coord.fY += fSingleLineHeight;
		}
	} 
	else
	{
		if( bReading )
		{
			//m_pParent->DrawDlgText( s_wszReadingString, pElement, dwTextColor, Coord );
			m_pParent->DrawDlgText( s_wszReadingString, pElement, dwTextColor, Coord, -1, -1, true );
		}
		else
		{
			m_pParent->DrawDlgText( s_CandList.HoriCand.GetBuffer(), pElement, dwTextColor, Coord, -1, -1, true/*, DT_VCENTER | DT_CENTER*/ );
		}
		if( !bReading )
		{
			int nXLeft, nXRight;
			
			nXLeft = FontMng.GetCaretPos( s_CandList.HoriCand.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_CandList.nFirstSelected );
			//s_CandList.HoriCand.CPtoX( s_CandList.nFirstSelected, FALSE, &nXLeft );
			nXRight = FontMng.GetCaretPos( s_CandList.HoriCand.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
										   s_CandList.nFirstSelected + s_CandList.nHoriSelectedLen );
			if( nXLeft != nXRight )
			{
				//s_CandList.HoriCand.CPtoX( s_CandList.nFirstSelected + s_CandList.nHoriSelectedLen, FALSE, &nXRight );
				Coord.fWidth = FloatXCoord( nXRight - nXLeft );
				Coord.fX += FloatXCoord( nXLeft );
				m_pParent->DrawRect( Coord, dwSelBkColor );
				m_pParent->DrawDlgText( s_CandList.HoriCand.GetBuffer() + s_CandList.nFirstSelected, pElement, dwSelTextColor, Coord, -1, -1, true );
			}
		}
	}
}

void CEtUIIMEEditBox::RenderComposition( float fElapsedTime )
{
	int nX, nXFirst;

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	SUIElement* pElement = GetElement( 0 );
	nX = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nCaret );
	//m_Buffer.CPtoX( m_nCaret, FALSE, &nX );
	nXFirst = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
	//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

	SUICoord CompCoord, FirstCoord, CaretCoord;

	//SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	CompCoord.SetCoord( m_TextCoord.fX + ( nX - nXFirst ) / m_pParent->GetScreenWidth(), m_TextCoord.fY, 0.1f, m_TextCoord.fHeight );
	m_pParent->CalcTextRect( s_CompString.GetBuffer(), pElement, CompCoord );
	//if( CompCoord.Right() > m_TextCoord.Right() )
	//{
	//	CompCoord.fX += m_TextCoord.fX - CompCoord.fX;
	//	CompCoord.fY += CompCoord.fHeight;
	//}

	FirstCoord = CompCoord;
	s_fCompStringX = CompCoord.fX;
	s_fCompStringY = CompCoord.fY;

	int nCharLeft;
	const WCHAR *pwszComp;
	DWORD dwTextColor;

	dwTextColor = m_Property.IMEEditBoxProperty.dwCompTextColor;
	pElement->FontColor.dwCurrentColor = dwTextColor;
	pwszComp = s_CompString.GetBuffer();
	nCharLeft = s_CompString.GetTextSize();

	if( nCharLeft == 0 ) return;

	while( 1 )
	{
		int nLastInLine, nTrail, nNumCharToDraw;
		nLastInLine = FontMng.GetCaretFromCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight,
												  (int)((CompCoord.fX + CompCoord.fWidth - m_TextCoord.fX) * m_pParent->GetScreenWidth() + 0.0001f), nTrail );	// Add 0.0001 to round up because 23.9999 gets truncated to 23 as int.
		if( 0 == nLastInLine )
			nLastInLine = 1;

		// Note: Trail value is always 1.. Newly cached strings from font manager include the Trail value, so it must be added..
		if( 0 != nTrail )
			nLastInLine += 1;

		//s_CompString.XtoCP( ( int )( ( m_TextCoord.Right() - CompCoord.fX ) * m_pParent->GetScreenWidth() ), &nLastInLine, &bTrail );
		
		nNumCharToDraw = min( nCharLeft, nLastInLine );
		m_pParent->CalcTextRect( pwszComp, pElement, CompCoord, nNumCharToDraw );
		CompCoord.fHeight = m_TextCoord.fHeight;

		if( GetPrimaryLanguage() == LANG_KOREAN )
		{
			if( m_bCaretOn )
			{
				m_pParent->DrawRect( CompCoord, m_Property.IMEEditBoxProperty.dwCompBackColor );
			}
			else
			{
				dwTextColor = pElement->FontColor.dwColor[ UI_STATE_NORMAL ];
			}
		} 
		else
		{
			m_pParent->DrawRect( CompCoord, m_Property.IMEEditBoxProperty.dwCompBackColor );
		}

		pElement->FontColor.dwCurrentColor = dwTextColor;
		//CEtFontMng::GetInstance().EnableFreeType( false );
		m_pParent->DrawDlgText( pwszComp, pElement, dwTextColor, CompCoord, nNumCharToDraw );
		//CEtFontMng::GetInstance().EnableFreeType( true );

		nCharLeft -= nNumCharToDraw;
		pwszComp += nNumCharToDraw;
		if( nCharLeft <= 0 )
		{
			break;
		}
		CompCoord.fX += m_TextCoord.fX - CompCoord.fX;
		CompCoord.fY += CompCoord.fHeight;
	}

	//CompCoord = FirstCoord;
	CompCoord.fX = FirstCoord.fX;
	CompCoord.fY = FirstCoord.fY;
	CompCoord.fWidth = FirstCoord.fWidth;

	int nCharFirst;
	BYTE *pAttr;
	const WCHAR *pcComp;

	nCharFirst = 0;
	nXFirst = 0;
	s_nFirstTargetConv = -1;
	for( pcComp = s_CompString.GetBuffer(), pAttr = s_abyCompStringAttr; *pcComp != L'\0'; pcComp++, pAttr++ )
	{
		DWORD dwBkColor;
		int nXLeft, nXRight;

		nXLeft = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, int(pcComp - s_CompString.GetBuffer()) );
		//s_CompString.CPtoX( ( int )( pcComp - s_CompString.GetBuffer() ), FALSE, &nXLeft );
		nXRight = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_CompString.GetTextSize() );
		//s_CompString.CPtoX( ( int )( pcComp - s_CompString.GetBuffer() ), TRUE, &nXRight );

		if( m_bFirstVisibleOver == false && m_nTempCompSize == 0 )
		{
			if( nXRight - nXFirst > ( int )( ( m_TextCoord.Right() - CompCoord.fX ) * m_pParent->GetScreenWidth() ) )
			{
				CompCoord.fX += m_TextCoord.fX - CompCoord.fX;
				CompCoord.fY += CompCoord.fHeight;
				nCharFirst = ( int )( pcComp - s_CompString.GetBuffer() );
				nXFirst = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, nCharFirst );
				//s_CompString.CPtoX( nCharFirst, FALSE, &nXFirst );
			}
		}

		if( s_nCompCaret == ( int )( pcComp - s_CompString.GetBuffer() ) )
		{
			CaretCoord = CompCoord;
			CaretCoord.fX += ( nXLeft - nXFirst - 1 ) / m_pParent->GetScreenWidth();
			CaretCoord.fWidth = 2.0f / m_pParent->GetScreenWidth();
		}

		if( *pAttr == ATTR_TARGET_CONVERTED )
		{
			pElement->FontColor.dwCurrentColor = m_dwCompTargetColor;
			dwBkColor = m_dwCompTargetBkColor;
		}
		else
		{
			if( *pAttr == ATTR_TARGET_NOTCONVERTED )
			{
				pElement->FontColor.dwCurrentColor = m_dwCompTargetNonColor;
				dwBkColor = m_dwCompTargetNonBkColor;
			}
			else
			{
				continue;
			}
		}

		SUICoord TargetCoord;

		TargetCoord.SetCoord( CompCoord.fX + ( nXLeft - nXFirst ) / m_pParent->GetScreenWidth(), CompCoord.fY, ( nXRight - nXLeft ) / m_pParent->GetScreenWidth(), CompCoord.fHeight );
		m_pParent->DrawRect( TargetCoord, dwBkColor );
		m_pParent->DrawDlgText( pcComp, pElement, pElement->FontColor.dwCurrentColor, TargetCoord, 1 );
		if( s_nFirstTargetConv == -1 )
		{
			s_nFirstTargetConv = ( int )( pAttr - s_abyCompStringAttr );
		}
	}

	if( m_bCaretOn )
	{
		if( s_nCompCaret == s_CompString.GetTextSize() )
		{
			nX = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_nCompCaret );
			//s_CompString.CPtoX( s_nCompCaret, FALSE, &nX );
			CaretCoord = CompCoord;
			CaretCoord.fX += ( nX - nXFirst - 1 ) / m_pParent->GetScreenWidth();
			CaretCoord.fWidth = 2.0f / m_pParent->GetScreenWidth();
		}
		m_pParent->DrawRect( CaretCoord, m_Property.IMEEditBoxProperty.dwCompCaretColor );
	}
}

void CEtUIIMEEditBox::RenderIndicator( float fElapsedTime )
{
	if( m_Property.IMEEditBoxProperty.fIndicatorSize <= 0.0f )
	{
		return;
	}

	UpdateBlendRate();

	SUIElement *pElement;
	SUICoord TempCoord;//, CalcCoord;
	WCHAR *pwszIndicator;

	pElement = GetElement(5);
	pElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_IndicatorCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_IndicatorCoord );

/*	TempCoord = m_IndicatorCoord;
	TempCoord.fX += m_Property.IMEEditBoxProperty.fSpace;
	TempCoord.fY += m_Property.IMEEditBoxProperty.fSpace;
	TempCoord.fWidth -= m_Property.IMEEditBoxProperty.fSpace * 2;
	TempCoord.fHeight -= m_Property.IMEEditBoxProperty.fSpace * 2;
*/
	pElement->FontColor.dwCurrentColor = m_Property.IMEEditBoxProperty.dwIndiTextColor;
	if( ( s_ImeState == IMEUI_STATE_ON ) && ( s_bEnableImeSystem ) )
	{
		pwszIndicator = s_pwszCurrIndicator;
	}
	else
	{
		pwszIndicator = s_aszIndicator[ 0 ];
	}
/*	if( ( s_ImeState == IMEUI_STATE_ON ) && ( s_bEnableImeSystem ) )
	{
		pElement->FontColor.dwCurrentColor = m_dwIndicatorImeColor;
	}
	else
	{
		pElement->FontColor.dwCurrentColor = m_dwIndicatorEngColor;
	}
	if( s_bEnableImeSystem )
	{
		pwszIndicator = s_pwszCurrIndicator;
	}
	else
	{
		pwszIndicator = s_aszIndicator[ 0 ];
	}
*/
	TempCoord = m_IndicatorCoord;
	m_pParent->CalcTextRect( pwszIndicator, pElement, TempCoord );
	float fMidHeight = ( m_IndicatorCoord.fHeight - TempCoord.fHeight ) / 2.0f;
	TempCoord.fX -= m_Property.IMEEditBoxProperty.fSpace;
	TempCoord.fY = m_IndicatorCoord.fY + fMidHeight;

	m_pParent->DrawDlgText( pwszIndicator, pElement, pElement->FontColor.dwCurrentColor, TempCoord );
}

void CEtUIIMEEditBox::TruncateCompString( bool bUseBackSpace, int iNewStrLen )
{
	if( !s_bInsertOnType )
	{
		return;
	}

	int i, cc;

	cc = ( int )wcslen( s_CompString.GetBuffer() );
	assert( iNewStrLen == 0 || iNewStrLen >= cc );

	for ( i = 0; i < cc - s_nCompCaret; i++ )
	{
		SendMessage( m_pParent->GetHWnd(), WM_KEYDOWN, VK_RIGHT, 0 );
	}
	SendMessage( m_pParent->GetHWnd(), WM_KEYUP, VK_RIGHT, 0 );

	if( ( bUseBackSpace ) || ( m_bInsertMode ) )
	{
		iNewStrLen = 0;
	}
	if( iNewStrLen < cc )
	{
		for( i = 0; i < cc - iNewStrLen; i++ )
		{
			SendMessage( m_pParent->GetHWnd(), WM_KEYDOWN, VK_BACK, 0 );
			SendMessageW( m_pParent->GetHWnd(), WM_CHAR, VK_BACK, 0 );
		}
		SendMessage( m_pParent->GetHWnd(), WM_KEYUP, VK_BACK, 0 );
	}
	else
	{
		iNewStrLen = cc;
	}

	for ( i = 0; i < iNewStrLen; i++ )
	{
		SendMessage( m_pParent->GetHWnd(), WM_KEYDOWN, VK_LEFT, 0 );
	}
	SendMessage( m_pParent->GetHWnd(), WM_KEYUP, VK_LEFT, 0 );
}

void CEtUIIMEEditBox::FinalizeString( bool bSend )
{
	HIMC hImc;
	static bool bProcessing = false;

	hImc = _ImmGetContext( m_pParent->GetHWnd() );
	if( hImc == NULL )
	{
		return;
	}

	if( bProcessing )
	{
		_ImmReleaseContext( m_pParent->GetHWnd(), hImc );
		return;
	}
	bProcessing = true;

	if( ( !s_bInsertOnType ) && ( bSend ) )
	{
		LONG lLength;

		lLength = lstrlenW( s_CompString.GetBuffer() );
		if( ( GetLanguage() == LANG_CHT ) && ( s_CompString[ lLength - 1 ] == 0x3000 ) )
		{
			s_CompString[ lLength - 1 ] = 0;
		}
		SendCompString();
	}

	TruncateCompString();
	s_bHideCaret = false;
	s_bShowReadingWindow = false;

	ResetCompositionString();
	_ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
	_ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 ); 
	_ImmReleaseContext( m_pParent->GetHWnd(), hImc );
	bProcessing = false;
}

void CEtUIIMEEditBox::SendCompString()
{
	for( int i = 0; i < lstrlenW( s_CompString.GetBuffer() ); i++ )
	{
		MsgProc( WM_CHAR, ( WPARAM )s_CompString[ i ], 0 );
	}
}

void CEtUIIMEEditBox::PlaceCaret( int nCP )
{
	int nX1, nX2, nCompSize(0);
	m_bFirstVisibleOver = false;
	m_nTempCompSize = 0;

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	SUIElement* pElement = GetElement( 0 );
	nX1 = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
	//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1 );
	nX2 = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nCaret );
	//m_Buffer.CPtoX( m_nCaret, FALSE, &nX2 );
	nCompSize = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, nCP );
	//s_CompString.CPtoX( nCP, FALSE, &nCompSize );

	int nTextWidth = (int)( m_TextCoord.fWidth * m_pParent->GetScreenWidth() );
	if( (nX2-nX1+nCompSize) > nTextWidth )
	{
		int nTemp = m_Buffer.GetMaxEditBoxLength() - m_Buffer.GetTextSize();
		int nCPNew1st(0);

		if( nTemp < 3 )	
			++nCPNew1st;
		else			
			nCPNew1st += 3;

		// Note: Prevent the composing string from being too long and pushing the completed string out of view.
		// Otherwise m_nFirstVisible would exceed the string length.
		if( m_nFirstVisible + nCPNew1st < (int)wcslen( m_Buffer.GetBuffer() ) )
			m_nFirstVisible += nCPNew1st;
		else
			m_bFirstVisibleOver = true;

		// Adding m_bFirstVisibleOver (which does not exist in CustomUI) to handle this does not seem good,
		// When entering this state, tried calling ImmSetCompositionStringW to limit the length of the composing characters, but
		// No matter what is set, the function returns FALSE.
		// Even searching Google, many people say it doesn't work, but I couldn't find anyone writing how to fix it.

		// #30251 When whisper[ID] is long, very little space remains to actually type text.
		// Since the text space was short and kept resetting, which was complained about, re-examined this routine and
		// Reason m_bFirstVisibleOver exists (unlike CustomUI) is that, unlike Custom UI, composing characters also occupy area
		// Because DN has a feature that pushes characters on the right (and this is more natural).
		// Almost all editors work like this, though at first I thought all composing characters behaved this way.
		// Actually, after testing, only Korean behaves like this, and
		// For languages with 2+ char composition strings like Japanese/Chinese, a separate composition window opens,
		// Even when composing in the middle of a string, do not push characters to the right until composition is committed.
		// However, currently DN does not make that distinction.
		// So use the s_CompString workaround as shown below.
		if( m_bFirstVisibleOver && (int)wcslen( s_CompString.GetBuffer() ) < 10 )
		{
			m_bFirstVisibleOver = false;
			m_nTempCompSize = nCompSize;
		}
	}
}

void CEtUIIMEEditBox::ClearText()
{
	CEtUIEditBox::ClearText();
	ResetCompositionString();
}

void CEtUIIMEEditBox::CheckIMEToggleState()
{
	CheckToggleState();
}

void CEtUIIMEEditBox::CancelIMEComposition()
{
	HIMC hImc;
	hImc = _ImmGetContext( m_pParent->GetHWnd() );
	if( hImc == NULL )
		return;

	_ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
}

void CEtUIIMEEditBox::GetIMECompositionString(std::wstring& str)
{
	HIMC hImc;
	hImc = _ImmGetContext( m_pParent->GetHWnd() );
	if( hImc == NULL )
		return;

	WCHAR wszCompStr[MAX_COMPSTRING_SIZE]={0};
	LONG lRet = _ImmGetCompositionStringW( hImc, GCS_COMPSTR, wszCompStr, sizeof( wszCompStr ) );
	if (lRet > 0)
	{
		lRet /= sizeof( WCHAR );
		wszCompStr[lRet] = 0;
		str = wszCompStr;
	}	
}

int CEtUIIMEEditBox::GetMaxChar() const
{
	return m_Property.IMEEditBoxProperty.dwMaxChars;
}

void CEtUIIMEEditBox::SetMaxChar( DWORD dwChar )
{
	m_Property.IMEEditBoxProperty.dwMaxChars = dwChar;
	SetMaxEditLength( dwChar );
}