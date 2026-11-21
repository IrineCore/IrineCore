#include "stdafx.h"
#ifdef _USE_VOICECHAT
#include "VoiceChatClient.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const float g_fVoiceChatVolumeRate = 2.0f;

CVoiceChatClient::CVoiceChatClient(void)
{
	m_pClient = NULL;
	m_nNumDevice = 0;
	m_Devices = NULL;
	m_fVoiceChatVolume = 0.5f;

	m_hMixer = NULL;
	m_fMicVolume = 0.5f;
#if defined(DOLBY_CLIENT_LOG)
	logout = NULL;
#endif
	m_fOrigMicVolume = 0.5f;
}

CVoiceChatClient::~CVoiceChatClient(void)
{
	SAFE_DELETE( m_Devices );
#if defined(DOLBY_CLIENT_LOG)
	fclose(logout);
#endif
	ICEClient_DestroyIceClient( m_pClient );
	RestoreOrigMicConfig();
	FinalizeMicControl();
}

#if defined(DOLBY_CLIENT_LOG)
void CVoiceChatClient::logging(unsigned int level, const char *msg, void *rock)
{
	fprintf((FILE*)rock, "%s\n", msg);
	fflush((FILE*)rock);
}
#endif

bool CVoiceChatClient::Initialize( const char *pServerIP, unsigned short nServerPort, const char *pUserName, UINT nPrivateID, VOICE_CHAT_CODEC Codec )
{
	m_pClient = ICEClient_CreateIceClient();
	if( !m_pClient )
	{
		return false;
	}

#if defined(DOLBY_CLIENT_LOG)
	const char* logfile = "DolbyAxonClientLog.txt";
	fopen_s(&logout, logfile, "a+");
	ICEClient_SetLogging(m_pClient, logging, logout, 3);
#endif

	ICECLIENT_ERROR IceError = ICEClient_Init( m_pClient, ICECLIENT_ENGINE_DOLBYHEADPHONE );
	if( IceError != ICECLIENT_ERROR_NONE)
	{
		IceError = ICEClient_Init( m_pClient, ICECLIENT_ENGINE_DIRECTX );
		if( IceError != ICECLIENT_ERROR_NONE && IceError != ICECLIENT_ERROR_ALREADY_INITED )
		{
			return false;
		}
	}
	
	IceError = ICEClient_SetCodec( m_pClient, Codec );
	if( IceError != ICECLIENT_ERROR_NONE )
	{
		return false;
	}

	// Select appropriate rendering engine.
	ICEClient_device OutputDevice;
	IceError = ICEClient_GetOutputDevice( m_pClient, &OutputDevice );
	OutputDevice.enginetype = ICEClient_GetDefaultEngineType( OutputDevice.devtype );
	IceError = ICEClient_SetDevice( m_pClient, &OutputDevice, NULL );

	//if multigame mode set gameid 
	ICEClient_SetID(m_pClient, nPrivateID, 0);

	ICEClient_SetServer( m_pClient, pServerIP, nServerPort );
	ICEClient_SetName( m_pClient, pUserName );
	//ICEClient_SetPrivateID( m_pClient, nPrivateID );.

	IceError = ICEClient_AutoTick( m_pClient );
	if( IceError != ICECLIENT_ERROR_NONE )
	{
		return false;
	}

	ICEClient_TalkInto( m_pClient, true, 0 );

	// Even doing this, why can't I hear my own voice.. is some other setting needed.
	//ICEClient_SetEchoSupression( m_pClient, true );

	// Re-apply the configured volume after initialization so volume settings made before initialization take effect.
	SetVoiceChatVolume( m_fVoiceChatVolume );

	// Verify whether mic control exists.
	ICEClient_device InputDevice;
	IceError = ICEClient_GetCaptureDevice( m_pClient, &InputDevice );
	if( IceError == ICECLIENT_ERROR_NONE )
	{
		InitializeMicControl( &InputDevice );
		CheckOrigMicConfig();
	}

	// Search for devices in advance. (Initial creation follows the default, allowing switching to a discovered device later.)
	m_nNumDevice = 0;
	ICEClient_GetDeviceList( m_Devices, &m_nNumDevice );
	if( !m_nNumDevice )
	{
		SAFE_DELETE( m_Devices );
	}
	else
	{
		m_Devices = (ICEClient_device *)malloc( sizeof(ICEClient_device)*m_nNumDevice );
		IceError = ICEClient_GetDeviceList( m_Devices, &m_nNumDevice );
		if( IceError != ICECLIENT_ERROR_NONE )
			SAFE_DELETE( m_Devices );
	}

	return true;
}


void CVoiceChatClient::Finalize()
{
	CVoiceChatClient *pClient = CVoiceChatClient::GetInstancePtr();
	SAFE_DELETE( pClient );
}

ICEClient_device *CVoiceChatClient::GetDevice( unsigned int nIndex )
{
	if( !m_Devices ) return NULL;
	if( nIndex < m_nNumDevice ) return &m_Devices[nIndex];
	return NULL;
}

bool CVoiceChatClient::SetDevice( unsigned int nIndex, bool bMic )
{
	if( !m_pClient ) return false;
	if( !m_Devices ) return false;
	if( nIndex >= m_nNumDevice ) return false;

	ICECLIENT_ERROR IceError;
	if( bMic )
		IceError = ICEClient_SetDevice( m_pClient, NULL, &m_Devices[nIndex] );
	else
		IceError = ICEClient_SetDevice( m_pClient, &m_Devices[nIndex], NULL );

	return (IceError == ICECLIENT_ERROR_NONE);
}

bool CVoiceChatClient::GetCaptureDeviceName( std::wstring &wszName )
{
	if( !m_pClient ) return false;

	// Verify whether mic control exists.
	ICEClient_device InputDevice;
	if( ICEClient_GetCaptureDevice( m_pClient, &InputDevice ) == ICECLIENT_ERROR_NONE )
	{
		ToWideString( InputDevice.name, wszName );
		return true;
	}
	return false;
}

bool CVoiceChatClient::IsExistMic()
{
	if( !m_Devices ) return false;
	for( unsigned int i = 0; i < m_nNumDevice; ++i )
	{
		if( m_Devices[i].devtype == ICECLIENT_DEVICE_CAPTURE )
			return true;
	}
	return false;
}

BYTE CVoiceChatClient::GetSpeaking()
{
	if( !IsExistMic() ) return 0;
	if( !m_pClient ) return 0;

	// Check if currently speaking using the function below.
	ICEClient_stat stat = ICEClient_GetStats(m_pClient);
	return stat.talking;
}

void CVoiceChatClient::SetVoiceChatVolume( float fValue )
{
	if( !m_pClient ) return;
	m_fVoiceChatVolume = fValue * g_fVoiceChatVolumeRate;
	ICEClient_SetVolume( m_pClient, m_fVoiceChatVolume );
}

float CVoiceChatClient::GetVoiceChatVolume()
{
	return m_fVoiceChatVolume / g_fVoiceChatVolumeRate;
}

void CVoiceChatClient::MuteMyMic( bool bMute )
{
	if( !m_pClient ) return;
	ICEClient_MuteMic( m_pClient, (bMute ? 1 : 0) );
}

void CVoiceChatClient::SetRotation( int nRotation )
{
	if( !m_pClient ) return;
	ICEClient_SetRotation( m_pClient, nRotation );
}

void CVoiceChatClient::SetVoiceFont( eVoiceFontType eVoiceFont, float fPitch, float fTimbre )
{
	if( !m_pClient ) return;

	switch( eVoiceFont )
	{
	case VoiceFontNone:		ICEClient_SetVoiceFont( m_pClient, ICECLIENT_VOICEFONT_NONE ); break;
	case VoiceFontW2M:		ICEClient_SetVoiceFont( m_pClient, ICECLIENT_VOICEFONT_PRESET_W2M ); break;
	case VoiceFontM2W:		ICEClient_SetVoiceFont( m_pClient, ICECLIENT_VOICEFONT_PRESET_M2W ); break;
	case VoiceFontELF:		ICEClient_SetVoiceFont( m_pClient, ICECLIENT_VOICEFONT_PRESET_ELF ); break;
	case VoiceFontCustom:
		{
			ICEClient_voicefont CustomVoiceFont;
			CustomVoiceFont.pitchchange = fPitch;
			CustomVoiceFont.timbrechange = fTimbre;
			ICEClient_SetCustomVoiceFont( m_pClient, CustomVoiceFont );
		}
		break;
	}
}

void CVoiceChatClient::StartMicTestPhase1()
{
	if( !m_pClient ) return;
	ICEClient_StartMicTestPhase1( m_pClient );
}

void CVoiceChatClient::StartMicTestPhase2( void (*callback)(void*) )
{
	if( !m_pClient ) return;
	ICEClient_StartMicTestPhase2( m_pClient, ICECLIENT_MICTEST_LOOPBACK, callback, NULL );
}

void CVoiceChatClient::StopMicTest()
{
	if( !m_pClient ) return;
	ICEClient_StopMicTest( m_pClient );
}

bool CVoiceChatClient::InitializeMicControl( ICEClient_device *pCaptureDevice )
{
	FinalizeMicControl();

	HRESULT hr;
	const int nMaxMixerCount = 20;	// Check up to roughly 20 devices.
	bool bFind = false;

	for( int i = 0; i < nMaxMixerCount; ++i ) {
		hr = mixerOpen(&m_hMixer, i, 0, 0, 0);
		if( hr != S_OK ) continue;

		memset(&m_mxl, 0, sizeof(m_mxl));
		m_mxl.cbStruct = sizeof(m_mxl);

		MIXERCAPS caps;
		if( mixerGetDevCaps((UINT_PTR)m_hMixer, &caps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR ) {
			FinalizeMicControl();
			continue;
		}

		WCHAR wszBuff[256] = L"";
		//ZeroMemory(wszBuff, sizeof(WCHAR)*_countof(wszBuff)); // sizeof * sizeof is usually incorrect; did you mean character count or byte count?
		ZeroMemory(wszBuff, sizeof(wszBuff));

		MultiByteToWideChar(CP_ACP, 0, pCaptureDevice->name, -1, wszBuff, _countof(wszBuff) );
		if( wcsncmp( caps.szPname, wszBuff, wcslen(caps.szPname) ) != 0 ) {	// caps.szPname comes with size 32, so compare up to size excluding null terminator.
			FinalizeMicControl();
			continue;
		}

		int nDest = caps.cDestinations;
		for( int j = 0; j < nDest; ++j ) {
			m_mxl.cbStruct = sizeof(m_mxl);
			m_mxl.dwSource = 0;
			m_mxl.dwDestination = j;
			if( mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_DESTINATION) != MMSYSERR_NOERROR ) {
				FinalizeMicControl();
				break;
			}

			if( m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN )
				break;
		}

		int nConnections = m_mxl.cConnections;
		for( int j = 0; j < nConnections; ++j ) {
			m_mxl.dwSource = j;
			if( mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_mxl, MIXER_GETLINEINFOF_SOURCE) != MMSYSERR_NOERROR ) {
				FinalizeMicControl();
				break;
			}
			if( MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE == m_mxl.dwComponentType ) {
				// Even when adjusting mic volume, it sometimes still cannot be found, so search directly down to the volume control.
				MIXERLINECONTROLS mlc = {0,};
				MIXERCONTROL mc = {0,};
				mlc.cbStruct = sizeof(MIXERLINECONTROLS);
				mlc.dwLineID = m_mxl.dwLineID;
				mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				mlc.cControls = 1;
				mlc.pamxctrl = &mc;
				mlc.cbmxctrl = sizeof(MIXERCONTROL);
				if( mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR ) {
					FinalizeMicControl();
					break;
				}
				CheckMicBoostControl();
				return true;
			}
		}

		FinalizeMicControl();
	}
	return false;
}

void CVoiceChatClient::FinalizeMicControl()
{
	if( m_hMixer ) {
		mixerClose(m_hMixer);
		m_hMixer = NULL;
		m_bEnableMicBoost = false;
	}
}

void CVoiceChatClient::CheckOrigMicConfig()
{
	if( IsExistMicControl() )
		m_fOrigMicVolume = GetMicVolume();
	//if( IsEnableMicBoost() )
	//	m_bOrigMicBoost = GetMicBoost();
}

void CVoiceChatClient::RestoreOrigMicConfig()
{
	if( IsExistMicControl() )
		SetMicVolume( m_fOrigMicVolume );
	//if( IsEnableMicBoost() )
	//	SetMicBoost( m_bOrigMicBoost );
}

void CVoiceChatClient::SetMicVolume( float fValue )
{
	if( !IsExistMicControl() ) return;

	MIXERLINECONTROLS mlc = {0,};
	MIXERCONTROL mc = {0,};
	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwLineID = m_mxl.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.pamxctrl = &mc;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

	MIXERCONTROLDETAILS mxcd = {0,};
	MIXERCONTROLDETAILS_UNSIGNED mxdu = {0,};
	mxdu.dwValue = DWORD(fValue * 65535);
	mxcd.cMultipleItems = 0;
	mxcd.cChannels = 1;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = mc.dwControlID;
	mxcd.cbDetails = sizeof(mxdu);
	mxcd.paDetails = &mxdu;

	mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
}

float CVoiceChatClient::GetMicVolume()
{
	if( !IsExistMicControl() ) return 0.0f;

	MIXERLINECONTROLS mlc = {0,};
	MIXERCONTROL mc = {0,};
	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwLineID = m_mxl.dwLineID;
	mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlc.cControls = 1;
	mlc.pamxctrl = &mc;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

	MIXERCONTROLDETAILS mxcd = {0,};
	MIXERCONTROLDETAILS_UNSIGNED mxdu = {0,};
	mxcd.cMultipleItems = 0;
	mxcd.cChannels = 1;
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = mc.dwControlID;
	mxcd.cbDetails = sizeof(mxdu);
	mxcd.paDetails = &mxdu;

	mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

	// Since the value ranges from 0 to 65535.
	return mxdu.dwValue / 65535.0f;
}

void CVoiceChatClient::CheckMicBoostControl()
{
	MMRESULT mmr;
	HMIXER hMixer;
	MIXERCAPS MixerCaps;
	MIXERLINE MixerLine;
	HRESULT hr;
	const int nMaxMixerCount = 20;	// Check up to roughly 20 devices.

	for( int i = 0; i < nMaxMixerCount; ++i ) {
		hr = mixerOpen(&hMixer, i, 0, 0, 0);
		if( hr != S_OK ) continue;

		if( mixerGetDevCaps((UINT_PTR)hMixer, &MixerCaps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR ) {
			mixerClose( hMixer );
			continue;
		}

		bool bFindDest = false;
		memset(&MixerLine, 0, sizeof(MixerLine));
		int nDest = MixerCaps.cDestinations;
		for( int j = 0; j < nDest; ++j ) {
			MixerLine.cbStruct = sizeof(MixerLine);
			MixerLine.dwDestination = j;
			MixerLine.dwSource = 0;

			mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_DESTINATION);
			if( mmr != MMSYSERR_NOERROR ) {
				mixerClose( hMixer );
				continue;
			}

			if( MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ) {
				bFindDest = true;
				break;
			}
		}
		if( !bFindDest ) {
			mixerClose( hMixer );
			continue;
		}

		// Since there is no way to tell which source control is actually connected to the microphone, apply it to all boost controls.
		int nConnection = MixerLine.cConnections;
		DWORD dwDstIndex = MixerLine.dwDestination;
		for( int j = 0; j < nConnection; ++j ) {
			MixerLine.cbStruct = sizeof( MIXERLINE );
			MixerLine.dwSource = j;
			MixerLine.dwDestination = dwDstIndex;
			if( mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_SOURCE) != MMSYSERR_NOERROR ) {
				continue;
			}

			if( (MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) || (MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_LAST) ) {
				MIXERCONTROL mxc;
				MIXERLINECONTROLS mxlc;
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
				mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
				mxlc.dwLineID = MixerLine.dwLineID;
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_ONOFF;
				mxlc.cControls = 1;
				mxlc.cbmxctrl = sizeof(MIXERCONTROL);
				mxlc.pamxctrl = &mxc;
				mxc.cbStruct = sizeof(MIXERCONTROL);
				if( mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR ) {
					// If execution reaches here, assume it will look for amplification.
					mixerClose( hMixer );
					m_bEnableMicBoost = true;
					return;
				}
			}
		}
		mixerClose( hMixer );
	}


	// Tried debugging this as sample code and it does not work properly. Clean up later.
/*
	HMIXER hMixer;
	MMRESULT mmr;
	MIXERCAPS MixerCaps;
	MIXERLINE MixerLine;
	int u;
	bool bFindDest = false;
	bool bFindCtrl = false;

	int index = 0;
	mmr = mixerOpen(&hMixer, index, 0, 0, 0); 
	if (mmr != MMSYSERR_NOERROR) {
		return;
	}

	mmr = mixerGetDevCaps((UINT_PTR)hMixer, &MixerCaps, sizeof(MixerCaps));
	if( mmr != MMSYSERR_NOERROR ) {
		mixerClose( hMixer );
		return;
	}

	memset(&MixerLine, 0, sizeof(MixerLine));
	for( u = 0; u < MixerCaps.cDestinations; u++ ) {
		MixerLine.cbStruct = sizeof(MixerLine);
		MixerLine.dwDestination = u;
		MixerLine.dwSource = 0;

		mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_DESTINATION); 
		if( mmr != MMSYSERR_NOERROR ) {
			mixerClose( hMixer );
			return;
		}

		if( MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN ) {
			bFindDest = true;
			break;
		}
	}
	if( !bFindDest ) {
		mixerClose( hMixer );
		return;
	}

	MIXERLINECONTROLS MixerLineControls;
	PMIXERCONTROL paMixerControls = new MIXERCONTROL[MixerLine.cControls];
	MixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	MixerLineControls.dwLineID = MixerLine.dwLineID;
	MixerLineControls.cControls = MixerLine.cControls;
	MixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
	MixerLineControls.pamxctrl = paMixerControls;

	// Get the controls for the current destination mixer line
	mmr = mixerGetLineControls((HMIXEROBJ)hMixer, &MixerLineControls, MIXER_GETLINECONTROLSF_ALL);
	if( mmr != MMSYSERR_NOERROR ) {
		delete[] paMixerControls;
		mixerClose( hMixer );
		return;
	}

	for( u = 0; u < MixerLine.cControls; u++ ) {
		if ((paMixerControls[u].dwControlType == MIXERCONTROL_CONTROLTYPE_MUX) || (paMixerControls[u].dwControlType == MIXERCONTROL_CONTROLTYPE_MIXER)) {
			bFindCtrl = true;
			break;
		}
	}

	if( !bFindCtrl ) {
		delete[] paMixerControls;
		mixerClose( hMixer );
		return;
	}

	MIXERCONTROLDETAILS MixerControlDetails;
	PMIXERCONTROLDETAILS_LISTTEXT pMixerControlDetails_Listtext = new MIXERCONTROLDETAILS_LISTTEXT[MixerLine.cChannels * paMixerControls[u].cMultipleItems];
	MixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	MixerControlDetails.dwControlID = paMixerControls[u].dwControlID;
	MixerControlDetails.cChannels = 1;//MixerLine.cChannels;
	MixerControlDetails.hwndOwner = (HWND)paMixerControls[u].cMultipleItems;
	MixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	MixerControlDetails.paDetails = pMixerControlDetails_Listtext;
	mmr = mixerGetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT); 
	if (mmr != MMSYSERR_NOERROR) {
		delete[] paMixerControls;
		delete[] pMixerControlDetails_Listtext;
		mixerClose( hMixer );
		return;
	}

	PMIXERCONTROLDETAILS_BOOLEAN pMixerControlDetails_Boolean = new MIXERCONTROLDETAILS_BOOLEAN[MixerLine.cChannels * paMixerControls[u].cMultipleItems];
	MixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	MixerControlDetails.dwControlID = paMixerControls[u].dwControlID;
	MixerControlDetails.cChannels = 1;//MixerLine.cChannels;
	MixerControlDetails.hwndOwner = (HWND)paMixerControls[u].cMultipleItems;
	MixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	MixerControlDetails.paDetails = pMixerControlDetails_Boolean;
	mmr = mixerGetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	if( mmr != MMSYSERR_NOERROR ) {
		delete[] paMixerControls;
		delete[] pMixerControlDetails_Listtext;
		delete[] pMixerControlDetails_Boolean;
		mixerClose( hMixer );
		return;
	}

	//Set only the microphone or line as input sources
	for( int v = 0; v < paMixerControls[u].cMultipleItems; v++ ) {
		pMixerControlDetails_Boolean[v].fValue = FALSE;
		if( pMixerControlDetails_Listtext[v].dwParam2 == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
			pMixerControlDetails_Boolean[v].fValue = TRUE;
	}

	mmr = mixerSetControlDetails((HMIXEROBJ)hMixer, &MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE); 
	if( mmr != MMSYSERR_NOERROR ) {
		delete[] paMixerControls;
		delete[] pMixerControlDetails_Listtext;
		delete[] pMixerControlDetails_Boolean;
		mixerClose( hMixer );
		return;
	}

	delete[] paMixerControls;
	delete[] pMixerControlDetails_Listtext;
	delete[] pMixerControlDetails_Boolean;
	mixerClose( hMixer );
	m_bEnableMicBoost = true;
	return;
*/

/*
	MMRESULT mmr;
	int u;
	PMIXERCONTROL paMixerControls;
	MIXERLINECONTROLS mlc = {0,};
	MIXERCONTROL mc = {0,};
	paMixerControls = (PMIXERCONTROL)malloc(sizeof(MIXERCONTROL) * m_mxl.cControls);
	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwLineID = m_mxl.dwLineID;
	mlc.cControls = m_mxl.cControls;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mlc.pamxctrl = paMixerControls;
	mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_GETLINECONTROLSF_ALL);

	for (u = 0; u < (int)m_mxl.cControls; u++)
		if (paMixerControls[u].dwControlType == MIXERCONTROL_CONTROLTYPE_MUX)
		//if (NULL != strstr(paMixerControls[u].szName, "Mic Boost"))
		//if (paMixerControls[u].dwControlType == MIXERCONTROL_CONTROLTYPE_ONOFF)
			break;

	mlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mlc.dwControlID = paMixerControls[u].dwControlID;
	mlc.cbmxctrl = sizeof(MIXERCONTROL);
	mlc.pamxctrl = &mc;
	mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYID);
	free(paMixerControls);

	PMIXERCONTROLDETAILS_LISTTEXT pMixerControlDetails_Listtext;
	MIXERCONTROLDETAILS MixerControlDetails;
	pMixerControlDetails_Listtext = (PMIXERCONTROLDETAILS_LISTTEXT)malloc(m_mxl.cChannels * mc.cMultipleItems * sizeof(MIXERCONTROLDETAILS_LISTTEXT));
	MixerControlDetails.cbStruct = sizeof(MixerControlDetails);
	MixerControlDetails.dwControlID = mc.dwControlID;
	MixerControlDetails.cChannels = m_mxl.cChannels;
	MixerControlDetails.cMultipleItems = mc.cMultipleItems;
	MixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	MixerControlDetails.paDetails = pMixerControlDetails_Listtext;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &MixerControlDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);

	for(u = 0; u < (int)mc.cMultipleItems; u++)
		if (0 == wcscmp(pMixerControlDetails_Listtext[u].szName, L"Microphone"))
			break;

	PMIXERCONTROLDETAILS_BOOLEAN pMixerControlDetails_Boolean;
	pMixerControlDetails_Boolean = (PMIXERCONTROLDETAILS_BOOLEAN)malloc(m_mxl.cChannels * mc.cMultipleItems * sizeof(MIXERCONTROLDETAILS_BOOLEAN));
	MixerControlDetails.cbStruct       = sizeof(MixerControlDetails);
	MixerControlDetails.dwControlID    = mc.dwControlID;
	MixerControlDetails.cChannels      = m_mxl.cChannels;
	MixerControlDetails.cMultipleItems = mc.cMultipleItems;
	MixerControlDetails.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	MixerControlDetails.paDetails      = pMixerControlDetails_Boolean;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &MixerControlDetails, 0L);

	pMixerControlDetails_Boolean[u].fValue = true;
	mmr = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &MixerControlDetails, 0L);

	free(pMixerControlDetails_Listtext);
	free(pMixerControlDetails_Boolean);
*/
}

void CVoiceChatClient::SetMicBoost( bool bBoost )
{
	if( !IsEnableMicBoost() ) return;

	MMRESULT mmr;
	HMIXER hMixer;
	MIXERCAPS MixerCaps;
	MIXERLINE MixerLine;
	HRESULT hr;
	const int nMaxMixerCount = 20;	// Check up to roughly 20 devices.

	for( int i = 0; i < nMaxMixerCount; ++i ) {
		hr = mixerOpen(&hMixer, i, 0, 0, 0);
		if( hr != S_OK ) continue;

		if( mixerGetDevCaps((UINT_PTR)hMixer, &MixerCaps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR ) {
			mixerClose( hMixer );
			continue;
		}

		bool bFindDest = false;
		memset(&MixerLine, 0, sizeof(MixerLine));
		int nDest = MixerCaps.cDestinations;
		for( int j = 0; j < nDest; ++j ) {
			MixerLine.cbStruct = sizeof(MixerLine);
			MixerLine.dwDestination = j;
			MixerLine.dwSource = 0;

			mmr = mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_DESTINATION);
			if( mmr != MMSYSERR_NOERROR ) {
				mixerClose( hMixer );
				continue;
			}

			if( MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ) {
				bFindDest = true;
				break;
			}
		}
		if( !bFindDest ) {
			mixerClose( hMixer );
			continue;
		}

		// Since there is no way to tell which source control is actually connected to the microphone, apply it to all boost controls.
		int nConnection = MixerLine.cConnections;
		DWORD dwDstIndex = MixerLine.dwDestination;
		for( int j = 0; j < nConnection; ++j ) {
			MixerLine.cbStruct = sizeof( MIXERLINE );
			MixerLine.dwSource = j;
			MixerLine.dwDestination = dwDstIndex;
			if( mixerGetLineInfo((HMIXEROBJ)hMixer, &MixerLine, MIXER_GETLINEINFOF_SOURCE) != MMSYSERR_NOERROR ) {
				continue;
			}

			if( (MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) || (MixerLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_LAST) ) {
				MIXERCONTROL mxc;
				MIXERLINECONTROLS mxlc;
				ZeroMemory(&mxlc, sizeof(MIXERLINECONTROLS));
				mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
				mxlc.dwLineID = MixerLine.dwLineID;
				mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_ONOFF;
				mxlc.cControls = 1;
				mxlc.cbmxctrl = sizeof(MIXERCONTROL);
				mxlc.pamxctrl = &mxc;
				mxc.cbStruct = sizeof(MIXERCONTROL);
				if( mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR ) {
					DWORD dwValue = bBoost ? 1 : 0;
					MIXERCONTROLDETAILS mxcd;

					DWORD dwControlID = mxc.dwControlID;
					DWORD cMultipleItems = mxc.cMultipleItems;

					// Initialize the MIXERCONTROLDETAILS structure
					ZeroMemory(&mxcd, sizeof(MIXERCONTROLDETAILS));
					mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
					mxcd.dwControlID = dwControlID;	//mxc.dwControlID;
					mxcd.cChannels = 1;
					mxcd.cMultipleItems = cMultipleItems;
					mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					mxcd.paDetails = &dwValue;
					//mxcd.paDetails = &mxcdStruct;

					// Set the values
					if( mixerSetControlDetails((HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE) != MMSYSERR_NOERROR ) {
						OutputDebug( "VoiceChat Boost Setting Fail!" );
					}
				}
			}
		}
		mixerClose( hMixer );
	}
}
#endif