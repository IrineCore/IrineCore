#include "stdafx.h"
#include "EtMesh.h"
#include "EtSubMesh.h"
#include "SundriesFunc.h"

CEtMesh::CEtMesh( CMultiRoom *pRoom )
: CEtResource( pRoom )
{
}

CEtMesh::~CEtMesh()
{
	Clear();
}

void CEtMesh::Clear()
{
	SAFE_DELETE_PVEC( m_vecCollisionPrimitive );
}

bool CEtMesh::HasCollisionBone()
{
	int i;

	for( i = 0; i < ( int )m_vecCollisionPrimitiveParentIndex.size(); i++ )
	{
		if( m_vecCollisionPrimitiveParentIndex[ i ] != -1 )
		{
			return true;
		}
	}

	return false;
}

void CEtMesh::LinkToAni( EtResourceHandle hAni )
{
	int i;
	std::vector< EtMatrix > vecInvWorldMat;

	vecInvWorldMat.resize( m_vecInvWorldMat.size() );
	std::copy( m_vecInvWorldMat.begin(), m_vecInvWorldMat.end(), vecInvWorldMat.begin() );
	for( i = 0; i < ( int )m_vecInvWorldMat.size(); i++ )
	{
		int nBoneIndex;

		nBoneIndex = ( ( EtAniHandle )hAni )->GetBoneIndex( m_vecBoneName[ i ].c_str() );
		if( nBoneIndex != -1 )
		{
			// Temp siva
			if( nBoneIndex >= (int)m_vecInvWorldMat.size() ) return;
			m_vecInvWorldMat[ nBoneIndex ] = vecInvWorldMat[ i ];
		}
		else
		{
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

int CEtMesh::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Mesh )" );
	if( pStream == NULL )
	{
		return ETERR_INVALIDRESOURCESTREAM;
	}

	int i;
	CEtSubMesh SubMesh;

	Clear();
	memset( &m_MeshHeader, 0, sizeof( SMeshFileHeader ) );
	pStream->Read( &m_MeshHeader, sizeof( SMeshFileHeader ) );
	pStream->Seek( MESH_HEADER_RESERVED, SEEK_CUR );
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
		SubMesh.LoadSubMesh( pStream );
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
		for( i = 0; i < m_MeshHeader.nDummyCount; i++ )
		{
			pStream->Seek( 256 , SEEK_CUR );
			pStream->Seek( sizeof( EtVector3 ), SEEK_CUR );
		}
	}

	m_BoundingSphere.Center = ( m_MeshHeader.MaxVec + m_MeshHeader.MinVec ) * 0.5f;
	m_BoundingSphere.fRadius = EtVec3Length( &( m_MeshHeader.MaxVec - m_BoundingSphere.Center ) );

	return ET_OK;
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
