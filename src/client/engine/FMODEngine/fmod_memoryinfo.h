#ifdef USE_FMOD_DLL	// Old version

/* ============================================================================================= */
/* FMOD Ex - Memory info header file. Copyright (c), Firelight Technologies Pty, Ltd. 2008-2009. */
/*                                                                                               */
/* Use this header if you are interested in getting detailed information on FMOD's memory        */
/* usage. See the documentation for more details.                                                */
/*                                                                                               */
/* ============================================================================================= */

#ifndef _FMOD_MEMORYINFO_H
#define _FMOD_MEMORYINFO_H

/*
[ENUM]
[
[DESCRIPTION]
Description of "memoryused_array" fields returned by the getMemoryInfo function of every public FMOD class.

[REMARKS]
Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
The FMOD_MEMTYPE enumeration values can be used to address the "memoryused_array" returned by getMemoryInfo. See System::getMemoryInfo for an example.

[PLATFORMS]
Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

[SEE_ALSO]
System::getMemoryInfo
EventSystem::getMemoryInfo
FMOD_MEMBITS
FMOD_EVENT_MEMBITS
]
*/
typedef enum
{
	FMOD_MEMTYPE_OTHER                 = 0,      /* Memory not accounted for by other types */
	FMOD_MEMTYPE_STRING                = 1,      /* String data */

	FMOD_MEMTYPE_SYSTEM                = 2,      /* System object and various internals */
	FMOD_MEMTYPE_PLUGINS               = 3,      /* Plugin objects and internals */
	FMOD_MEMTYPE_OUTPUT                = 4,      /* Output module object and internals */
	FMOD_MEMTYPE_CHANNEL               = 5,      /* Channel related memory */
	FMOD_MEMTYPE_CHANNELGROUP          = 6,      /* ChannelGroup objects and internals */
	FMOD_MEMTYPE_CODEC                 = 7,      /* Codecs allocated for streaming */
	FMOD_MEMTYPE_FILE                  = 8,      /* File buffers and structures */
	FMOD_MEMTYPE_SOUND                 = 9,      /* Sound objects and internals */
	FMOD_MEMTYPE_SOUND_SECONDARYRAM    = 10,     /* Sound data stored in secondary RAM */
	FMOD_MEMTYPE_SOUNDGROUP            = 11,     /* SoundGroup objects and internals */
	FMOD_MEMTYPE_STREAMBUFFER          = 12,     /* Stream buffer memory */
	FMOD_MEMTYPE_DSPCONNECTION         = 13,     /* DSPConnection objects and internals */
	FMOD_MEMTYPE_DSP                   = 14,     /* DSP implementation objects */
	FMOD_MEMTYPE_DSPCODEC              = 15,     /* Realtime file format decoding DSP objects */
	FMOD_MEMTYPE_PROFILE               = 16,     /* Profiler memory footprint. */
	FMOD_MEMTYPE_RECORDBUFFER          = 17,     /* Buffer used to store recorded data from microphone */
	FMOD_MEMTYPE_REVERB                = 18,     /* Reverb implementation objects */
	FMOD_MEMTYPE_REVERBCHANNELPROPS    = 19,     /* Reverb channel properties structs */
	FMOD_MEMTYPE_GEOMETRY              = 20,     /* Geometry objects and internals */
	FMOD_MEMTYPE_SYNCPOINT             = 21,     /* Sync point memory. */

	FMOD_MEMTYPE_EVENTSYSTEM           = 22,     /* EventSystem and various internals */
	FMOD_MEMTYPE_MUSICSYSTEM           = 23,     /* MusicSystem and various internals */
	FMOD_MEMTYPE_FEV                   = 24,     /* Definition of objects contained in all loaded projects e.g. events, groups, categories */
	FMOD_MEMTYPE_MEMORYFSB             = 25,     /* Data loaded with registerMemoryFSB */
	FMOD_MEMTYPE_EVENTPROJECT          = 26,     /* EventProject objects and internals */
	FMOD_MEMTYPE_EVENTGROUPI           = 27,     /* EventGroup objects and internals */
	FMOD_MEMTYPE_SOUNDBANKCLASS        = 28,     /* Objects used to manage wave banks */
	FMOD_MEMTYPE_SOUNDBANKLIST         = 29,     /* Data used to manage lists of wave bank usage */
	FMOD_MEMTYPE_STREAMINSTANCE        = 30,     /* Stream objects and internals */
	FMOD_MEMTYPE_SOUNDDEFCLASS         = 31,     /* Sound definition objects */
	FMOD_MEMTYPE_SOUNDDEFDEFCLASS      = 32,     /* Sound definition static data objects */
	FMOD_MEMTYPE_SOUNDDEFPOOL          = 33,     /* Sound definition pool data */
	FMOD_MEMTYPE_REVERBDEF             = 34,     /* Reverb definition objects */
	FMOD_MEMTYPE_EVENTREVERB           = 35,     /* Reverb objects */
	FMOD_MEMTYPE_USERPROPERTY          = 36,     /* User property objects */
	FMOD_MEMTYPE_EVENTINSTANCE         = 37,     /* Event instance base objects */
	FMOD_MEMTYPE_EVENTINSTANCE_COMPLEX = 38,     /* Complex event instance objects */
	FMOD_MEMTYPE_EVENTINSTANCE_SIMPLE  = 39,     /* Simple event instance objects */
	FMOD_MEMTYPE_EVENTINSTANCE_LAYER   = 40,     /* Event layer instance objects */
	FMOD_MEMTYPE_EVENTINSTANCE_SOUND   = 41,     /* Event sound instance objects */
	FMOD_MEMTYPE_EVENTENVELOPE         = 42,     /* Event envelope objects */
	FMOD_MEMTYPE_EVENTENVELOPEDEF      = 43,     /* Event envelope definition objects */
	FMOD_MEMTYPE_EVENTPARAMETER        = 44,     /* Event parameter objects */
	FMOD_MEMTYPE_EVENTCATEGORY         = 45,     /* Event category objects */
	FMOD_MEMTYPE_EVENTENVELOPEPOINT    = 46,     /* Event envelope point objects */
	FMOD_MEMTYPE_EVENTINSTANCEPOOL     = 47,     /* Event instance pool memory */

	FMOD_MEMTYPE_MAX,                            /* Number of "memoryused_array" fields. */
	FMOD_MEMTYPE_FORCEINT = 65536                /* Makes sure this enum is signed 32bit */

} FMOD_MEMTYPE;


/*
[DEFINE]
[
[NAME]
FMOD_MEMBITS

[DESCRIPTION]
Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Ex class.
Use with the "memorybits" parameter of getMemoryInfo to get information on FMOD Ex memory usage.

[REMARKS]
Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
The FMOD_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See System::getMemoryInfo for an example.

[PLATFORMS]
Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

[SEE_ALSO]
FMOD_EVENT_MEMBITS
System::getMemoryInfo
]
*/
#define FMOD_MEMBITS_OTHER                       (1 << FMOD_MEMTYPE_OTHER)               /* Memory not accounted for by other types */
#define FMOD_MEMBITS_STRING                      (1 << FMOD_MEMTYPE_STRING)              /* String data */

#define FMOD_MEMBITS_SYSTEM                      (1 << FMOD_MEMTYPE_SYSTEM)              /* System object and various internals */
#define FMOD_MEMBITS_PLUGINS                     (1 << FMOD_MEMTYPE_PLUGINS)             /* Plugin objects and internals */
#define FMOD_MEMBITS_OUTPUT                      (1 << FMOD_MEMTYPE_OUTPUT)              /* Output module object and internals */
#define FMOD_MEMBITS_CHANNEL                     (1 << FMOD_MEMTYPE_CHANNEL)             /* Channel related memory */
#define FMOD_MEMBITS_CHANNELGROUP                (1 << FMOD_MEMTYPE_CHANNELGROUP)        /* ChannelGroup objects and internals */
#define FMOD_MEMBITS_CODEC                       (1 << FMOD_MEMTYPE_CODEC)               /* Codecs allocated for streaming */
#define FMOD_MEMBITS_FILE                        (1 << FMOD_MEMTYPE_FILE)                /* Codecs allocated for streaming */
#define FMOD_MEMBITS_SOUND                       (1 << FMOD_MEMTYPE_SOUND)               /* Sound objects and internals */
#define FMOD_MEMBITS_SOUND_SECONDARYRAM          (1 << FMOD_MEMTYPE_SOUND_SECONDARYRAM)  /* Sound data stored in secondary RAM */
#define FMOD_MEMBITS_SOUNDGROUP                  (1 << FMOD_MEMTYPE_SOUNDGROUP)          /* SoundGroup objects and internals */
#define FMOD_MEMBITS_STREAMBUFFER                (1 << FMOD_MEMTYPE_STREAMBUFFER)        /* Stream buffer memory */
#define FMOD_MEMBITS_DSPCONNECTION               (1 << FMOD_MEMTYPE_DSPCONNECTION)       /* DSPConnection objects and internals */
#define FMOD_MEMBITS_DSP                         (1 << FMOD_MEMTYPE_DSP)                 /* DSP implementation objects */
#define FMOD_MEMBITS_DSPCODEC                    (1 << FMOD_MEMTYPE_DSPCODEC)            /* Realtime file format decoding DSP objects */
#define FMOD_MEMBITS_PROFILE                     (1 << FMOD_MEMTYPE_PROFILE)             /* Profiler memory footprint. */
#define FMOD_MEMBITS_RECORDBUFFER                (1 << FMOD_MEMTYPE_RECORDBUFFER)        /* Buffer used to store recorded data from microphone */
#define FMOD_MEMBITS_REVERB                      (1 << FMOD_MEMTYPE_REVERB)              /* Reverb implementation objects */
#define FMOD_MEMBITS_REVERBCHANNELPROPS          (1 << FMOD_MEMTYPE_REVERBCHANNELPROPS)  /* Reverb channel properties structs */
#define FMOD_MEMBITS_GEOMETRY                    (1 << FMOD_MEMTYPE_GEOMETRY)            /* Geometry objects and internals */
#define FMOD_MEMBITS_SYNCPOINT                   (1 << FMOD_MEMTYPE_SYNCPOINT)           /* Sync point memory. */
#define FMOD_MEMBITS_ALL                         0xffffffff                              /* All memory used by FMOD Ex */
/* [DEFINE_END] */                                                                  


/*
[DEFINE]
[
[NAME]
FMOD_EVENT_MEMBITS

[DESCRIPTION]
Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Event System class.
Use with the "event_memorybits" parameter of getMemoryInfo to get information on FMOD Event System memory usage.

[REMARKS]
Every public FMOD Event System class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
The FMOD_EVENT_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See EventSystem::getMemoryInfo for an example.

[PLATFORMS]
Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

[SEE_ALSO]
FMOD_MEMBITS
System::getMemoryInfo
]
*/
#define FMOD_EVENT_MEMBITS_EVENTSYSTEM           (1 << (FMOD_MEMTYPE_EVENTSYSTEM           - FMOD_MEMTYPE_EVENTSYSTEM))  /* EventSystem and various internals */
#define FMOD_EVENT_MEMBITS_MUSICSYSTEM           (1 << (FMOD_MEMTYPE_MUSICSYSTEM           - FMOD_MEMTYPE_EVENTSYSTEM))  /* MusicSystem and various internals */
#define FMOD_EVENT_MEMBITS_FEV                   (1 << (FMOD_MEMTYPE_FEV                   - FMOD_MEMTYPE_EVENTSYSTEM))  /* Definition of objects contained in all loaded projects e.g. events, groups, categories */
#define FMOD_EVENT_MEMBITS_MEMORYFSB             (1 << (FMOD_MEMTYPE_MEMORYFSB             - FMOD_MEMTYPE_EVENTSYSTEM))  /* Data loaded with registerMemoryFSB */
#define FMOD_EVENT_MEMBITS_EVENTPROJECT          (1 << (FMOD_MEMTYPE_EVENTPROJECT          - FMOD_MEMTYPE_EVENTSYSTEM))  /* EventProject objects and internals */
#define FMOD_EVENT_MEMBITS_EVENTGROUPI           (1 << (FMOD_MEMTYPE_EVENTGROUPI           - FMOD_MEMTYPE_EVENTSYSTEM))  /* EventGroup objects and internals */
#define FMOD_EVENT_MEMBITS_SOUNDBANKCLASS        (1 << (FMOD_MEMTYPE_SOUNDBANKCLASS        - FMOD_MEMTYPE_EVENTSYSTEM))  /* Objects used to manage wave banks */
#define FMOD_EVENT_MEMBITS_SOUNDBANKLIST         (1 << (FMOD_MEMTYPE_SOUNDBANKLIST         - FMOD_MEMTYPE_EVENTSYSTEM))  /* Data used to manage lists of wave bank usage */
#define FMOD_EVENT_MEMBITS_STREAMINSTANCE        (1 << (FMOD_MEMTYPE_STREAMINSTANCE        - FMOD_MEMTYPE_EVENTSYSTEM))  /* Stream objects and internals */
#define FMOD_EVENT_MEMBITS_SOUNDDEFCLASS         (1 << (FMOD_MEMTYPE_SOUNDDEFCLASS         - FMOD_MEMTYPE_EVENTSYSTEM))  /* Sound definition objects */
#define FMOD_EVENT_MEMBITS_SOUNDDEFDEFCLASS      (1 << (FMOD_MEMTYPE_SOUNDDEFDEFCLASS      - FMOD_MEMTYPE_EVENTSYSTEM))  /* Sound definition static data objects */
#define FMOD_EVENT_MEMBITS_SOUNDDEFPOOL          (1 << (FMOD_MEMTYPE_SOUNDDEFPOOL          - FMOD_MEMTYPE_EVENTSYSTEM))  /* Sound definition pool data */
#define FMOD_EVENT_MEMBITS_REVERBDEF             (1 << (FMOD_MEMTYPE_REVERBDEF             - FMOD_MEMTYPE_EVENTSYSTEM))  /* Reverb definition objects */
#define FMOD_EVENT_MEMBITS_EVENTREVERB           (1 << (FMOD_MEMTYPE_EVENTREVERB           - FMOD_MEMTYPE_EVENTSYSTEM))  /* Reverb objects */
#define FMOD_EVENT_MEMBITS_USERPROPERTY          (1 << (FMOD_MEMTYPE_USERPROPERTY          - FMOD_MEMTYPE_EVENTSYSTEM))  /* User property objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE         (1 << (FMOD_MEMTYPE_EVENTINSTANCE         - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event instance base objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_COMPLEX (1 << (FMOD_MEMTYPE_EVENTINSTANCE_COMPLEX - FMOD_MEMTYPE_EVENTSYSTEM))  /* Complex event instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_SIMPLE  (1 << (FMOD_MEMTYPE_EVENTINSTANCE_SIMPLE  - FMOD_MEMTYPE_EVENTSYSTEM))  /* Simple event instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_LAYER   (1 << (FMOD_MEMTYPE_EVENTINSTANCE_LAYER   - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event layer instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_SOUND   (1 << (FMOD_MEMTYPE_EVENTINSTANCE_SOUND   - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event sound instance objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPE         (1 << (FMOD_MEMTYPE_EVENTENVELOPE         - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event envelope objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPEDEF      (1 << (FMOD_MEMTYPE_EVENTENVELOPEDEF      - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event envelope definition objects */
#define FMOD_EVENT_MEMBITS_EVENTPARAMETER        (1 << (FMOD_MEMTYPE_EVENTPARAMETER        - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event parameter objects */
#define FMOD_EVENT_MEMBITS_EVENTCATEGORY         (1 << (FMOD_MEMTYPE_EVENTCATEGORY         - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event category objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPEPOINT    (1 << (FMOD_MEMTYPE_EVENTENVELOPEPOINT    - FMOD_MEMTYPE_EVENTSYSTEM))  /* Event envelope point objects */
#define FMOD_EVENT_MEMBITS_ALL                   0xffffffff                                                              /* All memory used by FMOD Event System */

/* All event instance memory */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_GROUP   (FMOD_EVENT_MEMBITS_EVENTINSTANCE            | \
	FMOD_EVENT_MEMBITS_EVENTINSTANCE_COMPLEX | \
	FMOD_EVENT_MEMBITS_EVENTINSTANCE_SIMPLE  | \
	FMOD_EVENT_MEMBITS_EVENTINSTANCE_LAYER   | \
	FMOD_EVENT_MEMBITS_EVENTINSTANCE_SOUND)

/* All sound definition memory */
#define FMOD_EVENT_MEMBITS_SOUNDDEF_GROUP        (FMOD_EVENT_MEMBITS_SOUNDDEFCLASS            | \
	FMOD_EVENT_MEMBITS_SOUNDDEFDEFCLASS      | \
	FMOD_EVENT_MEMBITS_SOUNDDEFPOOL)
/* [DEFINE_END] */

#endif

#else	// #ifdef USE_FMOD_DLL

/* ============================================================================================= */
/* FMOD Ex - Memory info header file. Copyright (c), Firelight Technologies Pty, Ltd. 2008-2009. */
/*                                                                                               */
/* Use this header if you are interested in getting detailed information on FMOD's memory        */
/* usage. See the documentation for more details.                                                */
/*                                                                                               */
/* ============================================================================================= */

#ifndef _FMOD_MEMORYINFO_H
#define _FMOD_MEMORYINFO_H

/*
[STRUCTURE]
[
    [DESCRIPTION]
    Structure to be filled with detailed memory usage information of an FMOD object

    [REMARKS]
    Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
    On return from getMemoryInfo, each member of this structure will hold the amount of memory used for its type in bytes.<br>
    <br>
    Members marked with [in] mean the user sets the value before passing it to the function.<br>
    Members marked with [out] mean FMOD sets the value to be used after the function exits.<br>


    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox, Xbox360, PlayStation 2, GameCube, PlayStation Portable, PlayStation 3

    [SEE_ALSO]
    System::getMemoryInfo
    EventSystem::getMemoryInfo
    FMOD_MEMBITS    
    FMOD_EVENT_MEMBITS
]
*/
typedef struct FMOD_MEMORY_USAGE_DETAILS
{
    unsigned int other;                          /* [out] Memory not accounted for by other types */
    unsigned int string;                         /* [out] String data */
    unsigned int system;                         /* [out] System object and various internals */
    unsigned int plugins;                        /* [out] Plugin objects and internals */
    unsigned int output;                         /* [out] Output module object and internals */
    unsigned int channel;                        /* [out] Channel related memory */
    unsigned int channelgroup;                   /* [out] ChannelGroup objects and internals */
    unsigned int codec;                          /* [out] Codecs allocated for streaming */
    unsigned int file;                           /* [out] File buffers and structures */
    unsigned int sound;                          /* [out] Sound objects and internals */
    unsigned int secondaryram;                   /* [out] Sound data stored in secondary RAM */
    unsigned int soundgroup;                     /* [out] SoundGroup objects and internals */
    unsigned int streambuffer;                   /* [out] Stream buffer memory */
    unsigned int dspconnection;                  /* [out] DSPConnection objects and internals */
    unsigned int dsp;                            /* [out] DSP implementation objects */
    unsigned int dspcodec;                       /* [out] Realtime file format decoding DSP objects */
    unsigned int profile;                        /* [out] Profiler memory footprint. */
    unsigned int recordbuffer;                   /* [out] Buffer used to store recorded data from microphone */
    unsigned int reverb;                         /* [out] Reverb implementation objects */
    unsigned int reverbchannelprops;             /* [out] Reverb channel properties structs */
    unsigned int geometry;                       /* [out] Geometry objects and internals */
    unsigned int syncpoint;                      /* [out] Sync point memory. */
    unsigned int eventsystem;                    /* [out] EventSystem and various internals */
    unsigned int musicsystem;                    /* [out] MusicSystem and various internals */
    unsigned int fev;                            /* [out] Definition of objects contained in all loaded projects e.g. events, groups, categories */
    unsigned int memoryfsb;                      /* [out] Data loaded with registerMemoryFSB */
    unsigned int eventproject;                   /* [out] EventProject objects and internals */
    unsigned int eventgroupi;                    /* [out] EventGroup objects and internals */
    unsigned int soundbankclass;                 /* [out] Objects used to manage wave banks */
    unsigned int soundbanklist;                  /* [out] Data used to manage lists of wave bank usage */
    unsigned int streaminstance;                 /* [out] Stream objects and internals */
    unsigned int sounddefclass;                  /* [out] Sound definition objects */
    unsigned int sounddefdefclass;               /* [out] Sound definition static data objects */
    unsigned int sounddefpool;                   /* [out] Sound definition pool data */
    unsigned int reverbdef;                      /* [out] Reverb definition objects */
    unsigned int eventreverb;                    /* [out] Reverb objects */
    unsigned int userproperty;                   /* [out] User property objects */
    unsigned int eventinstance;                  /* [out] Event instance base objects */
    unsigned int eventinstance_complex;          /* [out] Complex event instance objects */
    unsigned int eventinstance_simple;           /* [out] Simple event instance objects */
    unsigned int eventinstance_layer;            /* [out] Event layer instance objects */
    unsigned int eventinstance_sound;            /* [out] Event sound instance objects */
    unsigned int eventenvelope;                  /* [out] Event envelope objects */
    unsigned int eventenvelopedef;               /* [out] Event envelope definition objects */
    unsigned int eventparameter;                 /* [out] Event parameter objects */
    unsigned int eventcategory;                  /* [out] Event category objects */
    unsigned int eventenvelopepoint;             /* [out] Event envelope point objects */
    unsigned int eventinstancepool;              /* [out] Event instance pool memory */
} FMOD_MEMORY_USAGE_DETAILS;


/*
[DEFINE]
[
    [NAME]
    FMOD_MEMBITS

    [DESCRIPTION]
    Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Ex class.
    Use with the "memorybits" parameter of getMemoryInfo to get information on FMOD Ex memory usage.

    [REMARKS]
    Every public FMOD class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
    The FMOD_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See System::getMemoryInfo for an example.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_EVENT_MEMBITS
    System::getMemoryInfo
]
*/
#define FMOD_MEMBITS_OTHER                       0x00000001  /* Memory not accounted for by other types */
#define FMOD_MEMBITS_STRING                      0x00000002  /* String data */

#define FMOD_MEMBITS_SYSTEM                      0x00000004  /* System object and various internals */
#define FMOD_MEMBITS_PLUGINS                     0x00000008  /* Plugin objects and internals */
#define FMOD_MEMBITS_OUTPUT                      0x00000010  /* Output module object and internals */
#define FMOD_MEMBITS_CHANNEL                     0x00000020  /* Channel related memory */
#define FMOD_MEMBITS_CHANNELGROUP                0x00000040  /* ChannelGroup objects and internals */
#define FMOD_MEMBITS_CODEC                       0x00000080  /* Codecs allocated for streaming */
#define FMOD_MEMBITS_FILE                        0x00000100  /* Codecs allocated for streaming */
#define FMOD_MEMBITS_SOUND                       0x00000200  /* Sound objects and internals */
#define FMOD_MEMBITS_SOUND_SECONDARYRAM          0x00000400  /* Sound data stored in secondary RAM */
#define FMOD_MEMBITS_SOUNDGROUP                  0x00000800  /* SoundGroup objects and internals */
#define FMOD_MEMBITS_STREAMBUFFER                0x00001000  /* Stream buffer memory */
#define FMOD_MEMBITS_DSPCONNECTION               0x00002000  /* DSPConnection objects and internals */
#define FMOD_MEMBITS_DSP                         0x00004000  /* DSP implementation objects */
#define FMOD_MEMBITS_DSPCODEC                    0x00008000  /* Realtime file format decoding DSP objects */
#define FMOD_MEMBITS_PROFILE                     0x00010000  /* Profiler memory footprint. */
#define FMOD_MEMBITS_RECORDBUFFER                0x00020000  /* Buffer used to store recorded data from microphone */
#define FMOD_MEMBITS_REVERB                      0x00040000  /* Reverb implementation objects */
#define FMOD_MEMBITS_REVERBCHANNELPROPS          0x00080000  /* Reverb channel properties structs */
#define FMOD_MEMBITS_GEOMETRY                    0x00100000  /* Geometry objects and internals */
#define FMOD_MEMBITS_SYNCPOINT                   0x00200000  /* Sync point memory. */
#define FMOD_MEMBITS_ALL                         0xffffffff  /* All memory used by FMOD Ex */
/* [DEFINE_END] */


/*
[DEFINE]
[
    [NAME]
    FMOD_EVENT_MEMBITS

    [DESCRIPTION]
    Bitfield used to request specific memory usage information from the getMemoryInfo function of every public FMOD Event System class.
    Use with the "event_memorybits" parameter of getMemoryInfo to get information on FMOD Event System memory usage.

    [REMARKS]
    Every public FMOD Event System class has a getMemoryInfo function which can be used to get detailed information on what memory resources are associated with the object in question. 
    The FMOD_EVENT_MEMBITS defines can be OR'd together to specify precisely what memory usage you'd like to get information on. See EventSystem::getMemoryInfo for an example.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MEMBITS
    System::getMemoryInfo
]
*/                                               
#define FMOD_EVENT_MEMBITS_EVENTSYSTEM           0x00000001  /* EventSystem and various internals */
#define FMOD_EVENT_MEMBITS_MUSICSYSTEM           0x00000002  /* MusicSystem and various internals */
#define FMOD_EVENT_MEMBITS_FEV                   0x00000004  /* Definition of objects contained in all loaded projects e.g. events, groups, categories */
#define FMOD_EVENT_MEMBITS_MEMORYFSB             0x00000008  /* Data loaded with registerMemoryFSB */
#define FMOD_EVENT_MEMBITS_EVENTPROJECT          0x00000010  /* EventProject objects and internals */
#define FMOD_EVENT_MEMBITS_EVENTGROUPI           0x00000020  /* EventGroup objects and internals */
#define FMOD_EVENT_MEMBITS_SOUNDBANKCLASS        0x00000040  /* Objects used to manage wave banks */
#define FMOD_EVENT_MEMBITS_SOUNDBANKLIST         0x00000080  /* Data used to manage lists of wave bank usage */
#define FMOD_EVENT_MEMBITS_STREAMINSTANCE        0x00000100  /* Stream objects and internals */
#define FMOD_EVENT_MEMBITS_SOUNDDEFCLASS         0x00000200  /* Sound definition objects */
#define FMOD_EVENT_MEMBITS_SOUNDDEFDEFCLASS      0x00000400  /* Sound definition static data objects */
#define FMOD_EVENT_MEMBITS_SOUNDDEFPOOL          0x00000800  /* Sound definition pool data */
#define FMOD_EVENT_MEMBITS_REVERBDEF             0x00001000  /* Reverb definition objects */
#define FMOD_EVENT_MEMBITS_EVENTREVERB           0x00002000  /* Reverb objects */
#define FMOD_EVENT_MEMBITS_USERPROPERTY          0x00004000  /* User property objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE         0x00008000  /* Event instance base objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_COMPLEX 0x00010000  /* Complex event instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_SIMPLE  0x00020000  /* Simple event instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_LAYER   0x00040000  /* Event layer instance objects */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_SOUND   0x00080000  /* Event sound instance objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPE         0x00100000  /* Event envelope objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPEDEF      0x00200000  /* Event envelope definition objects */
#define FMOD_EVENT_MEMBITS_EVENTPARAMETER        0x00400000  /* Event parameter objects */
#define FMOD_EVENT_MEMBITS_EVENTCATEGORY         0x00800000  /* Event category objects */
#define FMOD_EVENT_MEMBITS_EVENTENVELOPEPOINT    0x01000000  /* Event envelope point object+s */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCEPOOL     0x02000000  /* Event instance pool data */
#define FMOD_EVENT_MEMBITS_ALL                   0xffffffff  /* All memory used by FMOD Event System */

/* All event instance memory */
#define FMOD_EVENT_MEMBITS_EVENTINSTANCE_GROUP   (FMOD_EVENT_MEMBITS_EVENTINSTANCE            | \
                                                     FMOD_EVENT_MEMBITS_EVENTINSTANCE_COMPLEX | \
                                                     FMOD_EVENT_MEMBITS_EVENTINSTANCE_SIMPLE  | \
                                                     FMOD_EVENT_MEMBITS_EVENTINSTANCE_LAYER   | \
                                                     FMOD_EVENT_MEMBITS_EVENTINSTANCE_SOUND)

/* All sound definition memory */
#define FMOD_EVENT_MEMBITS_SOUNDDEF_GROUP        (FMOD_EVENT_MEMBITS_SOUNDDEFCLASS            | \
                                                     FMOD_EVENT_MEMBITS_SOUNDDEFDEFCLASS      | \
                                                     FMOD_EVENT_MEMBITS_SOUNDDEFPOOL)
/* [DEFINE_END] */

#endif

#endif	// #ifdef USE_FMOD_DLL