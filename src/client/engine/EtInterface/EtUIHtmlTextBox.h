#pragma once
#include "EtUITextBox.h"
#include "LiteHTMLReader.h"

// Note : Below is a text box that reads and renders HTML.
//		Used a parser named LiteHTMLReader. See the NPC dialog window for reference.
//		Currently only paragraph, line break, and font color HTML tags are supported.
//
class CEtUIHtmlTextBox : public CEtUITextBox, public ILiteHTMLReaderEvents
{
public:
	CEtUIHtmlTextBox(CEtUIDialog *pParent);
	virtual ~CEtUIHtmlTextBox(void);

protected:
	DWORD m_dwTextColor;		// Current color
	DWORD m_dwDefaultTextColor;	// Default color

	CLiteHTMLReader m_htmlReader;

protected:
	virtual void BeginParse(DWORD dwAppData, bool &bAbort);
	virtual void StartTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort);
	virtual void EndTag(CLiteHTMLTag *pTag, DWORD dwAppData, bool &bAbort);
	virtual void Characters(const CStringW &rText, DWORD dwAppData, bool &bAbort);
	virtual void Comment(const CStringW &rComment, DWORD dwAppData, bool &bAbort);
	virtual void EndParse(DWORD dwAppData, bool bIsAborted);

	virtual void RenderLine( float fElapsedTime, CLine* sLine, bool bRollOver = false );
public:
	bool ReadHtmlString( LPCWSTR lpszString );
	bool ReadHtmlFile( LPCWSTR filename );
	bool ReadHtmlFile( LPCSTR filename );
	bool ReadHtmlStream( CStream *pStream );

	virtual int GetScrollBarTemplate();
	virtual void SetScrollBarTemplate( int nIndex );

	// Since HtmlTextBox is not used much, color has not been exposed as a property yet.
	// If needed later, extract as a property value (then the m_dwTextColor member could be removed).
	// Default color is white.
	//
	// If extracted as a property, colors in existing UI files will default to 0x00000000, so they will need to be found and reset.
	void SetDefaultTextColor( DWORD dwColor ) { m_dwTextColor = m_dwDefaultTextColor = dwColor; }
};
