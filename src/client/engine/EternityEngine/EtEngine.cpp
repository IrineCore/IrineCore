#include "StdAfx.h"
#include "EtEngine.h"
#include "EtObject.h"
#include "EtLight.h"
#include "EtTerrainArea.h"
#include "EtCamera.h"
#include "EtSaveMat.h"
#include "EtConvexVolume.h"
#include "EtBillboardEffect.h"
#include "EtShadowMap.h"
#include "EtBackBufferMng.h"
#include "EtPrimitiveDraw.h"
#include "EtOptionController.h"
#include "EtBaseEffect.h"
#include "EtEffectObject.h"
#include "EtEffectHelper.h"
#include "EtDecal.h"
#include "EtCustomRender.h"
#include "PerfCheck.h"
#include "EtLensFlare.h"
#include "EtMRTMng.h"
#include "EtWater.h"
#include "EtTextDraw.h"
#include "EtSprite.h"
#include "EtCollisionMng.h"
#include "EtBenchMark.h"
#include "EtRTTRenderer.h"
#include "EtSkyBoxRTT.h"
#include "HighResolutionCapture.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#define SKYBOX_RTT_SIZE	256

CEtEngine g_EtEngine;
float g_fElapsedTime = 0.0f;
float g_fTotalElapsedTime = 0.0f;
bool g_bSkipScene = false;
CSyncLock *g_pEtRenderLock = NULL;
CEtBenchMark g_Benchmark;
int g_nCurFrustumMask = 0;

CEtEngine::CEtEngine()
: m_pHRC( NULL )
{
}

CEtEngine::~CEtEngine()
{

}

bool CEtEngine::Initialize( HWND hWnd, int nWidth, int nHeight, SGraphicOption &Option, 
						   bool bWindow, bool bEnableMultiThread, bool bVSync, bool bShaderDebug )
{
	if( GetEtDevice()->Initialize( hWnd, nWidth, nHeight, bWindow, bEnableMultiThread, bVSync, bShaderDebug ) != ET_OK)
	{
		return false;
	}

	CEtTextDraw *pTextDraw = new CEtTextDraw();
	pTextDraw->Initialize( GetEtDevice()->Width(), GetEtDevice()->Height() );

	g_pRenderStack[ 0 ] = new CEtRenderStack();
	g_pRenderStack[ 1 ] = new CEtRenderStack();

	GetEtOptionController()->Initialize( Option );

	if( CEtSprite::CreateInstance() )
		CEtSprite::GetInstance().Initialize();

	if( CEtMRTMng::CreateInstance() )
		CEtMRTMng::GetInstance().Initialize();

	CEtCollisionMng::CreateInstance();

	if( g_pEtRenderLock == NULL )
	{
		g_pEtRenderLock = new CSyncLock();
	}

	CEtLight::InitializeClass();
	CEtObject::InitializeClass();
	
	SCameraInfo CameraInfo;
	
	if( CEtSkyBoxRTT::CreateInstance() )
		CEtSkyBoxRTT::GetInstance().Initialize( CameraInfo, SKYBOX_RTT_SIZE, SKYBOX_RTT_SIZE, nWidth, nHeight );

	CEtTexture::CreateDebugRes();

//	char szCurDir[ _MAX_PATH ];
//	GetCurrentDirectory( _MAX_PATH, szCurDir );
//	CEtResourceMng::GetInstance().AddResourcePath( szCurDir );
//	AddResourceDir( szCurDir );
	m_pHRC = new CHighResolutionCapture();
	return true;
}

void CEtEngine::Reinitialize( int nWidth, int nHeight )
{
	int i;

	OutputDebug("Reinitialize %d:%d\n", nWidth, nHeight);

	CEtResource::FlushWaitDelete();
	CLostDeviceProcess::OnLostDeviceList();

	GetEtDevice()->Reinitialize( nWidth, nHeight );
	ScopeLock<CSyncLock> Lock( CEtCamera::s_SmartPtrLock );
	for( i = 0; i < CEtCamera::GetItemCount(); i++ )
	{
		CEtCamera::GetItem( i )->Reset();
	}

	CLostDeviceProcess::OnResetDeviceList();
}

void CEtEngine::Finalize()
{
	SAFE_DELETE( m_pHRC );
	GetEtDevice()->SetGammaRamp( 1.0f, 0 );
	if( CEtTextDraw::GetInstancePtr() )
	{
		delete CEtTextDraw::GetInstancePtr();
	}
	if( CEtSprite::GetInstancePtr() )
	{
		delete CEtSprite::GetInstancePtr();
	}
	if( CEtMRTMng::GetInstancePtr() )
	{
		delete CEtMRTMng::GetInstancePtr();
	}
	if( CEtCollisionMng::GetInstancePtr() )
	{
		delete CEtCollisionMng::GetInstancePtr();
	}
	if( CEtSkyBoxRTT::GetInstancePtr() )
	{
		delete CEtSkyBoxRTT::GetInstancePtr();
	}
	SAFE_DELETE( g_pRenderStack[ 0 ] );
	SAFE_DELETE( g_pRenderStack[ 1 ] );

	// Delete Objects using the Resource first.
	CEtObject::DeleteAllObject();
	CEtEffectObject::DeleteAllObject();
	CEtBillboardEffect::DeleteAllObject();
	CEtDecal::DeleteAllObject();

	CEtResource::FlushWaitDelete();
	FinializeEffect();
	DeleteShadowMap();
	DeleteWaterMap();
	GetEtPostProcessMng()->Clear();
	CEtLight::DeleteAllObject();
	CEtCamera::DeleteAllObject();
	/*CEtObject::DeleteAllObject();
	CEtEffectObject::DeleteAllObject();
	CEtBillboardEffect::DeleteAllObject();*/
	CEtMaterial::DeleteEffectPool();
	CEtMaterial::DeleteCompiledShaders();
	CEtTerrainArea::DeleteAllObject();
	CEtBaseEffect::DeleteAllObject();
	GetEtBackBufferMng()->Clear();

	CEtLight::FinalizeClass();
	CEtObject::FinalizeClass();

	CEtPrimitiveDraw::GetInstance().Clear();

	CEtTexture::ReleaseDebugRes();
	SAFE_DELETE( g_pEtRenderLock );
	CEtResource::FlushWaitDelete();
}

int g_nRenderFrameCount = 0;
void CEtEngine::RenderFrame( float fElapsedTime )
{
	EtDeviceCheck Check;
	Check = GetEtDevice()->DeviceValidCheck();

	g_bSkipScene = false;
	if( Check == DC_CANNOT_RESET )
	{
		CEtEffectObject::ProcessDeleteEffectObjectList( fElapsedTime );
		CEtDecal::ProcessDeleteDecalList( fElapsedTime );
		CEtBillboardEffect::ProcessDeleteBillboardEffectList( fElapsedTime );

		Sleep( 10 );
		g_bSkipScene = true;
		return;
	}
	else if( Check == DC_CAN_RESET )
	{
		Reinitialize( GetEtDevice()->Width(), GetEtDevice()->Height() );
		g_bSkipScene = true;
		return;
	}

	if( CEtCamera::GetProcessItemCount() <= 0 )
	{
		g_bSkipScene = true;
		return;
	}

	// The reason for locking like this is at the moment it registers as a member in EtResource's SmartPtrMng and does push_back on the vector
	// If memory realloc occurs, the render frame side referencing the resource might crash...
	if( g_pEtRenderLock )
	{
		g_pEtRenderLock->Lock();
	}

	ScopeLock< CSyncLock > Lock( CEtResource::s_SmartPtrLock );

	CEtTerrainArea::CheckBakeLightMap();
	g_Benchmark.Process();

	int i;
	CEtConvexVolume Frustum;
	EtObjectHandle hObject;
	EtCameraHandle hCamera;
	EtViewPort OldViewPort;

	g_fElapsedTime = fElapsedTime;
	g_fTotalElapsedTime += fElapsedTime;
	CEtObject::ClearFrustumMask();
	CEtDecal::ClearFrustumMask();
	for( i = 0; i < CEtCamera::GetProcessItemCount(); i++ )
	{
		hCamera = CEtCamera::GetProcessItem( i );
		if( !hCamera ) continue;
		if( hCamera->GetCameraTargetType() != CT_SCREEN )
		{
			continue;
		}
		hCamera->Activate();
		Frustum.Initialize( *hCamera->GetViewProjMatForCull() );		

		CEtObject::MaskFrustumObjectList( &Frustum, 0x1 << i );
		CEtDecal::MaskFrustumDecalList( &Frustum, 0x1 << i );
	}
	CEtObject::InitRenderObjectList();

	GetEtDevice()->BeginScene();
	GetEtDevice()->GetViewport( &OldViewPort );

	// Must be called after InitRenderObjectList.
	CEtRTTRenderer::RenderRTTList( fElapsedTime );

	for( i = 0; i < CEtCamera::GetProcessItemCount(); i++ )
	{
		EtCameraHandle hCamera = CEtCamera::GetProcessItem(i);

		switch( hCamera->GetCameraTargetType() )
		{
			case CT_RENDERTARGET:
				hCamera->Activate();
				hCamera->RenderObjectList();
				break;
			case CT_SCREEN:
				hCamera->Activate();
				Frustum.Initialize( *hCamera->GetViewProjMatForCull() );
				g_nCurFrustumMask = 0x1 << i;
				m_pHRC->BeginCapture();

				// Make Shadow Buffer
				if( CEtLight::GetDirLightCount() > 0 && GetEtOptionController()->IsEnableDynamicShadow() )
				{
					CEtObject::RenderShadowObjectList( 0x1 << i, GetEtShadowMap()->GetShadowRange() * 2.0f );
				}

				CEtObject::RenderObjectList( 0x1 << i );
				CEtTerrainArea::RenderTerrainBlockList( &Frustum );
				CEtEffectObject::RenderEffectObjectList( fElapsedTime );
				CEtDecal::RenderDecalList( 0x1 << i, fElapsedTime );
				CEtBaseEffect::RenderEffectList( fElapsedTime );
				CEtBillboardEffect::RenderBillboardEffectList( fElapsedTime );

				GetCurRenderStack()->SetElapsedTime( fElapsedTime );
				GetCurRenderStack()->FlushRender();
				m_pHRC->EndCapture();
				break;
		}
	}
	GetEtDevice()->SetViewport( &OldViewPort );

	if( g_pEtRenderLock )
	{
		g_pEtRenderLock->UnLock();
	}
	g_nRenderFrameCount++;
}

void CEtEngine::ShowFrame( RECT *pTargetRect )
{
	GetEtSaveMat()->Clear();	
	if( g_bSkipScene ) {
		CEtPrimitiveDraw::GetInstance().Flush();
		CEtTextDraw::GetInstance().Clear();

		if( GetCurRenderStack() ) GetCurRenderStack()->EmptyRenderElement();

		return;
	}
	CEtPrimitiveDraw::GetInstance().Render( CEtCamera::GetActiveCamera() );
	CEtTextDraw::GetInstance().Render();		
	GetEtDevice()->EndScene();

	GetEtDevice()->ShowFrame( pTargetRect );	

}

#ifdef PRE_MOD_PRELOAD_SHADER
bool CEtEngine::PreLoadShader()
{
	std::vector< SHADER_COMPILE_INFO > & CompiledShaders = CEtMaterial::GetShaderCompileInfo();
	int nCount = (int)CompiledShaders.size();
	if( nCount == 0 ) return false;

	for( int i = 0; i < nCount; i++ ) 
	{
		EtMaterialHandle hMaterial;
		hMaterial = ::LoadResource( CompiledShaders[i].szFileName, RT_SHADER, true );
		if( !hMaterial )
		{
			return false;
		}
		m_hPreLoadMaterials.push_back( hMaterial );
	}
	return true;
}

void CEtEngine::ReleasePreLoadShader()
{
	SAFE_RELEASE_SPTRVEC( m_hPreLoadMaterials );
}
#endif

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
void CEtEngine::FlushBuffer()
{
	return;
	for( int i=0; i<2; i++ ) {
		if( g_pRenderStack[i] ) g_pRenderStack[i]->FlushBuffer();
	}
	if( GetEtSaveMat() ) GetEtSaveMat()->FlushBuffer();
}
#endif

void CEtEngine::CaptureScreen( const char *szFileName, int nWidth, int nHeight )
{
	m_pHRC->Capture( szFileName, nWidth, nHeight );
}

void CEtEngine::SetCaptureScreenClearColor( DWORD dwColor )
{
	m_pHRC->SetClearColor( dwColor );
}