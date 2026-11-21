#include "StdAfx.h"
#include "EtUITextBox.h"
#include "EtUIDialog.h"
#include "StringUtil.h"
#include "EtUINameLinkMgr.h"
#include "EternityEngine.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUITextBox::CEtUITextBox( CEtUIDialog *pParent )
	: CEtUIControl( pParent )
	, m_nStartLine(-1)
	, m_nEndLine(-1)
	, m_nCurLine(-1)
	, m_nVisibleCount(0)
	, m_bAutoScroll(true)
	, m_fSymbolWidth(0.0f)
	, m_fTextMargin(0.0f)
	, m_ScrollBar( pParent )
	, m_bUseWordBreak(true)
	, m_pNextPageTextBox(NULL)
	, m_eNextPageCondition(NextPage_DlgScreenHeight)
	, m_nNextPageLineCount(0)
	, m_nAdjustValue(0)
	, m_bAddTextToNextPage(false)
	, m_bNextPage(false)
	, m_nMaxLine(200)
	, m_RenderTextColor(0)
	, m_bUseSelect(false)
{
	m_Property.UIType = UI_CONTROL_TEXTBOX;
#ifdef TEXTBOX_RENDERLOCK
	m_nRenderingState = 0;
#endif
}

CEtUITextBox::~CEtUITextBox(void)
{
	SAFE_DELETE_PVEC( m_vecLine );
}

void CEtUITextBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.TextBoxProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.TextBoxProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	// Note : Calculate the size of the symbol.
	CalcSymbolTextRect();

	m_ScrollBar.SetTrackRange( 0, 0 );

	if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_HEIGHT ||
		m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
	{
		// Note : Ignore the scrollbar if it is a variable-length text box.
		m_Property.TextBoxProperty.bVerticalScrollBar = false;
	}

	SUIElement *pElement = GetElement(0);
	if( pElement )
	{
		pElement->ShadowFontColor.dwCurrentColor = pElement->ShadowFontColor.dwColor[UI_STATE_NORMAL];
	}
}

void CEtUITextBox::Render( float fElapsedTime )
{
	//if( !IsShow() )
	//	return;

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord, 0.0f, m_fZValue );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord, 0.0f, m_fZValue );
	// Note : Added for debugging
	//m_pParent->DrawRect( m_Property.UICoord, EtInterface::debug::BLUE );

#ifdef TEXTBOX_RENDERLOCK
	InterlockedExchange( &m_nRenderingState, 1 );
#endif
	RenderText( fElapsedTime );
#ifdef TEXTBOX_RENDERLOCK
	InterlockedExchange( &m_nRenderingState, 0 );
#endif

	if( m_Property.TextBoxProperty.bVerticalScrollBar )
	{
		// Note : Variable for scrolling while holding down the mouse click
		int nOldPosition=0, nCurPosition;

		if( !m_ScrollBar.IsArrowStateClear() )
		{
			nOldPosition = m_ScrollBar.GetTrackPos();
		}
		
		m_ScrollBar.Render( fElapsedTime );

		if( !m_ScrollBar.IsArrowStateClear() )
		{
			nCurPosition = m_ScrollBar.GetTrackPos();
			Scroll( nCurPosition-nOldPosition );
		}
	}
}

void CEtUITextBox::RenderText( float fElapsedTime )
{
	if( m_nCurLine == -1 )
		return;

	SUICoord sLineCoord;
	int nCount(0);
	int nVecSize = (int)m_vecLine.size();

	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;

		if( m_Property.TextBoxProperty.bRollOver )
		{
			if( IsMouseEnter() && m_vecLine[i]->IsMouseInLine() )
			{
				//sLineCoord = m_vecLine[i].m_uiCoord;
				//sLineCoord.fWidth = m_Property.UICoord.fWidth-m_Property.TextBoxProperty.fScrollBarSize;
				//m_pParent->DrawRect( sLineCoord, textcolor::DARKGRAY );

				RenderLine( fElapsedTime, m_vecLine[i], true );
				continue;
			}
			else if( m_vecLine[i]->IsSelected() )
			{
				RenderLine( fElapsedTime, m_vecLine[i], true );
				continue;
			}
		}

		RenderLine( fElapsedTime, m_vecLine[i] );
		//m_pParent->DrawRect( m_vecLine[i].m_uiCoord, EtInterface::debug::BLUE );
	}
}

void CEtUITextBox::RenderLine( float fElapsedTime, CLine* sLine, bool bRollOver )
{
	VECWORD & vecWord = sLine->m_vecWord;
#ifdef TEXTBOX_RENDERLOCK
	
	// To properly lock rendering, instead of doing it like now
	// Before starting rendering, must check if text addition functions like AddText, SetText, AppendText have completely stopped.
	// However, to do this,
	// Need to add a state indicating Adding is in progress to m_nRenderingState (values 0, 1) which only checks rendering state, and
	// Must also lock before and after the render call.
	// Since render lock is heavily used now, choose this method to minimize locks without causing dumps.
	int nBorderFlag = 0;
	for( int i=0; i<(int)vecWord.size(); i++ )
#else
	int nWordSize = (int)vecWord.size();

	for( int i=0; i<nWordSize; i++ )
#endif
	{
		if( vecWord[i].m_sProperty.BgColor )
		{
			nBorderFlag = 0;
			if( i == 0 ) nBorderFlag |= 1;
			if( i == (int)vecWord.size()-1 ) nBorderFlag |= 2;
			nBorderFlag |= 4;
			nBorderFlag |= 8;
		}
		RenderWord( fElapsedTime, vecWord[i], bRollOver, nBorderFlag );
	}
}

void CEtUITextBox::RenderWord( float fElapsedTime, CWord& sWord, bool bRollOver, int nBorderFlag )
{
	std::wstring strRenderString;
	SUICoord uiRenderCoord;

	strRenderString = sWord.m_strWord;
	uiRenderCoord = sWord.m_sProperty.uiCoord;

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

	uiRenderCoord.fX += pElement->fFontHoriOffset;
	uiRenderCoord.fY += pElement->fFontVertOffset;

	if( bRollOver )
	{
		m_pParent->DrawDlgText( strRenderString.c_str(), pElement, sWord.m_sProperty.uiColor.dwCurrentColor, uiRenderCoord, -1, -1, true, m_fZValue, sWord.m_sProperty.BgColor, nBorderFlag );
	}
	else
	{
		m_pParent->DrawDlgText( strRenderString.c_str(), pElement, sWord.m_sProperty.uiColor.dwCurrentColor, uiRenderCoord, -1, -1, true, m_fZValue, sWord.m_sProperty.BgColor, nBorderFlag );
	}

	//m_pParent->DrawRect( uiRenderCoord, EtInterface::debug::RED );
}

void CEtUITextBox::UpdateRects()
{
	//UpdateText();
	UpdateAlignText();

	if( m_Property.TextBoxProperty.bVerticalScrollBar )
	{
		if( m_Property.TextBoxProperty.bLeftScrollBar )
		{
			m_ScrollBar.SetPosition( m_Property.UICoord.fX, m_Property.UICoord.fY );
		}
		else
		{
			m_ScrollBar.SetPosition( m_Property.UICoord.Right()-m_Property.TextBoxProperty.fScrollBarSize, m_Property.UICoord.fY );
		}
		
		m_ScrollBar.SetSize( m_Property.TextBoxProperty.fScrollBarSize, m_Property.UICoord.fHeight );
	}
}

void CEtUITextBox::UpdateText()
{
	if( !UpdateVisibleCount() )
		return;

	float fX = m_Property.UICoord.fX;
	float fY = m_Property.UICoord.fY;

	if( m_Property.TextBoxProperty.bVerticalScrollBar && m_Property.TextBoxProperty.bLeftScrollBar )
	{
		fX += m_Property.TextBoxProperty.fScrollBarSize + 0.004f;
	}

	fY += m_fTextMargin;

	int nCount(0);
	int nVecSize = (int)m_vecLine.size();

	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;

		m_vecLine[i]->UpdatePos( fX, fY );
		fY += m_vecLine[i]->m_uiCoord.fHeight;
	}
}

void CEtUITextBox::UpdateText( float fX, float fY )
{
	//if( !UpdateVisibleCount() )
	//	return;

	if( m_Property.TextBoxProperty.bVerticalScrollBar && m_Property.TextBoxProperty.bLeftScrollBar )
	{
		fX += m_Property.TextBoxProperty.fScrollBarSize + 0.004f;
	}

	fY += m_fTextMargin;

	int nCount(0);
	int nVecSize = (int)m_vecLine.size();

	for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
	{
		if( nCount >= m_nVisibleCount )
			break;

		m_vecLine[i]->UpdatePos( fX, fY );
		fY += m_vecLine[i]->m_uiCoord.fHeight;
	}
}

bool CEtUITextBox::UpdateVisibleCount()
{
	if( m_vecLine.empty() )
		return false;

	if( m_Property.TextBoxProperty.AllignVert == AT_VERT_BOTTOM )
	{
		m_nVisibleCount = (int)(m_Property.UICoord.fHeight/m_vecLine[0]->m_uiCoord.fHeight);
	}
	else
	{
		// Unlike BOTTOM, instead of calculating using the entire control Height,
		// Must calculate according to the number of existing lines.
		float fUIHeight, fLineHeight(0.0f);
		fUIHeight = m_Property.UICoord.fHeight;

		int nCount(0);
		for( int i=m_nCurLine; i<(int)m_vecLine.size(); i++, nCount++ )
		{
			fLineHeight += m_vecLine[i]->m_uiCoord.fHeight;

			if( fLineHeight > fUIHeight )
				break;
		}

		m_nVisibleCount = nCount;
	}

	m_ScrollBar.SetPageSize( m_nVisibleCount );
	return true;
}

void CEtUITextBox::SetText( const LPCWSTR szText, const D3DCOLOR TextColor )
{
	ASSERT( szText );

	std::wstring strTemp(szText), strSubStr;
	std::wstring::size_type textSize, begIdx, endIdx;

	textSize = strTemp.size();
	begIdx = 0;
	endIdx = strTemp.find_first_of(L"\n", begIdx);

	while( begIdx < textSize )
	{
		if( endIdx != std::wstring::npos )
		{
			strSubStr = strTemp.substr(begIdx, endIdx-begIdx);
			endIdx++;
		}
		else
		{
			strSubStr = strTemp.substr(begIdx);
		}

		if( strSubStr.empty() )
		{
			AddLine();
		}
		else
		{
			DoAddText(strSubStr.c_str(), L"", TextColor);
		}

		begIdx = endIdx;
		endIdx = strTemp.find_first_of(L"\n", begIdx);
	}
}

void CEtUITextBox::AddText( LPCWSTR szText, const D3DCOLOR TextColor, DWORD dwFormat, const D3DCOLOR BgColor )
{
	if( m_bAddTextToNextPage ) {
		m_pNextPageTextBox->AddText( szText, TextColor, dwFormat, BgColor );
		return;
	}

	AddLine();

	SENTENCE curSentence;
	SWORD_PROPERTY basicProperty;
	basicProperty.uiColor.dwColor[UI_STATE_NORMAL] = TextColor;
	basicProperty.uiColor.dwCurrentColor = TextColor;
	basicProperty.dwFormat = dwFormat;
	basicProperty.bAdd = false;
	basicProperty.BgColor = BgColor;

	const CEtUINameLinkMgr& nameLinkMgr = EtInterface::GetNameLinkMgr();
	nameLinkMgr.TranslateText(curSentence, basicProperty, szText);

	m_OriginalStrings.push_back(curSentence);
	if( (int)m_OriginalStrings.size() > m_nMaxLine ) {
		m_OriginalStrings.pop_front();
	}

	std::wstring translated;
	SENTENCE::const_iterator iter = curSentence.begin();
	for (; iter != curSentence.end(); ++iter)
	{
		const CWord& curWord = *iter;
		const SWORD_PROPERTY& prop = curWord.m_sProperty;
		if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_WIDTH ||
			m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
		{
			AppendTextV( curWord.m_strWord.c_str(), curWord.m_strWordWithTag.c_str(), prop.uiColor.dwCurrentColor, prop.dwFormat, true, prop.BgColor );
		}
		else
		{
			AppendTextF( curWord.m_strWord.c_str(), curWord.m_strWordWithTag.c_str(), prop.uiColor.dwCurrentColor, prop.dwFormat, true, prop.BgColor );
		}

		translated += L"\n";
		m_szText = translated.c_str();
	}

	ClearLineData();
}

void CEtUITextBox::DoAddText( LPCWSTR szText, LPCWSTR szTextWithTag, const D3DCOLOR TextColor, DWORD dwFormat, const D3DCOLOR BgColor )
{
	if( m_bAddTextToNextPage ) {
		m_pNextPageTextBox->DoAddText( szText, szTextWithTag, TextColor, dwFormat, BgColor );
		return;
	}

	AddLine();

	if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_WIDTH ||
		m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
	{
		AppendTextV( szText, szTextWithTag, TextColor, dwFormat, true, BgColor );
	}
	else
	{
		AppendTextF( szText, szTextWithTag, TextColor, dwFormat, true, BgColor );
	}

	ClearLineData();
}

void CEtUITextBox::AppendText( LPCWSTR szText, const D3DCOLOR TextColor, DWORD dwFormat, bool bAdd, const D3DCOLOR BgColor, LPCWSTR szTextWithTag )
{
	if( m_bAddTextToNextPage ) {
		m_pNextPageTextBox->AppendText( szText, TextColor, dwFormat, bAdd, BgColor, szTextWithTag );
		return;
	}

	if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_WIDTH ||
		m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
		AppendTextV( szText, szTextWithTag, TextColor, dwFormat, bAdd, BgColor );
	else
		AppendTextF( szText, szTextWithTag, TextColor, dwFormat, bAdd, BgColor );

	m_szText += szText;
	ClearLineData();
}

void CEtUITextBox::AppendTextF( const LPCWSTR szText, const LPCWSTR szTextWithTag, const D3DCOLOR TextColor, DWORD dwFormat, bool bAdd, const D3DCOLOR BgColor )
{
	if( CEtFontMng::s_bUseUniscribe )
	{
		if( m_vecLine.empty() )
		{
			AddLine();
		}

		SUIElement *pElement;
		pElement = GetElement(0);
		if( !pElement || !m_pParent )
			return;

		SUICoord sTextCoord;
		m_pParent->CalcTextRect( szText, pElement, sTextCoord );

		// Note : Calculate remaining space on the current line.
		float fRemainWidth = m_Property.UICoord.fWidth;
		if( m_Property.TextBoxProperty.bLeftScrollBar || m_Property.TextBoxProperty.bVerticalScrollBar )
			fRemainWidth -= m_Property.TextBoxProperty.fScrollBarSize;
		float fOriginWidth = fRemainWidth;
		CLine* pLine = GetEndLine();
		if( pLine )
			fRemainWidth -= pLine->m_uiCoord.fWidth;

		if( fRemainWidth <= 0.0f )
		{
			DoAddText( szText, szTextWithTag, TextColor, dwFormat );
			return;
		}

		SWORD_PROPERTY wordProperty;
		wordProperty.uiColor.dwColor[UI_STATE_NORMAL] = TextColor;
		wordProperty.uiColor.dwCurrentColor = TextColor;
		wordProperty.dwFormat = dwFormat;
		wordProperty.bAdd = bAdd;
		wordProperty.BgColor = BgColor;
		if( dwFormat & UITEXT_SYMBOL )
		{
			fRemainWidth -= m_fSymbolWidth;
		}

		vector<wstring> vecStrLine;
		int nMaxWidth;
		if( !CEtFontMng::GetInstance().GetWordBreakText( std::wstring( szText ), pElement->nFontIndex, pElement->nFontHeight, 
														fRemainWidth * m_pParent->GetScreenWidth(), vecStrLine, nMaxWidth, true, fOriginWidth * m_pParent->GetScreenWidth() ) )
			return;

		bool bAddLine = false;
		if( static_cast<int>( vecStrLine.size() ) > 1 )
			bAddLine = true;

		if( static_cast<int>( vecStrLine.size() ) > 0 && bAddLine && dwFormat & UITEXT_SYMBOL && !m_Property.TextBoxProperty.bVerticalScrollBar )
		{
			wordProperty.bSymbol = true;
			SetTooltipText( szText );
			AddWord( vecStrLine[0].c_str(), szTextWithTag, wordProperty );
		}
		else
		{
			for( int i=0; i<static_cast<int>( vecStrLine.size() ); i++ )
			{
				AddWord( vecStrLine[i].c_str(), szTextWithTag, wordProperty );
				if( bAddLine && i < static_cast<int>( vecStrLine.size() ) - 1 )
					AddLine();
			}
		}
	}
	else
	{
		if( m_vecLine.empty() )
		{
			AddLine();
		}

		SUIElement *pElement;
		pElement = GetElement(0);
		if( !pElement || !m_pParent )
			return;

		SUICoord sTextCoord;
		m_pParent->CalcTextRect( szText, pElement, sTextCoord );

		// Note : Calculate remaining space on the current line.
		float fRemainWidth = m_Property.UICoord.fWidth;
		fRemainWidth -= m_Property.TextBoxProperty.fScrollBarSize;
		CLine* pLine = GetEndLine();
		if( pLine )
			fRemainWidth -= pLine->m_uiCoord.fWidth;

		if( fRemainWidth <= 0.0f )
		{
			DoAddText( szText, szTextWithTag, TextColor, dwFormat );
			return;
		}

		SWORD_PROPERTY wordProperty;
		wordProperty.uiColor.dwColor[UI_STATE_NORMAL] = TextColor;
		wordProperty.uiColor.dwCurrentColor = TextColor;
		wordProperty.dwFormat = dwFormat;
		wordProperty.bAdd = bAdd;
		wordProperty.BgColor = BgColor;

		if( sTextCoord.fWidth > fRemainWidth )
		{
			std::wstring strText, strSub;
			strText = szText;

			if( dwFormat & UITEXT_SYMBOL )
			{
				fRemainWidth -= m_fSymbolWidth;
			}

			if( fRemainWidth <= 0.0f )
			{
				DoAddText(szText, szTextWithTag, TextColor, dwFormat);
				return;
			}

			int nTrail = 0;
			int nCaret = CEtFontMng::GetInstance().GetCaretFromCaretPos( strText.c_str(), pElement->nFontIndex, pElement->nFontHeight, 
																		 int(fRemainWidth*m_pParent->GetScreenWidth()), nTrail );

			if( m_bUseWordBreak )
			{
				// Apply word break if needed. Currently applied for English letters and digits regardless of domestic or overseas build.
				// Word break in FontMng varies by resolution and might not look clean anyway, so skip applying it for now.
				bool bFirstWordInLine = ( pLine && pLine->m_uiCoord.fWidth == 0.0f );
				nCaret = GetCaretWithWordBreak( strText, nCaret, bFirstWordInLine );
			}

			if( nCaret < (int)strText.size() )
			{
				if( dwFormat & UITEXT_SYMBOL )
				{
					wordProperty.bSymbol = true;
					SetTooltipText( strText.c_str() );
				}

				strSub = strText.substr(0,nCaret+nTrail);
				AddWord( strSub.c_str(), szTextWithTag, wordProperty );

				if( dwFormat & (UITEXT_CLIP|UITEXT_SYMBOL) )
					goto RETURN;
			}

			// Prevent infinite loop
			if( nCaret+nTrail == 0 && pLine && pLine->m_uiCoord.fWidth == 0.0f )
				goto RETURN;

			// Note : Continue adding the remaining string to the next line.
			AddLine();
			wstring strLeftString = strText.substr( nCaret+nTrail );
			AppendTextF(strLeftString.c_str(), szTextWithTag, TextColor, dwFormat, bAdd, BgColor);
		}
		else
		{
			AddWord( szText, szTextWithTag, wordProperty );
		}
	}

RETURN:

	UpdateAlignText();
}

void CEtUITextBox::AppendTextV( const LPCWSTR szText, const LPCWSTR szTextWithTag, const D3DCOLOR TextColor, DWORD dwFormat, bool bAdd, const D3DCOLOR BgColor )
{
	if( m_vecLine.empty() )
	{
		AddLine();
	}

	SWORD_PROPERTY wordProperty;
	wordProperty.uiColor.dwColor[UI_STATE_NORMAL] = TextColor;
	wordProperty.uiColor.dwCurrentColor = TextColor;
	wordProperty.bAdd = bAdd;
	wordProperty.BgColor = BgColor;
	//wordProperty.dwFormat = dwFormat;

	AddWord( szText, szTextWithTag, wordProperty );

	UpdateAlignText();
}

void CEtUITextBox::AddLine()
{
#ifdef TEXTBOX_RENDERLOCK
	while( m_nRenderingState == 1 ) {}
#endif

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	SUICoord sTextCoord;
	m_pParent->CalcTextRect( L"x", pElement, sTextCoord );

	CLine *sLine = new CLine;

	if( m_vecLine.empty() )
	{
		// Note : Set coordinates of the first line on initial addition
		//		Set both start line and current line indices to 0
		sLine->m_uiCoord = m_Property.UICoord;
		m_nStartLine = 0;
		m_nCurLine = 0;
	}
	else
	{
		CLine* sCurLine = GetEndLine();
		if (sCurLine)
		{
			sLine->m_uiCoord.fX = sCurLine->m_uiCoord.fX;
			sLine->m_uiCoord.fY = sCurLine->m_uiCoord.Bottom();
		}
	}

	sLine->m_uiCoord.fWidth = 0.0f;
	sLine->m_uiCoord.fHeight = sTextCoord.fHeight;
	sLine->SetLineSpace( m_Property.TextBoxProperty.fLineSpace );
	sLine->SetLineData( GetLineData() );

	m_vecLine.push_back( sLine );
	m_nEndLine++;

	if( m_nEndLine >= m_nMaxLine ) {
		SAFE_DELETE( m_vecLine[0] );
		m_vecLine.erase( m_vecLine.begin() );
		m_nEndLine--;
	}

	m_ScrollBar.SetTrackRange( 0, ( int )m_vecLine.size() );

	// Note : Update variable-length text box
	UpdateTextBox();
}

void CEtUITextBox::AddImage( WCHAR *wszIamgeName, int nWidth, int nHeight )
{
#ifdef TEXTBOX_RENDERLOCK
	while( m_nRenderingState == 1 ) {}
#endif

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	SUICoord sTextCoord;
	m_pParent->CalcTextRect( L"x", pElement, sTextCoord );

	std::string szTextureName;
	ToMultiString( wszIamgeName, szTextureName );
	EtTextureHandle hTexture = EternityEngine::LoadTexture( szTextureName.c_str() );
	if( !hTexture ) return;

	SUICoord sUVCoord;
	int nWidthTemp = (int)( m_Property.UICoord.fWidth * DEFAULT_UI_SCREEN_WIDTH );
	int nHeightTemp = (int)( ( ( sTextCoord.fHeight + m_Property.TextBoxProperty.fLineSpace )  ) * DEFAULT_UI_SCREEN_HEIGHT );
	int nCount = (int)( nHeight / nHeightTemp );
	float fUVHeight = ( 1.f / (float)nHeight ) * (float)( ( ( sTextCoord.fHeight + m_Property.TextBoxProperty.fLineSpace ) ) * DEFAULT_UI_SCREEN_HEIGHT );
	// If textures appear broken on older GPUs due to non-power-of-two dimensions, convert all textures to power-of-two and enable below.
	//float fUVHeight = ( 1.f / (float)hTexture->OriginalHeight() ) * (float)( ( ( sTextCoord.fHeight + m_Property.TextBoxProperty.fLineSpace ) ) * DEFAULT_UI_SCREEN_HEIGHT );
	fUVHeight = ( (int)( fUVHeight * hTexture->OriginalHeight() ) / (float)hTexture->OriginalHeight() );
	if( nHeight % nHeightTemp != 0 ) nCount++;

	float fCoordWidth = nWidth / (float)DEFAULT_UI_SCREEN_WIDTH;
	if( fCoordWidth > m_Property.UICoord.fWidth ) fCoordWidth = m_Property.UICoord.fWidth;

	for( int i=0; i<nCount; i++ ) {
		CImageLine *sLine = new CImageLine;

		if( m_vecLine.empty() )
		{
			// Note : Set coordinates of the first line on initial addition
			//		Set both start line and current line indices to 0
			sLine->m_uiCoord = m_Property.UICoord;
			m_nStartLine = 0;
			m_nCurLine = 0;
		}
		else
		{
			CLine* sCurLine = GetEndLine();
			if (sCurLine)
			{
				sLine->m_uiCoord.fX = sCurLine->m_uiCoord.fX;
				sLine->m_uiCoord.fY = sCurLine->m_uiCoord.Bottom();
			}
		}

		sLine->m_uiCoord.fWidth = fCoordWidth;
		sLine->m_uiCoord.fHeight = sTextCoord.fHeight;
		sLine->SetLineSpace( m_Property.TextBoxProperty.fLineSpace );
		sLine->SetLineData( GetLineData() );

		sUVCoord.fX = 0.f;
		sUVCoord.fY = fUVHeight * i;
		sUVCoord.fWidth = 1.f;
		// Fallback for legacy graphics cards.
		//sUVCoord.fWidth = (float)nWidth / (float)hTexture->OriginalWidth();
		sUVCoord.fHeight = fUVHeight;

		sLine->SetImage( (char*)szTextureName.c_str(), sUVCoord );

		m_vecLine.push_back( sLine );
		m_nEndLine++;

		if( m_nEndLine >= m_nMaxLine ) {
			SAFE_DELETE( m_vecLine[0] );
			m_vecLine.erase( m_vecLine.begin() );
			m_nEndLine--;
		}
	}
	SAFE_RELEASE_SPTR( hTexture );
	AddLine();

	m_ScrollBar.SetTrackRange( 0, ( int )m_vecLine.size() );

	// Note : Update variable-length text box
	UpdateTextBox();
}

void CEtUITextBox::AddWord( const LPCWSTR szText, const LPCWSTR szTextWithTag, SWORD_PROPERTY &wordProperty )
{
#ifdef TEXTBOX_RENDERLOCK
	while( m_nRenderingState == 1 ) {}
#endif

	SUICoord sTextCoord;
	m_pParent->CalcTextRect( szText, GetElement(0), sTextCoord );

	CLine* pEndLine = GetEndLine();
	if (pEndLine == NULL)
		return;

	CLine& sEndLine = *pEndLine;

	CWord sWord;
	sWord.m_strWord = szText;
	sWord.m_strWordWithTag = szTextWithTag;

	if( wordProperty.dwFormat & UITEXT_RIGHT )
	{
		wordProperty.uiCoord.fX = m_Property.UICoord.fWidth - sTextCoord.fWidth;
	}
	else if( wordProperty.dwFormat & UITEXT_CENTER )
	{
		// Even though calculated like this here,
		// Later when UpdateText is called, void CLine::UpdatePos( float fX, float fY ) is called for each line.
		// Position is recalculated once more in this function.
		wordProperty.uiCoord.fX = (m_Property.UICoord.fWidth - sTextCoord.fWidth)*0.5f;
	}
	else
	{
		wordProperty.uiCoord.fX = sEndLine.m_uiCoord.Right();
	}
	wordProperty.uiCoord.fY = sEndLine.m_uiCoord.fY+sEndLine.GetLineSpace();
	if( wordProperty.bSymbol ) sTextCoord.fWidth += m_fSymbolWidth;
	wordProperty.uiCoord.fWidth = sTextCoord.fWidth;
	wordProperty.uiCoord.fHeight = sTextCoord.fHeight;

	sWord.m_sProperty = wordProperty;
	sEndLine.m_vecWord.push_back( sWord );

	// Note : Set the line size to the greatest height of the text.
	if( sEndLine.m_uiCoord.fHeight < wordProperty.uiCoord.fHeight )
		sEndLine.m_uiCoord.fHeight = wordProperty.uiCoord.fHeight;

	if( wordProperty.dwFormat & (UITEXT_RIGHT|UITEXT_CENTER) )
	{
		// Note : When the word-align flag is set, the line length becomes the width of the text box.
		sEndLine.m_uiCoord.fWidth = m_Property.UICoord.fWidth;
	}
	else
	{
		sEndLine.m_uiCoord.fWidth += wordProperty.uiCoord.fWidth;
	}

	// Note : Update variable-length text box
	UpdateTextBox();
}

void CEtUITextBox::AddColorText( LPCWSTR szText, const D3DCOLOR TextColor, const D3DCOLOR BgColor, const D3DCOLOR DecreaseColor )
{
	DWORD dwFontColor = TextColor;
	std::wstring szStr = szText;

	if( DecreaseColor ) {
		dwFontColor = CalcDecreaseColor( dwFontColor, DecreaseColor );
	}

	while( !szStr.empty() )
	{
		bool bStringLF = true;
		std::wstring::size_type nPosInStr;
		std::wstring::size_type nPosInStr1 = szStr.find_first_of( L"\n" );	// Could be an actual newline character,
		std::wstring::size_type nPosInStr2 = szStr.find( L"\\n" );			// May be a newline sequence entered directly as a string. Use find search if two or more characters.

		// Process whichever comes first.
		if( nPosInStr1 < nPosInStr2 )
			bStringLF = false;
		nPosInStr = min(nPosInStr1, nPosInStr2);

		std::wstring szCurLine = szStr.substr(0, nPosInStr);
		AddText( L"" );

		while( !szCurLine.empty() )
		{
			std::wstring::size_type nLength = szCurLine.size();
			std::wstring::size_type nPosInLine = szCurLine.find_first_of( L"#" );
			std::wstring szCurWord = szCurLine.substr(0, nPosInLine);

			if( !szCurWord.empty() )
				AppendText( szCurWord.c_str(), dwFontColor, UITEXT_NONE, false, BgColor );

			if( nPosInLine != std::wstring::npos && nLength >= 2 )
			{
				wchar_t cColorValue = szCurLine[nPosInLine+1];
				switch(cColorValue)
				{
				case 'r':	dwFontColor = descritioncolor::RED;			break;	// Red
				case 'g':	dwFontColor = descritioncolor::GREEN;		break;	// Green
				case 'b':	dwFontColor = descritioncolor::DODGERBLUE;	break;	// Blue
				case 'y':	dwFontColor = descritioncolor::YELLOW1;		break;	// Yellow
				case 'e':	dwFontColor = descritioncolor::YELLOW2;		break;	// Yellow
				case 'v':	dwFontColor = descritioncolor::VIOLET;		break;	// Purple
				case 's':	dwFontColor = descritioncolor::SKY;			break;	// Sky
				case 'j':	dwFontColor = descritioncolor::ORANGE;		break;	// Orange
				case 'w':	dwFontColor = descritioncolor::WHITE;		break;	// White
				case 'h':	dwFontColor = descritioncolor::GREY;		break;	// Gray
				case 'd':	dwFontColor = TextColor;					break;	// Default color received as the first argument
				default:												break;	// Do nothing if written incorrectly.
				}
				if( DecreaseColor ) {
					dwFontColor = CalcDecreaseColor( dwFontColor, DecreaseColor );
				}
			}

			std::wstring::size_type nNextPos = (nLength >= 2) ? nPosInLine+2 : nPosInLine+1;
			szCurLine = szCurLine.substr(nNextPos);
			if( nPosInLine == std::wstring::npos )
				break;
		}

		szStr = szStr.substr(nPosInStr+(bStringLF?2:1));
		if( nPosInStr == std::wstring::npos )
			break;
	}
}

int CEtUITextBox::Scroll( int nScrollAmount )
{
	if( !IsScrollMode() || nScrollAmount == 0 )
		return 0;

	int nScrollPos(0);

	if( nScrollAmount < 0 )
	{
		nScrollAmount = -nScrollAmount;

		if( m_nCurLine >= nScrollAmount )
		{
			m_nCurLine -= nScrollAmount;
		}
		else
		{
			m_nCurLine = 0;
		}

		nScrollPos = m_nCurLine;
	}
	else if( nScrollAmount > 0 )
	{
		int nTemp = (int)m_vecLine.size() - m_nCurLine - m_nVisibleCount;
		
		if( nTemp >= nScrollAmount )
		{
			m_nCurLine += nScrollAmount;
		}
		else
		{
			m_nCurLine += nTemp;
		}

		nScrollPos = m_nCurLine+m_nVisibleCount-1;
	}

	UpdateText();
	return nScrollPos;
}

void CEtUITextBox::ScrollLineUp()
{
	if( !IsScrollMode() )
		return;

	if( m_nCurLine > 1 )
	{
		m_nCurLine -= 2;
	}
	else if( m_nCurLine > 0 )
	{
		m_nCurLine--;
	}

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine );
}

void CEtUITextBox::ScrollLineDown()
{
	if( !IsScrollMode() )
		return;

	int nTemp = (int)m_vecLine.size() - m_nCurLine;
	nTemp -= m_nVisibleCount;

	if( nTemp > 1 )
	{
		m_nCurLine += 2;
	}
	else if( nTemp > 0 )
	{
		m_nCurLine++;
	}

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine+m_nVisibleCount-1 );
}

void CEtUITextBox::ScrollPageUp()
{
	if( !IsScrollMode() )
		return;

	if( m_nCurLine > m_nVisibleCount )
	{
		m_nCurLine -= m_nVisibleCount;
	}
	else
	{
		m_nCurLine = 0;
	}

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine );
}

void CEtUITextBox::ScrollPageDown()
{
	if( !IsScrollMode() )
		return;

	int nTemp = (int)m_vecLine.size() - m_nCurLine;
	nTemp -= m_nVisibleCount;

	if( nTemp > m_nVisibleCount )
	{
		m_nCurLine += m_nVisibleCount;
	}
	else
	{
		m_nCurLine += nTemp;
	}

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine+m_nVisibleCount-1 );
}

void CEtUITextBox::ScrollHome()
{
	if( !IsScrollMode() )
		return;

	m_nCurLine = 0;

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine );
}

void CEtUITextBox::ScrollEnd()
{
	if( !IsScrollMode() )
		return;

	int nVecSize = (int)m_vecLine.size();
	m_nCurLine = nVecSize - m_nVisibleCount;

	UpdateText();

	m_ScrollBar.ShowItem( nVecSize-1 );
}

void CEtUITextBox::ScrollPageTurn()
{
	if( !IsScrollMode() || IsLastPage() )
		return;

	m_nCurLine += m_nVisibleCount;

	UpdateText();

	m_ScrollBar.ShowItem( m_nCurLine );
}

bool CEtUITextBox::IsLastPage()
{
	if( (int)m_vecLine.size()-m_nCurLine <= m_nVisibleCount )
	{
		return true;
	}

	return false;
}

void CEtUITextBox::SetLineSpace( float fSpace )
{
	m_Property.TextBoxProperty.fLineSpace = fSpace;

	int nVecSize = (int)m_vecLine.size();

	for( int i=0; i<nVecSize; i++ )
	{
		m_vecLine[i]->SetLineSpace( fSpace );
	}

	UpdateRects();
}

void CEtUITextBox::UpdateAlignText()
{
	if( !UpdateVisibleCount() )
		return;

	float fX = m_Property.UICoord.fX;
	float fY = m_Property.UICoord.fY;

	switch( m_Property.TextBoxProperty.AllignVert )
	{
	case AT_VERT_NONE:
		break;
	case AT_VERT_TOP:
		break;
	case AT_VERT_CENTER:
		{
			int nVecSize = (int)m_vecLine.size();
			if( nVecSize < m_nVisibleCount )
			{
				CLine* pLine = GetEndLine();
				if (pLine)
				{
					float fTemp = (m_nVisibleCount - nVecSize) * pLine->m_uiCoord.fHeight;
					fTemp *= 0.5f;
					fY += fTemp;
				}
			}
		}
		break;
	case AT_VERT_BOTTOM:
		{
			if( m_bAutoScroll )
			{
				int nVecSize = (int)m_vecLine.size();

				if( nVecSize > m_nVisibleCount )
				{
					m_nCurLine = nVecSize - m_nVisibleCount;
				}
				else
				{
					m_nCurLine = 0;
					CLine* pLine = GetEndLine();
					if (pLine)
						fY += (m_nVisibleCount - nVecSize) * pLine->m_uiCoord.fHeight;
				}

				// Note : For bottom alignment, calculate remaining space to pad at the top.
				//
				m_fTextMargin = m_Property.UICoord.fHeight-(m_vecLine[0]->m_uiCoord.fHeight*m_nVisibleCount);

				m_ScrollBar.ShowItem( nVecSize-1 );
			}
			else
			{
				m_ScrollBar.ShowItem( m_nCurLine + m_nVisibleCount - 1 );
			}
		}
		break;
	}

	UpdateText( fX, fY );

	switch( m_Property.TextBoxProperty.AllignHori )
	{
	case AT_HORI_NONE:
		break;
	case AT_HORI_LEFT:
		break;
	case AT_HORI_CENTER:
	case AT_HORI_RIGHT:
		{
			CLine *pLine(NULL);
			float fGap;
			
			SUICoord sTextBoxCoord;
			GetUICoord(sTextBoxCoord);

			int nVecSize = (int)m_vecLine.size();
			for( int i=0; i<nVecSize; i++ )
			{
				pLine = m_vecLine[i];
				fGap = sTextBoxCoord.fWidth - pLine->m_uiCoord.fWidth;

				if( m_Property.TextBoxProperty.bVerticalScrollBar )
				{
					if( !m_Property.TextBoxProperty.bLeftScrollBar )
					{
						fGap -= m_Property.TextBoxProperty.fScrollBarSize;
					}
				}

				if( m_Property.TextBoxProperty.AllignHori == AT_HORI_CENTER)
				{
					fGap *= 0.5f;
				}

				pLine->UpdatePos(pLine->m_uiCoord.fX+fGap, pLine->m_uiCoord.fY);
			}
		}
		break;
	}
}

void CEtUITextBox::ClearText()
{
	if( m_pNextPageTextBox && m_bAddTextToNextPage ) {
		m_pNextPageTextBox->ClearText();
		m_bAddTextToNextPage = false;
	}

	m_szText.clear();
	SAFE_DELETE_PVEC( m_vecLine );
//	m_vecLine.clear();

	m_nStartLine = -1;
	m_nEndLine = -1;
	m_nCurLine = -1;
	m_nVisibleCount = 0;
	m_ScrollBar.SetTrackRange( 0, 0 );
}

void CEtUITextBox::CalcSymbolTextRect()
{
	SUICoord sSymbolCoord;
	m_pParent->CalcTextRect( L"...", GetElement(0), sSymbolCoord );
	m_fSymbolWidth = sSymbolCoord.fWidth;
}

bool CEtUITextBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	if( !m_Property.TextBoxProperty.bVerticalScrollBar )
	{
		return false;
	}

	if( m_ScrollBar.HandleKeyboard( uMsg, wParam, lParam ) )
	{
		return true;
	}

	if( uMsg == WM_KEYDOWN )
	{
		switch( wParam )
		{
		case VK_UP:		ScrollLineUp();		return true;
		case VK_DOWN:	ScrollLineDown();	return true;
		case VK_PRIOR: 	ScrollPageUp();		return true;
		case VK_NEXT: 	ScrollPageDown();	return true;
		case VK_HOME: 	ScrollHome(); 		return true;
		case VK_END: 	ScrollEnd();		return true;
		}
	}

	return false;
}

bool CEtUITextBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
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
						if( SelectWord( fX, fY ) )
						{
							m_pParent->ProcessCommand( EVENT_TEXTBOX_SELECTION, true, this, uMsgInner );
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
					SetMouseInWord(fX, fY);
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

std::wstring CEtUITextBox::GetLineText(int nIndex)
{
	if(nIndex >= (int)m_vecLine.size())
		return L"";

	return m_vecLine[nIndex]->GetText();
}

CLine* CEtUITextBox::GetEndLine()
{
	ASSERT(m_nEndLine>=0);
	ASSERT(!m_vecLine.empty());
	// Added defensive code because it sometimes crashes here.
	// Later, if it passes AddLine and reaches return sLine, there is a problem.
	if( m_nEndLine < 0 || m_nEndLine >= (int)m_vecLine.size() ) {
		AddLine();
		if( m_nEndLine < 0 || m_nEndLine >= (int)m_vecLine.size() ) {
			return NULL;
		}
	}

	return m_vecLine[m_nEndLine];
}

void CEtUITextBox::SetMouseInLine( float fX, float fY )
{
	ClearMouseEnteredLine();

	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		SUICoord uiLineCoord;
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			uiLineCoord = m_vecLine[i]->m_uiCoord;
			uiLineCoord.fWidth = m_Property.UICoord.fWidth;

			if( uiLineCoord.IsInside( fX, fY ) )
			{
				if( (int)m_vecLine[i]->m_vecWord.size() > 0 )
				{
					m_vecLine[i]->SetMouseInLine(true);
				}

				return;
			}
		}
	}
}

void CEtUITextBox::SetMouseInWord( float fX, float fY )
{
	ClearMouseEnteredLine();

	m_WordMouseOver.Clear();
	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		SUICoord uiLineCoord;
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			uiLineCoord = m_vecLine[i]->m_uiCoord;
			uiLineCoord.fWidth = m_Property.UICoord.fWidth;

			if( uiLineCoord.IsInside( fX, fY ) )
			{
				if( (int)m_vecLine[i]->m_vecWord.size() > 0 )
				{
					m_vecLine[i]->SetMouseInLine(true);

					const VECWORD& wordList = m_vecLine[i]->m_vecWord;
					VECWORD::const_iterator iter = wordList.begin();
					for (; iter != wordList.end(); ++iter)
					{
						const CWord& word = *iter;
						if (word.m_sProperty.uiCoord.IsInside(fX, fY))
						{
							m_WordMouseOver = word;
							return;
						}
					}
				}

				return;
			}
		}
	}
}

int CEtUITextBox::GetMouseEnteredLine()
{
	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			if( m_vecLine[i]->IsMouseInLine() )
			{
				return i;
			}
		}
	}

	return -1;
}

bool CEtUITextBox::SelectLine( float fX, float fY )
{
	// Note : Select a line by mouse click.
	//		Only one line can be selected at a time.
	//		Multi-line selection must be implemented separately.
	ClearSelectedLine();

	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		SUICoord uiLineCoord;
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			// Note : Must add m_Property's Width to get the coordinates of the full line size.
			//		Otherwise it is limited to the visible text length.
			uiLineCoord = m_vecLine[i]->m_uiCoord;
			uiLineCoord.fWidth = m_Property.UICoord.fWidth;

			if( uiLineCoord.IsInside( fX, fY ) )
			{
				if( (int)m_vecLine[i]->m_vecWord.size() > 0 )
				{
					int nCurSelLine = GetMouseEnteredLine();
					if( nCurSelLine == i )
					{
						m_vecLine[nCurSelLine]->Select(true);
						m_vecLine[nCurSelLine]->SetMouseInLine(false);
						return true;
					}
				}

				return false;
			}
		}
	}

	return false;
}

bool CEtUITextBox::SelectWord(float fX, float fY)
{
	ClearSelectedLine();

	m_WordSelected.Clear();

	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		SUICoord uiLineCoord;
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			// Note : Must add m_Property's Width to get the coordinates of the full line size.
			//		Otherwise it is limited to the visible text length.
			uiLineCoord = m_vecLine[i]->m_uiCoord;
			uiLineCoord.fWidth = m_Property.UICoord.fWidth;

			if( uiLineCoord.IsInside( fX, fY ) )
			{
				if( (int)m_vecLine[i]->m_vecWord.size() > 0 )
				{
					int nCurSelLine = GetMouseEnteredLine();
					if( nCurSelLine == i )
					{
						m_vecLine[nCurSelLine]->Select(true);
						m_vecLine[nCurSelLine]->SetMouseInLine(false);

						const VECWORD& wordList = m_vecLine[nCurSelLine]->m_vecWord;
						VECWORD::const_iterator iter = wordList.begin();
						for (; iter != wordList.end(); ++iter)
						{
							const CWord& word = *iter;
							if (word.m_sProperty.uiCoord.IsInside(fX, fY))
							{
								m_WordSelected = word;
								return true;
							}
						}
					}
				}

				return false;
			}
		}
	}

	return false;
}

int CEtUITextBox::GetSelectedLineIndex( bool bClearSelection /* = true */ ) const
{
	if( m_nCurLine >= 0 )
	{
		int nCount(0);
		int nVecSize = (int)m_vecLine.size();

		for( int i=m_nCurLine; i<nVecSize; i++, nCount++ )
		{
			if( nCount >= m_nVisibleCount )
				break;

			if( m_vecLine[i]->IsSelected() )
			{
				if( bClearSelection )
					m_vecLine[i]->Select(false);

				return i;
			}
		}
	}

	return -1;
}

void CEtUITextBox::UpdateTextBoxHeight()
{
	if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_HEIGHT ||
		m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
	{
		float fLineHeight(0);

		int nVecSize = (int)m_vecLine.size();
		for( int i=0; i<nVecSize; i++ )
		{
			CLine *line = m_vecLine[i];
			fLineHeight += line->m_uiCoord.fHeight;
		}

		float fBeforeHeight, fAfterHeight;
		SUICoord sTextBoxCoord;
		GetUICoord(sTextBoxCoord);
		fBeforeHeight = sTextBoxCoord.fHeight;
		sTextBoxCoord.fHeight = fLineHeight;
		fAfterHeight = sTextBoxCoord.fHeight;
		SetUICoord(sTextBoxCoord);

		if( !m_bNextPage )
			m_pParent->UpdateDlgCoord( 0, 0, 0, (fAfterHeight-fBeforeHeight) );

		// Check whether NextPage should be used
		if( m_pNextPageTextBox )	// && m_bAddTextToNextPage == false ) Skip flag check since Remove can also shrink it.
		{
			if( m_eNextPageCondition == NextPage_DlgScreenHeight )
			{
				// Check like this for the first page,
				if( !m_bNextPage )
				{
					SUIElement *pElement = GetElement(0);
					if( pElement ) {
						SUICoord sTextCoord, sDlgCoord;
						m_pParent->CalcTextRect( L"x", pElement, sTextCoord );
						m_pParent->GetDlgCoord( sDlgCoord );
						if( sDlgCoord.fHeight + sTextCoord.fHeight > m_pParent->GetScreenHeightRatio() ) {
							m_bAddTextToNextPage = true;
							m_pNextPageTextBox->SetNextPageLineCount( GetLineSize(), m_nAdjustValue );
						}
					}
				}
				else
				{
					// Subsequent pages use configured values
					if( GetLineSize() >= m_nNextPageLineCount )
						m_bAddTextToNextPage = true;
				}
			}
		}
	}
}

void CEtUITextBox::UpdateTextBoxWidth()
{
	if( m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_WIDTH ||
		m_Property.TextBoxProperty.VariableType == UI_TEXTBOX_BOTH )
	{
		float fLineWidth(0);

		int nVecSize = (int)m_vecLine.size();
		for( int i=0; i<nVecSize; i++ )
		{
			CLine *line = m_vecLine[i];
			if( fLineWidth < line->m_uiCoord.fWidth )
			{
				// Note : Store the longest line.
				//
				fLineWidth = line->m_uiCoord.fWidth;
			}
		}

		float fBeforeWidth, fAfterWidth;
		SUICoord sTextBoxCoord;
		GetUICoord(sTextBoxCoord);
		fBeforeWidth = sTextBoxCoord.fWidth;
		sTextBoxCoord.fWidth = fLineWidth;
		fAfterWidth = sTextBoxCoord.fWidth;
		SetUICoord(sTextBoxCoord);

		m_pParent->UpdateDlgCoord(0, 0, (fAfterWidth-fBeforeWidth), 0);
	}
}

void CEtUITextBox::FitTextBoxToLine()
{
	if( m_vecLine.empty() )
		return;

	int nvecLineSize = (int)m_vecLine.size();
	int nTemp = nvecLineSize - m_nCurLine;
	if( nTemp < m_nVisibleCount )
	{
		int nLineHeight = (int)(m_vecLine[0]->m_uiCoord.fHeight*m_pParent->GetScreenHeight());
		
		SUICoord uiCoord;
		GetUICoord(uiCoord);
		uiCoord.fHeight = (nTemp*nLineHeight)/m_pParent->GetScreenHeight();
		SetUICoord(uiCoord);
	}
}

void CEtUITextBox::ClearMouseEnteredLine()
{
	int nCurSelLine = GetMouseEnteredLine();
	if( nCurSelLine != -1 )
	{
		m_vecLine[nCurSelLine]->SetMouseInLine(false);
	}
}

void CEtUITextBox::ClearSelectedLine()
{
	int nCurSelLine = GetSelectedLineIndex();
	if( nCurSelLine != -1 )
	{
		m_vecLine[nCurSelLine]->Select(false);
	}
}

void CEtUITextBox::UpdateTextBox()
{
	UpdateTextBoxWidth();
	UpdateTextBoxHeight();

	UpdateVisibleCount();
	
	//  [5/1/2009 nextome]
	// Must do this so the scrollbar is disabled initially if not needed.
	m_ScrollBar.SetPageSize( m_nVisibleCount );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecLine.size() );
	m_ScrollBar.UpdateRects();
}

bool CEtUITextBox::GetSelectedLineData( SLineData &sLineData, bool bClearSelect ) const
{
	int nCurSelLine = GetSelectedLineIndex(bClearSelect);
	if( nCurSelLine != -1 )
	{
		sLineData = m_vecLine[nCurSelLine]->GetLineData();
		return true;
	}

	return false;
}

const CWord& CEtUITextBox::GetSelectedWordData() const
{
	return m_WordSelected;
}

std::wstring CEtUITextBox::GetLastLineText()
{
	if( GetUsedNextPage() ) {
		return m_pNextPageTextBox->GetLastLineText();
	}

	if( m_vecLine.size() )
		return m_vecLine[m_vecLine.size()-1]->GetText();

	return L"";
}

bool CEtUITextBox::GetLastLineCoord( SUICoord &Coord )
{
	if( GetUsedNextPage() ) {
		return m_pNextPageTextBox->GetLastLineCoord( Coord );
	}

	if( m_vecLine.size() )
	{
		Coord = m_vecLine[m_vecLine.size()-1]->m_uiCoord;
		return true;
	}

	return false;
}

void CEtUITextBox::RemoveLastLine()
{
	if( GetUsedNextPage() ) {
		m_pNextPageTextBox->RemoveLastLine();
		return;
	}

	// Currently verified only in tooltips; testing is recommended before using elsewhere.
	if( m_vecLine.size() )
	{
		SAFE_DELETE( m_vecLine[m_vecLine.size() - 1] );
		m_vecLine.pop_back();

		m_nEndLine--;
		int nCurLine = m_nCurLine;

		std::wstring::size_type endIdx;
		endIdx = m_szText.find_last_of(L"\n");
		m_szText = m_szText.substr(0, endIdx);

		m_ScrollBar.SetTrackRange( 0, (int)m_vecLine.size() );

		UpdateTextBox();
	}
}

void CEtUITextBox::ResizeLineTextWithSymbol(int nLineSize , std::wstring wszSymbol)
{
	int nSize = GetLineSize();

	for(int i=0;i<nSize - nLineSize;i++)
		RemoveLastLine();

	std::wstring wszString = GetLineText(nLineSize-1);
	std::wstring wszSubString;

	wszSubString = wszString.substr(0, wszString.size() - wszSymbol.size() );
	wszSubString += wszSymbol;

	ClearText();
	AppendText( wszSubString.c_str() );
}


void CEtUITextBox::SetNextPageTextBox( CEtUITextBox *pControl, eNextPageCondition eCondition, int nAdjustValue )
{
	m_pNextPageTextBox = pControl;
	m_eNextPageCondition = eCondition;
	m_nAdjustValue = nAdjustValue;
	pControl->SetNextPage( true );
}

void CEtUITextBox::SetNextPageLineCount( int nCount, int nAdjustValue )
{
	m_nNextPageLineCount = nCount + nAdjustValue;
	if( m_pNextPageTextBox ) m_pNextPageTextBox->SetNextPageLineCount( nCount, nAdjustValue );
}

int CEtUITextBox::GetUsedNextPage()
{
	if( !m_pNextPageTextBox ) return 0;

	if( m_pNextPageTextBox->IsEmpty() )
		return 0;

	// Make it recursive so it works properly even when chaining 3 or 4 NextPages together.
	return m_pNextPageTextBox->GetUsedNextPage() + 1;
}

void CEtUITextBox::Process( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;

	if( !IsShow() )
	{
		m_CurrentState = UI_STATE_HIDDEN;
	}
	else if( !IsEnable() )
	{
		m_CurrentState = UI_STATE_DISABLED;
	}

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
}

int CEtUITextBox::GetCaretWithWordBreak( const std::wstring& strText, const int nOrigCaret, bool bFirstWordInLine )
{
	int nNewCaret = nOrigCaret;

	// After splitting the string by whitespace
	std::vector<std::wstring> tokens;
	TokenizeW( strText, tokens );

	// When purely whitespace, return nOrigCaret to handle as original.
	if ( tokens.size() == 0 )
		return nOrigCaret;
	// There are a few more exceptions to handle besides pure whitespace.
	// 1. A ... A (type A, add more than one line of spaces, then A)
	// 2. A                                    
	// 3.                                      A
	// 4. A (spaces, A, then spaces)
	// These will be line-broken without word break, as originally handled.


	// Check if '-' exists in the split string. If so, separate and insert. ex: knocked-down => knocked-, down
	for( std::vector<std::wstring>::iterator iter = tokens.begin(); iter != tokens.end(); ++iter )
	{
		std::wstring::size_type nPos;
		if(	(nPos = iter->find_first_of( L"-" )) != std::wstring::npos )
		{
			std::wstring wszWord1 = iter->substr(0, nPos+1);
			std::wstring wszWord2 = iter->substr(nPos+1);
			iter = tokens.erase(iter);
			iter = tokens.insert(iter, wszWord2);
			iter = tokens.insert(iter, wszWord1);
		}
	}

	// After calculating start and end positions for each split string
	std::vector<int> vecTokenBeginPos;
	int nBeginPos = 0;
	for(int i = 0; i < (int)tokens.size(); ++i)
	{
		// Count the number of spaces and add them as well. Check for spaces first since it may start with whitespace.
		int nSpaceCount = 0;
		while( strText[nBeginPos+nSpaceCount] == L' ' )
			++nSpaceCount;

		nBeginPos += nSpaceCount;

		vecTokenBeginPos.push_back(nBeginPos);
		nBeginPos += (int)tokens[i].size();
	}

	// If the initial whitespace already exceeds one line, just break the line at the original caret. Exception #3 handling.
	if( (int)vecTokenBeginPos.size() > 0 && nOrigCaret < vecTokenBeginPos[0] )
		return nOrigCaret;

	// If the first word itself is too long and exceeds one line, return Orig.
	// aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa Exceptional case like this.
	if( bFirstWordInLine && (int)vecTokenBeginPos.size() > 0 && vecTokenBeginPos[0] < nOrigCaret && nOrigCaret < vecTokenBeginPos[0]+(int)tokens[0].size() )
		return nOrigCaret;

	// Recalculate the whitespace after the first word,
	// If a word+space exceeding one line comes first, break at the original caret. Handle exceptions 1 and 2.
	int nFirstWordSpaceCount = 0;
	while( strText[vecTokenBeginPos[0]+(int)tokens[0].size()+nFirstWordSpaceCount] == L' ' )
		++nFirstWordSpaceCount;
	if( vecTokenBeginPos[0]+(int)tokens[0].size() < nOrigCaret &&
		nOrigCaret <= vecTokenBeginPos[0]+(int)tokens[0].size()+nFirstWordSpaceCount )
		return nOrigCaret;

	// Update between word boundaries
	std::wstring strWord;
	for(int i = 0; i < (int)vecTokenBeginPos.size(); ++i)
	{
		if( nNewCaret < vecTokenBeginPos[i] )
		{
			nNewCaret = vecTokenBeginPos[i-1];
			strWord = tokens[i-1];
			break;
		}
	}

	if( nNewCaret > vecTokenBeginPos[(int)vecTokenBeginPos.size()-1] )
	{
		nNewCaret = vecTokenBeginPos[(int)vecTokenBeginPos.size()-1];
		strWord = tokens[(int)vecTokenBeginPos.size()-1];
	}

	// Check whether conditions apply for word-break.
	/*
	bool bWordBreakApply1 = false;
	bool bWordBreakApply2 = false;
	bool bWordBreakApply3 = true;
	// Apply word-break even for number%.
	if( strWord.size() > 1 && strWord[(int)strWord.size()-1] == L'%' )
	{
		bWordBreakApply1 = true;
		for( int i = 0; i < (int)strWord.size()-1; ++i )
		{
			if( L'0' <= strWord[i] && strWord[i] <= L'9' )
			{
			}
			else
			{
				bWordBreakApply1 = false;
				break;
			}
		}
	}

	// Apply word-break for English (English., English,) as well.
	if( strWord.size() > 1 && (strWord[(int)strWord.size()-1] == L'.' || strWord[(int)strWord.size()-1] == L','))
	{
		bWordBreakApply2 = true;
		for( int i = 0; i < (int)strWord.size()-1; ++i )
		{
			if( ((L'a' <= strWord[i] && strWord[i] <= L'z') ||
				(L'A' <= strWord[i] && strWord[i] <= L'Z') )
			{
			}
			else
			{
				bWordBreakApply2 = false;
				break;
			}
		}
	}
	*/

	bool bWordBreakApply = true;

	// Checking English words ending with '.', ',', numbers ending with '%', etc. individually creates too many cases.
	// So just treat anything included in the character set as valid.
	// Because things like 3th, 4m also exist.
	for( int i = 0; i < (int)strWord.size(); ++i )
	{
		//if( iswalpha(strWord[i]) == false ) // Checking with iswalpha also returns Korean as alpha. Seems it's just iswletter.
		if( (L'a' <= strWord[i] && strWord[i] <= L'z') ||
			(L'A' <= strWord[i] && strWord[i] <= L'Z') ||
			(0x0400 <= strWord[i] && strWord[i] <= 0x052F) ||	// Added Russian language.
			(L'0' <= strWord[i] && strWord[i] <= L'9') ||
			strWord[i] == L'.' ||
			strWord[i] == L',' ||
			strWord[i] == L'*' ||
			strWord[i] == L'%' ||
			strWord[i] == L'-' ||
			strWord[i] == L'/' ||
			strWord[i] == L'(' ||
			strWord[i] == L')' ||
			strWord[i] == L'[' ||
			strWord[i] == L']' ||
			strWord[i] == L':' ||
			strWord[i] == L'!' ||
			strWord[i] == L'?' ||
			strWord[i] == L'\"' ||
			strWord[i] == L'\'' )	// I'll or target's
		{
		}
		else
		{
			bWordBreakApply = false;
			break;
		}
	}

	//if( bWordBreakApply1 || bWordBreakApply2 || bWordBreakApply3 )
	if( bWordBreakApply )
	{
		return nNewCaret;
	}

	return nOrigCaret;
}

void CEtUITextBox::FindInputPos( std::vector<EtVector2> &vecPos )
{
	CEtUIDialog *pDialog = GetParent();
	if( !pDialog ) return;

	if( m_vecLine.size() > 0 )
	{
		m_ScrollBar.FindInputPos( vecPos );
	}
}