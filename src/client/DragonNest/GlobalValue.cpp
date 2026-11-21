#include "StdAfx.h"
#include "GlobalValue.h"
#include <mmsystem.h>
#include "FrameSync.h"
#include "DnBridgeTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
#ifndef _FINAL_BUILD
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char szText[256] = {0,};
	GetWindowTextA(hwnd, szText, 255);	

	if(strstr(szText, "(Stopped)")) {
		CGlobalValue::GetInstance().m_bDebugging = true;
	}
	if(strstr(szText, "(Debugging)")) {
		CGlobalValue::GetInstance().m_bDebugging = true;
	}
	if(strstr(szText, "(Running)")) {
		CGlobalValue::GetInstance().m_bDebugging = true;
	}
	if(strstr(szText, "(Debugging)")) {
		CGlobalValue::GetInstance().m_bDebugging = true;
	}
	if(strstr(szText, "(Running)")) {
		CGlobalValue::GetInstance().m_bDebugging = true;
	}
	return TRUE;
}
#endif

CGlobalValue::CGlobalValue()
{
	m_bSingleTest = false;
	m_bSoundOff = false;
	m_bNoDamage = false;

	TCHAR szBuf[6] = { 0, };
	m_szID = _T("");
	m_szID += szBuf;

	m_szPass = _T("");
	m_szSecondPass = _T("");

	m_szID.reserve( IDLENMAX );
	m_szPass.reserve( PASSWORDLENMAX );
	
	m_szSingleMapName = _T("Siva_TestMap");
	m_szSingleMapEnviName = _T("Shine");

	m_bUseLogWnd = false;

	m_nPlayerLevel = 1;
	m_nMapLevel = 1;
	m_nSingleMapArrayIndex = -1;

	m_szServerName = _T("");
	m_nChannelIndex = -1;
	m_nDumpLevel = 1;
	m_vOutPos.x = 0.0f;
	m_vOutPos.y = 0.0f;
	m_vOutPos.z = 0.0f;
	m_bPicked = false;
	m_bIgnoreCoolTime = false;

	m_bShowDamage = false;
	m_bShowCP = false;
	m_bShowCurrentAction = false;
	m_bShowDPS = false;
	m_bShowSkillDPS = false;
	m_nSumDPS = 0;
	m_fDPSTime = 0.f;

#ifdef _DEBUG
	m_bShowFPS = true;
#else
	m_bShowFPS = false;
#endif 
	m_nLocalPlayerClassID = 1;

#ifdef _SHADOW_TEST
	 m_bEnableShadow = false;
#endif

	 m_dwBeginFPSTime = 0;
	 m_dwEndFPSTime = 0;
	 m_fMinFPS = 0.f;
	 m_fMaxFPS = 0.f;
	 m_fAverageFPS = 0.f;
	 m_dwFPSCount = 0;

	 m_bPVPGameIgnoreCondition = false;
	 m_bEnableWinKey = false;

	 m_bEnableRespawnInfo = false;

	 m_bDebugging = false;
#ifndef _FINAL_BUILD
	 ::EnumWindows( EnumWindowsProc, NULL);
#endif
	 m_bDrawHitSignal = false;
	 m_fDrawHitSignalMinDelta = 1.f;

	 SecureZeroMemory( m_aiSelectedMapLayerTexture, sizeof(m_aiSelectedMapLayerTexture) );

	 m_iLogLevel = 0;
	 m_nNation = 0;

	 m_bMessageBoxWithMsgID = false;

	 m_dwSetMonsterID = -1;
	 m_bSetMonster = false;
	 m_bShowMonsterAction = false;
}

CGlobalValue::~CGlobalValue()
{
}

bool CGlobalValue::IsPermitLogin()
{
	if( !m_szID.empty() && !m_szPass.empty() && !m_szServerName.empty() && (m_nChannelIndex>=0) && !m_szSecondPass.empty() )
		return true;
	else
		return false;
}

void CGlobalValue::ProcessFPSLog( CFrameSync *pSync )
{
	m_dwFPSCount++;
	float fFps = pSync->GetFps();
	m_fAverageFPS += fFps;
	if( fFps < m_fMinFPS ) m_fMinFPS = fFps;
	if( fFps > m_fMaxFPS ) m_fMaxFPS = fFps;
}

#ifdef PRE_FIX_SHOWVER
#else
std::string CGlobalValue::MakeVersionString()
{
	char szVersionStr[256];
	sprintf_s( szVersionStr, "%s %d.%d.%s(%d).%d.%d Server(%s)", g_szBuildString, g_nBuildVersionMajor, g_nBuildVersionMinor, g_szBuildVersionRevision,
		g_nBuildCount, g_nBuildSourceRevision, g_nBuildResourceRevision, (CDnBridgeTask::IsActive() ) ? CDnBridgeTask::GetInstance().GetCurrentServerVersion() : "Unknown" );

	return std::string(szVersionStr);
}
#endif

//#ifndef _FINAL_BUILD
bool CGlobalValue::IsComputerName( char *szComputerName )
{
	static char name[255]={0,};
	if(name[0]=='\0') {
		int result = ::gethostname(name, 255);
	}
	if( stricmp(name, szComputerName)==0) {
		return true;
	}
	return false;
}
//#endif

CGlobalInfo::CGlobalInfo()
{
	m_fFadeDelta = 0.8f;
	m_nCurrentMapIndex = -1;
	m_LocalTime = 0;
	m_fDeltaTime = 0;

//	m_nPort = 0;

	m_iGlobalMessageCode = 0;
	m_cLocalAccountLevel = 0;

	m_bThemida_hsbgen = false;
#ifdef _FINAL_BUILD
	m_bUsePacking = true;
#else
	m_bUsePacking = false;
#endif

	m_bPlayingCutScene = false;
	m_bShowVersion = false;

	m_nClientCharNameLenMax = 10;

	m_bUITest = false;

#ifdef PRE_MOD_SELECT_CHAR
	m_nSelectedServerIndex = -1;
#endif // PRE_MOD_SELECT_CHAR


}

CGlobalInfo::~CGlobalInfo()
{
	m_VecLoginConnectList.clear();
#ifdef PRE_MOD_SELECT_CHAR
	m_mapServerList.clear();
#endif // PRE_MOD_SELECT_CHAR
}

DWORD CGlobalInfo::GetLoginConnectInfoCount()
{
	return (DWORD)m_VecLoginConnectList.size();
}

CGlobalInfo::LoginConnectInfo *CGlobalInfo::GetLoginConnectInfo( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_VecLoginConnectList.size() ) return NULL;
	return &m_VecLoginConnectList[dwIndex];
}

void CGlobalInfo::CalcLoginConnectInfo()
{
	m_VecLoginConnectList.clear();

	for( int i=0; ; i++ )
	{
		LoginConnectInfo Info;
		const char *szIP = _GetSubStrByCount( i, m_szIP.c_str(), ';' );
		if( szIP == NULL ) break;
		Info.szIP = szIP;

		const char *szPort = _GetSubStrByCount( i, m_szPort.c_str(), ';' );
		if( szPort == NULL ) break;
		Info.nPort = atoi(szPort);

		m_VecLoginConnectList.push_back( Info );
	}
}

#ifdef PRE_MOD_SELECT_CHAR

std::wstring CGlobalInfo::GetServerName( int nIndex )
{
	std::map<int, std::wstring>::iterator iter = m_mapServerList.find( nIndex );
	if( iter != m_mapServerList.end() )
	{
		return iter->second;
	}

	return std::wstring(L"");
}

int CGlobalInfo::GetServerIndex( LPCWSTR szServerName )
{
	std::map<int, std::wstring>::iterator iter = m_mapServerList.begin();
	for( ; iter!= m_mapServerList.end(); iter++ )
	{
		std::wstring strServerName = iter->second;
		if( wcsncmp( szServerName, strServerName.c_str(), wcslen( szServerName ) ) == 0 )
			return iter->first;
	}

	std::string strServerName;
	ToMultiString( std::wstring(szServerName), strServerName );
	CDebugSet::ToLogFile( "CDnServerListDlg::GetServerIndex, %s not found!", strServerName.c_str() );
	return -1;
}

#endif // PRE_MOD_SELECT_CHAR

#ifdef PRE_FIX_SHOWVER
std::string CGlobalInfo::MakeVersionString()
{
	char szVersionStr[256];
	sprintf_s( szVersionStr, "%s %d.%d.%s(%d).%d.%d Server(%s)", g_szBuildString, g_nBuildVersionMajor, g_nBuildVersionMinor, g_szBuildVersionRevision,
		g_nBuildCount, g_nBuildSourceRevision, g_nBuildResourceRevision, (CDnBridgeTask::IsActive() ) ? CDnBridgeTask::GetInstance().GetCurrentServerVersion() : "Unknown" );

	return std::string(szVersionStr);
}
#endif