#pragma once

#include "EtPrimitive.h"

class CEtConvexVolume;

class CEtTerrain
{
public:
	CEtTerrain(void);
	virtual ~CEtTerrain(void);
	virtual void Clear();

protected:
	EtMaterialHandle m_hMaterial;
	std::vector< SCustomParam >	m_vecCustomParam;
	int m_nTechniqueIndex;

	CEtMeshStream m_MeshStream;

	EtTextureHandle m_hLightMap;

	bool m_bEnable;
	bool m_bDrawGrid;
	DWORD m_dwGridColor;
	int m_nStride;
	int m_nStrideVert;
	int m_nBlockOffsetX;
	int m_nBlockOffsetY;
	STerrainInfo m_TerrainInfo;
	std::vector< float > m_vecTextureDist;
	std::vector< EtTextureHandle > m_vecTextureHandle;
	std::vector< float > m_vecTextureRotation;
	std::vector< float > m_vecTextureRotationParam;
	SAABox m_BoundingBox;
	int m_nMaterialName;

	int m_nRenderUniqueID;
	int m_nBakeDepthType;

	static char *s_szTerrainMaterial[ 6 ];

public:
	virtual void BakeLowDetailMap() {}
	void SetTextureLayerCount( int nCount );

	void Enable( bool bEnable ) { m_bEnable = bEnable; }
	bool IsEnable() { return m_bEnable; }

	void DrawGrid( bool bDraw ) { m_bDrawGrid = bDraw; }
	void SetGridColor( DWORD dwColor ) { m_dwGridColor = dwColor; }

	void SetStride( int nStride, int nStrideVert ) 
	{ 
		m_nStride = nStride; 
		m_nStrideVert = nStrideVert; 
	}
	void SetBlockOffset( int nOffsetX, int nOffsetY ) { m_nBlockOffsetX = nOffsetX, m_nBlockOffsetY = nOffsetY; }

	inline int TileOffset( int nX, int nY ) { return nY * m_TerrainInfo.nSizeX + nX; }
	inline int VertexOffset( int nX, int nY ) { return nY * ( m_TerrainInfo.nSizeX + 1 ) + nX; }

	inline float GetHeight( int nX, int nY ) 
	{
		if( ( nX + m_nBlockOffsetX >= m_nStride ) || ( nY + m_nBlockOffsetY >= m_nStrideVert ) || ( nX + m_nBlockOffsetX < 0 ) || ( nY + m_nBlockOffsetY < 0 ) )
		{
			return 0.0f;
		}
		else
		{
			return m_TerrainInfo.pHeight[ nY * m_nStride + nX ] * m_TerrainInfo.fHeightMultiply; 
		}
	}
	inline DWORD GetLayerValue( int nX, int nY ) 
	{ 
		if( ( nX + m_nBlockOffsetX >= m_nStride ) || ( nY + m_nBlockOffsetY >= m_nStrideVert ) )
		{
			return 0;
		}
		else
		{
			return m_TerrainInfo.pLayerDensity[ nY * m_nStride + nX ]; 
		}
	}
	inline void SetLayerValue( int nX, int nY, DWORD dwValue ) 
	{ 
		if( ( nX + m_nBlockOffsetX >= m_nStride ) || ( nY + m_nBlockOffsetY >= m_nStrideVert ) )
		{
			m_TerrainInfo.pLayerDensity[ nY * m_nStride + nX ] = 0xff000000;
		}
		else
		{
			m_TerrainInfo.pLayerDensity[ nY * m_nStride + nX ] = dwValue; 
		}
	}


	virtual void CheckLayerCount() {}
	virtual void Initialize();
	virtual void CreateMaterial( const char *pEffectName );
	STerrainInfo *GetTerrainInfo() { return &m_TerrainInfo; }
	void SetTerrainInfo( STerrainInfo *pInfo );
	virtual void GenerateTexureCoord();
	void GeneratePosition( EtVector3 * pBuffer );
	void GenerateNormal( EtVector3 * pBuffer );
	DWORD ConvertLayerValue( DWORD dwLayer );
	void GenerateLayer( DWORD *pBuffer );
	virtual void GenerateVertexBuffer();
	void SetCommonIndexBuffer( EtIndexBuffer *pIndexBuffer, int nFaceCount ) 
	{
		m_MeshStream.SetIndexBuffer( pIndexBuffer, nFaceCount * 3 );
	}
	void CalcPosition( EtVector3 *pPosition, int nCount );
	void CalcNormal( EtVector3 *pNormal, int nCount );
	EtVector3 GetVertexNormal( int nIndexX, int nIndexY );
	void UpdateHeight();
	void UpdateLayer();
	void UpdateNormal();
	void UpdateAll();

	void Render();
	void RenderWater(int index);
	void GetExtent( EtVector3 &Origin, EtVector3 &Extent );
	SAABox *GetBoundingBox() { return &m_BoundingBox; }
	void CalcBoundingBox();
	CEtMeshStream &GetMeshStream() { return m_MeshStream; }

	float Pick( EtVector3 &Origin, EtVector3 &Direction, EtVector3 &PickPos );
	bool IsInTerrain( int nX, int nY );
	float TestLintToTile( EtVector3 &Origin, EtVector3 &Direction, int nTileX, int nTileY );

	SCustomParam *FindEffectParam( EffectParamType Type, int nIndex );
	EtMaterialHandle GetMaterial()
		{ return m_hMaterial; }
	int GetTextureCount() 
		{ return ( int )m_vecTextureHandle.size(); }
	const char *GetTextureName( int nTexIndex );
	virtual bool SetTexture( int nTexIndex, const char *pTexName );
	void SetTextureParam( int nTexIndex );
	const char *GetTextureSemanticName( int nTexIndex );

	void SetTextureDistance( int nTexLayer, float fDistance );
	float GetTextureDistance( int nTexLayer );

	void SetTextureRotation( int nTexLayer, float fRotate );
	float GetTextureRotation( int nTexLayer );

	void DrawRect( int nX, int nY, DWORD dwColor );

	void SetLightMap( EtTextureHandle hLightMap );
	EtTextureHandle GetLightMap();

#ifdef _TOOLCOMPILE
	EtTextureHandle GetTexture( int nTexIndex ) { return m_vecTextureHandle[nTexIndex]; }
#endif
};

class CEtDetailTerrain : public CEtTerrain
{
public:
	CEtDetailTerrain(void);
	virtual ~CEtDetailTerrain(void);

protected:
	void CreateMaterial( const char *pEffectName );

public:

};
