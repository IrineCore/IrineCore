#pragma once

#include "DnWorld.h"
#include "EtWorldSector.h"

class CEtWorldProp;
class CDnWorldProp;
class CEtWorldDecal;
class CDnWorldSector : public CEtWorldSector
{
public:
	CDnWorldSector();
	virtual ~CDnWorldSector();

protected:
	std::vector<CDnWorldProp *> m_pVecProcessProp;
	char *m_pTileTypePtr;

	int m_nDrawAttributeCount;
	std::vector<DWORD> m_dwVecAttributePrimitiveCount;
	std::vector<SPrimitiveDraw3D *> m_pVecAttributePrimitive;
	std::vector<bool *> m_bVecAttributePrimitiveType;
	std::set<GlobalEnum::TileTypeEnum> m_SetUseTileTypeList;

protected:
	virtual void InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, std::vector< std::vector<float> > &fVecLayerTextureDistance, std::vector< std::vector<float> > &fVecLayerTextureRotation, char *szGrassTexture, char *pGrassTable, float *fGrassWidth, float *fGrassHeightMin, float *fGrassHeightMax, float fShakeMin, float fShakeMax );
	virtual int GetPropClassID( const char *szFileName );

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void Free();

	virtual bool LoadProp( const char *szSectorPath, int nBlockIndex = -1, bool bThreadLoad = true );
	virtual bool LoadControlArea( const char *szSectorPath );

	virtual CEtWorldProp *AllocProp( int nClass );
	virtual CEtWorldSound *AllocSound();
	virtual CEtTrigger *AllocTrigger();
	virtual CEtWorldWater *AllocWater();
	virtual CEtWorldDecal *AllocDecal();

	bool CheckBakedLightMap();
	void BakeLightmap( int nBlurSize );

	void InsertProcessProp( CDnWorldProp *pProp );
	void RemoveProcessProp( CDnWorldProp *pProp );

	TileTypeEnum GetTileType( float fX, float fZ );

	int GetPropTableID( const char *szFileName );

	void RefreshQualityLevel( int nLevel );

	// There are props whose position changes. Call Update directly on those props.
	CEtOctree<CEtWorldProp*>* GetOctree( void ) { return m_pPropOctree; };

	void Show( bool bShow );
	bool IsUsingTileType( GlobalEnum::TileTypeEnum Type );
};