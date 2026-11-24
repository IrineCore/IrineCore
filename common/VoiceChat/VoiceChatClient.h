#pragma once

#ifdef _USE_VOICECHAT
#include "Singleton.h"
#include "iceclientlib.h"
#include <mmsystem.h>

enum VOICE_CHAT_CODEC
{
	VOICE_CHAR_8BIT = 21,
	VOICE_CHAR_16BIT = 22,
};

class CVoiceChatClient : public CSingleton< CVoiceChatClient >
{
public:
	CVoiceChatClient(void);
	virtual ~CVoiceChatClient(void);

	enum eVoiceFontType
	{
		VoiceFontNone,
		VoiceFontW2M,
		VoiceFontM2W,
		VoiceFontELF,
		VoiceFontCustom,
	};

protected:
	ICECLIENT *m_pClient;

	// Searched devices.
	unsigned int m_nNumDevice;
	ICEClient_device *m_Devices;

	// What can be done locally on client side: volume control, voice modulation
	float m_fVoiceChatVolume;

	// No suitable class for mic handling, so keep it in the voice chat client.
	HMIXER m_hMixer;
	MIXERLINE m_mxl;
	float m_fMicVolume;

	// Whether boost control exists.
	bool m_bEnableMicBoost;
	void CheckMicBoostControl();

	// These modify Windows settings, so save initial values and restore them on exit.
	float m_fOrigMicVolume;
	//bool m_bOrigMicBoost; // For boost, we cannot get just a single control, so skip handling it.
	void CheckOrigMicConfig();
	void RestoreOrigMicConfig();

public:
	bool Initialize( const char *pServerIP, unsigned short nServerPort, const char *pUserName, UINT nPrivateID, VOICE_CHAT_CODEC Codec = VOICE_CHAR_8BIT );
	static void Finalize();

	bool InitializeMicControl( ICEClient_device *pCaptureDevice );
	void FinalizeMicControl();

	int GetNumDevice() { return m_nNumDevice; }
	ICEClient_device *GetDevice( unsigned int nIndex );
	bool SetDevice( unsigned int nIndex, bool bMic );
	bool GetCaptureDeviceName( std::wstring &wszName );

	// Whether Dolby detected the microphone.
	bool IsExistMic();

	// Check if mic control exists
	bool IsExistMicControl() { return (m_hMixer != NULL); }
	// Check if self is speaking
	BYTE GetSpeaking();


	// Client's SetVolume
	void SetVoiceChatVolume( float fValue );
	float GetVoiceChatVolume();

	// Client's own microphone mute
	void MuteMyMic( bool bMute );

	// Client Rotation setting, value between 0~359.
	void SetRotation( int nRotation );

	// Voice font
	// Uses Pitch and Timbre in custom mode; valid range is 0.5~2.0.
	// Pitch : Closer to 0.5 for low male voice, closer to 2.0 for high female voice
	// Timbre: 0.5 gives a stretched, heavy tone (low TV voice modulation); 2.0 gives a fast-forwarded tape feel (high TV voice modulation)
	void SetVoiceFont( eVoiceFontType eVoiceFont, float fPitch = 0.0f, float fTimbre = 0.0f );

	// Mic recording test
	void StartMicTestPhase1();
	void StartMicTestPhase2( void (*callback)(void*) );
	void StopMicTest();

	// Control panel microphone volume
	void SetMicVolume( float fValue );
	float GetMicVolume();

	// Mic boost control
	bool IsEnableMicBoost() { return m_bEnableMicBoost; }
	void SetMicBoost( bool bBoost );

//#define DOLBY_CLIENT_LOG
#if defined(DOLBY_CLIENT_LOG)
	FILE* logout;
	static void logging(unsigned int level, const char *msg, void *rock);
#endif
};
#endif