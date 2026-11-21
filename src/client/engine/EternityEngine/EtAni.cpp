#include "StdAfx.h"
#include "EtAni.h"
#include "EtBone.h"
#include "EtSaveMat.h"
#include "DebugSet.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtAni::CEtAni(void)
{
	memset( &m_AniHeader, 0, sizeof(SAniFileHeader) );
}

CEtAni::~CEtAni(void)
{
	Clear();
}

void CEtAni::Clear()
{
	SAFE_DELETE_PVEC( m_vecBone );
}

int CEtAni::GetBoneIndex( const char *pBoneName )
{
	int i;

	if( strlen( pBoneName ) <= 0 )
	{
		return -1;
	}

	if( IsReady() )
	{
		for( i = 0; i < ( int )m_vecBone.size(); i++ )
		{
			if( stricmp( pBoneName, m_vecBone[ i ]->GetName() ) == 0 )
			{
				return i;
			}
		}

		return -1;
	}
	else
	{
		for( i = 0; i < ( int )m_vecCacheBoneName.size(); i++ )
		{
			if( stricmp( pBoneName, m_vecCacheBoneName[ i ].szBoneName.c_str() ) == 0 )
			{
				return ( i + 1 ) << 16;
			}
		}
		m_vecCacheBoneName.resize( m_vecCacheBoneName.size() + 1 );
		m_vecCacheBoneName.back().szBoneName = pBoneName;
		m_vecCacheBoneName.back().nBoneIndex = -1;
		return ( ( ( int )m_vecCacheBoneName.size() ) << 16 );
	}
}

CEtBone *CEtAni::GetBone( int nIndex ) 
{ 
	if( nIndex > 65535 )
	{
		if( !IsReady() )
		{
			return NULL;
		}

		int nBoneIndex;
		nBoneIndex = ( nIndex >> 16 ) - 1;
		return m_vecBone[ m_vecCacheBoneName[ nBoneIndex ].nBoneIndex ];
	}
	else
	{
		if( nIndex < 0 || nIndex >= (int)m_vecBone.size() ) {
			ASSERT( 0 && "Requested bone index does not exist." );
			return NULL;
		}
		return m_vecBone[ nIndex ]; 
	}
}

CEtBone *CEtAni::FindBone( const char *pBoneName )
{
	if( !IsReady() )
	{
		return NULL;
	}

	int nBoneIndex;

	nBoneIndex = GetBoneIndex( pBoneName );
	if( nBoneIndex == -1 )
	{
		return NULL;
	}

	return m_vecBone[ nBoneIndex ];
}

int CEtAni::ConvertBoneIndex( int nIndex )
{
	if( nIndex > 65535 )
	{
		return m_vecCacheBoneName[ ( nIndex >> 16 ) - 1 ].nBoneIndex;
	}
	return nIndex;
}

int CEtAni::LoadAni( CStream *pStream )
{
	int i;
	CEtBone *pBone;
	char szAniName[ 256 ];

	Clear();
	pStream->Read( &m_AniHeader, sizeof( SAniFileHeader ) );
	pStream->Seek( ANI_HEADER_RESERVED, SEEK_CUR );

	if( strstr( m_AniHeader.szHeaderString, ANI_FILE_STRING ) == NULL ) return ETERR_FILENOTFOUND;

	for( i = 0; i < m_AniHeader.nAniCount; i++ )
	{
		pStream->Read( szAniName, 256 );
		m_vecAniName.push_back( szAniName );
	}

	m_vecAniLength.resize( m_AniHeader.nAniCount );
	pStream->Read( &m_vecAniLength[ 0 ], sizeof( int ) * m_AniHeader.nAniCount );

	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		pBone = new CEtBone();
		pBone->LoadBone( pStream, m_AniHeader.nAniCount, m_AniHeader.nVersion );
		m_vecBone.push_back( pBone );
		pBone->SetBoneIndex( i );
	}

	BuildHierarchy();
	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		if( m_vecBone[ i ]->GetParent() == NULL )
		{
			m_vecBone[ i ]->SetRootBone( true );
		}
	}

	SetReady( true );
	for( i = 0; i < ( int )m_vecCacheBoneName.size(); i++ )
	{
		m_vecCacheBoneName[ i ].nBoneIndex = GetBoneIndex( m_vecCacheBoneName[ i ].szBoneName.c_str() );
	}

	return ET_OK;
}

int CEtAni::LoadMultiAni( CStream *pStream )
{
	int nSize = pStream->Size();
	char *pBuffer = new char[ nSize + 1 ];

	memset( pBuffer, 0, nSize + 1 );
	pStream->Read( pBuffer, nSize );

	char *pToken = strtok( pBuffer, "\n\r" );
	while( pToken )
	{
		if( m_vecBone.size() )
		{
			EtAniHandle hLoadAni = ::LoadResource( pToken, RT_ANI, false );
			if( hLoadAni )
			{
				MergeAni( hLoadAni );
				hLoadAni->SetDeleteImmediate( true );
				SAFE_RELEASE_SPTR( hLoadAni );
			}
		}
		else
		{
			CResMngStream *pMainResource = dynamic_cast< CResMngStream * >( pStream );
			if( pMainResource )
			{
				char szFileName[ _MAX_PATH ] = { 0 };
				const char *szOriginalPath = pMainResource->GetAddPath();
				int nLen = (int)strlen(szOriginalPath);
				if( nLen > 0 ) {
					strcpy_s( szFileName, _MAX_PATH, szOriginalPath );
					strcat_s( szFileName, _MAX_PATH, "\\" );
				}
				strcat_s( szFileName, _MAX_PATH, pToken );

				CResMngStream *pSubAniStream = new CResMngStream( szFileName, true, true );
				if( pSubAniStream )
				{
					if( pSubAniStream->IsValid() )
					{
						LoadAni( pSubAniStream );
					}
					delete pSubAniStream;
				}
			}
		}
		pToken = strtok( NULL, "\n\r" );
	}

	SAFE_DELETEA( pBuffer );

	return ET_OK;
}

int CEtAni::LoadResource( CStream *pStream )
{
	ASSERT( pStream && "Invalid Resource Stream( Animation )" );
	if( pStream == NULL )
	{
		return ETERR_INVALIDRESOURCESTREAM;
	}

	char szHeaderString[ 256 ];
	bool bMultiAni = false;
	if( pStream->Size() < ANI_HEADER_RESERVED + sizeof( SAniFileHeader ) )
	{
		bMultiAni = true;
	}
	else
	{
		pStream->Read( szHeaderString, 256 );
		if( strstr( szHeaderString, ANI_FILE_STRING ) == NULL )
		{
			bMultiAni = true;
		}
	}
	if( bMultiAni )
	{
		return LoadMultiAni( pStream );
	}
	else
	{
		pStream->Seek( 0, SEEK_SET );
		return LoadAni( pStream );
	}
}

void CEtAni::BuildHierarchy()
{
	int i, j;

	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		for( j = 0; j < m_AniHeader.nBoneCount; j++ )
		{
			const char *pParentName;
			pParentName = m_vecBone[ i ]->GetParentName();
			if( stricmp( pParentName, m_vecBone[ j ]->GetName() ) == 0 )
			{
				m_vecBone[ i ]->SetParent( m_vecBone[ j ] );
				m_vecBone[ j ]->AddChild( m_vecBone[ i ] );
				break;
			}
		}
	}
}

void CEtAni::Reset()
{
	int i;

	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		m_vecBone[ i ]->Reset();
	}
}

float CEtAni::CalcCorrectFrame( int nAni, float fFrame )
{
	if( nAni < 0 || nAni >= m_AniHeader.nAniCount || fFrame < 0.0f )
	{
		ASSERT( 0 && "Invalid Ani Frame!!!( CEtAni::CalcCorrectFrame )" );
		return 0.0f;
	}

	/*
	fFrame = fmod( fFrame, ( float )GetAniLength( nAni ) );
	if( fFrame > GetLastFrame( nAni ) )
	{
		fFrame -= GetLastFrame( nAni );
	}
	*/
	if( fFrame > ( float )GetAniLength( nAni ) )
		fFrame = ( float )GetAniLength( nAni );
	return fFrame;
}

void CEtAni::SetAni( SBoneAniInfo *pAniInfo, int nBoneIndex )
{
	if( IsValidBoneIndex( nBoneIndex ) )
	{
		CEtBone *pBone;

		pAniInfo->fFrame = CalcCorrectFrame( pAniInfo->nAni, pAniInfo->fFrame );

		if( nBoneIndex == 0 )
		{
			int i;

			for( i = 0; i < m_AniHeader.nBoneCount; i++ )
			{
				if( m_vecBone[ i ]->IsRootBone() )
				{
					m_vecBone[ i ]->SetAni( pAniInfo );
				}
			}
		}
		else
		{
			pBone = GetBone( nBoneIndex );
			if( pBone == NULL )
			{
				ASSERT( 0 && "Invalid Bone Index" );
				return;
			}
			pBone->SetAni( pAniInfo );
		}
	}
}

void CEtAni::EnableBlend( int nBoneIndex, bool bEnable )
{
	if( IsValidBoneIndex( nBoneIndex ) )
	{
		m_vecBone[ nBoneIndex ]->EnableBlend( bEnable ); 
	}
}

void CEtAni::BlendAni( SBoneAniInfo *pAniInfo, int nBoneIndex )
{
	if( IsValidBoneIndex( nBoneIndex ) )
	{
		CEtBone *pBone;

		pAniInfo->fFrame = CalcCorrectFrame( pAniInfo->nAni, pAniInfo->fFrame );

		if( nBoneIndex == 0 )
		{
			int i;

			for( i = 0; i < m_AniHeader.nBoneCount; i++ )
			{
				if( m_vecBone[ i ]->IsRootBone() )
				{
					m_vecBone[ i ]->BlendAni( pAniInfo );
				}
			}
		}
		else
		{
			pBone = GetBone( nBoneIndex );
			if( pBone == NULL )
			{
				ASSERT( 0 && "Invalid Bone Index" );
				return;
			}
			pBone->BlendAni( pAniInfo );
		}
	}
}

int CEtAni::CalcAni( std::vector< EtMatrix > &vecInvWorldMat )
{
	if( vecInvWorldMat.empty() )
	{
		return -1;
	}

	if( m_AniHeader.nBoneCount == 0 )
	{
		return -1;
	}

	int i, nSaveMatStart;
	EtMatrix VertexTransMat;

	EtMatrix *pLockedMatrices = NULL;
	nSaveMatStart = GetEtSaveMat()->LockMatrix( m_AniHeader.nBoneCount, &pLockedMatrices );
	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		if( i >= ( int )vecInvWorldMat.size() || i >= (int) m_vecBone.size() )
		{
			break;
		}
		m_vecBone[ i ]->CalcAni();
		EtMatrixMultiply( &VertexTransMat, &vecInvWorldMat[ i ], m_vecBone[ i ]->GetTransMat() );
		pLockedMatrices[ i ] = VertexTransMat;
		m_vecBone[ i ]->SetBoneRotation( NULL );
		m_vecBone[ i ]->SetBoneScale( 1.0f );
	}

	return nSaveMatStart;
}

void CEtAni::CalcAniDistance( int nAni, float fCurFrame, float fPrevFrame, EtVector3 &DistVec )
{
	fCurFrame = CalcCorrectFrame( nAni, fCurFrame );
	fPrevFrame = CalcCorrectFrame( nAni, fPrevFrame );

	if( m_vecBone.empty() )
	{
		DistVec = EtVector3( 0.0f, 0.0f, 0.0f );
	}
	else
	{
		m_vecBone[ 0 ]->CalcAniDistance( nAni, fCurFrame, fPrevFrame, DistVec );
	}
}

void CEtAni::SetCalcPositionFlag( int nFlag )
{
	CEtBone::SetCalcPositionFlag( nFlag );
}

void CEtAni::CalcRootBoneMat( int nAni, float fFrame, EtMatrix &BoneMat )
{
	if( m_vecBone.empty() )
	{
		EtMatrixIdentity( &BoneMat );
	}
	else
	{
		m_vecBone[ 0 ]->GetAniRotationMatrix( nAni, fFrame, BoneMat );
	}
}

void CEtAni::SetBoneRotation( int nBoneIndex, EtVector3 *pBoneRotation )
{
	nBoneIndex = ConvertBoneIndex( nBoneIndex );
	if( IsValidBoneIndex( nBoneIndex ) )
	{
		m_vecBone[ nBoneIndex ]->SetBoneRotation( pBoneRotation );
	}
}

void CEtAni::SetBoneScale( int nBoneIndex, float fScale )
{
	nBoneIndex = ConvertBoneIndex( nBoneIndex );
	if( IsValidBoneIndex( nBoneIndex ) )
	{
		m_vecBone[ nBoneIndex ]->SetBoneScale( fScale );
	}
}

int CEtAni::Save( const char *pFileName )
{
	CFileStream Stream( pFileName, CFileStream::OPEN_WRITE );

	if( !Stream.IsValid() )
	{
		return ETERR_FILECREATEFAIL;
	}

	SaveAni( &Stream );

	return ET_OK;
}

int CEtAni::SaveAni( CStream *pStream )
{
	int i;
	char cReserved[ ANI_HEADER_RESERVED ];
	char szAniName[ 256 ];

	m_AniHeader.nVersion = ANI_FILE_VERSION;
	pStream->Write( &m_AniHeader, sizeof( SAniFileHeader ) );
	memset( cReserved, 0, ANI_HEADER_RESERVED );
	pStream->Write( cReserved, ANI_HEADER_RESERVED );
	for( i = 0; i < m_AniHeader.nAniCount; i++ )
	{
		memset( szAniName, 0, 256 );
		strcpy( szAniName, m_vecAniName[ i ].c_str() );
		pStream->Write( szAniName, 256 );
	}
	pStream->Write( &m_vecAniLength[ 0 ], sizeof( int ) * m_AniHeader.nAniCount );
	for( i = 0; i < m_AniHeader.nBoneCount; i++ )
	{
		m_vecBone[ i ]->SaveBone( pStream, m_AniHeader.nAniCount );
	}

	return ET_OK;
}

int CEtAni::MergeAni( CEtAni *pAni )
{
	int i, nBoneCount;

	if( pAni->GetBoneCount() != GetBoneCount() )
	{
		return ETERR_DIFFERENTBONECOUNT;
	}

	for( i = 0; i < pAni->GetAniCount(); i++ )
	{
		m_vecAniName.push_back( pAni->GetAniName( i ) );
		m_vecAniLength.push_back( pAni->GetAniLength( i ) );
	}

	nBoneCount = pAni->GetBoneCount();
	for( i = 0; i < nBoneCount; i++)
	{
		m_vecBone[ i ]->MergeBone( pAni->FindBone( m_vecBone[ i ]->GetName() ), pAni->GetAniCount() );
	}
	m_AniHeader.nAniCount += pAni->GetAniCount();

	return ET_OK;
}
