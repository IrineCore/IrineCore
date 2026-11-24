#include "stdafx.h"
#include "EtWorldWaterRiver.h"
#include "EtWorldSector.h"
#include "EtWorldWater.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CEtWorldWaterRiver::CEtWorldWaterRiver( CEtWorldWater *pWater )
: m_vUVRatio( 5.f, 5.f )
, m_vSize( 0.f, 0.f )
, m_vOffset( 0.f, 0.f )
{
	m_pWater = pWater;

	m_pAlphaTable = NULL;

	m_fWidth = 500.f;
	m_InterpolationType = Linear;
	m_nAlphaTableWidth = 256;
	m_nAlphaTableHeight = 256;

	m_nBorderReduction = 10;
	m_nGaussianValue = 5;
	m_fGaussianEpsilon = 3.f;
	m_nMinAlpha = 0;
	m_nMaxAlpha = 200;
	m_fRevisionAlpha = 1.f;

	m_pCurrentNode = NULL;
}

CEtWorldWaterRiver::~CEtWorldWaterRiver()
{
	SAFE_DELETE_VEC( m_vVecPointList );
	SAFE_DELETEA( m_pAlphaTable );
	SAFE_DELETE_PVEC( m_pVecCustomParamList );
}


bool CEtWorldWaterRiver::Load( CStream *pStream )
{
	// Actual data
	ReadStdString( m_szName, pStream );
	pStream->Read( &m_InterpolationType, sizeof(int) );
	pStream->Read( &m_fWidth, sizeof(float) );
	pStream->Read( &m_nAlphaTableWidth, sizeof(int) );
	pStream->Read( &m_nAlphaTableHeight, sizeof(int) );
	pStream->Read( &m_vUVRatio, sizeof(EtVector2) );

	SAFE_DELETE_VEC( m_vVecPointList );

	/*
	EtVector3 vOffset = *GetWater()->GetSector()->GetOffset();
	vOffset.x -= ( GetWater()->GetSector()->GetTileWidthCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.z -= ( GetWater()->GetSector()->GetTileHeightCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.y = 0.f;
	*/
	EtVector3 vPos;
	int nCount;
	pStream->Read( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		pStream->Read( &vPos, sizeof(EtVector3) );
//		vPos += vOffset;
		m_vVecPointList.push_back( vPos );
	}

	SAFE_DELETE( m_pAlphaTable );
	m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];
	pStream->Read( m_pAlphaTable, sizeof(BYTE) * ( m_nAlphaTableWidth * m_nAlphaTableHeight ) );

	pStream->Read( &m_nBorderReduction, sizeof(int) );
	pStream->Read( &m_nGaussianValue, sizeof(int) );
	pStream->Read( &m_fGaussianEpsilon, sizeof(float) );
	pStream->Read( &m_nMinAlpha, sizeof(int) );
	pStream->Read( &m_nMaxAlpha, sizeof(int) );
	pStream->Read( &m_fRevisionAlpha, sizeof(float) );

	ReadStdString( m_szShaderName, pStream );

	int nCustomParamCount = 0;
	pStream->Read( &nCustomParamCount, sizeof(int) );

	SAFE_DELETE_PVEC( m_pVecCustomParamList );
	for( int i=0; i<nCustomParamCount; i++ ) {
		CEtWorldWater::CustomParamStruct *pStruct = new CEtWorldWater::CustomParamStruct;
		ReadStdString( pStruct->szName, pStream );
		pStream->Read( &pStruct->nType, sizeof(int) );

		switch( pStruct->nType ) {
			case EPT_INT: pStream->Read( &pStruct->nInt, sizeof(int) ); break;
			case EPT_FLOAT: pStream->Read( &pStruct->fFloat, sizeof(float) ); break;
			case EPT_VECTOR: pStream->Read( pStruct->fFloat4, sizeof(float)*4 ); break;
			case EPT_TEX: 
				{
					std::string szTemp;
					ReadStdString( szTemp, pStream );
					if( szTemp.empty() ) szTemp = "(null)";
					pStruct->szTexName = new char[szTemp.size()+1];
					memset( pStruct->szTexName, 0, szTemp.size()+1 );
					strcpy_s( pStruct->szTexName, szTemp.size()+1, szTemp.c_str() );
				}
		}
		m_pVecCustomParamList.push_back( pStruct );
	}
	GenerationPointList();
	CalcSize();
	return true;
}

bool CEtWorldWaterRiver::GetHeight( float fX, float fZ, float &fHeight )
{
	EtVector3 vVertex[3];
	float fMinHeight = GetWater()->GetSector()->GetOffset()->y - ( GetWater()->GetSector()->GetHeightMultiply() * 32767.f );
	EtVector3 vPosTemp = EtVector3( fX, fMinHeight, fZ );
	float fDist, fBary1, fBary2;

	EtVector3 vUpDir( 0.f, 1.f, 0.f );
	for( DWORD i=0; i<m_vVecPrimitiveList.size() - 3; i+=2 ) {
		vVertex[0] = m_vVecPrimitiveList[i];
		vVertex[1] = m_vVecPrimitiveList[i+2];
		vVertex[2] = m_vVecPrimitiveList[i+1];
		if( TestLineToTriangle( vPosTemp, vUpDir, vVertex[0], vVertex[1], vVertex[2], fDist, fBary1, fBary2 ) == true ) {
			fHeight = vPosTemp.y + fDist;
			return true;
		}

		vVertex[0] = m_vVecPrimitiveList[i+1];
		vVertex[1] = m_vVecPrimitiveList[i+2];
		vVertex[2] = m_vVecPrimitiveList[i+3];
		if( TestLineToTriangle( vPosTemp, vUpDir, vVertex[0], vVertex[1], vVertex[2], fDist, fBary1, fBary2 ) == true ) {
			fHeight = vPosTemp.y + fDist;
			return true;
		}
	}
	return false;
}

void CEtWorldWaterRiver::GetBoundingCircle( SCircle &Circle )
{
	Circle.Center = m_vOffset + ( m_vSize / 2.f );
	Circle.fRadius = max( m_vSize.x, m_vSize.y ) / 2.f;
}

void CEtWorldWaterRiver::CalcSize()
{
	EtVector2 vMin = EtVector2( FLT_MAX, FLT_MAX );
	EtVector2 vMax = EtVector2( FLT_MAX, FLT_MAX );

	/*
	EtVector3 vDir, vVertex, vCross;
	for( DWORD i=0; i<m_vVecPointList.size(); i++ ) {
		if( i == m_vVecPointList.size() - 1 ) vDir = m_vVecPointList[i] - m_vVecPointList[i-1];
		else vDir = m_vVecPointList[i+1] - m_vVecPointList[i];

		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		vVertex = m_vVecPointList[i] + ( vCross * ( m_fWidth / 2.f ) );
		if( vMin.x == FLT_MAX || vVertex.x < vMin.x ) vMin.x = vVertex.x;
		if( vMax.x == FLT_MAX || vVertex.x > vMax.x ) vMax.x = vVertex.x;
		if( vMin.y == FLT_MAX || vVertex.z < vMin.y ) vMin.y = vVertex.z;
		if( vMax.y == FLT_MAX || vVertex.z > vMax.y ) vMax.y = vVertex.z;

		vVertex = m_vVecPointList[i] - ( vCross * ( m_fWidth / 2.f ) );
		if( vMin.x == FLT_MAX || vVertex.x < vMin.x ) vMin.x = vVertex.x;
		if( vMax.x == FLT_MAX || vVertex.x > vMax.x ) vMax.x = vVertex.x;
		if( vMin.y == FLT_MAX || vVertex.z < vMin.y ) vMin.y = vVertex.z;
		if( vMax.y == FLT_MAX || vVertex.z > vMax.y ) vMax.y = vVertex.z;

	}
	*/
	EtVector3 vVertex;
	for( DWORD i=0; i<m_vVecPrimitiveList.size(); i++ ) {
		vVertex = m_vVecPrimitiveList[i];
		if( vMin.x == FLT_MAX || vVertex.x < vMin.x ) vMin.x = vVertex.x;
		if( vMax.x == FLT_MAX || vVertex.x > vMax.x ) vMax.x = vVertex.x;
		if( vMin.y == FLT_MAX || vVertex.z < vMin.y ) vMin.y = vVertex.z;
		if( vMax.y == FLT_MAX || vVertex.z > vMax.y ) vMax.y = vVertex.z;
	}

	m_vOffset = vMin + *m_pWater->GetOffset();
	m_vMin = vMin;

	m_vSize.x = vMax.x - vMin.x;
	m_vSize.y = vMax.y - vMin.y;
}


void CEtWorldWaterRiver::GenerationPointList()
{
	EtVector3 vDir, vCross;
	EtVector3 vVertex;
	for( DWORD i=0; i<m_vVecPointList.size(); i++ ) {
		if( i == m_vVecPointList.size() - 1 ) vDir = m_vVecPointList[i] - m_vVecPointList[i-1];
		else vDir = m_vVecPointList[i+1] - m_vVecPointList[i];

		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		vVertex = m_vVecPointList[i] + ( vCross * ( m_fWidth / 2.f ) );
		m_vVecPrimitiveList.push_back( vVertex );
		vVertex = m_vVecPointList[i] - ( vCross * ( m_fWidth / 2.f ) );
		m_vVecPrimitiveList.push_back( vVertex );
	}

}

float CEtWorldWaterRiver::Pick( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	float fDistance;
	float fBary1, fBary2;
	float fMinDistance = FLT_MAX;
	int nIndex = -1;
	for( DWORD i=0; i<m_vVecPrimitiveList.size() - 2; i++ ) {
		if( TestLineToTriangle( vOrig, vDir, m_vVecPrimitiveList[i], m_vVecPrimitiveList[i+1], m_vVecPrimitiveList[i+2], fDistance, fBary1, fBary2 ) ) {
			if( fDistance < fMinDistance ) {
				nIndex = i;
				fMinDistance = fDistance;
			}
		}
	}

	if( nIndex != -1 ) {
		vPickPos = vOrig + ( vDir * fMinDistance );
	}

	return fMinDistance;
}