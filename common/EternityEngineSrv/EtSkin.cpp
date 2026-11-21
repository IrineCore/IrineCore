#include "stdafx.h"
#include "EtSkin.h"
#include "EtMesh.h"
#include "EtLoader.h"
#include "EtWrapper.h"

CEtSkin::CEtSkin( CMultiRoom *pRoom )
: CEtResource( pRoom )
{
}

CEtSkin::~CEtSkin(void)
{
	Clear();
}

void CEtSkin::Clear()
{
	SAFE_RELEASE_SPTR( m_MeshHandle );
}

int CEtSkin::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Skin )" );
	if( pStream == NULL )
	{
		return ETERR_INVALIDRESOURCESTREAM;
	}

	int i, j, nRet;

	Clear();
	pStream->Read( &m_SkinHeader, sizeof( SSkinFileHeader ) );
	pStream->Seek( SKIN_HEADER_RESERVED, SEEK_CUR );

	nRet = LoadMesh( m_SkinHeader.szMeshName );
	if( nRet != ET_OK )
	{
		return nRet;
	}

	for( i = 0; i < m_SkinHeader.nSubMeshCount; i++ )
	{
		int nParamCount, nNameLength;
		EffectParamType ParamType;

		pStream->Seek( 1024, SEEK_CUR );
		pStream->Read( &nParamCount, sizeof( int ) );
		for( j = 0; j < nParamCount; j++ )
		{
			pStream->Read( &nNameLength, sizeof( int ) );
			pStream->Seek( nNameLength, SEEK_CUR );
			pStream->Read( &ParamType, sizeof( EffectParamType ) );
			switch( ParamType )
			{
				case EPT_INT:
					pStream->Seek( sizeof( int ), SEEK_CUR );
					break;
				case EPT_FLOAT:
					pStream->Seek( sizeof( float ), SEEK_CUR );
					break;
				case EPT_VECTOR:
					pStream->Seek( sizeof( float ) * 4, SEEK_CUR );
					break;
				case EPT_TEX:
					pStream->Read( &nNameLength, sizeof( int ) );
					pStream->Seek( nNameLength, SEEK_CUR );
					break;
			}
		}
	}

	return ET_OK;
}

int CEtSkin::LoadMesh( const char *pMeshName )
{
	m_MeshHandle = ::LoadResource( GetRoom(), pMeshName, RT_MESH );
	if( !m_MeshHandle )
	{
		m_MeshHandle = ::LoadResource( GetRoom(), pMeshName, RT_MESH );
		ASSERT( 0 && "Mesh File Not Found!!!" );
		return ETERR_MESHNOTFOUND;
	}

	return ET_OK;
}
