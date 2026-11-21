#pragma once
#include "LostDeviceProcess.h"
#include "EtUIType.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <usp10.h>
#pragma comment( lib, "usp10.lib" )

using namespace std;

#define PRE_MOD_MINIMIZE_FORCE_BLUR

//struct SRegisterFontInfo
//{
//	std::string szFontName;
//	HANDLE hFontHandle;
//};

struct SFontInfo_Old
{
	int nFontHeight;
	int nFontWeight;
	char szFontName[ LF_FACESIZE ];
	BOOL bItalic;
};
 
typedef struct FT_FaceRec_*  FT_Face;
struct SFontInfo
{
	enum { VERSION = 0x101 };

	FT_Face Face;
	BYTE *pMemFont;
	int nFontHeight;
	int nFontWeight;
	char szFontName[ LF_FACESIZE ];
	BOOL bItalic;

	map<wchar_t, int> mapAdvanceX;		// Calling the FreeType function every time to get advance.x causes overhead, so...
	map<wchar_t, int> mapBoldAdvanceX;	// For bold font... italic is not implemented yet...

	SFontInfo() : Face( NULL ), pMemFont( NULL ), nFontHeight( 0 ), nFontWeight( 0 ), bItalic( false )
	{
		ZeroMemory( szFontName, sizeof(szFontName) );
		//ZeroMemory( this, sizeof(SFontInfo) );
	}

public:
	bool Load( CStream &stream );
	bool Save( CStream &stream );
};

struct SUIFontSet
{
	enum { VERSION = 0x101 };

	int nIndex;
	std::string strFontName;
	std::string strFileName;
	std::string strFullFileName;
	int nFontHeight;
	int nFontWeight;
	bool bItalic;

	SUIFontSet()
		: nIndex(-1)
		, strFontName("")
		, strFileName("")
		, strFullFileName("")
		, nFontHeight(0)
		, nFontWeight(0)
		, bItalic(false)
	{
	}

	SUIFontSet( int Index, std::string FontName, std::string FileName, int FontHeight, int FontWeight, bool Italic )
	{
		nIndex = Index;
		strFontName = FontName;
		strFileName = FileName;
		nFontHeight = FontHeight;
		nFontWeight = FontWeight;
		bItalic = Italic;
	}

	void Load( CStream &stream )
	{
		DWORD dwVersion;
		stream >> dwVersion;

		switch( dwVersion )
		{
		case VERSION:
			{
				stream >> nIndex;
				stream >> strFontName;
				stream >> strFileName;
				stream >> nFontHeight;
				stream >> nFontWeight;
				stream >> bItalic;
			}
			break;
		default:
			ASSERT(0&&"SUIFontSet::Load, default");
			break;
		}
	}

	void Save( CStream &stream )
	{
		stream << VERSION;

		stream << nIndex;
		stream << strFontName;
		stream << strFileName;
		stream << nFontHeight;
		stream << nFontWeight;
		stream << bItalic;
	}
};

struct SFontTextureInfo
{
	//enum
	//{
	//	NORMAL,
	//	SHADOW_BASE,
	//	STROKE_BASE,
	//	GLOW_BASE,
	//	COUNT,
	//};

	EtTextureHandle hTexture;
	/*int nDrawedType;*/
	int Width;
	int Height;

	SFontTextureInfo( void ) : /*nDrawedType(NORMAL),*/ Width( 0 ), Height( 0 ) {};
};

struct SFontDrawEffectInfo
{
	enum
	{
		NORMAL = 0,
		SHADOW,
		STROKE,
		GLOW,
		COUNT,
	};

	int nDrawType;
	int nWeight;
	float fAlphaWeight;
	DWORD dwFontColor;
	DWORD dwEffectColor;

	float fGlobalBlurAlphaWeight;

	SFontDrawEffectInfo( void ) : nDrawType( NORMAL ), nWeight( 1 ), fAlphaWeight( 1.5f ),
								  fGlobalBlurAlphaWeight( 0.0f ),
								  dwFontColor( 0 ), dwEffectColor( 0 )
	{};

	bool operator == ( const SFontDrawEffectInfo& Info )
	{
		return (Info.nDrawType == nDrawType &&
				Info.nWeight == nWeight &&
				Info.fAlphaWeight == fAlphaWeight &&
				Info.fGlobalBlurAlphaWeight == Info.fGlobalBlurAlphaWeight &&
				//(Info.dwFontColor&0x00ffffff) == (dwFontColor&0x00ffffff) && // Compare excluding alpha.. 
				Info.dwEffectColor == dwEffectColor);
	}
};

//  Remove vector and map to reduce memory allocation overhead during assignment in vector erase on cache update.
struct SCachedFont
{
	std::wstring szText;
	int	nFontIndex;
	bool bBold;
	float fWidth;
	SFontTextureInfo TextureInfo;
	SFontDrawEffectInfo FontEffectInfo;
	int nLastUseTick;
};

// Do not cache caret info; keep all of it in memory.
struct SCaretKey
{
	std::wstring szText;
	int nFontIndex;

	const bool operator < ( const SCaretKey & rhs )  const {
		if( szText < rhs.szText ) return true;
		else if( szText > rhs.szText ) return false;
		if( nFontIndex < rhs.nFontIndex ) return true;
		else if( nFontIndex > rhs.nFontIndex ) return false;
		return false;
	}
};

struct SCaretInfo
{
	vector<int> vecCaretPos;
	map<int, int> mapPosByCaret;
	int nHeight;
};

struct SFontInfoHandle
{
	int		nFontHeight;
	char	szFontName[ LF_FACESIZE ];
	HFONT	hFont;
};

struct SAddFontMemResource
{
	char	szFontName[ LF_FACESIZE ];
	HANDLE	hFontHandle;
};

#define UPPER_PADDING_PIXEL		4.0f		// Top margin of font texture

typedef struct FT_LibraryRec_  *FT_Library;
class CEtFontMng : public CSingleton< CEtFontMng >, public CLostDeviceProcess
{
public:
	CEtFontMng(void);
	virtual ~CEtFontMng(void);

	static float s_fLinePitchRate;	// Adjust line spacing globally.
	static bool s_bUseUniscribe;	// For locales with large font files, fontset processing on ResetDevice takes long, so exclude Uniscribe when not needed
	static bool s_bUseWordBreak;	// Whether to use word-level line wrapping (uses Uniscribe API)
											
protected:
	std::vector< SFontInfo > m_vecFont;
	std::vector< SUIFontSet > m_vecFontSet;
	std::map<int, int> m_mapFontIndex;
	std::string m_strFontSetFileName;
	FT_Library m_Library;
	std::vector< SCachedFont > m_cachedFontList;
	std::list< EtTextureHandle > m_texturePool;

	std::map< SCaretKey, SCaretInfo > m_caretInfo;

	int		m_nUpdateTerm;

	std::map< std::string, std::pair<BYTE*, DWORD> > m_FontMemCache;
	std::vector< SAddFontMemResource > m_vecAddFontMemResourceList;
	std::vector< SFontInfoHandle > m_vecStFontList;
	HFONT	m_hOldFont;

	float	m_fOriginWidth;

#if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
	bool	m_bForceBlur;
	float	m_fBlurWeight;
	int		m_nMinimizeSize;
#endif	// #if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )

protected:
	int GetFontIndex( int &nFontSetIndex, int nFontHeight );
	void AccumulateColor( DWORD& dwAlpha, DWORD& dwRed, DWORD& dwGreen, DWORD& dwBlue, DWORD& dwNowColor, int iWeight );
	void Blur( BYTE* pTextureBuffer, BYTE* pOriginalBuffer, bool* pabBlurDir, float fAlphaWeight, int nPitch, int nTexelWidth, int nTexelHeight, int nTextTexelWidth, int nTextTexelHeight, bool bReverseLoop = false );

	void BlurProcess( BYTE* pOriginalBuffer, int nBytePos, bool* pabBlurDir, int nY, int nX, BYTE* pTextureBuffer, int nPitch, int nTexelWidth, int nTexelHeight, float fAlphaWeight );
	
	bool IsExistAddFontResourceList( SFontInfo& stFontInfo );
	HFONT FindFontHandle( SFontInfo& stFontInfo );
	void GetTTFLocalFontName( const char* pFontFileName, char* pChangeFontName );
	int GetGlyphGOffset(  SFontInfo& stFontInfo, wstring& strText, vector<WORD>& vecGlyph, vector<GOFFSET>& vecGOffset );
	void GetWordBreakInfo( wstring& strText, vector<SCRIPT_LOGATTR>& vecLogAttr );
	int WordBreak( wstring& strText, SCaretKey& CaretKey, bool bAlreadyExistCaret, FT_Face& Face, bool bBold, SFontInfo& FontInfo, float fWidth, 
					vector<wstring>& vecStrLine, vector<int>& vecWidthList, bool bUseRemainWidth = false );
	void SetOriginWidth( float& fWidth );
	bool IsSymbol( wchar_t cText );
	bool IsOpenBraket( wchar_t cText );

public:
	void Initialize( const char *szFileName );
	void Finalize();

	int AddFont( const char *pFontFileName, const char *pFontName, int nFontHeight, int nFontWeight, BOOL bItalic = false );
	void DeleteAllFont();

	unsigned int CalcTextureSize(unsigned int size);
	SFontTextureInfo GetFontTexture( int nFontIndex, LPCWSTR szText, float fWidth, DWORD dwFontFormat, 
									 SFontDrawEffectInfo& Info, bool bRemoveCache = true );

	void UpdateFontCache( bool bRemoveCache );
	SCachedFont* GetCachedFontTexture( LPCWSTR szText, float fWidth, SFontDrawEffectInfo& FontEffectInfo, int nFontIndex );

	void DrawTextW( int &nFontSetIndex, int nFontHeight, LPCWSTR szText,  DWORD dwFontFormat, SUICoord &ScreenCoord, 
					int nCount, SFontDrawEffectInfo& FontEffectInfo, bool bRemoveCache = true, float ZValue = 0.f, DWORD dwBgColor = 0, int nBorderFlag = 0 );
	void DrawTextW3D( int &nFontSetIndex, int nFontHeight, LPCWSTR szText, EtVector3 &vPosition, SFontDrawEffectInfo& FontEffectInfo, int nCount, float fScale = 1.0f );

	void CalcTextRect( int &nFontSetIndex, int nFontHeight, LPCWSTR szText, DWORD dwFontFormat, SUICoord &CalcCoord, int nCount, bool bUseCache = true );
	void GetFontInfo( int &nFontSetIndex, int nFontHeight, SFontInfo &FontInfo );

	bool LoadFontSet( const char *szFileName );
	void SaveFontSet( const char *szFileName );
	std::string& GetFontSetFileName() { return m_strFontSetFileName; }
	std::vector< SUIFontSet >& GetFontSetList() { return m_vecFontSet; }
	void AddFontSet( SUIFontSet &fontSet );
	void DeleteFontSet( int nIndex );
	void ModifyFontSet( int nIndex, SUIFontSet &fontSet );
	SUIFontSet *GetFontSet( int nIndex );
	int GetFontSetIndex();
	int GetCaretPos( LPCWSTR szText, int& nFontSetIndex, int nFontHeight, int nCharIndex, int* pHeight = NULL, bool bUseCache = true );
	
	// nTrail is set to 1 when the caret needs to be displayed past the very end of the text.
	int GetCaretFromCaretPos( LPCWSTR szText, int& nFontSetIndex, int nFontHeight, int nCaretPos, /*OUT*/ int& nTrail );
	int GetCachedCharAdvanceX( FT_Face Face, SFontInfo& FontInfo, wchar_t ch, bool bBold, int iNowAdvance );

	void FlushFontCache();

	virtual void OnLostDevice();
	virtual void OnResetDevice();

	bool GetWordBreakText( wstring& strText, int nFontSetIndex, int nFontHeight, float fWidth, vector<wstring>& vecStrLine, 
							int& nMaxWidth, bool bUseRemainWidth = false, float fOriginWidth = 0.0f );

	void GetBasicFontTextureInfo(SFontTextureInfo& result, int nFontSetIndex, int nFontHeight, LPCWSTR szText, DWORD dwFontFormat, const SUICoord &ScreenCoord, SFontDrawEffectInfo& Info);

#if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
	void SetForceBlur( bool bForce ) { m_bForceBlur = bForce; }
	void SetForceBlurWeight( float fWeight ) { m_fBlurWeight = fWeight; }
	float GetForceBlurWeight()	{ return m_fBlurWeight; }
	void SetForceMinimizeSize( int nMinimizeSize ) {m_nMinimizeSize = nMinimizeSize; }
	int GetForceMinimizeSize()	{ return m_nMinimizeSize; }
#endif	// #if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
};
