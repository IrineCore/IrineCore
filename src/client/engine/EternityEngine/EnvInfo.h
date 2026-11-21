#pragma once

class CResMngStream;
class CEnvInfo
{
public:
	static const DWORD ms_VersionMajor;
	static const DWORD ms_VersionMinor;
	static const std::string ms_FileHeaderInfo;

	struct EnvHeader {
		char szHeaderString[32];
		DWORD nVersionMajor;
		DWORD nVersionMinor;
	};

	CEnvInfo(void);
	~CEnvInfo(void);

	struct LightInfoStruct {
		SLightInfo *pInfo;

		bool bEnable;
		std::string szSunTexture;
		std::string szLensFlareTexture;
		float fSunScale;
		float fLensScale;
		float fRevisionXAxisAngle;
		float fRevisionYAxisAngle;

		LightInfoStruct()
		{
			pInfo = NULL;
			bEnable = false;
			fSunScale = 0.1f;
			fLensScale = 0.2f;
			fRevisionXAxisAngle = 0.f;
			fRevisionYAxisAngle = 0.f;
		}
	};
protected:
	std::string m_szName;
	std::string m_szSkyBox;
	DWORD m_dwAmbient;
	DWORD m_dwFogColor;
	float m_fFogNear;
	float m_fFogFar;
	std::string m_szEnvironmentTexture;
	std::vector<LightInfoStruct *> m_pVecLightList;
	std::string m_szEnviBGM;
	float m_fEnviBGMRatio;

	bool m_bEnableColorFilter;
	EtVector4 m_vColorAdjShadow;
	EtVector4 m_vColorAdjMidtones;
	EtVector4 m_vColorAdjHighlights;
	float m_fSaturation;

	bool m_bEnableBloomFilter;
	float m_fBloomScale;
	float m_fBloomSensitivity;

	int m_nLightmapBlurSize;

	// Weather - Rain
	bool m_bEnableWeather;
	std::string m_szWeatherTexture;
	float m_fWeatherSpeed;
	float m_fWeatherStretch;
	int m_nWeatherLayerCount;
	float m_fWeatherLayerDecreaseRate;
	EtVector3 m_vWeatherLayerDir;

	// Weather - Snow
	bool m_bEnableSnowEffect;
	std::string m_szSnowEffectTexture;
	int m_nSnowCount;
	int m_nSnowTimeOut;
	float m_fSnowHeight;
	bool m_isUp;
	float m_fSnowSize;
	float m_fSnowAniSpeed;

	// SplashDrop
	bool m_bEnableSplashDrop;
	std::string m_szSplashDropTexture;
	float m_fSplashDropSize;
	float m_fSplashDropLifeTime;
	int m_nSplashDropRadius;
	int m_nSplashDropCount;

	// HeatHaze
	bool m_bEnableHeatHaze;
	float m_fHeatHazeDistortionPower;
	float m_fHeatHazeWaveSpeed;

	// DOF
	bool m_bEnableDOF;
	float m_fDOFFocusDistance;
	float m_fDOFNearStart;
	float m_fDOFNearEnd;
	float m_fDOFFarStart;
	float m_fDOFFarEnd;
	float m_fNearBlur;
	float m_fFarBlur;

	// Environment Effect
	std::string m_szEnvironmentEffect;

public:
	bool IsOldVersion(int nMajorVersion, int nMinorVersion);

	void Reset();
	void AddLight( SLightInfo &Info );
	void RemoveLight(int nIndex);
	bool Load(const char* szFileName);

	bool LoadNonFileHeader(const char* szFileName);	//Attempted to load a file without a header
	bool LoadOldVersion(const char* szFileName);	//Attempt to load item without version info
	void LoadVersion1_0( CResMngStream *Stream );
	void LoadVersion1_1( CResMngStream *Stream );
	void LoadVersion1_2( CResMngStream *Stream );
	
	void Save(const char* szFileName);

	void SaveVersion1_0( CFileStream &Stream );
	void SaveVersion1_1( CFileStream &Stream );
	void SaveVersion1_2( CFileStream &Stream );

	DWORD GetLightCount();
	CEnvInfo::LightInfoStruct *GetLight( DWORD dwIndex );


	void SetName( const char* szName ) { m_szName = szName; }
	void SetSkyBox( const char* szName ) { m_szSkyBox = szName; }
	void SetAmbient( DWORD dwColor ) { m_dwAmbient = dwColor; }
	void SetFogColor( DWORD dwColor ) { m_dwFogColor = dwColor; }
	void SetFogNear( float fValue ) { m_fFogNear = fValue; }
	void SetFogFar( float fValue ) { m_fFogFar = fValue; }
	void SetEnvironmentTexture( const char* szStr ) { m_szEnvironmentTexture = szStr; }
	void SetColorAdjShadow( EtVector4 &vVector ) { m_vColorAdjShadow = vVector; }
	void SetColorAdjMidtones( EtVector4 &vVector ) { m_vColorAdjMidtones = vVector; }
	void SetColorAdjHighlights( EtVector4 &vVector ) { m_vColorAdjHighlights = vVector; }
	void SetColorAdjSaturation( float fValue ) { m_fSaturation = fValue; }
	void SetEnviBGM( const char* szName ) { m_szEnviBGM = szName; }
	void SetEnviBGMRatio( float fValue ) { m_fEnviBGMRatio = fValue; }

	void SetBloomScale( float fValue ) { m_fBloomScale = fValue; }
	void SetBloomSensitivity( float fValue ) { m_fBloomSensitivity = fValue ; }
	void SetLightmapBlurSize( int nValue ) { m_nLightmapBlurSize = nValue; }

	void EnableColorFilter( bool bEnable ) { m_bEnableColorFilter = bEnable; }
	void EnableBloomFilter( bool bEnable ) { m_bEnableBloomFilter = bEnable; }

	void EnableWeather( bool bEnable ) { m_bEnableWeather = bEnable; }
	void SetWeatherTexture( const char* szStr ) { m_szWeatherTexture = szStr; }
	void SetWeatherSpeed( float fValue ) { m_fWeatherSpeed = fValue; }
	void SetWeatherStretch( float fValue ) { m_fWeatherStretch = fValue; }
	void SetWeatherLayerCount( int nValue ) { m_nWeatherLayerCount = nValue; }
	void SetWeatherLayerDecreaseRate( float fValue ) { m_fWeatherLayerDecreaseRate = fValue; }
	void SetWeatherLayerDir( EtVector3 &vVector ) { m_vWeatherLayerDir = vVector; }

	void EnableSplashDrop( bool bEnable ) { m_bEnableSplashDrop = bEnable; }
	void SetSplashDropTexture( const char* szStr ) { m_szSplashDropTexture = szStr; }
	void SetSplashDropSize( float fValue ) { m_fSplashDropSize = fValue; }
	void SetSplashDropLifeTime( float fValue ) { m_fSplashDropLifeTime = fValue; }
	void SetSplashDropRadius( int nValue ) { m_nSplashDropRadius = nValue; }
	void SetSplashDropCount( int nValue ) { m_nSplashDropCount = nValue; }

	void EnableSnow( bool bEnable ) { m_bEnableSnowEffect = bEnable; }
	void SetSnowTexture( const char* szStr ) { m_szSnowEffectTexture = szStr; }
	void SetSnowCount( int nValue ) { m_nSnowCount = nValue; }
	void SetSnowTimeOut( int nValue ) { m_nSnowTimeOut = nValue; }
	void SetSnowHeight( float fValue ) { m_fSnowHeight = fValue; }
	void SetIsUp(bool fValue) { m_isUp = fValue; }
	void SetSnowSize(float fSnowSize) { m_fSnowSize = fSnowSize; }
	void SetSnowAniSpeed(float fSnowAniSpeed) { m_fSnowAniSpeed = fSnowAniSpeed; }

	void EnableHeatHaze( bool bEnable ) { m_bEnableHeatHaze = bEnable; }
	void SetHeatHazeDistortionPower( float fValue ) { m_fHeatHazeDistortionPower = fValue; }
	void SetHeatHazeWaveSpeed( float fValue ) { m_fHeatHazeWaveSpeed = fValue; }

	void EnableDOF( bool bEnable ) { m_bEnableDOF = bEnable; }
	void SetDOFFocusDistance( float fValue ) { m_fDOFFocusDistance = fValue; }
	void SetDOFNearStart( float fValue ) { m_fDOFNearStart = fValue; }
	void SetDOFNearEnd( float fValue ) { m_fDOFNearEnd = fValue; }
	void SetDOFFarStart( float fValue ) { m_fDOFFarStart = fValue; }
	void SetDOFFarEnd( float fValue ) { m_fDOFFarEnd = fValue; }
	void SetDOFNearBlur( float fValue ) { m_fNearBlur = fValue; }
	void SetDOFFarBlur( float fValue ) { m_fFarBlur = fValue; }

	void SetEnvironmentEffect( const char* szStr ) { m_szEnvironmentEffect = szStr; }

	//////////////////////////////////////////////////////////
	const char* GetName() { return m_szName.c_str(); }
	const char* GetSkyBox() { return m_szSkyBox.c_str(); }
	DWORD GetAmbient() { return m_dwAmbient; }
	DWORD GetFogColor() { return m_dwFogColor; }
	float GetFogNear() { return m_fFogNear; }
	float GetFogFar() { return m_fFogFar; }
	const char* GetEnvironmentTexture() { return m_szEnvironmentTexture.c_str(); }
	EtVector4 *GetColorAdjShadow() { return &m_vColorAdjShadow; }
	EtVector4 *GetColorAdjMidtones() { return &m_vColorAdjMidtones; }
	EtVector4 *GetColorAdjHighlights() { return &m_vColorAdjHighlights; }
	const EtVector4 *GetColorAdjHighlights() const { return &m_vColorAdjHighlights; }
	float GetColorAdjSaturation() const { return m_fSaturation; }
	float GetBloomScale() { return m_fBloomScale; }
	float GetBloomSensitivity() { return m_fBloomSensitivity; }
	int GetLightmapBlurSize() { return m_nLightmapBlurSize; }
	const char* GetEnviBGM() { return m_szEnviBGM.c_str(); }
	float GetEnviBGMRatio() { return m_fEnviBGMRatio; }
	bool IsEnableColorFilter() { return m_bEnableColorFilter; }
	bool IsEnableBloomFilter() { return m_bEnableBloomFilter; }

	bool IsEnableWeather() { return m_bEnableWeather; }
	const char* GetWeatherTexture() { return m_szWeatherTexture.c_str(); }
	float GetWeatherSpeed() { return m_fWeatherSpeed; }
	float GetWeatherStretch() { return m_fWeatherStretch; }
	int GetWeatherLayerCount() { return m_nWeatherLayerCount; }
	float GetWeatherLayerDecreaseRate() { return m_fWeatherLayerDecreaseRate; }
	EtVector3 *GetWeatherLayerDir() { return &m_vWeatherLayerDir; }

	bool IsEnableSplashDrop() { return m_bEnableSplashDrop; }
	const char* GetSplashDropTexture() { return m_szSplashDropTexture.c_str(); }
	float GetSplashDropSize() { return m_fSplashDropSize; }
	float GetSplashDropLifeTime() { return m_fSplashDropLifeTime; }
	int GetSplashDropRadius() { return m_nSplashDropRadius; }
	int GetSplashDropCount() { return m_nSplashDropCount; }

	bool IsEnableSnow() { return m_bEnableSnowEffect; }
	const char* GetSnowTexture() { return m_szSnowEffectTexture.c_str(); }
	int GetSnowCount() { return m_nSnowCount; }
	int GetSnowTimeOut() { return m_nSnowTimeOut; }
	float GetSnowHeight() { return m_fSnowHeight; }
	bool GetIsUp() { return m_isUp; }
	float GetSnowSize() { return m_fSnowSize; }
	float GetSnowAniSpeed() { return m_fSnowAniSpeed; }
	
	bool IsEnableHeatHaze() { return m_bEnableHeatHaze; }
	float GetHeatHazeDistortionPower() { return m_fHeatHazeDistortionPower; }
	float GetHeatHazeWaveSpeed() { return m_fHeatHazeWaveSpeed; }

	bool IsEnableDOF() const { return m_bEnableDOF; }
	float GetDOFFocusDistance() const { return m_fDOFFocusDistance; }
	float GetDOFNearStart() const { return m_fDOFNearStart; }
	float GetDOFNearEnd() const { return m_fDOFNearEnd; }
	float GetDOFFarStart() const { return m_fDOFFarStart; }
	float GetDOFFarEnd() const { return m_fDOFFarEnd; }
	float GetDOFNearBlur() const { return m_fNearBlur; }
	float GetDOFFarBlur() const { return m_fFarBlur; }	

	const char* GetEnvironmentEffect() { return m_szEnvironmentEffect.c_str(); }
};
