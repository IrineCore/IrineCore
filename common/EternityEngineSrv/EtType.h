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

enum TerrainType
{
	TT_NORMAL,
	TT_CLIFF,
	TT_DETAILNORMAL,
	TT_DETAILCLIFF,
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
		fViewHeight = 0.0f;
		fViewWidth = 0.0f;
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

struct SCollisionInfo
{
	EtVector3 CollisionNormal;
	float fContactTime;
};
