#include "StdAfx.h"
#include "EtLineEffect.h"
#include "EtPrimitiveDraw.h"

EtIndexBuffer *CEtLineEffect::s_pCommonIndexBuffer = NULL;
EtMaterialHandle CEtLineEffect::s_hMaterial;

CEtLineEffect::CEtLineEffect(void)
{
	m_bModify = false;
	m_LineColor = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );
	m_bShow = true;
	m_fElapsedTime = 0.f;
	m_fUMoveSpeed = 0.0f;
	m_fCurrentStartTime = 0.0f;
	m_fInverseLifeTime = 0.0f;
	m_fLifeTime = 0.0f;
	m_fLineWidth = 0.0f;
	m_fMinSegment = 0.0f;
}

CEtLineEffect::~CEtLineEffect(void)
{
}

void CEtLineEffect::Initialize( EtTextureHandle hTexture, float fLifeTime, float fLineWidth, float fUMoveSpeed )
{
	m_fLifeTime = fLifeTime;
	m_fLineWidth = fLineWidth;
	m_fUMoveSpeed = fUMoveSpeed;

	m_hTexture = hTexture;

	//CreateVertexBuffer();

	/*int nTexIndex;
	nTexIndex = hTexture->GetMyIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, s_hMaterial, "g_LineTrailTexture", &nTexIndex );
	AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, s_hMaterial, "g_LineTrailColor", &m_LineColor );
	AddCustomParam( m_vecCustomParam, EPT_FLOAT, s_hMaterial, "g_fLineTrailWidth", &m_fLineWidth );*/
}

void CEtLineEffect::CreateVertexBuffer()
{
	int nFaceCount;

	m_MeshStream.CreateVertexStream( MST_POSITION, 0, MAX_LINE_VERTEX );
	m_MeshStream.CreateVertexStream( MST_NORMAL, 0, MAX_LINE_VERTEX );
	m_MeshStream.CreateVertexStream( MST_TEXCOORD, 0, MAX_LINE_VERTEX );

	nFaceCount = ( MAX_LINE_VERTEX - 2 );
	m_MeshStream.SetIndexBuffer( s_pCommonIndexBuffer, nFaceCount * 3 );
}

void CEtLineEffect::CopyVertexBuffer()
{
	CMemoryStream Stream;
	int nVertexCount;

	nVertexCount = min( ( int )m_vecPosition.size(), MAX_LINE_VERTEX );
	if( nVertexCount <= 0 )
	{
		return;
	}
	Stream.Initialize( &m_vecPosition[ 0 ], nVertexCount * sizeof( EtVector3 ) );
	m_MeshStream.LoadVertexStream( &Stream, MST_POSITION, 0, nVertexCount );
	Stream.Initialize( &m_vecTangent[ 0 ], nVertexCount * sizeof( EtVector3 ) );
	m_MeshStream.LoadVertexStream( &Stream, MST_NORMAL, 0, nVertexCount );
	Stream.Initialize( &m_vecTime[ 0 ], nVertexCount * sizeof( EtVector2 ) );
	m_MeshStream.LoadVertexStream( &Stream, MST_TEXCOORD, 0, nVertexCount );
}

void CEtLineEffect::AddPoint( EtVector3 &start, EtVector3 &end )
{
	EtVector3 tangent = end - start;

	m_vecPosition.push_back( start );
	m_vecPosition.push_back( start );
	m_vecPosition.push_back( end );
	m_vecPosition.push_back( end );

	m_vecTangent.push_back(tangent);
	m_vecTangent.push_back(tangent);
	m_vecTangent.push_back(tangent);
	m_vecTangent.push_back(tangent);
	
	m_vecTime.push_back( EtVector2(0.0f, 1.0f) );
	m_vecTime.push_back( EtVector2(0.0f, 0.0f) );
	m_vecTime.push_back( EtVector2(1.0f, 0.0f) );
	m_vecTime.push_back( EtVector2(1.0f, 1.0f) );

	//CopyVertexBuffer();

/*	
	if( !m_vecPosition.empty() )
	{
		int i;
		float fCurTime, fLastTime;

		fLastTime = m_vecTime[ m_vecTime.size() - 1 ].x;	
		fCurTime = m_fElapsedTime;
if( ( fCurTime - fLastTime ) < 0.01666f ) // Skip points that arrive faster than 1/60th of a second
		{
			return;
		}

		int nLastIndex, nInterpolation;
		float fLength, fElapsedTime;
		EtVector3 Tangent;

		nLastIndex = ( int )m_vecPosition.size() - 2;
		Tangent = Point - m_vecPosition[ nLastIndex ];
		fLength = EtVec3Length( &Tangent );
		m_vecTangent[ nLastIndex ] = Tangent;
		m_vecTangent[ nLastIndex + 1 ] = Tangent;
		nInterpolation = min( ( int )( fLength / m_fMinSegment ) + 1, 5 );
		fElapsedTime = fCurTime - fLastTime;

		m_vecPosition.resize( m_vecPosition.size() + nInterpolation * 2 );
		m_vecTangent.resize( m_vecPosition.size() );
		m_vecTime.resize( m_vecPosition.size() );

		for( i = 1; i <= nInterpolation; i++ )
		{
			int nCurIndex;
			float fWeight;

			fWeight = i / ( float )nInterpolation;
			nCurIndex = nLastIndex + i * 2;
			EtVec3Hermite( &m_vecPosition[ nCurIndex ], &m_vecPosition[ nLastIndex ], &m_vecTangent[ nLastIndex ], &Point, &Tangent, fWeight );
			m_vecPosition[ nCurIndex + 1 ] = m_vecPosition[ nCurIndex ];

			m_vecTangent[ nCurIndex ] = Tangent;
			m_vecTangent[ nCurIndex + 1 ] = Tangent;

			m_vecTime[ nCurIndex ].x = m_vecTime[ nLastIndex ].x + fElapsedTime * fWeight;
			m_vecTime[ nCurIndex + 1 ].x = m_vecTime[ nCurIndex ].x;
			m_vecTime[ nCurIndex ].y = 0.0f;
			m_vecTime[ nCurIndex + 1 ].y = 1.0f;
		}
		m_bModify = true;
	}
	else
	{
		EtVector2 Time;
		m_fElapsedTime = 0.f;

		m_vecPosition.push_back( Point );
		m_vecPosition.push_back( Point );
		m_vecTangent.push_back( Point );
		m_vecTangent.push_back( Point );
		Time.x = 0.0f;
		Time.y = 0.0f;
		m_vecTime.push_back( Time );
		Time.y = 1.0f;
		m_vecTime.push_back( Time );
	}
*/
}

void CEtLineEffect::ModifyStartPoint( EtVector3 &start )
{
	m_vecPosition.at( 0 ) = start;
	m_vecPosition.at( 1 ) = start;
}

void CEtLineEffect::ModifyEndPoint( EtVector3 &end )
{
	m_vecPosition.at( 2 ) = end;
	m_vecPosition.at( 3 ) = end;
}

void CEtLineEffect::Render( float fElapsedTime )
{
	m_fElapsedTime += fElapsedTime;
	if( !m_bShow || m_vecPosition.empty() ) {
		return;
	}

	//Handle UV scrolling
	//Emissive processing

	EtVector3 vertices[4];
	EtVector2 * texcoord[4];
	EtVector3 position, view_vector;
	float lineWidth;

	EtMatrix view_mat = *(CEtCamera::GetActiveCamera()->GetViewMat());
	EtMatrixInverse(&view_mat, NULL, &view_mat);
	view_vector = EtVector3( view_mat._41, view_mat._42, view_mat._43 );

	for( int itr = 0; itr < MAX_LINE_VERTEX; ++itr )
	{
		position = view_vector - m_vecPosition[itr];
		EtVec3Cross(&position, &position, &m_vecTangent[0]);
		EtVec3Normalize(&position, &position);
		lineWidth = (m_vecTime[itr].y - 0.5f) * m_fLineWidth;
		vertices[itr] = m_vecPosition[itr] + (position * lineWidth);

		m_vecTime[itr].x += (fElapsedTime * m_fUMoveSpeed);

		texcoord[itr] = &m_vecTime[itr];
	}

	CEtPrimitiveDraw::GetInstance().DrawQuad3DWithTex( vertices, *texcoord, m_hTexture.GetPointer()->GetTexturePtr() );

	/*int i, nEraseEnd;
	float fCurTime = m_fElapsedTime;
	nEraseEnd = -1;
	for( i = 0; i < ( int )m_vecTime.size(); i += 2 )
	{
		if( fCurTime - m_vecTime[ i ].x > m_fLifeTime )
		{
			nEraseEnd = i + 2;
		}
		else
		{
			break;
		}
	}
	if( nEraseEnd != -1 )
	{
		m_vecPosition.erase( m_vecPosition.begin(), m_vecPosition.begin() + nEraseEnd );
		m_vecTangent.erase( m_vecTangent.begin(), m_vecTangent.begin() + nEraseEnd );
		m_vecTime.erase( m_vecTime.begin(), m_vecTime.begin() + nEraseEnd );
		m_bModify = true;
	}
	if( m_vecPosition.size() <= 2 )
	{
		return;
	}

	if( m_bModify )
	{
		CopyVertexBuffer();
		m_bModify = false;
	}
	m_fCurrentStartTime = m_vecTime[ 0 ].x;
	m_fInverseLifeTime = 1.0f / ( m_vecTime[ m_vecTime.size() - 1 ].x - m_fCurrentStartTime );

	SRenderStackElement RenderElement;
	EtMatrix WorldMat;

	EtMatrixIdentity( &WorldMat );
	RenderElement.hMaterial = s_hMaterial;
	RenderElement.nTechniqueIndex = 0;
	RenderElement.WorldMat = WorldMat;
	RenderElement.nSaveMatIndex = -1;
	RenderElement.pvecCustomParam = &m_vecCustomParam;
	RenderElement.pRenderMeshStream = &m_MeshStream;
	RenderElement.nDrawCount = ( int )m_vecPosition.size() - 2;
	RenderElement.nBakeDepthIndex = DT_NONE;
	GetCurRenderStack()->AddAlphaRenderElement( RenderElement );*/
}

void CEtLineEffect::InitializeEffect()
{
	//s_hMaterial = LoadResource( "Line.fx", RT_SHADER );
	if( s_pCommonIndexBuffer == NULL )
	{
		int i, nQuadCount, nFaceCount;
		WORD *pIndexBuffer;
		CMemoryStream Stream;

		nFaceCount = ( MAX_LINE_VERTEX - 2 );
		nQuadCount = MAX_LINE_VERTEX / 2 - 1;
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

void CEtLineEffect::FinializeEffect()
{
	//SAFE_RELEASE_SPTR( s_hMaterial );
	SAFE_RELEASE( s_pCommonIndexBuffer );
}
