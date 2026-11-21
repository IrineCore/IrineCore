#include "StdAfx.h"
#include "GameOption.h"
#include "DnWorld.h"
#include "DnMouseCursor.h"
#include "DnMainFrame.h"
#include "EtSoundEngine.h"
#include "EtOptionController.h"
#include "D3DDevice9/EtDevice.h"
#include "shlobj.h"
#include <Direct.h>
#ifdef _USE_VOICECHAT
#include "VoiceChatClient.h"
#include "DNVoiceChatTask.h"
#endif
#include "InputWrapper.h"
#include "DnGameControlComboDlg.h"
#include "DnGameControlTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 
CGameOption::CGameOption()
{
	int nScreenWidth = GetSystemMetrics( SM_CXSCREEN );
	int nScreenHeight = GetSystemMetrics( SM_CYSCREEN );
	if( ( nScreenHeight / ( float )nScreenWidth ) > 0.7f ) {
		m_nWidth = 1024;
		m_nHeight = 768;
	}
	else {
		m_nWidth = 1280;
		m_nHeight = 800;
	}
	m_bWindow = false;
	m_bVSync = true;

#ifdef PRE_ADD_37745
	m_bHideWeaponByBattleMode = false;
#endif

#ifdef PRE_ADD_CASH_AMULET
	m_bShowCashChatBalloon = true;
#endif
#ifdef 	PRE_ADD_VIEW_OPITION_PARTY_HEAL
	m_bShowPartyHealInfo = true;
#endif

	m_bHideQuestMarkByLevel = true;
	m_bEnableQuestNotifyPriority = true;

#ifdef PRE_ADD_AUTO_DICE
	m_bEnableAutoDice = false;
#endif
#ifdef PRE_ADD_CHAR_SATUS_RENEW
	m_nCharStatusPrimaryPage = 0;
	m_nCharStatusSecondaryPage = 0;
#endif

	m_nGraphicQuality = 0;
	m_nCurGraphicQuality = m_nGraphicQuality;
#ifdef PRE_MOD_OPTION_TEXTURE_QUALITY
	m_nTextureQuality = 0;
	m_nCurTextureQuality = m_nTextureQuality;
#endif // PRE_MOD_OPTION_TEXTURE_QUALITY

#ifdef PRE_MOD_OPTION_EFFECT_QUALITY
	m_nEffectQuality	= 0;
	m_nCurEffectQuality = 0;
#endif
	m_nBrightness = 0;
	m_nUISize = 2;	// Stage 1~4
	m_nScreenShotFormat = D3DXIFF_JPG;

#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	m_nMemoryOptimize = 1;
#endif

	m_fMouseSensitivity = 0.07f;
	m_fCursorSensitivity = 0.1f;
	m_fCameraSensitivity = 0.27f;
	m_bSmartMove = true;
	m_fDefaultMouseSensitivity = m_fMouseSensitivity;
	m_bDefaultSmartMove = m_bSmartMove;
	m_bSmartMoveMainMenu = false;
	m_bDefaultSmartMoveMainMenu = m_bSmartMoveMainMenu;

	m_fTotalVolume = 1.f;
	m_fBGMVolume = 1.f;
	m_f3DVolume = 1.f;
	m_f2DVolume = 1.f;
	m_fVoiceVolume = 1.f;

	m_bTotalMute = false;
	m_bBGMMute = false;
	m_b3DMute = false;
	m_b2DMute = false;
	m_bVoiceMute = false;

	m_nSoundQuality = 0;

	m_bMouseInvert = false;

	m_nInputDevice = 0;

	SetDefault();

	for( int i = 0; i < NUM_COMM_MACROS; i++) {
		m_bDefaultMacro[ i ] = true;
		m_wszMacro[ i ][ 0 ] = L'\0';
	}

	// Note : Help options
	//
	m_bHelpCaption = true;

	m_nRecordResolution = 1;
	m_nRecordFPS = 1;
	m_nRecordVideoQuality = 1;
	m_nRecordAudioQuality = 2;

	m_bVoiceChat = false;
	m_fVoiceChatVolume = 0.5f;
	m_fMicVolume = 0.2f;
	m_bMicTest = false;
	m_bMicBoost = false;
	m_bUseAutoAdjustVolume = false;
	m_fAutoAdjustVolume = 0.1f;
	m_nVoiceFont = 0;
	m_fVoicePitch = 1.0f;
	m_fVoiceTimbre = 1.0f;

	m_bCommGuildHideOffline = true;
	m_bCommFriendHideOffline = false;

	m_nWndPosX	= 0;
	m_nWndPosY	= 0;

	m_nMiniSiteVersion = 0;

#ifdef PRE_MOD_CHATBG
	m_bChatBackground = false;
#endif

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	m_bEnablePlayerEffect = true;
#endif


#ifdef PRE_ADD_STAMPSYSTEM
	m_nWeekDay = -1;
	m_nShowStampAlram = -1;
#endif // PRE_ADD_STAMPSYSTEM

}

CGameOption::~CGameOption()
{
}

void CGameOption::CheckValidGraphicOption()
{
	int nVSVersion = ( GetEtDevice()->GetVSVersion() & 0xffff ) >> 8;
	int nPSVersion = ( GetEtDevice()->GetPSVersion() & 0xffff ) >> 8;

	if( ( nVSVersion < 3 ) || ( nPSVersion < 3 ) )
	{
		m_nGraphicQuality = Graphic_Low;
		m_nCurGraphicQuality = m_nGraphicQuality;

		SGraphicOption Option;
		Option = GetGraphicOption();
		GetEtOptionController()->Initialize( Option );
	}
}

bool CGameOption::Load()
{
	FILE *stream = NULL;

	char szString[256];
	char szFilename[MAX_PATH];

	TCHAR pBuffer[MAX_PATH]={0};
	char szPath[_MAX_PATH] = { 0, };
	SHGetSpecialFolderPath(CDnMainFrame::GetInstance().GetHWnd(), pBuffer, CSIDL_PERSONAL, 0);
	WideCharToMultiByte( CP_ACP, 0, pBuffer, -1, szPath, sizeof(szPath), NULL, NULL );

	sprintf_s( szFilename, "%s\\DragonNest", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config\\%s", szPath, "Config.ini" );

#ifndef _FINAL_BUILD
	sprintf_s( szFilename, 256, ".\\Config.ini" );
#endif

	if( fopen_s( &stream, szFilename, "r+" ) != 0 )
		return false;

	fseek( stream, 0L, SEEK_SET );

	for( int i = 0; ;i++ )
	{
		int numRead = fscanf( stream, "%s", szString );
		if( numRead != 1 ) break;
		SetOptionValue( stream , szString );
	}

	fclose( stream );	

	if( m_nWidth < MIN_RES_WIDTH ) {
		m_nWidth = MIN_RES_WIDTH;
		m_nHeight = MIN_RES_HEIGHT;
	}
	
	return LoadMacro();
}

bool CGameOption::LoadMacro()
{
	FILE *stream = NULL;
	char szString[256];
	char szFilename[MAX_PATH];

#ifndef _FINAL_BUILD
	sprintf_s( szFilename, 256, ".\\Config_Macro.ini" );
#else
	TCHAR pBuffer[MAX_PATH]={0};
	char szPath[_MAX_PATH] = { 0, };
	SHGetSpecialFolderPath(CDnMainFrame::GetInstance().GetHWnd(), pBuffer, CSIDL_PERSONAL, 0);
	WideCharToMultiByte( CP_ACP, 0, pBuffer, -1, szPath, sizeof(szPath), NULL, NULL );

	sprintf_s( szFilename, "%s\\DragonNest", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config\\%s", szPath, "Config_Macro.ini" );
#endif

	if( fopen_s( &stream, szFilename, "r+" ) != 0 )
		return false;

	fseek( stream, 0L, SEEK_SET );

	for( int i = 0; ;i++ )
	{
		int numRead = fscanf( stream, "%s", szString );
		if( numRead != 1 ) break;
		SetOptionValue( stream , szString );
	}

	fclose( stream );

	return true;
}

//blondy text-based option file 
bool CGameOption::SetOptionValue( FILE *stream , char * szString )
{
	if( !szString)
		return false;

	if( strcmp("Width",szString) == 0 )
	{
		fscanf( stream, "%d", &m_nWidth );
		return true;

	}else if( strcmp("Height",szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nHeight );
		return true;

	}else if( strcmp("GraphicQuality",szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nGraphicQuality );
		m_nCurGraphicQuality = m_nGraphicQuality;
		return true;

	}
#ifdef PRE_MOD_OPTION_TEXTURE_QUALITY
	else if( strcmp("TextureQuality",szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nTextureQuality );
		m_nCurTextureQuality = m_nTextureQuality;
		return true;
	}
#endif // PRE_MOD_OPTION_TEXTURE_QUALITY

#ifdef PRE_MOD_OPTION_EFFECT_QUALITY
	else if( strcmp("EffectQuality",szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nEffectQuality );
		m_nCurEffectQuality = m_nEffectQuality;
		return true;
	}
#endif
	else if( strcmp("MouseSensitivity",szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fMouseSensitivity );
		return true;

	}else if( strcmp( "Brightness",szString) == 0 )
	{
		fscanf( stream, "%d", &m_nBrightness );
		return true;

	}else if( strcmp( "TotalVolume",szString) == 0 )
	{
		fscanf( stream, "%f", &m_fTotalVolume );
		return true;

	}else if( strcmp( "BGMvolume",szString) == 0 )
	{
		fscanf( stream, "%f", &m_fBGMVolume );
		return true;

	}else if( strcmp("UIVolume(2D)",szString) == 0 )
	{
		fscanf( stream, "%f", &m_f2DVolume );
		return true;

	}else if( strcmp("EffectVolume(3D)",szString ) == 0 )
	{
		fscanf( stream, "%f", &m_f3DVolume );
		return true;

	}else if( strcmp("TotalMute",szString ) == 0 )
	{
		int iTotalMute = 0;
		fscanf( stream, "%d", &iTotalMute );
		m_bTotalMute = iTotalMute?true:false;
		return true;

	}else if( strcmp("BGMmute",szString ) == 0 )
	{
		int iBGMMute = 0;
		fscanf( stream, "%d", &iBGMMute );
		m_bBGMMute = iBGMMute?true:false;
		return true;

	}else if( strcmp("Mute2D",szString ) == 0 )
	{
		int i2DMute = 0;
		fscanf( stream, "%d", &i2DMute );
		m_b2DMute = i2DMute?true:false;
		return true;

	}else if( strcmp("Mute3D",szString ) == 0 )
	{
		int i3DMute = 0;
		fscanf( stream, "%d", &i3DMute );
		m_b3DMute = i3DMute?true:false;
		return true;

	}
	else if( strcmp("WindowMode",szString ) == 0 )
	{
		int iWindowMode = 0;
		fscanf( stream, "%d", &iWindowMode );
		m_bWindow = iWindowMode?true:false;
		return true;
	}
	else if( strcmp( "VSync", szString ) == 0 )
	{
		int iVSync = 0;
		fscanf( stream, "%d", &iVSync );
		m_bVSync = iVSync?true:false;
		return true;
	}
#ifdef 	PRE_ADD_37745
	else if(strcmp("HideWeaponByBattleMode",szString) == 0)
	{
		int iHideWeaponByBattleMode = 0;
		fscanf( stream, "%d", &iHideWeaponByBattleMode );
		m_bHideWeaponByBattleMode = iHideWeaponByBattleMode?true:false;
		return true;

	}
#endif
#ifdef 	PRE_ADD_CASH_AMULET
	else if(strcmp("ShowCashChatBalloon",szString) == 0)
	{
		int iShowCashChatBalloon = 0;
		fscanf( stream, "%d", &iShowCashChatBalloon );
		m_bShowCashChatBalloon = iShowCashChatBalloon?true:false;
		return true;

	}
#endif
#ifdef 	PRE_ADD_VIEW_OPITION_PARTY_HEAL
	else if(strcmp("ShowPartyHealInfo",szString) == 0)
	{
		int iShowPartyHealInfo = 0;
		fscanf( stream, "%d", &iShowPartyHealInfo );
		m_bShowPartyHealInfo = iShowPartyHealInfo?true:false;
		return true;

	}
#endif
	else if(strcmp("HideQuestMark",szString) == 0)
	{
		int iHideQuest = 0;
		fscanf( stream, "%d", &iHideQuest );
		m_bHideQuestMarkByLevel = iHideQuest?true:false;
		return true;
	}
	else if(strcmp("EnalbeQuestNotifyPriority",szString) == 0)
	{
		int nEnalbe = 0;
		fscanf( stream, "%d", &nEnalbe );
		m_bEnableQuestNotifyPriority = nEnalbe?true:false;
		return true;
	}
#ifdef PRE_ADD_AUTO_DICE
	else if(strcmp("EnalbeAutoDice",szString) == 0)
	{
		int nEnalbe = 0;
		fscanf( stream, "%d", &nEnalbe );
		m_bEnableAutoDice = nEnalbe?true:false;
		return true;
	}
#endif
#ifdef PRE_ADD_CHAR_SATUS_RENEW
	else if(strcmp("StatusPrimaryPage",szString) == 0)
	{
		int nPage = 0;
		fscanf( stream, "%d", &nPage );
		m_nCharStatusPrimaryPage = nPage;
		return true;
	}
	else if(strcmp("StatusSecondaryPage",szString) == 0)
	{
		int nPage = 0;
		fscanf( stream, "%d", &nPage );
		m_nCharStatusSecondaryPage = nPage;
		return true;
	}
#endif
	else if( strcmp("VoiceVol",szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fVoiceVolume );
		return true;


	}else if( strcmp("VoiceMute",szString ) == 0 )
	{
		int iVoiceMute = 0;
		fscanf( stream, "%d", &iVoiceMute );
		m_bVoiceMute = iVoiceMute?true:false;
		return true;

	}else if( strcmp("UseSmartMove",szString ) == 0 )
	{
		int iUseSmartMove = 0;
		fscanf( stream, "%d", &iUseSmartMove );
		m_bSmartMove = iUseSmartMove?true:false;
		return true;

	}else if( strcmp("UseSmartMoveEx",szString ) == 0 )
	{
		int iUseSmartMoveEx = 0;
		fscanf( stream, "%d", &iUseSmartMoveEx );
		m_bSmartMoveMainMenu = iUseSmartMoveEx?true:false;
		return true;

	}else if( strcmp("CommFriendHideOffline",szString ) == 0 )
	{
		int iCommFriendHideOffline = 0;
		fscanf( stream, "%d", &iCommFriendHideOffline );
		m_bCommFriendHideOffline = iCommFriendHideOffline?true:false;
		return true;

	}
	else if( strcmp("CommGuildHideOffline",szString ) == 0 )
	{
		int iCommGuildHideOffline = 0;
		fscanf( stream, "%d", &iCommGuildHideOffline );
		m_bCommGuildHideOffline = iCommGuildHideOffline?true:false;
		return true;

	}
	else if( strcmp("UISize",szString ) == 0 )
	{
		int iUISize = 0;
		fscanf( stream, "%d", &iUISize );
		m_nUISize = iUISize;
		if( m_nUISize < 1 || m_nUISize > 4 ) m_nUISize = 2;
		return true;
	}
	else if( strcmp( "ScreenShotFormat", szString ) == 0 )
	{
		int iScreenShotFormat = 0;
		fscanf( stream, "%d", &iScreenShotFormat );
		m_nScreenShotFormat = iScreenShotFormat;
		return true;
	}
#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	else if( strcmp("MemOpt", szString ) == 0 )
	{
		int iMemOpt = 0;
		fscanf( stream, "%d", &iMemOpt );
		m_nMemoryOptimize = iMemOpt;
		if( m_nMemoryOptimize < 1 || m_nMemoryOptimize > 6 ) m_nMemoryOptimize = 1;
		return true;
	}
#endif
	else if( strcmp( "RecordResolution", szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nRecordResolution );
	}
	else if( strcmp( "RecordFPS", szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nRecordFPS );
	}
	else if( strcmp( "RecordVideoQuality", szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nRecordVideoQuality );
	}
	else if( strcmp( "RecordAudioQuality", szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nRecordAudioQuality );
	}
#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	else if( strcmp( "EnablePlayerEffect", szString ) == 0 )
	{
		int nEnalbe = 0;
		fscanf( stream, "%d", &nEnalbe );
		m_bEnablePlayerEffect = nEnalbe ? true : false;
	}
#endif
#ifdef PRE_ADD_STAMPSYSTEM
	else if( strcmp( "EventStamp", szString ) == 0 )
	{
		int nValue = 0;
		fscanf( stream, "%d", &nValue );
		m_nWeekDay = nValue;
	}
	else if( strcmp( "EventStampAlram", szString ) == 0 )
	{
		int nValue = 0;
		fscanf( stream, "%d", &nValue );
		m_nShowStampAlram = nValue;
	}
#endif // PRE_ADD_STAMPSYSTEM

#ifdef _USE_VOICECHAT
	else if( strcmp( "UseVoiceChat", szString ) == 0 )
	{
		int iUseVoiceChat = 0;
		fscanf( stream, "%d", &iUseVoiceChat );
		m_bVoiceChat = iUseVoiceChat?true:false;
		return true;
	}
	else if( strcmp( "VoiceChatVolume", szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fVoiceChatVolume );
		return true;
	}
	else if( strcmp( "MicVolume", szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fMicVolume );
		return true;
	}
	else if( strcmp( "MicTest", szString ) == 0 )
	{
		int iMicTest = 0;
		fscanf( stream, "%d", &iMicTest );
		m_bMicTest = iMicTest?true:false;
		return true;
	}
	else if( strcmp( "MicBoost", szString ) == 0 )
	{
		int iMicBoost = 0;
		fscanf( stream, "%d", &iMicBoost );
		m_bMicBoost = iMicBoost?true:false;
		return true;
	}
	else if( strcmp( "UseAutoAdjustVolume", szString ) == 0 )
	{
		int iAutoAdjustVolume = 0;
		fscanf( stream, "%d", &iAutoAdjustVolume );
		m_bUseAutoAdjustVolume = iAutoAdjustVolume?true:false;
		return true;
	}
	else if( strcmp( "AutoAdjustVolume", szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fAutoAdjustVolume );
		return true;
	}
	else if( strcmp( "VoiceFont", szString) == 0 )
	{
		fscanf( stream, "%d", &m_nVoiceFont );
		return true;
	}
	else if( strcmp( "VoiceFontPitch", szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fVoicePitch );
		return true;
	}
	else if( strcmp( "VoiceFontTimbre", szString ) == 0 )
	{
		fscanf( stream, "%f", &m_fVoiceTimbre );
		return true;
	}
#endif
	else if( strcmp("WindowPosX",szString) == 0 )
	{
		fscanf( stream, "%d", &m_nWndPosX );
		return true;

	}else if( strcmp("WindowPosY",szString ) == 0 )
	{
		fscanf( stream, "%d", &m_nWndPosY );
		return true;

	}
	else if( strcmp("CursorSensitivity", szString) == 0 )
	{
		fscanf( stream, "%f", &m_fCursorSensitivity );
		return true;
	}
	else if( strcmp("CameraSensitivity", szString) == 0 )
	{
		fscanf( stream, "%f", &m_fCameraSensitivity );
		return true;
	}
	else if( strcmp("MouseInvert", szString) == 0 )
	{
		int iMouseInvert = 0;
		fscanf( stream, "%d", &iMouseInvert );
		m_bMouseInvert = iMouseInvert?true:false;
	}
	else if( strcmp("InputDevice", szString) == 0 )
	{
		fscanf( stream, "%d", &m_nInputDevice );
		return true;
	}
	else if( strstr( szString, "CommMacro" ) )
	{
		int nIndex = atoi(&szString[strlen(szString)-1]);

		int iDefaultMacro = 0;
		char szTemp[255*2];
		if( fscanf( stream, "%d", &iDefaultMacro ) == 1 )
		{
			m_bDefaultMacro[nIndex] = iDefaultMacro?true:false;
			if( iDefaultMacro == 0 )
			{
				fseek( stream, 1, SEEK_CUR );	// After skipping whitespace
				fgets( szTemp, 255*2, stream );
				if( szTemp[strlen(szTemp)-1] == '\n' ) szTemp[strlen(szTemp)-1] = '\0';
				MultiByteToWideChar( CP_ACP, 0, szTemp, -1, m_wszMacro[nIndex], 255 );
			}
		}
		return true;
	}
	else if (strstr(szString, "MiniSiteVersion"))
	{
		fscanf(stream, "%d", &m_nMiniSiteVersion);
		return true;
	}
#ifdef PRE_MOD_CHATBG
	else if (strcmp(szString, "ChatBackground") == 0)
	{
		int iChatBackground = 0;
		fscanf( stream, "%d", &iChatBackground );
		m_bChatBackground = iChatBackground ? true:false;
		return true;
	}
#endif
	return false;
}

bool CGameOption::Save()
{
	FILE *stream = NULL;
	char szFilename[MAX_PATH];

#ifndef _FINAL_BUILD
	sprintf_s( szFilename, 256, ".\\Config.ini" );
#else
	TCHAR pBuffer[MAX_PATH]={0};
	char szPath[_MAX_PATH] = { 0, };
	SHGetSpecialFolderPath(CDnMainFrame::GetInstance().GetHWnd(), pBuffer, CSIDL_PERSONAL, 0);
	WideCharToMultiByte( CP_ACP, 0, pBuffer, -1, szPath, sizeof(szPath), NULL, NULL );

	sprintf_s( szFilename, "%s\\DragonNest", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config\\%s", szPath, "Config.ini" );
#endif

	// If read-only, set as normal file.
	if( GetFileAttributes( _T( "Config.ini" ) ) != INVALID_FILE_ATTRIBUTES )
		SetFileAttributes( _T( "Config.ini" ), FILE_ATTRIBUTE_NORMAL );

	if( fopen_s( &stream, szFilename, "w+" ) != 0 )
		return false;

	fseek( stream, 0L, SEEK_SET );

	fprintf( stream, "Width %d\n", m_nWidth );
	fprintf( stream, "Height %d\n", m_nHeight );
	fprintf( stream, "GraphicQuality %d\n", m_nGraphicQuality );
#ifdef PRE_MOD_OPTION_TEXTURE_QUALITY
	fprintf( stream, "TextureQuality %d\n", m_nTextureQuality );
#endif // PRE_MOD_OPTION_TEXTURE_QUALITY

#ifdef PRE_MOD_OPTION_EFFECT_QUALITY
	fprintf( stream, "EffectQuality %d\n", m_nEffectQuality );
#endif // PRE_MOD_OPTION_EFFECT_QUALITY

	fprintf( stream, "MouseSensitivity %f\n", m_fMouseSensitivity );
	fprintf( stream, "Brightness %d\n", m_nBrightness );
	fprintf( stream, "TotalVolume %f\n", m_fTotalVolume);
	fprintf( stream, "BGMvolume %f\n", m_fBGMVolume );
	fprintf( stream, "UIVolume(2D) %f\n", m_f2DVolume );
	fprintf( stream, "EffectVolume(3D) %f\n", m_f3DVolume );
	fprintf( stream, "TotalMute %d\n", m_bTotalMute?1:0 );
	fprintf( stream, "BGMmute %d\n", m_bBGMMute?1:0 );
	fprintf( stream, "Mute2D %d\n", m_b2DMute?1:0 );
	fprintf( stream, "Mute3D %d\n", m_b3DMute?1:0  );
	fprintf( stream, "WindowMode %d\n", m_bWindow?1:0  );
	fprintf( stream, "VSync %d\n", m_bVSync?1:0  );
#ifdef PRE_ADD_37745
	fprintf( stream, "HideWeaponByBattleMode %d\n", m_bHideWeaponByBattleMode?1:0  );
#endif
#ifdef 	PRE_ADD_CASH_AMULET
	fprintf( stream, "ShowCashChatBalloon %d\n", m_bShowCashChatBalloon?1:0  );
#endif
#ifdef 	PRE_ADD_VIEW_OPITION_PARTY_HEAL
	fprintf( stream, "ShowPartyHealInfo %d\n", m_bShowPartyHealInfo?1:0  ); ;
#endif
	fprintf( stream, "HideQuestMark %d\n", m_bHideQuestMarkByLevel?1:0  );
	fprintf( stream, "EnalbeQuestNotifyPriority %d\n", m_bEnableQuestNotifyPriority?1:0  );
#ifdef PRE_ADD_AUTO_DICE
	fprintf( stream, "EnalbeAutoDice %d\n", m_bEnableAutoDice?1:0  );
#endif
#ifdef PRE_ADD_CHAR_SATUS_RENEW
	fprintf( stream, "StatusPrimaryPage %d\n", m_nCharStatusPrimaryPage  );
	fprintf( stream, "StatusSecondaryPage %d\n", m_nCharStatusSecondaryPage  );
#endif
	fprintf( stream, "VoiceVol %f\n", m_fVoiceVolume );
	fprintf( stream, "VoiceMute %d\n", m_bVoiceMute?1:0  );
	fprintf( stream, "UseSmartMove %d\n", m_bSmartMove?1:0  );
	fprintf( stream, "UseSmartMoveEx %d\n", m_bSmartMoveMainMenu?1:0  );
	fprintf( stream, "CommFriendHideOffline %d\n", m_bCommFriendHideOffline?1:0  );
	fprintf( stream, "CommGuildHideOffline %d\n", m_bCommGuildHideOffline?1:0  );
	fprintf( stream, "UISize %d\n", m_nUISize );
	fprintf( stream, "ScreenShotFormat %d\n", m_nScreenShotFormat );
#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	fprintf( stream, "MemOpt %d\n", m_nMemoryOptimize );
#endif
	fprintf( stream, "RecordResolution %d\n", m_nRecordResolution );
	fprintf( stream, "RecordFPS %d\n", m_nRecordFPS );
	fprintf( stream, "RecordVideoQuality %d\n", m_nRecordVideoQuality );
	fprintf( stream, "RecordAudioQuality %d\n", m_nRecordAudioQuality );
#ifdef _USE_VOICECHAT
	fprintf( stream, "UseVoiceChat %d\n", m_bVoiceChat?1:0 );
	fprintf( stream, "VoiceChatVolume %f\n", m_fVoiceChatVolume );
	fprintf( stream, "MicVolume %f\n", m_fMicVolume );
	fprintf( stream, "MicTest %d\n", m_bMicTest?1:0 );
	fprintf( stream, "MicBoost %d\n", m_bMicBoost?1:0 );
	fprintf( stream, "UseAutoAdjustVolume %d\n", m_bUseAutoAdjustVolume?1:0 );
	fprintf( stream, "AutoAdjustVolume %f\n", m_fAutoAdjustVolume );
	fprintf( stream, "VoiceFont %d\n", m_nVoiceFont );
	fprintf( stream, "VoiceFontPitch %f\n", m_fVoicePitch );
	fprintf( stream, "VoiceFontTimbre %f\n", m_fVoiceTimbre );
#endif
	fprintf( stream, "WindowPosX %d\n", m_nWndPosX );
	fprintf( stream, "WindowPosY %d\n", m_nWndPosY );
	fprintf( stream, "CursorSensitivity %f\n", m_fCursorSensitivity );
	fprintf( stream, "CameraSensitivity %f\n", m_fCameraSensitivity );
	fprintf( stream, "MouseInvert %d\n", m_bMouseInvert?1:0 );

	fprintf( stream, "InputDevice %d\n", m_nInputDevice);
	fprintf( stream, "MiniSiteVersion %d\n", m_nMiniSiteVersion);
#ifdef PRE_MOD_CHATBG
	fprintf( stream, "ChatBackground %d\n", m_bChatBackground?1:0 );
#endif
#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	fprintf( stream, "EnablePlayerEffect %d\n", m_bEnablePlayerEffect?1:0 );
#endif
#ifdef PRE_ADD_STAMPSYSTEM
	fprintf( stream, "EventStamp %d\n", m_nWeekDay );
	fprintf( stream, "EventStampAlram %d\n", m_nShowStampAlram );
#endif // PRE_ADD_STAMPSYSTEM

	fclose( stream );

	return SaveMacro();

	/*
	CFileStream Stream( ".\\Config.dat", CFileStream::OPEN_WRITE );
	if( !Stream.IsValid() ) 
		return false;

	Stream.Write( &m_nWidth, sizeof(int) );
	Stream.Write( &m_nHeight, sizeof(int) );
	Stream.Write( &m_nGraphicQuality, sizeof(int) );
	Stream.Write( &m_fMouseSensitivity, sizeof(float) );
	Stream.Write( &m_nBrightness, sizeof(int) );
	Stream.Write( &m_fTotalVolume, sizeof(float) );
	Stream.Write( &m_fBGMVolume, sizeof(float) );
	Stream.Write( &m_f2DVolume, sizeof(float) );
	Stream.Write( &m_f3DVolume, sizeof(float) );
	Stream.Write( &m_bTotalMute, sizeof(bool) );
	Stream.Write( &m_bBGMMute, sizeof(bool) );
	Stream.Write( &m_b2DMute, sizeof(bool) );
	Stream.Write( &m_b3DMute, sizeof(bool) );
	Stream.Write( &m_bShowQuestNotifier, sizeof(bool) );
	Stream.Write( &m_bWindow, sizeof( bool ) );
	Stream.Write( &m_fVoiceVolume, sizeof(float) );
	Stream.Write( &m_bVoiceMute, sizeof(bool) );
	Stream.Write( &m_bSmartMove, sizeof(bool) );
	Stream.Write( &m_bCommFriendHideOffline, sizeof(bool) );

	return true;
	*/
}

bool CGameOption::SaveMacro()
{
	FILE *stream = NULL;
	char szFilename[MAX_PATH];

#ifndef _FINAL_BUILD
	sprintf_s( szFilename, 256, ".\\Config_Macro.ini" );
#else
	TCHAR pBuffer[MAX_PATH]={0};
	char szPath[_MAX_PATH] = { 0, };
	SHGetSpecialFolderPath(CDnMainFrame::GetInstance().GetHWnd(), pBuffer, CSIDL_PERSONAL, 0);
	WideCharToMultiByte( CP_ACP, 0, pBuffer, -1, szPath, sizeof(szPath), NULL, NULL );

	sprintf_s( szFilename, "%s\\DragonNest", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config", szPath );
	_mkdir( szFilename );
	sprintf_s( szFilename, "%s\\DragonNest\\Config\\%s", szPath, "Config_Macro.ini" );
#endif

	if( fopen_s( &stream, szFilename, "w+" ) != 0 )
		return false;

	fseek( stream, 0L, SEEK_SET );

	// Macro
	for( int i = 0; i < NUM_COMM_MACROS; i++) {
		fprintf( stream, "CommMacro%d %d ", i, m_bDefaultMacro[i]?1:0 );
		char szTemp[255*2];
		WideCharToMultiByte( CP_ACP, 0, m_wszMacro[i], -1, szTemp, 255*2, NULL, NULL );
		fputs( szTemp, stream );
		fputs( "\n", stream );
	}

	fclose( stream );

	return true;
}

//blondy end
void CGameOption::ApplyGraphicOption()
{
	CheckValidGraphicOption();
	GetEtDevice()->SetGammaRamp( 1.0f, m_nBrightness * 2 );
	CEtUIDialog::SetDialogSize( m_nUISize );
#ifdef PRE_ADD_ACTION_DYNAMIC_OPTIMIZE
	CDnActionBase::SetMemoryOptimize( m_nMemoryOptimize );
#endif
	CEtMaterial::SetShaderQuality( m_nGraphicQuality );
	// Since depth is baked only on High option.. enable occlusion..
	switch( m_nGraphicQuality )
	{
		case 0:
			CEtRenderStack::EnableOcclusion( true );
			CEtRenderStack::EnableDepthRender( true );
			break;
		case 1:
			CEtRenderStack::EnableOcclusion( false );
			CEtRenderStack::EnableDepthRender( false );
			break;
		case 2:
			CEtRenderStack::EnableOcclusion( false );
			CEtRenderStack::EnableDepthRender( false );
			break;
	}
	// Temporarily turned off as it has no effect..
//	CEtTerrainArea::EnableOptimize( true );
}

void CGameOption::ChangeGraphicQuality()
{
	SGraphicOption Option = GetGraphicOption();
	EternityEngine::SetGraphicOption( Option );

	// Jiho.. please check the graphics option here and hide the props..
	CEtMaterial::SetShaderQuality( m_nGraphicQuality );
	CEtMaterial::ReloadMaterial();

	if( CDnWorld::IsActive() ) 
		CDnWorld::GetInstance().RefreshQualityLevel( m_nGraphicQuality );
}

void CGameOption::ChangeDisplayMode()
{
	RECT rc;
	DWORD dwStyle = ( DWORD )GetWindowLong( CDnMainFrame::GetInstance().GetHWnd(), GWL_STYLE );
	DWORD dwStyleEx = ( DWORD )GetWindowLong( CDnMainFrame::GetInstance().GetHWnd(), GWL_EXSTYLE );
	if( m_bWindow ) 
	{
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_DRAGONNEST;

		dwStyleEx &= ~WS_EX_TOPMOST;
	}
	else
	{
		dwStyle &= ~WS_DRAGONNEST;
		dwStyle |= WS_POPUP;

		dwStyleEx |= WS_EX_TOPMOST;
	}
	SetWindowLong( CDnMainFrame::GetInstance().GetHWnd(), GWL_STYLE, ( LONG )dwStyle );
	SetWindowLong( CDnMainFrame::GetInstance().GetHWnd(), GWL_EXSTYLE, ( LONG )dwStyleEx );
	SetRect( &rc, 0, 0, CGameOption::GetInstance().m_nWidth, CGameOption::GetInstance().m_nHeight );
	AdjustWindowRectEx( &rc, dwStyle, FALSE, dwStyleEx );
	SetWindowPos( CDnMainFrame::GetInstance().GetHWnd(), HWND_NOTOPMOST, -1, -1, rc.right - rc.left, rc.bottom - rc.top, SWP_DRAWFRAME );

	GetEtDevice()->SetWindowMode( m_bWindow );
	GetEtDevice()->SetSize( m_nWidth, m_nHeight );
	GetEtDevice()->SetVSync( m_bVSync );
	CEtUIDialog::CalcDialogScaleByResolution( m_nWidth, m_nHeight );
	CEtUIDialog::ProcessChangeResolution();
	EternityEngine::ReinitializeEngine( m_nWidth, m_nHeight );
}

SGraphicOption CGameOption::GetGraphicOption()
{
	SGraphicOption Option;

	Option.DynamicShadowType = ST_DEPTHSHADOWMAP;
	switch( m_nGraphicQuality ) {
		case Graphic_High:
			Option.bEnableDOF = true;
			Option.bUseSplatting = true;
			Option.bDrawGrass = true;
			Option.WaterQuality = WQ_HIGH;
			Option.ShadowQuality = SQ_HIGH;
			Option.nMaxDirLightCount = 5;
			Option.nMaxPointLightCount = 5;
			Option.nMaxSpotLightcount = 5;
			Option.bUseTerrainLightMap = true;			// Prevent shader reload!
			Option.bSoftShadow = true;
			Option.bDecalShadow = false;
			Option.nGraphicQuality = 0;
			break;
		case Graphic_Middle:
			Option.bEnableDOF = false;
			Option.bUseSplatting = true;
			Option.bDrawGrass = true;
			Option.WaterQuality = WQ_NORMAL;	
			Option.ShadowQuality = SQ_NORMAL;
			Option.nMaxDirLightCount = 2;
			Option.nMaxPointLightCount = 2;
			Option.nMaxSpotLightcount = 1;
			Option.bUseTerrainLightMap = true;		// Prevent shader reload!
			Option.bSoftShadow = false;
			Option.bDecalShadow = false;
			Option.nGraphicQuality = 1;
			break;
		case Graphic_Low:
			Option.bEnableDOF = false;
			Option.bUseSplatting = false;
			Option.bDrawGrass = false;
			Option.WaterQuality = WQ_LOW;
			Option.ShadowQuality = SQ_NONE;
			Option.nMaxDirLightCount = 1;
			Option.nMaxPointLightCount = 1;
			Option.nMaxSpotLightcount = 1;
			Option.bUseTerrainLightMap = false;
			Option.bSoftShadow = false;
			Option.bDecalShadow = true;
			Option.nGraphicQuality = 2;
			break;
	}
#ifdef PRE_MOD_OPTION_TEXTURE_QUALITY
	switch( m_nTextureQuality )
	{
		case Texture_High:
			Option.nTextureQuality = 0;
			break;
		case Texture_Low:
			Option.nTextureQuality = 1;
	}
#endif // PRE_MOD_OPTION_TEXTURE_QUALITY

#ifdef PRE_MOD_OPTION_EFFECT_QUALITY //rhrnak
	Option.nEffectQuality = m_nEffectQuality;
#endif

	return Option;
}

void CGameOption::ApplyControl()
{
//	SetMouseSensitivity( m_fMouseSensitivity
//	m_fMouseSensitivity = CDnMouseCursor::GetInstance().GetMouseSensitivity();
	CDnMouseCursor::GetInstance().SetMouseSensitivity( m_fMouseSensitivity );

	Save();
}

void CGameOption::CancelControl()
{
	m_fMouseSensitivity = CDnMouseCursor::GetInstance().GetMouseSensitivity();
//	CDnMouseCursor::GetInstance().SetMouseSensitivity( m_fMouseSensitivity );
}

void CGameOption::SetMouseSensitivity( float fValue )
{
	m_fMouseSensitivity = fValue;
//	CDnMouseCursor::GetInstance().SetMouseSensitivity( fValue );
}

float CGameOption::GetMouseSendsitivity()
{
	return m_fMouseSensitivity;//CDnMouseCursor::GetInstance().GetMouseSensitivity();
}

void CGameOption::ApplySound()
{
	CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_fBGMVolume * m_fTotalVolume);
	CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_f3DVolume * m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_f2DVolume * m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_fVoiceVolume * m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "NULL", m_fTotalVolume );

	CEtSoundEngine::GetInstance().SetMute( "BGM" , m_bBGMMute || m_bTotalMute);
	CEtSoundEngine::GetInstance().SetMute( "3D" , m_b3DMute || m_bTotalMute);
	CEtSoundEngine::GetInstance().SetMute( "2D" , m_b2DMute || m_bTotalMute);
	CEtSoundEngine::GetInstance().SetMute( "VOICE" , m_bVoiceMute || m_bTotalMute);
	CEtSoundEngine::GetInstance().SetMute( "NULL", m_bTotalMute );
}

float CGameOption::GetVolumeTotal()
{
	return m_fTotalVolume;
}

float CGameOption::GetVolumeBGM()
{
	return m_fBGMVolume;
}

float CGameOption::GetVolume3D()
{
	return m_f3DVolume;
}

float CGameOption::GetVolume2D()
{
	return m_f2DVolume;
}

float CGameOption::GetVolumeVoice()
{
	return m_fVoiceVolume;
}

void CGameOption::SetVolumeTotal( float fVolume )
{
	m_fTotalVolume = fVolume;
	CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_fBGMVolume*m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_f3DVolume*m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_f2DVolume*m_fTotalVolume );
	CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_fVoiceVolume*m_fTotalVolume );
}

void CGameOption::SetVolumeBGM( float fVolume )
{
	m_fBGMVolume = fVolume;
	CEtSoundEngine::GetInstance().SetMasterVolume( "BGM", m_fBGMVolume*m_fTotalVolume );
}

void CGameOption::SetVolume3D( float fVolume )
{
	m_f3DVolume = fVolume;
	CEtSoundEngine::GetInstance().SetMasterVolume( "3D", m_f3DVolume*m_fTotalVolume );
}

void CGameOption::SetVolume2D( float fVolume )
{
	m_f2DVolume = fVolume;
	CEtSoundEngine::GetInstance().SetMasterVolume( "2D", m_f2DVolume*m_fTotalVolume );
}

void CGameOption::SetVolumeVoice( float fVolume )
{
	m_fVoiceVolume = fVolume;
	CEtSoundEngine::GetInstance().SetMasterVolume( "VOICE", m_fVoiceVolume*m_fTotalVolume );
}

bool CGameOption::IsMuteTotal()
{
	return m_bTotalMute;//CEtSoundEngine::GetInstance().IsMute( "BGM" );
}

bool CGameOption::IsMuteBGM()
{
	return m_bBGMMute;//CEtSoundEngine::GetInstance().IsMute( "BGM" );
}

bool CGameOption::IsMute3D()
{
	return m_b3DMute;//CEtSoundEngine::GetInstance().IsMute( "3D" );
}

bool CGameOption::IsMute2D()
{
	return m_b2DMute;//CEtSoundEngine::GetInstance().IsMute( "2D" );
}

bool CGameOption::IsMuteVoice()
{
	return m_bVoiceMute;//CEtSoundEngine::GetInstance().IsMute( "VOICE" );
}

void CGameOption::SetMuteTotal( bool bMute )
{
	m_bTotalMute = bMute;
	CEtSoundEngine::GetInstance().SetMute( "BGM", m_bBGMMute || m_bTotalMute );
	CEtSoundEngine::GetInstance().SetMute( "3D", m_b3DMute || m_bTotalMute );
	CEtSoundEngine::GetInstance().SetMute( "2D", m_b2DMute || m_bTotalMute);
	CEtSoundEngine::GetInstance().SetMute( "VOICE", m_bVoiceMute || m_bTotalMute);
}

void CGameOption::SetMuteBGM( bool bMute )
{
	m_bBGMMute = bMute;
	CEtSoundEngine::GetInstance().SetMute( "BGM", m_bBGMMute|| m_bTotalMute );
}

void CGameOption::SetMute3D( bool bMute )
{
	m_b3DMute = bMute;
	CEtSoundEngine::GetInstance().SetMute( "3D", m_b3DMute || m_bTotalMute);
}

void CGameOption::SetMute2D( bool bMute )
{
	m_b2DMute = bMute;
	CEtSoundEngine::GetInstance().SetMute( "2D", m_b2DMute || m_bTotalMute);
}

void CGameOption::SetMuteVoice( bool bMute )
{
	m_bVoiceMute = bMute;
	CEtSoundEngine::GetInstance().SetMute( "VOICE", m_bVoiceMute || m_bTotalMute);
}

void CGameOption::ApplyVoiceChat()
{
#ifdef _USE_VOICECHAT
	if( CDnVoiceChatTask::IsActive() )
	{
		GetVoiceChatTask().SetAutoAdjustVolume( m_bUseAutoAdjustVolume, m_fAutoAdjustVolume );
		GetVoiceChatTask().CheckVoiceFont();
	}
	if( !CVoiceChatClient::IsActive() ) return;
	CVoiceChatClient::GetInstance().SetVoiceChatVolume( m_fVoiceChatVolume );
	CVoiceChatClient::GetInstance().SetMicVolume( m_fMicVolume );
	CVoiceChatClient::GetInstance().SetMicBoost( m_bMicBoost );
#endif
}

void CGameOption::ApplyInputDevice()
{
	CInputDevice::GetInstance().CreateDevice( CInputDevice::JOYPAD );

	CInputJoyPad * pDeviceJoypad = (CInputJoyPad *)CInputDevice::GetInstance().GetDeviceObject( CInputDevice::JOYPAD );
	if( pDeviceJoypad )
	{
		((CInputJoyPad *)CInputDevice::GetInstance().GetDeviceObject( CInputDevice::JOYPAD ))->SetKeyboard(
			((CInputKeyboard *)CInputDevice::GetInstance().GetDeviceObject( CInputDevice::KEYBOARD )) );

		((CInputJoyPad *)CInputDevice::GetInstance().GetDeviceObject( CInputDevice::JOYPAD ))->SetMouse(
			((CInputMouse *)CInputDevice::GetInstance().GetDeviceObject( CInputDevice::MOUSE )) );
	}
	
	if (CDnGameControlTask::IsActive())
		GetGameControlTask().ChangePadType( m_nInputDevice );

	if( pDeviceJoypad )
	{
		pDeviceJoypad->SetWrappingButton(g_WrappingJoypadData, g_WrappingJoypadAssistData, g_UIWrappingJoypadData, g_UIWrappingJoypadAssistData, WrappingKeyIndex_Amount, UIWrappingKeyIndex_Amount);
		pDeviceJoypad->SetWrappingKey( g_WrappingKeyData, g_UIWrappingKeyData );
	}
}