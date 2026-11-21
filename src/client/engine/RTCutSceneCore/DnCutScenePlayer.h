#pragma once
//#include "Timer.h"
#include "EtMatrixEx.h"
#include "EternityEngine.h"
#include "IDnCutSceneDataReader.h"
#include "ICustomActorProcessor.h"

//#ifdef PRE_ADD_FILTEREVENT
#include "DnCutSceneWorld.h"
//#endif // PRE_ADD_FILTEREVENT

class CEtWorld;
class CEtDOFFilter;


// Actually receive data and play
class CDnCutScenePlayer
{
public:
	enum
	{
		ACTION_SEQUENCE,
		KEY_SEQUENCE,
		CAM_EVENT_SEQUENCE,
		PARTICLE_EVENT_SEQUENCE,
		DOF_EVENT_SEQUENCE,
		//SOUND_EVENT_SEQUENCE,
		FADE_EVENT_SEQUENCE,
		PROP_EVENT_SEQUENCE,
		SOUND_EVENT_1_SEQUENCE,
		SOUND_EVENT_2_SEQUENCE,
		SOUND_EVENT_3_SEQUENCE,
		SOUND_EVENT_4_SEQUENCE,
		SOUND_EVENT_5_SEQUENCE,
		SOUND_EVENT_6_SEQUENCE,
		SOUND_EVENT_7_SEQUENCE,
		SOUND_EVENT_8_SEQUENCE,
		SUBTITLE_EVENT_SEQUENCE,
// #ifdef PRE_ADD_FILTEREVENT
		COLORFILTER_EVENT_SEQUENCE,
//#endif // PRE_ADD_FILTEREVENT
		SEQUENCE_COUNT,
	};

	struct S_SEQUENCE
	{
		S_CS_ACTOR_INFO*		pAniObjectInfoToPlay;
		int iType;

		// in ms
		DWORD			dwStartTime;
		DWORD			dwTimeLength;

		S_SEQUENCE*		pPrevSequence;			// Holds the pointer to the previous sequence of the same type.
		S_SEQUENCE*		pNextSequence;			// Holds the pointer to the next sequence of the same type.

		S_SEQUENCE( void )  : pAniObjectInfoToPlay( NULL ), iType( SEQUENCE_COUNT ),
							  dwStartTime( 0 ), dwTimeLength( 0 ), pPrevSequence( NULL ), pNextSequence( NULL )
							  {}
		virtual ~S_SEQUENCE( void )
		{}
	};

	struct S_ACTION_SEQUENCE : public S_SEQUENCE
	{
		ActionInfo*		pActionInfo;
		
		S_ACTION_SEQUENCE( void ) : pActionInfo( NULL )
		{
			iType = ACTION_SEQUENCE;
		}

		S_ACTION_SEQUENCE( const S_SEQUENCE* pObject ) : pActionInfo( NULL )
		{
			const S_ACTION_SEQUENCE* pRhs = static_cast<const S_ACTION_SEQUENCE*>(pObject);
			*this = *(pRhs);
			pActionInfo = new ActionInfo(*(pRhs->pActionInfo));
		}

		~S_ACTION_SEQUENCE( void )
		{
			SAFE_DELETE( pActionInfo );
		}
	};


	struct S_KEY_SEQUENCE : public S_SEQUENCE
	{
		const KeyInfo*			pKeyInfo;

		S_KEY_SEQUENCE( void ) : pKeyInfo( NULL ) 
		{
			iType = KEY_SEQUENCE;
		};

		S_KEY_SEQUENCE( const S_SEQUENCE* pObject ) : pKeyInfo( NULL )
		{
			const S_KEY_SEQUENCE* pRhs = static_cast<const S_KEY_SEQUENCE*>(pObject);
			*this = *(pRhs);
			pKeyInfo = new KeyInfo( *(pRhs->pKeyInfo) );
		}

		~S_KEY_SEQUENCE( void )
		{
			SAFE_DELETE( pKeyInfo );
		}
	};


	// Camera event sequence
	struct S_CAM_EVENT_SEQUENCE : public S_SEQUENCE
	{
		CamEventInfo* pCamEventInfo;
		float fFOVKeyFrame;
		float fPosKeyFrame;
		float fLookAtKeyFrame;
		float fRotKeyFrame;

		S_CAM_EVENT_SEQUENCE( void ) : pCamEventInfo( NULL ), fPosKeyFrame( 0.0f ), fLookAtKeyFrame( 0.0f ), fRotKeyFrame( 0.0f ), fFOVKeyFrame( 0.0f )
		{
			iType = CAM_EVENT_SEQUENCE;
		};

		S_CAM_EVENT_SEQUENCE( const S_SEQUENCE* pObject ) : pCamEventInfo( NULL ), fPosKeyFrame( 0.0f ), fLookAtKeyFrame( 0.0f ), fRotKeyFrame( 0.0f )
		{
			const S_CAM_EVENT_SEQUENCE* pRhs = static_cast<const S_CAM_EVENT_SEQUENCE*>(pObject);
			*this = *(pRhs);
			pCamEventInfo = static_cast<CamEventInfo*>(pRhs->pCamEventInfo->clone());
		}

		~S_CAM_EVENT_SEQUENCE( void ) 
		{ 
			SAFE_DELETE( pCamEventInfo );
		}
	};


	// Particle event sequence
	struct S_PARTICLE_EVENT_SEQUENCE : public S_SEQUENCE
	{
		ParticleEventInfo* pParticleEventInfo;
		EtBillboardEffectHandle hParticle;
		MatrixEx crossWorld;
		
		S_PARTICLE_EVENT_SEQUENCE( void ) : pParticleEventInfo( NULL ) 
		{
			iType = PARTICLE_EVENT_SEQUENCE;
		}

		~S_PARTICLE_EVENT_SEQUENCE( void ) 
		{ 
			SAFE_DELETE( pParticleEventInfo );
			SAFE_RELEASE_SPTR( hParticle );
		};
	};


	// Sound event sequence
	struct S_SOUND_EVENT_SEQUENCE : public S_SEQUENCE
	{
		SoundEventInfo* pSoundEventInfo;
		EtSoundChannelHandle hSound;
	
		S_SOUND_EVENT_SEQUENCE( void ) : pSoundEventInfo( NULL )
		{
			iType = SOUND_EVENT_1_SEQUENCE;		// Note: Default is 1, and it must be set properly after creation
		}

		~S_SOUND_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pSoundEventInfo );
			//SAFE_RELEASE_SPTR( hSound );
		}
	};

	// DOF event sequence
	struct S_DOF_EVENT_SEQUENCE : public S_SEQUENCE
	{
		DofEventInfo* pDofEventInfo;

		S_DOF_EVENT_SEQUENCE( void ) : pDofEventInfo( NULL )
		{
			iType = DOF_EVENT_SEQUENCE;
		}

		~S_DOF_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pDofEventInfo );
		}
	};

	// Fade in/out event sequence
	struct S_FADE_EVENT_SEQUENCE : public S_SEQUENCE
	{
		FadeEventInfo* pFadeEventInfo;

		S_FADE_EVENT_SEQUENCE( void ) : pFadeEventInfo( NULL )
		{
			iType = FADE_EVENT_SEQUENCE;
		}

		~S_FADE_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pFadeEventInfo );
		}
	};

	// Prop action event sequence
	struct S_PROP_EVENT_SEQUENCE : public S_SEQUENCE
	{
		PropEventInfo* pPropEventInfo;
		
		S_PROP_EVENT_SEQUENCE( void ) : pPropEventInfo( NULL )
		{
			iType = PROP_EVENT_SEQUENCE;
		}

		~S_PROP_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pPropEventInfo );
		}
	};

	// Subtitle event sequence
	struct S_SUBTITLE_EVENT_SEQUENCE : public S_SEQUENCE
	{
		SubtitleEventInfo* pSubtitleEventInfo;

		S_SUBTITLE_EVENT_SEQUENCE( void ) : pSubtitleEventInfo( NULL )
		{
			iType = SUBTITLE_EVENT_SEQUENCE;
		}

		~S_SUBTITLE_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pSubtitleEventInfo );
		}
	};

// #ifdef PRE_ADD_FILTEREVENT
	// Color filter event sequence
	struct S_COLORFILTER_EVENT_SEQUENCE : public S_SEQUENCE
	{
		ColorFilterEventInfo* pEventInfo;

		S_COLORFILTER_EVENT_SEQUENCE( void ) : pEventInfo( NULL )
		{
			iType = COLORFILTER_EVENT_SEQUENCE;
		}

		~S_COLORFILTER_EVENT_SEQUENCE( void )
		{
			SAFE_DELETE( pEventInfo );
		}
	};
// #endif PRE_ADD_FILTEREVENT

private:
	IDnCutSceneDataReader*			m_pCutSceneData;

	// Current stage world object
	CEtWorld*						m_pWorld;

	EtCameraHandle					m_hCamera;
	CEtDOFFilter*					m_pDOFFilter;
	S_SUBTITLE_EVENT_SEQUENCE*		m_pLiveSubtitleSequence;

	// Particles currently visible on screen
	vector<EtBillboardEffectHandle>		m_vlhParticles;

	// Custom actor processor set externally
	ICustomActorProcessor*			m_pActorProcessor;

	vector<S_SEQUENCE*>				m_vlpSequenceDB;			// Must be sorted in chronological order
	
	deque<S_SEQUENCE*>				m_dLiveSequences;

	LOCAL_TIME						m_StartTime;
	LOCAL_TIME						m_PrevTime;

	int								m_iNowLiveSequence;

	S_ACTION_SEQUENCE*				m_pLastProcessedActionSeq;
	
	LOCAL_TIME						m_WholeSceneLength;		// Total scene length.
	bool							m_bSceneEnd;
	
	EtMatrix						m_matOriProj;

	set<S_SEQUENCE*>				m_setProcessedTrigger;

	set<string>						m_setHideActor;

	// Keep each actor's initial action and reset to frame 0 when the cutscene starts.
	// The vector index matches each actor's index.
	vector<S_ACTION_SEQUENCE*>		m_vlpFirstActionSequence;


	//struct S_SOUND_CHANNEL_INFO
	//{
	//	EtSoundChannelHandle hChannel;
	//	LOCAL_TIME StartTime;
	//};
	vector<EtSoundChannelHandle>	m_vlhPlayingChannel;

private:
	int _SearchFOVOffsetIndex( vector<SCameraFOVKey>& vlCamFOV, int iFrame );
	int _SearchPosOffsetIndex( vector<SCameraPositionKey>& vlCamPosition, int iFrame );
	int _SearchRotOffsetIndex( vector<SCameraRotationKey>& vlCamRotation, int iFrame );
	//int _SearchLaySequence( LOCAL_TIME ElapsedTime, int iType );
	void _CalcRotateFront( S_CS_ACTOR_INFO* pActorInfo, EtVector3& vDir );

	void _BeginParticleEvent( S_SEQUENCE* pParticleEvent );
	void _BeginSoundEvent( S_SEQUENCE* pSoundEvent );
	void _BeginFadeEvent( S_SEQUENCE* pFadeEvent );
	void _ProcessPropActionEvent( S_SEQUENCE* pPropActionEvent, LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime );
	void _BeginSubtitleEvent( S_SEQUENCE* pEvent );

	//void _FigureOutEndSeuqnces( LOCAL_TIME ElapsedTime );
	//void _FigureOutLiveSequences( LOCAL_TIME ElapsedTime );
	void _UpdateSequence( LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime );
	//void _ProcessSequences( LOCAL_TIME ElapsedTime );
	void _ProcessActors( LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime );
	void _ProcessSubtitle( LOCAL_TIME ElapsedTime );


public:
	CDnCutScenePlayer(void);
	~CDnCutScenePlayer(void);

	void Initialize( IDnCutSceneDataReader* pCutSceneData );
	void Clear( void );

	void MakeUpSequences( ICustomActorProcessor* pCustomActorProcessor );
	void StartPlay( LOCAL_TIME StartTime );
	void StopPlay();
	//void AddPlaySequence( S_SEQUENCE* pNewSequence );
	void SetWorld( CEtWorld* pWorld ) { m_pWorld = pWorld; };
	void SetCamera( EtCameraHandle hCamera ) { m_hCamera = hCamera; };
	EtCameraHandle GetCamera( void ) { return m_hCamera; };
	void SetDOFFilter( CEtDOFFilter* pFilter ) { m_pDOFFilter = pFilter; };
	void ResetLiveSequences( void );

	//const MatrixEx& GetActorEtMatrixEx( const char* pActorName );
	
	void Process( LOCAL_TIME AbsoluteTime );

	void ResetPlayingSoundChannels( void );
	void FadeInPlayingSoundChannels( void );
	void FadeOutPlayingSoundChannels( void );

	bool IsEndScene( void ) { return (NULL == m_pActorProcessor) || (m_bSceneEnd); };

	// Actors that exist in the cutscene file from initial setup but should not appear
	void HideActor( const char* pActorName ) 
	{ 
		m_setHideActor.insert( string(pActorName) );
	};
	bool IsHideActor( const char* pActorName ) { return (m_setHideActor.find( string(pActorName) ) != m_setHideActor.end()); }
};
