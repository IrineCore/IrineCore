#include "StdAfx.h"
#include "EtUITemplate.h"
#include "EtUITemplateOld.h"
#include "EtFontMng.h"
#include "DebugSet.h"
#include "EtLoader.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

SUIColor::SUIColor()
{
	Clear();
}

void SUIColor::Blend( int nState, float fElapsedTime, float fRate )
{
	if( HIBYTE(HIWORD(dwCurrentColor)) < (BYTE)(255*0.05f) )
	{
		dwCurrentColor = dwColor[nState];
	}
	else if( dwCurrentColor != dwColor[ nState ] ) {
		EtColor CurrentColor;
		EtColorLerp( &CurrentColor, &EtColor( dwCurrentColor ), &EtColor( dwColor[ nState ] ), 1.0f - powf( fRate, 30 * fElapsedTime ) );
		dwCurrentColor = CurrentColor;
	}
}

void SUIColor::Clear()
{
	dwCurrentColor = 0xffffffff;

	dwColor[UI_STATE_NORMAL]	= 0xffffffff;
	dwColor[UI_STATE_DISABLED]	= 0xc87f7f7f;
	dwColor[UI_STATE_HIDDEN]	= 0x00000000;
	dwColor[UI_STATE_FOCUS]		= 0xffffffff;
	dwColor[UI_STATE_MOUSEENTER]= 0xffffffff;
	dwColor[UI_STATE_PRESSED]	= 0xffffffff;
}

SUIColorEx::SUIColorEx()
	: m_fConst(1.0f)
	, m_fBlendTime(0.0f)
	, m_SrcColor((DWORD)0xffffffff)
	, m_nCurState(UI_STATE_NORMAL)
{
}

void SUIColorEx::SetState( UI_CONTROL_STATE uiControlState )
{
	m_nCurState = uiControlState;
	dwCurrentColor = dwColor[m_nCurState];
}

void SUIColorEx::BlendEx( int nState, float fElapsedTime, float fBlendTime )
{
	if( m_nCurState != nState )
	{
		m_nCurState = nState;
		m_fConst = m_fBlendTime = fBlendTime;
		m_SrcColor = dwCurrentColor;
	}

	if( m_fBlendTime > 0.0f )
	{
		EtColor CurColor;
		EtColorLerp( &CurColor, &m_SrcColor, &EtColor( dwColor[m_nCurState] ), 1.0f-(1/powf(m_fConst,2)*powf(m_fBlendTime,2) ) );
		dwCurrentColor = CurColor;
		m_fBlendTime -= fElapsedTime;
	}
	else
	{
		dwCurrentColor = dwColor[m_nCurState];
	}
}

bool SUIElement::Load_01( CStream &stream )
{
	SUIColor tempColor;
	stream >> tempColor;
	TextureColor.Assign( tempColor );
	stream >> tempColor;
	FontColor.Assign( tempColor );
	stream >> UVCoord;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream.SeekCur<int>();
	stream.SeekCur<int>();
	stream >> bShadowFont;
	stream.SeekCur<bool>();

	return true;
}

bool SUIElement::Load_02( CStream &stream )
{
	SUIColor tempColor;
	stream >> tempColor;
	TextureColor.Assign( tempColor );
	stream >> tempColor;
	FontColor.Assign( tempColor );
	stream >> UVCoord;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> bShadowFont;

	return true;
}

bool SUIElement::Load_03( CStream &stream )
{
	SUIColor tempColor;
	stream >> tempColor;
	TextureColor.Assign( tempColor );
	stream >> tempColor;
	FontColor.Assign( tempColor );
	stream >> UVCoord;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;

	return true;
}

bool SUIElement::Load_04( CStream &stream )
{
	SUIColor tempColor;
	stream >> tempColor;
	TextureColor.Assign( tempColor );
	stream >> tempColor;
	FontColor.Assign( tempColor );
	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;

	return true;
}

bool SUIElement::Load_05( CStream &stream )
{
	SUIColor tempColor;
	stream >> tempColor;
	TextureColor.Assign( tempColor );
	stream >> tempColor;
	FontColor.Assign( tempColor );
	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;

	return true;
}

bool SUIElement::Load_06( CStream &stream )
{
	TextureColor.Load(stream);
	FontColor.Load(stream);

	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;

	return true;
}

bool SUIElement::Load_07( CStream &stream )
{
	TextureColor.Load(stream);
	FontColor.Load(stream);
	ShadowFontColor.Load(stream);

	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;

	return true;
}


bool SUIElement::Load_08( CStream& stream )
{
	TextureColor.Load(stream);
	FontColor.Load(stream);
	ShadowFontColor.Load(stream);

	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;
	stream >> fFontHoriOffset;
	stream >> fFontVertOffset;

	if( bShadowFont )
		nDrawType = SFontDrawEffectInfo::SHADOW;

	return true;
}

bool SUIElement::Load_09( CStream &stream )
{
	TextureColor.Load(stream);
	FontColor.Load(stream);
	ShadowFontColor.Load(stream);

	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;
	stream >> fFontHoriOffset;
	stream >> fFontVertOffset;
	stream >> nDrawType;
	stream >> nWeight;
	stream >> fAlphaWeight;

	return true;
}

bool SUIElement::Load_10( CStream &stream )
{
	TextureColor.Load(stream);
	FontColor.Load(stream);
	ShadowFontColor.Load(stream);

	stream >> UVCoord;
	stream >> fTextureWidth;
	stream >> fTextureHeight;
	stream >> fDelayTime;
	stream >> dwFontFormat;
	stream >> nFontIndex;
	stream >> nFontHeight;
	stream >> bShadowFont;
	stream >> fFontHoriOffset;
	stream >> fFontVertOffset;
	stream >> nDrawType;
	stream >> nWeight;
	stream >> fAlphaWeight;
	stream >> fGlobalBlurAlphaWeight;

	return true;
}

bool SUIElement::Load( CStream &stream )
{
	DWORD dwVersion;
	stream >> dwVersion;

	switch( dwVersion )
	{
		case SUIElement_01::VERSION: Load_01(stream); break;
		case SUIElement_02::VERSION: Load_02(stream); break;
		case SUIElement_03::VERSION: Load_03(stream); break;
		case SUIElement_04::VERSION: Load_04(stream); break;
		case SUIElement_05::VERSION: Load_05(stream); break;
		case SUIElement_06::VERSION: Load_06(stream); break;
		case SUIElement_07::VERSION: Load_07(stream); break;
		case SUIElement_08::VERSION: Load_08(stream); break;
		case SUIElement_09::VERSION: Load_09(stream); break;
		case SUIElement_10::VERSION: Load_10(stream); break;
		case VERSION:
			{
				TextureColor.Load(stream);
				FontColor.Load(stream);
				ShadowFontColor.Load(stream);

				stream >> UVCoord;
				stream >> fTextureWidth;
				stream >> fTextureHeight;
				stream >> fDelayTime;
				stream >> dwFontFormat;
				stream >> nFontIndex;
				stream >> nFontHeight;
				stream >> bShadowFont;
				stream >> fFontHoriOffset;
				stream >> fFontVertOffset;
				stream >> nDrawType;
				stream >> nWeight;
				stream >> fAlphaWeight;
				stream >> fGlobalBlurAlphaWeight;
				stream >> TemplateUVCoord;
			}
			break;

		default: 
			CDebugSet::ToLogFile( "SUIElement::Load, default case(%d)!", dwVersion );
			return false;
	}

	return true;
}

bool SUIElement::Save( CStream &stream )
{
	stream << VERSION;

	TextureColor.Save(stream);
	FontColor.Save(stream);
	ShadowFontColor.Save(stream);

	stream << UVCoord;
	stream << fTextureWidth;
	stream << fTextureHeight;
	stream << fDelayTime;
	stream << dwFontFormat;
	stream << nFontIndex;
	stream << nFontHeight;
	stream << bShadowFont;
	stream << fFontHoriOffset;
	stream << fFontVertOffset;
	stream << nDrawType;
	stream << nWeight;
	stream << fAlphaWeight;
	stream << fGlobalBlurAlphaWeight;
	stream << TemplateUVCoord;

	return true;
}

CEtUITemplate::CEtUITemplate(void)
: m_dwVersion( 0 )
, m_bExistTemplateTexture( false )
{
}

CEtUITemplate::~CEtUITemplate(void)
{
	SAFE_RELEASE_SPTR( m_hTemplateTexture );
}

bool CEtUITemplate::Save( CStream &Stream )
{
	Stream << VERSION;

	Stream << m_szTemplateName;
	Stream << (int)m_vecElement.size();

	for( int i = 0; i < (int)m_vecElement.size(); i++ )
	{
		m_vecElement[i].Save(Stream);
	}
	m_bExistTemplateTexture = ( m_hTemplateTexture ) ? true : false;
	Stream << m_bExistTemplateTexture;

	return true;
}

bool CEtUITemplate::Load( CStream &Stream )
{
	DWORD dwVersion;
	Stream >> dwVersion;
	m_dwVersion = dwVersion;

	switch( dwVersion )
	{
	case VERSION_OLD:
	case VERSION:
		{
			int nSize;

			Stream >> m_szTemplateName;
			Stream >> nSize;

			m_vecElement.resize( nSize );
			for( int i = 0; i < nSize; i++ )
			{
				m_vecElement[i].Load(Stream);
			}
		}
		break;
	default:
		CDebugSet::ToLogFile( "CEtUITemplate::Load, default case(%d)!", dwVersion );
		break;
	}

	if( m_dwVersion > VERSION_OLD )
		Stream >> m_bExistTemplateTexture;

	return true;
}

void CEtUITemplate::OnLoaded()
{
	// Loads all templates when the tool starts, and
	// Also, loading all template textures at this time takes too long, and
	// No real need to hold onto it, so define an OnLoaded function and call it separately.
	if( m_dwVersion > VERSION_OLD && m_bExistTemplateTexture )
		LoadTemplateTexture();
}

void CEtUITemplate::LoadTemplateTexture()
{
	SAFE_RELEASE_SPTR( m_hTemplateTexture );
	char szFileName[_MAX_PATH] = {0,};
	sprintf_s( szFileName, _countof(szFileName), "UIT_%s.dds", m_szTemplateName.c_str() );
	m_hTemplateTexture = LoadResource( szFileName, RT_TEXTURE );
}

CEtUITemplate& CEtUITemplate::operator = ( const CEtUITemplate &rhs )
{
	if(this==&rhs)
		return *this;

	m_dwVersion = rhs.m_dwVersion;
	m_szTemplateName = rhs.m_szTemplateName;
	m_vecElement.clear();
	m_vecElement = rhs.m_vecElement;
	m_bExistTemplateTexture = rhs.m_bExistTemplateTexture;
	SAFE_RELEASE_SPTR( m_hTemplateTexture );
	m_hTemplateTexture = rhs.m_hTemplateTexture;
	if( m_hTemplateTexture ) m_hTemplateTexture->AddRef();
	else if( !m_szTemplateName.empty() ) OnLoaded();

	return *this;
} 