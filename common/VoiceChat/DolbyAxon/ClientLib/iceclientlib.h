/* C interface */

#include <stdio.h>

typedef void ICECLIENT;

#ifdef __cplusplus
extern "C"
{
#endif

/* All functions are identical to the C++ version unless noted otherwise. */
/* They take an additional parameter which is the ICECLIENT pointer returned from CreateIceClient */

typedef struct _ICEClient_stat
{
		int ping;               //Round trip time in ms of last ping message 
		int netin;              //Total payload bytes received
		int netout;             //Total payload bytes sent 
		char gsname[256];       //Name of Game server (this is "" if not properly connected).
		char asname[256];       //Name of Audio server (this is "" if not properly connected) 
		unsigned char talking;	//non zero value between 1 and 255 indicating talking volume if player is currently talking
		int running;            //set if the client is running, i.e the library is processing audio 
		int premium;            //set if the client is in premium mode

		//input output levels
		unsigned int mic_clipping;	//1 if the mic input is clipping
		float rms_in_level;			//RMS level of the input
		unsigned int chan_out_num;	//number of output channels (max 8)
		float rms_out_level[8];		//RMS level of the output channels
} ICEClient_stat;

typedef enum _ICECLIENT_ERROR
{
	ICECLIENT_ERROR_NONE = 0,
	ICECLIENT_ERROR_INVALID_DEVICE,
	ICECLIENT_ERROR_NOT_INITED,
	ICECLIENT_ERROR_ALREADY_INITED,
	ICECLIENT_ERROR_CREATE_AUDIO_FAILED,
	ICECLIENT_ERROR_CREATE_CAPTURE_FAILED,
	ICECLIENT_ERROR_START_AUDIO_FAILED,
	ICECLIENT_ERROR_TEST_FAILED,
	ICECLIENT_ERROR_INVALID_PARAM,
	ICECLIENT_ERROR_NULL_PARAM,
	ICECLIENT_ERROR_DATA_TOO_BIG,
	ICECLIENT_ERROR_OUTOFBOUND_ROTATION,
	ICECLIENT_ERROR_CREATE_THREAD_FAILED,
	ICECLIENT_ERROR_CREATE_ENGINE_FAILED,
	ICECLIENT_ERROR_BIND_SOCKET_FAILED,
	ICECLIENT_ERROR_INVALID_CODEC_RATE,
	ICECLIENT_ERROR_UNRECOGNISED_DATA,
	ICECLIENT_ERROR_CALLBACK_ALREADY_SET,
	ICECLIENT_ERROR_UNKNOWN
} ICECLIENT_ERROR;

typedef struct _ICEClient_codecpair
{
	unsigned char id;
	const char* name;
} ICEClient_codecpair;

typedef enum _ICECLIENT_DEVICE_TYPE
{
	ICECLIENT_DEVICE_UNKNOWN = 0,
	ICECLIENT_DEVICE_CAPTURE = 1,
	ICECLIENT_DEVICE_HEADPHONES = 2,
	ICECLIENT_DEVICE_STEREOSPEAKERS = 3,
	ICECLIENT_DEVICE_SURROUNDSPEAKERS = 4
} ICECLIENT_DEVICE_TYPE;

typedef enum _ICECLIENT_ENGINE_TYPE
{
	ICECLIENT_ENGINE_DEFAULT = 0,
	ICECLIENT_ENGINE_MONO = 1,
	ICECLIENT_ENGINE_DIRECTX = 2,
	ICECLIENT_ENGINE_DOLBYHEADPHONE = 3,
	ICECLIENT_ENGINE_DOLBYVIRTUALSPEAKER = 4
} ICECLIENT_ENGINE_TYPE;

typedef enum _ICECLIENT_MICTEST_MODE
{
	ICECLIENT_MICTEST_LOOPBACK = 0,
	ICECLIENT_MICTEST_TO_SERVER = 1,
} ICECLIENT_MICTEST_MODE;

typedef enum _ICECLIENT_SPKTEST_MODE
{
	ICECLIENT_SPKTEST_VOICE_5_1 = 0,   //5.1 speaker enum with voice
	ICECLIENT_SPKTEST_NOISE_5_1 = 1,   //5.1 speaker enum with noise
	ICECLIENT_SPKTEST_NOISE_SWEEP = 2  //noise sweeping through 360 degrees
} ICECLIENT_SPKTEST_MODE;

typedef enum _ICECLIENT_ROOM_MODE
{
	ICECLIENT_ROOM_NORMAL = 0,
	ICECLIENT_ROOM_CAVERN = 1,
	ICECLIENT_ROOM_SMALL = 2
} ICECLIENT_ROOM_MODE;

typedef enum _ICECLIENT_VOICEFONT_PRESET
{
	ICECLIENT_VOICEFONT_NONE = 0,
	ICECLIENT_VOICEFONT_PRESET_W2M = 1,
	ICECLIENT_VOICEFONT_PRESET_M2W = 2,
	ICECLIENT_VOICEFONT_PRESET_ELF = 3
} ICECLIENT_VOICEFONT_PRESET;

typedef struct _ICEClient_voicefont
{
	float pitchchange;                //valid values are between 0.5 and 2 inclusive, 1 for no change and > 1 for pitch increase.
	float timbrechange;               //valid values are between 0.5 and 2 inclusive, changes character of voice
} ICEClient_voicefont;

typedef struct _ICEClient_device
{
	unsigned int id;                 //device id
	char name[256];                  //device name as reported by DirectX
	ICECLIENT_DEVICE_TYPE devtype;
	int systemdefault;                //1 if device is system default device
	int icedefault;                   //1 if device is selected as default for new ice clients
	ICECLIENT_ENGINE_TYPE enginetype; //audio engine type to use if this is
	                                  //an output device, ignored for capture devices
} ICEClient_device;

typedef enum _ICECLIENT_WAVFORMAT
{
	ICECLIENT_WAVFORMAT_PCM = 0,	//16-bit short samples
	ICECLIENT_WAVFORMAT_IEEE_FLOAT,	//32-bit float samples
	ICECLIENT_WAVFORMAT_CODED,		//reserved, not yet supported
} ICECLIENT_WAVFORMAT;

typedef struct _ICEClient_wavspec
{
	ICECLIENT_WAVFORMAT format;
	unsigned int sampling_rate;		//in Hz (valid values are 8000, 16000, 24000, 32000, 441000, 48000)
	unsigned int channels;			//number of channels, 1 for mono, 2 for stereo wavs
} ICEClient_wavspec;

// This function is deprecated.
__declspec(dllexport) void ICEClient_SetPremium(int prem);


// Description:
//   Creates the ICECLIENT object used for all subsequent client-side ICE API
//   calls.
// Parameters:
//   None.
// Return value:
//   Pointer to ICECLIENT if successful; NULL otherwise.
__declspec(dllexport) ICECLIENT* ICEClient_CreateIceClient();


// Description:
//   Destroys the ICECLIENT object and cleans up any  memory allocations.
// Parameters:
//   ic: The ICECLIENT pointer.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_DestroyIceClient(ICECLIENT* ic);


// Description:
//   This function is called when a player is first created. Until this function
//   or ICEClient_InitDolby is called, all other functions that take an ICECLIENT
//   pointer should not be called. After successful initialization, repeated
//   calls are ignored. 
// Parameters:
//   ic: Valid ICECLIENT pointer created by ICEClient_CreateIceClient.
//   engine_type: Type of audio engine to use. If engine_type = ICECLIENT_ENGINE_DEFAULT
//   then: 1) if an earlier call to ICEClient_SetDefaultDevice has
//   set an engine type for the default output device, that engine type will be
//   used, 2) if not, the engine type will be Dolby Headphone
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
//   If ICECLIENT_ERROR_CREATE_CAPTURE_FAILED is returned, either no capture device
//   was found, or default capture device could not be initialized. In this case
//   audio output would still work.
// Note:
//   If premium has not been set engine_type is ignored and ICECLIENT_ENGINE_MONO
//   is used.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_Init(ICECLIENT* ic, ICECLIENT_ENGINE_TYPE engine_type);


// Description
//   This function is an alternative to ICEClient_Init and is optimized for
//   headphone use. It is the same as calling
//   ICEClient_Init(ic, ICECLIENT_ENGINE_DOLBYHEADPHONE). After successful
//   initialization, repeated calls are ignored. 
// Parameters:
//   ic: Valid ICECLIENT pointer created by ICEClient_CreateIceClient.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_InitDolby(ICECLIENT* ic);


// Description:
//   GetCodecs returns the list of codecs detected in the system.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
// Return value:
//   The return value is a pointer to a list of codec ids. This 
//   list is valid until another call is made to this function.
//   The list is terminated by a 0.
__declspec(dllexport) const unsigned char* ICEClient_GetCodecs(ICECLIENT* ic);


// Description:
//   Set Codec selects which codec to use.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   codec_id: The ID of the codec to use, this can be retrieved by calling
//   ICEClient_GetCodecs.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetCodec(ICECLIENT* ic, unsigned char codec_id);


// Description:
//   Sets the callback to use to log information and errors.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   logfunc: Callback to handle log messages. The callback function should take
//   three arguments: the log message level, the log message, and a void 
//   pointer.
//   rock: A void pointer that is to be passed back to callback function
//   logfunc.
//   level: The logging level to use; higher numbers indicate more
//     logging is wanted. Log levels are as follows:
//       0 : CRITICAL
//       1 : SERIOUS
//       2 : WARNING
//       3 : DEBUG
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetLogging(ICECLIENT* ic, void (*logfunc)(unsigned int level, const char* msg, void* rock), void* rock, unsigned int level);


// Description:
//   Tick performs all the necessary work in the audio client. Should be
//   called regularly with a period of 10-20ms, depending on internal
//   settings.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_Tick(ICECLIENT* ic);


// Description:
//   AutoTick creates a thread to automatically tick. Call this once to
//   start it. Tick can still be called.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_AutoTick(ICECLIENT* ic);


// Description:
//   Move client into and out of paused state. When a client is paused,
//   no packets and no audio will be processed.
//   Use this function if the client needs to be disabled temporarily but
//   not destroyed.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   on: if is 1 client will be paused, if is 0 client will be unpaused.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_Pause(ICECLIENT* ic, int on);


// Description:
//   This function provides relevant information about the internal state of
//   the audio client. See the ICEClient_stat structure for more information.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
// Return value:
//   ICECLient_stat struct with internal information.
__declspec(dllexport) ICEClient_stat ICEClient_GetStats(ICECLIENT* ic);


// Description:
//   This function adjusts the rendered scenes volume. The audio scene's volume
//   will be adjusted so that clipping does not occur due to this setting. 
//   However, values past 1.0 may cause distortion.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   vol: The multiplier for the audio scene.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_SetVolume(ICECLIENT* ic, const float vol);


// Description:
//   This function sets the IP address and the port of the audio server
//   to which the client should connect.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   server: IP address of audio server in dotted string format.
//   port: Port of audio server.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   Calling this function disable packet encapsulation if it was
//   previously set with SetGameEncapsulation()
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetServer(ICECLIENT* ic, const char* server, const unsigned short port);


// Description:
//   Assigns a name that the server can use to help differentiate clients
//   behind a NAT.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   player_name: The player name used in game.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_SetName(ICECLIENT* ic, const char* player_name);


// Description:
//   Sets the ID of the player and the game ID of the library the player is connected through.
//   This allows Axon audio server to properly match incoming audio connections to players
// Parameters:
//   player_id: The id of the player.
//    Valid value is between 0 and 2^64-2 if game_id == 0
//    Valid value is between 0 and 2^48-2 if game_id != 0
//   game_id: The id of the game. Must be set to 0 if Axon is not being used in multigame mode.
//    Valid game_id value is between 0 and 2^16-2
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetID(ICECLIENT* ic, const unsigned long long player_id, const unsigned short game_id);


// Description:
//   This function changes the player's current rotation in the world.
// Parameters:
//   ic: ICECLIENT pointer initialised by ICEClient_Init or ICEClient_InitDolby.
//   rot: The rotation in degrees and must be between 0 and 359.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   The rotation is given in degrees, where 0 is along the positive x axis
//   and 90 is along the positive y axis. rot must be between 0 and 359
//   inclusive. If rotation is not set or set to -1, spatialization will be done
//   based on the rotation specified by the game server through the server ICE
//   library (ICE_position and ICE_position_channel).   
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetRotation(ICECLIENT* ic, const int rot);


// Description:
//   This function changes the player's current rotation in the world.
// Parameters:
//   ic: Valid and initialized ICECLIENT pointer
//   yaw: The rotation in degrees and must be between 0 and 359.
//   pitch: The rotation in degrees and must be between -90 and 90.
//   roll: The rotation in degrees and must be between -180 and 180.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   The yaw is given in degrees, where 0 is along the positive x axis
//   and 90 is along the positive y axis. yaw must be between 0 and 359
//   inclusive. If yaw is not set or set to -1, spatialization will be done based
//   on the yaw specified by the game server through the server ICE
//   library (ICE_position and ICE_position_channel).
//	 The pitch is given in degrees, where 0 is along the positive x axis (assuming 0 yaw)
//	 and 90 is along the positive z axis. Pitch must be from -90 to 90 degrees inclusive
//	 The roll is given in degrees, where 0 means that, assuming a yaw and pitch of 0
//	 respectively, the positive y axis is to the right, while a roll of 90 means the positive
//	 y axis is up.
//   Do not mix use of the two SetRotation functions
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetRotation_npr(ICECLIENT* ic, const int yaw, const int pitch, const int roll);


// Description:
//   Sets the type of room the player is currently in, which adds room
//   effects to audio being rendered. This setting only takes effect 
//   if either Doldby Heaphone or Dolby Virtual Speaker engine is being
//   used.
// Parameters:
//   ic: Valid and initialized ICECLIENT pointer
//   mode: specify the type of room the player is in, see enumerated type
//   ICECLIENT_ROOM_MODE for possible values
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetRoomMode(ICECLIENT* ic, ICECLIENT_ROOM_MODE mode); 


// Description:
//   Selects the voice font preset to change outgoing voice
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   preset: id of the voice font preset to use. Use ICECLIENT_VOICEFONT_NONE
//   to disable voice font
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetVoiceFont(ICECLIENT* ic, ICECLIENT_VOICEFONT_PRESET preset);


// Description:
//   Sets the voice font configuration to change outgoing voice
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   vfont: voice font settings
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
//   To disable voice font call ICEClient_SetVoiceFont(ICECLIENT_VOICEFONT_NONE)
//   or call ICEClient_SetCustomVoiceFont with vfont.pitchchange = 1 and
//   vfont.timbrechange = 1
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetCustomVoiceFont(ICECLIENT* ic, ICEClient_voicefont vfont);

// Description:
//   This function specifies whether the player is speaking to their team.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   on: If set to 1, the player's voice will be sent to the channel mapping
//       If set to 0, the player's voice will not be sent to the channel mapping
//	 channel_mapping: the channel mapping to enable or disable
// Return value:
//   No return value.
// Note:
// - channel_mapping is the id that the player uses to refer to a channel. When
//   a player is given talking privilege in a channel using the server side
//   ICE API, a channel_mapping value is passed to ICE_channel_talk_join()
//   That channel_mapping value is then used on the client side to specify
//   which channel a player is talking on.
__declspec(dllexport) void ICEClient_TalkInto(ICECLIENT* ic, const int on, const unsigned char channel_mapping);


// Description:
//   This function specifies which channel mappings the player is talking into.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   channel_mappings: the channel mappings the player will talk into. Any 
//     unlisted channel mappings will be disabled
// len: the length of the array channel_mappings
// Return value:
//   No return value.
// Note:
// - channel_mappings is an array of ids that the player uses to refer to channels.
//   When a player is given talking privilege in a channel using the server side
//   ICE API, a channel_mapping value is passed to ICE_channel_talk_join()
//   That channel_mapping value is then used on the client side to specify
//   which channel a player is talking on.
__declspec(dllexport) void ICEClient_OnlyTalkInto(ICECLIENT* ic, const unsigned char* channel_mappings, const unsigned short len);


// Description:
//   MuteMic selects if the player's voice is being transmitted.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   on: If true then the player's voice is not sent.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_MuteMic(ICECLIENT* ic, int on);


// Description:
//   Mute selects if the audio should be rendered.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   on: If true the audio scene is not rendered.
// Return value:
//   No return value.
__declspec(dllexport) void ICEClient_Mute(ICECLIENT* ic, int on);


// Description:
//   Fills in the devices array with the available audio devices
//   on the system.
// Parameters:
//   devices: Array of length len of ICEClient_device.
//   len: Length of the supplied array. Changed to the number of
//   devices on return.
// Return value:
//   ICECLIENT_ERROR_NONE if success, ICECLIENT_ERROR_DATA_TOO_BIG
//   if the array is too small to fit all available devices. len is 
//   then set to the required size of the devices array.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_GetDeviceList(ICEClient_device* devices, unsigned int *len);


// Description:
//   Sets the default device to be used from now on. Client instances
//   created in the future would be affected by this.
//   If client is premium, spkdev->enginetype specifies the type of audio
//   engine to be used.
//   We recommend the following values for spkdev->enginetype depending on
//   the type of output device being used:
//   - ICECLIENT_ENGINE_DOLBYHEADPHONE for headphones
//   - ICECLIENT_ENGINE_DOLBYVIRTUALSPEAKER for stereo speakers
//   - ICECLIENT_ENGINE_DIRECTX for surround speakers
//   Note that from release 1.1.1, ICECLIENT_ENGINE_DEFAULT is equivalent to
//   ICECLIENT_ENGINE_DOLBYHEADPHONE
// Parameters:
//   spkdev: The output device. NULL means keep current device.
//   micdev: The capture device. NULL means keep current device.
// Return value.
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetDefaultDevice(const ICEClient_device* spkdev, const ICEClient_device* micdev);


// Description:
// Helper function that returns the audio engine type that
// we recommend for a given output device type. The returned engine type can
// then be used in ICEClient_Init and ICEClient_SetDevice calls.
// Parameters:
//   devtype: The output device type.
// Return value.
//     ICECLIENT_ENGINE_DOLBYHEADPHONE if devtype is ICECLIENT_DEVICE_HEADPHONES
//     ICECLIENT_ENGINE_DOLBYVIRTUALSPEAKER if devtype is ICECLIENT_DEVICE_STEREOSPEAKERS
//     ICECLIENT_ENGINE_DIRECTX if devtype is ICECLIENT_DEVICE_SURROUNDSPEAKERS
//     ICECLIENT_ENGINE_DEFAULT if devtype is an invalid/unknown speaker type
// Notes:
//   - In output devices returned by GetDeviceList or GetOutputDevice calls,
//     the field "devtype" is set based on speaker configuration settings in Windows
//     and may not correctly reflect the type of the physical output device.
//     To ensure the best audio experience, we recommended that the
//     application allows users to specify the type of output device being used,
//     which can then be translated into a suitable audio engine type using this API call.
__declspec(dllexport) ICECLIENT_ENGINE_TYPE ICEClient_GetDefaultEngineType(const ICECLIENT_DEVICE_TYPE devtype);


// Description:
//   Sets audio device for the given ice client instance to use
//   from now on.
//   If client is premium, spkdev->enginetype specifies the type of audio
//   engine to be used.
//   We recommend the following values for spkdev->enginetype depending on
//   the type of output being used:
//   - ICECLIENT_ENGINE_DOLBYHEADPHONE for headphones
//   - ICECLIENT_ENGINE_DOLBYVIRTUALSPEAKER for stereo speakers
//   - ICECLIENT_ENGINE_DIRECTX for surround speakers
//   Note that from release 1.1.1, ICECLIENT_ENGINE_DEFAULT is equivalent to
//   ICECLIENT_ENGINE_DOLBYHEADPHONE
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby..
//   spkdev: The output device to use. NULL means keep current device.
//   micdev: The capture device to use. NULL means keep current device.
// Return value:
//   ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetDevice(ICECLIENT* ic, const ICEClient_device* spkdev, const ICEClient_device* micdev); 


// Description:
//   Returns the currently used audio device.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby..
//   spkdev: Device structure to be filled in with audio device.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_GetOutputDevice(ICECLIENT* ic, ICEClient_device* spkdev); 


// Description:
//   Returns the currently used capture device.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init
//   or ICEClient_InitDolby.
//   micdev: Device structure to be filled in with capture device.
// Return value:
//   ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_GetCaptureDevice(ICECLIENT* ic, ICEClient_device* micdev); 


// Description:
//   Plays surround audio to test audio output.
//   If client is premium, test audio will be played using the
//   current audio engine.
//   If the client is non premium, a surround sound audio engine will
//   be created to play test audio. The type of audio engine created
//   depends on the type of output device (as reported by DirectX):
//   Dolby Headphone engine for headphones, Dolby Virtual Speaker
//   engine for stereo speakers, DirectX engine for surround speakers.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby.
//   mode: Type of output test. Possible options include 5.1 speaker enumeration
//   with voice, 5.1 speaker enumeration with noise, and noise sweeping
//   through 360 degrees.
//   callback: Function to call when output test is done, could be NULL.
//   rock: Void pointer to pass to callback function, could be NULL.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_StartSpkTest(ICECLIENT* ic, ICECLIENT_SPKTEST_MODE mode, void (*callback)(void*), void *rock);


// Description:
//   Stops test output if it is currently on.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init
//   or ICEClient_InitDolby.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_StopSpkTest(ICECLIENT* ic);


// Description:
//   Start recording audio for microphone test. The audio will be recorded in a
//   buffer of up to 10 seconds and can be played back by a call to
//   StartMicTestPhase2.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init
//   or ICEClient_InitDolby.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_StartMicTestPhase1(ICECLIENT* ic);


// Description:
//   Plays the audio recorded from the call to ICEClient_MicTestPhase1.
//   Recorded audio is played locally if mode is ICECLIENT_MICTEST_LOOPBACK,
//   echos from server if mode is ICECLIENT_MICTEST_TO_SERVER.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init
//   or ICEClient_InitDolby.
//   mode: Type of test, either ICECLIENT_MICTEST_LOOPBACK or
//   ICECLIENT_MICTEST_TO_SERVER
//   callback: Function to call when microphone test is done, could be NULL.
//   rock: Void pointer to pass to callback function, could be NULL.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_StartMicTestPhase2(ICECLIENT* ic, ICECLIENT_MICTEST_MODE mode, void (*callback)(void*), void *rock);


// Description:
//   Stop recording if currently recording (ICEClient_MicTestPhase1),
//   or stop test playback if ICEClient_MicTestPhase2 had already been
//   called.
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby.
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_StopMicTest(ICECLIENT* ic);


// Description:
//   Enables game enpasulation mode and sets the encapsulation callback function
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   encapsulatefunc: Callback funtion to handle data to be sent to the audio server.
//   The callback function should take three arguments: the data to send (msg),
//   length of the data (len) and a void pointer (rock).
//   rock: A void pointer that is to be passed back to callback function
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   This function only works in single server mode
//   Calling this function disable sending audio directly to the server
//   if it was previously set with SetServer()
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetGameEncapsulation(ICECLIENT* ic, void (*encapsulatefunc)(const char* msg, unsigned int len, void* rock), void* rock);


// Description:
//   Pass encapsulated data received by the game client to Axon audio client.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   msg: pointer to the data
//   len: size of the data
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   This function only works in single server mode
__declspec(dllexport) ICECLIENT_ERROR ICEClient_ReceiveCapsule(ICECLIENT* ic, const char* msg, unsigned int len);


// Description:
//   Switch on/off peer to peer mode. In peer to peer mode the client will only
//   send audio packets, and will keep sending regardless of any reply
//   from its peer.
//   This is intended for use in conjunction with encapsulation mode, where the
//   application takes care of NAT traversing and delivering packet data between the peers.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or ICEClient_InitDolby.
//   int: use non-zero value to turns on and zero value to turn off peer to peer mode
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetP2P(ICECLIENT* ic, int on);


// Description:
//   Provides a gameicon that could be used by the external Dolby Axon
//   chat client to indicate which game the player is playing
// Parameters:
//   ic: Valid ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby.
//   data: a pointer to the string of bytes containing the icon data.
//   Icon data should be the content of either a .PNG or .ICO file.
//   Set to NULL to clear any existing icon.
//   len: the number of bytes used to store this icon. Maximum size is
//   1000000 bytes
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetGameIcon(ICECLIENT* ic, const char* data, unsigned int len);


// Description:
//   Switch echo suppression on or off (default on)
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init
//   on: if 1 turns echo supression on and if 0 turns echo supression off
// Return value
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetEchoSupression(ICECLIENT* ic, int on);


// Description:
//   Turn on internal audio logging, used for debugging the audio
//   processing components. By default audio logging is not turned on
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init
//   on: if 1 audio logging on is turned on and if 0 audio logging
//   is turned off
// Return value
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_SetAudioLogging(ICECLIENT* ic, int on);


// Description:
//   Dump the internal state of the client into a file. If audio logging
//   has been turned on, audio logs are also dumped.
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_DumpState(ICECLIENT* ic);


// Description:
//   Dump the internal state of the client into files in memory. If audio logging
//   has been turned on and include_audio is 1, audio logs are also dumped.
//   This function is similar to DumpState, except that state dumps are stored in
//   memory and can be retrieved via ICEClient_GetStateDumpFile.
// Parameters:
//   include_audio: if 1 audio logs will also be dumped.
// Return value:
//   number of log files created
__declspec(dllexport) unsigned int ICEClient_CreateStateDump(ICECLIENT* ic, const int include_audio);


// Description:
//   Retrieve a state dump file that was created by ICEClient_CreateStateDump.
// Parameters:
//   index: index of the dump file to be retrieved, index of the first file is 0
//   filename: address of a pointer that will be pointing to NULL terminated
//   string containing filename
//   data: address of pointer that will be pointing to file data
//   len: address of a variable that will contain size of file data in bytes
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails
// Notes:
//   filename and data pointers are valid untill ICEClient_CreateStateDump
//   is called again or ICEClient_ClearStateDump is called.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_GetStateDumpFile(ICECLIENT* ic, const unsigned int index, const char** filename, const char** data, unsigned int *len);


// Description:
//   Clear the memory used by state dump files that were created by CreateStateDump.
// Parameters:
//   None
// Return value:
//   None
__declspec(dllexport) void ICEClient_ClearStateDump(ICECLIENT* ic);


// Description:
//   Plays given wave data
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby.
//   data: pointer to wave data
//   len: size of wave data, in bytes
//   spec: format of the wave data
//   repeat: number of times the data should be played, set to 1 to play just once
//   volume: multiplication factor to apply to wav samples. Set to 1.0f to leave samples unchanged
//   wav_id: pointer to variable that will hold the id of this wave, which can
//   later be used to cancel its playback. Could be NULL if the id is not required.
//   callback: function to call when the playback is completed, may be NULL
//   rock: pointer to pass back to callback function, may be NULL
// Return value:
//   ICECLIENT_ERROR_NONE if successful; non-zero ICECLIENT_ERROR if it fails.
// Note:
//   The wave data passed to this function should include only audio samples without
//   any file headers. Thus if data is read from a .wav file on disk, wav headers must
//   be discarded before calling this function.
__declspec(dllexport) ICECLIENT_ERROR ICEClient_PlayWav(ICECLIENT* ic, const char* data, const unsigned int len, const ICEClient_wavspec spec, const unsigned int repeat, const float volume, unsigned int *wav_id, void (*callback)(void*), void* rock);


// Description:
//   Cancels the playback of a wave with given id
// Parameters:
//   ic: ICECLIENT pointer initialized by ICEClient_Init or
//   ICEClient_InitDolby.
//   wav_id: the wave's id, as set by ICEClient_PlayWav when the playback was started
// Return value:
//   None
__declspec(dllexport) void ICEClient_CancelWav(ICECLIENT* ic, const unsigned int wav_id);


// Description:
//   Returns the version string of the audio client.
// Parameters:
//   None.
// Return value:
//   Audio client version string.
__declspec(dllexport) const char* ICEClient_Version();


// Description:
//   Returns the interface's version string.
// Parameters:
//   None.
// Return value:
//   The interface version string.
__declspec(dllexport) const char* ICEClient_InterfaceVersion();


// Description:
//   Converts an ICECLIENT_ERROR error code into a human readable string.
// Parameters:
//   e: The error code.
// Return value:
//   The human readable string.
__declspec(dllexport) const char* ICEClient_ErrorToString(const ICECLIENT_ERROR e);


#ifdef __cplusplus
}
#endif
