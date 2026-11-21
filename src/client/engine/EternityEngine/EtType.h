#pragma once

#define CALC_POSITION_X		0x00000001
#define CALC_POSITION_Y		0x00000002
#define CALC_POSITION_Z		0x00000004

enum ResourceType
{
	RT_NONE = 0,
	RT_ANI = 1,
	RT_SKIN = 2,
	RT_MESH = 3,
	RT_SHADER = 4,
	RT_TEXTURE = 5,
	RT_PARTICLE = 6,
	RT_EFFECT = 7,
};

enum ShadowQuality
{
	SQ_HIGH,
	SQ_NORMAL,
	SQ_NONE,
};

enum WaterQuality
{
	WQ_HIGH,
	WQ_NORMAL,
	WQ_LOW,
};

enum DOFQuality
{
	DQ_HIGH,
	DQ_LOW,
};

enum ScreenFilter
{
	SF_DEFAULT = 0,	
	SF_DOF = 1,
	SF_MOTIONBLUR = 2,
	SF_OUTLINE = 3,
	SF_WATERCOLOR = 4,
	SF_COLORADJUST = 5,
	SF_COLORADJUSTTEX = 6,
	SF_BLOOM = 7,
	SF_LIGHTSHAFT = 8,
	SF_CAMERABLUR = 9,
	SF_RADIALBLUR = 10,
	SF_HEATHAZE = 11,
	SF_TRANSITION = 12,
};

enum LightType
{
	LT_DIRECTIONAL,
	LT_POINT,
	LT_SPOT,
};
struct SLightInfo
{
	SLightInfo()
	{
		Type = LT_DIRECTIONAL;
		Diffuse = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );
		Specular = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );
		Position = EtVector3( 0.0f, 0.0f, 0.0f );
		Direction = EtVector3( 0.0f, -1.0f, 0.0f );
		fRange = 500.0f;
		fFalloff = 1.0f;
		fTheta = 0.96592581f;
		fPhi = 0.86602539f;
		bCastShadow = false;
	}

	LightType Type;
	EtColor Diffuse;
	EtColor Specular;
	EtVector3 Position;
	EtVector3 Direction;
	float fRange;
	float fFalloff;
	float fTheta;
	float fPhi;
	bool bCastShadow;
};

enum RenderType
{
	RT_SCREEN,
	RT_TARGET,
};

enum CameraType
{
	CT_PERSPECTIVE,
	CT_ORTHOGONAL,
};
enum CameraTarget
{
	CT_SCREEN,
	CT_RENDERTARGET,
	CT_RENDERTARGET_NO_GENERATE_BACKBUFFER,
};
struct SCameraInfo
{
	SCameraInfo()
	{
		Type = CT_PERSPECTIVE;
		Target = CT_SCREEN;
		fFOV = ET_PI / 4;
		fNear = 10.0f;
		fFar = 10000.0f;
		fFogNear = 8000.0f;
		fFogFar = 10000.0f;
		FogColor = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );
		fStartX = 0.0f;
		fStartY = 0.0f;
		fWidth = 1.0f;	// When camera target is CT_RENDERTARGET, fWidth and fHeight are not relative to screen size
		fHeight = 1.0f;	// Recognize and use as actual size.
		fViewWidth = 100.0f;	// Default 1 meter horizontal and vertical camera FOV when CT_ORTHOGONAL
		fViewHeight = 100.0f;
		fWaterFarRatio = 0.5f;
	}

	CameraType Type;
	CameraTarget Target;
	float fFOV;
	float fNear;
	float fFar;
	float fViewWidth;		// Camera horizontal and vertical FOV when CT_ORTHOGONAL
	float fViewHeight;
	float fFogNear;
	float fFogFar;
	EtColor FogColor;
	float fStartX;
	float fStartY;
	float fWidth;
	float fHeight;
	float fWaterFarRatio;
};

enum TerrainType
{
	TT_NORMAL,
	TT_CLIFF,
	TT_DETAILNORMAL,
	TT_DETAILCLIFF,
};

struct STerrainInfo
{
	STerrainInfo()
	{
		Type = TT_NORMAL;
		TerrainOffset = EtVector3( 0.0f, 0.0f, 0.0f );
		nSizeX = 128;
		nSizeY = 128;
		fTileSize = 200.0f;
		fHeightMultiply = 1.0f;
		fTextureDistance = 2000.0f;
		pColor = NULL;
		pHeight = NULL;
		pLayerDensity = NULL;

		pGrassBuffer = NULL;
		fGrassWidth[ 0 ] = 60.0f;
		fGrassWidth[ 1 ] = 60.0f;
		fGrassWidth[ 2 ] = 60.0f;
		fGrassWidth[ 3 ] = 60.0f;
		fMaxGrassHeight[ 0 ] = 75.0f;
		fMaxGrassHeight[ 1 ] = 75.0f;
		fMaxGrassHeight[ 2 ] = 75.0f;
		fMaxGrassHeight[ 3 ] = 75.0f;
		fMinGrassHeight[ 0 ] = 55.0f;
		fMinGrassHeight[ 1 ] = 55.0f;
		fMinGrassHeight[ 2 ] = 55.0f;
		fMinGrassHeight[ 3 ] = 55.0f;
		fMaxShake = 15.0f;
		fMinShake = 8.0f;
	}

	TerrainType Type;
	EtVector3 TerrainOffset;
	int nSizeX;
	int nSizeY;
	float fTileSize;
	float fHeightMultiply;
	float fTextureDistance;
	DWORD *pColor;
	short *pHeight;
	DWORD *pLayerDensity;

	char *pGrassBuffer;
	float fGrassWidth[ 4 ];
	float fMaxGrassHeight[ 4 ];
	float fMinGrassHeight[ 4 ];
	float fMaxShake;
	float fMinShake;
};

struct SGrassBlockInfo
{
	int nSizeX;
	int nSizeY;
	int nStride;
	float fTileSize;
	EtVector3 GrassOffset;
	char *pGrassBuffer;
	float fGrassWidth[ 4 ];
	float fMaxGrassHeight[ 4 ];
	float fMinGrassHeight[ 4 ];
	float fMaxShake;
	float fMinShake;
};

struct SPrimitiveDraw3D
{
	EtVector3 Position;
	DWORD dwColor;
};

struct SPrimitiveDraw3DTex
{
	EtVector3 Position;
	EtVector2 TexCoord;
};

struct SPrimitiveDraw2DTex
{
	EtVector3 Position;
	EtVector2 TexCoord;
};

struct SPrimitiveDraw2D
{
	EtVector4 Position;
	DWORD dwColor;
};

enum ShadowType
{
	ST_NONE,
	ST_SHADOWMAP,
	ST_DEPTHSHADOWMAP,
};

#define MAX_DIRECTIONAL_LIGHT_COUNT		5
#define MAX_POINT_LIGHT_COUNT			5
#define MAX_SPOT_LIGHT_COUNT			5
struct SGraphicOption
{
	SGraphicOption()
	{
		bUseTerrainLightMap = true;
		bEnableDOF = false;
		bEnableMotionBlur = false;
		bUseSplatting = true;
		bDrawGrass = true;
		bSoftShadow = false;
		DynamicShadowType = ST_SHADOWMAP;
		ShadowQuality = SQ_HIGH;
		WaterQuality = WQ_HIGH;	
		nMaxDirLightCount = MAX_DIRECTIONAL_LIGHT_COUNT;
		nMaxPointLightCount = MAX_POINT_LIGHT_COUNT;
		nMaxSpotLightcount = MAX_SPOT_LIGHT_COUNT;
		bDecalShadow = false;
		nGraphicQuality = 0;
		nTextureQuality = 0;
		nEffectQuality	= 0;
		bIsOnlyLowShaderAvailable = false;
	}

	bool bUseTerrainLightMap;
	bool bEnableDOF;
	bool bEnableMotionBlur;
	bool bUseSplatting;
	bool bDrawGrass;
	bool bSoftShadow;
	bool bDecalShadow;
	ShadowType DynamicShadowType;
	ShadowQuality ShadowQuality;
	WaterQuality WaterQuality;
	int nMaxDirLightCount;
	int nMaxPointLightCount;
	int nMaxSpotLightcount;
	int nGraphicQuality;
	int nTextureQuality;
	int nEffectQuality;
	bool bIsOnlyLowShaderAvailable;
	
};

struct SCollisionInfo
{
	EtVector3 CollisionNormal;
	float fContactTime;
};

enum EffectResourceType
{
	ERT_UNKNOWN = -1,
	ERT_MESH,
	ERT_PARTICLE,
	ERT_POINT_LIGHT,
	ERT_FORCE_DWORD	= 0xFFFFFFFF,
};
enum EffectTableType 
{
	ERT_COLOR_TABLE = 0,
	ERT_ALPHA_TABLE = 1,
	ERT_SCALE_TABLE = 2,
	ERT_POSITION_TABLE = 3,
	ERT_ROTATE_TABLE = 4,
	ERT_AMOUNT = 5,
};

struct STextureVertex2D
{
	EtVector4 Position;
	EtVector2 TexCoordinate;
};
struct STextureVertex
{
	EtVector3 Position;
	EtVector2 TexCoordinate;
};

struct STextureDiffuseVertex
{
	EtVector3 Position;
	D3DCOLOR Color;
	EtVector2 TexCoordinate;
};

struct STextureDiffuseVertex2D
{
	EtVector4 Position;
	D3DCOLOR Color;
	EtVector2 TexCoordinate;
};

struct SColorVertex
{
	float x, y, z, h;
	D3DCOLOR color;
};

struct SUICoord
{
public:
	SUICoord()
		: fX(.0f)
		, fY(.0f)
		, fWidth(.0f)
		, fHeight(.0f)
	{
	}

	SUICoord( float X, float Y, float Width, float Height )
		: fX(X)
		, fY(Y)
		, fWidth(Width)
		, fHeight(Height)
	{
	}

	SUICoord( const SUICoord &uiCoord )
	{
		SetCoord( uiCoord.fX, uiCoord.fY, uiCoord.fWidth, uiCoord.fHeight );
	}

	void SetPosition( float fx, float fy )
	{
		fX = fx;
		fY = fy;
	}

	void SetSize( float fwidth, float fheight )
	{
		fWidth = fwidth;
		fHeight = fheight;
	}

	void SetCoord( float X, float Y, float Width, float Height )
	{
		fX = X;
		fY = Y;
		fWidth = Width;
		fHeight = Height;
	}

	void SetRect( float left, float top, float right, float bottom )
	{
		fX = left;
		fY = top;
		fWidth = right-left;
		fHeight = bottom-top;
	}

	bool IsInside( float X, float Y ) const
	{
		if( ( X >= fX ) && ( X <= Right() ) && ( Y >= fY ) && ( Y <= Bottom() ) )
		{
			return true;
		}

		return false;
	}

	bool IntersectRect( SUICoord &coord )
	{
		float fminX = min(fX, coord.fX);
		float fmaxX = max(Right(), coord.Right());
		float fminY = min(fY, coord.fY);
		float fmaxY = max(Bottom(), coord.Bottom());

		if((fmaxX - fminX) < (fWidth+coord.fWidth) && (fmaxY - fminY) < (fHeight+coord.fHeight))
		{
			return true;
		}

		return false;
	}

	float Right() const
	{
		return fX + fWidth;
	}

	float Bottom() const
	{
		return fY + fHeight;
	}

	void Merge( SUICoord mergeCoord )
	{
		float fRight = EtMax( fX + fWidth, mergeCoord.fX + mergeCoord.fWidth ) ;
		float fBottom = EtMax( fY + fHeight, mergeCoord.fY + mergeCoord.fHeight );
		
		fX = EtMin( fX, mergeCoord.fX );
		fY = EtMin( fY, mergeCoord.fY );

		fWidth = fRight - fX;
		fHeight = fBottom - fY;	
	}

	float fX;
	float fY;
	float fWidth;
	float fHeight;
};
