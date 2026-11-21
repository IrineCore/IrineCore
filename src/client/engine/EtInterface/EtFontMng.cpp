#include "StdAfx.h"
#include "EtSprite.h"
#include "EtFontMng.h"
#include "..\EtCamera.h"
#include "DebugSet.h"
#include "EtResourceMng.h"
#include "EtStdFileIO.h"
#include <shlobj.h>
#include <strsafe.h>
#include "./boost/algorithm/string.hpp"


float CEtFontMng::s_fLinePitchRate = 1.0f;
bool CEtFontMng::s_bUseUniscribe = true;
bool CEtFontMng::s_bUseWordBreak = false;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

/**/
#define ACCUMLATE_COLOR( ALPHA, RED, GREEN, BLUE, NOWCOLOR, WEIGHT )		\
{																												\
	ALPHA += ((0xff000000 & NOWCOLOR) >> 24) * WEIGHT;								\
	RED += ((0x00ff0000 & NOWCOLOR) >> 16) * WEIGHT;									\
	GREEN += ((0x0000ff00 & NOWCOLOR) >> 8) * WEIGHT;									\
	BLUE  += (0x000000ff & NOWCOLOR) * WEIGHT;											\
}

#define BLUR_PROCESS( ORIGIN, BYTEPOS, BLURDIR, Y, X, TEXBUF, PITCH, WIDTH, HEIGHT, WEIGHT )					\
{																																									\
	DWORD dwResultColor = 0;																																\
	int nWeightCount = 0;																																	\
	DWORD dwAlpha = 0;																																	\
	DWORD dwRed = 0;																																		\
	DWORD dwGreen = 0;																																	\
	DWORD dwBlue = 0;																																		\
	if( BLURDIR[ 0 ] &&																																		\
	1 <= Y && 1 <= X )																																	\
	{																																								\
	nWeightCount += 1;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS - PITCH - 1*sizeof(DWORD) ]);											\
	dwResultColor += dwNowColor;																													\
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );															   \
	}																																							   \
	if( BLURDIR[ 1 ] &&																																		\
	1 <= Y )																																					\
	{																																							   \
	nWeightCount += 2;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS - PITCH ]);																	\
	dwResultColor += dwNowColor*2;																												\
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );															   \
	}																																							   \
	if( BLURDIR[ 2 ] &&																																		\
	1 <= Y && X < WIDTH-1 )																														   \
	{																																							   \
	nWeightCount += 1;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS - PITCH + 1*sizeof(DWORD) ]);										\
	dwResultColor += dwNowColor;																													\
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1);															   \
	}																																							   \
	if( BLURDIR[ 3 ] &&																																		\
	1 <= X )																																					\
	{																																							   \
	nWeightCount += 2;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS - sizeof(DWORD) ]);                                                       \
	dwResultColor += dwNowColor*2;																											   \
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );															   \
	}																																							   \
	if( BLURDIR[ 4 ] )																																			\
	{																																							   \
	nWeightCount += 4;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS ]);																			   \
	dwResultColor += dwNowColor*4;																											   \
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 4 );															   \
	}																																							   \
	if( BLURDIR[ 5 ] &&																																		\
	X < WIDTH-1 )                                                                                                                                        \
	{																																							   \
	nWeightCount += 2;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS + sizeof(DWORD) ]);														\
	dwResultColor += dwNowColor*2;																											   \
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );															   \
	}																																							   \
	if( BLURDIR[ 6 ] &&																																		\
	Y < HEIGHT-1 && 1 <= X )																															\
	{																																							   \
	nWeightCount += 1;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS + PITCH - 1*sizeof(DWORD) ]);										\
	dwResultColor += dwNowColor;																													\
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );															   \
	}																																							   \
	if( BLURDIR[ 7 ] &&																																		\
	Y < HEIGHT-1 )																																	   \
	{																																							   \
	nWeightCount += 2;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS + PITCH ]);																   \
	dwResultColor += dwNowColor*2;																											   \
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );															   \
	}																																							   \
	if( BLURDIR[ 8 ] &&																																		\
	Y < HEIGHT-1 && X < WIDTH-1 )                                                                                                                \
	{																																							   \
	nWeightCount += 1;																																	\
	DWORD dwNowColor = *(DWORD*)(&ORIGIN[ BYTEPOS + PITCH + 1*sizeof(DWORD) ]);									   \
	dwResultColor += dwNowColor;																													\
	ACCUMLATE_COLOR( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );															   \
	}																																								\
	if( 0 != nWeightCount )																																	\
	{																																							   \
	dwAlpha /= nWeightCount;                                                                                                                        \
	if( 0.0f != WEIGHT )																																   \
	dwResultColor = D3DCOLOR_ARGB( min(DWORD((float)dwAlpha*WEIGHT), 255), 255, 255, 255 );						\
		else																																						\
		dwResultColor = D3DCOLOR_ARGB( min(dwAlpha*(dwAlpha/50), 255), 255, 255, 255 );                                     \
		*(DWORD*)(&TEXBUF[ BYTEPOS ]) = dwResultColor;																						\
	}																																							   \
}

//////////////////////////////////////////////////////////////////////////
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include "freetype/ftsynth.h"

extern bool g_bSkipScene;																																   

static CSyncLock s_Lock;																																	

bool SFontInfo::Load( CStream &stream )																											   
{																																								   
	DWORD dwVersion;																																	   
	stream >> dwVersion;																																	

	switch( dwVersion )																																	   
	{																																							   
	case VERSION:																																			
		{																																						   
			stream.ReadBuffer( szFontName, sizeof(char)*LF_FACESIZE );																		
			stream >> nFontHeight;                                                                                                                        
			stream >> nFontWeight;																													   
			stream >> bItalic;																																
		}																																						   
		break;																																				   

	default: 
		ASSERT(0&&"SFontInfo::Load");
		CDebugSet::ToLogFile( "SFontInfo::Load, default case!" );
		return false;
	}

	return true;
}

bool SFontInfo::Save( CStream &stream )
{
	stream << VERSION;

	stream.WriteBuffer( szFontName, sizeof(char)*LF_FACESIZE );
	stream << nFontHeight;
	stream << nFontWeight;
	stream << bItalic;

	return true;
}

CEtFontMng::CEtFontMng(void)
: m_nUpdateTerm( 0 )
, m_hOldFont( NULL )
, m_fOriginWidth( 0.0f )
#if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
, m_bForceBlur( false )
, m_fBlurWeight( 0.6f )
, m_nMinimizeSize( 16 )
#endif	// #if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
{
	FT_Error error = FT_Init_FreeType(&m_Library);
	if( error ) {
		CDebugSet::ToLogFile( "CEtFontMng::Initialize, FT_Init_FreeType the return value is %d", error );
	}
}

CEtFontMng::~CEtFontMng(void)
{
	if( m_Library ) {
		FT_Done_FreeType(m_Library);
		m_Library = NULL;
	}
}

void CEtFontMng::Initialize( const char *szFileName )
{	
	CEtUIDialog::CalcDialogScaleByResolution( CEtDevice::GetInstance().Width(), CEtDevice::GetInstance().Height() );
	if( !LoadFontSet( szFileName ) )
	{
		SUIFontSet sUIFontSet[3] = 
		{ 
			SUIFontSet(0, "Arial", "Arial.ttf", 12, 4, false),
			SUIFontSet(1, "Arial", "Arial.ttf", 14, 4, false),
			SUIFontSet(2, "Arial", "Arial.ttf", 36, 7, false),
		};

		int nFontIndex(-1);
		for( int i=0; i<3; ++i)
		{
			m_vecFontSet.push_back( sUIFontSet[i] );
			nFontIndex = AddFont( m_vecFontSet[i].strFileName.c_str(), m_vecFontSet[i].strFontName.c_str(), m_vecFontSet[i].nFontHeight, m_vecFontSet[i].nFontWeight, m_vecFontSet[i].bItalic );
			m_mapFontIndex.insert( std::make_pair( m_vecFontSet[i].nIndex, nFontIndex ) );
		}
	}

}

void CEtFontMng::Finalize()
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	m_vecFontSet.clear();
	m_mapFontIndex.clear();
	DeleteAllFont();
	/*UnregisterAllFont();*/

	int i, nSize;
	nSize = (int)m_cachedFontList.size();
	for( i = 0; i < nSize; i++) {
		SAFE_RELEASE_SPTR( m_cachedFontList[i].TextureInfo.hTexture );
	}
	m_cachedFontList.clear();


	std::list< EtTextureHandle >::iterator it = m_texturePool.begin();
	for( std::list< EtTextureHandle >::iterator it = m_texturePool.begin(); it != m_texturePool.end(); it++ ) {
		SAFE_RELEASE_SPTR( (*it) );
	}
	m_texturePool.clear();

	m_caretInfo.clear();
}

//void CEtFontMng::RegisterFont( const char *pFontName )
//{
//	ASSERT(pFontName&&"CEtFontMng::RegisterFont");
//
//	SRegisterFontInfo ResterFont;
//	DWORD dwFileSize, dwFontCount;
//	HANDLE hFontHandle;
//	char *pFontData;
//
//	CResMngStream Stream( pFontName );
//
//	Stream.Seek(0, SEEK_END);
//	dwFileSize = Stream.Tell();
//	Stream.Seek(0, SEEK_SET);
//
//	if ( dwFileSize > 300000000 )
//	{ 
//		assert(false);
//		return;
//	}
//
//	pFontData = new char[ dwFileSize ];
//	Stream.Read(pFontData, dwFileSize);
//	hFontHandle = AddFontMemResourceEx( pFontData, dwFileSize, 0, &dwFontCount );
//	delete [] pFontData;
//
//	ResterFont.hFontHandle = hFontHandle;
//	ResterFont.szFontName = pFontName;
//
//	m_vecResiteredFont.push_back( ResterFont );
//}
//
//void CEtFontMng::UnregisterFont( const char *pFontName )
//{
//	ASSERT(pFontName&&"CEtFontMng::UnregisterFont");
//
//	for( int i = 0; i < ( int )m_vecResiteredFont.size(); i++ )
//	{
//		if( stricmp( pFontName, m_vecResiteredFont[ i ].szFontName.c_str() ) == 0 )
//		{
//			RemoveFontMemResourceEx( m_vecResiteredFont[ i ].hFontHandle );
//			m_vecResiteredFont.erase( m_vecResiteredFont.begin() + i );
//			return;
//		}
//	}
//}
//
//void CEtFontMng::UnregisterAllFont()
//{
//	for( int i = 0; i < ( int )m_vecResiteredFont.size(); i++ )
//	{
//		RemoveFontMemResourceEx( m_vecResiteredFont[ i ].hFontHandle );
//	}
//	m_vecResiteredFont.clear();
//}

void CEtFontMng::DeleteAllFont()
{
	for( int i = 0; i < ( int )m_vecFont.size(); i++ )
	{
		FT_Done_Face( m_vecFont[i].Face );
	}
	m_vecFont.clear();

	for( std::map< std::string, std::pair<BYTE*, DWORD> >::iterator it = m_FontMemCache.begin(); it != m_FontMemCache.end(); ++it)
	{
		SAFE_DELETEA( it->second.first );
	}
	m_FontMemCache.clear();

	if( s_bUseUniscribe )
	{
		for( int i=0; i<static_cast<int>( m_vecAddFontMemResourceList.size() ); i++ )
		{
			RemoveFontMemResourceEx( m_vecAddFontMemResourceList[i].hFontHandle );
		}
		m_vecAddFontMemResourceList.clear();
		for( int i=0; i<static_cast<int>( m_vecStFontList.size() ); i++ )
		{
			if( m_vecStFontList[i].hFont )
				DeleteObject( m_vecStFontList[i].hFont );
		}
		m_vecStFontList.clear();
	}
}

void CEtFontMng::GetFontInfo( int &nFontSetIndex, int nFontHeight, SFontInfo &FontInfo )
{ 
	ASSERT(m_vecFont.size()&&"CEtFontMng::GetFontInfo");
	ASSERT(nFontSetIndex>=0&&"CEtFontMng::GetFontInfo");
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	if( nFontIndex < 0 || nFontIndex >= (int)m_vecFont.size() ) {
		_ASSERT(0&&"CEtFontMng::GetFontInfo" );
		return;
	}
	FontInfo = m_vecFont[ nFontIndex ];
}

int CEtFontMng::AddFont( const char *pFileName, const char *pFontName, int nFontHeight, int nFontWeight, BOOL bItalic )
{
	ASSERT(pFontName&&"CEtFontMng::AddFont");

	SFontInfo FontInfo;
	FontInfo.nFontHeight = nFontHeight;
	FontInfo.nFontWeight = nFontWeight;
	FontInfo.bItalic = bItalic;

	int nNewFontHeight = int(nFontHeight * CEtUIDialog::GetDialogScale());
	if( s_bUseUniscribe )
	{
		char szChangeFontName[ LF_FACESIZE ] = {0,};
		GetTTFLocalFontName( pFileName, szChangeFontName );
		strcpy_s( FontInfo.szFontName, LF_FACESIZE, szChangeFontName );

		for( int i = 0; i < ( int )m_vecFont.size(); i++ )
		{
			if( ( m_vecFont[ i ].nFontHeight == nNewFontHeight ) && 
				( m_vecFont[ i ].nFontWeight == nFontWeight ) && 
				( m_vecFont[ i ].bItalic == bItalic ) && 
				( stricmp( m_vecFont[ i ].szFontName, szChangeFontName ) == 0 ) )
			{
				return i;
			}
		}
	}
	else
	{
		strcpy_s( FontInfo.szFontName, LF_FACESIZE, pFontName );

		for( int i = 0; i < ( int )m_vecFont.size(); i++ )
		{
			if( ( m_vecFont[ i ].nFontHeight == nNewFontHeight ) && 
				( m_vecFont[ i ].nFontWeight == nFontWeight ) && 
				( m_vecFont[ i ].bItalic == bItalic ) && 
				( stricmp( m_vecFont[ i ].szFontName, pFontName ) == 0 ) )
			{
				return i;
			}
		}
	}

	// Search the system folder first; if found, redirect the path to the font folder, otherwise proceed as is
	char acFontFileName[ MAX_PATH ];
	char acFontFileExt[ 32 ];
	_splitpath_s( pFileName, NULL, 0, NULL, 0, acFontFileName, MAX_PATH, acFontFileExt, 32 );

	const char* pFontPathToLoad = NULL;
	char acFontFolder[ MAX_PATH ];
	char acFontFolderFullPath[ MAX_PATH ];
	SHGetFolderPathA( NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_DEFAULT, acFontFolder );
	sprintf_s( acFontFolderFullPath, "%s\\%s%s", acFontFolder, acFontFileName, acFontFileExt );
	WIN32_FIND_DATAA Result;
	HANDLE hFileFind = FindFirstFileA( acFontFolderFullPath, &Result );

	bool bExistInSystemFontFolder = false;
	if( INVALID_HANDLE_VALUE == hFileFind )
	{
		pFontPathToLoad = pFileName;
	}
	else
	{
		bExistInSystemFontFolder = true;
		pFontPathToLoad = acFontFolderFullPath;
		FindClose( hFileFind );
	}

	DWORD dwQuality(DEFAULT_QUALITY);
	int nOsType = GetOSVersionType();

	switch( nOsType )
	{
	case 1:								// 1 : Windows 95
	case 2:								// 2 : Windows 98, 98SE
	case 4:		dwQuality = 4;	break;	// 4 : Windows NT,			DEFAULT_QUALITY
	case 3:								// 3 : Windows ME
	case 5:		dwQuality = 5;	break;	// 5 : Windows 2000,		CLEARTYPE_QUALITY
	case 6:								// 6 : Windows XP
	case 7:								// 7 : Windows Server 2003
	case 8:		dwQuality = 6;	break;	// 8 : Windows Vista,		CLEARTYPE_NATURAL_QUALITY
	}

	FontInfo.nFontHeight = nNewFontHeight;
	DWORD dwFileSize = 0;

	if( m_FontMemCache.count( pFontPathToLoad  ) != 0) {
		FontInfo.pMemFont = m_FontMemCache[ pFontPathToLoad ].first;
		dwFileSize = m_FontMemCache[ pFontPathToLoad ].second;
	}
	else {	
		if( !bExistInSystemFontFolder )
		{
			CResMngStream Stream( pFontPathToLoad );
			if( !Stream.IsValid() ) {
				_ASSERT( !"Failed to read font file!" );
				CDebugSet::ToLogFile( "CEtFontMng::AddFont, Font file load fail (%s)", pFontPathToLoad );
				return -1;
			}

			Stream.Seek(0, SEEK_END);
			dwFileSize = Stream.Tell();
			Stream.Seek(0, SEEK_SET);
			FontInfo.pMemFont = new BYTE[ dwFileSize ];
			Stream.Read(FontInfo.pMemFont, dwFileSize);
			
			m_FontMemCache[ pFontPathToLoad ].first = FontInfo.pMemFont;
			m_FontMemCache[ pFontPathToLoad ].second = dwFileSize;
		}
		else
		{
			CEtStdFileIO Stream;
			if( Stream.Open( pFontPathToLoad, CEtStdFileIO::ET_READ ) )
			{
				Stream.Seek( 0, CEtStdFileIO::ET_SEEK_END );
				dwFileSize = Stream.Tell();
				Stream.Seek( 0, CEtStdFileIO::ET_SEEK_BEGIN );
				FontInfo.pMemFont = new BYTE[ dwFileSize ];
				Stream.Read(FontInfo.pMemFont, dwFileSize);

				m_FontMemCache[ pFontPathToLoad ].first = FontInfo.pMemFont;
				m_FontMemCache[ pFontPathToLoad ].second = dwFileSize;
				// Close() is in the destructor
			}
			else
			{
				_ASSERT( !"Failed to read font file!" );
				CDebugSet::ToLogFile( "CEtFontMng::AddFont, Font file load fail (%s)", pFontPathToLoad );
				return -1;
			}
		}
	}

	FT_Error error = FT_New_Memory_Face(m_Library, FontInfo.pMemFont, dwFileSize, 0, &FontInfo.Face);
	if( error ) 
	{
		CDebugSet::ToLogFile( "CEtFontMng::AddFont, FT_New_Face the return value is %d", error );
	}

	//HDC hDC = FontInfo.pFont->GetDC();
	//HDC hDC = ::GetDC( GetEtDevice()->GetHWnd() );
	//int nDPI = 72 * 72 / GetDeviceCaps(hDC, LOGPIXELSY);

	// Windows uses 96dpi resolution by default
	//int nXDPI = GetDeviceCaps( hDC, LOGPIXELSX );
	//int nYDPI = GetDeviceCaps( hDC, LOGPIXELSY );
	//FT_Set_Char_Size( FontInfo.Face, (nNewFontHeight) << 6, 0, nXDPI, nYDPI);
	FT_Set_Pixel_Sizes( FontInfo.Face, nNewFontHeight, nNewFontHeight );

	if( s_bUseUniscribe )
	{
		if( !IsExistAddFontResourceList( FontInfo ) )
		{
			DWORD dwFonts;
			HANDLE hFontHandle = AddFontMemResourceEx( FontInfo.pMemFont, dwFileSize, NULL, &dwFonts );
		
			if( hFontHandle )
			{
				SAddFontMemResource sAddFontMemResource;
				strcpy_s( sAddFontMemResource.szFontName, LF_FACESIZE, FontInfo.szFontName );
				sAddFontMemResource.hFontHandle = hFontHandle;
				m_vecAddFontMemResourceList.push_back( sAddFontMemResource );
			}
			else
			{
				_ASSERT( !"Failed to add font resource!" );
				CDebugSet::ToLogFile( "CEtFontMng::AddFont, AddFontMemResourceEx Fail! (%s)", pFontPathToLoad );
				return -1;
			}
		}
		
		HFONT hFont = CreateFontA( FontInfo.nFontHeight, 0, 0, 0, FW_NORMAL, FALSE, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
									CLIP_DEFAULT_PRECIS, dwQuality, FF_DONTCARE, FontInfo.szFontName );
		if( hFont )
		{
			SFontInfoHandle sFontInfoHandle;
			sFontInfoHandle.nFontHeight = FontInfo.nFontHeight;
			strcpy_s( sFontInfoHandle.szFontName, LF_FACESIZE, FontInfo.szFontName );
			sFontInfoHandle.hFont = hFont;
			m_vecStFontList.push_back( sFontInfoHandle );
		}
	}
	m_vecFont.push_back( FontInfo );
	return ( int )m_vecFont.size() - 1;
}

SCachedFont* CEtFontMng::GetCachedFontTexture( LPCWSTR szText, float fWidth, 
											  SFontDrawEffectInfo& FontEffectInfo, int nFontIndex )
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	SCachedFont* pResult = NULL;

	int i, nSize;
	nSize = (int)m_cachedFontList.size();
	for( i = 0; i < nSize; i++) 
	{
		SCachedFont& CachedFont = m_cachedFontList[i];
		if( CachedFont.nFontIndex == nFontIndex &&
			CachedFont.fWidth == fWidth &&
			CachedFont.FontEffectInfo.nDrawType == FontEffectInfo.nDrawType &&
			CachedFont.FontEffectInfo.fGlobalBlurAlphaWeight == FontEffectInfo.fGlobalBlurAlphaWeight &&
			wcscmp(CachedFont.szText.c_str(), szText) == 0 )
		{
			if( SFontDrawEffectInfo::NORMAL !=  FontEffectInfo.nDrawType )
			{
				if( (CachedFont.FontEffectInfo.nDrawType != FontEffectInfo.nDrawType) ||
					(CachedFont.FontEffectInfo.nWeight != FontEffectInfo.nWeight) ||
					(CachedFont.FontEffectInfo.fAlphaWeight != FontEffectInfo.fAlphaWeight) )//||
					/*( (CachedFont.FontEffectInfo.dwFontColor&0x00ffffff) != (FontEffectInfo.dwFontColor&0x00ffffff) ) ||
					( (CachedFont.FontEffectInfo.dwEffectColor&0x00ffffff) != (FontEffectInfo.dwEffectColor&0x00ffffff) ) )*/
						continue;
			}

			CachedFont.nLastUseTick = GetTickCount();
			pResult = &CachedFont;
			break;
		}
	}

	return pResult;
}

//SCachedFont* CEtFontMng::GetCachedFontTextureJustSameText( LPCWSTR szText, int nFontIndex )
//{
//	ScopeLock<CSyncLock> Lock(s_Lock);
//
//	SCachedFont* pResult = NULL;
//
//	int i, nSize;
//	nSize = (int)m_cachedFontList.size();
//	for( i = 0; i < nSize; i++) 
//	{
//		SCachedFont& CachedFont = m_cachedFontList[i];
//		if( CachedFont.nFontIndex == nFontIndex &&
//			wcscmp(CachedFont.szText.c_str(), szText) == 0 )
//		{
//			CachedFont.nLastUseTick = GetTickCount();
//			pResult = &CachedFont;
//			break;
//		}
//	}
//	return pResult;
//}

void CEtFontMng::UpdateFontCache( bool bRemoveCache )
{
	if( bRemoveCache && ((m_nUpdateTerm++) % 100 ) == 0 ) {
		int nCurrTick = GetTickCount();
		ScopeLock<CSyncLock> Lock(s_Lock);
		for( std::vector<SCachedFont>::iterator it = m_cachedFontList.begin(); it != m_cachedFontList.end(); ) {
			if( it->nLastUseTick + 2000 < nCurrTick ) {
				m_texturePool.push_back( it->TextureInfo.hTexture );
				if( m_texturePool.size() > 300 ) {
					EtTextureHandle hTexture = m_texturePool.front();
					SAFE_RELEASE_SPTR( hTexture );
					m_texturePool.pop_front();
				}

				SCaretKey CaretKey;
				CaretKey.szText = it->szText;
				CaretKey.nFontIndex = it->nFontIndex;
				m_caretInfo.erase( CaretKey );
//				bool bAlreadyExistCaret = (m_caretInfo.find( CaretKey ) != m_caretInfo.end() );

				it = m_cachedFontList.erase( it );

			}
			else {
				++it;
			}
		}
	}
}

void CEtFontMng::FlushFontCache()
{
	std::list< EtTextureHandle >::iterator it = m_texturePool.begin();
	for( std::list< EtTextureHandle >::iterator it = m_texturePool.begin(); it != m_texturePool.end(); it++ ) {
		SAFE_RELEASE_SPTR( (*it) );
	}
	m_texturePool.clear();

	int nSize = (int)m_cachedFontList.size();
	for( int i = 0; i < nSize; i++) {
		SAFE_RELEASE_SPTR( m_cachedFontList[i].TextureInfo.hTexture );
		SCaretKey CaretKey;
		CaretKey.szText = m_cachedFontList[i].szText;
		CaretKey.nFontIndex = m_cachedFontList[i].nFontIndex;
		m_caretInfo.erase( CaretKey );
	}
	m_cachedFontList.clear();
//	SAFE_RELEASE_SPTRVEC( m_texturePool );
	/*
	for( DWORD i=0; i<m_texturePool.size(); i++ ) {
		SAFE_RELEASE_SPTR( m_texturePool[i] );
	}
	m_texturePool.clear();
	*/
}

int CEtFontMng::GetCachedCharAdvanceX( FT_Face Face, SFontInfo& FontInfo, wchar_t ch, bool bBold, int iNowAdvance )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	int nAdvanceX = 0;

	if( false == bBold )
	{
		map<wchar_t, int>::iterator iter = FontInfo.mapAdvanceX.find( (wchar_t)ch );
		if( FontInfo.mapAdvanceX.end() != iter )
			nAdvanceX = iter->second;
	}
	else
	{
		map<wchar_t, int>::iterator iter = FontInfo.mapBoldAdvanceX.find( (wchar_t)ch );
		if( FontInfo.mapBoldAdvanceX.end() != iter )
			nAdvanceX = iter->second;
	}

	bool bCantCaching = false;
	if( 0 == nAdvanceX )
	{
		FT_Error err = FT_Load_Char( Face, ch, FT_LOAD_RENDER|/*FT_LOAD_NO_BITMAP|*/FT_LOAD_FORCE_AUTOHINT );
		if( err ) 
			return -1;

		if( bBold )
			FT_GlyphSlot_Embolden( Face->glyph );

		nAdvanceX = (Face->glyph->advance.x >> 6);

		if( iNowAdvance + Face->glyph->bitmap_left < 0 )
		{
			nAdvanceX += -(iNowAdvance + Face->glyph->bitmap_left);
			bCantCaching = true;
		}

		if( false == bCantCaching )
		{
			if( false == bBold )
				FontInfo.mapAdvanceX.insert( make_pair((wchar_t)ch, nAdvanceX) );
			else
				FontInfo.mapBoldAdvanceX.insert( make_pair((wchar_t)ch, nAdvanceX) );
		}
	}	

	return nAdvanceX;
}

void CEtFontMng::AccumulateColor( DWORD& dwAlpha, DWORD& dwRed, DWORD& dwGreen, DWORD& dwBlue, DWORD& dwNowColor, int iWeight )
{
	dwAlpha += ((0xff000000 & dwNowColor) >> 24) * iWeight;
	dwRed += ((0x00ff0000 & dwNowColor) >> 16) * iWeight;
	dwGreen += ((0x0000ff00 & dwNowColor) >> 8) * iWeight;
	dwBlue += (0x000000ff & dwNowColor) * iWeight;
}

void CEtFontMng::BlurProcess( BYTE* pOriginalBuffer, int nBytePos, bool* pabBlurDir, int nY, int nX, BYTE* pTextureBuffer, 
int nPitch, int nTexelWidth, int nTexelHeight, float fAlphaWeight )
{
	DWORD dwResultColor = 0;
	int nWeightCount = 0;

	// Don't blend alpha values.. let's check this first
	//DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos ]);
	//DWORD dwAlpha = 0xff000000 & *(DWORD*)(&pTextureBuffer[ nBytePos ]);
	DWORD dwAlpha = 0;
	DWORD dwRed = 0;
	DWORD dwGreen = 0;
	DWORD dwBlue = 0;

	// Top-left
	if( pabBlurDir[ 0 ] &&
	1 <= nY && 1 <= nX )
	{
		nWeightCount += 1;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos - nPitch - 1*sizeof(DWORD) ]);
		dwResultColor += dwNowColor;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );
	}

	// Top
	if( pabBlurDir[ 1 ] &&
	1 <= nY )
	{
		nWeightCount += 2;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos - nPitch ]);
		dwResultColor += dwNowColor*2;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );
	}

	// Top right
	if( pabBlurDir[ 2 ] &&
	1 <= nY && nX < nTexelWidth-1 )
	{
		nWeightCount += 1;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos - nPitch + 1*sizeof(DWORD) ]);
		dwResultColor += dwNowColor;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1);
	}

	// Left
	if( pabBlurDir[ 3 ] &&
	1 <= nX )
	{
		nWeightCount += 2;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos - sizeof(DWORD) ]);
		dwResultColor += dwNowColor*2;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );
	}

	// Current texel
	if( pabBlurDir[ 4 ] )
	{
		nWeightCount += 4;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos ]);
		dwResultColor += dwNowColor*4;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 4 );
	}

	// Right
	if( pabBlurDir[ 5 ] && 
	nX < nTexelWidth-1 )
	{
		nWeightCount += 2;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos + sizeof(DWORD) ]);
		dwResultColor += dwNowColor*2;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );
	}

	// Bottom-left
	if( pabBlurDir[ 6 ] &&
	nY < nTexelHeight-1 && 1 <= nX )
	{
		nWeightCount += 1;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos + nPitch - 1*sizeof(DWORD) ]);
		dwResultColor += dwNowColor;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );
	}

	// Low
	if( pabBlurDir[ 7 ] &&
	nY < nTexelHeight-1 )
	{
		nWeightCount += 2;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos + nPitch ]);
		dwResultColor += dwNowColor*2;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 2 );
	}

	// Bottom right
	if( pabBlurDir[ 8 ] &&
	nY < nTexelHeight-1 && nX < nTexelWidth-1 )
	{
		nWeightCount += 1;
		DWORD dwNowColor = *(DWORD*)(&pOriginalBuffer[ nBytePos + nPitch + 1*sizeof(DWORD) ]);
		dwResultColor += dwNowColor;
		AccumulateColor( dwAlpha, dwRed, dwGreen, dwBlue, dwNowColor, 1 );
	}

	if( 0 != nWeightCount )
	{
		dwAlpha /= nWeightCount;
		if( 0.0f != fAlphaWeight )
			dwResultColor = D3DCOLOR_ARGB( min(DWORD((float)dwAlpha*fAlphaWeight), 255), 255, 255, 255 );
		else
			dwResultColor = D3DCOLOR_ARGB( min(dwAlpha*(dwAlpha/50), 255), 255, 255, 255 );

		*(DWORD*)(&pTextureBuffer[ nBytePos ]) = dwResultColor;
	}

	//if( 0 != nWeightCount )
	//{
	//	dwAlpha /= nWeightCount;
	//	if( 0.0f != fAlphaWeight )
	//		dwResultColor = D3DCOLOR_ARGB( min(DWORD((float)dwAlpha*fAlphaWeight), 255), 255, 255, 255 );
	//	else
	//		dwResultColor = D3DCOLOR_ARGB( min(dwAlpha*(dwAlpha/50), 255), 255, 255, 255 );

	//	dwResultColor |= (dwColor & 0x00ffffff);
	//	//dwResultColor = D3DCOLOR_ARGB( min(dwAlpha*(dwAlpha/50), 255), dwRed/nWeightCount, dwGreen/nWeightCount, dwBlue/nWeightCount );
	//	//dwResultColor = D3DCOLOR_ARGB( min(dwAlpha, 255), dwRed/nWeightCount, dwGreen/nWeightCount, dwBlue/nWeightCount );
	//	//dwResultColor /= nWeightCount;
	//	*(DWORD*)(&pTextureBuffer[ nBytePos ]) = dwResultColor;
	//}
}

void CEtFontMng::Blur( BYTE* pTextureBuffer, BYTE* pOriginalBuffer, bool* pabBlurDir, 
					  float fAlphaWeight, int nPitch, int nTexelWidth, int nTexelHeight, int nTextTexelWidth, int nTextTexelHeight, bool bReverseLoop /*= false */ )
{
	int nEndBytePos = nTexelHeight*nPitch;

	if( false == bReverseLoop )
	{
		for( int nY = 0; nY < nTexelHeight; ++nY )
		{
			for( int nX = 0; nX < nTexelWidth; ++nX )
			{
				int nBytePos = nY*nPitch + nX*sizeof(DWORD);
				if( nBytePos < nEndBytePos )
				{
					BLUR_PROCESS( pOriginalBuffer, nBytePos, pabBlurDir, nY, nX, pTextureBuffer, nPitch, nTexelWidth, nTexelHeight, fAlphaWeight );
					//BlurProcess( pOriginalBuffer, nBytePos, pabBlurDir, nY, nX, pTextureBuffer, nPitch, nTexelWidth, nTexelHeight, fAlphaWeight );
				}
			}
		}
	}
	else
	{
		for( int nY = nTexelHeight-1; nY > 0; --nY )
		{
			for( int nX = nTexelWidth; nX > 0; --nX )
			{
				int nBytePos = nY*nPitch + nX*sizeof(DWORD);
				if( nBytePos < nEndBytePos )
				{
					BLUR_PROCESS(pOriginalBuffer, nBytePos, pabBlurDir, nY, nX, pTextureBuffer, nPitch, nTexelWidth, nTexelHeight, fAlphaWeight);
					//BlurProcess(pOriginalBuffer, nBytePos, pabBlurDir, nY, nX, pTextureBuffer, nPitch, nTexelWidth, nTexelHeight, fAlphaWeight);
				}
			}
		}
	}
}

SFontTextureInfo CEtFontMng::GetFontTexture( int nFontIndex, LPCWSTR szText, float fWidth, DWORD dwFontFormat, 
											 SFontDrawEffectInfo& Info, bool bRemoveCache /*= true */ )
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	SCaretKey CaretKey;
	CaretKey.szText = szText;
	CaretKey.nFontIndex = nFontIndex;

	bool bAlreadyExistCaret = ( m_caretInfo.find( CaretKey ) != m_caretInfo.end() );

	FT_Face Face = m_vecFont[ nFontIndex ].Face;
	bool bBold = m_vecFont[ nFontIndex ].nFontWeight*100 >= FW_BOLD;

	SCachedFont* pCachedTextureInfo = GetCachedFontTexture( szText, fWidth, Info, nFontIndex );
	if( pCachedTextureInfo )
		return pCachedTextureInfo->TextureInfo;

	SFontInfo& FontInfo = m_vecFont.at( nFontIndex );
	SCachedFont fkInfo;
	wstring strText = szText;

	// Calculate required texture size
	int nHeightPerLine = (int)floorf((float)Face->size->metrics.height / 64.0f);	// Rounding processing

	// Recalculate line spacing
	nHeightPerLine = int(nHeightPerLine * s_fLinePitchRate);
	int nLineGap = nHeightPerLine - (Face->size->metrics.y_ppem);

	vector<wstring> vecStrLine;
	vector<int> vecWidthList;

	int nNumLine = WordBreak( strText, CaretKey, bAlreadyExistCaret, Face, bBold, FontInfo, fWidth, vecStrLine, vecWidthList );
	if( nNumLine == -1 )
		return fkInfo.TextureInfo;

	int nLongestWidth = 0;
	for( int nLine=0; nLine<nNumLine; ++nLine )
	{
		int nWidth = vecWidthList.at( nLine );
		if( nLongestWidth < nWidth )
			nLongestWidth = nWidth;
	}

	int nWholeHeight = nHeightPerLine * nNumLine;

	int nNeedWidth = nLongestWidth;
	int nNeedHeight = nWholeHeight;
	if( SFontDrawEffectInfo::NORMAL != Info.nDrawType )
	{
		nNeedWidth += Info.nWeight*2;
		nNeedHeight += Info.nWeight*2;
	}

	if( s_bUseUniscribe )
		nNeedHeight += (int)( UPPER_PADDING_PIXEL );

	int nBufferTextureWidth = CEtTexture::CalcTextureSize( nNeedWidth );
	int nBufferTextureHeight = CEtTexture::CalcTextureSize( nNeedHeight );

	EtTextureHandle hFontTexture;

	std::list<EtTextureHandle>::iterator it = m_texturePool.begin();
	while( it != m_texturePool.end() ) {
		EtTextureHandle hTexture = *it;
		if( hTexture && 
			hTexture->Width() == nBufferTextureWidth && 
			hTexture->Height() == nBufferTextureHeight ) {
				hFontTexture = hTexture;
				it = m_texturePool.erase( it );
				break;
		}
		++it;
	}
	if( !hFontTexture ) {
		if( GetEtDevice()->IsAutoGenMipmap() ) {
			hFontTexture = CEtTexture::CreateNormalTexture( nBufferTextureWidth, nBufferTextureHeight, FMT_A8R8G8B8, USAGE_AUTOGENMIPMAP, POOL_MANAGED, 0 );
			if( !hFontTexture->GetTexturePtr() ) {
				SAFE_RELEASE_SPTR( hFontTexture );
				hFontTexture = CEtTexture::CreateNormalTexture( nBufferTextureWidth, nBufferTextureHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
			}
		}
		else {
			hFontTexture = CEtTexture::CreateNormalTexture( nBufferTextureWidth, nBufferTextureHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
		}
	}
	/*
	else {
		hFontTexture->ChangeFormat( FMT_A8R8G8B8 );
	}
	*/

	int nFontTexturePitch = 0;
	DWORD *pPtr = (DWORD*)hFontTexture->Lock( nFontTexturePitch, true );

	if( pPtr ) {
		BYTE *pFontTextureBuffer = (BYTE*)pPtr;

		// Clear the entire buffer since it can be reused.
		for( int y = 0; y < nBufferTextureHeight; y++) {
			memset( pFontTextureBuffer + y * nFontTexturePitch, 0x0, nBufferTextureWidth*sizeof(DWORD) );
		}

		// Copy font bitmap data to the created texture	
		// For debugging
/*
		DWORD* adwTest = new DWORD[ nBufferTextureWidth*nBufferTextureHeight ];
		memset( adwTest, 200, nBufferTextureWidth*nBufferTextureHeight*sizeof(DWORD) );
		memcpy( pPtr, adwTest, nBufferTextureWidth*nBufferTextureHeight*sizeof(DWORD) );
		delete [] adwTest;
		DWORD dwRed = ((Info.dwFontColor) & 0x00ff0000) >> 16;
		DWORD dwGreen = ((Info.dwFontColor) & 0x00000ff00) >> 8;
		DWORD dwBlue = Info.dwFontColor & 0x000000ff;
*/
		int nBaseLinePos = int((float)(nHeightPerLine-nLineGap) * 0.9f);
		int nPenX = 0, nPenY = 0;

		//bool bHasKerningInfo = FT_HAS_GETKERNING( Face );

		for( int nLine = 0; nLine < nNumLine; ++nLine )
		{
			// Update Pen position per line
			int nXCenterOffset = 0;
			if( dwFontFormat & DT_CENTER ) 
				nXCenterOffset = (nLongestWidth - vecWidthList.at(nLine)) / 2;

			nPenX = 0;
			nPenY = ((nHeightPerLine*nLine + nBaseLinePos) << 6);

			//if( SFontDrawEffectInfo::NORMAL != Info.nDrawType )
			//{
			//	nPenX += (Info.nWeight << 6);
			//	nPenY += (Info.nWeight << 6);
			//}
			int iEffectXOffset = 0;
			int iEffectYOffset = 0;
			if( SFontDrawEffectInfo::NORMAL != Info.nDrawType )
			{
				iEffectXOffset += Info.nWeight;
				iEffectYOffset += Info.nWeight;
			}

			int nCharIndex = 0;
			int iFTBitmapLeftOffset = 0;		// Check because glyph's bitmap_left value can be negative (daum_regular font..).
			int iRenderXOffset = 0;
			wstring& strLine = vecStrLine.at(nLine);
			int nStrLength = (int)strLine.length();

			if( s_bUseUniscribe )
			{
				vector<WORD> vecGlyph;
				vector<GOFFSET> vecGOffset;
				
				int nGlyphCount = GetGlyphGOffset( m_vecFont[ nFontIndex ], strLine, vecGlyph, vecGOffset );
				nPenX = 0;
				nPenY = ((nHeightPerLine*nLine + nBaseLinePos) << 6);

				for( int i=0; i<nGlyphCount; i++ )
				{
					FT_Glyph Glyph;
					if( FT_Load_Glyph( Face, vecGlyph[i], FT_LOAD_DEFAULT ) )
						continue;

					if( bBold )
						FT_GlyphSlot_Embolden( Face->glyph );
					
					// Load the glyph data into our local array.
					if( FT_Get_Glyph( Face->glyph, &Glyph ) )
						continue;

					// Convert the glyph to bitmap form.
					FT_Glyph_To_Bitmap( &Glyph, FT_RENDER_MODE_NORMAL, NULL, TRUE );
					FT_Bitmap &Bitmap = ((FT_BitmapGlyph)Glyph)->bitmap;

					int nGlyphBitmapWidth = Bitmap.width;
					int nGlyphBitmapHeight = Bitmap.rows;

					for( int nY = 0; nY < nGlyphBitmapHeight; ++nY )
					{
						for( int nX = 0; nX < nGlyphBitmapWidth; ++nX )
						{
							int nColor = Bitmap.buffer[ nY*nGlyphBitmapWidth + nX ];

							int nBytePos = ((nPenY >> 6)  - vecGOffset[i].dv - ((FT_BitmapGlyph)Glyph)->top + nY + (int)( UPPER_PADDING_PIXEL ) + iEffectYOffset) * nFontTexturePitch +
										   ((nPenX >> 6)  + vecGOffset[i].du + ((FT_BitmapGlyph)Glyph)->left + nX + nXCenterOffset + iEffectXOffset + iRenderXOffset) * sizeof(DWORD);
							/*ASSERT( 0 <= nBytePos );*/
							if( nColor > 0 && 0 <= nBytePos && nBytePos <= int(nBufferTextureWidth*nBufferTextureHeight*sizeof(DWORD)) )
							{
								BYTE* pTexel = pFontTextureBuffer + nBytePos;
								*(DWORD*)pTexel = D3DCOLOR_ARGB( nColor, 255, 255, 255 );
							}
						}
					}

					nPenX += Face->glyph->advance.x;
					nPenY += Face->glyph->advance.y;

					FT_Done_Glyph( Glyph );
				}

				vecGlyph.clear();
				vecGOffset.clear();
			}
			else
			{
				nPenX = 0;
				nPenY = ((nHeightPerLine*nLine + nBaseLinePos) << 6);

				for( nCharIndex = 0; nCharIndex < nStrLength; ++nCharIndex )
				{
					wchar_t Char = strLine.at( nCharIndex );
					FT_Error err = FT_Load_Char( Face, Char, FT_LOAD_RENDER|/*FT_LOAD_NO_BITMAP|*/FT_LOAD_FORCE_AUTOHINT );
					if( err ) 
						continue;

					if( bBold )
						FT_GlyphSlot_Embolden( Face->glyph );

					int nGlyphBitmapWidth = Face->glyph->bitmap.width;
					int nGlyphBitmapHeight = Face->glyph->bitmap.rows;

					for( int nY = 0; nY < nGlyphBitmapHeight; ++nY )
					{
						for( int nX = 0; nX < nGlyphBitmapWidth; ++nX )
						{
							int nColor = Face->glyph->bitmap.buffer[ nY*nGlyphBitmapWidth + nX ];

							int nBytePos = ((nPenY >> 6) - Face->glyph->bitmap_top + nY + iEffectYOffset)*nFontTexturePitch + \
										   ((nPenX >> 6) + Face->glyph->bitmap_left + nX + nXCenterOffset /*- iFTBitmapLeftOffset*/ + iEffectXOffset + iRenderXOffset)*sizeof(DWORD);
							/*ASSERT( 0 <= nBytePos );*/
							if( nColor > 0 && 0 <= nBytePos && nBytePos <= int(nBufferTextureWidth*nBufferTextureHeight*sizeof(DWORD)) )
							{
								BYTE* pTexel = pFontTextureBuffer + nBytePos;
								//*(DWORD*)pTexel = D3DCOLOR_ARGB( nColor, nColor, nColor, nColor );

								//if( SFontDrawEffectInfo::NORMAL != Info.nDrawType )
								//{
								//if( 255 == nColor )
								//	*(DWORD*)pTexel = D3DCOLOR_ARGB( z255, nColor, nColor, nColor );
								//else
								*(DWORD*)pTexel = D3DCOLOR_ARGB( nColor, 255, 255, 255 );
								//*(DWORD*)pTexel = D3DCOLOR_ARGB( 255, nColor, nColor, nColor );
								//}
								//else
								//{
								//	if( 255 == nColor )
								//		*(DWORD*)pTexel = D3DCOLOR_ARGB( DWORD((float)255*fFontAlpha), dwRed, dwGreen, dwBlue);
								//	else
								//		*(DWORD*)pTexel = D3DCOLOR_ARGB( DWORD((float)nColor*fFontAlpha), dwRed, dwGreen, dwBlue );
								//}
							}
						}
					}

					nPenX += Face->glyph->advance.x;
					nPenY += Face->glyph->advance.y;
				}
			}
		}
		// Post-processing by font usage type
		if( /*0.0f != Info.fGlobalBlurAlphaWeight || */
			SFontDrawEffectInfo::NORMAL != Info.nDrawType )
		{
			BYTE* apCopyBuffer = new BYTE[nBufferTextureHeight*nFontTexturePitch];

			bool abDirection[ 9 ];
			ZeroMemory( abDirection, sizeof(abDirection) );

			//if( SFontDrawEffectInfo::NORMAL == Info.nDrawType )
			//{
			//	// Filter only 2 directions (up/down)
			//	abDirection[ 1 ] /*= abDirection[ 3 ]*/ = abDirection[ 4 ] /*= abDirection[ 5 ]*/ = abDirection[ 7 ] = true;

			//	memcpy( apCopyBuffer, pFontTextureBuffer, nBufferTextureHeight*nFontTexturePitch );
			//	Blur( pFontTextureBuffer, apCopyBuffer, abDirection, Info.fGlobalBlurAlphaWeight,
			//		nFontTexturePitch, nBufferTextureWidth, nBufferTextureHeight, nLongestWidth, nWholeHeight-nLineGap+1, false );
			//}
			//else
			{
				float fAlphaWeight = Info.fAlphaWeight;
				bool bReverseLoop = false;
				switch( Info.nDrawType )
				{
					case SFontDrawEffectInfo::SHADOW:
						{
							// Process blur referencing only top-left pixels.
							abDirection[ 0 ] = abDirection[ 1 ] = abDirection[ 2 ] = abDirection[ 3 ] = abDirection[ 4 ] = true;

							// Blur result is affected by the direction in which blur is applied.
							bReverseLoop = true;
						}
						break;

					case SFontDrawEffectInfo::STROKE:
						{
							memset( abDirection, true, sizeof(abDirection) );
							// Not used in STROKE.
							fAlphaWeight = 0.0f;		// If alpha weight is 0, uses squared texture alpha.
						}
						break;

					case SFontDrawEffectInfo::GLOW:
						{
							memset( abDirection, true, sizeof(abDirection) );
						}
						break;
				}

				for( int i=0; i<Info.nWeight; ++i )
				{
					memcpy( apCopyBuffer, pFontTextureBuffer, nBufferTextureHeight*nFontTexturePitch );
					Blur( pFontTextureBuffer, apCopyBuffer, abDirection, fAlphaWeight,
						  nFontTexturePitch, nBufferTextureWidth, nBufferTextureHeight, nLongestWidth, nWholeHeight-nLineGap+1, bReverseLoop );
				}
			}

			delete [] apCopyBuffer;
		}

		hFontTexture->Unlock();
	}
	/*
	if( hFontTexture ) {
		hFontTexture->ChangeFormat( FMT_DXT5 );
	}
	*/

	fkInfo.szText = szText;
	fkInfo.nFontIndex = nFontIndex;
	fkInfo.bBold = bBold;
	fkInfo.TextureInfo.hTexture = hFontTexture;
	fkInfo.TextureInfo.Width = nLongestWidth;
	fkInfo.TextureInfo.Height = nWholeHeight-nLineGap;		// Whether multi-line or single-line, subtract line gap from metric.height (which includes default line spacing) to return actual font size
	fkInfo.FontEffectInfo = Info;
	fkInfo.fWidth = fWidth;
	fkInfo.nLastUseTick = GetTickCount();
	m_cachedFontList.push_back( fkInfo );

	if( !bAlreadyExistCaret && 1 == nNumLine ) {
		m_caretInfo[ CaretKey ].nHeight = fkInfo.TextureInfo.Height;
	}

	UpdateFontCache( bRemoveCache );

	return fkInfo.TextureInfo;
}

void CEtFontMng::GetBasicFontTextureInfo(SFontTextureInfo& result, int nFontSetIndex, int nFontHeight, LPCWSTR szText, DWORD dwFontFormat, const SUICoord &ScreenCoord, SFontDrawEffectInfo& Info)
{
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	if( nFontIndex < 0 || nFontIndex >= (int)m_vecFont.size() )
		return;

	float fTempWidth = ScreenCoord.fWidth * GetEtDevice()->Width();
	float fTempHeight = ScreenCoord.fHeight * GetEtDevice()->Height();

	SUICoord ScreenCoordBackup = ScreenCoord;

	SFontDrawEffectInfo NormalEffectInfo = Info;
	NormalEffectInfo.nDrawType = SFontDrawEffectInfo::NORMAL;
	NormalEffectInfo.fGlobalBlurAlphaWeight = 0.0f;
	result = GetFontTexture(nFontIndex, szText, fTempWidth, dwFontFormat, NormalEffectInfo, true);
}

bool CEtFontMng::IsExistAddFontResourceList( SFontInfo& stFontInfo )
{
	bool bExist = false;
	for( int i=0; i<static_cast<int>( m_vecAddFontMemResourceList.size() ); i++ )
	{
		if( strcmp( m_vecAddFontMemResourceList[i].szFontName, stFontInfo.szFontName ) == 0 )
		{
			bExist = true;
			break;
		}
	}

	return bExist;
}

HFONT CEtFontMng::FindFontHandle( SFontInfo& stFontInfo )
{
	HFONT hFont = NULL;
	for( int i=0; i<static_cast<int>( m_vecStFontList.size() ); i++ )
	{
		if( m_vecStFontList[i].nFontHeight == stFontInfo.nFontHeight
			&& strcmp( m_vecStFontList[i].szFontName, stFontInfo.szFontName ) == 0 )
		{
			hFont = m_vecStFontList[i].hFont;
			break;
		}
	}

	return hFont;
}

int CEtFontMng::GetGlyphGOffset( SFontInfo& stFontInfo, wstring& strText, vector<WORD>& vecGlyph, vector<GOFFSET>& vecGOffset )
{
	HDC hDC = ::GetDC( GetEtDevice()->GetHWnd() );

	if( hDC == NULL )
		return 0;

	HFONT hFont = FindFontHandle( stFontInfo );
	if( hFont == NULL )
	{
		::ReleaseDC( GetEtDevice()->GetHWnd(), hDC );
		return 0;
	}

	HFONT hOldFont = (HFONT)SelectObject( hDC, hFont );
	
	HRESULT hResult;
	int nStrLength = static_cast<int>( strText.length() );
	if( nStrLength <= 0 )
		return 0;

	const SCRIPT_CONTROL *psControl = NULL;
	const SCRIPT_STATE *psState = NULL;
	int cMaxItems = nStrLength + 1;
	SCRIPT_ITEM* psItems = new SCRIPT_ITEM[cMaxItems];
	int nItemCount;

	/* Itemize */
	hResult = ScriptItemize( strText.c_str(), nStrLength, cMaxItems, psControl, psState, psItems, &nItemCount );
	if( SUCCEEDED( hResult ) == FALSE )
	{
		SAFE_DELETE( psItems );
		::ReleaseDC( GetEtDevice()->GetHWnd(), hDC );
		return 0;
	}

	SCRIPT_CACHE sCache = NULL;
	WORD* pwdClusts = NULL;
	WORD* pwdGlyphs = NULL;
	SCRIPT_VISATTR* psVisattr = NULL;
	int* pnAdvance = NULL;
	GOFFSET* pGOffset = NULL;

	for( int i=0; i<nItemCount; i++ )
	{
		int nIndex = psItems[i].iCharPos;
		int nChars = psItems[i+1].iCharPos - nIndex;
		int nMaxGlyph = nChars * 2 + 16;
		int nGlyphCount = 0;

		pwdClusts = new WORD[nMaxGlyph];
		pwdGlyphs = new WORD[nMaxGlyph];
		psVisattr = new SCRIPT_VISATTR[nMaxGlyph];

		/* Shape */
		hResult = ScriptShape( hDC, &sCache, strText.c_str()+nIndex, nChars, nMaxGlyph, &psItems[i].a, pwdGlyphs, pwdClusts, psVisattr, &nGlyphCount );
		if( SUCCEEDED( hResult ) == FALSE )
			break;

		ABC abc;
		pGOffset = new GOFFSET[nMaxGlyph];
		pnAdvance = new int[nMaxGlyph];

		/* Place */
		hResult = ScriptPlace( hDC, &sCache, pwdGlyphs, nGlyphCount, psVisattr, &psItems[i].a, pnAdvance, pGOffset, &abc );
		if( SUCCEEDED( hResult ) == FALSE )
			break;

		for( int k=0; k<nGlyphCount; k++ )
		{
			vecGOffset.push_back( pGOffset[k] );
			vecGlyph.push_back( pwdGlyphs[k] );
		}

		SAFE_DELETEA( pwdClusts );
		SAFE_DELETEA( pwdGlyphs );
		SAFE_DELETEA( psVisattr );		
		SAFE_DELETEA( pnAdvance );
		SAFE_DELETEA( pGOffset	);
	}

	SAFE_DELETEA( psItems );
	SAFE_DELETEA( pwdClusts );
	SAFE_DELETEA( pwdGlyphs );
	SAFE_DELETEA( psVisattr );	
	SAFE_DELETEA( pnAdvance );
	SAFE_DELETEA( pGOffset	);

	ScriptFreeCache(&sCache);
	SelectObject( hDC, hOldFont );
	::ReleaseDC( GetEtDevice()->GetHWnd(), hDC );

	return static_cast<int>( vecGOffset.size() );
}

typedef struct _tagTT_OFFSET_TABLE
{
	USHORT	uMajorVersion;
	USHORT	uMinorVersion;
	USHORT	uNumOfTables;
	USHORT	uSearchRange;
	USHORT	uEntrySelector;
	USHORT	uRangeShift;
}TT_OFFSET_TABLE;

typedef struct _tagTT_TABLE_DIRECTORY
{
	char	szTag[4];			//table name
	ULONG	uCheckSum;			//Check sum
	ULONG	uOffset;			//Offset from beginning of file
	ULONG	uLength;			//length of the table in bytes
}TT_TABLE_DIRECTORY;

typedef struct _tagTT_NAME_TABLE_HEADER
{
	USHORT	uFSelector;			//format selector. Always 0
	USHORT	uNRCount;			//Name Records count
	USHORT	uStorageOffset;		//Offset for strings storage, from start of the table
}TT_NAME_TABLE_HEADER;

typedef struct _tagTT_NAME_RECORD
{
	USHORT	uPlatformID;
	USHORT	uEncodingID;
	USHORT	uLanguageID;
	USHORT	uNameID;
	USHORT	uStringLength;
	USHORT	uStringOffset;	//from start of storage area
}TT_NAME_RECORD;

#define SWAPWORD(x)		MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)		MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

void CEtFontMng::GetTTFLocalFontName( const char* pFontFileName, char* pChangeFontName )
{
	if( pFontFileName == NULL || pChangeFontName == NULL )
		return;

	CResMngStream Stream;
	Stream.Open( pFontFileName );
	if( !Stream.IsValid() ) 
	{
		Stream.Close();
		return;
	}

	TT_OFFSET_TABLE ttOffsetTable;
	Stream.Read( &ttOffsetTable, sizeof( TT_OFFSET_TABLE ) );
	ttOffsetTable.uNumOfTables = SWAPWORD( ttOffsetTable.uNumOfTables );
	ttOffsetTable.uMajorVersion = SWAPWORD( ttOffsetTable.uMajorVersion );
	ttOffsetTable.uMinorVersion = SWAPWORD( ttOffsetTable.uMinorVersion );

	//check is this is a true type font and the version is 1.0
//	if( ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0 )
//	{
//		Stream.Close();
//		return;
//	}

	TT_TABLE_DIRECTORY tblDir;
	BOOL bFound = FALSE;
	CString csTemp;

	for( int i=0; i<ttOffsetTable.uNumOfTables; i++ )
	{
		Stream.Read( &tblDir, sizeof( TT_TABLE_DIRECTORY ) );
		strncpy( csTemp.GetBuffer(5), tblDir.szTag, 4 );
		csTemp.ReleaseBuffer( 4 );
		if( csTemp.CompareNoCase( _T( "name" ) ) == 0 ) 
		{
			bFound = TRUE;
			tblDir.uLength = SWAPLONG( tblDir.uLength );
			tblDir.uOffset = SWAPLONG( tblDir.uOffset );
			break;
		}
	}

	if( bFound )
	{
		Stream.Seek( tblDir.uOffset, SEEK_SET );
		TT_NAME_TABLE_HEADER ttNTHeader;
		Stream.Read( &ttNTHeader, sizeof( TT_NAME_TABLE_HEADER ) );
		ttNTHeader.uNRCount = SWAPWORD( ttNTHeader.uNRCount );
		ttNTHeader.uStorageOffset = SWAPWORD( ttNTHeader.uStorageOffset );
		TT_NAME_RECORD ttRecord;
		bFound = FALSE;

		for( int i=0; i<ttNTHeader.uNRCount; i++ )
		{
			Stream.Read( &ttRecord, sizeof( TT_NAME_RECORD ) );
			ttRecord.uNameID = SWAPWORD( ttRecord.uNameID );
			if( ttRecord.uNameID == 4 )
			{
				ttRecord.uStringLength = SWAPWORD( ttRecord.uStringLength );
				ttRecord.uStringOffset = SWAPWORD( ttRecord.uStringOffset );
				int nPos = Stream.SeekCur<int>();
				Stream.Seek( tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset, SEEK_SET );

				char* pCharFontName = new char[ ttRecord.uStringLength + 1 ];
				ZeroMemory( pCharFontName, ttRecord.uStringLength + 1 );
				Stream.Read( pCharFontName, ttRecord.uStringLength );

				if( strlen( pCharFontName ) > 0 )
				{
					strcpy_s( pChangeFontName, LF_FACESIZE, pCharFontName );
				}
				else if( pCharFontName[0] == '\0' )	// UTf8 encoding font name
				{
					int nCount = 0;
					for( int j=1; j<ttRecord.uStringLength; j+=2 )
					{
						pChangeFontName[ nCount++ ] = pCharFontName[j];
					}
				}

				SAFE_DELETEA( pCharFontName );
				if( strlen( pChangeFontName ) > 0 )
					break;

				Stream.Seek( nPos, SEEK_SET );
			}
		}	
	}

	Stream.Close();
}

int CEtFontMng::GetCaretPos( LPCWSTR szText, int& nFontSetIndex, int nFontHeight, int nCharIndex, int* pHeight/* = NULL*/, bool bUseCache )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	
	if( nCharIndex <= 0 )
	{
		ASSERT( nCharIndex <= 0 );
		return 0;
	}

	nCharIndex -= 1;

	int nLen = (int)wcslen( szText );

	ASSERT( m_vecFont.size() && "CEtFontMng::GetCaretPos" );
	if( m_vecFont.empty() ) {
		return -1;
	}
	ASSERT( nFontHeight >=0 && "CEtFontMng::GetCaretPos" );

	if( nLen == 0 ) {
		return -1;
	}

	ASSERT( nCharIndex < nLen && "CEtFontMng::DrawTextW" );

	int nSentenceWidth = 0;
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	bool bBold = (m_vecFont[ nFontIndex ].nFontWeight*100 >= FW_BOLD);

	FT_Face Face = m_vecFont[ nFontIndex ].Face;

	// If it's in cache, return the precomputed value
	if (bUseCache)
	{
		SCaretKey caretKey;
		caretKey.szText = szText;
		caretKey.nFontIndex = nFontIndex;

		if( m_caretInfo.count( caretKey ) != 0 ) 
		{
			const SCaretInfo &caretInfo = m_caretInfo[ caretKey ];
			ASSERT( wcscmp(caretKey.szText.c_str(), szText) == 0 );

			// If nCharIndex is greater by 1, simply return the very end position. 
			// That happens when a string contains \n in the middle.
			if( nCharIndex < (int)caretInfo.vecCaretPos.size() )
				nSentenceWidth = caretInfo.vecCaretPos.at( nCharIndex );
			else
				nSentenceWidth = caretInfo.vecCaretPos.back();

			if( pHeight )
				*pHeight = caretInfo.nHeight;

			return nSentenceWidth;
		}
	}

	SFontInfo& FontInfo = m_vecFont[ nFontIndex ];

	for ( int i = 0; i <= nCharIndex; ++i )
	{
		FT_ULong ch = szText[ i ];

		int nAdvanceX =  GetCachedCharAdvanceX( Face, FontInfo, (wchar_t)ch, bBold, nSentenceWidth );
		if( nAdvanceX < 0 )
			continue;

		// Accumulate total width per line
		nSentenceWidth += nAdvanceX;
	}

	// NOTE: If the height value is just passed like this, for multi-line text containing \n
	// rect value may not be calculated properly in CalcRect 
	// Calling this function from CalcRect was wrong in the first place..
	if( pHeight )
		*pHeight = Face->size->metrics.y_ppem;

	return nSentenceWidth;
}

int CEtFontMng::GetCaretFromCaretPos( LPCWSTR szText, int& nFontSetIndex, int nFontHeight, int nCaretPos, int& nTrail )
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	int nLen = (int)wcslen( szText );

	ASSERT( m_vecFont.size() && "CEtFontMng::GetCaretPos" );
	ASSERT( nFontHeight >=0 && "CEtFontMng::GetCaretPos" );

	if( nLen == 0 ) {
		return -1;
	}

	nTrail = 0;
	int nCaretIndex = 0;
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	bool bBold = (m_vecFont[ nFontIndex ].nFontWeight*100 >= FW_BOLD);

	// If it's in cache, return the precomputed value
	SCaretKey caretKey;
	caretKey.szText = szText;
	caretKey.nFontIndex = nFontIndex;

	if( m_caretInfo.count( caretKey ) != 0 ) 
	{
		const SCaretInfo &caretInfo = m_caretInfo[ caretKey ];

		ASSERT( wcscmp(caretKey.szText.c_str(), szText) == 0 );
		map<int, int>::const_iterator iter = caretInfo.mapPosByCaret.find( nCaretPos );

		if( caretInfo.mapPosByCaret.end() != iter )
		{
			nCaretIndex = iter->second;
			return nCaretIndex;
		}
		else
		{
			// Returns the closest value..
			int nApproximateCaretIndex = (int)caretInfo.vecCaretPos.size()-1;
			nTrail = 1;
			int nNumCaret = (int)caretInfo.vecCaretPos.size();
			for( int nCaret = 0; nCaret < nNumCaret; ++nCaret )
			{
				int nNowCaretPos = caretInfo.vecCaretPos.at( nCaret );
				if( nNowCaretPos > nCaretPos )
				{
					nApproximateCaretIndex = nCaret;
					nTrail = 0;
					break;
				}
			}
			return nApproximateCaretIndex;
		}
	}

	SFontInfo& FontInfo = m_vecFont[ nFontIndex ];
	int nSentenceWidth = 0;
	FT_Face Face = m_vecFont[ nFontIndex ].Face;
	for ( int i = 0; i < nLen; ++i )
	{
		int nAdvanceX = GetCachedCharAdvanceX( Face, FontInfo, szText[i], bBold, nSentenceWidth );
		if( nAdvanceX < 0 )
			continue;

		// Accumulate total width per line
		nSentenceWidth += nAdvanceX;
		if( nCaretPos < nSentenceWidth )
		{
			nCaretIndex = i;
			return nCaretIndex;
		}
	}

	// If not found, simply return the end
	nTrail = 1;
	nCaretIndex = nLen - 1;
	return nCaretIndex;
}

void CEtFontMng::DrawTextW( int &nFontSetIndex, int nFontHeight, LPCWSTR szText, DWORD dwFontFormat, SUICoord &ScreenCoord, int nCount, 
						   SFontDrawEffectInfo& FontEffectInfo, bool bRemoveCache /*= true */, float ZValue/* = 0.f*/, DWORD dwBgColor/* = 0*/, int nBorderFlag/* = 0*/)
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	ASSERT(m_vecFont.size()&&"CEtFontMng::DrawTextW");
	ASSERT(nFontSetIndex>=0&&"CEtFontMng::DrawTextW");


	
	int nLen = (int)wcslen( szText );
	if( nLen == 0 ) {
		return;
	}
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	if( nFontIndex < 0 || nFontIndex >= (int)m_vecFont.size() ) return;


	float fWidth = ScreenCoord.fWidth * GetEtDevice()->Width();
	float fHeight = ScreenCoord.fHeight * GetEtDevice()->Height();

	SUICoord ScreenCoordBackup = ScreenCoord;

	// If a Global Blur value exists, render one more pass separately from other font effects.
	SFontDrawEffectInfo NormalEffectInfo = FontEffectInfo;
	NormalEffectInfo.nDrawType = SFontDrawEffectInfo::NORMAL;
	NormalEffectInfo.fGlobalBlurAlphaWeight = 0.0f;
	SFontTextureInfo TextureInfo = GetFontTexture( nFontIndex, szText, fWidth, dwFontFormat, NormalEffectInfo, bRemoveCache );

	float fShear = m_vecFont[ nFontIndex ].bItalic ? 0.3f : 0.0f;

	float fHalfSizeX = ((fWidth-TextureInfo.Width)*0.5f) / GetEtDevice()->Width();
	float fHalfSizeY = ((fHeight-TextureInfo.Height)*0.5f) / GetEtDevice()->Height();

	ScreenCoord.fX += fHalfSizeX;
	ScreenCoord.fY += fHalfSizeY;

	if( dwFontFormat & DT_RIGHT ) {
		ScreenCoord.fX += fHalfSizeX;
	}
	else if ( !(dwFontFormat & DT_CENTER) ){
		ScreenCoord.fX -= fHalfSizeX;
	}
	if( dwFontFormat & DT_BOTTOM ) {
		ScreenCoord.fY += fHalfSizeY;
	}
	else if ( !(dwFontFormat & DT_VCENTER) ){
		ScreenCoord.fY -= fHalfSizeY;
	}

	float fFracX = ScreenCoord.fX * GetEtDevice()->Width();
	float fFracY = ScreenCoord.fY * GetEtDevice()->Height();
	fFracX -= (int)fFracX;
	fFracY -= (int)fFracY;
	ScreenCoord.fX -= fFracX / GetEtDevice()->Width();
	ScreenCoord.fY -= fFracY / GetEtDevice()->Height();

	EtTextureHandle hFontTexture = TextureInfo.hTexture;
	if( !hFontTexture ) return;
	ScreenCoord.fWidth  = (float)hFontTexture->Width() / GetEtDevice()->Width();
	ScreenCoord.fHeight = (float)hFontTexture->Height() / GetEtDevice()->Height();	

	SUICoord UVCoord;
	UVCoord.SetCoord( 0.0f, 0.0f, 1.0f, 1.0f );

#if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )
	if( true == m_bForceBlur && m_vecFont[ nFontIndex ].nFontHeight < m_nMinimizeSize && FontEffectInfo.fGlobalBlurAlphaWeight < m_fBlurWeight )
		FontEffectInfo.fGlobalBlurAlphaWeight = m_fBlurWeight;
#endif	// #if defined( PRE_MOD_MINIMIZE_FORCE_BLUR )

	const bool bBegin = CEtSprite::GetInstance().IsBegin();

	if( !bBegin ) 
		CEtSprite::GetInstance().Begin( 0 );

	if( dwBgColor != 0 )
	{	
		SUICoord RectCoord;
		RectCoord.fX = ScreenCoord.fX - ( (((nBorderFlag&1)>0)?4.f:0.f) / GetEtDevice()->Width());
		RectCoord.fY = ScreenCoord.fY - ( (((nBorderFlag&4)>0)?2.f:0.f) / GetEtDevice()->Height());
		int nAddWidth = ((nBorderFlag&1)>0)?4:0; nAddWidth += ((nBorderFlag&2)>0)?4:0;
		int nAddHeight = ((nBorderFlag&4)>0)?2:0; nAddHeight += ((nBorderFlag&8)>0)?3:0;
		RectCoord.fWidth = (float)(TextureInfo.Width + nAddWidth) / GetEtDevice()->Width();
		RectCoord.fHeight= (float)(TextureInfo.Height + nAddHeight) / GetEtDevice()->Height();
		CEtSprite::GetInstance().DrawRect( RectCoord, dwBgColor, ZValue );
	}

	if( SFontDrawEffectInfo::NORMAL != FontEffectInfo.nDrawType )
	{
		SFontTextureInfo FontEffectTexture = GetFontTexture( nFontIndex, szText, fWidth, dwFontFormat, FontEffectInfo, bRemoveCache );

		SUICoord EffectFontCoord = ScreenCoord;
		if( s_bUseUniscribe )
		{
			EffectFontCoord.fX = ScreenCoord.fX - ((float)FontEffectInfo.nWeight / (float)GetEtDevice()->Width() );
			EffectFontCoord.fY = ScreenCoord.fY - (( (float)FontEffectInfo.nWeight + UPPER_PADDING_PIXEL ) / (float)GetEtDevice()->Height() );
		}
		else
		{
			EffectFontCoord.fX = ScreenCoord.fX - ((float)FontEffectInfo.nWeight / (float)GetEtDevice()->Width() );
			EffectFontCoord.fY = ScreenCoord.fY - ((float)FontEffectInfo.nWeight / (float)GetEtDevice()->Height() );
		}
		if( FontEffectTexture.hTexture )
		{
			EffectFontCoord.fWidth  = (float)FontEffectTexture.hTexture->Width() / GetEtDevice()->Width();
			EffectFontCoord.fHeight = (float)FontEffectTexture.hTexture->Height() / GetEtDevice()->Height();

			CEtSprite::GetInstance().DrawSprite( (EtTexture*)FontEffectTexture.hTexture->GetTexturePtr(), 
													FontEffectTexture.hTexture->Width(), FontEffectTexture.hTexture->Height(), 
													UVCoord, FontEffectInfo.dwEffectColor, EffectFontCoord, 0.f, ZValue, fShear );
		}
	}

	if( 0.0f != FontEffectInfo.fGlobalBlurAlphaWeight )
	{
		SFontDrawEffectInfo GlobalBlurEffectInfo = FontEffectInfo;
		GlobalBlurEffectInfo.nDrawType = SFontDrawEffectInfo::GLOW;
		GlobalBlurEffectInfo.nWeight = 1;
		GlobalBlurEffectInfo.fAlphaWeight = GlobalBlurEffectInfo.fGlobalBlurAlphaWeight;
		SFontTextureInfo GlobalBlurTextureInfo = GetFontTexture( nFontIndex, szText, fWidth, dwFontFormat, GlobalBlurEffectInfo, bRemoveCache );

		SUICoord EffectFontCoord = ScreenCoord;
		if( s_bUseUniscribe )
		{
			EffectFontCoord.fX = ScreenCoord.fX - ((float)GlobalBlurEffectInfo.nWeight / (float)GetEtDevice()->Width() );
			EffectFontCoord.fY = ScreenCoord.fY - (( (float)GlobalBlurEffectInfo.nWeight + UPPER_PADDING_PIXEL ) / (float)GetEtDevice()->Height() );
		}
		else
		{
			EffectFontCoord.fX = ScreenCoord.fX - ((float)GlobalBlurEffectInfo.nWeight / (float)GetEtDevice()->Width() );
			EffectFontCoord.fY = ScreenCoord.fY - ((float)GlobalBlurEffectInfo.nWeight / (float)GetEtDevice()->Height() );
		}

		EffectFontCoord.fWidth  = (float)GlobalBlurTextureInfo.hTexture->Width() / GetEtDevice()->Width();
		EffectFontCoord.fHeight = (float)GlobalBlurTextureInfo.hTexture->Height() / GetEtDevice()->Height();

		CEtSprite::GetInstance().DrawSprite( (EtTexture*)GlobalBlurTextureInfo.hTexture->GetTexturePtr(), 
												GlobalBlurTextureInfo.hTexture->Width(), GlobalBlurTextureInfo.hTexture->Height(),
												UVCoord, GlobalBlurEffectInfo.dwFontColor, EffectFontCoord, 0.f, ZValue, fShear );
	}

	if( hFontTexture ) 
	{
		if( s_bUseUniscribe )
		{
			SUICoord FontCoord = ScreenCoord;
			FontCoord.fY = ScreenCoord.fY - ( UPPER_PADDING_PIXEL / (float)GetEtDevice()->Height() );
			CEtSprite::GetInstance().DrawSprite( (EtTexture*)hFontTexture->GetTexturePtr(), hFontTexture->Width(), hFontTexture->Height(),
													UVCoord, FontEffectInfo.dwFontColor, FontCoord, 0.f, ZValue, fShear );
		}
		else
		{
			CEtSprite::GetInstance().DrawSprite( (EtTexture*)hFontTexture->GetTexturePtr(), hFontTexture->Width(), hFontTexture->Height(),
													UVCoord, FontEffectInfo.dwFontColor, ScreenCoord, 0.f, ZValue, fShear );
		}
	}

	if( !bBegin ) 
		CEtSprite::GetInstance().End();

	ScreenCoord = ScreenCoordBackup;
}

void CEtFontMng::DrawTextW3D( int &nFontSetIndex, int nFontHeight, LPCWSTR szText, EtVector3 &vPosition, SFontDrawEffectInfo& FontEffectInfo, 
							 int nCount, float fScale /*= 1.0f */ )
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	if( g_bSkipScene )
	{
		return;
	}
	if( !CEtCamera::GetActiveCamera() )
	{
		return;
	}
	if( wcslen(szText) == 0 ) 
	{
		return;
	}
	ASSERT(m_vecFont.size()&&"CEtFontMng::DrawTextW3D");
	ASSERT(nFontSetIndex>=0&&"CEtFontMng::DrawTextW3D");
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	if( nFontIndex < 0 || nFontIndex >= (int)m_vecFont.size() ) return;

	EtMatrix IdentMat, TextMat;
	EtVector3 FontPos;

	EtVec3TransformCoord( &FontPos, &vPosition, CEtCamera::GetActiveCamera()->GetViewMat() );
	EtMatrixIdentity( &IdentMat );
	GetEtDevice()->SetWorldTransform( &IdentMat );
	GetEtDevice()->SetViewTransform( &IdentMat );
	GetEtDevice()->SetProjTransform( CEtCamera::GetActiveCamera()->GetProjMat() );

	ASSERT( !CEtSprite::GetInstance().IsBegin() );

	CEtSprite::GetInstance().Begin( D3DXSPRITE_OBJECTSPACE );
	EtMatrixRotationX( &TextMat, EtToRadian( 180.0f ) );
	TextMat._41 = FontPos.x;
	TextMat._42 = FontPos.y;
	TextMat._43 = FontPos.z;
	if( fScale != 1.0f )
	{
		EtMatrix ScaleMat;
		EtMatrixScaling( &ScaleMat, fScale, fScale, fScale );
		EtMatrixMultiply( &TextMat, &ScaleMat, &TextMat );
	}
	CEtSprite::GetInstance().SetTransform( TextMat );

	// If BLUR type, create font texture, use its info, and do not render.
	if( SFontDrawEffectInfo::NORMAL != FontEffectInfo.nDrawType )
	{
		SFontTextureInfo FontEffectTexture = GetFontTexture( nFontIndex, szText, 1024.0f, 0, FontEffectInfo );

		//SUICoord EffectFontCoord = ScreenCoord;
		//EffectFontCoord.fX = ScreenCoord.fX - ((float)FontEffectInfo.nWeight / (float)GetEtDevice()->Width() );
		//EffectFontCoord.fY = ScreenCoord.fY - ((float)FontEffectInfo.nWeight / (float)GetEtDevice()->Height() );
		//EffectFontCoord.fWidth  = (float)FontEffectTexture.hTexture->Width() / GetEtDevice()->Width();
		//EffectFontCoord.fHeight = (float)FontEffectTexture.hTexture->Height() / GetEtDevice()->Height();
		RECT Rect;
		SetRect( &Rect, FontEffectInfo.nWeight, FontEffectInfo.nWeight, FontEffectTexture.hTexture->Width(), FontEffectTexture.hTexture->Height() );
		//((LPDIRECT3DDEVICE9)CEtDevice::GetInstance().GetDevicePtr())->SetRenderState( D3DRS_ZENABLE, FALSE );

		CEtSprite::GetInstance().Draw( (EtTexture*)FontEffectTexture.hTexture->GetTexturePtr(), &Rect, NULL, NULL, 
																	  FontEffectInfo.dwEffectColor );
		//((LPDIRECT3DDEVICE9)CEtDevice::GetInstance().GetDevicePtr())->SetRenderState( D3DRS_ZENABLE, TRUE );
		//CEtSprite::GetInstance().DrawSprite( (EtTexture*)FontEffectTexture.hTexture->GetTexturePtr(), 
		//									 FontEffectTexture.hTexture->Width(), FontEffectTexture.hTexture->Height(),
		//									 UVCoord, FontEffectInfo.dwEffectColor, EffectFontCoord, 0.0f );
	}

	// If BLUR type, create font texture, use its info, and do not render.
	SFontDrawEffectInfo NormalEffectInfo = FontEffectInfo;
	NormalEffectInfo.nDrawType = SFontDrawEffectInfo::NORMAL;
	SFontTextureInfo TextureInfo = GetFontTexture( nFontIndex, szText, 1024.f, 0, NormalEffectInfo );
	EtTextureHandle hFontTexture = TextureInfo.hTexture;

	if( hFontTexture )
	{
		RECT Rect;
		SetRect( &Rect, 0, 0, hFontTexture->Width(), hFontTexture->Height());

		//for( int i = 0; i < 3; ++i )
		/*LPDIRECT3DDEVICE9 pd3dDevice = ((LPDIRECT3DDEVICE9)CEtDevice::GetInstance().GetDevicePtr());*/
		//GetEtDevice()->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		CEtSprite::GetInstance().Draw( (EtTexture*)hFontTexture->GetTexturePtr(), &Rect, NULL, NULL, FontEffectInfo.dwFontColor );
		//GetEtDevice()->SetSamplerState( D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	}

	CEtSprite::GetInstance().End();
}

void CEtFontMng::CalcTextRect( int &nFontSetIndex, int nFontHeight, LPCWSTR szText, DWORD dwFontFormat, SUICoord &CalcCoord, int nCount, bool bUseCache )
{
	ScopeLock<CSyncLock> Lock(s_Lock);

	ASSERT(m_vecFont.size()&&"CEtFontMng::CalcTextRect");
	ASSERT(nFontSetIndex>=0&&"CEtFontMng::CalcTextRect");

	int nHeight = 0;
	int nWidth = GetCaretPos( szText, nFontSetIndex, nFontHeight, (int)wcslen(szText), &nHeight, bUseCache );

	CalcCoord.fWidth = ( nWidth ) / ( float )GetEtDevice()->Width();
	CalcCoord.fHeight = ( nHeight) / ( float )GetEtDevice()->Height();
}

void CEtFontMng::OnLostDevice()
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	m_mapFontIndex.clear();
	DeleteAllFont();
}

void CEtFontMng::OnResetDevice()
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	int i, nFontIndex(-1);
	for( i=0; i<(int)m_vecFontSet.size(); i++ )
	{
		nFontIndex = AddFont( m_vecFontSet[i].strFullFileName.c_str(), m_vecFontSet[i].strFontName.c_str(), m_vecFontSet[i].nFontHeight, m_vecFontSet[i].nFontWeight, m_vecFontSet[i].bItalic );
		m_mapFontIndex.insert( std::make_pair( m_vecFontSet[i].nIndex, nFontIndex ) );
	}
	int nSize;
	nSize = (int)m_cachedFontList.size();
	for( i = 0; i < nSize; i++) {
		SAFE_RELEASE_SPTR( m_cachedFontList[i].TextureInfo.hTexture );
	}
	m_cachedFontList.clear();

	std::list< EtTextureHandle >::iterator it = m_texturePool.begin();
	for( std::list< EtTextureHandle >::iterator it = m_texturePool.begin(); it != m_texturePool.end(); it++ ) {
		SAFE_RELEASE_SPTR( (*it) );
	}
	m_texturePool.clear();

	m_caretInfo.clear();
}

void CEtFontMng::AddFontSet( SUIFontSet &fontSet )
{
	m_vecFontSet.push_back( fontSet );
	int nFontIndex = AddFont( fontSet.strFileName.c_str(), fontSet.strFontName.c_str(), fontSet.nFontHeight, fontSet.nFontWeight, fontSet.bItalic );
	m_mapFontIndex.insert( std::make_pair( fontSet.nIndex, nFontIndex ) );
}

void CEtFontMng::DeleteFontSet( int nIndex )
{
	if( m_vecFontSet.empty() ) return;
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecFontSet.size() ) return;

	std::map<int,int>::iterator iter;
	iter = m_mapFontIndex.find( m_vecFontSet[nIndex].nIndex );
	if( iter != m_mapFontIndex.end() )
	{
		m_mapFontIndex.erase(iter);
	}

	m_vecFontSet.erase( m_vecFontSet.begin()+nIndex );
}

void CEtFontMng::ModifyFontSet( int nIndex, SUIFontSet &fontSet )
{
	if( m_vecFontSet.empty() ) return;
	if( nIndex < 0 ) return;
	if( nIndex >= (int)m_vecFontSet.size() ) return;

	m_vecFontSet[nIndex] = fontSet;
	int nFontIndex = AddFont( fontSet.strFileName.c_str(), fontSet.strFontName.c_str(), fontSet.nFontHeight, fontSet.nFontWeight, fontSet.bItalic );
	m_mapFontIndex[m_vecFontSet[nIndex].nIndex] = nFontIndex;
}

SUIFontSet *CEtFontMng::GetFontSet( int nIndex )
{
	if( m_vecFontSet.empty() )					return NULL;
	if( nIndex < 0 )							return NULL;
	if( nIndex >= (int)m_vecFontSet.size() )	return NULL;

	return &m_vecFontSet[nIndex];
}

bool CEtFontMng::LoadFontSet( const char *szFileName )
{
	Finalize();

	CResMngStream Stream( szFileName );
	if( Stream.IsValid() )
	{
		m_strFontSetFileName = szFileName;
		m_vecFontSet.clear();

		int nCount;
		Stream >> nCount;
		if( nCount <= 0 ) 
			return false;

		m_vecFontSet.resize(nCount);

		for( int i=0; i<(int)m_vecFontSet.size(); i++ )
		{
			m_vecFontSet[i].Load( Stream );
		}

		std::string strFileName;
		std::string::size_type offSet;
		offSet = m_strFontSetFileName.find_last_of( "\\" );
		if( offSet != std::string::npos )
		{
			strFileName = m_strFontSetFileName.substr( 0, offSet+1 );
		}

		int nFontIndex(-1);
		for( int i=0; i<(int)m_vecFontSet.size(); i++ )
		{
			std::string strTemp = strFileName + m_vecFontSet[i].strFileName;

			m_vecFontSet[i].strFullFileName = strFileName + m_vecFontSet[i].strFileName;	// Added for FreeType
			nFontIndex = AddFont( strTemp.c_str(), m_vecFontSet[i].strFontName.c_str(), m_vecFontSet[i].nFontHeight, m_vecFontSet[i].nFontWeight, m_vecFontSet[i].bItalic );
			m_mapFontIndex.insert( std::make_pair( m_vecFontSet[i].nIndex, nFontIndex ) );
		}

		return true;
	}

	return false;
}

void CEtFontMng::SaveFontSet( const char *szFileName )
{
	CFileStream Stream( szFileName, CFileStream::OPEN_WRITE );
	if( Stream.IsValid() )
	{
		Stream << (int)m_vecFontSet.size();
		for( int i=0; i<(int)m_vecFontSet.size(); i++ )
		{
			m_vecFontSet[i].Save( Stream );
		}
	}
}

int CEtFontMng::GetFontIndex( int &nFontSetIndex, int nFontHeight )
{
	std::map<int,int>::iterator iter;
	iter = m_mapFontIndex.find( nFontSetIndex );
	if( iter != m_mapFontIndex.end() )
	{
		return iter->second;
	}
	else
	{
		int nRetIndex(0), nFontIndex(0);
		int nTemp, nValue(INT_MAX);

		// Note : Find the index with the closest matching size.
		//
		for( iter=m_mapFontIndex.begin(); iter != m_mapFontIndex.end(); ++iter )
		{
			nTemp = abs(m_vecFont[iter->second].nFontHeight - nFontHeight);
			if( nValue > nTemp )
			{
				nValue = nTemp;
				nFontIndex = iter->first;
				nRetIndex = iter->second;
			}
		}

		//CDebugSet::ToLogFile( "CEtFontMng::GetFontIndex, FontSetIndex:%d->%d, FontIndex:%d", nFontSetIndex, nFontIndex, nRetIndex );
		nFontSetIndex = nFontIndex;
		return nRetIndex;
	}
}

int CEtFontMng::GetFontSetIndex()
{
	std::map<int,int>::iterator iter;
	for( int i=0; true; i++ )
	{
		iter = m_mapFontIndex.find( i );
		if( iter == m_mapFontIndex.end() )
		{
			return i;
		}
	}
}


// Use Uniscribe Function.

bool CEtFontMng::IsSymbol( wchar_t cText )
{
	bool bIsSymbol = false;
	if( cText == L'.' ||
		cText == L',' ||
		cText == L'*' ||
		cText == L'%' ||
		cText == L'/' ||
		cText == L'(' ||
		cText == L')' ||
		cText == L'[' ||
		cText == L']' ||
		cText == L'<' ||
		cText == L'>' ||
		cText == L':' ||
		cText == L'!' ||
		cText == L'?' ||
		cText == L'\"' ||
		cText == L'\'' )	// I'll or target's
	{
		bIsSymbol = true;
	}

	return bIsSymbol;
}

bool CEtFontMng::IsOpenBraket( wchar_t cText )
{
	bool bIsOpenBraket = false;
	if( cText == L'(' ||
		cText == L'[' ||
		cText == L'<' )
	{
		bIsOpenBraket = true;
	}

	return bIsOpenBraket;
}

void CEtFontMng::GetWordBreakInfo( wstring& strText, vector<SCRIPT_LOGATTR>& vecLogAttr )
{
	HRESULT hResult;
	int nStrLength = static_cast<int>( strText.length() );
	if( nStrLength <= 0 )
		return;

	const SCRIPT_CONTROL *psControl = NULL;
	const SCRIPT_STATE *psState = NULL;
	int cMaxItems = nStrLength + 1;
	SCRIPT_ITEM* psItems = new SCRIPT_ITEM[cMaxItems];
	int nItemCount;

	hResult = ScriptItemize( strText.c_str(), nStrLength, cMaxItems, psControl, psState, psItems, &nItemCount );
	if( SUCCEEDED( hResult ) == FALSE )
	{
		SAFE_DELETEA( psItems );
		return;
	}

	SCRIPT_LOGATTR* psLogAttr = NULL;

	bool bIsPrevSymbol = false;
	for( int i=0; i<nItemCount; i++ )
	{
		int nIndex = psItems[i].iCharPos;
		int nChars = psItems[i+1].iCharPos - nIndex;

		psLogAttr = new SCRIPT_LOGATTR[nChars];
		ScriptBreak( strText.c_str()+nIndex, nChars, &psItems[i].a, psLogAttr );
	
		for( int k=0; k<nChars; k++ )
		{
			if( i > 0 && k == 0 )
				psLogAttr[k].fSoftBreak = 1;

			if( nChars == 1 )
			{
				if( IsSymbol( strText.at( nIndex ) ) )
				{
					psLogAttr[k].fSoftBreak = 0;
					bIsPrevSymbol = true;
					vecLogAttr.push_back( psLogAttr[k] );
					continue;
				}
				else if( strText.at( nIndex ) == L'-' )
				{
					if( nIndex > 0 && nIndex + 1 < static_cast<int>( strText.length() ) &&
						strText.at( nIndex - 1 ) != L' ' && strText.at( nIndex + 1 ) != L' ' )
						psLogAttr[k].fSoftBreak = 0;
				}
			}

			if( bIsPrevSymbol )
			{
				if( psLogAttr[k].fSoftBreak == 1 && nIndex > 0 )
				{
					if( IsSymbol( strText.at( nIndex - 1 ) ) )
					{
						psLogAttr[k].fSoftBreak = 0;
					}
				}

				bIsPrevSymbol = false;
			}

			if( IsSymbol( strText.at( nIndex ) ) )
			{
				psLogAttr[k].fSoftBreak = 0;
			}
			
			vecLogAttr.push_back( psLogAttr[k] );
		}

		SAFE_DELETEA( psLogAttr );
	}

	SAFE_DELETEA( psItems );

	// Insert softbreak for the first word of a sentence
	for( int i=0; i<static_cast<int>( vecLogAttr.size() ); i++ )
	{
		if( strText.at(i) != ' ' )
		{
			vecLogAttr[i].fSoftBreak = 1;
			break;
		}
	}

	// Insert a softbreak before the opening parenthesis for parenthesized sentences
	if( vecLogAttr.size() == strText.length() )
	{
		for( int i=0; i<strText.length(); i++ )
		{
			if( IsOpenBraket( strText.at( i ) ) )
			{
				vecLogAttr[i].fSoftBreak = 1;
				if( i+1 < static_cast<int>( vecLogAttr.size() ) && vecLogAttr[i+1].fSoftBreak == 1 )
				{
					vecLogAttr[i+1].fSoftBreak = 0;
				}
			}
		}
	}
}

bool CEtFontMng::GetWordBreakText( wstring& strText, int nFontSetIndex, int nFontHeight, float fWidth, 
									vector<wstring>& vecStrLine, int& nMaxWidth, bool bUseRemainWidth, float fOriginWidth )
{
	int nFontIndex = GetFontIndex( nFontSetIndex, nFontHeight );
	if( !GetEtDevice() || strText.length() == 0 || fWidth <= 0.0f )
		return false;

	if( nFontIndex < 0 || nFontIndex >= static_cast<int>( m_vecFont.size() ) )
		return false;

	if( bUseRemainWidth )
		m_fOriginWidth = fOriginWidth;

	SCaretKey CaretKey;
	CaretKey.szText = strText;
	CaretKey.nFontIndex = nFontIndex;

	FT_Face Face = m_vecFont[ nFontIndex ].Face;
	bool bBold = m_vecFont[ nFontIndex ].nFontWeight*100 >= FW_BOLD;

	SFontInfo& FontInfo = m_vecFont.at( nFontIndex );

	vector<int> vecWidthList;
	int nNumLine = WordBreak( strText, CaretKey, true, Face, bBold, FontInfo, fWidth, vecStrLine, vecWidthList, bUseRemainWidth );

	nMaxWidth = 0;
	for( int nLine=0; nLine<nNumLine; ++nLine )
	{
		int nWidth = vecWidthList.at( nLine );
		if( nMaxWidth < nWidth )
			nMaxWidth = nWidth;
	}
	vecWidthList.clear();

	if( nNumLine == -1 )
		return false;

	return true;
}

void CEtFontMng::SetOriginWidth( float& fWidth )
{
	fWidth = m_fOriginWidth;
}

int CEtFontMng::WordBreak( wstring& strText, SCaretKey& CaretKey, bool bAlreadyExistCaret, FT_Face& Face, bool bBold, SFontInfo& FontInfo, 
						  float fWidth, vector<wstring>& vecStrLine, vector<int>& vecWidthList, bool bUseRemainWidth )
{
	vector<wstring> vlWords;
	vector<int> vlWordWidths;
	vector<SCRIPT_LOGATTR> vecLogAttr;

	if( s_bUseWordBreak )
	{
		GetWordBreakInfo( strText, vecLogAttr );
		if( vecLogAttr.size() != strText.length() )
			return -1;

		int nWordStartPos = 0;
		for( int i=0; i<static_cast<int>(vecLogAttr.size()); i++ )
		{
			if( vecLogAttr[i].fSoftBreak == 1 )
			{
				std::wstring strWord = strText.substr( nWordStartPos, i - nWordStartPos );
				strWord = boost::algorithm::trim_copy( strWord );

				if( static_cast<int>( strWord.length() ) > 0 )
					vlWords.push_back( strWord );

				nWordStartPos = i;
			}

			if( i == static_cast<int>(vecLogAttr.size()) - 1 )	// Word Break last character
			{
				std::wstring strWord = strText.substr( nWordStartPos, i - nWordStartPos + 1 );
				strWord = boost::algorithm::trim_copy( strWord );
				if( static_cast<int>( strWord.length() ) > 0 )
					vlWords.push_back( strWord );
			}
		}

		for( int i=0; i<static_cast<int>(vlWords.size()); ++i )
		{
			wstring& strWord = vlWords.at( i );
			int iWordWidth = 0;
			for( int k=0; k<static_cast<int>(strWord.size()); ++k )
			{
				wchar_t chWord = strWord.at( k );
				iWordWidth += GetCachedCharAdvanceX( Face, FontInfo, chWord, bBold, 100 );	// Fix iNowAdvance to a large value (only the actual Advance value is needed here)
			}
			vlWordWidths.push_back( iWordWidth );
		}
	}

	int nSentenceWidth = 0;
	int nTextLength = (int)strText.length();
	wstring strLineText;
	int iNowWordIndex = -1;

	for( int i=0; i<nTextLength; ++i )
	{
		int nWidth = static_cast<int>( fWidth );
		if( fWidth - static_cast<float>( nWidth ) > 0.0f )
			nWidth++;	// Round up because float->int conversion truncates

		FT_ULong ch = strText.at( i );
		if( i < nTextLength-2 )
		{
			if( ch == '\r' && (strText.at(i+1) == '\n') || ch == '\n' )
			{
				nSentenceWidth = 0;
				strLineText += L"\n";

				if( ch == '\r' && (strText.at(i+1) == '\n') )
					++i;

				continue;
			}
		}

		int nAdvanceX = GetCachedCharAdvanceX( Face, FontInfo, (wchar_t)ch, bBold, 100 );	// Fix iNowAdvance to a large value (only the actual Advance value is needed here)
		if( nAdvanceX < 0 )
			continue;

		bool bNowBlank = strText.at( i ) == ' ';
		bool bOverFlowSentence = nSentenceWidth + nAdvanceX > nWidth;

		if( s_bUseWordBreak && ( vecLogAttr[i].fSoftBreak && !vecLogAttr[i].fWhiteSpace ) )
		{
			iNowWordIndex++;
			// If a next word exists, get its length to check whether it exceeds the specified width.
			if( iNowWordIndex < static_cast<int>( vlWordWidths.size() ) )
			{
				int iWordWidth = vlWordWidths.at( iNowWordIndex );
				int iNextWordIncludeSentenceWidth = nSentenceWidth + iWordWidth;
				if( iNextWordIncludeSentenceWidth > (int)(fWidth + 0.005f) )		// Add a correction value because floating-point error could cause misjudgment even when the exact width is passed from outside.
					bOverFlowSentence = true;
			}
		}

		// Insert a new line if it exceeds the width.
		if( bOverFlowSentence )
		{
			// Remove current character if it is whitespace.
			if( bNowBlank )
			{
				strLineText += L"\n";
				nSentenceWidth = 0;
			}
			else
			{
				strLineText += L"\n";
				strLineText += strText.at( i );
				nSentenceWidth = nAdvanceX;
			}

			if( bUseRemainWidth )
				SetOriginWidth( fWidth );

			continue;
		}

		strLineText += strText.at( i );
		nSentenceWidth += nAdvanceX;

		if( !bAlreadyExistCaret ) 
		{
			m_caretInfo[ CaretKey ].vecCaretPos.push_back( nSentenceWidth );
			m_caretInfo[ CaretKey ].mapPosByCaret.insert( make_pair(nSentenceWidth, i+1) );
		}
	}

	nTextLength = (int)strLineText.length();
	int nLastNewLine = 0;

	for( int i=0; i<nTextLength; ++i )
	{
		FT_ULong ch = strLineText.at( i );
		// There are cases with two line break codes and cases with a single \n.
		// Use -2 so a trailing space in the last character is not converted to \n
		if( ch == '\r' )
		{
			if( i+1 < nTextLength - 1 && strText.at( i+1 ) == '\n' )
			{
				std::wstring strWord = strLineText.substr( nLastNewLine, i-nLastNewLine );
				// Handle line break.
				// Since \r and \n characters also need to be removed, subtract 1 from count and add 2 to LastNewLine
				vecStrLine.push_back( strWord );
				nLastNewLine = i + 2;
			}
		}
		else if( ch == '\n' )
		{
			std::wstring strWord = strLineText.substr( nLastNewLine, i-nLastNewLine );
			vecStrLine.push_back( strWord );
			nLastNewLine = i + 1;
		}
	}

	vlWords.clear();
	vlWordWidths.clear();
	vecLogAttr.clear();

	if( vecStrLine.empty() )
	{
		vecStrLine.push_back( strText );
	}
	else	// Push the last sentence into the line vector as well.
	{
		if( (int)strLineText.length() > nLastNewLine )
		{
			std::wstring strWord = strLineText.substr( nLastNewLine, (int)strLineText.length() - nLastNewLine );
			vecStrLine.push_back( strWord );
		}
	}

	int nNumLine = (int)vecStrLine.size();
	// If newline-processed, remove from caret cache. The caret cache stores the currently rendered string
	// Keep only when it can all be rendered within the given width. Because once line wrapping occurs, the caret position 
	// Meaningless, and for the same string and font set that will be rendered with different width later, CalcTextRect -> GetCaretPos 
	// because the width of the cached line-wrapped string rendered earlier is returned. Re-layout string with new width
	// When rendering is needed, recalculating width is right. To exclude such cases, keep string on one line
	// Remove from cache if unable to render.
	if( 1 < nNumLine )
		m_caretInfo.erase( CaretKey );

	for( int i=0; i<static_cast<int>(vecStrLine.size()); ++i )
	{
		wstring& strWord = vecStrLine.at( i );
		int iWordWidth = 0;
		for( int k=0; k<static_cast<int>(strWord.size()); ++k )
		{
			wchar_t chWord = strWord.at( k );
			iWordWidth += GetCachedCharAdvanceX( Face, FontInfo, chWord, bBold, 100 );
		}
		vecWidthList.push_back( iWordWidth );
	}

	return nNumLine;
}