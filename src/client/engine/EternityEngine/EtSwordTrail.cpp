#include "StdAfx.h"
#include "EtSwordTrail.h"
#include "EtBackBufferMng.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

EtIndexBuffer *CEtSwordTrail::s_pCommonIndexBuffer = NULL;
EtMaterialHandle CEtSwordTrail::s_hMaterial;

/*********************************** R N S **************************************************/
// cubic curve defined by 2 positions and 2 velocities
D3DXVECTOR3 CEtRNS::GetPositionOnCubic(const D3DXVECTOR3 &startPos, const D3DXVECTOR3 &startVel, const D3DXVECTOR3 &endPos, const D3DXVECTOR3 &endVel, float time)
{
	D3DXMATRIX hermite(	2.f,-2.f, 1.f, 1.f,
		-3.f, 3.f,-2.f,-1.f,
		0.f, 0.f, 1.f, 0.f,
		1.f, 0.f, 0.f, 0.f);
	D3DXMATRIX m;
	*(D3DXVECTOR3*)&m._11 = startPos;
	*(D3DXVECTOR3*)&m._21 = endPos;
	*(D3DXVECTOR3*)&m._31 = startVel;
	*(D3DXVECTOR3*)&m._41 = endVel;
	D3DXMatrixMultiply(&m, &hermite, &m);

	D3DXVECTOR4 timeVector = D3DXVECTOR4(time*time*time, time*time, time, 1);
	D3DXVec4Transform(&timeVector, &timeVector, &m);
	return D3DXVECTOR3(timeVector.x, timeVector.y, timeVector.z);
}
// adds node and updates segment length
void CEtRNS::AddNode(const D3DXVECTOR3 &pos)
{
	if (m_nodeCount == 0)
		m_maxDistance = 0.f;
	else
	{
		m_nodes[m_nodeCount-1].distance = D3DXVec3Length(&(m_nodes[m_nodeCount-1].position - pos));
		m_maxDistance += m_nodes[m_nodeCount-1].distance;
	}
	m_nodes[m_nodeCount++].position = pos;
}

// called after all nodes added. This function calculates the node velocities
void CEtRNS::BuildSpline()
{
	for (int i = 1; i<m_nodeCount-1; i++) {
		// split the angle (figure 4)
		D3DXVECTOR3 v1 = m_nodes[i+1].position - m_nodes[i].position;
		D3DXVECTOR3 v2 = m_nodes[i-1].position - m_nodes[i].position;
		D3DXVec3Normalize(&v1, &v1);
		D3DXVec3Normalize(&v2, &v2);
		D3DXVec3Normalize(&m_nodes[i].velocity, &(v1 - v2));
	}
	// calculate start and end velocities
	m_nodes[0].velocity = GetStartVelocity(0);
	m_nodes[m_nodeCount-1].velocity = GetEndVelocity(m_nodeCount-1);
}

// spline access function. time is 0 -> 1
D3DXVECTOR3 CEtRNS::GetPosition(float time)
{
	float distance = time * m_maxDistance;
	float currentDistance = 0.f;
	int i = 0;
	while (i < m_maxCount && currentDistance + m_nodes[i].distance < distance ) {
		currentDistance += m_nodes[i].distance;
		i++;
	}
	if( i > m_maxCount-2 ) {
		return m_nodes.back().position;		
	}
	float t = distance - currentDistance;
	t /= m_nodes[i].distance; // scale t in range 0 - 1
	D3DXVECTOR3 startVel = m_nodes[i].velocity * m_nodes[i].distance;
	D3DXVECTOR3 endVel = m_nodes[i+1].velocity * m_nodes[i].distance;   
	return GetPositionOnCubic(m_nodes[i].position, startVel,
		m_nodes[i+1].position, endVel, t);
}

// internal. Based on Equation 14 
D3DXVECTOR3 CEtRNS::GetStartVelocity(int index)
{
	D3DXVECTOR3 temp = 3.f * (m_nodes[ index + 1 ].position - m_nodes[ index ].position) / m_nodes[ index ].distance;
	return (temp - m_nodes[ index + 1 ].velocity) * 0.5f;
}

// internal. Based on Equation 15 
D3DXVECTOR3 CEtRNS::GetEndVelocity(int index)
{
	D3DXVECTOR3 temp = 3.f * (m_nodes[ index ].position - m_nodes[ index - 1 ].position) / m_nodes[ index - 1 ].distance;
	return (temp - m_nodes[ index - 1 ].velocity) * 0.5f;
}

void CEtRNS::Init( int maxCount_ )
{
	m_nodeCount = 0;
	m_maxCount = maxCount_;
	m_nodes.resize( maxCount_ );
}


//////////////////////////////////////////////////////////////////////////

CEtSwordTrail::CEtSwordTrail(void)
{
	m_SwordColor = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );
	m_bAdd = false;
	m_bPrevAdd = false;
	m_nAddIndex = -1;
	m_nTechniqueIndex = 0;
	m_fElapsedTime = 0.f;
	m_fTotalTime = 0.f;
	m_fFinishTime = 0.f;
	m_nLengthTime = 0;
	m_fLifeTime = 0.f;
}

CEtSwordTrail::~CEtSwordTrail(void)
{
	for( std::vector< TrailSet >::iterator it = m_Trails.begin(); it != m_Trails.end(); ++it ) {
		SAFE_DELETE( it->pMeshStream );
	}
	m_Trails.clear();

}

void CEtSwordTrail::Initialize( EtAniObjectHandle hAniObject, EtTextureHandle hTexture, EtTextureHandle hNormalTexture, float fLifeTime, float fMinSegment )
{
	m_fLifeTime = fLifeTime;
	m_hParent = hAniObject;

	if( !m_hParent || !m_hParent->GetMesh() || m_hParent->GetMesh()->GetDummyNameList().empty() ) {
		return;
	}

	std::map< std::string, int > DummyIndices;
	int nIndex = 0;
	for each ( std::string szName in m_hParent->GetMesh()->GetDummyNameList() ) 
	{
		if( strncmp(szName.c_str(), "#trail", 6) == 0 )
			DummyIndices[ szName ] = nIndex;

		++nIndex;
	}

	//	assert( (DummyIndices.size() % 2) == 0 );
	if( ( DummyIndices.size() % 2 ) != 0 )
	{
		return;
	}

	std::map< std::string, int >::iterator it = DummyIndices.begin();
	for ( int i = 0; i < (int)DummyIndices.size()/2; i++) {
		TrailSet trailSet;
		trailSet.nDummyIndex[ 0 ] = it->second; ++it;
		trailSet.nDummyIndex[ 1 ] = it->second; ++it;
		trailSet.pMeshStream = new CEtMeshStream;
		CreateVertexBuffer( trailSet.pMeshStream );
		m_Trails.push_back( trailSet );
	}

#ifdef PRE_FIX_MATERIAL_DUMP
	if( !s_hMaterial ) return;
#endif

	int nTexIndex;
	nTexIndex = hTexture ? hTexture->GetMyIndex() : -1;
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_SwordTrailTexture", &nTexIndex );

	nTexIndex = hNormalTexture ? hNormalTexture->GetMyIndex() : -1;
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_SwordTrailNormalTexture", &nTexIndex );

	int nBackBufferIndex = GetEtBackBufferMng()->GetBackBufferIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_BackBuffer", &nBackBufferIndex );	
	AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, s_hMaterial, "g_SwordTrailColor", &m_SwordColor );	
}

void CEtSwordTrail::ChangeTexture( EtTextureHandle hTexture, EtTextureHandle hNormalTexture )
{
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !s_hMaterial ) return;
#endif

	int nTexIndex;
	nTexIndex = ( hTexture ) ? hTexture->GetMyIndex() : -1;
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_SwordTrailTexture", &nTexIndex );

	nTexIndex = ( hNormalTexture ) ? hNormalTexture->GetMyIndex() : -1;
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_SwordTrailNormalTexture", &nTexIndex );
}

void CEtSwordTrail::CreateVertexBuffer( CEtMeshStream *pMeshStream )
{
	pMeshStream->CreateVertexStream( MST_POSITION, 0, MAX_SWORD_TRAIL_VERTEX );
	pMeshStream->CreateVertexStream( MST_TEXCOORD, 0, MAX_SWORD_TRAIL_VERTEX );
	pMeshStream->SetIndexBuffer( s_pCommonIndexBuffer, ( MAX_SWORD_TRAIL_VERTEX - 2 ) * 3 );
}

void CEtSwordTrail::CopyVertexBuffer()
{
	for( std::vector< TrailSet >::iterator it = m_Trails.begin(); it != m_Trails.end(); ++it ) {

		CMemoryStream Stream;
		int nVertexCount;

		nVertexCount = min( ( int )it->vecPosition.size(), MAX_SWORD_TRAIL_VERTEX );
		if( nVertexCount <= 2 ) {
			continue;
		}

		it->vecPositionResult.clear();
		it->vecTimeResult.clear();

		int nHalfVertexCount = nVertexCount / 2;

		it->spline[0].Init( nHalfVertexCount );
		it->spline[1].Init( nHalfVertexCount );
		for( int i = 0; i < nHalfVertexCount; i++) {
			it->spline[0].AddNode( it->vecPosition[ i*2 ] );
			it->spline[1].AddNode( it->vecPosition[ i*2+1 ] );
		}
		it->spline[0].BuildSpline();
		it->spline[1].BuildSpline();

		int nTotalVertexCount = min( 40, nHalfVertexCount*4 );

		for( int i = 0; i <= nTotalVertexCount; i++) {
			float t = float(i) / (nTotalVertexCount) + 0.01f;
			if( nVertexCount == 4 ) {
				EtVector3 vPos1, vPos2;
				EtVec3Lerp(&vPos1, &it->vecPosition[0], &it->vecPosition[2], t);
				EtVec3Lerp(&vPos2, &it->vecPosition[1], &it->vecPosition[3], t);
				it->vecPositionResult.push_back( vPos1 );
				it->vecPositionResult.push_back( vPos2 );			
			}
			else {
				it->vecPositionResult.push_back( it->spline[0].GetPosition( t ) );
				it->vecPositionResult.push_back( it->spline[1].GetPosition( t ) );
			}		
			it->vecTimeResult.push_back( EtVector2(t, 1) );
			it->vecTimeResult.push_back( EtVector2(t, 0) );
		}

		if( !it->vecPositionResult.empty() ) {
			int nVertexCountInner = (int)it->vecPositionResult.size();
			Stream.Initialize( &it->vecPositionResult[ 0 ], nVertexCountInner * sizeof( EtVector3 ) );
			it->pMeshStream->LoadVertexStream( &Stream, MST_POSITION, 0, nVertexCountInner );

			Stream.Initialize( &it->vecTimeResult[ 0 ], nVertexCountInner * sizeof( EtVector2 ) );
			it->pMeshStream->LoadVertexStream( &Stream, MST_TEXCOORD, 0, nVertexCountInner );
		}
	}

}

void CEtSwordTrail::AddPoint( int nIndex, int nLengthTime  )
{
	if( m_bAdd ) return; 
	m_bAdd = true;
	m_nAddIndex = nIndex;	
	m_nLengthTime = nLengthTime;
}

void CEtSwordTrail::Render( float fElapsedTime )
{
	m_fElapsedTime += fElapsedTime;		// Can Reset
	m_fTotalTime += fElapsedTime;		// Cannot Reset

	ProcessAdd();

	int nEraseEnd;

	float fCurTime = m_fElapsedTime;

	for( std::vector< TrailSet >::iterator it = m_Trails.begin(); it != m_Trails.end(); ++it ) {
		nEraseEnd = -1;

		for( int i = 0; i < ( int )it->vecTime.size(); i ++ ) {
			if( fCurTime - it->vecTime[ i ] > m_fLifeTime )
			{
				nEraseEnd = i + 1;
			}
			else
			{
				break;
			}
		}
		if( nEraseEnd != -1 ) {
			it->vecPosition.erase( it->vecPosition.begin(), it->vecPosition.begin() + nEraseEnd * 2);
			it->vecTime.erase( it->vecTime.begin(), it->vecTime.begin() + nEraseEnd );
		}
		if( it->vecPosition.size() <= 2 ) {
			continue;
		}
		CopyVertexBuffer();

		if( it->vecPositionResult.size() <=  2 ) {
			continue;
		}

		SRenderStackElement RenderElement;
		EtMatrix WorldMat;

		EtMatrixIdentity( &WorldMat );
		RenderElement.hMaterial = s_hMaterial;
		RenderElement.nTechniqueIndex = m_nTechniqueIndex;
		RenderElement.WorldMat = WorldMat;
		RenderElement.nSaveMatIndex = -1;
		RenderElement.pvecCustomParam = &m_vecCustomParam;
		RenderElement.pRenderMeshStream = it->pMeshStream;
		RenderElement.nDrawCount = ( int )it->vecPositionResult.size() - 2;

		SStateBlock RenderState;
		RenderState.AddRenderState( D3DRS_CULLMODE, CULL_NONE );
		RenderState.AddRenderState( D3DRS_BLENDOP, m_BlendOP );
		RenderState.AddRenderState( D3DRS_SRCBLEND, m_SourceBlend );
		RenderState.AddRenderState( D3DRS_DESTBLEND, m_DestBlend );
		RenderElement.nStateBlockIndex = CEtStateBlockManager::GetInstance().CreateStateBlock(&RenderState);

		GetCurRenderStack()->AddUseBackBufferRenderElement( RenderElement );
	}
}

void CEtSwordTrail::InitializeEffect()
{
	s_hMaterial = LoadResource( "SwordTrail.fx", RT_SHADER );
	if( s_pCommonIndexBuffer == NULL )
	{
		int i, nQuadCount, nFaceCount;
		WORD *pIndexBuffer;
		CMemoryStream Stream;

		nFaceCount = ( MAX_SWORD_TRAIL_VERTEX - 2 );
		nQuadCount = MAX_SWORD_TRAIL_VERTEX / 2 - 1;
		s_pCommonIndexBuffer = GetEtDevice()->CreateIndexBuffer( nFaceCount * 3 * sizeof( WORD ) );
		s_pCommonIndexBuffer->Lock( 0, nFaceCount * 3 * sizeof( WORD ), ( void ** )&pIndexBuffer, 0 );
		for( i = 0; i < nQuadCount; i++ )
		{
			pIndexBuffer[ i * 6 ] = i * 2;
			pIndexBuffer[ i * 6 + 1 ] = i * 2 + 2;
			pIndexBuffer[ i * 6 + 2 ] = i * 2 + 1;
			pIndexBuffer[ i * 6 + 3 ] = i * 2 + 1;
			pIndexBuffer[ i * 6 + 4 ] = i * 2 + 2;
			pIndexBuffer[ i * 6 + 5 ] = i * 2 + 3;			
		}
		s_pCommonIndexBuffer->Unlock();
	}
}

void CEtSwordTrail::FinializeEffect()
{
	SAFE_RELEASE_SPTR( s_hMaterial );
	SAFE_RELEASE( s_pCommonIndexBuffer );
}

void CEtSwordTrail::ProcessAdd()
{
	if( m_bPrevAdd != m_bAdd ) {
		if( m_bAdd ) {
			m_fFinishTime = m_fTotalTime + m_nLengthTime * 0.001f;
		}
		m_bPrevAdd = m_bAdd;
	}
	if( m_bAdd ) {
		if( m_fFinishTime < m_fTotalTime ) {
			m_bAdd = false;
			m_bPrevAdd = false;
			return;
		}
	}
	else {
		return;
	}
	if( !m_hParent ) {
		return;
	}
	if( !m_hParent->GetMesh() || m_hParent->GetMesh()->GetDummyMatrixList().empty() ) {
		return;
	}

	int nIndex = 0;

	for( std::vector< TrailSet >::iterator it = m_Trails.begin(); it != m_Trails.end(); ++it, nIndex++ ) {

		if( it->nDummyIndex[0] >= (int)m_hParent->GetMesh()->GetDummyMatrixList().size() || 
			it->nDummyIndex[1] >= (int)m_hParent->GetMesh()->GetDummyMatrixList().size() ||
			it->nDummyIndex[0] < 0 || it->nDummyIndex[1] < 0  ) {
				continue;
		}
		if( m_nAddIndex != -1 && nIndex != m_nAddIndex ) {
			continue;
		}

		EtVector3 PointA(0,0,0), PointB(0,0,0);

		EtMatrix *pMatParent = m_hParent->GetWorldMat();		

		if( m_hParent->GetAniHandle() ) {
			//			if( !m_hParent->GetAniHandle() ) {
			//				continue;
			//			}

			const char *szBoneName1 = m_hParent->GetMesh()->GetDummyNameList()[ it->nDummyIndex[0] ].c_str();
			const char *szBoneName2 = m_hParent->GetMesh()->GetDummyNameList()[ it->nDummyIndex[1] ].c_str();

			EtVector3 vLocalPos1 = m_hParent->GetMesh()->GetDummyPosByName( szBoneName1 );
			EtVector3 vLocalPos2 = m_hParent->GetMesh()->GetDummyPosByName( szBoneName2 );

			char *pParentName1 = m_hParent->GetMesh()->GetDummyParentByName( szBoneName1 );
			char *pParentName2 = m_hParent->GetMesh()->GetDummyParentByName( szBoneName2 );

			int nBoneIndex1 = m_hParent->GetAniHandle()->GetBoneIndex( pParentName1 );
			int nBoneIndex2 = m_hParent->GetAniHandle()->GetBoneIndex( pParentName2 );

			if( ( nBoneIndex1 == -1 ) || ( nBoneIndex1 == -1 ) )
			{
				EtVec3TransformCoord(&PointA, ( EtVector3 * )&m_hParent->GetMesh()->GetDummyMatrixList()[ it->nDummyIndex[0] ]._41, pMatParent);
				EtVec3TransformCoord(&PointB, ( EtVector3 * )&m_hParent->GetMesh()->GetDummyMatrixList()[ it->nDummyIndex[1] ]._41, pMatParent);
			}
			else
			{
				EtMatrix mat;
				EtMatrixMultiply( &mat, m_hParent->GetBoneTransMat( nBoneIndex1 ), pMatParent ); 
				EtVec3TransformCoord(&PointA, &vLocalPos1, &mat);
				EtMatrixMultiply( &mat, m_hParent->GetBoneTransMat( nBoneIndex2 ), pMatParent ); 
				EtVec3TransformCoord(&PointB, &vLocalPos2, &mat);
			}

		}
		else {

			EtVec3TransformCoord(&PointA, ( EtVector3 * )&m_hParent->GetMesh()->GetDummyMatrixList()[ it->nDummyIndex[0] ]._41, pMatParent);
			EtVec3TransformCoord(&PointB, ( EtVector3 * )&m_hParent->GetMesh()->GetDummyMatrixList()[ it->nDummyIndex[1] ]._41, pMatParent);
		}		

		if( !it->vecPosition.empty() )
		{
			float fLastTime = it->vecTime.back();

			float fCurTime = m_fElapsedTime;
			if( ( fCurTime - fLastTime ) < 0.01666f )	// Do not omit points arriving faster than 1/60 sec; update at the front.
			{
				it->vecPosition.back() = PointA;
				it->vecPosition.back() = PointB;
			}
			else {
				it->vecPosition.push_back( PointA );
				it->vecPosition.push_back( PointB );

				it->vecTime.push_back( fCurTime );
			}
		}
		else
		{
			m_fElapsedTime = 0.f;
			it->vecPosition.push_back( PointA );
			it->vecPosition.push_back( PointB );
			it->vecTime.push_back( 0.0f );
		}		
	}
}