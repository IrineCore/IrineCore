#pragma once

#include "Singleton.h"
#include "DnPacket.h"

/*
Class collecting global variables. Values in this class are excluded from the final build version.
Variables that must be stored globally, other than debug/cheat values, should be set in CGlobalInfo.
*/

class CFrameSync;
class CGlobalValue : public CSingleton<CGlobalValue> {
public:
	CGlobalValue();
	~CGlobalValue();

public:
	tstring m_szID;
	tstring m_szPass;
	tstring m_szServerName;
	tstring m_szSecondPass;
	int m_nChannelIndex;

	std::string m_szNation;
	std::string m_szResourcePath;
	bool m_bSingleTest;
	bool m_bNoDamage;
	int m_nLocalPlayerClassID;
	
	tstring m_szSingleMapName;
	tstring m_szSingleMapEnviName;
	int m_nSingleMapArrayIndex;

	bool m_bSoundOff;
	int m_nPlayerLevel;
	int m_nMapLevel;
	bool m_bIgnoreCoolTime;

	int m_nNation;

	std::string m_szRequestTime;
	std::string m_szKeyID;
	
	bool m_bShowDamage;
	bool m_bShowCP;
	bool m_bShowCurrentAction;
	bool m_bShowFPS;
	bool m_bShowDPS;
	bool m_bShowSkillDPS;
	UINT m_nSumDPS;
	float m_fDPSTime;
	DWORD m_dwBeginFPSTime;
	DWORD m_dwEndFPSTime;
	float m_fMinFPS;
	float m_fMaxFPS;
	float m_fAverageFPS;
	DWORD m_dwFPSCount;

	// Picking position for debugging
	EtVector3 m_vOutPos;
	bool	  m_bPicked;
	//---------------------
	// Dev utility code;
	bool m_bUseLogWnd;
	int m_nDumpLevel;

	int m_iLogLevel;

	bool m_bPVPGameIgnoreCondition;

#ifdef _SHADOW_TEST
	bool m_bEnableShadow;
#endif
	bool m_bEnableWinKey;

public:
	bool IsPermitLogin();
	void ProcessFPSLog( CFrameSync *pSync );

#ifdef PRE_FIX_SHOWVER
#else
	std::string MakeVersionString();
#endif

	//blondy 
	//Log respawn point info for debugging
	bool m_bEnableRespawnInfo;	
	std::vector<SCPVP_RESPAWN_POINT> m_vRespawnInfo;	
	//blondy end

	bool m_bDebugging;

	bool m_bDrawHitSignal;
	float m_fDrawHitSignalMinDelta;
//#ifndef _FINAL_BUILD
	bool IsComputerName( char *szName );
//#endif

	bool m_bMessageBoxWithMsgID;

	DWORD   m_dwSetMonsterID;
	bool	m_bShowMonsterAction;
	bool	m_bSetMonster;
	

	// #39851 Related to specifying 2 terrain textures for low-spec testing.
	int m_aiSelectedMapLayerTexture[ 2 ];
};

// Set variables that need to be stored globally here.
class CGlobalInfo : public CSingleton<CGlobalInfo>
{
public:
	CGlobalInfo();
	virtual ~CGlobalInfo();

	struct LoginConnectInfo {
		std::string szIP;
		short nPort;
	};
	std::vector<LoginConnectInfo> m_VecLoginConnectList;
	std::string m_szIP;
	std::string m_szPort;
//	short m_nPort;

	// Index of the map I am currently in
	int m_nCurrentMapIndex;	
	float m_fFadeDelta;
	LOCAL_TIME m_LocalTime;
	float	   m_fDeltaTime;

	std::wstring	 m_wszBuildVersion;

	char m_cLocalAccountLevel;

	int m_iGlobalMessageCode; //Added to handle messages globally
	bool m_bUsePacking;

	bool m_bPlayingCutScene;
	bool m_bThemida_hsbgen;
	bool m_bShowVersion;

	int m_nClientCharNameLenMax;

	bool m_bUITest;

#ifdef PRE_MOD_SELECT_CHAR
	std::map<int, std::wstring>		m_mapServerList;			// Server list
	int								m_nSelectedServerIndex;		// Selected server index
#endif // PRE_MOD_SELECT_CHAR

public:
	void SetGlobalMessageCode( int lMessageCode ){m_iGlobalMessageCode = lMessageCode;};
	int GetGlobalMessageCode(){return m_iGlobalMessageCode;};

	DWORD GetLoginConnectInfoCount();
	LoginConnectInfo *GetLoginConnectInfo( DWORD dwIndex );
	void CalcLoginConnectInfo();

	void SetPlayingCutScene( bool bPlaying ) { m_bPlayingCutScene = bPlaying; };
	bool IsPlayingCutScene( void ) { return m_bPlayingCutScene; };

#ifdef PRE_MOD_SELECT_CHAR
	std::wstring GetServerName( int nIndex );
	int GetServerIndex( LPCWSTR szServerName );
#endif // PRE_MOD_SELECT_CHAR

#ifdef PRE_FIX_SHOWVER
	std::string MakeVersionString();
#endif
};