#include "StdAfx.h"
#include "EtUILineIMEEditBox.h"
#include "EtUIDialog.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUILineIMEEditBox::CEtUILineIMEEditBox( CEtUIDialog *pParent )
	: CEtUILineEditBox( pParent )
{
	m_Property.UIType = UI_CONTROL_LINE_IMEEDITBOX;

	m_dwCompTargetColor = D3DCOLOR_ARGB( 255, 255, 255, 255 );
	m_dwCompTargetBkColor = D3DCOLOR_ARGB( 255, 150, 150, 150 );
	m_dwCompTargetNonColor = D3DCOLOR_ARGB( 255, 255, 255, 0 );
	m_dwCompTargetNonBkColor = D3DCOLOR_ARGB( 255, 150, 150, 150 );
}

CEtUILineIMEEditBox::~CEtUILineIMEEditBox(void)
{
}

void CEtUILineIMEEditBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );
	UpdateRects();
}

bool CEtUILineIMEEditBox::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
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
		return CEtUILineEditBox::MsgProc( uMsg, wParam, lParam );
	}

	SBufferInfo *pBufferInfo(NULL);
	pBufferInfo = GetCurrentLine();
	if( !pBufferInfo ) return false;

	switch( uMsg )
	{
	case WM_IME_COMPOSITION:
		{
			// Note : Check here because m_Buffer is not a static variable.
			//		If at max length, do not compose characters anymore.
			//
			//if( pBufferInfo->pUniBuffer->IsMaxEditBoxLength() )
			//{
			//	PostMessage( GetFocus(), WM_IME_ENDCOMPOSITION, 0, 0 );
			//	break;
			//}

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
				return CEtUILineEditBox::MsgProc( uMsg, wParam, lParam );
			}

			// Note : Adjust string when it exceeds control size during Hangul composition.
			//		Currently tested only with Korean; Chinese and Japanese need to be checked later.
			//
			//if( lParam & GCS_COMPSTR )
			//{
			//	PlaceCaret( pBufferInfo, (int)lRet );
			//}
		}
		break;

	case WM_IME_ENDCOMPOSITION:
		TruncateCompString();
		ResetCompositionString();
		s_bHideCaret = false;
		s_bShowReadingWindow = false;
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
		return CEtUILineEditBox::MsgProc( uMsg, wParam, lParam );

	}  

	return *pbTrapped;
}

bool CEtUILineIMEEditBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			//float fFontHeight = m_pParent->GetFontHeight( m_Template.m_vecElement[5].nFontIndex, m_Template.m_vecElement[5].nFontHeight );
			//ID3DXFont *pFont = m_pParent->GetFont( m_Template.m_vecElement[5].nFontIndex, m_Template.m_vecElement[5].nFontHeight );
			//int nCompStrWidth;
			//s_CompString.CPtoX( s_CompString.GetTextSize(), FALSE, &nCompStrWidth );

			//if( ( s_fCompStringX <= fX ) && ( s_fCompStringY <= fY ) &&
			//	( s_fCompStringX + nCompStrWidth / m_pParent->GetScreenWidth() > fX ) && ( s_fCompStringY + fFontHeight > fY ) )
			//{
			//	HIMC hImc;
			//	int nCharBodyHit, nCharHit;
			//	int nTrail;

			//	s_CompString.XtoCP( ( int )( ( fX - s_fCompStringX ) * m_pParent->GetScreenWidth() ), &nCharBodyHit, &nTrail );
			//	if( ( nTrail ) && ( nCharBodyHit < s_CompString.GetTextSize() ) )
			//	{
			//		nCharHit = nCharBodyHit + 1;
			//	}
			//	else
			//	{
			//		nCharHit = nCharBodyHit;
			//	}

			//	hImc = _ImmGetContext( m_pParent->GetHWnd() );
			//	if( !hImc )
			//	{
			//		return true;
			//	}

			//	_ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 );
			//	_ImmReleaseContext( m_pParent->GetHWnd(), hImc );

			//	switch( GetPrimaryLanguage() )
			//	{
			//	case LANG_JAPANESE:
			//		if( s_nFirstTargetConv != -1 )
			//		{
			//			int nClauseClicked, nClauseSelected, nSendCount;
			//			BYTE byVirtKey;

			//			nClauseClicked = 0;
			//			while( ( int )s_adwCompStringClause[ nClauseClicked + 1 ] <= nCharBodyHit )
			//			{
			//				nClauseClicked++;
			//			}

			//			nClauseSelected = 0;
			//			while( ( int )s_adwCompStringClause[ nClauseSelected + 1 ] <= s_nFirstTargetConv )
			//			{
			//				nClauseSelected++;
			//			}

			//			if( nClauseClicked > nClauseSelected )
			//			{
			//				byVirtKey = VK_RIGHT;
			//			}
			//			else
			//			{
			//				byVirtKey = VK_LEFT;
			//			}
			//			nSendCount = abs( nClauseClicked - nClauseSelected );
			//			while( nSendCount-- > 0 )
			//			{
			//				SendKey( byVirtKey );
			//			}
			//			return true;
			//		}
			//	case LANG_CHINESE:
			//		{
			//			BYTE byVirtKey;
			//			int nSendCount;

			//			if( nCharHit > s_nCompCaret )
			//			{
			//				byVirtKey = VK_RIGHT;
			//			}
			//			else
			//			{
			//				byVirtKey = VK_LEFT;
			//			}
			//			nSendCount = abs( nCharHit - s_nCompCaret );
			//			while( nSendCount-- > 0 )
			//			{
			//				SendKey( byVirtKey );
			//			}
			//			break;
			//		}
			//	}
			//	return true;
			//}

			//if( ( s_CandList.bShowWindow ) && ( s_CandList.CandidateCoord.IsInside( fX, fY ) ) )
			//{
			//	if( s_bVerticalCand )
			//	{
			//		int nRow;

			//		nRow = ( int )( ( fY - s_CandList.CandidateCoord.fY ) / fFontHeight );

			//		if( nRow < ( int )s_CandList.dwCount )
			//		{
			//			switch( GetPrimaryLanguage() )
			//			{
			//			case LANG_CHINESE:
			//			case LANG_KOREAN:
			//				SendKey( ( BYTE ) ( '0' + nRow + 1 ) );
			//				break;

			//			case LANG_JAPANESE:
			//				{
			//					BYTE byVirtKey;
			//					int nNumToHit, nStrike;

			//					if( nRow > ( int )s_CandList.dwSelection )
			//					{
			//						byVirtKey = VK_DOWN;
			//					}
			//					else
			//					{
			//						byVirtKey = VK_UP;
			//					}
			//					nNumToHit = abs( ( int) ( nRow - s_CandList.dwSelection ) );
			//					for( nStrike = 0; nStrike < nNumToHit; nStrike++ )
			//					{
			//						SendKey( byVirtKey );
			//					}
			//					SendKey( VK_RIGHT );
			//					SendKey( VK_LEFT );
			//					break;
			//				}
			//			}
			//		}
			//	} 
			//	else
			//	{
			//		int i, nCharHit, nTrail, nCandidate, nEntryStart;

			//		s_CandList.HoriCand.XtoCP( ( int )( ( fX - s_CandList.CandidateCoord.fX ) * m_pParent->GetScreenWidth() ), &nCharHit, &nTrail );

			//		nCandidate = s_CandList.dwCount - 1;
			//		nEntryStart = 0;
			//		for( i = 0; i < ( int )s_CandList.dwCount; i++ )
			//		{
			//			if( nCharHit >= nEntryStart )
			//			{
			//				nEntryStart += lstrlenW( s_CandList.awszCandidate[ i ] ) + 1;
			//			} 
			//			else
			//			{
			//				nCandidate = i - 1;
			//				break;
			//			}
			//		}

			//		switch( GetPrimaryLanguage() )
			//		{
			//		case LANG_CHINESE:
			//		case LANG_KOREAN:
			//			SendKey( ( BYTE ) ( '0' + nCandidate + 1 ) );
			//			break;
			//		}
			//	}
			//	return true;
			//}
		}
	}

	return CEtUILineEditBox::HandleMouse( uMsg, fX, fY, wParam, lParam );
}

void CEtUILineIMEEditBox::Focus( bool fFocus )
{
	CEtUILineEditBox::Focus( fFocus );

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

void CEtUILineIMEEditBox::PumpMessage()
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

void CEtUILineIMEEditBox::Render( float fElapsedTime )
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
		if( s_bHideCaret )
		{
			SUICoord CompCoord;
			m_pParent->CalcTextRect( s_CompString.GetBuffer(), pElement, CompCoord );
			m_fCompositionWidth = CompCoord.fWidth;
		}

		CEtUILineEditBox::Render( fElapsedTime );
	}

	//if( pElement )
	//{
	//	ID3DXFont *pFont = m_pParent->GetFont( pElement->nFontIndex, pElement->nFontHeight );
	//	s_CompString.SetFont( pFont );
	//	s_CandList.HoriCand.SetFont( pFont );
	//}

	if( CEtUIIME::s_bDisableIMEonIMEControl )
		return;

	if( m_bFocus )
	{
		//RenderIndicator( fElapsedTime );

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

void CEtUILineIMEEditBox::RenderCandidateReadingWindow( float fElapsedTime, bool bReading )
{
	UINT i, nNumEntries;
	SUICoord Coord;
	DWORD dwTextColor, dwTextBkColor, dwSelTextColor, dwSelBkColor;
	int nX, nXComp;
	//int nXFirst;

	SBufferInfo *pBufferInfo(NULL);
	pBufferInfo = GetCurrentLine();
	if( !pBufferInfo ) return;

	nNumEntries = MAX_CANDLIST;
	if( bReading )
	{
		nNumEntries = 4;
	}

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	SUIElement* pElement = GetElement( 0 );
	nX = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, pBufferInfo->nCaretPos );
	//pBufferInfo->CPtoX( pBufferInfo->nCaretPos, FALSE, &nX );
	//pBufferInfo->pUniBuffer->CPtoX( m_nFirstVisible, FALSE, &nXFirst );

	if( bReading )
	{
		dwTextColor = m_Property.LineIMEEditBoxProperty.dwReadingTextColor;
		dwTextBkColor = m_Property.LineIMEEditBoxProperty.dwReadingBackColor;
		dwSelTextColor = m_Property.LineIMEEditBoxProperty.dwReadingSelTextColor;
		dwSelBkColor = m_Property.LineIMEEditBoxProperty.dwReadingSelBackColor;
	} 
	else
	{
		dwTextColor = m_Property.LineIMEEditBoxProperty.dwCandiTextColor;
		dwTextBkColor = m_Property.LineIMEEditBoxProperty.dwCandiBackColor;
		dwSelTextColor = m_Property.LineIMEEditBoxProperty.dwCandiSelTextColor;
		dwSelBkColor = m_Property.LineIMEEditBoxProperty.dwCandiSelBackColor;
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

	Coord.SetCoord( s_fCompStringX + FloatXCoord( nXComp ), s_fCompStringY + pBufferInfo->TextCoord.fHeight, fWidthRequired, fHeightRequired );

	if( Coord.Right() > m_pParent->Width() )
	{
		Coord.fX -= Coord.Right() - m_pParent->Width();
		Coord.fWidth = m_pParent->Width() - Coord.fX;
	}
	if( Coord.Bottom() <= m_pParent->Height() )
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
				m_pParent->DrawDlgText( s_CandList.HoriCand.GetBuffer() + s_CandList.nFirstSelected, pElement, dwSelTextColor, Coord );
			}
		}
	}
}

void CEtUILineIMEEditBox::RenderComposition( float fElapsedTime )
{
	int nX(0), nXFirst(0);

	SBufferInfo *pBufferInfo = GetCurrentLine();
	if( !pBufferInfo ) return;

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	SUIElement* pElement = GetElement( 0 );
	nX = FontMng.GetCaretPos( pBufferInfo->pUniBuffer->GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, pBufferInfo->nCaretPos );
	//pBufferInfo->CPtoX( pBufferInfo->nCaretPos, FALSE, &nX );

	//SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	SUICoord CompCoord, FirstCoord, CaretCoord;

	CompCoord.SetCoord( pBufferInfo->TextCoord.fX + ( nX - nXFirst ) / m_pParent->GetScreenWidth(), pBufferInfo->TextCoord.fY, 0.1f, pBufferInfo->TextCoord.fHeight );
	m_pParent->CalcTextRect( s_CompString.GetBuffer(), pElement, CompCoord );

	if( CompCoord.Right() > pBufferInfo->TextCoord.Right() )
	{
		CompCoord.fX += pBufferInfo->TextCoord.fX - CompCoord.fX;
		CompCoord.fY += CompCoord.fHeight;
	}

	FirstCoord = CompCoord;
	s_fCompStringX = CompCoord.fX;
	s_fCompStringY = CompCoord.fY;

	int nCharLeft;
	const WCHAR *pwszComp;
	DWORD dwTextColor;
	WCHAR wszRenderTemp[1024] = {0,};

	dwTextColor = m_Property.LineIMEEditBoxProperty.dwCompTextColor;
	pElement->FontColor.dwCurrentColor = dwTextColor;
	pwszComp = s_CompString.GetBuffer();
	nCharLeft = s_CompString.GetTextSize();
	while( 1 )
	{
		int nLastInLine, nTrail, nNumCharToDraw;

		nLastInLine = FontMng.GetCaretFromCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
												  (int)((pBufferInfo->TextCoord.Right() - CompCoord.fX) * m_pParent->GetScreenWidth() + 0.0001f), nTrail );	// Add 0.0001 to round up because 23.9999 gets truncated to 23 as int.

		if( nLastInLine == -1 ) break;
		if( 0 == nLastInLine )
			nLastInLine = 1;

		// Note: Trail value is always 1.. Newly cached strings from font manager include the Trail value, so it must be added..
		if( 0 != nTrail )
			nLastInLine += 1;

		//s_CompString.XtoCP( ( int )( ( pBufferInfo->TextCoord.Right() - CompCoord.fX ) * m_pParent->GetScreenWidth() ), &nLastInLine, &bTrail );
		nNumCharToDraw = min( nCharLeft, nLastInLine );
		wcsncpy_s( wszRenderTemp, _countof(wszRenderTemp), pwszComp, nNumCharToDraw );
		m_pParent->CalcTextRect( wszRenderTemp, pElement, CompCoord, nNumCharToDraw );
		CompCoord.fHeight = pBufferInfo->TextCoord.fHeight;

		if( GetPrimaryLanguage() == LANG_KOREAN )
		{
			if( m_bCaretOn )
			{
				m_pParent->DrawRect( CompCoord, m_Property.LineIMEEditBoxProperty.dwCompBackColor );
			}
			else
			{
				dwTextColor = pElement->FontColor.dwColor[ UI_STATE_NORMAL ];
			}
		} 
		else
		{
			m_pParent->DrawRect( CompCoord, m_Property.LineIMEEditBoxProperty.dwCompBackColor );
		}

		pElement->FontColor.dwCurrentColor = dwTextColor;
		m_pParent->DrawDlgText( wszRenderTemp, pElement, dwTextColor, CompCoord, nNumCharToDraw );

		nCharLeft -= nNumCharToDraw;
		pwszComp += nNumCharToDraw;
		if( nCharLeft <= 0 )
		{
			break;
		}
		CompCoord.fX += pBufferInfo->TextCoord.fX - CompCoord.fX;
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

		// How can the two have different values when they're identical? -_-
		nXLeft = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight,  (int)(pcComp - s_CompString.GetBuffer()) );
		//s_CompString.CPtoX( ( int )( pcComp - s_CompString.GetBuffer() ), FALSE, &nXLeft );
		nXRight = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, s_CompString.GetTextSize() );
		//s_CompString.CPtoX( ( int )( pcComp - s_CompString.GetBuffer() ), TRUE, &nXRight );

		if( nXRight - nXFirst > ( int )( ( pBufferInfo->TextCoord.Right() - CompCoord.fX ) * m_pParent->GetScreenWidth() ) )
		{
			CompCoord.fX += pBufferInfo->TextCoord.fX - CompCoord.fX;
			CompCoord.fY += CompCoord.fHeight;
			nCharFirst = ( int )( pcComp - s_CompString.GetBuffer() );
			nXFirst = FontMng.GetCaretPos( s_CompString.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, nCharFirst );
			//s_CompString.CPtoX( nCharFirst, FALSE, &nXFirst );
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

			if( IsFocus() )
			{
				CEtUIEditBox::s_CurrentCaretPos.fY = CaretCoord.fY;
				if( m_pParent )
				{
					CEtUIEditBox::s_CurrentCaretPos.fY += m_pParent->GetYCoord();
				}
			}
		}
		m_pParent->DrawRect( CaretCoord, m_Property.LineIMEEditBoxProperty.dwCompCaretColor );
	}
}

void CEtUILineIMEEditBox::TruncateCompString( bool bUseBackSpace, int iNewStrLen )
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

void CEtUILineIMEEditBox::FinalizeString( bool bSend )
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

void CEtUILineIMEEditBox::SendCompString()
{
	for( int i = 0; i < lstrlenW( s_CompString.GetBuffer() ); i++ )
	{
		MsgProc( WM_CHAR, ( WPARAM )s_CompString[ i ], 0 );
	}
}