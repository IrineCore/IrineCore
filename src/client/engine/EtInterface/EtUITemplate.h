#pragma once

#include "EtUIType.h"

struct SUIColor
{
public:
	DWORD dwColor[ UI_STATE_COUNT ];
	DWORD dwCurrentColor;

public:
	SUIColor();
	void Blend( int nState, float fElapsedTime, float fRate = 0.7f );
	void Clear();
};

struct SUIColorEx : public SUIColor
{
	enum { VERSION = 0x101 };

public:
	float m_fConst;
	float m_fBlendTime;
	EtColor m_SrcColor;
	int m_nCurState;

public:
	SUIColorEx();

	void SetState( UI_CONTROL_STATE uiControlState );
	int GetCurrentState() { return m_nCurState; }
	void BlendEx( int nState, float fElapsedTime, float fBlendTime );

public:
	void Load( CStream &stream )
	{
		DWORD dwVersion;
		stream >> dwVersion;

		switch( dwVersion )
		{
		case VERSION:
			{
				stream >> dwCurrentColor;
				stream.ReadBuffer( dwColor, sizeof(DWORD)*UI_STATE_COUNT );
			}
			break;
		default:
			ASSERT(0&&"SUIColorEx::Load!");
			break;
		}
	}

	void Save( CStream &stream )
	{
		stream << VERSION;
		stream << dwCurrentColor;
		stream.WriteBuffer( dwColor, sizeof(DWORD)*UI_STATE_COUNT );
	}

	void Assign( SUIColor &uiColor )
	{
		dwCurrentColor = uiColor.dwCurrentColor;
		memcpy( dwColor, uiColor.dwColor, sizeof(DWORD)*UI_STATE_COUNT );
	}
};

struct SUIElement
{
	enum { VERSION = 0x10b };

	//enum
	//{
	//	NORMAL = 0,
	//	SHADOW,
	//	STROKE,
	//	GLOW,
	//	COUNT,
	//};

	SUIColorEx TextureColor;
	SUIColorEx FontColor;
	SUIColorEx ShadowFontColor;
	SUICoord UVCoord;
	float fTextureWidth;	// Uses this parameter to split regions when rendering controls so they don't look stretched.
	float fTextureHeight;	// (For scrollbars, it won't render properly if missing. For Static, it won't render with region splitting if missing.)
							// Originally set manually, changed to auto-set on texture load since that seemed better.
	float fDelayTime;
	DWORD dwFontFormat;
	int nFontIndex;
	int nFontHeight;
	bool bShadowFont;		// Unused since version 0x109. Replaced by DrawType.
	float fFontHoriOffset;
	float fFontVertOffset;
	int nDrawType;
	int nWeight;
	float fAlphaWeight;
	float fGlobalBlurAlphaWeight;
	SUICoord TemplateUVCoord;	// Having a separate template texture added one new UVCoord.

	SUIElement()
	{
		fTextureWidth = 0.0f;
		fTextureHeight = 0.0f;
		dwFontFormat = DT_CENTER | DT_VCENTER;
		nFontIndex = 0;
		nFontHeight = 0;
		bShadowFont = false;
		fDelayTime = 0.0f;
		fFontHoriOffset = 0.0f;
		fFontVertOffset = 0.0f;
		nDrawType = 0;		/*SFontDrawEffectInfo::NORMAL*/
		nWeight = 1;
		fAlphaWeight = 1.5f;
		fGlobalBlurAlphaWeight = 0.0f;

		ShadowFontColor.dwColor[UI_STATE_NORMAL]	= 0xff000000;
		ShadowFontColor.dwColor[UI_STATE_DISABLED]	= 0xff000000;
		ShadowFontColor.dwColor[UI_STATE_HIDDEN]	= 0xff000000;
		ShadowFontColor.dwColor[UI_STATE_FOCUS]		= 0xff000000;
		ShadowFontColor.dwColor[UI_STATE_MOUSEENTER]= 0xff000000;
		ShadowFontColor.dwColor[UI_STATE_PRESSED]	= 0xff000000;
	}

public:
	bool Load_01( CStream &stream );
	bool Load_02( CStream &stream );
	bool Load_03( CStream &stream );
	bool Load_04( CStream &stream );
	bool Load_05( CStream &stream );
	bool Load_06( CStream &stream );
	bool Load_07( CStream &stream );
	bool Load_08( CStream &stream );
	bool Load_09( CStream &stream );
	bool Load_10( CStream &stream );

	bool Load( CStream &stream );
	bool Save( CStream &stream );
};

class CEtUITemplate
{
	enum 
	{ 
		VERSION_OLD = 0x101,
		VERSION = 0x102,
	};

public:
	CEtUITemplate();
	~CEtUITemplate();

public:
	std::vector< SUIElement > m_vecElement;
	std::string m_szTemplateName;
	EtTextureHandle m_hTemplateTexture;

public:
	bool Save( CStream &Stream );
	bool Load( CStream &Stream );
	void OnLoaded();

protected:
	DWORD m_dwVersion;
	bool m_bExistTemplateTexture;
	void LoadTemplateTexture();

public:
	CEtUITemplate& operator = ( const CEtUITemplate &rhs );
};