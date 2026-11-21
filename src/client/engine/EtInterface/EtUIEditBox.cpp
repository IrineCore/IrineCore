#include "StdAfx.h"
#include "EtUIEditBox.h"
#include "EtUIDialog.h"
#include "EtUIIME.h"
#include "StringUtil.h"
#include "strsafe.h"
#include "EtUINameLinkMgr.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

SUICoord CEtUIEditBox::s_CurrentCaretPos;
std::vector<WCHAR> CEtUIEditBox::s_vecInvalidCharater;
bool CEtUIEditBox::s_bCheckThaiLanguageVowelRule = false;

CEtUIEditBox::CEtUIEditBox( CEtUIDialog *pParent )
	: CEtUIControl( pParent )
{
	m_Property.UIType = UI_CONTROL_EDITBOX;

	m_bCaretOn = true;
	m_fCaretBlinkTime = GetCaretBlinkTime() * 0.001f;
	QueryPerformanceCounter( &m_liLastBlinkTime );
	m_nFirstVisible = 0;
	m_nLastVisible = 0;
	m_nCaret = 0;
	m_FullTextCaret = 0;
	m_nSelStart = 0;
	m_bInsertMode = true;
	m_bMouseDrag = false;
	m_fCompositionWidth = 0.0f;
	m_nFilterOption = 0;
	m_RenderTextColor = 0;
}

CEtUIEditBox::~CEtUIEditBox(void)
{
}

void CEtUIEditBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );
	UpdateRects();

	m_RenderTextColor = m_Property.EditBoxProperty.dwTextColor;
}

void CEtUIEditBox::SetTemplate( int nTemplateIndex )
{
	CEtUIControl::SetTemplate( nTemplateIndex );

	SUIElement *pElement(NULL);
	pElement = GetElement(0);
	//if( pElement )
	//{
	//	ID3DXFont *pFont = m_pParent->GetFont( pElement->nFontIndex, pElement->nFontHeight );
	//	SetFont( pFont );
	//}

	if( m_Property.EditBoxProperty.dwMaxChars == 0 ) 
		m_Property.EditBoxProperty.dwMaxChars = 256;
	
	SetMaxEditLength( m_Property.EditBoxProperty.dwMaxChars );
}

void CEtUIEditBox::Focus( bool bFocus )
{
	CEtUIControl::Focus( bFocus );
	CEtUIControl::SetFocusEditBox( IsFocus() );
	ResetCaretBlink();

	if( IsFocus() )
	{
		m_pParent->ProcessCommand( EVENT_EDITBOX_FOCUS, true, this );
	}
	else
	{
		m_pParent->ProcessCommand( EVENT_EDITBOX_RELEASEFOCUS, true, this );
		SetSelection(GetCaretPos(), GetCaretPos());
	}
}

void CEtUIEditBox::SetText( LPCWSTR wszText, bool bSelected )
{
	ASSERT( wszText != NULL );

	const CEtUINameLinkMgr& nameLinkMgr = EtInterface::GetNameLinkMgr();

	std::wstring translated = wszText;
	if (translated.empty() == false)
		nameLinkMgr.TranslateText(translated, wszText);

	if( m_Buffer.SetText( translated.c_str() ) )
		m_FullTextBuffer.SetText( wszText );
	
	m_nFirstVisible = 0;
	PlaceCaret( m_Buffer.GetTextSize() );

	if( bSelected )
	{
		m_nSelStart = 0;
	}
	else
	{
		m_nSelStart = m_nCaret;
	}
}

void CEtUIEditBox::AddText( LPCWSTR wszText, bool bFocus )
{
	ASSERT( wszText != NULL );

	std::wstring translated;
	int pureTextLength = EtInterface::GetNameLinkMgr().TranslateText( translated, wszText );

	if( m_Buffer.IsEnableInsertString( m_nCaret, translated.c_str() ) && 
		m_FullTextBuffer.IsEnableInsertString( m_FullTextCaret, wszText ) )
	{
		m_Buffer.InsertString( m_nCaret, translated.c_str(), -1, true );
		m_FullTextBuffer.InsertString( m_FullTextCaret, wszText, -1, true );

		PlaceCaret( m_nCaret + pureTextLength );
		m_nSelStart = m_nCaret;

		if( bFocus && m_bFocus == false )
			m_pParent->RequestFocus( this );
	}
}

HRESULT CEtUIEditBox::GetTextCopy( LPWSTR strDest, UINT bufferCount )
{
	ASSERT( strDest );

	if( bufferCount != 0 ) {
		//wcscpy_s( strDest, bufferCount, m_Buffer.GetBuffer() );
		StringCchCopyW(strDest, bufferCount, m_Buffer.GetBuffer());
	}

	return S_OK;
}

void CEtUIEditBox::ClearText()
{
	m_FullTextBuffer.Clear();
	m_Buffer.Clear();
	m_RenderBuffer.Clear();
	m_nFirstVisible = 0;
	PlaceCaret( 0 );
	m_nSelStart = 0;
}

void CEtUIEditBox::ClearTextMemory()
{
	m_FullTextBuffer.ClearMemory();
	m_Buffer.ClearMemory();
	m_RenderBuffer.ClearMemory();
	m_nFirstVisible = 0;
	PlaceCaret( 0 );
	m_nSelStart = 0;
}

int CEtUIEditBox::GetTextToInt()
{
	const WCHAR *pBuffer = m_Buffer.GetBuffer();
	if( pBuffer )
	{
		if( wcslen( pBuffer ) == 0 )
			return -1;

		return _wtoi( pBuffer );
	}

	return 0;
}

void CEtUIEditBox::SetIntToText( int nValue )
{
	WCHAR wszBuffer[256]={0};
	_itow( nValue, wszBuffer, 10 );
	SetText( wszBuffer, false );
}

INT64 CEtUIEditBox::GetTextToInt64()
{
	const WCHAR *pBuffer = m_Buffer.GetBuffer();
	if( pBuffer )
	{
		return _wtoi64( pBuffer );
	}

	return 0;
}

void CEtUIEditBox::SetInt64ToText( INT64 nValue )
{
	WCHAR wszBuffer[256]={0};
	_i64tow( nValue, wszBuffer, 10 );
	SetText( wszBuffer, false );
}

#define IN_FLOAT_CHARSET( c ) \
	( (c) == L'-' || (c) == L'.' || ( (c) >= L'0' && (c) <= L'9' ) )

void CEtUIEditBox::ParseFloatArray( float *pNumbers, int nCount )
{
	int nWritten;
	const WCHAR *pToken, *pEnd;
	WCHAR wszToken[ 60 ];

	nWritten = 0;
	pToken = m_Buffer.GetBuffer();
	while( ( nWritten < nCount ) && ( *pToken != L'\0' ) )
	{
		while( *pToken == L' ' )
		{
			pToken++;
		}
		if( *pToken == L'\0' )
		{
			break;
		}

		// Locate the end of number
		pEnd = pToken;
		while( IN_FLOAT_CHARSET( *pEnd ) )
		{
			pEnd++;
		}

		int nTokenLen;

		nTokenLen = min( sizeof( wszToken ) / sizeof( wszToken[ 0 ] ) - 1, int( pEnd - pToken ) );
		wcscpy_s( wszToken, nTokenLen, pToken );
		*pNumbers = ( float )wcstod( wszToken, NULL );
		nWritten++;
		pNumbers++;
		pToken = pEnd;
	}
}

void CEtUIEditBox::SetTextFloatArray( const float *pNumbers, int nCount )
{
	WCHAR wszBuffer[ 512 ] = { 0 };
	WCHAR wszTmp[ 64 ];

	if( pNumbers == NULL )
	{
		return;
	}

	int i;

	for( i = 0; i < nCount; i++ )
	{
		swprintf_s( wszTmp, 64, L"%.4f ", pNumbers[i] );
		wcscat_s( wszBuffer, 512, wszTmp );
	}

	if( ( nCount > 0 ) && ( wcslen( wszBuffer ) > 0 ) )
	{
		wszBuffer[ wcslen( wszBuffer ) - 1 ] = 0;
	}

	SetText( wszBuffer );
}

bool CEtUIEditBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() || !IsKeyLock())
	{
		return false;
	}

	bool bHandled(false);

	if( uMsg == WM_KEYDOWN )
	{
		switch( wParam )
		{
		case VK_HOME:
			PlaceCaret( 0 );
			if( GetKeyState( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}
			ResetCaretBlink();
			bHandled = true;
			break;
		case VK_END:
			PlaceCaret( m_Buffer.GetTextSize() );
			if( GetKeyState( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}
			ResetCaretBlink();
			bHandled = true;
			break;
		case VK_INSERT:
			if( GetKeyState( VK_CONTROL ) < 0 )
			{
				CopyToClipboard();
			} 
			else if( GetKeyState( VK_SHIFT ) < 0 )
			{
				PasteFromClipboard();
			} 
			else
			{
				m_bInsertMode = !m_bInsertMode;
			}
			break;
		case VK_DELETE:
			if( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText();
				m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
			}
			else
			{
				if (RemoveChar(m_nCaret, 1))
				{
					m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
				}
			}
			ResetCaretBlink();
			bHandled = true;
			break;
		case VK_LEFT:
			if( GetKeyState( VK_CONTROL ) < 0 )
			{
				// Note : Ctrl functionality does not work with Hangul input. Needs to be fixed...
				m_Buffer.GetPriorItemPos( m_nCaret, &m_nCaret );
				PlaceCaret( m_nCaret );
			}
			else if( m_nCaret > 0 )
			{
				PlaceCaret( GetPrevCaret() );
			}
			if( GetKeyState( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}
			ResetCaretBlink();
			bHandled = true;
			break;
		case VK_RIGHT:
			if( GetKeyState( VK_CONTROL ) < 0 )
			{
				m_Buffer.GetNextItemPos( m_nCaret, &m_nCaret );
				PlaceCaret( m_nCaret );
			}
			else if( m_nCaret < m_Buffer.GetTextSize() )
			{
				PlaceCaret( GetNextCaret() );
			}
			if( GetKeyState( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}
			ResetCaretBlink();
			bHandled = true;
			break;
		case VK_UP:
			if( GetKeyState( VK_SHIFT ) < 0 )
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_SHIFT_KEYUP, true, this );
			}
			else if ( GetKeyState( VK_LCONTROL ) < 0 )
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_LCTRL_KEYUP, true, this );
			}
			else
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_KEYUP, true, this );
			}
			bHandled = true;
			break;
		case VK_DOWN:
			if( GetKeyState( VK_SHIFT ) < 0 )
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_SHIFT_KEYDOWN, true, this );
			}
			else if ( GetKeyState( VK_LCONTROL ) < 0 )
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_LCTRL_KEYDOWN, true, this );
			}
			else
			{
				m_pParent->ProcessCommand( EVENT_EDITBOX_KEYDOWN, true, this );
			}
			bHandled = true;
			break;
		case VK_ESCAPE:
			m_pParent->ProcessCommand( EVENT_EDITBOX_ESCAPE, true, this );
			bHandled = true;
			break;
		case VK_TAB:
			break;
		default:
			bHandled = wParam != VK_ESCAPE;
		}
	}

	return bHandled;
}

bool CEtUIEditBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() || !IsKeyLock())
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			if( !m_bFocus )
			{
				m_pParent->RequestFocus( this );
			}
			if( !IsInside( fX, fY ) )
			{
				return false;
			}
			m_bMouseDrag = true;
			SetCapture( m_pParent->GetHWnd() );

			int nCP, nTrail, nX1st;

			CEtFontMng& FontMng = CEtFontMng::GetInstance();
			SUIElement* pElement = GetElement( 0 );
			nX1st = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
			//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );
			nCP = FontMng.GetCaretFromCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
											  ( int )( m_pParent->GetScreenWidth() * ( fX - m_TextCoord.fX ) + nX1st ), nTrail );
			
			// nTrail indicates whether the caret should be placed at the very end
			if( 0 <= nCP )
			//if( SUCCEEDED( m_Buffer.XtoCP( ( int )( m_pParent->GetScreenWidth() * ( fX - m_TextCoord.fX ) + nX1st ), &nCP, &nTrail ) ) )
			{
				if( ( nTrail ) && ( nCP < m_Buffer.GetTextSize() ) )
				{
					PlaceCaret( nCP + 1 );
				}
				else
				{
					PlaceCaret( nCP );
				}
				m_nSelStart = m_nCaret;
				ResetCaretBlink();
			}
			return true;
		}
	case WM_LBUTTONUP:
		ReleaseCapture();
		m_bMouseDrag = false;
		break;
	case WM_MOUSEMOVE:
		if( m_bMouseDrag )
		{
			int nCP, nTrail, nX1st;

			CEtFontMng& FontMng = CEtFontMng::GetInstance();
			SUIElement* pElement = GetElement( 0 );
			nX1st = FontMng.GetCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
			//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );
			nCP = FontMng.GetCaretFromCaretPos( m_Buffer.GetBuffer(), pElement->nFontIndex, pElement->nFontHeight, 
											  ( int )( m_pParent->GetScreenWidth() * (fX-GetUICoord().fX) + nX1st ), nTrail );

			if( 0 <= nCP )
			//if( SUCCEEDED( m_Buffer.XtoCP( ( int )( m_pParent->GetScreenWidth() * fX + nX1st ), &nCP, &nTrail ) ) )
			{
				if( ( nTrail ) && ( nCP < m_Buffer.GetTextSize() ) )
				{
					PlaceCaret( nCP + 1 );
				}
				else
				{
					PlaceCaret( nCP );
				}
			}
		}
		break;
	}

	return false;
}

bool CEtUIEditBox::InsertChar(WCHAR wChar)
{
	if( m_Buffer.IsEnableInsertChar( m_nCaret, wChar ) && 
		m_FullTextBuffer.IsEnableInsertChar( m_FullTextCaret, wChar ) )
	{
		m_Buffer.InsertChar( m_nCaret, wChar, true );
		m_FullTextBuffer.InsertChar( m_FullTextCaret, wChar, true );
		return true;
	}

	return false;
}

bool CEtUIEditBox::OverTypeChar(WCHAR wChar)
{
	if (RemoveChar(m_nCaret, 1))
		return InsertChar(wChar);

	return false;
}

void CEtUIEditBox::GetFullTextTagFieldPos(int& tagFieldStart, int& tagFieldEnd, int curCursorPos) const
{
	std::wstring fullText = m_FullTextBuffer.GetBuffer();

	std::wstring targetTag;
	std::wstring::size_type tagStart, tagEnd;
	tagStart = tagEnd = std::wstring::npos;
	EtInterface::GetNameLinkMgr().MakeTag(targetTag, CEtUINameLinkMgr::eNLTAG_START);
	tagStart = fullText.rfind(targetTag, curCursorPos);

	if (tagStart != std::wstring::npos)
	{
		EtInterface::GetNameLinkMgr().MakeTag(targetTag, CEtUINameLinkMgr::eNLTAG_END);
		tagEnd = fullText.find(targetTag, tagStart);

		if (int(tagStart) <= curCursorPos && curCursorPos < int(tagEnd))
		{
			tagFieldStart = int(tagStart);
			tagFieldEnd = int(tagEnd);
		}
	}
}

bool CEtUIEditBox::RemoveChar(int startCaretPos, int length)
{
	int tempS, tempE;
	tempS = tempE = -1;

	int fullTextStartCaretPos = GetFullTextCaretPos(startCaretPos, false);
	GetFullTextTagFieldPos(tempS, tempE, fullTextStartCaretPos);
	fullTextStartCaretPos = (tempS == -1) ? fullTextStartCaretPos : tempS;

	tempS = tempE = -1;
	int endCaretPos = startCaretPos + length;
	int fullTextEndCaretPos = GetFullTextCaretPos(endCaretPos, false);
	GetFullTextTagFieldPos(tempS, tempE, fullTextEndCaretPos);
	fullTextEndCaretPos = (tempE == -1) ? fullTextEndCaretPos : tempE;

	int processingCaretPos = fullTextStartCaretPos;

	int i = 0;
	for (; i < fullTextEndCaretPos - fullTextStartCaretPos;)
	{
		std::wstring fullText = m_FullTextBuffer.GetBuffer();
		if (fullText.size() <= 0)
			break;

		std::wstring targetTag;
		EtInterface::GetNameLinkMgr().MakeTag(targetTag, CEtUINameLinkMgr::eNLTAG_START);

		if (processingCaretPos >= 0)
		{
			std::wstring::size_type tagStart = std::wstring::npos;
			if (EtInterface::GetNameLinkMgr().IsTagType(fullText, processingCaretPos, CEtUINameLinkMgr::eNLTAG_START))
				tagStart = (std::wstring::size_type)processingCaretPos;

			if (tagStart == std::wstring::npos && processingCaretPos > 0)
				tagStart = fullText.rfind(targetTag, processingCaretPos - 1);

			if (tagStart != std::wstring::npos)
			{
				EtInterface::GetNameLinkMgr().MakeTag(targetTag, CEtUINameLinkMgr::eNLTAG_END);
				std::wstring::size_type tagEnd = fullText.find(targetTag, tagStart);
				int calculatedTagEnd = int(tagEnd) + 2;

				if (int(tagStart) <= processingCaretPos && processingCaretPos <= int(tagEnd))
				{
					fullText.erase(tagStart, calculatedTagEnd - tagStart);

					SetText(fullText.c_str(), false);
					processingCaretPos = (int)tagStart;

					i += calculatedTagEnd - int(tagStart);
					continue;
				}
			}
		}

		int textCaretPos = GetTextCaretPos(processingCaretPos);

		if( m_Buffer.RemoveChar(textCaretPos) == false ||
			m_FullTextBuffer.RemoveChar(processingCaretPos) == false )
		{
			_ASSERT(0);
			return false;
		}
		++i;
	}

	PlaceCaret(GetTextCaretPos(processingCaretPos));
	m_nSelStart = m_nCaret;

	return true;
}

bool CEtUIEditBox::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() || !IsKeyLock() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_KEYUP:
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_RETURN:
			m_pParent->ProcessCommand( EVENT_EDITBOX_STRING, true, this, (UINT)wParam );
			return true;
		}
		break;

	case WM_CHAR:
		switch( ( WCHAR )wParam )
		{
			// Pressing Ctrl+Backspace types a weird character; checking its value shows 0x7f, which is neither in the Hangul range nor a valid character.
			// DxCustomUI also had issues, so just block it out.
		case 0x7f:
			break;
			// Pressing Ctrl+H sends WM_CHAR 0x08, which deletes characters in the edit box.
			// To resolve,
			// Either handle VK_BACK in WM_KEYDOWN (in that case, do nothing and break on WM_CHAR-VK_BACK)
			// Check if H is pressed.
			// The latter is cleaner, so choose the latter.
		case VK_BACK:
			if( GetKeyState('H')&0x80 )
				break;

			if( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText();
				m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
			}
			else if( m_nCaret > 0 )
			{
				PlaceCaret( m_nCaret - 1 );
				m_nSelStart = m_nCaret;

				RemoveChar(m_nCaret, 1);
				m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
			}
			ResetCaretBlink();
			break;
		case 24:        // Ctrl-X Cut
		case VK_CANCEL: // Ctrl-C Copy
			CopyToClipboard();
			if( ( WCHAR )wParam == 24 )
			{
				DeleteSelectionText();
				m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
			}
			break;
		case 22:		// Ctrl-V Paste
			PasteFromClipboard();
			m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
			break;
		case 1:			// Ctrl-A Select All
			if( m_nSelStart == m_nCaret )
			{
				m_nSelStart = 0;
				PlaceCaret( m_Buffer.GetTextSize() );
			}
			break;
		case VK_RETURN:
// 			m_pParent->ProcessCommand( EVENT_EDITBOX_STRING, true, this, (UINT)wParam );
// 			break;
		// Junk characters we don't want in the string
		case 26:  // Ctrl Z
		case 2:   // Ctrl B
		case 14:  // Ctrl N
		case 19:  // Ctrl S
		case 4:   // Ctrl D
		case 6:   // Ctrl F
		case 7:   // Ctrl G
		case 10:  // Ctrl J
		case 11:  // Ctrl K
		case 12:  // Ctrl L
		case 17:  // Ctrl Q
		case 23:  // Ctrl W
		case 5:   // Ctrl E
		case 18:  // Ctrl R
		case 20:  // Ctrl T
		case 25:  // Ctrl Y
		case 21:  // Ctrl U
		case 9:   // Ctrl I
		case 15:  // Ctrl O
		case 16:  // Ctrl P
		case 27:  // Ctrl [
		case 29:  // Ctrl ]
		case 28:  // Ctrl \ 
			break;

		default:
			// Half-width character conversion processing
			ProcessHalfWidth( wParam );

			// Check if character should be ignored
			if( !CheckValidCharacter( (WCHAR)wParam ) )
				break;

			if( s_bCheckThaiLanguageVowelRule )
			{
				if( !CheckThaiLanguageVowelRule( (WCHAR)wParam ) )
					break;
			}

			if( m_Property.UIType == UI_CONTROL_EDITBOX )
			{
				if( m_Property.EditBoxProperty.bOnlyDigit && !iswdigit( (WCHAR)wParam ) )
				{
					break;
				}
			}

			bool bFiltering = false;
			if (m_nFilterOption != 0)
			{
				if (m_nFilterOption & eDIGIT)
				{
					if (iswdigit((WCHAR)wParam))
						bFiltering = true;
				}

				if (m_nFilterOption & eALPHABET)
				{
					if (iswalpha((WCHAR)wParam))
						bFiltering = true;
				}

				if (bFiltering == false)
					break;
			}

			if( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText();
			}

			if( ( !m_bInsertMode ) && ( m_nCaret < m_Buffer.GetTextSize() ) )
			{
				if (OverTypeChar((WCHAR)wParam))
				{
					PlaceCaret(m_nCaret + 1);
					m_nSelStart = m_nCaret;
				}
			} 
			else
			{
				if (InsertChar((WCHAR)wParam))
				{
					PlaceCaret( m_nCaret + 1 );
					//UpdateLastVisible();
					m_nSelStart = m_nCaret;
				}
			}

			ResetCaretBlink();
			m_pParent->ProcessCommand( EVENT_EDITBOX_CHANGE, true, this, (UINT)wParam );
		}
		return true;
	}

	return false;
}

void CEtUIEditBox::ProcessHalfWidth(WPARAM &wParam)
{
	const int nOffset = 0xFEE0;
	if( (WCHAR)wParam >= 0x0041+nOffset && (WCHAR)wParam <= 0x005A+nOffset )	// L'A' ~ L'Z'
		wParam -= nOffset;
	else if( (WCHAR)wParam >= 0x0061+nOffset && (WCHAR)wParam <= 0x007A+nOffset )	// L'a' ~ L'z'
		wParam -= nOffset;
	else if( (WCHAR)wParam >= 0x0030+nOffset && (WCHAR)wParam <= 0x0039+nOffset )	// L'0' ~ L'9'
		wParam -= nOffset;
}

bool CEtUIEditBox::CheckValidCharacter( WCHAR wChar )
{
	for( DWORD dwIndex = 0; dwIndex < (DWORD)s_vecInvalidCharater.size(); ++dwIndex )
	{
		if( s_vecInvalidCharater[dwIndex] == wChar )
			return false;
	}
	return true;
}

void CEtUIEditBox::AddInvalidCharacter( wchar_t wCharStart, wchar_t wCharEnd )
{
	if( static_cast<DWORD>(wCharStart) > static_cast<DWORD>(wCharEnd) )
		return;

	for( DWORD dwIndex = static_cast<DWORD>(wCharStart); dwIndex <= static_cast<DWORD>(wCharEnd); ++dwIndex )
		s_vecInvalidCharater.push_back( (WCHAR)dwIndex );
}

void CEtUIEditBox::UpdateRects()
{
	m_TextCoord = m_Property.UICoord;
	m_TextCoord.fX += m_Property.EditBoxProperty.fBorder;
	m_TextCoord.fWidth -= m_Property.EditBoxProperty.fBorder * 2;
	m_TextCoord.fY += m_Property.EditBoxProperty.fBorder;
	m_TextCoord.fHeight -= m_Property.EditBoxProperty.fBorder * 2;

	m_RenderCoord[ 0 ] = m_TextCoord;
	m_RenderCoord[ 1 ].SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, m_TextCoord.fX - m_Property.UICoord.fX, m_Property.UICoord.fHeight );
	m_RenderCoord[ 2 ].SetCoord( m_TextCoord.Right(), m_Property.UICoord.fY, m_Property.UICoord.Right() - m_TextCoord.Right(), m_Property.UICoord.fHeight );
	m_RenderCoord[ 3 ].SetCoord( m_TextCoord.fX, m_Property.UICoord.fY, m_TextCoord.fWidth, m_TextCoord.fY - m_Property.UICoord.fY );
	m_RenderCoord[ 4 ].SetCoord( m_TextCoord.fX, m_TextCoord.Bottom(), m_TextCoord.fWidth, m_Property.UICoord.Bottom() - m_TextCoord.Bottom() );

	m_TextCoord.fX += m_Property.EditBoxProperty.fSpace;
	m_TextCoord.fWidth -= m_Property.EditBoxProperty.fSpace * 2;
	m_TextCoord.fY += m_Property.EditBoxProperty.fSpace;
	m_TextCoord.fHeight -= m_Property.EditBoxProperty.fSpace * 2;
}

void CEtUIEditBox::RenderText(float fElapsedTime, SUIElement* pElement, const SUICoord& TempCoord, bool bNeedDivide)
{
	// Wordbreak is not applied to EditBox text.
	bool bFontMngWordBreak = CEtFontMng::s_bUseWordBreak;
	CEtFontMng::s_bUseWordBreak = false;

	//pElement = GetElement(0);
	if (pElement)
	{
		pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

		if (false == bNeedDivide)
		{
			SUICoord drawCoord = TempCoord;
			DrawTextWithTraslation(m_FullTextBuffer.GetBuffer(), pElement, drawCoord);
		}
		else
		{ 
			wstring strBuf = m_RenderBuffer.GetBuffer() + m_nFirstVisible;
			wstring strFormer = strBuf.substr( 0, (m_nCaret-m_nFirstVisible) );
			wstring strLatter = strBuf.substr( max(0, min((int)strBuf.length(), m_nCaret-m_nFirstVisible)), strBuf.length()-(m_nCaret-m_nFirstVisible) );
			SUICoord FormerCoord = TempCoord;
			SUICoord LatterCoord = TempCoord;
			SUICoord WidthCoord;		// Temporary variable used to get width

			m_pParent->DrawDlgText( strFormer.c_str(), pElement, 
				m_Property.EditBoxProperty.dwTextColor, FormerCoord, -1, pElement->dwFontFormat, true );

			m_pParent->CalcTextRect( strFormer.c_str(), pElement, WidthCoord );
			LatterCoord.fX += (WidthCoord.fWidth + m_fCompositionWidth);
			m_pParent->DrawDlgText( strLatter.c_str(), pElement, 
				m_Property.EditBoxProperty.dwTextColor, LatterCoord, -1, pElement->dwFontFormat, true );
		}
	}

	CEtFontMng::s_bUseWordBreak = bFontMngWordBreak;
}

void CEtUIEditBox::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	//HRESULT hr;
	int nSelStartX(0), nCaretX(0);
	SUIElement *pElement = NULL;

	// Note : Probably no need to set the font in Render.
	//		Will uncomment if it causes issues later...
	//pElement = &m_Template.m_vecElement[ 0 ];
	//if( pElement )
	//{
	//	m_Buffer.SetFont( m_pParent->GetFont( pElement->nFontIndex ) );
	//	PlaceCaret( m_nCaret );
	//}

	UpdateBlendRate();

	int nVecSize = (int)m_Template.m_vecElement.size();
	for( int i = 0; i < nVecSize; i++ )
	{
		pElement = GetElement(i);
		if( !m_bFocus )
		{
			if (IsEnable() == false)
				pElement->TextureColor.Blend( UI_STATE_DISABLED, fElapsedTime, 0.5f );
			else
				pElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
		}
		else
		{
			pElement->TextureColor.Blend( UI_STATE_FOCUS, fElapsedTime, m_fBlendRate );
		}
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_RenderCoord[ i ] );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_RenderCoord[ i ] );
	}

	UpdateLastVisible();

	wstring strBuf = m_Buffer.GetBuffer();
	if( m_Property.EditBoxProperty.bPassword )
		strBuf.replace(0, int(strBuf.size()), int(strBuf.size()), '*');

	m_RenderBuffer.SetText( strBuf.substr( 0, m_nLastVisible ).c_str() );
	
	const WCHAR* pTextToRender = m_RenderBuffer.GetBuffer();
	int nXFirst = 0;
	CEtFontMng& FontMng = CEtFontMng::GetInstance();

	pElement = GetElement(0);
	if( pElement )
	{
		nXFirst = FontMng.GetCaretPos( pTextToRender, pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible);
		//m_RenderBuffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

#ifdef _TEST_CODE_KAL
		nCaretX = FontMng.GetCaretPos( strBuf.c_str(), pElement->nFontIndex, pElement->nFontHeight, m_nCaret );
#else
		nCaretX = FontMng.GetCaretPos( pTextToRender, pElement->nFontIndex, pElement->nFontHeight, m_nCaret );
#endif
		//hr = m_RenderBuffer.CPtoX( m_nCaret, FALSE, &nCaretX );
		if( m_nCaret != m_nSelStart )
		{
			//hr = m_RenderBuffer.CPtoX( m_nSelStart, FALSE, &nSelStartX );
			// Note: The entire buffer must be handled since invisible areas can also be selected
			//m_nSelStart = m_nSelStart - m_nFirstVisible;
			//if( m_nSelStart < 0 ) 
			//	m_nSelStart = 0;
#ifdef _TEST_CODE_KAL
			nSelStartX = FontMng.GetCaretPos(strBuf.c_str(), pElement->nFontIndex, pElement->nFontHeight, m_nSelStart);
#else
			nSelStartX = FontMng.GetCaretPos(m_RenderBuffer.GetBuffer()/*+m_nFirstVisible*/, pElement->nFontIndex, pElement->nFontHeight, m_nSelStart);
#endif
		}
		else
		{
			nSelStartX = nCaretX;
		}
	}

	SUICoord SelectionCoord;

	if( m_nCaret != m_nSelStart )
	{
		int nSelLeftX, nSelRightX;

		nSelLeftX = nCaretX;
		nSelRightX = nSelStartX;
		if( nSelLeftX > nSelRightX )
		{
			std::swap( nSelLeftX, nSelRightX );
		}

		float fMinRight, fMinBottom;

		int nTextWidth = ( int )( m_TextCoord.fWidth * m_pParent->GetScreenWidth() );
		int nSelRightXTemp = nSelRightX, nSelLeftXTemp = nSelLeftX;
		if( nSelRightX >= nXFirst )
			nSelRightXTemp = nSelRightX - nXFirst;
		if( nSelLeftX >= nXFirst )
			nSelLeftXTemp = nSelLeftX - nXFirst;
		int nSelWidth = nSelRightXTemp - nSelLeftXTemp;
		//if( nSelWidth > nTextWidth )
		//	nSelWidth -= nXFirst;
		//nSelWidth = nSelWidth % nTextWidth;
		SelectionCoord.SetCoord( ( ( float )nSelLeftX ) / m_pParent->GetScreenWidth(), m_TextCoord.fY, 
			( ( float )( nSelWidth ) ) / m_pParent->GetScreenWidth(), m_TextCoord.fHeight );
		SelectionCoord.fX += m_TextCoord.fX - ( ( float )nXFirst ) / m_pParent->GetScreenWidth();
		fMinRight = min( m_TextCoord.Right(), SelectionCoord.Right() );
		fMinBottom = min( m_TextCoord.Bottom(), SelectionCoord.Bottom() );
		SelectionCoord.fWidth = fMinRight - SelectionCoord.fX;
		SelectionCoord.fHeight = fMinBottom - SelectionCoord.fY;
		SelectionCoord.fX = max( m_TextCoord.fX, SelectionCoord.fX );
		SelectionCoord.fY = max( m_TextCoord.fY, SelectionCoord.fY );
		SelectionCoord.fWidth = min( m_TextCoord.fWidth, SelectionCoord.fWidth );

		bool bEnableZ = GetEtDevice()->EnableZ( false );
		m_pParent->DrawRect( SelectionCoord, m_Property.EditBoxProperty.dwSelBackColor );
		GetEtDevice()->EnableZ( bEnableZ );
	}

	// If a string is being composed at the front or middle, shift back by its width.
	// If IME composition is happening in the middle, render it separately
	SUICoord TempCoord = m_TextCoord;
	bool bNeedDivide = false;
	if( 0.0f != m_fCompositionWidth )
	{
		if( m_nCaret < m_nLastVisible )
		{
			if( 0 == m_nCaret )
				TempCoord.fX += m_fCompositionWidth;
			else
				bNeedDivide = true;
		}
	}

	RenderText(fElapsedTime, pElement, TempCoord, bNeedDivide);

	//if( wcslen(m_RenderBuffer.GetBuffer() + m_nFirstVisible) > 0 )
	//{
	//	OutputDebugStringW( L"[Edit Box]" );
	//	OutputDebugStringW( m_RenderBuffer.GetBuffer() + m_nFirstVisible );
	//	OutputDebugStringW( L"\n" );
	//}

	//if( m_nCaret != m_nSelStart )
	//{
	//	int nFirstToRender, nNumChatToRender;

	//	nFirstToRender = max( m_nFirstVisible, min( m_nSelStart, m_nCaret ) );
	//	nNumChatToRender = max( m_nSelStart, m_nCaret ) - nFirstToRender;
	//	m_pParent->DrawDlgText( m_RenderBuffer.GetBuffer() + nFirstToRender, pElement, 
	//							m_Property.EditBoxProperty.dwSelTextColor, SelectionCoord, -1, pElement->dwFontFormat, true );
	//}
	//CEtFontMng::GetInstance().EnableFreeType( true );

	LARGE_INTEGER liCurTime, liFrequency;
	float fElapsedBlinkTime;

	QueryPerformanceCounter( &liCurTime );
	QueryPerformanceFrequency( &liFrequency );
	fElapsedBlinkTime = ( ( float )( liCurTime.QuadPart - m_liLastBlinkTime.QuadPart ) ) / liFrequency.QuadPart;
	if( fElapsedBlinkTime >= m_fCaretBlinkTime )
	{
		m_bCaretOn = !m_bCaretOn;
		m_liLastBlinkTime = liCurTime;
	}

	SUICoord CaretCoord;
	CaretCoord = TempCoord;
	CaretCoord.fX += ( ( float )( -nXFirst + nCaretX - 1 ) ) / m_pParent->GetScreenWidth();
	CaretCoord.fWidth = 2.0f / m_pParent->GetScreenWidth();

	if (IsFocus())
	{
		s_CurrentCaretPos = CaretCoord;
		// Since it is hard to get and handle the parent dialog at the call site, pre-add it if a parent exists.
		// During character composition, the ( !CEtUIIME::s_bHideCaret ) condition below evaluates to false so s_CurrentCaretPos cannot be updated,
		// Moved this up so it is always processed.
		if( m_pParent )
		{
			s_CurrentCaretPos.fX += m_pParent->GetXCoord();
			s_CurrentCaretPos.fY += m_pParent->GetYCoord();
		}
	}

	if( ( m_bFocus ) && ( m_bCaretOn ) && ( !CEtUIIME::s_bHideCaret ) )
	{
		if( !m_bInsertMode )
		{
			int nRightEdgeX = 0;

			//m_RenderBuffer.CPtoX( m_nCaret, TRUE, &nRightEdgeX );
			nRightEdgeX = FontMng.GetCaretPos( pTextToRender, pElement->nFontIndex, pElement->nFontHeight, m_nCaret );

			CaretCoord.fWidth = m_TextCoord.fX + ( ( float )( nRightEdgeX - nXFirst ) ) / m_pParent->GetScreenWidth() - CaretCoord.fX;
		}
		bool bEnableZ = GetEtDevice()->EnableZ( false );
		m_pParent->DrawRect( CaretCoord, m_Property.EditBoxProperty.dwCaretColor );
		GetEtDevice()->EnableZ( bEnableZ );
	}
}

void CEtUIEditBox::PlaceCaret( int nCP )
{
	ASSERT( ( nCP >= 0 ) && ( nCP <= m_Buffer.GetTextSize() ) );

	int nX1st, nX, nX2;
	m_nCaret = nCP;
	m_FullTextCaret = GetFullTextCaretPos(m_nCaret, true);

	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	const WCHAR* pText = m_Buffer.GetBuffer();
	SUIElement* pElement = GetElement( 0 );
	if( !pElement ) {
		return;
	}
	nX1st = FontMng.GetCaretPos( pText, pElement->nFontIndex, pElement->nFontHeight, m_nFirstVisible );
	//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );
	nX = FontMng.GetCaretPos( pText, pElement->nFontIndex, pElement->nFontHeight, nCP );
	//m_Buffer.CPtoX( nCP, FALSE, &nX );  // LEAD
	if( nCP == m_Buffer.GetTextSize() )
	{
		nX2 = nX;
	}
	else
	{
		//m_Buffer.CPtoX( nCP, TRUE, &nX2 );
		nX2 = FontMng.GetCaretPos( pText, pElement->nFontIndex, pElement->nFontHeight, nCP );
	}

	if( nX < nX1st )
	{
		// Note : When deleting characters, because the preceding characters were not visible
		//		Modified m_nFirstVisible.
		if( nCP >= 2 )			
			nCP -= GetTextMoveCount( 2, false );
	
		m_nFirstVisible = nCP;
		//UpdateLastVisible();
	}
	else
	{
		int nTextWidth;

		nTextWidth = ( int )( m_TextCoord.fWidth * m_pParent->GetScreenWidth() );
		if( nX2 > nX1st + nTextWidth )
		{
			int nXNewLeft = 0, nCPNew1st = 0, nNewTrail = 0, nXNew1st = 0;

			nXNewLeft = nX2 - nTextWidth;
			//m_Buffer.XtoCP( nXNewLeft, &nCPNew1st, &nNewTrail );
			nCPNew1st = FontMng.GetCaretFromCaretPos( pText, pElement->nFontIndex, pElement->nFontHeight, nXNewLeft, nNewTrail );
			nXNew1st = FontMng.GetCaretPos( pText, pElement->nFontIndex, pElement->nFontHeight, nCPNew1st );
			//m_Buffer.CPtoX( nCPNew1st, FALSE, &nXNew1st );
			if( nXNew1st < nXNewLeft )
			{
				// Note : Calculate the remaining maximum input length.
				//		The code below runs when 3 or more characters of max input length remain
				//		Shift the string forward by 3 characters.
				if( s_bCheckThaiLanguageVowelRule )
				{
					int nTemp = m_Buffer.GetMaxEditBoxLength() - GetThaiLength( m_Buffer.GetBuffer() );

					if( nTemp < 3 )	
						nCPNew1st += GetTextMoveCount( 1, true );
					else			
						nCPNew1st += GetTextMoveCount( 3, true );
				}
				else
				{
					int nTemp = m_Buffer.GetMaxEditBoxLength() - m_Buffer.GetTextSize();

					if( nTemp < 3 )	
						++nCPNew1st;
					else			
						nCPNew1st += 3;
				}
			}

			m_nFirstVisible = nCPNew1st;
		}
	}
}

void CEtUIEditBox::DeleteSelectionText()
{
	int nFirst, nLast;

	nFirst = min( m_nCaret, m_nSelStart );
	nLast = max( m_nCaret, m_nSelStart );
	RemoveChar( nFirst, nLast - nFirst );
}

void CEtUIEditBox::ResetCaretBlink()
{
	m_bCaretOn = true;
	QueryPerformanceCounter( &m_liLastBlinkTime );
}

void CEtUIEditBox::CopyToClipboard()
{
	if( ( m_nCaret != m_nSelStart ) && ( OpenClipboard( NULL ) ) )
	{
		HGLOBAL hBlock;

		EmptyClipboard();
		hBlock = GlobalAlloc( GMEM_MOVEABLE, sizeof( WCHAR ) * ( m_Buffer.GetTextSize() + 1 ) );
		if( hBlock )
		{
			WCHAR *pwszText;
			int nFirst, nLast;

			pwszText = ( WCHAR * )GlobalLock( hBlock );
			if( pwszText )
			{
				nFirst = min( m_nCaret, m_nSelStart );
				nLast = max( m_nCaret, m_nSelStart );
				if( nLast - nFirst > 0 )
				{
					memcpy( pwszText, m_Buffer.GetBuffer() + nFirst, ( nLast - nFirst ) * sizeof( WCHAR ) );
				}
				pwszText[ nLast - nFirst ] = L'\0';
				GlobalUnlock( hBlock );
			}
			SetClipboardData( CF_UNICODETEXT, hBlock );
		}
		CloseClipboard();
		if( hBlock )
		{
			GlobalFree( hBlock );
		}
	}
}

void CEtUIEditBox::PasteFromClipboard()
{
	DeleteSelectionText();

	if( OpenClipboard( NULL ) )
	{
		HANDLE handle;

		handle = GetClipboardData( CF_UNICODETEXT );
		if( handle )
		{
			WCHAR *pwszText;

			pwszText = ( WCHAR * )GlobalLock( handle );
			if( pwszText )
			{
				// The current edit box cannot handle newlines; replace them with spaces for now.
				int nLen = (int)wcslen( pwszText );
				for( int i = 0; i < nLen; ++i )
				{
					if( pwszText[i] == '\n' )
						pwszText[i] = ' ';

					// Check if characters are allowed even when pasting
					if( !CheckValidCharacter( pwszText[i] ) )
						pwszText[i] = ' ';
				}

				// Do not paste if non-numeric characters are present when only digits are allowed.
				if( m_Property.EditBoxProperty.bOnlyDigit == TRUE )
				{
					int nLenInner = (int)wcslen( pwszText );
					for( int i = 0; i < nLenInner; ++i )
					{
						if( !iswdigit( pwszText[i] ) )
						{
							CloseClipboard();
							return;
						}
					}
				}

				AddText(pwszText, true);
				GlobalUnlock( handle );
			}
		}
		CloseClipboard();
	}
}

void CEtUIEditBox::SetMaxEditLength( DWORD dwMaxLength )
{
	ASSERT( dwMaxLength > 0 );
	m_RenderBuffer.SetMaxEditBoxLength( dwMaxLength );
	m_Buffer.SetMaxEditBoxLength( dwMaxLength-1 );
}

void CEtUIEditBox::UpdateLastVisible( void )
{
	CEtFontMng& FontMng = CEtFontMng::GetInstance();
	const WCHAR* pText = m_Buffer.GetBuffer();
	SUIElement* pElement = GetElement( 0 );

	int nTrail = 0;
	int nTextWidth = ( int )( m_TextCoord.fWidth * m_pParent->GetScreenWidth() );

	if( CEtFontMng::s_bUseUniscribe )
	{
		// Wordbreak is not applied to EditBox text.
		bool bFontMngWordBreak = CEtFontMng::s_bUseWordBreak;
		CEtFontMng::s_bUseWordBreak = false;
		
		std::vector<std::wstring> vecTextLine;
		std::wstring strText( pText + m_nFirstVisible );
		int nMaxWidth;
		bool bRtnWordBreakText = CEtFontMng::GetInstance().GetWordBreakText( strText, pElement->nFontIndex, pElement->nFontHeight, static_cast<float>( nTextWidth ), vecTextLine, nMaxWidth );
		CEtFontMng::s_bUseWordBreak = bFontMngWordBreak;

		if( !bRtnWordBreakText )
			return;

		if( static_cast<int>( vecTextLine.size() ) > 0 )
		{
			m_nLastVisible = static_cast<int>( vecTextLine[0].length() ) + m_nFirstVisible;
		}
	}
	else
	{
		m_nLastVisible = FontMng.GetCaretFromCaretPos( pText+m_nFirstVisible, pElement->nFontIndex, pElement->nFontHeight, nTextWidth, nTrail )+m_nFirstVisible+1; // +1 because it must be treated as a count 
	}
}

void CEtUIEditBox::SetSelection(int startPos, int caretPos)
{
	if (caretPos < 0 || caretPos > GetTextLength() ||
		startPos < 0 || startPos > GetTextLength())
		return;

	m_nSelStart = startPos;
	m_nCaret = caretPos;
}

int CEtUIEditBox::GetMaxChar() const
{
	return m_Property.EditBoxProperty.dwMaxChars;
}

void CEtUIEditBox::SetMaxChar( DWORD dwChar )
{
	m_Property.EditBoxProperty.dwMaxChars = dwChar;
	SetMaxEditLength( dwChar );
}

void CEtUIEditBox::SetTextWithEllipsis(const wchar_t* text, const wchar_t* wszSymbol, bool bSelected)
{
	std::wstring result;
	std::wstring tempStr = text;
	std::wstring ellipsis = wszSymbol;

	DWORD maxChar = GetMaxChar();
	if (wcslen(text) >= maxChar)
	{
		result = tempStr.substr(0, maxChar - ellipsis.size() - 2);
		result += ellipsis;
	}
	else
	{
		result = tempStr;
	}

	m_pParent->MakeStringWithEllipsis(GetElement(0), m_TextCoord.fWidth, result, wszSymbol);

	SetText(result.c_str(), bSelected);
}

bool CEtUIEditBox::DrawTextPiece(const std::wstring& piece, int pieceStart, int pieceEnd, SUIElement* pElement, SUICoord& drawCoord)
{
	if (piece.empty())
		return true;

	if (m_nFirstVisible > pieceEnd || m_nLastVisible < pieceStart)
		return true;

	// viewporting(?)
	bool res = true;
	SUICoord widthCoord;
	std::wstring drawText(piece);

	int pieceSize = (int)piece.size() - abs(m_nFirstVisible - pieceStart);

	if (pieceStart >= m_nFirstVisible)
	{
		if (pieceEnd > m_nLastVisible)
		{
			drawText = piece.substr(0, m_nLastVisible - pieceStart);
			res = false;
		}
	}
	else
	{
		if (pieceEnd > m_nLastVisible)
		{
			drawText = piece.substr(m_nFirstVisible - pieceStart, m_nLastVisible - m_nFirstVisible);
			res = false;
		}
		else
		{
			drawText = piece.substr(m_nFirstVisible - pieceStart, std::wstring::npos);
		}
	}

	if( m_Property.EditBoxProperty.bPassword )
		drawText.replace(0, int(drawText.size()), int(drawText.size()), '*');

	m_pParent->DrawDlgText(drawText.c_str(), pElement, m_RenderTextColor, drawCoord, -1, pElement->dwFontFormat, true, 0.f);
	m_pParent->CalcTextRect(drawText.c_str(), pElement, widthCoord);
	drawCoord.fX += widthCoord.fWidth;

	return res;
}

int CEtUIEditBox::GetFullTextCaretPos(int caretPos, bool bRetWithCalcTagLast) const
{
	if (caretPos == 0)
		return 0;

	std::wstring text = m_FullTextBuffer.GetBuffer();
	std::wstring::size_type linkStartPos = 0;
	int caretPosFound = 0;
	int fullTextCaretPos = 0;
	int processTextCaretPos = 0;
	bool bInsideTag = false;

	WORD sep = EtInterface::GetNameLinkMgr().GetNameLinkSeperator();
	while(fullTextCaretPos < int(text.size()))
	{
		if (caretPos == caretPosFound)
		{
			if (bRetWithCalcTagLast)
			{
				if (bInsideTag)
				{
					std::wstring parsee = text.substr(fullTextCaretPos, std::wstring::npos);
					std::wstring targetTag;
					targetTag = FormatW(L"%ce", sep);
					std::wstring::size_type loc = parsee.find(targetTag.c_str());
					fullTextCaretPos += (loc != std::wstring::npos) ? int(loc + 2) : int(parsee.length());
				}
			}
			else
			{
				CEtUINameLinkMgr::eNameLinkTagType type = EtInterface::GetNameLinkMgr().IsTagType(text, fullTextCaretPos);
				if (type != CEtUINameLinkMgr::eNLTAG_NONE)
				{
					std::wstring parsee = text.substr(fullTextCaretPos, std::wstring::npos);
					std::wstring targetTag;
					targetTag = FormatW(L"%ce", sep);
					std::wstring::size_type loc = parsee.find(targetTag.c_str());
					fullTextCaretPos += (loc != std::wstring::npos) ? int(loc + 2) : int(parsee.length());
				}
			}
			break;
		}

		CEtUINameLinkMgr::eNameLinkTagType type = EtInterface::GetNameLinkMgr().IsTagType(text, fullTextCaretPos);
		if (type != CEtUINameLinkMgr::eNLTAG_NONE)
		{
			if (type == CEtUINameLinkMgr::eNLTAG_START)
				bInsideTag = true;
			else if (type == CEtUINameLinkMgr::eNLTAG_END)
				bInsideTag = false;

			//	todo by kalliste : Handle error when parsee tag text length is below threshold; otherwise use a function returning a fixed value by tag type.
			int offset = 0;
			std::wstring parsee = text.substr(++fullTextCaretPos, std::wstring::npos);
			EtInterface::GetNameLinkMgr().GetTagOffsetByParseText(parsee, offset);
			fullTextCaretPos += offset;
		}
		else
		{
			++caretPosFound;
			++fullTextCaretPos;
		}
	}

	return fullTextCaretPos;
}

int CEtUIEditBox::GetTextCaretPos(int fullTextCaretPos)
{
	if (fullTextCaretPos <= 0)
		return 0;

	std::wstring text = m_FullTextBuffer.GetBuffer();
	std::wstring::size_type linkStartPos = 0;
	int caretPosFound = 0;
	int curFullCaretPos = 0;
	bool bInsideTag = false;

	while(curFullCaretPos < int(text.size()) && (curFullCaretPos < fullTextCaretPos))
	{
		CEtUINameLinkMgr::eNameLinkTagType type = EtInterface::GetNameLinkMgr().IsTagType(text, curFullCaretPos);
		if (type != CEtUINameLinkMgr::eNLTAG_NONE)
		{
			int offset = 0;
			std::wstring parsee = text.substr(curFullCaretPos + 1, std::wstring::npos);
			EtInterface::GetNameLinkMgr().GetTagOffsetByParseText(parsee, offset);
			curFullCaretPos += (offset + 1);
		}
		else
		{
			++caretPosFound;
			++curFullCaretPos;
		}
	}

	return caretPosFound;
}

// todo : refactoring using command pattern. by kalliste
int CEtUIEditBox::DrawTextWithTraslation(const WCHAR* originalStr, SUIElement* pElement, SUICoord& drawCoord)
{
	if (pElement == NULL || (drawCoord.fHeight == 0.f && drawCoord.fWidth == 0.f))
		return 0;

	std::wstring rest;
	const std::wstring& trs_org = originalStr;
	std::wstring::size_type linkStartPos = 0;
	int posWithoutTag, lengthWithoutTag, fullTextCaretIdx, i;
		posWithoutTag		= 
		lengthWithoutTag	= 
		i					= 
		fullTextCaretIdx	= 0;

	m_RenderTextColor = m_Property.EditBoxProperty.dwTextColor;

	WCHAR sep = EtInterface::GetNameLinkMgr().GetNameLinkSeperator();
	rest = originalStr;
	while(rest.size() > 0 || linkStartPos != std::wstring::npos)
	{
		linkStartPos = rest.find_first_of(sep, linkStartPos);
		if (linkStartPos == std::wstring::npos)
		{
			lengthWithoutTag = posWithoutTag + int(rest.length());
			DrawTextPiece(rest, posWithoutTag, lengthWithoutTag, pElement, drawCoord);
			break;
		}
		else
		{
			std::wstring drawText = rest.substr(0, linkStartPos);
			if (DrawTextPiece(drawText, posWithoutTag, posWithoutTag + (int)linkStartPos, pElement, drawCoord) == false)
				break;

			rest = rest.substr(linkStartPos + 1, std::wstring::npos);
			posWithoutTag += int(drawText.length());
			lengthWithoutTag += int(drawText.length());

			int tagOffset = 1;
			if (EtInterface::GetNameLinkMgr().ParseText(rest, tagOffset, this) == false)
			{
				lengthWithoutTag = posWithoutTag + int(rest.length());
				DrawTextPiece(rest, posWithoutTag, posWithoutTag + int(rest.length()), pElement, drawCoord);
				break;
			}

			linkStartPos = 0;
		}
	}

	return lengthWithoutTag;
}

bool CEtUIEditBox::OnParseTextColor(DWORD color)
{
	m_RenderTextColor = (color != 0) ? color : m_Property.EditBoxProperty.dwTextColor;
	return true;
}

bool CEtUIEditBox::OnParseTextFailColor()
{
	m_RenderTextColor = m_Property.EditBoxProperty.dwTextColor;
	return true;
}

bool CEtUIEditBox::OnParseTextRestoreColor()
{
	m_RenderTextColor = m_Property.EditBoxProperty.dwTextColor;
	return true;
}

void CEtUIEditBox::SetFilterOption(int option)
{
	m_nFilterOption |= option;
}


static const WCHAR s_wThaiVowels[26] = 
{
	// Allow to use double vowels
	0xE40, 0xE41, 0xE42, 0xE43,	0xE44, 0xE45, 0xE46,
	// Behind vowels
	0xE30, 0xE32,
	// Below vowels
	0xE38, 0xE39, 0xE3A,
	// Above vowels in 1st class
	0xE31, 0xE33, 0xE34, 0xE35, 0xE36, 0xE37, 
	// Above vowels in 2nd class
	0xE47, 0xE48, 0xE49, 0xE4A, 0xE4B, 0xE4C, 0xE4D, 0xE4E
};

#define ISTHAIBASECONSONANT( ch ) ( (unsigned int)(ch) - 0xE01 ) <= ( 0xE2E - 0xE01 )	// Check Thai consonant

static const WCHAR s_wThaiStart = 0xE01;
static const int s_nThaiAllowDoubleVowelLastIndex = 6;
static const int s_nBehindVowelStartIndex = 7;
static const int s_nBelowVowelStartIndex = 9;
static const int s_nAboveVowelStartIndex = 12;
static const int s_nAboveVowelsIn1stClassStartIndex = 12;
static const int s_nAboveVowelsIn2ndClassStartIndex = 18;


int CEtUIEditBox::CheckThaiLanguageVowel( WCHAR wChar )
{
	int nVowelIndex = -1;
	for( int i=0; i<26; i++ )
	{
		if( s_wThaiVowels[i] == wChar )
		{
			nVowelIndex = i;
			break;
		}
	}

	return nVowelIndex;
}

int CEtUIEditBox::GetThaiLength( const WCHAR* pWChar )
{
	if( pWChar == NULL ) return 0;
	int nBufferThaiLength = 0;
	int nBufferSize = lstrlenW( pWChar );

	for( int i=0; i<nBufferSize; i++ )
	{
		int nTextIndex = CheckThaiLanguageVowel( pWChar[i] );
		if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )	// Consonant, excluding upper/lower vowels, check only if 0xE33
			nBufferThaiLength++;
	}

	return nBufferThaiLength;
}

WCHAR CEtUIEditBox::GetThaiStartChar()
{
	return s_wThaiStart;
}

int CEtUIEditBox::GetThaiBelowVowelStartIndex()
{
	return s_nBelowVowelStartIndex;
}

bool CEtUIEditBox::CheckThaiLanguageVowelRule( WCHAR wChar )
{
	int nVowelIndex = CheckThaiLanguageVowel( wChar );
	
	bool bCheckAvailableChar = true;
	if( nVowelIndex >= 0 )	// Check only when input is a vowel
	{
		int nTextSize = m_Buffer.GetTextSize();
		WCHAR wLastChar = m_Buffer[m_nCaret-1];

		if( m_nCaret > 0 && m_nCaret <= nTextSize && wLastChar >= s_wThaiStart )	// When a previous character exists
		{
			int nLastTextIndex = CheckThaiLanguageVowel( wLastChar );
			
			if( nVowelIndex > s_nThaiAllowDoubleVowelLastIndex && nLastTextIndex == nVowelIndex )	// Check characters that must not be used as diphthongs
			{
				bCheckAvailableChar = false;
			}
			else if( nLastTextIndex == -1 && !( ISTHAIBASECONSONANT( wLastChar ) ) )				// Skip if the previous character is neither a consonant nor a vowel
			{
				bCheckAvailableChar = false;
			}
			else if( nLastTextIndex >= 0 && nVowelIndex >= s_nBehindVowelStartIndex )				// When the previous character is a vowel and input vowel is back, up, or down vowel
			{
				if( nLastTextIndex < s_nBelowVowelStartIndex )	// When the previous character is a standalone vowel
				{
					bCheckAvailableChar = false;
				}
				else if( ( nLastTextIndex >= s_nAboveVowelStartIndex ) && ( nVowelIndex >= s_nBelowVowelStartIndex && nVowelIndex < s_nAboveVowelStartIndex ) )	// When the previous character is an up vowel and a down vowel is input
				{
					bCheckAvailableChar = false;
				}
				else if( ( nLastTextIndex >= s_nBelowVowelStartIndex && nLastTextIndex < s_nAboveVowelStartIndex ) && ( nVowelIndex >= s_nBelowVowelStartIndex && nVowelIndex < s_nAboveVowelStartIndex ) )
				{
					bCheckAvailableChar = false;		// When the previous character is a down vowel and a down vowel is input
				}
				else
				{
					std::vector<WCHAR> vecVowelChar;
					for( int i=m_nCaret-1; i>=0; i-- )
					{
						int nTextIndex = CheckThaiLanguageVowel( m_FullTextBuffer[i] );
						if( nTextIndex >= 0 )
							vecVowelChar.push_back( nTextIndex );
						else
							break;
					}

					int nAboveVowelsCount = 0;
					int nAboveVowelsIn1stClassCount = 0;
					int nAboveVowelsIn2ndClassCount = 0;

					for( int i=0; i<static_cast<int>( vecVowelChar.size() ); i++ )
					{
						if( vecVowelChar[i] == nVowelIndex )
						{
							bCheckAvailableChar = false;
							break;
						}

						if( vecVowelChar[i] >= s_nAboveVowelStartIndex )
							nAboveVowelsCount++;

						if( vecVowelChar[i] >= s_nAboveVowelsIn1stClassStartIndex && vecVowelChar[i] < s_nAboveVowelsIn2ndClassStartIndex )
							nAboveVowelsIn1stClassCount++;

						if( vecVowelChar[i] >= s_nAboveVowelsIn2ndClassStartIndex )
							nAboveVowelsIn2ndClassCount++;

						if( vecVowelChar[i] == 13 )		// If behind vowel 0xE33 is present, no more vowels can be appended.
							bCheckAvailableChar = false;
					}

					if( nAboveVowelsCount >= 2 )		// Up to 2 upper vowels are allowed
						bCheckAvailableChar = false;
					
					// Only 1 upper vowel per group is allowed, and group 1 cannot be used if group 2 is present. 0xE33 can be entered even if a group 2 vowel is present.
					if( ( nVowelIndex >= s_nAboveVowelsIn1stClassStartIndex && nVowelIndex < s_nAboveVowelsIn2ndClassStartIndex ) && 
						( nAboveVowelsIn1stClassCount > 0 || ( nVowelIndex != 13 && nAboveVowelsIn2ndClassCount > 0 ) )	)
						bCheckAvailableChar = false;

					if( nVowelIndex >= s_nAboveVowelsIn2ndClassStartIndex && nAboveVowelsIn2ndClassCount > 0 )
						bCheckAvailableChar = false;
				}
			}
		}
		else	// If it is the first character
		{
			if( nVowelIndex >= s_nBehindVowelStartIndex )	// Back, up, and down vowels cannot be used alone
				bCheckAvailableChar = false;
		}
	}

	return bCheckAvailableChar;
}

int	CEtUIEditBox::GetPrevCaret()
{
	int nPrevCaret = 0;
	if( s_bCheckThaiLanguageVowelRule )
	{
		const WCHAR* pBuffer = m_Buffer.GetBuffer();
		if( pBuffer == NULL ) return false;
		for( int i=m_nCaret-1; i>=0; i-- )
		{
			int nTextIndex = CheckThaiLanguageVowel( pBuffer[i] );
			if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )	// Consonant, excluding upper/lower vowels, check only if 0xE33
			{
				nPrevCaret = i;
				break;
			}
		}
	}
	else
	{
		nPrevCaret = m_nCaret - 1;
	}

	return nPrevCaret;
}

int	CEtUIEditBox::GetNextCaret()
{
	int nNextCaret = 0;
	if( s_bCheckThaiLanguageVowelRule )
	{
		int nBufferSize = m_Buffer.GetTextSize();
		const WCHAR* pBuffer = m_Buffer.GetBuffer();
		if( pBuffer == NULL ) return false;
		for( int i=m_nCaret; i<nBufferSize; i++ )
		{
			int nTextIndex = CheckThaiLanguageVowel( pBuffer[i] );
			if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )	// Consonant, excluding upper/lower vowels, check only if 0xE33
			{
				int nVowels = 0;
				for( int j=i+1; j<nBufferSize; j++ )
				{
					int nTextIndex = CheckThaiLanguageVowel( pBuffer[j] );
					if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )
						break;
					else
						nVowels++;
				}
				nNextCaret = i + 1 + nVowels;
				break;
			}
		}
	}
	else
	{
		nNextCaret = m_nCaret + 1;
	}

	return nNextCaret;
}

int CEtUIEditBox::GetTextMoveCount( int nMoveCount, bool bForward )
{
	int nResultMoveCount = 0;
	if( s_bCheckThaiLanguageVowelRule )
	{
		const WCHAR* pBuffer = m_Buffer.GetBuffer();
		int nBufferSize = m_Buffer.GetBufferSize();
		if( pBuffer == NULL ) return 0;
		int nCount = 0;

		if( bForward )
		{
			for( int i=m_nFirstVisible; i<nBufferSize; i++, nResultMoveCount++ )
			{
				int nTextIndex = CheckThaiLanguageVowel( pBuffer[i] );
				if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )	// Consonant, excluding upper/lower vowels, check only if 0xE33
				{
					nCount++;
					if( nCount > nMoveCount )
						break;
				}
			}
		}
		else
		{
			for( int i=m_nCaret; i>=0; i--, nResultMoveCount++ )
			{
				int nTextIndex = CheckThaiLanguageVowel( pBuffer[i] );
				if( nTextIndex == -1 || nTextIndex < s_nBelowVowelStartIndex || nTextIndex == 13 )	// Consonant, excluding upper/lower vowels, check only if 0xE33
				{
					nCount++;
					if( nCount > nMoveCount )
						break;
				}
			}
		}
	}
	else
	{
		nResultMoveCount = nMoveCount;
	}

	return nResultMoveCount;
}