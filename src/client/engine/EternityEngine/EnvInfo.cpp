#include "StdAfx.h"
#include "EnvInfo.h"

enum MinorVersion
{
	MiniorVersion_Default = 0,	//2011-04-12 Modified config file
	MiniorVersion_SnowSize = 1,	//2011-05-06 Added snow size/texture animation config feature
	MiniorVersion_EnviEffect = 2, //2012-10-04 Effect file attached to local camera
	MiniorVersion_Max
};

enum MajorVersion
{
	MajorVersion_Default = 1,	//2011-04-12 Modified config file
	MajorVersion_Max
};

const DWORD CEnvInfo::ms_VersionMajor = MajorVersion_Max - 1;
const DWORD CEnvInfo::ms_VersionMinor = MiniorVersion_Max - 1;
const std::string CEnvInfo::ms_FileHeaderInfo = "EnvInfo File Header";

CEnvInfo::CEnvInfo(void)
{
	Reset();
}

CEnvInfo::~CEnvInfo(void)
{
	Reset();
}

void CEnvInfo::Reset()
{
	m_szSkyBox = "";
	m_vColorAdjShadow = EtVector4( 0.f, 0.f, 0.f, 0.f );
	m_vColorAdjMidtones = EtVector4( 0.f, 0.f, 0.f, 0.f );
	m_vColorAdjHighlights = EtVector4( 0.f, 0.f, 0.f, 0.f );
	m_dwAmbient = RGB(255,255,255);
	m_dwFogColor = RGB(255,255,255);
	m_fFogNear = 1000000.f;
	m_fFogFar = 1000000.f;
	m_fSaturation = 1.f;
	m_bEnableColorFilter = false;
	m_bEnableBloomFilter = false;
	m_fBloomScale = 1.f;
	m_fBloomSensitivity = 6.f;
	m_szEnviBGM = "";
	m_fEnviBGMRatio = 0.f;

	m_nLightmapBlurSize = 1;

	m_bEnableWeather = false;
	m_fWeatherSpeed = 1.f;
	m_fWeatherStretch = 1.f;
	m_nWeatherLayerCount = 1;
	m_fWeatherLayerDecreaseRate = 0.9f;
	m_vWeatherLayerDir = EtVector3( 0.f, -1.f, 0.f );

	m_bEnableSplashDrop = false;
	m_fSplashDropSize = 5.f;
	m_fSplashDropLifeTime = 0.2f;
	m_nSplashDropRadius = 1000;
	m_nSplashDropCount = 1500;

	m_bEnableHeatHaze = false;
	m_fHeatHazeDistortionPower = 1.f;
	m_fHeatHazeWaveSpeed = 5.f;

	m_bEnableDOF = false;
	m_fDOFFocusDistance = 1000.f;
	m_fDOFNearStart = 800.f;
	m_fDOFNearEnd = 1000.;
	m_fDOFFarStart = 500.f;
	m_fDOFFarEnd = 1000.f;
	m_fNearBlur = 8.f;
	m_fFarBlur = 8.f;

	m_bEnableSnowEffect = false;
	m_nSnowCount = 1500;
	m_nSnowTimeOut = 7000;
	m_fSnowHeight = 1500.f;
	m_isUp = false;
	m_fSnowSize = 0.0f;
	m_fSnowAniSpeed = 1.0f;

	m_szEnvironmentEffect = "";

	std::vector<LightInfoStruct *>::iterator iter = m_pVecLightList.begin();
	std::vector<LightInfoStruct *>::iterator endIter = m_pVecLightList.end();
	for (; iter != endIter; ++iter)
	{
		LightInfoStruct* pLightInfo = (*iter);
		if (pLightInfo)
		{
			if (pLightInfo->pInfo)
				delete pLightInfo->pInfo;

			delete pLightInfo;
		}
	}

	m_pVecLightList.clear();

}

void CEnvInfo::AddLight( SLightInfo &Info )
{
	LightInfoStruct *pStruct = new LightInfoStruct;
	pStruct->pInfo = new SLightInfo;
	*pStruct->pInfo = Info;

	m_pVecLightList.push_back( pStruct );
}

void CEnvInfo::RemoveLight(int nIndex)
{
	if( nIndex < 0 || nIndex >= (int)m_pVecLightList.size() ) return;
	SAFE_DELETE( m_pVecLightList[nIndex]->pInfo );
	SAFE_DELETE( m_pVecLightList[nIndex] );
	m_pVecLightList.erase( m_pVecLightList.begin() + nIndex );
}

DWORD CEnvInfo::GetLightCount()
{
	return (DWORD)m_pVecLightList.size();
}

CEnvInfo::LightInfoStruct * CEnvInfo::GetLight( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= (DWORD)m_pVecLightList.size() ) return NULL;
	return m_pVecLightList[dwIndex];
}

bool CEnvInfo::IsOldVersion(int nMajorVersion, int nMinorVersion)
{
	bool missMatchMajorVersion = (nMajorVersion != MajorVersion_Default);
	
	bool missMatchMinorVersion = (nMinorVersion != MiniorVersion_Default ||
								  nMinorVersion != MiniorVersion_SnowSize ||
								  nMinorVersion != MiniorVersion_EnviEffect);

	//If neither Major nor Minor version matches..
	return (missMatchMajorVersion && missMatchMinorVersion);
}

bool CEnvInfo::Load(const char* szFileName)
{
// 	CString szFullName;
// 	szFullName.Format( "%s\\Resource\\Envi\\%s.env", CFileServer::GetInstance().GetLocalWorkingFolder(), m_szName );

	//CFileStream Stream( szFileName, CFileStream::OPEN_READ );
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return false;

	//Header info
	//File header info
	EnvHeader envHeader;
	memset(&envHeader, 0, sizeof(EnvHeader));	//Initialize.

	Stream.Read(&envHeader, sizeof(EnvHeader));
	//File header characters do not match...
	if( strcmp( envHeader.szHeaderString, ms_FileHeaderInfo.c_str() ) != 0 ) 
	{
		//As a version without a file header..
		return LoadNonFileHeader(szFileName);
	}

	//Read basic info..
	LoadVersion1_0(&Stream);
	//According to additional version info..
	if (envHeader.nVersionMinor >= MiniorVersion_SnowSize)
		LoadVersion1_1(&Stream);
	if (envHeader.nVersionMinor >= MiniorVersion_EnviEffect)
		LoadVersion1_2(&Stream);

	return true;
}

bool CEnvInfo::LoadNonFileHeader(const char* szFileName)
{
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return false;

	//File version info..
	DWORD nVersionMajor = 0;
	DWORD nVersionMinor = 0;

	Stream.Read( &nVersionMajor, sizeof(nVersionMajor));
	Stream.Read( &nVersionMinor, sizeof(nVersionMinor));

	//Check if it is an old file without version info
	bool isOldVersion = IsOldVersion(nVersionMajor, nVersionMinor);
	if (isOldVersion)
	{
		std::string msg = szFileName;
		msg += "Old version of environment file!!!!!!";
		ASSERT( 0 && msg.c_str());
		return LoadOldVersion(szFileName);
	}

	//Read basic info..
	LoadVersion1_0(&Stream);
	//According to additional version info..
	if (nVersionMinor >= MiniorVersion_SnowSize)
		LoadVersion1_1(&Stream);
	if (nVersionMinor >= MiniorVersion_EnviEffect)
		LoadVersion1_2(&Stream);

	return true;
}

bool CEnvInfo::LoadOldVersion(const char* szFileName)
{
	// 	CString szFullName;
	// 	szFullName.Format( "%s\\Resource\\Envi\\%s.env", CFileServer::GetInstance().GetLocalWorkingFolder(), m_szName );

	//CFileStream Stream( szFileName, CFileStream::OPEN_READ );
	CResMngStream Stream( szFileName );
	if( !Stream.IsValid() ) return false;

	ReadStdString( m_szSkyBox, &Stream );
	Stream.Read( &m_dwAmbient, sizeof(DWORD) );
	Stream.Read( &m_dwFogColor, sizeof(DWORD) );
	Stream.Read( &m_fFogNear, sizeof(float) );
	Stream.Read( &m_fFogFar, sizeof(float) );
	ReadStdString( m_szEnvironmentTexture, &Stream );

	Stream.Read( &m_bEnableColorFilter, sizeof(bool) );
	Stream.Read( &m_vColorAdjShadow, sizeof(EtVector4) );
	Stream.Read( &m_vColorAdjMidtones, sizeof(EtVector4) );
	Stream.Read( &m_vColorAdjHighlights, sizeof(EtVector4) );
	Stream.Read( &m_fSaturation, sizeof(float) );

	Stream.Read( &m_bEnableBloomFilter, sizeof(bool) );
	Stream.Read( &m_fBloomScale, sizeof(float) );
	Stream.Read( &m_fBloomSensitivity, sizeof(float) );

	Stream.Read( &m_nLightmapBlurSize, sizeof(int) );

	ReadStdString( m_szEnviBGM, &Stream );
	Stream.Read( &m_fEnviBGMRatio, sizeof(float) );

	//////////////////////
	Stream.Read( &m_bEnableWeather, sizeof(bool) );
	ReadStdString( m_szWeatherTexture, &Stream );
	Stream.Read( &m_fWeatherSpeed, sizeof(float) );
	Stream.Read( &m_fWeatherStretch, sizeof(float) );
	Stream.Read( &m_nWeatherLayerCount, sizeof(int) );
	Stream.Read( &m_fWeatherLayerDecreaseRate, sizeof(float) );
	Stream.Read( &m_vWeatherLayerDir, sizeof(EtVector3) );


	Stream.Read( &m_bEnableSplashDrop, sizeof(bool) );
	ReadStdString( m_szSplashDropTexture, &Stream );
	Stream.Read( &m_fSplashDropSize, sizeof(float) );
	Stream.Read( &m_fSplashDropLifeTime, sizeof(float) );
	Stream.Read( &m_nSplashDropRadius, sizeof(int) );
	Stream.Read( &m_nSplashDropCount, sizeof(int) );
	//////////////////////

	Stream.Read( &m_bEnableHeatHaze, sizeof(bool) );
	Stream.Read( &m_fHeatHazeDistortionPower, sizeof(float) );
	Stream.Read( &m_fHeatHazeWaveSpeed, sizeof(float) );

	Stream.Read( &m_bEnableDOF, sizeof(bool) );
	Stream.Read( &m_fDOFFocusDistance, sizeof(float) );
	Stream.Read( &m_fDOFNearStart, sizeof(float) );
	Stream.Read( &m_fDOFNearEnd, sizeof(float) );
	Stream.Read( &m_fDOFFarStart, sizeof(float) );
	Stream.Read( &m_fDOFFarEnd, sizeof(float) );
	Stream.Read( &m_fNearBlur, sizeof(float) );
	Stream.Read( &m_fFarBlur, sizeof(float) );

	Stream.Read( &m_bEnableSnowEffect, sizeof(bool) );
	ReadStdString( m_szSnowEffectTexture, &Stream );
	Stream.Read( &m_nSnowCount, sizeof(int) );
	Stream.Read( &m_nSnowTimeOut, sizeof(int) );
	Stream.Read( &m_fSnowHeight, sizeof(float) );
	
	// Read light
	SLightInfo Info;
	int nCount;
	Stream.Read( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		Stream.Read( &Info, sizeof(SLightInfo) );
		AddLight( Info );

		Stream.Read( &m_pVecLightList[m_pVecLightList.size() - 1]->bEnable, sizeof(bool) );
		ReadStdString( m_pVecLightList[m_pVecLightList.size() - 1]->szSunTexture, &Stream );
		ReadStdString( m_pVecLightList[m_pVecLightList.size() - 1]->szLensFlareTexture, &Stream );
		Stream.Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fSunScale, sizeof(float) );
		Stream.Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fLensScale, sizeof(float) );
		Stream.Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fRevisionXAxisAngle, sizeof(float) );
		Stream.Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fRevisionYAxisAngle, sizeof(float) );
	}

	return true;
}

void CEnvInfo::Save(const char* szFileName)
{
	CFileStream Stream( szFileName, CFileStream::OPEN_WRITE );
	if( !Stream.IsValid() ) return;

	
	//File header info
	EnvHeader envHeader;
	memset(&envHeader, 0, sizeof(EnvHeader));	//Initialize.

	//Set Header string.
	sprintf(envHeader.szHeaderString, "%s", ms_FileHeaderInfo.c_str());

	//Set version info.
	envHeader.nVersionMajor = ms_VersionMajor;
	envHeader.nVersionMinor = ms_VersionMinor;

	//Save header info
	Stream.Write(&envHeader, sizeof(EnvHeader));

	
	// Save additional version-specific info by appending it to the end of the previous file.
	// Grouping and saving by feature may cause compatibility issues with older files.
	// | Previous version info... | Added info | Added info | ...
	// | <--- Ver1.0 ----> | <-Ver1.1--> | <--Ver1.2-->| ..
	// Seems this can also resolve issues when older versions read newer version files.
	SaveVersion1_0(Stream);
	SaveVersion1_1(Stream);
	SaveVersion1_2(Stream);
}

void CEnvInfo::SaveVersion1_2(CFileStream &Stream)
{
	WriteStdString( m_szEnvironmentEffect, &Stream );
}

void CEnvInfo::SaveVersion1_1(CFileStream &Stream)
{
	Stream.Write(&m_fSnowSize, sizeof(m_fSnowSize));			//Snow Size
	Stream.Write(&m_fSnowAniSpeed, sizeof(m_fSnowAniSpeed));	//Snow Ani Speed
}

void CEnvInfo::SaveVersion1_0( CFileStream &Stream )
{
	WriteStdString( m_szSkyBox, &Stream );
	Stream.Write( &m_dwAmbient, sizeof(DWORD) );
	Stream.Write( &m_dwFogColor, sizeof(DWORD) );
	Stream.Write( &m_fFogNear, sizeof(float) );
	Stream.Write( &m_fFogFar, sizeof(float) );
	WriteStdString( m_szEnvironmentTexture, &Stream );

	Stream.Write( &m_bEnableColorFilter, sizeof(bool) );
	Stream.Write( &m_vColorAdjShadow, sizeof(EtVector4) );
	Stream.Write( &m_vColorAdjMidtones, sizeof(EtVector4) );
	Stream.Write( &m_vColorAdjHighlights, sizeof(EtVector4) );
	Stream.Write( &m_fSaturation, sizeof(float) );

	Stream.Write( &m_bEnableBloomFilter, sizeof(bool) );
	Stream.Write( &m_fBloomScale, sizeof(float) );
	Stream.Write( &m_fBloomSensitivity, sizeof(float) );

	Stream.Write( &m_nLightmapBlurSize, sizeof(int) );

	WriteStdString( m_szEnviBGM, &Stream );
	Stream.Write( &m_fEnviBGMRatio, sizeof(float) );

	//////////////////////
	Stream.Write( &m_bEnableWeather, sizeof(bool) );
	WriteStdString( m_szWeatherTexture, &Stream );
	Stream.Write( &m_fWeatherSpeed, sizeof(float) );
	Stream.Write( &m_fWeatherStretch, sizeof(float) );
	Stream.Write( &m_nWeatherLayerCount, sizeof(int) );
	Stream.Write( &m_fWeatherLayerDecreaseRate, sizeof(float) );
	Stream.Write( &m_vWeatherLayerDir, sizeof(EtVector3) );

	Stream.Write( &m_bEnableSplashDrop, sizeof(bool) );
	WriteStdString( m_szSplashDropTexture, &Stream );
	Stream.Write( &m_fSplashDropSize, sizeof(float) );
	Stream.Write( &m_fSplashDropLifeTime, sizeof(float) );
	Stream.Write( &m_nSplashDropRadius, sizeof(int) );
	Stream.Write( &m_nSplashDropCount, sizeof(int) );
	//////////////////////

	Stream.Write( &m_bEnableHeatHaze, sizeof(bool) );
	Stream.Write( &m_fHeatHazeDistortionPower, sizeof(float) );
	Stream.Write( &m_fHeatHazeWaveSpeed, sizeof(float) );

	Stream.Write( &m_bEnableDOF, sizeof(bool) );
	Stream.Write( &m_fDOFFocusDistance, sizeof(float) );
	Stream.Write( &m_fDOFNearStart, sizeof(float) );
	Stream.Write( &m_fDOFNearEnd, sizeof(float) );
	Stream.Write( &m_fDOFFarStart, sizeof(float) );
	Stream.Write( &m_fDOFFarEnd, sizeof(float) );
	Stream.Write( &m_fNearBlur, sizeof(float) );
	Stream.Write( &m_fFarBlur, sizeof(float) );

	Stream.Write( &m_bEnableSnowEffect, sizeof(bool) );
	WriteStdString( m_szSnowEffectTexture, &Stream );
	Stream.Write( &m_nSnowCount, sizeof(int) );
	Stream.Write( &m_nSnowTimeOut, sizeof(int) );
	Stream.Write( &m_fSnowHeight, sizeof(float) );
	Stream.Write( &m_isUp, sizeof(m_isUp));

	// Save light
	int nCount = (int)m_pVecLightList.size();
	Stream.Write( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		Stream.Write( m_pVecLightList[i]->pInfo, sizeof(SLightInfo) );

		Stream.Write( &m_pVecLightList[i]->bEnable, sizeof(bool) );
		WriteStdString( m_pVecLightList[i]->szSunTexture, &Stream );
		WriteStdString( m_pVecLightList[i]->szLensFlareTexture, &Stream );
		Stream.Write( &m_pVecLightList[i]->fSunScale, sizeof(float) );
		Stream.Write( &m_pVecLightList[i]->fLensScale, sizeof(float) );
		Stream.Write( &m_pVecLightList[i]->fRevisionXAxisAngle, sizeof(float) );
		Stream.Write( &m_pVecLightList[i]->fRevisionYAxisAngle, sizeof(float) );
	}
}

void CEnvInfo::LoadVersion1_2( CResMngStream *Stream )
{
	ReadStdString( m_szEnvironmentEffect, Stream );
}

void CEnvInfo::LoadVersion1_1( CResMngStream *Stream)
{
	Stream->Read(&m_fSnowSize, sizeof(m_fSnowSize));
	Stream->Read(&m_fSnowAniSpeed, sizeof(m_fSnowAniSpeed));
}

void CEnvInfo::LoadVersion1_0( CResMngStream *Stream )
{
	ReadStdString( m_szSkyBox, Stream );
	Stream->Read( &m_dwAmbient, sizeof(DWORD) );
	Stream->Read( &m_dwFogColor, sizeof(DWORD) );
	Stream->Read( &m_fFogNear, sizeof(float) );
	Stream->Read( &m_fFogFar, sizeof(float) );
	ReadStdString( m_szEnvironmentTexture, Stream );

	Stream->Read( &m_bEnableColorFilter, sizeof(bool) );
	Stream->Read( &m_vColorAdjShadow, sizeof(EtVector4) );
	Stream->Read( &m_vColorAdjMidtones, sizeof(EtVector4) );
	Stream->Read( &m_vColorAdjHighlights, sizeof(EtVector4) );
	Stream->Read( &m_fSaturation, sizeof(float) );

	Stream->Read( &m_bEnableBloomFilter, sizeof(bool) );
	Stream->Read( &m_fBloomScale, sizeof(float) );
	Stream->Read( &m_fBloomSensitivity, sizeof(float) );

	Stream->Read( &m_nLightmapBlurSize, sizeof(int) );

	ReadStdString( m_szEnviBGM, Stream );
	Stream->Read( &m_fEnviBGMRatio, sizeof(float) );

	//////////////////////
	Stream->Read( &m_bEnableWeather, sizeof(bool) );
	ReadStdString( m_szWeatherTexture, Stream );
	Stream->Read( &m_fWeatherSpeed, sizeof(float) );
	Stream->Read( &m_fWeatherStretch, sizeof(float) );
	Stream->Read( &m_nWeatherLayerCount, sizeof(int) );
	Stream->Read( &m_fWeatherLayerDecreaseRate, sizeof(float) );
	Stream->Read( &m_vWeatherLayerDir, sizeof(EtVector3) );


	Stream->Read( &m_bEnableSplashDrop, sizeof(bool) );
	ReadStdString( m_szSplashDropTexture, Stream );
	Stream->Read( &m_fSplashDropSize, sizeof(float) );
	Stream->Read( &m_fSplashDropLifeTime, sizeof(float) );
	Stream->Read( &m_nSplashDropRadius, sizeof(int) );
	Stream->Read( &m_nSplashDropCount, sizeof(int) );
	//////////////////////

	Stream->Read( &m_bEnableHeatHaze, sizeof(bool) );
	Stream->Read( &m_fHeatHazeDistortionPower, sizeof(float) );
	Stream->Read( &m_fHeatHazeWaveSpeed, sizeof(float) );

	Stream->Read( &m_bEnableDOF, sizeof(bool) );
	Stream->Read( &m_fDOFFocusDistance, sizeof(float) );
	Stream->Read( &m_fDOFNearStart, sizeof(float) );
	Stream->Read( &m_fDOFNearEnd, sizeof(float) );
	Stream->Read( &m_fDOFFarStart, sizeof(float) );
	Stream->Read( &m_fDOFFarEnd, sizeof(float) );
	Stream->Read( &m_fNearBlur, sizeof(float) );
	Stream->Read( &m_fFarBlur, sizeof(float) );

	Stream->Read( &m_bEnableSnowEffect, sizeof(bool) );
	ReadStdString( m_szSnowEffectTexture, Stream );
	Stream->Read( &m_nSnowCount, sizeof(int) );
	Stream->Read( &m_nSnowTimeOut, sizeof(int) );
	Stream->Read( &m_fSnowHeight, sizeof(float) );
	Stream->Read( &m_isUp, sizeof(m_isUp));

	// Read light
	SLightInfo Info;
	int nCount;
	Stream->Read( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		Stream->Read( &Info, sizeof(SLightInfo) );
		AddLight( Info );

		Stream->Read( &m_pVecLightList[m_pVecLightList.size() - 1]->bEnable, sizeof(bool) );
		ReadStdString( m_pVecLightList[m_pVecLightList.size() - 1]->szSunTexture, Stream );
		ReadStdString( m_pVecLightList[m_pVecLightList.size() - 1]->szLensFlareTexture, Stream );
		Stream->Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fSunScale, sizeof(float) );
		Stream->Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fLensScale, sizeof(float) );
		Stream->Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fRevisionXAxisAngle, sizeof(float) );
		Stream->Read( &m_pVecLightList[m_pVecLightList.size() - 1]->fRevisionYAxisAngle, sizeof(float) );
	}
}
