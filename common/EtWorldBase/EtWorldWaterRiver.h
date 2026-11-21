#pragma once

#include "EtWorldWater.h"

class CEtWorldWaterRiver
{
public:
	CEtWorldWaterRiver( CEtWorldWater *pWater );
	virtual ~CEtWorldWaterRiver();

	enum InterpolationType {
		Linear,
		CatmullRom,
		Helmite,
	};

protected:
	CEtWorldWater *m_pWater;
	std::string m_szName;
	std::string m_szShaderName;
	std::vector<EtVector3> m_vVecPointList;

	float m_fWidth;
	InterpolationType m_InterpolationType;
	// A minor issue occurred when using WaterRiver for flowing desert sand as well.
	// The Desert Dragon map is huge, so placing 6 water rivers far apart,
	// Drawing objects while baking in each water river caused performance to drop dramatically.
	// Could make a separate skn for flowing desert sand, but time is limited and making skins fit the terrain is difficult, so
	// Since it doesn't reflect anyway, added a property to skip the reflection routine, like m_bIgnoreReflection.
	// However, trying to add it required re-saving all Water.ini files (had to batch-convert since there was no version).
	// Creating a new file didn't look great, and it was inconvenient to manage overseas builds too.
	// So the conclusion was to use one of the unused attributes as an option indicating non-reflective (weird as it is..)
	// m_InterpolationType currently only uses linear; other options are completely unused.
	// If set to CatmullRom, treat it the same as linear but skip baking.

	BYTE *m_pAlphaTable;

	int m_nAlphaTableWidth;
	int m_nAlphaTableHeight;
	EtVector2 m_vUVRatio;
//	float m_fWaterLevel;

	int m_nBorderReduction;
	int m_nGaussianValue;
	float m_fGaussianEpsilon;
	int m_nMinAlpha;
	int m_nMaxAlpha;
	float m_fRevisionAlpha;

	std::vector<CEtWorldWater::CustomParamStruct *> m_pVecCustomParamList;
	CEtQuadtreeNode<CEtWorldWaterRiver *> *m_pCurrentNode;
	std::vector<EtVector3> m_vVecPrimitiveList;

	EtVector2 m_vSize;
	EtVector2 m_vOffset;
	EtVector2 m_vMin;

protected:
	void CalcSize();
	void GenerationPointList();

public:
	virtual bool Load( CStream *pStream );
	const char *GetName() { return m_szName.c_str(); }
	void SetName( char *szName ) { m_szName = szName; }

	const char *GetShaderName() { return m_szShaderName.c_str(); }
	void SetShaderName( char *szName ) { m_szShaderName = szName; }

	CEtWorldWater *GetWater() { return m_pWater; }

	void SetCurQuadtreeNode( CEtQuadtreeNode<CEtWorldWaterRiver *> *pNode ) { m_pCurrentNode = pNode; }
	CEtQuadtreeNode<CEtWorldWaterRiver *> *GetCurQuadtreeNode() { return m_pCurrentNode; }
	void GetBoundingCircle( SCircle &Circle );

	bool GetHeight( float fX, float fZ, float &fHeight );
	float Pick( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos );

	float GetRevisionAlpha() { return m_fRevisionAlpha; }
};