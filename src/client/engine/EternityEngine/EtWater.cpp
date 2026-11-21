#include "StdAfx.h"
#include "EtWater.h"
#include "EtSaveMat.h"
#include "EtOptionController.h"
#include "EtRenderStack.h"
#include "EtCamera.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtWater *g_pEtWater = NULL;

CEtWater *GetEtWater()
{ 
	return g_pEtWater; 
}

void DeleteWaterMap() 
{ 
	SAFE_DELETE( g_pEtWater ); 
}

void CreateSimpleWaterMap( int nCount ) 
{
	SAFE_DELETE( g_pEtWater ); 
	g_pEtWater = new CEtWater;
	g_pEtWater->Initialize( nCount );
}

CEtWater::CEtWater(void)
{
	m_fWaterBakeTime = 0.f;
	EtMatrixIdentity(&m_WaterBakeViewParam);
	EtMatrixIdentity(&m_WaterBakeLastView);
	memset( m_fAveDeltaTime, 0, sizeof(m_fAveDeltaTime) );
	m_nDeltaIndex = 0;
	m_bOptimize = false;
	m_pBackupDepthTarget = NULL;
	m_pBackupRenderTarget = NULL;

	SecureZeroMemory( &m_BackupViewport, sizeof(m_BackupViewport) );
}

CEtWater::~CEtWater(void)
{
	Clear();
}

void CEtWater::Clear()
{
	int i, nSize;
	nSize = (int)m_WaterResources.size();
	for( i = 0; i < nSize; i++) {
		SAFE_RELEASE_SPTR( m_WaterResources[i].hWaterMap );
	}
	m_WaterResources.clear();
	SAFE_RELEASE_SPTR( m_hWaterMapDepth );
}

void CEtWater::Initialize( int nCount )
{
	Clear();

	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );

	m_nWaterMapSize = 512;
	switch( Option.WaterQuality )
	{
	case WQ_HIGH:
		m_nWaterMapSize = 1024;	
		break;
	case WQ_NORMAL:
	case WQ_LOW:
		m_nWaterMapSize = 512;
		break;
	}

	m_WaterResources.resize( nCount );
	for( int i = 0; i < nCount; i++) {
		m_WaterResources[i].hWaterMap = CEtTexture::CreateRenderTargetTexture( m_nWaterMapSize, m_nWaterMapSize, FMT_A8R8G8B8 );
	}

	SAFE_RELEASE_SPTR( m_hWaterMapDepth );
	m_hWaterMapDepth = CEtDepth::CreateDepthStencil( m_nWaterMapSize, m_nWaterMapSize );
	GetCurRenderStack()->SetWaterCount( nCount );
}

EtMatrix CEtWater::GetViewProjMat(int index)
{
	EtPlane WaterPlane(0, 1, 0, -m_WaterResources[index].fWaterHeight );
	EtMatrix ReflectMat;
	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	if( hCamera )
	{
		EtMatrix MatWaterProj;
		EtMatrix CamMat = *hCamera->GetInvViewMat();

		EtMatrixReflect(&ReflectMat, &WaterPlane );
		EtMatrixMultiply(&ReflectMat, &CamMat, &ReflectMat );
		EtMatrixInverse(&ReflectMat, 0, &ReflectMat);
		// Cull water at a distance of 20 meters..
		EtMatrixPerspectiveFovLH( &MatWaterProj, hCamera->GetFOV(), hCamera->GetAspectRatio(), hCamera->GetCameraNear(), hCamera->GetCameraFar() * hCamera->GetWaterFarRatio() );
		EtMatrixMultiply(&ReflectMat, &ReflectMat, &MatWaterProj );
	}
	else {
		EtMatrixIdentity(&ReflectMat);
	}
	return ReflectMat;
}

void CEtWater::BeginWater( int index )
{
	m_pBackupRenderTarget = GetEtDevice()->GetRenderTarget();
	m_pBackupDepthTarget = GetEtDevice()->GetDepthStencilSurface();
	GetEtDevice()->SetRenderTarget( m_WaterResources[index].hWaterMap->GetSurfaceLevel() );	
	GetEtDevice()->SetDepthStencilSurface( m_hWaterMapDepth->GetDepthBuffer() );
	GetEtDevice()->SetCullMode( CULL_CW );
	GetEtDevice()->ClearBuffer( 0xffffffff, 1.0f, 0 );	
	GetEtSaveMat()->EnableSaveTransMat( false );

	EtPlane WaterPlane(0, 1, 0, -m_WaterResources[index].fWaterHeight);
	const float fDummyHeight = 30.f;

	m_BackupCamMat = *CEtCamera::GetActiveCamera()->GetInvViewMat();
	EtMatrix ReflectMat, CamMat = m_BackupCamMat;
	if( CamMat._42 < m_WaterResources[index].fWaterHeight+fDummyHeight+1.f ) {
		CamMat._42 = m_WaterResources[index].fWaterHeight+fDummyHeight+1.f;
	}
	EtMatrixReflect(&ReflectMat, &WaterPlane );
	EtMatrixMultiply(&ReflectMat, &CamMat, &ReflectMat );
	CEtCamera::GetActiveCamera()->Update( &ReflectMat );

	GetEtDevice()->GetViewport( &m_BackupViewport );
	EtViewPort waterViewport;
	waterViewport.X = 0;
	waterViewport.Y = 0;
	waterViewport.Width = m_nWaterMapSize;
	waterViewport.Height = m_nWaterMapSize;
	waterViewport.MinZ = 0;
	waterViewport.MaxZ = 1;
	GetEtDevice()->SetViewport( &waterViewport );

	EtPlane ClipPlane;	
//	WaterPlane.d += fDummyHeight;

	D3DXVECTOR3 v1(0, 0, 0);	
	D3DXVECTOR3 v2(100000.f, 0, 0);
	D3DXVECTOR3 v3(0, 0, 100000.f);
	v1.y = -(D3DXVec3Dot(&v1, (D3DXVECTOR3*)&WaterPlane) + WaterPlane.d ) / WaterPlane.b;
	v2.y = -(D3DXVec3Dot(&v2, (D3DXVECTOR3*)&WaterPlane) + WaterPlane.d ) / WaterPlane.b;
	v3.y = -(D3DXVec3Dot(&v3, (D3DXVECTOR3*)&WaterPlane) + WaterPlane.d ) / WaterPlane.b;
	EtMatrix ViewProjMat = *CEtCamera::GetActiveCamera()->GetViewProjMat();
	D3DXVec3TransformCoord(&v1, &v1, &ViewProjMat );
	D3DXVec3TransformCoord(&v2, &v2, &ViewProjMat );
	D3DXVec3TransformCoord(&v3, &v3, &ViewProjMat );
	D3DXPlaneFromPoints(&ClipPlane, &v1, &v2, &v3);

	if(ClipPlane.c < 0.f) {
		ClipPlane = -ClipPlane;
	}

	((IDirect3DDevice9*)GetEtDevice()->GetDevicePtr())->SetClipPlane(0, (float*)&ClipPlane);
	GetEtDevice()->SetRenderState( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);
}

void CEtWater::EndWater(int index)
{	
	GetEtDevice()->SetRenderState( D3DRS_CLIPPLANEENABLE, 0);
	
	GetEtDevice()->SetViewport( &m_BackupViewport );

	CEtCamera::GetActiveCamera()->Update( &m_BackupCamMat );

	GetEtDevice()->SetCullMode( CULL_CCW );
	GetEtDevice()->SetRenderTarget( m_pBackupRenderTarget );
	GetEtDevice()->SetDepthStencilSurface( m_pBackupDepthTarget );	
	GetEtSaveMat()->EnableSaveTransMat( true );
}

void CEtWater::Bake( CEtRenderStack *pRenderStack, float  fElapsedTime )
{
	const int nElemCount = _countof( m_fAveDeltaTime );
	m_fAveDeltaTime[ m_nDeltaIndex++ ] = fElapsedTime;
	m_nDeltaIndex %= nElemCount;

	float fAveDelta = 0.f;
	for( int i = 0; i < _countof(m_fAveDeltaTime); i++) {
		fAveDelta += m_fAveDeltaTime[ i ];
	}
	fAveDelta /= nElemCount;

	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	int nWaterCount = (int)m_WaterResources.size();
	if( !m_bOptimize || (1.0f / fAveDelta) >= 60 ) {
		m_WaterBakeLastView = m_WaterBakeViewParam = *hCamera->GetViewMat();
		for( int i = 0; i < nWaterCount; i++) {
			//if( m_WaterResources[i].bIgnoreBake ) continue; // Probably don't need to continue, since buffer also needs clearing. No slowdown.
			pRenderStack->Sort( CEtRenderStack::PASS_WATER + i, true);
			BeginWater( i );
			pRenderStack->RenderBlock( (CEtRenderStack::PassType)(CEtRenderStack::PASS_WATER + i) );
			EndWater( i );
		}
	}
	else {
		static float fBakeTerm = 1.f / 15.f;

		m_fWaterBakeTime += EtMin( fBakeTerm, fElapsedTime );
		if( m_fWaterBakeTime >= fBakeTerm ) {
			m_fWaterBakeTime -= fBakeTerm;
			m_WaterBakeLastView = m_WaterBakeViewParam = *hCamera->GetViewMat();
			for( int i = 0; i < nWaterCount; i++) {
				//if( m_WaterResources[i].bIgnoreBake ) continue;
				pRenderStack->Sort( CEtRenderStack::PASS_WATER + i, true);
				BeginWater( i );
				pRenderStack->RenderBlock( (CEtRenderStack::PassType)(CEtRenderStack::PASS_WATER + i) );
				EndWater( i );
			}
		}
		else {
			float fRatio = 1.0f - (m_fWaterBakeTime / fBakeTerm);

			EtMatrix currMat = *hCamera->GetViewMat();
			EtMatrix lastMat = m_WaterBakeLastView;
			D3DXMatrixInverse(&currMat, 0, &currMat);
			D3DXMatrixInverse(&lastMat, 0, &lastMat);

			EtVector3 vCamPos = *(EtVector3*)&currMat._41;
			EtVector3 vCamLastPos = *(EtVector3*)&lastMat._41;

			EtVec3Lerp(&vCamPos, &vCamLastPos, &vCamPos, fRatio);

			EtMatrix MatCurrent = currMat;
			EtMatrix MatLast = lastMat;
			MatCurrent._41 = MatCurrent._42 = MatCurrent._43 = 0.f;
			MatLast._41 = MatLast._42 = MatLast._43 = 0.f;

			EtQuat qCurrent, qLast, q;
			EtQuaternionRotationMatrix(&qCurrent, &MatCurrent );
			EtQuaternionRotationMatrix(&qLast, &MatLast );

			EtQuaternionSlerp( &q, &qLast, &qCurrent, fRatio );
			EtMatrix MatResult;
			EtMatrixRotationQuaternion( &MatResult, &q );
			MatResult._41 = vCamPos.x;
			MatResult._42 = vCamPos.y;
			MatResult._43 = vCamPos.z;
			D3DXMatrixInverse(&MatResult, 0, &MatResult);
			m_WaterBakeViewParam = MatResult;

			for( int i = 0; i < nWaterCount; i++) {
				pRenderStack->ClearElement( CEtRenderStack::PASS_WATER + i );
			}
		}
	}
}
