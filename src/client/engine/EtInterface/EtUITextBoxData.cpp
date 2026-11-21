#include "StdAfx.h"
#include "EtUITextBoxData.h"
#include "EtTexture.h"
#include "EternityEngine.h"
#include "EtResourceMng.h"

// Note : Class Word
//
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CWord::CWord()
{
}

CWord::~CWord()
{
}

void CWord::UpdatePos( float fX, float fY )
{
	m_sProperty.uiCoord.fX = fX;
	m_sProperty.uiCoord.fY = fY;
}

void CWord::Clear()
{
	m_sProperty.Clear();
	m_strWord.clear();
	m_strWordWithTag.clear();
}

/////////////////////////////////////////////////////////////////


// Note : Class Line
//
CLine::CLine()
	: m_fLineSpace(0.0f)
	, m_bSelected(false)
	, m_bMouseInLine(false)
	, m_uiCoord( 0.f, 0.f, 0.f, 0.f )
{
}

CLine::~CLine()
{
	SAFE_DELETE_VEC( m_vecWord );
}

void CLine::SetLineSpace( float fLineSpace )
{
	m_uiCoord.fHeight -= GetLineSpace();
	this->m_fLineSpace = fLineSpace;
	m_uiCoord.fHeight += GetLineSpace();
}

void CLine::UpdatePos( float fX, float fY )
{
	m_uiCoord.fX = fX;
	m_uiCoord.fY = fY;

	fY += GetLineSpace();

	for( int i=0; i<(int)m_vecWord.size(); i++ )
	{
		if( m_vecWord[i].m_sProperty.dwFormat & UITEXT_RIGHT )
		{
			// Bug found.
			// When determining Word Pos, instead of calculating fX relative to the line's top-left,
			// Must calculate fX relative to the top-left of the parent dialog.
			// (This applies when determining fX for all controls.)
			// Because it ultimately renders using the parent dialog's DrawDlgText.
			//
			// Therefore, when determining a Word's fX, it must be offset by the line's fX.
			//
			// When right-aligned, decide regardless of Append (add to current line) or Add (add to next line).
			m_vecWord[i].UpdatePos( m_uiCoord.fX + m_uiCoord.fWidth - m_vecWord[i].m_sProperty.uiCoord.fWidth, fY );
			// Note : Right alignment refers to the last Word.
			return;
		}
		else if( m_vecWord[i].m_sProperty.dwFormat & UITEXT_CENTER )
		{
			// When CENTER is attached to Append, should it be centered on the line or in the remaining space?
			// If placed at the center of the line, accepting CENTER itself in Append would be an error, so
			// Should center it in the remaining space.
			float fEmptyWidth = m_uiCoord.fWidth - (fX - m_uiCoord.fX);
			float fWordWidth = m_vecWord[i].m_sProperty.uiCoord.fWidth;

			// Unlike RIGHT, must add to fX to remember the width when something is written on the left.
			m_vecWord[i].UpdatePos( fX + (fEmptyWidth - fWordWidth)*0.5f, fY );
		}
		else
		{
			m_vecWord[i].UpdatePos( fX, fY );
		}

		fX = m_vecWord[i].m_sProperty.uiCoord.Right();
	}
}

std::wstring CLine::GetText()
{
	std::wstring strTemp;

	for( int i=0; i<(int)m_vecWord.size(); i++ )
	{
		strTemp += m_vecWord[i].m_strWord;
	}

	return strTemp;
}

CImageLine::CImageLine()
{
	m_hTexture;
	std::string m_szFileName;
}

CImageLine::~CImageLine()
{
	SAFE_RELEASE_SPTR( m_hTexture );
}

void CImageLine::SetImage( WCHAR *wszFileName, SUICoord &Coord )
{
	ToMultiString( wszFileName, m_szFileName );
	SetImage( (char*)m_szFileName.c_str(), Coord );
}

void CImageLine::SetImage( char *szFileName, SUICoord &Coord )
{
	m_szFileName = szFileName;
	m_hTexture = EternityEngine::LoadTexture( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str() );
	m_uiUV = Coord;
}

EtTextureHandle CImageLine::GetTexture()
{
	return m_hTexture;
}
