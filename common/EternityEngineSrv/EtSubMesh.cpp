#include "stdafx.h"
#include "EtSubMesh.h"
#include "EtAni.h"

CEtSubMesh::CEtSubMesh(void)
{
}

CEtSubMesh::~CEtSubMesh(void)
{
}

void CEtSubMesh::LoadSubMesh( CStream *pStream )
{
	int i;
	SSubMeshHeader SubMeshHeader;

	pStream->Read( &SubMeshHeader, sizeof( SSubMeshHeader ) );
	pStream->Seek( SUBMESH_HEADER_RESERVED, SEEK_CUR );

	pStream->Seek( sizeof( WORD ) * SubMeshHeader.nFaceBufferSize, SEEK_CUR );
	pStream->Seek( sizeof( float ) * 3 * SubMeshHeader.nVertexCount, SEEK_CUR );
	pStream->Seek( sizeof( float ) * 3 * SubMeshHeader.nVertexCount, SEEK_CUR );

	for( i = 0; i < SubMeshHeader.nTextureChannel; i++ )
	{
		pStream->Seek( sizeof( float ) * 2 * SubMeshHeader.nVertexCount, SEEK_CUR );
	}
	if( SubMeshHeader.bUseVertexColor )
	{
		pStream->Seek( sizeof( DWORD ) * SubMeshHeader.nVertexCount, SEEK_CUR );
	}
	if( SubMeshHeader.bUsePhysique )
	{
		pStream->Seek( sizeof( short ) * 4 * SubMeshHeader.nVertexCount, SEEK_CUR );
		pStream->Seek( sizeof( float ) * 4 * SubMeshHeader.nVertexCount, SEEK_CUR );

		int nBoneCount;

		pStream->Read( &nBoneCount, sizeof( int ) );
		pStream->Seek( 256 * nBoneCount, SEEK_CUR );
	}
}
