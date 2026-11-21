#include "StdAfx.h"
#include "EtBillboardEffect.h"
#include "EtMatrixEx.h"

DECL_SMART_PTR_STATIC( CEtBillboardEffect, 1000 )

int CEtBillboardEffect::s_nEffectCountOption = 0;

CEtBillboardEffect::CEtBillboardEffect()
{
	m_nBillboardEffectTick = 0;
	m_fFloatTick = 0.0f;
	m_fScale = 1.0f;
	m_bShow = true;
	m_bTracePos = false;
	m_bLoop = false;
	m_bStop = false;
	m_Color = EtColor( 1.0f, 1.0f, 1.0f, 1.0f );

	m_bSelfDelete = true;
	m_bDelete = false;
	m_pInstance = NULL;
	m_pLoopInstance = NULL;

	m_fCullDistance = 10000.0f;

	m_nPrevTracePos = -1;

	EtMatrixIdentity( &m_WorldMat );
	EtMatrixIdentity( &m_PrevWorldMat );
	EtMatrixIdentity( &m_LinkWorldMat );
	m_bLinkFixedY = false;
	m_fLinkY = 0.f;
	m_LinkPos = EtVector3(0, 0, 0);
	m_LinkRotate = EtVector3(0, 0, 0);
	m_bLink = false;
	m_bReuseInstance = true;
	m_bReduceFillRate = false;

	m_RenderType = RT_SCREEN;

	m_bEnableCull = true;
	m_LinkBoneIndex = 0;
	m_nLinkType = 0;
}

CEtBillboardEffect::~CEtBillboardEffect()
{
	if( m_bReuseInstance )
	{
		if( m_hBillboardEffectData )
		{
			m_hBillboardEffectData->ReleaseInstance( m_pInstance );
			m_hBillboardEffectData->ReleaseInstance( m_pLoopInstance );
		}
	}
	else
	{
		SAFE_DELETE( m_pInstance );
		SAFE_DELETE( m_pLoopInstance );
	}
	SAFE_RELEASE_SPTR( m_hBillboardEffectData );
}

void CEtBillboardEffect::Initialize( EtBillboardEffectDataHandle hBillboardEffectData, EtMatrix *pInitMat, bool bReuseInstance )
{
	SAFE_RELEASE_SPTR( m_hBillboardEffectData );
	m_hBillboardEffectData = hBillboardEffectData;
	m_hBillboardEffectData->AddRef();
	m_bReuseInstance = bReuseInstance;

	if( pInitMat )
	{
		m_WorldMat = *pInitMat;
		m_PrevWorldMat = m_WorldMat;
	}
	if( !m_hBillboardEffectData->AddCallback( this ) )
	{
		if( bReuseInstance )
		{
			if( m_pInstance == NULL )
			{
				m_pInstance = m_hBillboardEffectData->GetInstance();
			}
		}
		else
		{
			if( m_pInstance == NULL )
			{
				m_pInstance = new CEtBillboardEffectDataInstance;
			}
			m_pInstance->CopyDataInfo( m_hBillboardEffectData );
		}		
	}
}

void CEtBillboardEffect::EnableLoop( bool bLoop )
{ 
	m_bLoop = bLoop; 
	if( m_hBillboardEffectData->IsReady() )
	{
		if( ( m_bLoop ) && ( m_pLoopInstance == NULL ) )
		{
			if( m_bReuseInstance )
			{
				m_pLoopInstance = m_hBillboardEffectData->GetInstance();
			}
			else
			{
				m_pLoopInstance = new CEtBillboardEffectDataInstance;
				m_pLoopInstance->CopyDataInfo( m_hBillboardEffectData );
			}
		}
	}
}

bool CEtBillboardEffect::IsCull()
{
	// In cutscenes, the camera can be directed from various angles, where particles get culled 
	// Added because sometimes it does not appear. - Hanki
	if( false == m_bEnableCull )
		return false;

	if( m_bTracePos )
		return false;

	EtCameraHandle hCamera;
	EtVector3 *pvPosition, *pvDirection, vViewPosition;
	float fDist;

	hCamera = CEtCamera::GetActiveCamera();
	pvPosition = hCamera->GetPosition();

	vViewPosition = ( *( EtVector3 * )&m_WorldMat._41 ) - *pvPosition;
	fDist = EtVec3Length( &vViewPosition );
	if( fDist > m_fCullDistance )
		return true;

	vViewPosition /= fDist;
	pvDirection = hCamera->GetDirection();
	if( EtVec3Dot( pvDirection, &vViewPosition ) < 0.7071f )
		return true;

	return false;
}

RenderType CEtBillboardEffect::SetRenderType( RenderType Type )
{
	RenderType Ret = m_RenderType;
	m_RenderType = Type;
	return Ret;
}

void CEtBillboardEffect::SaveTracePosition()
{
	int i, j;

	int nCurTracePos = ( max(0, m_nBillboardEffectTick) % m_hBillboardEffectData->GetMaxBillboardEffectLife() ) * TRACE_POS_COUNT / m_hBillboardEffectData->GetMaxBillboardEffectLife();
	int nCount = nCurTracePos - m_nPrevTracePos;
	if( nCurTracePos < m_nPrevTracePos )
	{
		nCount += TRACE_POS_COUNT;
	}
	for( i = 0; i < m_hBillboardEffectData->GetEmitterCount(); i++ )
	{
		for( j = 0; j < nCount; j++ )
		{
			int nCurIndex = ( m_nPrevTracePos + 1 + j ) % TRACE_POS_COUNT;
			EtVector4 *pTracePos = ( &m_vecTracePos[ i * TRACE_POS_COUNT ] ) + nCurIndex;
			EtVec4Lerp( pTracePos, ( EtVector4 * )&m_PrevWorldMat._41, ( EtVector4 * )&m_WorldMat._41, ( j + 1.0f ) / nCount );
		}
	}
	m_nPrevTracePos = nCurTracePos;
}

void CEtBillboardEffect::InitRender()
{
	if( ( !m_bShow ) || ( !m_hBillboardEffectData ) )
	{
		return;
	}

	if( m_bLink )  
	{
		MatrixEx Cross;
		if( m_hLinkParent ) m_LinkWorldMat = *(m_hLinkParent->GetWorldMat());
		Cross = m_LinkWorldMat;
		Cross.MoveLocalZAxis( m_LinkPos.z );
		Cross.MoveLocalXAxis( m_LinkPos.x );
		if( m_bLinkFixedY ) Cross.m_vPosition.y = m_fLinkY;
		Cross.MoveLocalYAxis( m_LinkPos.y );
		if( m_LinkBoneIndex != -1 ) 
		{
			EtMatrix MatParent;
			EtMatrixIdentity( &MatParent );
			if( m_hLinkParent && m_hLinkParent->IsShow() )
			{								
				if( m_nLinkType == CEtObject::LT_BONE )
				{
					EtMatrixMultiply( &MatParent, m_hLinkParent->GetBoneTransMat( m_LinkBoneIndex ), m_hLinkParent->GetWorldMat() );
				}
				else
				{
					EtMatrixMultiply( &MatParent, m_hLinkParent->GetDummyTransMat( m_LinkBoneIndex ), m_hLinkParent->GetWorldMat() );
				}
			}
			else
			{
				StopPlay();
			}
			memcpy( &Cross.m_vXAxis, &MatParent._11, sizeof(EtVector3) );
			memcpy( &Cross.m_vYAxis, &MatParent._21, sizeof(EtVector3) );
			memcpy( &Cross.m_vZAxis, &MatParent._31, sizeof(EtVector3) );				
			Cross.m_vPosition += *(EtVector3*)&MatParent._41;
			Cross.m_vPosition -= *(EtVector3*)&m_LinkWorldMat._41;
		}

		Cross.RotateYaw( m_LinkRotate.y );
		Cross.RotateRoll( m_LinkRotate.z );
		Cross.RotatePitch( m_LinkRotate.x );
		if( IsTracePos() )
		{
			SetWorldPos( Cross.GetPosition() );
		}
		else
		{
			SetWorldMat( Cross );
		}
	}

	if( m_bTracePos )
	{
		if( (int)m_vecTracePos.size() < m_hBillboardEffectData->GetEmitterCount() * TRACE_POS_COUNT )
		{
			m_vecTracePos.resize( m_hBillboardEffectData->GetEmitterCount() * TRACE_POS_COUNT );
		}
		SaveTracePosition();
	}
}

void CEtBillboardEffect::Render()
{
	if( ( !m_bShow ) || ( !m_hBillboardEffectData ) || ( !m_hBillboardEffectData->IsReady() ) )
		return;

	if( IsCull() )
		return;

	if( m_pInstance )
	{
		m_pInstance->Render( m_WorldMat, m_nBillboardEffectTick, m_bTracePos, m_vecTracePos, m_Color, m_fScale, m_bReduceFillRate );
		if( ( m_bLoop ) && ( m_nBillboardEffectTick >= m_hBillboardEffectData->GetMaxBillboardEffectLife() ) )
		{
			m_pLoopInstance->Render( m_WorldMat, m_nBillboardEffectTick - m_hBillboardEffectData->GetMaxBillboardEffectLife(), 
				m_bTracePos, m_vecTracePos, m_Color, m_fScale, m_bReduceFillRate );
		}
	}
}

void CEtBillboardEffect::RenderImmediate()
{
	if( ( !m_bShow ) || ( !m_hBillboardEffectData ) || ( !m_hBillboardEffectData->IsReady() ) )
		return;

	if( IsCull() )
		return;

	if( m_pInstance )
	{
		m_pInstance->RenderImmediate( m_WorldMat, m_nBillboardEffectTick, m_bTracePos, m_vecTracePos, m_Color, m_fScale, m_bReduceFillRate );
		if( ( m_bLoop ) && ( m_nBillboardEffectTick >= m_hBillboardEffectData->GetMaxBillboardEffectLife() ) )
		{
			m_pLoopInstance->RenderImmediate( m_WorldMat, m_nBillboardEffectTick - m_hBillboardEffectData->GetMaxBillboardEffectLife(), 
				m_bTracePos, m_vecTracePos, m_Color, m_fScale, m_bReduceFillRate );
		}
	}
}

bool CEtBillboardEffect::CalcTick( float fElapsedTime )
{
	if( !m_hBillboardEffectData )
	{
		m_bDelete = true;
		return true;
	}
	int nTickAdd;

	m_fFloatTick += ( fElapsedTime * BILLBOARD_EFFECT_FRAME );
	nTickAdd = ( int )( m_fFloatTick );
	m_nBillboardEffectTick += nTickAdd;
	m_fFloatTick -= nTickAdd;

	if( m_bLoop )
	{
		if( m_nBillboardEffectTick >= m_hBillboardEffectData->GetMaxBillboardEffectLife() * 2 )
		{
			if( m_bStop )
				m_bLoop = false;
			m_nBillboardEffectTick -= m_hBillboardEffectData->GetMaxBillboardEffectLife();
		}
	}
	else
	{
		if( m_nBillboardEffectTick >= m_hBillboardEffectData->GetMaxBillboardEffectLife() * 2 )
		{			
			if( m_bSelfDelete )
			{
				m_bDelete = true;
				return true;
			}
			m_bStop = true;
			m_bShow = false;
		}
	}

	return false;
}

void CEtBillboardEffect::StopPlay()
{
	m_bStop = true;
	if( m_nBillboardEffectTick < m_hBillboardEffectData->GetMaxBillboardEffectLife() )
		m_bLoop = false;
}

void CEtBillboardEffect::RenderBillboardEffectList( float fElapsedTime )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	int i, nCount;
	EtBillboardEffectHandle hHandle;
	std::vector< EtBillboardEffectHandle > vecDeleteList;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		hHandle = GetItem( i );
		if( hHandle->m_bDelete )
		{
			vecDeleteList.push_back( hHandle );
			continue;
		}
		hHandle->InitRender();
		if( hHandle->GetRenderType() == RT_SCREEN )
		{
			hHandle->Render();
		}
		hHandle->CalcTick( fElapsedTime );
	}

	nCount = ( int )vecDeleteList.size();
	for( i = 0; i < nCount; i++ )
	{
		SAFE_RELEASE_SPTR( vecDeleteList[ i ] );
	}
}

void CEtBillboardEffect::ProcessDeleteBillboardEffectList( float fElapsedTime )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	int i, nCount;
	EtBillboardEffectHandle hHandle;
	std::vector< EtBillboardEffectHandle > vecDeleteList;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		hHandle = GetItem( i );
		if( hHandle->m_bDelete )
		{
			vecDeleteList.push_back( hHandle );
			continue;
		}
		hHandle->CalcTick( fElapsedTime );
	}

	nCount = ( int )vecDeleteList.size();
	for( i = 0; i < nCount; i++ )
	{
		SAFE_RELEASE_SPTR( vecDeleteList[ i ] );
	}
}

void CEtBillboardEffect::OnLoadComplete( CBackgroundLoader *pLoader )
{
	if( m_pInstance == NULL )
	{
		if( m_bReuseInstance )
			m_pInstance = m_hBillboardEffectData->GetInstance();
		else
			m_pInstance = new CEtBillboardEffectDataInstance;
	}
	if( ( m_bLoop ) && ( m_pLoopInstance == NULL ) )
	{
		if( m_bReuseInstance )
			m_pLoopInstance = m_hBillboardEffectData->GetInstance();
		else
			m_pLoopInstance = new CEtBillboardEffectDataInstance;
	}
}

void CEtBillboardEffect::SetLink( EtMatrix WorldMat, bool bFixedY, float fY, char *szBoneName, EtVector3 vPos, EtVector3 vRotate, EtAniObjectHandle hParent )
{	
	m_LinkWorldMat = WorldMat;
	m_bLinkFixedY = bFixedY;
	m_fLinkY = fY;

	m_LinkBoneIndex = -1;
	m_nLinkType = CEtObject::LT_BONE;

	if( hParent )
	{
		EtMeshHandle hMesh = hParent->GetMesh();
		if( hMesh && szBoneName )
		{
			m_LinkBoneIndex = hMesh->FindDummy( szBoneName );
			if( m_LinkBoneIndex != -1 )
			{
				m_nLinkType = CEtObject::LT_MESH_DUMMY;
			}
			else
			{
				m_LinkBoneIndex = hMesh->FindBone( szBoneName );
			}
		}
	}

	m_LinkPos = vPos;
	m_LinkRotate = vRotate;
	m_hLinkParent = hParent;
	m_bLink = true;
}