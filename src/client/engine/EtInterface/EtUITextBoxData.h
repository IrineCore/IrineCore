#pragma once

// Note : Line
#define UITEXT_NONE		0x00000000
#define UITEXT_CLIP		0x00000001
#define UITEXT_SYMBOL	0x00000010
// Note : Word
#define UITEXT_RIGHT	0x00000100
#define UITEXT_CENTER	0x00001000

using namespace EtInterface;

struct SWORD_PROPERTY
{
	SUICoord		uiCoord;		// Coordinates and size where text will be rendered
	SUIColorEx		uiColor;
	DWORD			dwFormat;		// Text format (UITEXT_CLIP, UITEXT_SYMBOL)
	bool			bSymbol;		// Whether to append "..." at the end of a word
	bool			bAdd;			// true if word was added via AddText(), false if via AppendText()
	//int nFontIndex; // Index for using a different font
	D3DCOLOR		BgColor;

	SWORD_PROPERTY()
		: dwFormat(UITEXT_NONE)
		//, nFontIndex(-1)
		, bSymbol(false)
		, bAdd(false)
		, BgColor(0)
	{
	}

	void Clear()
	{
		uiCoord.SetCoord(0.f, 0.f, 0.f, 0.f);
		uiColor.Clear();
		dwFormat = UITEXT_NONE;
		bSymbol = false;
		bAdd = false;
		BgColor = 0;
	}
};

// Note : The added text has a single color.
//		Therefore, even if it forms a sentence, treat it as a single word.
class CWord
{
public:
	CWord();
	~CWord();

public:
	std::wstring m_strWord;
	SWORD_PROPERTY m_sProperty;
	std::wstring m_strWordWithTag;

public:
	void UpdatePos( float fX, float fY );
	void Clear();
};
typedef	std::vector<CWord>	VECWORD;

// Store a separate string value for each line.
// Seems to be for implementing whispering to the user when clicking a line in the chat window.
struct SLineData
{
	std::wstring m_strData;
	int m_nData;

	void Clear()
	{
		m_strData.clear();
		m_nData = 0;
	}
};

// Note : Splits the rendered text into lines.
//		To manage by line unit later.
class CLine
{
public:
	CLine();
	virtual ~CLine();

	enum LineTypeEnum {
		Line,
		Image,
	};
public:
	VECWORD m_vecWord;	
	SUICoord m_uiCoord;
	float m_fLineSpace;			// Note : Spacing between lines.

protected:
	bool m_bSelected;
	bool m_bMouseInLine;

	SLineData m_sLineData;		// Separate string value held by the line.

public:
	void Select( bool bSelect )				{ m_bSelected = bSelect; }
	bool IsSelected()						{ return m_bSelected; }
	void SetMouseInLine( bool bMouseInCtl )	{ m_bMouseInLine = bMouseInCtl; }
	bool IsMouseInLine()					{ return m_bMouseInLine; }

public:
	std::wstring GetText();

	void SetLineData( SLineData &sLineData ) { m_sLineData = sLineData; }
	SLineData &GetLineData() { return m_sLineData; }

public:
	void SetLineSpace( float fLineSpace );
	float GetLineSpace()					{ return m_fLineSpace; }
	void UpdatePos( float fX, float fY );

	virtual LineTypeEnum GetLineType() { return CLine::Line; }
};

class CImageLine : public CLine
{
public:
	CImageLine();
	virtual ~CImageLine();

protected:
	EtTextureHandle m_hTexture;
	std::string m_szFileName;
	SUICoord m_uiUV;
	
public:
	void SetImage( WCHAR *wszFileName, SUICoord &Coord );
	void SetImage( char *szFileName, SUICoord &Coord );
	EtTextureHandle GetTexture();
	SUICoord &GetTextureUV() { return m_uiUV; }

	virtual LineTypeEnum GetLineType() { return CLine::Image; }
};

typedef	std::vector<CLine *>	VECLINE;
typedef	VECLINE::iterator	VECLINE_ITER;