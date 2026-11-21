#include "StdAfx.h"
#include "DnCutSceneWorld.h"
#include "EternityEngine.h"
#include "DnCutSceneWorldGrid.h"
#include "DnCutSceneWorldSector.h"
#include "DnCutSceneActProp.h"
#include "EtMatrixEx.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//const char* RESOURCE_TILE_PATH = "./Resource/MapData/Resource/Tile";
//const char* RESOURCE_PROP_PATH = "./Resource/MapData/Resource/Prop";
//const char* RESOURCE_ENVI_PATH = "./Resource/MapData/Resource/Envi";


CDnCutSceneWorld::CDnCutSceneWorld(void) : m_pColorAdjFilter( NULL ),
										   m_pBloomFilter( NULL ),
										   m_pResPathFinder( NULL ),
										   m_pHeatHazeFilter( NULL )
{
	m_bMonochrome = false;
	m_fFilterEventVolume = 1.0f;
}

CDnCutSceneWorld::~CDnCutSceneWorld(void)
{

}

bool CDnCutSceneWorld::Initialize( const char *szWorldFolder, const char *szGridName, bool bForceInitializeSector )
{
	// Add Resource Path 
	string strWorldPath( szWorldFolder );
	m_strResourceTilePath.assign( strWorldPath ).append( "/Resource/Tile" );
	m_strResourcePropPath.assign( strWorldPath ).append( "/Resource/Prop" );
	m_strResourceEnviPath.assign( strWorldPath ).append( "/Resource/Envi" );

	CEtResourceMng::GetInstance().AddResourcePath( m_strResourceTilePath.c_str(), true );
	CEtResourceMng::GetInstance().AddResourcePath( m_strResourcePropPath.c_str(), true );
	CEtResourceMng::GetInstance().AddResourcePath( m_strResourceEnviPath.c_str(), true );
//	AddResourceDir( m_strResourceTilePath.c_str() );
//	AddResourceDir( m_strResourcePropPath.c_str() );
//	AddResourceDir( m_strResourceEnviPath.c_str() );

//#ifdef PRE_ADD_FILTEREVENT
	m_bMonochrome = false;
	m_vFilterEventColor = EtVector3(1.0f,1.0f,1.0f);
	m_fFilterEventVolume = 1.0f;
//#endif PRE_ADD_FILTEREVENT

	return CEtWorld::Initialize( szWorldFolder, szGridName );
}


//void CDnCutSceneWorld::SetResPathFinder( IResourcePathFinder* pResPathFinder )
//{
//	SectorIndex Index;
//	Index.nX = 0; Index.nY = 0;
//	static_cast<CDnCutSceneWorldSector*>(m_pGrid->GetSector( Index ))->SetResPathFinder( pResPathFinder );
//	//int iNumProp = m_pGrid->GetSector( Index )->GetPropCount();
//	//for( int iProp = 0; iProp < iNumProp; ++iProp )
//	//{
//	//	CEtWorldProp* pActProp = m_pGrid->GetProcessSector()->GetPropFromIndex( iProp );
//	//	static_cast<CDnCutSceneActProp*>(pActProp)->SetResPathFinder( pResPathFinder );
//	//}
//}


void CDnCutSceneWorld::Destroy( void )
{
	CEtResourceMng::GetInstance().RemoveResourcePath( m_strResourceTilePath.c_str(), true );
	CEtResourceMng::GetInstance().RemoveResourcePath( m_strResourcePropPath.c_str(), true );
	CEtResourceMng::GetInstance().RemoveResourcePath( m_strResourceEnviPath.c_str(), true );


//	RemoveResourceDir( m_strResourceTilePath.c_str() );
//	RemoveResourceDir( m_strResourcePropPath.c_str() );
//	RemoveResourceDir( m_strResourceEnviPath.c_str() );

	_FinalizeEnvironment();

	CEtWorld::Destroy();
}


CEtWorldGrid* CDnCutSceneWorld::AllocGrid( void )
{
	return new CDnCutSceneWorldGrid( this );
}

bool CDnCutSceneWorld::_LoadEnvironmentFile( const char* pFileName )
{
	return m_EnvInfo.Load(pFileName);
}

// Environment loading and setup
bool CDnCutSceneWorld::InitializeEnvironment( const char* pFileName, const char* pResourceFolder )
{
	string strResFolder( pResourceFolder );

	m_strEnvironmentName.assign( pFileName );
	bool bResult = _LoadEnvironmentFile( pFileName );
	assert( bResult && "Failed to read environment file!" );

	// Ambient Setting
	EtColor Color = m_EnvInfo.GetAmbient();
	m_PrevAmbient = EternityEngine::GetGlobalAmbient();
	EternityEngine::SetGlobalAmbient( &EtColor(Color.b, Color.g, Color.r, 1.0f) );

	// Color Filter
	if( m_EnvInfo.IsEnableColorFilter() )
	{
		m_pColorAdjFilter = (CEtColorAdjustTexFilter*)EternityEngine::CreateFilter( SF_COLORADJUSTTEX );
		m_pColorAdjFilter->SetParam( *m_EnvInfo.GetColorAdjShadow(), *m_EnvInfo.GetColorAdjMidtones(), *m_EnvInfo.GetColorAdjHighlights(), m_EnvInfo.GetColorAdjSaturation() );
	}
	else
		SAFE_DELETE( m_pColorAdjFilter );

	// Bloom Filter
	if( m_EnvInfo.IsEnableBloomFilter() )
	{
		m_pBloomFilter = (CEtBloomFilter*)EternityEngine::CreateFilter( SF_BLOOM );
		m_pBloomFilter->SetBloomScale( m_EnvInfo.GetBloomScale() );
		m_pBloomFilter->SetBloomSensitivity( m_EnvInfo.GetBloomSensitivity() );
	}
	else
		SAFE_DELETE( m_pBloomFilter );

	//// Fog Setting - skip this
	//if( m_hCamera ) 
	//{		
	//	//SCameraInfo *pInfo = m_hCamera  GetCameraInfo();
	//	//m_PrevCameraInfo = *pInfo;
	//	SCameraInfo Info;

	//	Info.fFogNear = m_fFogNear;
	//	Info.fFogFar = m_fFogFar;
	//	Info.fFar = m_hCamera->GetFogFar() + 200000.f; // temporary 20 meters

	//	Color = m_dwFogColor;
	//	pInfo->FogColor = EtColor( Color.b, Color.g, Color.r, 1.f );

	//	hCamera->DeActivate();
	//	hCamera->Activate();
	//}

	// Environment Texture Setting
	if( !m_strEnvironmentName.empty() )
	{
		m_hEnviTexture = EternityEngine::LoadEnvTexture( m_EnvInfo.GetEnvironmentTexture() );
	}
	else
		SAFE_RELEASE_SPTR( m_hEnviTexture );

	// Light Setting
	EtLightHandle hLight;
	EtLensFlareHandle hLensFlare;
	string strMapTextureFolder( pResourceFolder );
	strMapTextureFolder.append( "/Resource/Envi/" );
	for( DWORD i = 0; i < m_EnvInfo.GetLightCount(); ++i )
	{
		CEnvInfo::LightInfoStruct* Light = m_EnvInfo.GetLight(i);;
		Light->pInfo->Diffuse.r *= Light->pInfo->Diffuse.a;
		Light->pInfo->Diffuse.g *= Light->pInfo->Diffuse.a;
		Light->pInfo->Diffuse.b *= Light->pInfo->Diffuse.a;
		Light->pInfo->Specular.r *= Light->pInfo->Specular.a;
		Light->pInfo->Specular.g *= Light->pInfo->Specular.a;
		Light->pInfo->Specular.b *= Light->pInfo->Specular.a;
		hLight = EternityEngine::CreateLight( Light->pInfo );
		m_vlLight.push_back( hLight );

		// Lens Flare Setting
		if( Light->bEnable )
		{
			MatrixEx MatExTemp;
			MatExTemp.m_vZAxis = Light->pInfo->Direction;
			MatExTemp.MakeUpCartesianByZAxis();
			MatExTemp.RotateYAxis( Light->fRevisionYAxisAngle );
			MatExTemp.RotatePitch( Light->fRevisionXAxisAngle );

			hLensFlare = CEtLensFlare::CreateLensFlare( Light->pInfo->Direction, 
														string(strMapTextureFolder + Light->szSunTexture).c_str(),
														string(strMapTextureFolder + Light->szLensFlareTexture).c_str() );
			if( hLensFlare )
			{
				hLensFlare->SetFlareRadius( Light->fSunScale, Light->fLensScale );
				m_vlLensFlare.push_back( hLensFlare );
			}
		}
	}

	// SkyBox Setting
	if( m_EnvInfo.GetSkyBox() && strlen(m_EnvInfo.GetSkyBox()) > 0 )
	{
		m_hSkybox = EternityEngine::CreateStaticObject( m_EnvInfo.GetSkyBox() );
		if( m_hSkybox )
			m_hSkybox->EnableCull( false );
	}
	else
		SAFE_RELEASE_SPTR( m_hSkybox );

	// Skip BGM

	// Weather Setting
	if( m_EnvInfo.IsEnableWeather() )
	{
		m_hRain = CEtRainEffect::CreateRainEffect( string(strMapTextureFolder + m_EnvInfo.GetWeatherTexture()).c_str() );
		if( m_hRain )
		{
			m_hRain->SetRainSpeed( m_EnvInfo.GetWeatherSpeed() );
			m_hRain->SetStretchValue( m_EnvInfo.GetWeatherStretch() );
			m_hRain->SetLayerCount( m_EnvInfo.GetWeatherLayerCount() );
			m_hRain->SetRainSpeedDecreaseRate( m_EnvInfo.GetWeatherLayerDecreaseRate() );
			m_hRain->SetRainDir( *m_EnvInfo.GetWeatherLayerDir() );
		}
	}
	else
		SAFE_RELEASE_SPTR( m_hRain );

	// Splash Drop Setting
	if( m_EnvInfo.IsEnableSplashDrop() ) 
	{
		m_hSplashDrop = CEtRainDropSplash::CreateRainDropSplash( string(strMapTextureFolder + m_EnvInfo.GetSplashDropTexture()).c_str(), m_EnvInfo.GetSplashDropSize() );
		if( m_hSplashDrop ) 
		{
			m_hSplashDrop->SetLifeTime( m_EnvInfo.GetSplashDropLifeTime() );
			m_hSplashDrop->SetCreationRadius( (float)m_EnvInfo.GetSplashDropRadius() );
			m_hSplashDrop->SetSplashPerSecond( m_EnvInfo.GetSplashDropCount() );
		}
	}

	// Head Haze Filter Setting
	if( m_EnvInfo.IsEnableHeatHaze() ) 
	{
		m_pHeatHazeFilter = (CEtHeatHazeFilter*)EternityEngine::CreateFilter( SF_HEATHAZE );
		m_pHeatHazeFilter->SetDistortionPower( m_EnvInfo.GetHeatHazeDistortionPower() );
		m_pHeatHazeFilter->SetWaveSpeed( m_EnvInfo.GetHeatHazeWaveSpeed() );
	}

	// DOF Filter Setting
	//if( m_bEnableDOF ) 
	//{
	//	DnCameraHandle hCamera;
	//	if( hCamera )
	//	{
	//		m_nDOFCameraEffectIndex = hCamera->DepthOfField( -1, m_fDOFFocusDistance, m_fDOFNearStart, m_fDOFNearEnd, m_fDOFFarStart, m_fDOFFarEnd, m_fDOFNearBlur, m_fDOFFarBlur );
	//	}
	//}

	// Volume Fog
	//std::vector<CEtWorldEventControl *> pVecControl;
	//FindEventControlFromUniqueID( ETE_VolumeFog, &pVecControl );

	//EtVolumeFogHandle hVolumeFog;
	//BOOL bWeatherCondition[CDnWorld::WeatherEnum_Amount];
	//for( DWORD i=0; i<pVecControl.size(); i++ ) {
	//	CEtWorldEventControl *pControl = pVecControl[i];
	//	for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
	//		CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(j);
	//		VolumeFogStruct *pStruct = (VolumeFogStruct *)pArea->GetData();
	//		memcpy( bWeatherCondition, &pStruct->bApplyFineDay, sizeof(BOOL) * CDnWorld::WeatherEnum_Amount );
	//		if( bWeatherCondition[ CDnWorld::GetInstance().GetCurrentWeather() ] == FALSE ) continue;

	//		float fRadius = max( pArea->GetOBB()->Extent[0], pArea->GetOBB()->Extent[2] );
	//		float fHeight = pArea->GetMax()->y - pArea->GetMin()->y;
	//		D3DXCOLOR Color = *pStruct->vFogColor;
	//		EtVector3 vCenter = pArea->GetOBB()->Center;
	//		vCenter.y = CDnWorld::GetInstance().GetHeight( vCenter );
	//		vCenter.y += pArea->GetMin()->y;

	//		hVolumeFog = CEtVolumeFog::CreateVolumeFog( pStruct->szTextureName, vCenter, fRadius, pStruct->fDensity, fHeight, Color );
	//		m_hVecVolumeFog.push_back( hVolumeFog );
	//	}
	//}

	// Snow 
	if( m_EnvInfo.IsEnableSnow() ) {
		m_hSnow = CEtSnowEffect::CreateSnowEffect( string(strMapTextureFolder + m_EnvInfo.GetSnowTexture() ).c_str(), m_EnvInfo.GetSnowCount(), m_EnvInfo.GetSnowSize() );
		if( m_hSnow ) {
			m_hSnow->SetSnowTimeOut( m_EnvInfo.GetSnowTimeOut() );
			m_hSnow->SetSnowHeight( m_EnvInfo.GetSnowHeight() );
			m_hSnow->SetIsUp(m_EnvInfo.GetIsUp());
			m_hSnow->SetSnowAniSpeed(m_EnvInfo.GetSnowAniSpeed());
		}
	}
	else SAFE_RELEASE_SPTR( m_hSnow );


	CDnCutSceneWorldSector *pSector;
	float fVolume = 1.f;
	for( DWORD i = 0; i < m_pGrid->GetActiveSectorCount(); i++ ) 
	{
		pSector = (CDnCutSceneWorldSector*)m_pGrid->GetActiveSector( i );

		pSector->BakeLightMap( 128, m_EnvInfo.GetLightmapBlurSize() );
	}

	return true;
}


void CDnCutSceneWorld::_FinalizeEnvironment( void )
{
	m_strEnvironmentName.clear();
	m_EnvInfo.Reset();

	SAFE_RELEASE_SPTRVEC( m_vlLight );
	
	SAFE_RELEASE_SPTR( m_hSkybox );
	SAFE_RELEASE_SPTR( m_hEnviTexture );
	SAFE_DELETE( m_pColorAdjFilter );
	SAFE_DELETE( m_pBloomFilter );
	SAFE_RELEASE_SPTRVEC( m_vlLensFlare );
	SAFE_RELEASE_SPTR( m_hRain );
	SAFE_RELEASE_SPTR( m_hSplashDrop );
	SAFE_RELEASE_SPTR( m_hSnow );
	//SAFE_RELEASE_SPTRVEC( m_vlhVolumeFog
	SAFE_DELETE( m_pHeatHazeFilter );
		
	// Ambient Restore
	EternityEngine::SetGlobalAmbient( &m_PrevAmbient );

	// Skip Sound Restore..

	//m_bActivate = false;
}


//#ifdef PRE_ADD_FILTEREVENT
void CDnCutSceneWorld::SetSceneAbsoluteColor( bool bMonochrome, EtVector3 & vColor, float fVolume )
{	
	bool bExe = false;
	if( m_bMonochrome != bMonochrome || m_vFilterEventColor != vColor || m_fFilterEventVolume != fVolume )
		bExe = true;

	m_bMonochrome = bMonochrome;
	m_vFilterEventColor = vColor;
	m_fFilterEventVolume = fVolume;

	if( bExe && m_pColorAdjFilter )
		m_pColorAdjFilter->SetSceneAbsoluteColor( m_bMonochrome, m_vFilterEventColor, m_fFilterEventVolume );		
}
//#endif PRE_ADD_FILTEREVENT