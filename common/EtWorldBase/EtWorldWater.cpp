#include "stdafx.h"
#include "EtWorldWater.h"
#include "EtWorldWaterRiver.h"
#include "EtWorldGrid.h"
#include "EtWorldSector.h"
#include "EngineUtil.h"
//#include "EtWater.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

template<> float CEtQuadtreeNode<CEtWorldWaterRiver *>::s_fMinRadius = 1000.f;

CEtWorldWater::CEtWorldWater( CEtWorldSector *pSector )
: m_vUVRatio( 5.f, 5.f )
, m_vOffset( 0.f, 0.f )
{
	m_pSector = pSector;
	m_fWaterLevel = 0.f;
	m_fGridSize = 200.f;
	m_pAlphaTable = NULL;

	m_nAlphaTableWidth = 512;
	m_nAlphaTableHeight = 512;

	m_nBorderReduction = 10;
	m_nGaussianValue = 5;
	m_fGaussianEpsilon = 3.f;
	m_nMinAlpha = 0;
	m_nMaxAlpha = 200;
	m_fRevisionAlpha = 1.f;
	m_bEnable = false;
	m_pRiverQuadtree = NULL;
}

CEtWorldWater::~CEtWorldWater()
{
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		m_pRiverQuadtree->Remove( m_pVecRiver[i], m_pVecRiver[i]->GetCurQuadtreeNode() );
	}

	SAFE_DELETE( m_pRiverQuadtree );
	SAFE_DELETEA( m_pAlphaTable );
	SAFE_DELETE_PVEC( m_pVecCustomParamList );
	SAFE_DELETE_PVEC( m_pVecRiver );
}

CEtWorldWaterRiver *CEtWorldWater::AllocRiver()
{
	return new CEtWorldWaterRiver( this );
}

bool CEtWorldWater::Initialize()
{
	float fSize = max( GetSector()->GetParentGrid()->GetGridWidth() * 100.f, GetSector()->GetParentGrid()->GetGridHeight() * 100.f );

	m_pRiverQuadtree = new CEtQuadtree<CEtWorldWaterRiver *>;
	m_pRiverQuadtree->Initialize( EtVector2( GetSector()->GetOffset()->x, GetSector()->GetOffset()->z ), fSize );

	m_vOffset = EtVec3toVec2( *GetSector()->GetOffset() );
	m_vOffset.x -= ( GetSector()->GetTileWidthCount() * GetSector()->GetTileSize() ) / 2.f;
	m_vOffset.y -= ( GetSector()->GetTileHeightCount() * GetSector()->GetTileSize() ) / 2.f;
	return true;
}

bool CEtWorldWater::Load( const char *szFileName )
{
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return true;

	// Actual data
	Stream.Read( &m_bEnable, sizeof(bool) );
	Stream.Read( &m_nAlphaTableWidth, sizeof(int) );
	Stream.Read( &m_nAlphaTableHeight, sizeof(int) );
	Stream.Read( &m_fGridSize, sizeof(float) );
	Stream.Read( &m_vUVRatio, sizeof(EtVector2) );
	Stream.Read( &m_fWaterLevel, sizeof(float) );
	SAFE_DELETE( m_pAlphaTable );
	m_pAlphaTable = new BYTE[ m_nAlphaTableWidth * m_nAlphaTableHeight ];
	Stream.Read( m_pAlphaTable, sizeof(BYTE) * ( m_nAlphaTableWidth * m_nAlphaTableHeight ) );

	Stream.Read( &m_nBorderReduction, sizeof(int) );
	Stream.Read( &m_nGaussianValue, sizeof(int) );
	Stream.Read( &m_fGaussianEpsilon, sizeof(float) );
	Stream.Read( &m_nMinAlpha, sizeof(int) );
	Stream.Read( &m_nMaxAlpha, sizeof(int) );
	Stream.Read( &m_fRevisionAlpha, sizeof(float) );

	ReadStdString( m_szShaderName, &Stream );

	int nCustomParamCount = 0;
	Stream.Read( &nCustomParamCount, sizeof(int) );

	SAFE_DELETE_PVEC( m_pVecCustomParamList );
	for( int i=0; i<nCustomParamCount; i++ ) {
		CustomParamStruct *pStruct = new CustomParamStruct;
		ReadStdString( pStruct->szName, &Stream );
		Stream.Read( &pStruct->nType, sizeof(int) );

		switch( pStruct->nType ) {
			case EPT_INT: Stream.Read( &pStruct->nInt, sizeof(int) ); break;
			case EPT_FLOAT: Stream.Read( &pStruct->fFloat, sizeof(float) ); break;
			case EPT_VECTOR: Stream.Read( pStruct->fFloat4, sizeof(float)*4 ); break;
			case EPT_TEX: 
				{
					std::string szTemp;
					ReadStdString( szTemp, &Stream );
					if( szTemp.empty() ) szTemp = "(null)";
					pStruct->szTexName = new char[szTemp.size()+1];
					memset( pStruct->szTexName, 0, szTemp.size()+1 );
					strcpy_s( pStruct->szTexName, szTemp.size()+1, szTemp.c_str() );
				}
		}
		m_pVecCustomParamList.push_back( pStruct );
	}
	int nRiverCount = 0;
	Stream.Read( &nRiverCount, sizeof(int) );
	
	for( int i=0; i<nRiverCount; i++ ) {
		CEtWorldWaterRiver *pRiver = AllocRiver();
		if( pRiver->Load( &Stream ) == false ) {
			SAFE_DELETE( pRiver );
			continue;
		}
		m_pVecRiver.push_back( pRiver );

		SCircle Circle;
		pRiver->GetBoundingCircle( Circle );
		pRiver->SetCurQuadtreeNode( m_pRiverQuadtree->Insert( pRiver, Circle ) );
	}

	return true;
}

DWORD CEtWorldWater::GetRiverCount()
{
	return (DWORD)m_pVecRiver.size();
}

CEtWorldWaterRiver *CEtWorldWater::GetRiverFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_pVecRiver.size() ) return NULL;
	return m_pVecRiver[dwIndex];
}

CEtWorldWaterRiver *CEtWorldWater::GetRiverFromName( const char *szName )
{
	for( DWORD i=0; i<m_pVecRiver.size(); i++ ) {
		if( _stricmp( m_pVecRiver[i]->GetName(), szName ) == NULL ) return m_pVecRiver[i];
	}
	return NULL;
}

bool CEtWorldWater::GetHeight( float fX, float fZ, float *pfHeight )
{
	std::vector<CEtWorldWaterRiver *> vecResult;

	EtVector2 vRiverPos( fX + m_vOffset.x, fZ + m_vOffset.y );
	ScanRiver( vRiverPos, 100.f, &vecResult );
	if( !vecResult.empty() ) {
		CEtWorldWaterRiver *pRiver;
		for( DWORD i=0; i<vecResult.size(); i++ ) {
			pRiver = vecResult[i];
			if( pRiver->GetRevisionAlpha() > 3.0f ) continue;	// Rivers with high RevisionAlpha are things like sand rivers, so handle as an exception.
			if( pRiver->GetHeight( fX, fZ, *pfHeight ) == true ) return true;
		}
	}
	if( m_bEnable ) {
		if( pfHeight ) *pfHeight = m_fWaterLevel;
		return true;
	}
	return false;
}

void CEtWorldWater::ScanRiver( EtVector2 &vPos, float fRadius, std::vector<CEtWorldWaterRiver *> *pVecResult )
{
	if( !m_pRiverQuadtree ) return;

	SCircle Circle;
	Circle.Center = vPos;
	Circle.fRadius = fRadius;
	m_pRiverQuadtree->Pick( Circle, *pVecResult, false );
}

bool CEtWorldWater::Pick( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	if( !m_bEnable ) return false;
	SAABox Box;
	Box.Reset();

	std::vector<CEtWorldWaterRiver *> pVecList;
	EtVector2 vOrig2D = EtVec3toVec2( vOrig );
	ScanRiver( vOrig2D, EtVec3Length( &vDir ), &pVecList );

	float fDistance;
	float fMinDist = FLT_MAX;
	EtVector3 vTempPick, vMinPick;
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		fDistance = pVecList[i]->Pick( vOrig, vDir, vTempPick );
		if( fDistance < fMinDist ) {
			vMinPick = vTempPick;
		}
	}
	if( fMinDist != FLT_MAX ) {
		vPickPos = vMinPick;
		return true;
	}


	EtVector3 *vOffset = m_pSector->GetOffset();
	float fWidth = ( m_pSector->GetTileWidthCount() * m_pSector->GetTileSize() ) / 2.f;
	float fHeight = ( m_pSector->GetTileHeightCount() * m_pSector->GetTileSize() ) / 2.f;
	Box.AddPoint( EtVector3( vOffset->x - fWidth, m_fWaterLevel, vOffset->z - fHeight ) );
	Box.AddPoint( EtVector3( vOffset->x + fWidth, m_fWaterLevel, vOffset->z + fHeight ) );

	if( TestLineToBox( vOrig, vDir, Box, fDistance ) ) {
		vPickPos = vOrig + ( vDir * fDistance );
		return true;
	}

	return false;
}