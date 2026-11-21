#include "StdAfx.h"
#include "DnNpcAnswerHtmlTextBox.h"
#include "DnTableDB.h"
#ifdef NPC_ANSWER_KEYBOARD_SELECT
#include "InputWrapper.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnNpcAnswerHtmlTextBox::CDnNpcAnswerHtmlTextBox(CEtUIDialog *pParent)
: CEtUIHtmlTextBox(pParent)
, m_nAnswerIndex(0)
, m_nSelectionSoundIndex(-1)
#ifdef NPC_ANSWER_ARROW
, m_nArrowLineIndex(0)
, m_fOffsetTime(0.0f)
#endif
#ifdef NPC_ANSWER_KEYBOARD_SELECT
, m_nKeyboardSelectLineIndex(0)
#endif
{
	// Giving a variable type like this resizes the NPC dialog based on choice count, but without exception handling it breaks.
	// To do this, it probably needs separate handling unlike tooltips.
	//m_Property.TextBoxProperty.VariableType = UI_TEXTBOX_HEIGHT;
}

CDnNpcAnswerHtmlTextBox::~CDnNpcAnswerHtmlTextBox(void)
{
	if( m_nSelectionSoundIndex == -1 ) 
	{
		CEtSoundEngine::GetInstance().RemoveSound( m_nSelectionSoundIndex );
	}
}

void CDnNpcAnswerHtmlTextBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIHtmlTextBox::Initialize( pProperty );

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10002 );
	if( strlen( szFileName ) > 0 )
	{
		m_nSelectionSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}
}

bool CDnNpcAnswerHtmlTextBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	if( ( WM_LBUTTONDOWN == uMsg ) && ( !m_bFocus ) )
	{
		m_pParent->RequestFocus( this );
	}

	int nOldPosition = m_ScrollBar.GetTrackPos();

	if( m_ScrollBar.HandleMouse( uMsg, fX, fY, wParam, lParam ) )
	{
		int nCurPosition = m_ScrollBar.GetTrackPos();
		Scroll( nCurPosition-nOldPosition );
		return true;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			if( IsInside( fX, fY ) )
			{
				m_bPressed = true;
				SetCapture( m_pParent->GetHWnd() );

				if( !m_bFocus )
				{
					m_pParent->RequestFocus( this );
				}

				return true;
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();

				if( IsInside( fX, fY ) )
				{
					UINT uMsgInner(WM_LBUTTONUP);
					if( wParam & MK_CONTROL )
					{
						uMsgInner |= 0x0100;
					}

					if( m_Property.TextBoxProperty.bRollOver )
					{
						if( SelectLine( fX, fY ) )
						{
							m_pParent->ProcessCommand( EVENT_TEXTBOX_SELECTION, true, this, uMsgInner );

							if( m_nSelectionSoundIndex != -1 )
							{
								CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSelectionSoundIndex );
							}
						}
					}
				}

				return true;
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			if( IsInside( fX, fY ) )
			{
				if( m_Property.TextBoxProperty.bRollOver )
				{
					SetMouseInLine( fX, fY );
				}
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			UINT uLines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			int nScrollPos = Scroll( -nScrollAmount );
			m_ScrollBar.ShowItem( nScrollPos );
			return true;
		}
		break;
	}

	return false;
}

#ifdef NPC_ANSWER_KEYBOARD_SELECT
bool CDnNpcAnswerHtmlTextBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{	
		return false;
	}

	if( !m_Property.TextBoxProperty.bVerticalScrollBar )
	{
		return false;
	}

	if( uMsg == WM_KEYDOWN )
	{
		if( m_nCurLine < 0 )
			return false;

		int nVecSize = (int)m_vecLine.size();
		SLineData LineData;

		bool bSelectMoved = false;
		if( GetKeyState(_ToVK(IW(IW_MOVEFRONT))) & 0x80 || GetKeyState(VK_UP) & 0x80 )
		{
			if( nVecSize == 0 )
				return false;

			// Move keyboard selection to the option directly above
			int nPrevLineIndex = m_vecLine[0]->GetLineData().m_nData;
			for( int i=0; i<nVecSize; i++ )
			{
				LineData = m_vecLine[i]->GetLineData();
				if( LineData.m_nData == m_nKeyboardSelectLineIndex )
					break;

				nPrevLineIndex = LineData.m_nData;
			}
			if( m_nKeyboardSelectLineIndex != nPrevLineIndex )
			{
				m_nKeyboardSelectLineIndex = nPrevLineIndex;
				bSelectMoved = true;
			}
		}
		else if( GetKeyState(_ToVK(IW(IW_MOVEBACK))) & 0x80 || GetKeyState(VK_DOWN) & 0x80 )
		{
			// Move keyboard selection to the option directly below
			bool bSelectNextLineIndex = false;
			for( int i=0; i<nVecSize; i++ )
			{
				LineData = m_vecLine[i]->GetLineData();

				if( bSelectNextLineIndex && LineData.m_nData != m_nKeyboardSelectLineIndex )
				{
					m_nKeyboardSelectLineIndex = LineData.m_nData;
					bSelectMoved = true;
					break;
				}

				if( LineData.m_nData == m_nKeyboardSelectLineIndex )
					bSelectNextLineIndex = true;
			}
		}
		else if( GetKeyState(_ToVK(IW(IW_JUMP))) & 0x80 || GetKeyState(VK_SPACE) & 0x80)
		{
			int nCount(0);
			for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
			{
				if( nCount >= m_nVisibleCount )
					break;

				LineData = m_vecLine[i]->GetLineData();
				if( LineData.m_nData == m_nKeyboardSelectLineIndex )
				{
					if( (int)m_vecLine[i]->m_vecWord.size() > 0 )
					{
						m_vecLine[i]->Select(true);
						m_vecLine[i]->SetMouseInLine(false);

						UINT uMsgInner( WM_KEYDOWN );
						m_pParent->ProcessCommand( EVENT_TEXTBOX_SELECTION, true, this, uMsgInner );

						if( m_nSelectionSoundIndex != -1 )
						{
							CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSelectionSoundIndex );
						}
						break;
					}
				}
			}
		}

		if( bSelectMoved )
		{
			// Recalculate ArrowLineIndex since there may be multiple lines.
			for( int i=0; i<nVecSize; i++ )
			{
				LineData = m_vecLine[i]->GetLineData();
				if( LineData.m_nData == m_nKeyboardSelectLineIndex )
				{
					m_nArrowLineIndex = i;
					break;
				}
			}

			// Time when scrolling is needed.
			if( m_nArrowLineIndex < m_nCurLine )
			{
				int nScroll = (m_nCurLine - m_nArrowLineIndex + 1) / 2;	// +1 is a magic value to make it look more natural
				for( int i = 0; i < nScroll; ++i )
					ScrollLineUp();
			}
			else if( m_nCurLine + m_nVisibleCount - 1 < m_nArrowLineIndex + 1 )	// Do not catch on the last line, make sure the line above is visible too.
			{
				int nScroll = (m_nArrowLineIndex + 1 - (m_nCurLine + m_nVisibleCount - 1) + 1) / 2;
				for( int i = 0; i < nScroll; ++i )
					ScrollLineDown();
			}
		}
	}

	// Do not call base class CEtUITextBox::HandleKeyboard. Handled completely separately here.
	return false;
}
#endif

void CDnNpcAnswerHtmlTextBox::BeginParse(DWORD dwAppData, bool &bAbort)
{
	CEtUIHtmlTextBox::BeginParse(dwAppData, bAbort);
	m_nAnswerIndex = 0;

	// Done in advance in case a BR (newline) appears at the very beginning and line data needs to be set.
	SLineData LineData;
	LineData.m_nData = m_nAnswerIndex;
	SetLineData(LineData);

#ifdef NPC_ANSWER_ARROW
	// Also initialize Arrow on initial parse
	m_nArrowLineIndex = 0;
#endif
#ifdef NPC_ANSWER_KEYBOARD_SELECT
	m_nKeyboardSelectLineIndex = 0;
#endif
}

void CDnNpcAnswerHtmlTextBox::StartTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort)
{
	if( pTag )
	{
		static CStringW strStartTag;
		strStartTag = pTag->getTagName();

		if( strStartTag.CompareNoCase(L"font") == 0 )
		{
			CLiteHTMLAttributes *pAtt = const_cast<CLiteHTMLAttributes*>(pTag->getAttributes());
			if( pAtt )
			{
				for( int i=0; i<pAtt->getCount(); i++ )
				{
					CLiteHTMLElemAttr &pElemAttr = pAtt->getAttribute(i);
					if( pElemAttr.isColorValue() )
					{
						// Not sure why, but HTML color seems to have R and B swapped..
						COLORREF clr = pElemAttr.getColorValue();
						BYTE b = GetRValue(clr);
						BYTE g = GetGValue(clr);
						BYTE r = GetBValue(clr);

						m_dwTextColor = D3DCOLOR_XRGB(r,g,b);
						break;
					}
				}
			}
		}
		else if( strStartTag.CompareNoCase(L"br") == 0 )
		{
			++m_nAnswerIndex;
			SLineData LineData;
			LineData.m_nData = m_nAnswerIndex;
			SetLineData(LineData);

			AddLine();
		}
		else if( strStartTag.CompareNoCase(L"p") == 0 )
		{
			// Never seen p before, but since it's a newline, handle it anyway.
			++m_nAnswerIndex;
			SLineData LineData;
			LineData.m_nData = m_nAnswerIndex;
			SetLineData(LineData);

			AddLine();
		}
		else if (strStartTag.CompareNoCase(L"div") == 0)
		{
			CLiteHTMLAttributes *pAtt = const_cast<CLiteHTMLAttributes*>(pTag->getAttributes());
			if( pAtt )
			{
				for (int i = 0; i < pAtt->getCount(); ++i)
				{
					//	todo : vertical center align
					CLiteHTMLElemAttr &pElemAttr = pAtt->getAttribute(i);
					if (pElemAttr.getValue().CompareNoCase(L"center") == 0)
						m_Property.TextBoxProperty.AllignHori = AT_HORI_CENTER;
				}
			}
		}
	}

	bAbort = false;
}

void CDnNpcAnswerHtmlTextBox::EndTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort)
{
	if( pTag )
	{
		static CStringW strEndTag;
		strEndTag = pTag->getTagName();

		if( strEndTag.CompareNoCase(L"font") == 0 )
		{
			//m_dwTextColor = textcolor::NPC_SPEECH;
			m_dwTextColor = m_dwDefaultTextColor;
		}
		else if( strEndTag.CompareNoCase(L"p") == 0 )
		{
			// Never seen p before, but since it's a newline, handle it anyway.
			++m_nAnswerIndex;
			SLineData LineData;
			LineData.m_nData = m_nAnswerIndex;
			SetLineData(LineData);

			AddLine();
		}
		else if (strEndTag.CompareNoCase(L"div") == 0)
		{
			m_Property.TextBoxProperty.AllignHori = AT_HORI_NONE;
		}
	}

	bAbort = false;
}

void CDnNpcAnswerHtmlTextBox::Characters(const CStringW &rText, DWORD dwAppData, bool &bAbort)
{
	// If a tag like Font is in the middle of a line, remaining trailing characters are AppendText'd separately, so line data may be split.
	// For example, when structured as below
	// <HTML>[Event] <FONT color=#086171>Get Smarter with Biscuit!</FONT><BR>[Event] <FONT color=#086171>Irene's Etiquette Class</FONT>[In Progress]<BR>(To Top)</HTML>
	// [Event] 
	// Get smart with Biscuit! (since font color changed)
	// AppendText is called twice like this, and since ClearLineData is called on the first AppendText, re-set the line data.
	SLineData LineData;
	LineData.m_nData = m_nAnswerIndex;
	SetLineData(LineData);

	// NPC dialogue enters here.
	// Because it usually comes in this format ([Quest]<BR>[ShopTrade])
	// Increment index on each entry and use it as line data.
	//
	// However, there is also data like this.
	// ([Lv2] <Font tag...> Hound's Nutrient Food <BR> [Lv3] omitted..)
	// That is, since Lv2 and Hound's Nutritious Food can arrive separately, increment line data only on BR (newline) to distinguish them.
	CEtUIHtmlTextBox::Characters(rText, dwAppData, bAbort);
}

void CDnNpcAnswerHtmlTextBox::EndParse(DWORD dwAppData, bool bIsAborted)
{
	CEtUIHtmlTextBox::EndParse(dwAppData, bIsAborted);
	m_nAnswerIndex = 0;
	ClearLineData();
}

void CDnNpcAnswerHtmlTextBox::RenderText( float fElapsedTime )
{
	if( m_nCurLine == -1 )
		return;

	SUICoord sLineCoord;
	int nCount(0);
	int nVecSize = (int)m_vecLine.size();

	// Pre-fetch line data of the rolled-over line,
	int nSelectedLineData = -1;

	SLineData LineData;
	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;

		if( m_Property.TextBoxProperty.bRollOver )
		{
			if( IsMouseEnter() && m_vecLine[i]->IsMouseInLine() )
			{
				LineData = m_vecLine[i]->GetLineData();
				nSelectedLineData = LineData.m_nData;
#ifdef NPC_ANSWER_KEYBOARD_SELECT
				m_nKeyboardSelectLineIndex = nSelectedLineData;
#endif
			}
		}
	}

	nCount = 0;
	// Draw lines with identical line data in select style.
	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;

		if( m_Property.TextBoxProperty.bRollOver )
		{
			LineData = m_vecLine[i]->GetLineData();
#ifdef NPC_ANSWER_KEYBOARD_SELECT
			if( LineData.m_nData == m_nKeyboardSelectLineIndex )
#else
			if( LineData.m_nData == nSelectedLineData )
#endif
			{
				RenderLine( fElapsedTime, m_vecLine[i], true );
				continue;
			}
		}
		RenderLine( fElapsedTime, m_vecLine[i] );
		//m_pParent->DrawRect( m_vecLine[i].m_uiCoord, EtInterface::debug::BLUE );
	}

#ifdef NPC_ANSWER_ARROW
	// Compute the line where the arrow will be placed.
	if( nSelectedLineData != -1 )
	{
		// Unlike above, start from 0 instead of m_CurLine.
		// This hides the arrow when the top line scrolls out of view for multi-line replies.
		for( int i=0; i<nVecSize; i++ )
		{
			LineData = m_vecLine[i]->GetLineData();
			if( LineData.m_nData == nSelectedLineData )
			{
				m_nArrowLineIndex = i;
				break;
			}
		}
	}
#endif
}

#ifdef NPC_ANSWER_ARROW
void CDnNpcAnswerHtmlTextBox::Render( float fElapsedTime )
{
	CEtUIHtmlTextBox::Render( fElapsedTime );

	// Arrow rendering
	SUIElement *pArrowElement = GetElement(3);
	if( m_nCurLine != -1 && pArrowElement ) {
		SUICoord sCoord;
		int nCount(0);
		int nVecSize = (int)m_vecLine.size();
		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount ) {
				// When not drawn
				m_fOffsetTime = 0.0f;
				break;
			}

			if( i == m_nArrowLineIndex )
			{
				// When drawn
				m_fOffsetTime += fElapsedTime * 1000;

				int nOffsetX = 0;
				float OFFSET_VALUE = 4.0f;	// How far it moves.
				int MOVE_TIME = 300;		// Time to move once from left to right.
				float BLEND_RANGE = 1.0f;
				int nTemp = (int)m_fOffsetTime%(MOVE_TIME+MOVE_TIME);
				if( nTemp < MOVE_TIME )
					nOffsetX = int(( BLEND_RANGE / (float)MOVE_TIME ) * nTemp * 255.0f);
				else
					nOffsetX = int(( BLEND_RANGE / (float)MOVE_TIME ) * ( (MOVE_TIME+MOVE_TIME) - nTemp ) * 255.0f);
				nOffsetX = min(nOffsetX, 255);

				// Convert value 0~255 to range -OFFSET_VALUE ~ +OFFSET_VALUE.
				float fOffsetResult = ((nOffsetX / 255.0f) - 0.5f) * OFFSET_VALUE / DEFAULT_UI_SCREEN_WIDTH;
				sCoord = m_vecLine[i]->m_uiCoord;
				sCoord.fX = sCoord.fX - 11.0f / DEFAULT_UI_SCREEN_WIDTH + fOffsetResult;
				sCoord.fY = sCoord.fY + (sCoord.fHeight-pArrowElement->fTextureHeight)/2.0f + 4.0f / DEFAULT_UI_SCREEN_HEIGHT;
				sCoord.fWidth = pArrowElement->fTextureWidth;
				sCoord.fHeight = pArrowElement->fTextureHeight;
				if( m_bExistTemplateTexture )
					m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pArrowElement->TemplateUVCoord, 0xffffffff, sCoord );
				else
					m_pParent->DrawSprite( pArrowElement->UVCoord, 0xffffffff, sCoord );
				break;
			}
		}
	}
}

void CDnNpcAnswerHtmlTextBox::RenderWord( float fElapsedTime, CWord& sWord, bool bRollOver, int nBorderFlag )
{
	std::wstring strRenderString;
	SUICoord uiRenderCoord;

	strRenderString = sWord.m_strWord;
	uiRenderCoord = sWord.m_sProperty.uiCoord;

	// Properly, when calling AddText on the text box, the line text area should be calculated minus the arrow width, but
	// Doing this would require rewriting everything from AddText to UpdateRect.
	// Rather than doing that, since the right edge of NPC dialog is naturally unclear anyway,
	// Shift only the text to the right when rendering.
	SUIElement *pArrowElement = GetElement(3);
	if( pArrowElement )
	{
		uiRenderCoord.fX += pArrowElement->fTextureWidth * 0.66f;
		uiRenderCoord.fWidth += pArrowElement->fTextureWidth * 0.66f;
	}

	if( sWord.m_sProperty.bSymbol )
	{
		strRenderString += L"...";
		uiRenderCoord.fWidth += m_fSymbolWidth;
	}

	UI_CONTROL_STATE currentState = UI_STATE_NORMAL;

	if( !IsShow() )
	{
		currentState = UI_STATE_HIDDEN;
	}
	else if( !IsEnable() )
	{
		currentState = UI_STATE_DISABLED;
	}

	sWord.m_sProperty.uiColor.Blend( currentState, fElapsedTime, m_fBlendRate );

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	if( bRollOver )
	{
		//m_pParent->DrawDlgText( strRenderString.c_str(), pElement, textcolor::ORANGE, uiRenderCoord, -1, -1, true );
		m_pParent->DrawDlgText( strRenderString.c_str(), pElement, sWord.m_sProperty.uiColor.dwCurrentColor, uiRenderCoord, -1, -1, true, 0.f, sWord.m_sProperty.BgColor, nBorderFlag );
	}
	else
	{
		m_pParent->DrawDlgText( strRenderString.c_str(), pElement, sWord.m_sProperty.uiColor.dwCurrentColor, uiRenderCoord, -1, -1, true, 0.f, sWord.m_sProperty.BgColor, nBorderFlag );
	}

	//m_pParent->DrawRect( uiRenderCoord, EtInterface::debug::RED );
}
#endif

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
void CDnNpcAnswerHtmlTextBox::GetLinePos( std::vector<EtVector2> &vecSmartMovePos )
{
	if( m_nCurLine == -1 )
		return;

	SUICoord sLineCoord;
	int nCount(0);
	int nVecSize = (int)m_vecLine.size();

	float fX, fY;
	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;
		fX = m_pParent->GetXCoord() + m_vecLine[i]->m_uiCoord.fX + GetUICoord().fWidth / 2.0f;
		fY = m_pParent->GetYCoord() + m_vecLine[i]->m_uiCoord.fY + m_vecLine[i]->m_uiCoord.fHeight / 2.0f + 4.0f/DEFAULT_UI_SCREEN_HEIGHT;	// Since rendering offsets y, offset it here as well.
		vecSmartMovePos.push_back( EtVector2(fX, fY) );
	}
}
#endif