#pragma once
#include "EtUIControl.h"
#include "EtUITextUtil.h"
#include "EtUIScrollBar.h"
#include "EtUITextBoxData.h"
#include "EtUINameLinkInterface.h"

using namespace EtInterface;

#define TEXTBOX_RENDERLOCK

class CEtUINameLinkMgr;
class CEtUITextBox : public CEtUIControl, public CEtUINameLinkInterface
{
public:
	CEtUITextBox( CEtUIDialog *pParent );
	virtual ~CEtUITextBox(void);

	enum eNextPageCondition
	{
		NextPage_DlgScreenHeight,	// If parent dialog height exceeds screen, move to next page. Currently only this is supported.
		//NextPage_LineCount, // Advance to the next page when the line count reaches a certain number.
	};

protected:
	VECLINE m_vecLine;

	int m_nStartLine;
	int	m_nEndLine;

	int m_nCurLine;
	int m_nVisibleCount;

	bool m_bAutoScroll;			// Auto-scrolls when align option is bottom (chat window); if false, do not scroll.

	float m_fSymbolWidth;		// Width of the ellipsis symbol "..."
	float m_fTextMargin;		// Remaining height after subtracting actual visible line height from control height

	CEtUIScrollBar m_ScrollBar;
	bool m_bUseSelect;

	// Textbox LineData is only used to pass values when creating a new line.
	SLineData m_sLineData;

	bool m_bUseWordBreak;
	int m_nMaxLine;

#ifdef TEXTBOX_RENDERLOCK
	volatile long m_nRenderingState;
#endif

	DWORD					m_RenderTextColor;
	typedef std::vector<CWord> SENTENCE;
	std::list<SENTENCE>		m_OriginalStrings;
	CWord					m_WordSelected;
	CWord					m_WordMouseOver;

	CEtUITextBox *m_pNextPageTextBox;
	eNextPageCondition m_eNextPageCondition;
	bool m_bAddTextToNextPage;	// Value checked to determine whether to register to NextPage.
	bool m_bNextPage;			// true if self is NextPage.
	int m_nNextPageLineCount;	// From the second page onward, follow the first page's max line count.
	int m_nAdjustValue;			// If the second page's start point differs from the first page, its max line count also differs.
	void SetNextPage( bool bNextPage ) { m_bNextPage = bNextPage; }
	void SetNextPageLineCount( int nCount, int nAdjustValue );

protected:
	virtual void RenderText( float fElapsedTime );
	virtual void RenderLine( float fElapsedTime, CLine *sLine, bool bRollOver = false );
	virtual void RenderWord( float fElapsedTime, CWord& sWord, bool bRollOver = false, int nBorderFlag = 0 );

	void AddWord( const LPCWSTR szText, const LPCWSTR szTextWithTag, SWORD_PROPERTY &wordProperty );
	void AddLine();
	void AddImage( WCHAR *wszIamgeName, int nWidth, int nHeight );

	void SetMaxLine( int nValue ) { m_nMaxLine = nValue; }

	bool UpdateVisibleCount();
	void UpdateAlignText();
	void UpdateText();
	void UpdateText( float fX, float fY );

	void CalcSymbolTextRect();

	bool SelectLine( float fX, float fY );
	bool SelectWord(float fX, float fY);
	void SetMouseInWord( float fX, float fY );
	void SetMouseInLine( float fX, float fY );
	int GetMouseEnteredLine();
	CLine* GetEndLine();

protected:

	// Note : Fixed-size text box
	void AppendTextF( const LPCWSTR szText, const LPCWSTR szTextWithTag, const D3DCOLOR TextColor, DWORD dwFormat, bool bAdd, const D3DCOLOR BgColor = 0 );
	// Note : Variable-size text box
	void AppendTextV( const LPCWSTR szText, const LPCWSTR szTextWithTag, const D3DCOLOR TextColor, DWORD dwFormat, bool bAdd, const D3DCOLOR BgColor = 0);

public:
	void UpdateTextBoxHeight();
	void UpdateTextBoxWidth();
	void UpdateTextBox();

	// Note : Append text to the existing text line.
	void AppendText( LPCWSTR szText, const D3DCOLOR TextColor = textcolor::WHITE, DWORD dwFormat = UITEXT_NONE, bool bAdd = false, const D3DCOLOR BgColor = 0, LPCWSTR szTextWithTag = L"" );

	// Note : Add text to a new line.
	void AddText( LPCWSTR szText, const D3DCOLOR TextColor = textcolor::WHITE, DWORD dwFormat = UITEXT_NONE, const D3DCOLOR BgColor = 0 );
	void DoAddText( LPCWSTR szText, LPCWSTR szTextWithTag, const D3DCOLOR TextColor = textcolor::WHITE, DWORD dwFormat = UITEXT_NONE, const D3DCOLOR BgColor = 0 );
	
	// Note : Input a sentence or paragraph containing '\n'.
	void SetText( LPCWSTR szText, const D3DCOLOR TextColor = textcolor::WHITE );

	// Colored text used in item descriptions or system messages
	void AddColorText( LPCWSTR szText, const D3DCOLOR DefaultTextColor = textcolor::WHITE, const D3DCOLOR BgColor = 0, const D3DCOLOR DecreaseColor = 0 );

	void ClearText();

	// Note : Set the line spacing for all lines uniformly.
	void SetLineSpace( float fSpace );

	void SetLineData( const SLineData &sLineData ) { m_sLineData = sLineData; }
	SLineData &GetLineData() { return m_sLineData; }
	void ClearLineData() { m_sLineData.Clear(); }

protected:
	int Scroll( int nScrollAmount );

public:

	void ScrollLineUp();
	void ScrollLineDown();
	void ScrollHome();
	void ScrollEnd();
	void ScrollPageUp();
	void ScrollPageDown();

	bool IsEmpty()								{ return m_vecLine.empty(); }
	bool IsScrollMode()							{ return m_nVisibleCount<(int)m_vecLine.size(); }
	bool IsLastPage();
	
	int  GetLineSize()                          { return (int)m_vecLine.size(); }
	std::wstring GetLineText(int nIndex);

	// Note : Fit the text box size to the height of the currently visible lines.
	void FitTextBoxToLine();

	// Note : Page turning for the NPC dialog window.
	void ScrollPageTurn();

	// Note : Automatically scroll whenever a line is added.
	void SetAutoScroll( bool bAutoScroll )		{ m_bAutoScroll = bAutoScroll; }

	//int AddFont( const char *pFontName );
	//void SetFontHeight( int nFontHeight );

	virtual int GetScrollBarTemplate() { return m_Property.TextBoxProperty.nScrollBarTemplate; }
	virtual void SetScrollBarTemplate( int nIndex ) { m_Property.TextBoxProperty.nScrollBarTemplate = nIndex; }

	// Note : Get the index of the currently selected line.
	//
	int GetSelectedLineIndex( bool bClearSelection = true ) const;
	void ClearMouseEnteredLine();
	void ClearSelectedLine();

	bool GetSelectedLineData( SLineData &sLineData, bool bClearSelect = true ) const;
	const CWord& GetSelectedWordData() const;

	// Get the last line info.
	std::wstring GetLastLineText();
	bool GetLastLineCoord( SUICoord &Coord );

	// Delete the last line?
	void RemoveLastLine();

	// nLineSize = which line to keep <1,2,3,4> adjust line size and append symbol at the end for remaining text, used to truncate when size is large
	void ResizeLineTextWithSymbol(int nLineSize , std::wstring wszSymbol);

	// WordBreak
	void EnableWorkBreak(bool bEnable = true) { m_bUseWordBreak = bEnable; }
	static int GetCaretWithWordBreak( const std::wstring& strText, const int nOrigCaret, bool bFirstWordInLine );

	// NextPage
	void SetNextPageTextBox( CEtUITextBox *pControl, eNextPageCondition eCondition = NextPage_DlgScreenHeight, int nAdjustValue = 0 );

	// Check if NextPage has been filled with content. 0 means not filled, 1 means filled up to the first NextPage, and 2 means filled up to the second NextPage.
	int GetUsedNextPage();

	virtual void FindInputPos( std::vector<EtVector2> &vecPos );

	// Change line string...
	

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual void Render( float fElapsedTime );
	virtual bool CanHaveFocus() { return ( IsShow() && IsEnable() ); }
	virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void UpdateRects();
	virtual void Process( float fElapsedTime );
};