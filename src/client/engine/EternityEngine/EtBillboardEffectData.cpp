#include "StdAfx.h"
#include "EtBillboardEffectData.h"

EtBlendOP g_nBlendOPList[] = 
{
	BLENDOP_ADD,
	BLENDOP_SUBTRACT,
	BLENDOP_REVSUBTRACT,
	BLENDOP_MIN,
	BLENDOP_MAX,
};

EtBlendMode g_nBlendModeList[] = 
{
	BLEND_ZERO,
	BLEND_ONE,             
	BLEND_SRCCOLOR,        
	BLEND_INVSRCCOLOR,     
	BLEND_SRCALPHA,
	BLEND_INVSRCALPHA,
	BLEND_DESTALPHA,       
	BLEND_INVDESTALPHA,    
	BLEND_DESTCOLOR,       
	BLEND_INVDESTCOLOR,    
	BLEND_SRCALPHASAT,     
};

CEtBillboardEffectData::CEtBillboardEffectData()
{
	m_nTotalEffectCount = 0;
	m_nMaxBillboardEffectLife = 0;	
	m_bAlphaFog = false;
}

CEtBillboardEffectData::~CEtBillboardEffectData()
{
	Clear();
}

void CEtBillboardEffectData::Clear()
{
	SAFE_RELEASE_SPTR( m_hMaterial );
	SAFE_DELETE_PVEC( m_vecEmitter );
	SAFE_DELETE_PVEC( m_InstancePool );
}

int CEtBillboardEffectData::LoadResource( CStream *pStream )
{
	SBillboardEffectHeader Header;

	Clear();
	m_hMaterial = ::LoadResource( "Particle.fx", RT_SHADER );

	pStream->Read( &Header, sizeof(SBillboardEffectHeader) );
	assert( strstr(Header.szHeader, "Eternity Engine Particle File") );

	if( strstr(Header.szHeader, BILLBOARD_EFFECT_HEADER) == NULL ) return ETERR_FILENOTFOUND;

	pStream->Seek( BILLBOARD_EFFECT_HEADER_RESERVED, SEEK_CUR );
	m_vecEmitter.resize( Header.nEmitterCount );
	for( int i = 0; i < Header.nEmitterCount; i++ )
	{
		m_vecEmitter[ i ] = new CEtBillboardEffectEmitter();
		m_vecEmitter[ i ]->LoadEmitter( pStream, Header.nVersion );
	}
	CreateBillboardEffectBuffer();

	return ET_OK;
}

int CEtBillboardEffectData::Save( const char *pFileName )
{
	CFileStream Stream( pFileName, CFileStream::OPEN_WRITE );

	if( !Stream.IsValid() )
	{
		return ETERR_FILECREATEFAIL;
	}

	int i;
	SBillboardEffectHeader Header;
	char cDummy[ BILLBOARD_EFFECT_HEADER_RESERVED ];

	memset( &Header, 0, sizeof( SBillboardEffectHeader ) );
	Header.nVersion = EMITTER_VER;
	Header.nEmitterCount = ( int )m_vecEmitter.size();
	//strcpy( Header.szHeader, BILLBOARD_EFFECT_HEADER );
	strncpy_s(Header.szHeader, _countof(Header.szHeader), BILLBOARD_EFFECT_HEADER, _TRUNCATE);
	Stream.Write( &Header, sizeof( SBillboardEffectHeader ) );
	memset( cDummy, 0, BILLBOARD_EFFECT_HEADER_RESERVED );
	Stream.Write( cDummy, BILLBOARD_EFFECT_HEADER_RESERVED );

	for( i = 0; i < Header.nEmitterCount; i++ )
	{
		m_vecEmitter[ i ]->SaveEmitter( &Stream );
	}

	return ET_OK;
}

void CEtBillboardEffectData::CreateBillboardEffectBuffer()
{
	int i;

	CheckLifeTime();
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	for( i = 0; i < ( int )m_vecEmitter.size(); i++ )
	{
		m_vecEmitter[ i ]->CreateEmitterBuffer();
		m_vecEmitter[ i ]->SetEffectParam( m_hMaterial );
	}
}

void CEtBillboardEffectData::CheckLifeTime()
{
	int i;
	m_nMaxBillboardEffectLife = 0;
	m_nTotalEffectCount = 0;
	for( i = 0; i < ( int )m_vecEmitter.size(); i++ )
	{
		m_nTotalEffectCount += m_vecEmitter[ i ]->GetGenerateCount();
		if( m_nMaxBillboardEffectLife < m_vecEmitter[ i ]->GetLifeDuration() )
		{
			m_nMaxBillboardEffectLife = m_vecEmitter[ i ]->GetLifeDuration();
		}
	}

	for( i = 0; i < ( int )m_vecEmitter.size(); i++ )
	{
		m_vecEmitter[ i ]->SetMaxEffectLifeTime( m_nMaxBillboardEffectLife );
	}
}

int CEtBillboardEffectData::AddEmitter( SEmitter *pEmitterInfo )
{
	CEtBillboardEffectEmitter *pEmitter;

	pEmitter = new CEtBillboardEffectEmitter();
	m_vecEmitter.push_back( pEmitter );
	if( pEmitterInfo )
	{
		pEmitter->SetEmitterInfo( pEmitterInfo );
		pEmitter->LoadTexture( pEmitterInfo->szTextureName );
	}
	CheckLifeTime();
	CreateBillboardEffectBuffer();

	return ( int )m_vecEmitter.size() - 1;
}

int CEtBillboardEffectData::CopyEmitter( int nIndex )
{
	AddEmitter();
	( *m_vecEmitter.end() )->SetEmitterInfo( m_vecEmitter[ nIndex ]->GetEmitterInfo() );
	CreateBillboardEffectBuffer();

	return ( int )m_vecEmitter.size() - 1;
}

void CEtBillboardEffectData::DeleteEmitter( int nIndex )
{
	if( nIndex < ( int )m_vecEmitter.size() )
	{
		delete m_vecEmitter[ nIndex ];
		m_vecEmitter.erase( m_vecEmitter.begin() + nIndex );
	}
}

void CEtBillboardEffectData::ChangeEmitter( int nIndex1, int nIndex2 )
{
	int nSize = (int)m_vecEmitter.size();
	if( nIndex1 < 0 || nIndex1 >= nSize ) return;
	if( nIndex2 < 0 || nIndex2 >= nSize ) return;
	CEtBillboardEffectEmitter *pEmitter = m_vecEmitter[nIndex1];
	m_vecEmitter[nIndex1] = m_vecEmitter[nIndex2];
	m_vecEmitter[nIndex2] = pEmitter;
}

void CEtBillboardEffectData::LoadTexture( int nIndex, const char *pFileName ) 
{
	m_vecEmitter[ nIndex ]->LoadTexture( pFileName );
	m_vecEmitter[ nIndex ]->SetEffectParam( m_hMaterial );
}

CEtBillboardEffectDataInstance* CEtBillboardEffectData::GetInstance()
{
	if( m_InstanceFreeSlots.empty() )
	{
		CEtBillboardEffectDataInstance *pDataInstance = new CEtBillboardEffectDataInstance;
		pDataInstance->CopyDataInfo( GetMySmartPtr() );
		m_InstancePool.push_back( pDataInstance );
		return pDataInstance;
	}
	else
	{
		int nIndex = m_InstanceFreeSlots.back();
		m_InstanceFreeSlots.pop_back();
		m_InstancePool[ nIndex ]->Reset();
		return m_InstancePool[ nIndex ];
	}
}

void CEtBillboardEffectData::ReleaseInstance( CEtBillboardEffectDataInstance *pInstance )
{
	if( pInstance == NULL )
		return;

	int i, nSize;
	nSize = (int)m_InstancePool.size();
	for( i = 0; i < nSize; i++ )
	{
		if( m_InstancePool[ i ] == pInstance )
		{
			m_InstanceFreeSlots.push_back( i );
			break;
		}
	}
}


CEtBillboardEffectDataInstance::CEtBillboardEffectDataInstance()
{
	Reset();
}

CEtBillboardEffectDataInstance::~CEtBillboardEffectDataInstance()
{
}

void CEtBillboardEffectDataInstance::Reset()
{
	m_bAlphaFog = true;
	m_pTracePos = NULL;
}

void CEtBillboardEffectDataInstance::CopyDataInfo( EtBillboardEffectDataHandle hData )
{
	int i;

	m_hData = hData;
	m_vecCustomParams.resize( hData->GetEmitterCount() );
	m_vecRenderInfo.resize( hData->GetEmitterCount() );
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !hData->m_hMaterial ) return;
#endif
	for( i = 0; i < ( int )m_vecCustomParams.size(); i++ )
	{
		m_vecCustomParams[ i ] = hData->m_vecEmitter[ i ]->m_vecCustomParam;
		AddCustomParam( m_vecCustomParams[ i ], EPT_FLOAT_PTR, hData->m_hMaterial, "g_fParticleTime", &m_vecRenderInfo[ i ].fTime );
		AddCustomParam( m_vecCustomParams[ i ], EPT_VECTOR_PTR, hData->m_hMaterial, "g_Origin", &m_vecRenderInfo[ i ].Origin );
		AddCustomParam( m_vecCustomParams[ i ], EPT_FLOAT_PTR, hData->m_hMaterial, "g_fGravityAccel", &m_vecRenderInfo[ i ].fGravitySpeed );
		AddCustomParam( m_vecCustomParams[ i ], EPT_VECTOR_PTR, hData->m_hMaterial, "g_GravityVecView", &m_vecRenderInfo[ i ].vGravity );
		AddCustomParam( m_vecCustomParams[ i ], EPT_VECTOR_PTR, hData->m_hMaterial, "g_fParticleColor", &m_vecRenderInfo[ i ].Color );
		AddCustomParam( m_vecCustomParams[ i ], EPT_FLOAT_PTR, hData->m_hMaterial, "g_fParticleScale", &m_vecRenderInfo[ i ].fScale );
		AddCustomParam( m_vecCustomParams[ i ], EPT_MATRIX_PTR, hData->m_hMaterial, "g_BillBoardMat", &m_vecRenderInfo[ i ].BillBoardMatrix );
		AddCustomParam( m_vecCustomParams[ i ], EPT_VECTOR_PTR, hData->m_hMaterial, "g_IteratePos", NULL, TRACE_POS_COUNT );
	}
}

void CEtBillboardEffectDataInstance::SetTracePosCustomParam( std::vector< EtVector4 > &vecTracePos )
{
	if( !vecTracePos.empty() )
	{
		if( m_pTracePos != &vecTracePos[ 0 ] )
		{
			m_pTracePos = &vecTracePos[ 0 ];

			if( vecTracePos.size() < TRACE_POS_COUNT )
				vecTracePos.resize( TRACE_POS_COUNT );

			for( int i = 0; i < ( int )m_vecCustomParams.size(); i++ )
			{
				AddCustomParam( m_vecCustomParams[ i ], EPT_VECTOR_PTR, m_hData->m_hMaterial, "g_IteratePos", m_pTracePos, TRACE_POS_COUNT );
			}
		}
	}
}

int CEtBillboardEffectDataInstance::CalcTechnique( SEmitter *pEmitter, bool bTracePosMode )
{
	int nTechniqueIndex;
	bool bApplyTracePos = false;
	if( ( bTracePosMode ) && ( !pEmitter->bDisregardTracePos ) )
		bApplyTracePos = true;

	if( bApplyTracePos ) nTechniqueIndex = 1;
	else nTechniqueIndex = 0;

	if( pEmitter->bUseBumpEffectTexture )
		nTechniqueIndex += 2;

	return nTechniqueIndex;
}

void CEtBillboardEffectDataInstance::Render( EtMatrix &WorldMat, int nTick, bool bTracePos, std::vector< EtVector4 > &vecTracePos, EtColor &Color, float fScale, bool bReduceFillRate )
{
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hData->m_hMaterial ) return;
#endif

	int i, nSize;
	EtVector4 Time, vCurOrigin;
	EtColor CurColor;
	SEmitter EmitterInfo;

	SetTracePosCustomParam( vecTracePos );

	nSize = ( int )m_hData->m_vecEmitter.size();
	EtVec4Transform( &vCurOrigin, ( EtVector4 * )&WorldMat._41, CEtCamera::GetActiveCamera()->GetViewMat() );
	CurColor = Color;

	if( m_bAlphaFog )
	{
		float fFogValue = ( CEtCamera::GetActiveCamera()->GetFogFar() - vCurOrigin.z ) / ( CEtCamera::GetActiveCamera()->GetFogFar() - CEtCamera::GetActiveCamera()->GetFogNear() );
		fFogValue = min( fFogValue, 1.0f );
		CurColor.a *= fFogValue;
	}

	for( i = 0; i < nSize; i++ )
	{
		CEtBillboardEffectEmitter *pEmitter;

		pEmitter = m_hData->m_vecEmitter[ i ];
		if( !pEmitter->IsDraw() )
			continue;
		if( nTick >= pEmitter->GetLifeDuration() * 2 )
			continue;

		pEmitter->GetEmitterInfo( &EmitterInfo );
		int nTechniqueIndex = CalcTechnique( &EmitterInfo, bTracePos );
		m_vecRenderInfo[ i ].fTime = nTick / ( float )m_hData->GetMaxBillboardEffectLife();
		m_vecRenderInfo[ i ].fGravitySpeed = EmitterInfo.fFallSpeed * fScale;
		EtVec3TransformNormal( &m_vecRenderInfo[ i ].vGravity, &EmitterInfo.vFallVector, CEtCamera::GetActiveCamera()->GetViewMat() );
		m_vecRenderInfo[ i ].fScale = fScale;
		m_vecRenderInfo[ i ].Origin = vCurOrigin;
		m_vecRenderInfo[ i ].Color = CurColor;

		if( EmitterInfo.bUseYBillBoard )
		{
			EtMatrix ViewMat = *CEtCamera::GetActiveCamera()->GetViewMat();
			EtMatrix InvViewMat = *CEtCamera::GetActiveCamera()->GetInvViewMat();
			EtMatrix MatRot;
			EtMatrixRotationY( &MatRot, atan2f( InvViewMat._31, InvViewMat._33 ) );
			EtMatrixMultiply( &m_vecRenderInfo[ i ].BillBoardMatrix, &MatRot, &ViewMat );
		}
		else
		{
			EtMatrixIdentity( &m_vecRenderInfo[ i ].BillBoardMatrix );
		}

		SRenderStackElement RenderElement;

		RenderElement.hMaterial = m_hData->m_hMaterial;
		RenderElement.nTechniqueIndex = nTechniqueIndex;
		RenderElement.WorldMat = WorldMat;
		RenderElement.nSaveMatIndex = -1;
		RenderElement.pvecCustomParam = &m_vecCustomParams[ i ];
		RenderElement.pRenderMeshStream = &pEmitter->m_EmitterMeshStream;

		DWORD dwSrcBlend = g_nBlendModeList[ EmitterInfo.nSourceBlendMode ];
		DWORD dwDestBlend = g_nBlendModeList[ EmitterInfo.nDestBlendMode ];
		SStateBlock RenderStateBlock;		
		RenderStateBlock.AddRenderState( D3DRS_BLENDOP, g_nBlendOPList[ EmitterInfo.nBlendOP ] );
		RenderStateBlock.AddRenderState( D3DRS_SRCBLEND,  dwSrcBlend );
		RenderStateBlock.AddRenderState( D3DRS_DESTBLEND, dwDestBlend );
		RenderElement.nStateBlockIndex = CEtStateBlockManager::GetInstance().CreateStateBlock( &RenderStateBlock );
		RenderElement.nBakeDepthIndex = DT_NONE;
		RenderElement.fDist = -FLT_MAX;

		if( EmitterInfo.bUseBumpEffectTexture ) 
		{
			RenderElement.renderPriority = RP_NORMAL;
			RenderElement.nBakeDepthIndex = DT_NORMAL;
			GetCurRenderStack()->AddUseBackBufferRenderElement( RenderElement );
		}
		else 
		{
			GetCurRenderStack()->AddAlphaRenderElement( RenderElement );
		}
	}
}

void CEtBillboardEffectDataInstance::RenderImmediate( EtMatrix &WorldMat, int nTick, bool bTracePos, std::vector< EtVector4 > &vecTracePos, EtColor &Color, float fScale, bool bReduceFillRate )
{
	int i, nSize;
	EtVector4 Time, vCurOrigin;
	EtColor CurColor;
	SEmitter EmitterInfo;

	SetTracePosCustomParam( vecTracePos );

	nSize = ( int )m_hData->m_vecEmitter.size();
	EtVec4Transform( &vCurOrigin, ( EtVector4 * )&WorldMat._41, CEtCamera::GetActiveCamera()->GetViewMat() );
	CurColor = Color;

	if( m_bAlphaFog )
	{
		float fFogValue = ( CEtCamera::GetActiveCamera()->GetFogFar() - vCurOrigin.z ) / ( CEtCamera::GetActiveCamera()->GetFogFar() - CEtCamera::GetActiveCamera()->GetFogNear() );
		fFogValue = min( fFogValue, 1.0f );
		CurColor.a *= fFogValue;
	}

	for( i = 0; i < nSize; i++ )
	{
		CEtBillboardEffectEmitter *pEmitter;

		pEmitter = m_hData->m_vecEmitter[ i ];
		if( !pEmitter->IsDraw() )
			continue;
		if( nTick >= pEmitter->GetLifeDuration() * 2 )
			continue;
		pEmitter->GetEmitterInfo( &EmitterInfo );
		if( EmitterInfo.bUseBumpEffectTexture )
			continue;

		int nTechniqueIndex = CalcTechnique( &EmitterInfo, bTracePos );
		m_vecRenderInfo[ i ].fTime = nTick / ( float )m_hData->GetMaxBillboardEffectLife();
		m_vecRenderInfo[ i ].fGravitySpeed = EmitterInfo.fFallSpeed * fScale;
		EtVec3TransformNormal( &m_vecRenderInfo[ i ].vGravity, &EmitterInfo.vFallVector, CEtCamera::GetActiveCamera()->GetViewMat() );
		m_vecRenderInfo[ i ].fScale = fScale;
		m_vecRenderInfo[ i ].Origin = vCurOrigin;
		m_vecRenderInfo[ i ].Color = CurColor;

		if( EmitterInfo.bUseYBillBoard )
		{
			EtMatrix ViewMat = *CEtCamera::GetActiveCamera()->GetViewMat();
			EtMatrix InvViewMat = *CEtCamera::GetActiveCamera()->GetInvViewMat();
			EtMatrix MatRot;
			EtMatrixRotationY( &MatRot, atan2f( InvViewMat._31, InvViewMat._33 ) );
			EtMatrixMultiply( &m_vecRenderInfo[ i ].BillBoardMatrix, &MatRot, &ViewMat );
		}
		else
		{
			EtMatrixIdentity( &m_vecRenderInfo[ i ].BillBoardMatrix );
		}

		GetEtDevice()->SetBlendOP( g_nBlendOPList[ EmitterInfo.nBlendOP ] );
		GetEtDevice()->SetSrcBlend( g_nBlendModeList[ EmitterInfo.nSourceBlendMode ] );
		GetEtDevice()->SetDestBlend( g_nBlendModeList[ EmitterInfo.nDestBlendMode ] );

		GetEtDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

		EtMaterialHandle hParticleMaterial = m_hData->m_hMaterial;
		hParticleMaterial->SetTechnique( nTechniqueIndex );
		int nPasses = 0;
		hParticleMaterial->BeginEffect( nPasses );
		hParticleMaterial->BeginPass( 0 );
		hParticleMaterial->SetGlobalParams();
		hParticleMaterial->SetWorldMatParams( &WorldMat, &WorldMat );
		hParticleMaterial->SetCustomParamList( m_vecCustomParams[ i ] );
		hParticleMaterial->CommitChanges();
		pEmitter->m_EmitterMeshStream.Draw( hParticleMaterial->GetVertexDeclIndex( nTechniqueIndex, 0 ) );
		hParticleMaterial->EndPass();
		hParticleMaterial->EndEffect();
	}
	GetEtDevice()->SetBlendOP( BLENDOP_ADD );
	GetEtDevice()->SetSrcBlend( BLEND_SRCALPHA );
	GetEtDevice()->SetDestBlend( BLEND_INVSRCALPHA );
	GetEtDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
}