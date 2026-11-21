#include "StdAfx.h"
#include "EtMesh.h"
#include "EtSubMesh.h"
#include "SundriesFunc.h"

#include "EtPrimitive.h"
#include "EtCollisionPrimitive.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CEtMesh::CEtMesh()
{
	m_bLinkToAni = false;
}

CEtMesh::~CEtMesh()
{
	Clear();
}

void CEtMesh::Clear()
{
	SAFE_DELETE_PVEC( m_vecCollisionPrimitive );
	SAFE_DELETE_PVEC( m_vecSubMesh );
}

void CEtMesh::LinkToAni( EtResourceHandle hAni )
{
	if( m_bLinkToAni && hAni == m_hLinkAni )
	{
		return;
	}

	int i;
	std::vector< EtMatrix > vecInvWorldMat;

	for( i = 0; i < ( int )m_vecSubMesh.size(); i++ )
	{
		m_vecSubMesh[ i ]->LinkToAni( hAni );
	}

	if( !m_bLinkToAni ) {
		vecInvWorldMat.resize( m_vecInvWorldMat.size() );
		std::copy( m_vecInvWorldMat.begin(), m_vecInvWorldMat.end(), vecInvWorldMat.begin() );
		for( i = 0; i < ( int )m_vecInvWorldMat.size(); i++ )
		{
			int nBoneIndex;

			nBoneIndex = ( ( EtAniHandle )hAni )->GetBoneIndex( m_vecBoneName[ i ].c_str() );
			if( nBoneIndex != -1 )
			{
				if( nBoneIndex < ( int )m_vecInvWorldMat.size() ) 
				{
					m_vecInvWorldMat[ nBoneIndex ] = vecInvWorldMat[ i ];
				}
				else
				{
	//				ASSERT( 0 && "Bone count mismatch between Mesh and Ani!!! Please check" );
				}
			}
			else
			{
	//			ASSERT( 0 && "Bone count mismatch between Mesh and Ani!!! Please check" );
				EtMatrixIdentity( &m_vecInvWorldMat[ i ] );
			}
		}
		for( i = 0; i < ( int )m_vecCollisionPrimitive.size(); i++ )
		{
			if( !m_vecCollisionPrimitiveParentName[ i ].empty() )
			{
				m_vecCollisionPrimitiveParentIndex[ i ] = ( ( EtAniHandle )hAni )->GetBoneIndex( m_vecCollisionPrimitiveParentName[ i ].c_str() );
			}
		}
	}
	m_bLinkToAni = true;
	m_hLinkAni = hAni;
}

int CEtMesh::FindDummy( const char *pDummyName )
{
	int i;
	for( i = 0; i < ( int )m_vecDummyName.size(); i++ )
	{
		if( stricmp( m_vecDummyName[ i ].c_str(), pDummyName ) == 0 )
		{
			return i;
		}
	}
	return -1;
}

int CEtMesh::FindBone( const char *pBoneName )
{
	if( !m_hLinkAni )
	{
		return -1;
	}

	return ( ( EtAniHandle )m_hLinkAni )->GetBoneIndex( pBoneName );
}

int CEtMesh::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Mesh )" );
	if( pStream == NULL )
	{
		return ETERR_INVALIDRESOURCESTREAM;
	}

	int i;
	CEtSubMesh *pSubMesh;

	Clear();
	pStream->Read( &m_MeshHeader, sizeof( SMeshFileHeader ) );
	pStream->Seek( MESH_HEADER_RESERVED, SEEK_CUR );	

	if( strstr( m_MeshHeader.szHeaderString, MESH_FILE_STRING ) == NULL ) return ETERR_FILENOTFOUND;
	
	if( m_MeshHeader.nBoneCount )
	{
		m_vecInvWorldMat.resize( m_MeshHeader.nBoneCount );
		m_vecBoneName.resize( m_MeshHeader.nBoneCount );
		for( i = 0; i < m_MeshHeader.nBoneCount; i++ )
		{
			char szBoneName[ 256 ];
			pStream->Read( szBoneName, 256 );
			m_vecBoneName[ i ] = szBoneName;
			pStream->Read( &m_vecInvWorldMat[ i ], sizeof( EtMatrix ) );
		}
	}

	for( i = 0; i < m_MeshHeader.nSubMeshCount; i++ )
	{
		pSubMesh = new CEtSubMesh();
		pSubMesh->LoadSubMesh( pStream );
		m_vecSubMesh.push_back( pSubMesh );
	}

	m_vecCollisionPrimitiveParentIndex.resize( m_MeshHeader.nCollisionPrimitiveCount );
	for( i = 0; i < m_MeshHeader.nCollisionPrimitiveCount; i++ )
	{
		CollisionType Type;
		std::string szParentName;

		pStream->Read( &Type, sizeof( CollisionType ) );
		if( m_MeshHeader.nVersion > 10 )
		{
			ReadStdString( szParentName, pStream );
			if( strcmp( szParentName.c_str(), "Scene Root" ) == 0 )
			{
				szParentName = "";
			}
		}
		m_vecCollisionPrimitiveParentIndex[ i ] = -1;

		switch( Type )
		{
			case CT_BOX:
				{
					SCollisionBox *pBox;

					pBox = new SCollisionBox();
					pBox->Load( pStream );
					EtVec3Cross( pBox->vAxis, pBox->vAxis + 1, pBox->vAxis + 2 );
					EtVec3Cross( pBox->vAxis + 1, pBox->vAxis + 2, pBox->vAxis );
					EtVec3Cross( pBox->vAxis + 2, pBox->vAxis, pBox->vAxis + 1 );
					m_vecCollisionPrimitive.push_back( pBox );
					m_vecCollisionPrimitiveParentName.push_back( szParentName );
				}
				break;
			case CT_SPHERE:
				{
					SCollisionSphere *pSphere;

					pSphere = new SCollisionSphere();
					pSphere->Load( pStream );
					m_vecCollisionPrimitive.push_back( pSphere );
					m_vecCollisionPrimitiveParentName.push_back( szParentName );
				}
				break;
			case CT_CAPSULE:
				{
					SCollisionCapsule *pCapsule;

					pCapsule = new SCollisionCapsule();
					pCapsule->Load( pStream );
					m_vecCollisionPrimitive.push_back( pCapsule );
					m_vecCollisionPrimitiveParentName.push_back( szParentName );
				}
				break;
			case CT_TRIANGLE:
			case CT_TRIANGLE_LIST:
				{
					int nCount, nLoop;
					SCollisionTriangleList *pTriangleList;

					pTriangleList = new SCollisionTriangleList();
					pStream->Read( &nCount, sizeof( int ) );
					pTriangleList->vecTriangle.resize( nCount );
					for( nLoop = 0; nLoop < nCount; nLoop++ )
					{
						pStream->Read( pTriangleList->vecTriangle[ nLoop ].vOrigin, sizeof( EtVector3 ) );
						pStream->Read( pTriangleList->vecTriangle[ nLoop ].vEdge1, sizeof( EtVector3 ) );
						pStream->Read( pTriangleList->vecTriangle[ nLoop ].vEdge2, sizeof( EtVector3 ) );
					}
					m_vecCollisionPrimitive.push_back( pTriangleList );
					m_vecCollisionPrimitiveParentName.push_back( szParentName );
				}
				break;
		}
	}
	if( m_MeshHeader.nDummyCount )
	{
		m_vecDummyName.resize( m_MeshHeader.nDummyCount );
		m_vecDummyParentName.resize( m_MeshHeader.nDummyCount );
		m_vecDummyMat.resize( m_MeshHeader.nDummyCount );
		if( m_MeshHeader.nVersion >= 13 )
		{
			for( i = 0; i < m_MeshHeader.nDummyCount; i++ )
			{
				char szDummyName[ 256 ];
				pStream->Read( szDummyName, 256 );
				m_vecDummyName[ i ] = szDummyName;
				pStream->Read( szDummyName, 256 );
				m_vecDummyParentName[ i ] = szDummyName;

				pStream->Read( &m_vecDummyMat[ i ], sizeof( EtMatrix ) );
			}
		}
		else
		{
			for( i = 0; i < m_MeshHeader.nDummyCount; i++ )
			{
				char szDummyName[ 256 ];
				pStream->Read( szDummyName, 256 );
				if( szDummyName[0] == 'L' ) {
					m_vecDummyName[ i ] = (szDummyName + 1);
					EtMatrixIdentity( &m_vecDummyMat[ i ] );
					pStream->Read( &m_vecDummyMat[ i ]._41, sizeof( EtVector3 ) );

					char szParentName[ 256 ];
					pStream->Read( szParentName, 256 );
					m_vecDummyParentName[ i ] = szParentName;
				}
				else {
					m_vecDummyName[ i ] = szDummyName;
					m_vecDummyParentName[ i ] = "";
					EtMatrixIdentity( &m_vecDummyMat[ i ] );
					pStream->Read( &m_vecDummyMat[ i ]._41, sizeof( EtVector3 ) );
				}
			}
		}
	}

	m_BoundingSphere.Center = ( m_MeshHeader.MaxVec + m_MeshHeader.MinVec ) * 0.5f;
	m_BoundingSphere.fRadius = EtVec3Length( &( m_MeshHeader.MaxVec - m_BoundingSphere.Center ) );

	return ET_OK;
}

int CEtMesh::GetSubMeshIndex( const char *pSubMeshName )
{
	int i;

	for( i = 0; i < ( int )m_vecSubMesh.size(); i++ )
	{
		if( _stricmp( pSubMeshName, m_vecSubMesh[ i ]->GetSubMeshName() ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

void CEtMesh::Draw( int nSubmeshIndex, int nVertexDeclIndex ) 
{ 
	m_vecSubMesh[ nSubmeshIndex ]->Draw( nVertexDeclIndex ); 
}

bool CEtMesh::TestLineToTriangle( EtVector3 &vPosition, EtVector3 &vDirection, EtMatrix &TransMat, float &fDist )
{
	int i;

	for( i = 0; i < ( int )m_vecSubMesh.size(); i++ )
	{
		if( m_vecSubMesh[ i ]->TestLineToTriangle( vPosition, vDirection, TransMat, fDist ) )
		{
			return true;
		}
	}

	return false;
}

void CEtMesh::GetExtent( EtVector3 &Origin, EtVector3 &Extent )
{
	Origin = ( m_MeshHeader.MaxVec + m_MeshHeader.MinVec ) / 2;
	Extent = ( m_MeshHeader.MaxVec - m_MeshHeader.MinVec ) / 2;
}

void CEtMesh::GetBoundingBox( SAABox &Box )
{
	Box.Min = m_MeshHeader.MinVec;
	Box.Max = m_MeshHeader.MaxVec;
}

EtMatrix &CEtMesh::GetDummyMatByName( const char *pName )
{
	static EtMatrix sDefaultMat;
	int i, nSize;
	nSize = (int)m_vecDummyName.size();
	for( i = 0; i < nSize; i++) {
		if(strcmp(m_vecDummyName[i].c_str(), pName) == 0 )
		{
			return m_vecDummyMat[i];
		}
	}
	return sDefaultMat;
}

EtVector3 CEtMesh::GetDummyPosByName( const char *pName )
{
	int i, nSize;
	nSize = (int)m_vecDummyName.size();
	for( i = 0; i < nSize; i++) {
		if(strcmp(m_vecDummyName[i].c_str(), pName) == 0 ) {
			return *( ( EtVector3 * )&m_vecDummyMat[i]._41 );
		}
	}
	return EtVector3(0,0,0);
}

char * CEtMesh::GetDummyParentByName( const char *pName )
{
	int i, nSize;
	nSize = (int)m_vecDummyName.size();
	for( i = 0; i < nSize; i++) {
		if(strcmp(m_vecDummyName[i].c_str(), pName) == 0 ) {
			char *pParentName = const_cast<char*>(m_vecDummyParentName[i].c_str());
			return pParentName;
		}
	}
	return NULL;
}

void CEtMesh::Assign( EtMeshHandle hMesh, EtVector4 ScaleBias )
{
	if( !hMesh ) return;

	m_MeshHeader = hMesh->m_MeshHeader;
	assert( hMesh->m_vecSubMesh.size() == 1 );
	
	CEtSubMesh *pSubMesh = new CEtSubMesh;
	pSubMesh->Assign( hMesh->m_vecSubMesh[0], ScaleBias );
	m_vecSubMesh.push_back( pSubMesh );

	m_vecBoneName = hMesh->m_vecBoneName;
	m_vecInvWorldMat = hMesh->m_vecInvWorldMat;
	m_vecDummyName = hMesh->m_vecDummyName;
	m_vecDummyMat = hMesh->m_vecDummyMat;
	m_BoundingSphere = hMesh->m_BoundingSphere;
}
