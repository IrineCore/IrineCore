#include "StdAfx.h"
#include "DnCutScenePlayer.h"
#include "EtWorld.h"
#include "EtWorldProp.h"
#include "EtWorldSector.h"
#include "EtSoundEngine.h"
#include "EtUIXML.h"
#include "EtAniKey.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCutScenePlayer::CDnCutScenePlayer(void) : m_pCutSceneData( NULL ),
											 m_iNowLiveSequence( 0 ),
											 m_StartTime( 0 ),
											 m_pWorld( NULL ),
											 m_pLastProcessedActionSeq( NULL ),		// Store the last played action for continuous animation processing
											 m_pActorProcessor( NULL ),
											 m_WholeSceneLength( 0 ),
											 m_bSceneEnd( false ),
											 m_pDOFFilter( NULL ),
											 m_pLiveSubtitleSequence( NULL ),
											 m_PrevTime(0)
{

}

CDnCutScenePlayer::~CDnCutScenePlayer(void)
{
	for_each( m_vlpSequenceDB.begin(), m_vlpSequenceDB.end(), DeleteData<S_SEQUENCE*>() );
}

void CDnCutScenePlayer::Initialize( IDnCutSceneDataReader* pCutSceneData )
{
	m_pCutSceneData = pCutSceneData;
}

void CDnCutScenePlayer::Clear( void )
{
	m_vlhParticles.clear();
	for_each( m_vlpSequenceDB.begin(), m_vlpSequenceDB.end(), DeleteData<S_SEQUENCE*>() );
	//for_each( m_vlpActors.begin(), m_vlpActors.end(), DeleteData<S_CS_ACTOR_INFO*>() );
	//m_vlpActors.clear();
	//m_mapActors.clear();
	m_pLastProcessedActionSeq = NULL;
	m_vlpSequenceDB.clear();
	m_dLiveSequences.clear();
	m_iNowLiveSequence = 0;
	m_pLiveSubtitleSequence = NULL;
	m_vlpFirstActionSequence.clear();
}

void CDnCutScenePlayer::MakeUpSequences( ICustomActorProcessor* pCustomActorProcessor )
{
	S_SEQUENCE* apPrevSequence[ SEQUENCE_COUNT ];
	ZeroMemory( &apPrevSequence, sizeof(apPrevSequence) );
	
	m_setProcessedTrigger.clear();

	m_pActorProcessor = pCustomActorProcessor;

	int iNumActor = m_pActorProcessor->GetNumActors();//(int)m_vlpActors.size();
	
	for( int iActor = 0; iActor < iNumActor; ++iActor )
	{
		S_CS_ACTOR_INFO* pActorObject = m_pActorProcessor->GetActorInfoByIndex( iActor );//m_vlpActors.at( iActor );
		if( NULL == pActorObject->pActor )		// Members not in party during quest cutscene are set to NULL and excluded from cutscene playback.
			continue;

		// Set lightmap influence flags as configured.
		if( pActorObject->hObject )
			pActorObject->hObject->EnableLightMapInfluence( m_pCutSceneData->GetThisActorsInfluenceLightmap( pActorObject->strActorName.c_str() ) );

		ZeroMemory( &apPrevSequence, sizeof(apPrevSequence) );

		int iNumAction = m_pCutSceneData->GetThisActorsActionNum( pActorObject->strActorName.c_str() );
		for( int iAction = 0; iAction < iNumAction; ++iAction )
		{
			// Receive key info and data info separately.
			const ActionInfo* pActionInfo = m_pCutSceneData->GetThisActorsActionInfoByIndex( pActorObject->strActorName.c_str(), iAction );

			S_ACTION_SEQUENCE* pNewSequence = new CDnCutScenePlayer::S_ACTION_SEQUENCE;
			pNewSequence->pActionInfo = new ActionInfo( *pActionInfo );
			pNewSequence->dwStartTime = DWORD(pActionInfo->fStartTime * 1000.0f );			// Since it is in seconds..
			pNewSequence->dwTimeLength = DWORD(pActionInfo->fTimeLength * 1000.0f );

			pNewSequence->pAniObjectInfoToPlay = m_pActorProcessor->GetActorInfoByName( pActionInfo->strActorName.c_str() );//m_mapActors[ pActionInfo->strActorName ];
			//assert( pNewSequence->pAniObjectInfoToPlay );

			if( pNewSequence->pAniObjectInfoToPlay )
			{
				pNewSequence->pAniObjectInfoToPlay->vDefaultPos = m_pCutSceneData->GetRegResPos( pActionInfo->strActorName.c_str() );
				pNewSequence->pAniObjectInfoToPlay->fDefaultYRotation = m_pCutSceneData->GetRegResRot( pActionInfo->strActorName.c_str() );
				pNewSequence->pPrevSequence = apPrevSequence[ ACTION_SEQUENCE ];

				// If the immediately preceding action is continue, extend its duration up to this action's start time.
				if( pNewSequence->pPrevSequence )
				{
					pNewSequence->pPrevSequence->pNextSequence = pNewSequence;

					if( ActionInfo::AT_CONTINUE == static_cast<S_ACTION_SEQUENCE*>(pNewSequence->pPrevSequence)->pActionInfo->iActionType )
						pNewSequence->pPrevSequence->dwTimeLength = pNewSequence->dwStartTime - pNewSequence->pPrevSequence->dwStartTime;
				}

				apPrevSequence[ ACTION_SEQUENCE ] = pNewSequence;
				m_vlpSequenceDB.push_back( pNewSequence );

				// Store the first action separately.
				if( 0 == iAction )
					m_vlpFirstActionSequence.push_back( pNewSequence );
			}
			else
				delete pNewSequence;
		}

		// An actor without actions has no first action either, so push NULL to the vector.
		if( 0 == iNumAction )
			m_vlpFirstActionSequence.push_back( NULL );

		int iNumKey = m_pCutSceneData->GetThisActorsKeyNum( pActorObject->strActorName.c_str() );
		for( int iKey = 0; iKey < iNumKey; ++iKey )
		{
			const KeyInfo* pKeyInfo = m_pCutSceneData->GetThisActorsKeyInfoByIndex( pActorObject->strActorName.c_str(), iKey );

			S_KEY_SEQUENCE* pNewSequence = new CDnCutScenePlayer::S_KEY_SEQUENCE;
			pNewSequence->pKeyInfo = new KeyInfo( *pKeyInfo );
			pNewSequence->dwStartTime = DWORD(pKeyInfo->fStartTime * 1000.0f );
			pNewSequence->dwTimeLength = DWORD(pKeyInfo->fTimeLength * 1000.0f );

			
			pNewSequence->pAniObjectInfoToPlay = m_pActorProcessor->GetActorInfoByName( pKeyInfo->strActorName.c_str() );//m_mapActors[ pKeyInfo->strActorName ];
			//assert( pNewSequence->pAniObjectInfoToPlay );

			if( pNewSequence->pAniObjectInfoToPlay )
			{
				pNewSequence->pPrevSequence = apPrevSequence[ KEY_SEQUENCE ];
				if( apPrevSequence[ KEY_SEQUENCE ] )
					pNewSequence->pPrevSequence->pNextSequence = pNewSequence;

				apPrevSequence[ KEY_SEQUENCE ] = pNewSequence;
				m_vlpSequenceDB.push_back( pNewSequence );
			}
			else
				delete pNewSequence;
		}
	}

	// Added event-related data. Camera unit is ms.
	for( int iEventType = 0; iEventType < EventInfo::TYPE_COUNT; ++iEventType )
	{
		int iNumEvent = m_pCutSceneData->GetThisTypesEventNum( iEventType );

		for( int iEvent = 0; iEvent < iNumEvent; ++iEvent )
		{
			const EventInfo* pNowEventInfo = m_pCutSceneData->GetEventInfoByIndex( iEventType, iEvent );

			S_SEQUENCE* pNewSequence = NULL;
			switch( pNowEventInfo->iType )
			{
				case EventInfo::CAMERA:
					{
						S_CAM_EVENT_SEQUENCE* pCamSequence = new S_CAM_EVENT_SEQUENCE;
						pCamSequence->pCamEventInfo = new CamEventInfo;
						pCamSequence->pCamEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pCamSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ CAM_EVENT_SEQUENCE ];
						if( apPrevSequence[ CAM_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;

						apPrevSequence[ CAM_EVENT_SEQUENCE ] = pNewSequence;

						// If this is a camera tracking a specific actor, pass down the pointer to that actor.
						//if( CamEventInfo::TRACE_ACTOR == pCamSequence->pCamEventInfo->iTraceType /*||
						//	CamEventInfo::TRACE_ACTOR_ONLY_LOOKAT == pNewSequence->pCamEventInfo->iTraceType*/ )
						//{
						//	if( false == pCamSequence->pCamEventInfo->strActorToTrace.empty() )
						//	{
						//		pCamSequence->pAniObjectInfoToPlay = m_pActorProcessor->GetActorInfoByName( pCamSequence->pCamEventInfo->strActorToTrace.c_str() );
						//		assert( pCamSequence->pAniObjectInfoToPlay );
						//	}
						//}
						//else
							pCamSequence->pAniObjectInfoToPlay = NULL;
					}
					break;

				case EventInfo::PARTICLE:
					{
						S_PARTICLE_EVENT_SEQUENCE* pParticleSequence = new S_PARTICLE_EVENT_SEQUENCE;
						pParticleSequence->pParticleEventInfo = new ParticleEventInfo;
						pParticleSequence->pParticleEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pParticleSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ PARTICLE_EVENT_SEQUENCE ];
						if( apPrevSequence[ PARTICLE_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ PARTICLE_EVENT_SEQUENCE ] = pNewSequence;
					}
					break;

				//case EventInfo::SOUND:
				//	{
				//		S_SOUND_EVENT_SEQUENCE* pSoundSequence = new S_SOUND_EVENT_SEQUENCE;
				//		pSoundSequence->pSoundEventInfo = new SoundEventInfo;
				//		pSoundSequence->pSoundEventInfo->copy_from( pNowEventInfo );
				//		pNewSequence = pSoundSequence;

				//		pNewSequence->pPrevSequence = apPrevSequence[ SOUND_EVENT_SEQUENCE ];
				//		if( apPrevSequence[ SOUND_EVENT_SEQUENCE ] )
				//			pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
				//		apPrevSequence [ SOUND_EVENT_SEQUENCE ] = pNewSequence;
				//	}
				//	break;

				case EventInfo::DOF:
					{
						S_DOF_EVENT_SEQUENCE* pDofEventSequence = new S_DOF_EVENT_SEQUENCE;
						pDofEventSequence->pDofEventInfo = new DofEventInfo;
						pDofEventSequence->pDofEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pDofEventSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ DOF_EVENT_SEQUENCE ];
						if( apPrevSequence[ DOF_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ DOF_EVENT_SEQUENCE ] = pNewSequence;

						//S_DOF_EVENT_SEQUENCE* pPrevDofEventSequence = static_cast<S_DOF_EVENT_SEQUENCE*>(pDofEventSequence->pPrevSequence);
						//DofEventInfo* pDofEventInfo = pDofEventSequence->pDofEventInfo;
						//if( pPrevDofEventSequence && pDofEventInfo->bUseNowValueAsStart )
						//{
						//	DofEventInfo* pPrevDofEventInfo = pPrevDofEventSequence->pDofEventInfo;
						//	pDofEventInfo->fNearStartFrom = pPrevDofEventInfo->fNearStartDest;
						//	pDofEventInfo->fNearEndFrom = pPrevDofEventInfo->fNearEndDest;
						//	pDofEventInfo->fFarStartFrom = pPrevDofEventInfo->fFarStartDest;
						//	pDofEventInfo->fFarEndFrom = pPrevDofEventInfo->fFarEndDest;
						//	pDofEventInfo->fFocusDistFrom = pPrevDofEventInfo->fFocusDistDest;
						//	pDofEventInfo->fNearBlurSizeFrom = pPrevDofEventInfo->fNearBlurSizeDest;
						//	pDofEventInfo->fFarBlurSizeFrom = pPrevDofEventInfo->fFarBlurSizeDest;
						//}
					}
					break;

				case EventInfo::FADE:
					{
						S_FADE_EVENT_SEQUENCE* pFadeEventSequence = new S_FADE_EVENT_SEQUENCE;
						pFadeEventSequence->pFadeEventInfo = new FadeEventInfo;
						pFadeEventSequence->pFadeEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pFadeEventSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ FADE_EVENT_SEQUENCE ];
						if( apPrevSequence[ FADE_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ FADE_EVENT_SEQUENCE ] = pNewSequence;
					}
					break;

				case EventInfo::PROP:
					{
						// TODO: Need to make the action prop perform a specific action. This is related to the client too.
						S_PROP_EVENT_SEQUENCE* pPropActionSequence = new S_PROP_EVENT_SEQUENCE;
						pPropActionSequence->pPropEventInfo = new PropEventInfo;
						pPropActionSequence->pPropEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pPropActionSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ PROP_EVENT_SEQUENCE ];
						if( apPrevSequence[ PROP_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ PROP_EVENT_SEQUENCE ] = pNewSequence;
					}
					break;

				case EventInfo::SOUND_1:
				case EventInfo::SOUND_2:
				case EventInfo::SOUND_3:
				case EventInfo::SOUND_4:
				case EventInfo::SOUND_5:
				case EventInfo::SOUND_6:
				case EventInfo::SOUND_7:
				case EventInfo::SOUND_8:
					{
						int iSequenceIndex = SOUND_EVENT_1_SEQUENCE + (pNowEventInfo->iType-EventInfo::SOUND_1);

						S_SOUND_EVENT_SEQUENCE* pSoundSequence = new S_SOUND_EVENT_SEQUENCE;
						pSoundSequence->pSoundEventInfo = new SoundEventInfo;
						pSoundSequence->iType = iSequenceIndex;
						pSoundSequence->pSoundEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pSoundSequence;
						
						pNewSequence->pPrevSequence = apPrevSequence[ iSequenceIndex ];
						if( apPrevSequence[ iSequenceIndex ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence [ iSequenceIndex ] = pNewSequence;
					}
					break;

				case EventInfo::SUBTITLE:
					{
						S_SUBTITLE_EVENT_SEQUENCE* pSubtitleSequence = new S_SUBTITLE_EVENT_SEQUENCE;
						pSubtitleSequence->pSubtitleEventInfo = new SubtitleEventInfo;
						pSubtitleSequence->pSubtitleEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pSubtitleSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ SUBTITLE_EVENT_SEQUENCE ];
						if( apPrevSequence[ SUBTITLE_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ SUBTITLE_EVENT_SEQUENCE ] = pNewSequence;
					}
					break;

//#ifdef PRE_ADD_FILTEREVENT
				case EventInfo::COLORFILTER:
					{
						S_COLORFILTER_EVENT_SEQUENCE* pColorFilterSequence = new S_COLORFILTER_EVENT_SEQUENCE;
						pColorFilterSequence->pEventInfo = new ColorFilterEventInfo;
						pColorFilterSequence->pEventInfo->copy_from( pNowEventInfo );
						pNewSequence = pColorFilterSequence;

						pNewSequence->pPrevSequence = apPrevSequence[ COLORFILTER_EVENT_SEQUENCE ];
						if( apPrevSequence[ COLORFILTER_EVENT_SEQUENCE ] )
							pNewSequence->pPrevSequence->pNextSequence = pNewSequence;
						apPrevSequence[ COLORFILTER_EVENT_SEQUENCE ] = pNewSequence;
					}
					break;
//#endif PRE_ADD_FILTEREVENT

			}

			pNewSequence->dwStartTime = DWORD(pNowEventInfo->fStartTime * 1000.0f);
			pNewSequence->dwTimeLength = DWORD(pNowEventInfo->fTimeLength * 1000.0f);

			m_vlpSequenceDB.push_back( pNewSequence );
		}
	}

	set<string>::iterator iter = m_setHideActor.begin();
	for( ; iter != m_setHideActor.end(); ++iter )
	{
		m_pActorProcessor->GetActorInfoByName( iter->c_str() )->bHided = true;
	}
}

void CDnCutScenePlayer::StartPlay( LOCAL_TIME StartTime )
{
	assert( m_pWorld && "World object is not set." );

	m_StartTime = StartTime;
	m_PrevTime = StartTime;

	m_WholeSceneLength = 0;
	m_bSceneEnd = false;

	// Sequence DB must be sorted in chronological order.
	struct SortByStartTime : public binary_function<const S_SEQUENCE*, const S_SEQUENCE*, bool>
	{
		bool operator () ( const S_SEQUENCE* pA, const S_SEQUENCE* pB )
		{
			return pA->dwStartTime < pB->dwStartTime;
		}
	};

	// Extract the longest duration.
	int iNumSequence = (int)m_vlpSequenceDB.size();
	for( int iSequence = 0; iSequence < iNumSequence; ++iSequence )
	{
		const S_SEQUENCE* pSequence = m_vlpSequenceDB.at( iSequence );
		DWORD dwThisSeqEndTime = pSequence->dwStartTime+pSequence->dwTimeLength;
		if( dwThisSeqEndTime > m_WholeSceneLength )
			m_WholeSceneLength = dwThisSeqEndTime;
	}

	sort( m_vlpSequenceDB.begin(), m_vlpSequenceDB.end(), SortByStartTime() );

	// Initialize initial actions to frame 0.
	for( int iActor = 0; iActor < m_pActorProcessor->GetNumActors(); ++iActor )
	{
		S_ACTION_SEQUENCE* pActionSequence = m_vlpFirstActionSequence.at( iActor );
		if( NULL == pActionSequence )
			continue;

		S_CS_ACTOR_INFO* pActorInfo = m_pActorProcessor->GetActorInfoByIndex( iActor );
		if( false == pActionSequence->pActionInfo->strAnimationName.empty() )
		{
			//int iAniIndex = m_pActorProcessor->GetActionIndex( iActor, pActionSequence->pActionInfo->strAnimationName.c_str() );
			//if( -1 != iAniIndex )
			//{
			//	pActorInfo->hObject->SetAniFrame( pActorInfo->vlAniIndex[ iAniIndex ], 0.0f );
			//}

			if( pActorInfo->hObject && pActionSequence->pActionInfo->iAnimationIndex < (int)pActorInfo->vlAniIndex.size() )
			{
				pActorInfo->hObject->SetAniFrame( pActorInfo->vlAniIndex[ pActionSequence->pActionInfo->iAnimationIndex ], 0.0f );
			}
		}
	}

	// Back up the projection matrix, as FOV may change during scene playback.
	m_matOriProj = *(m_hCamera->GetProjMat());
}

void CDnCutScenePlayer::ResetLiveSequences( void )
{
	m_dLiveSequences.clear();
	m_iNowLiveSequence = 0;
}

void CDnCutScenePlayer::_CalcRotateFront( S_CS_ACTOR_INFO* pActorInfo, EtVector3& vDir )
{
	// Default facing direction of the character
	EtVector3 vCharDefaultDir( 0.0f, 0.0f, 1.0f );
	EtVec3Normalize( &vDir, &vDir );

	if( fabs(vDir.x-vCharDefaultDir.x) < 0.0001f )
		vDir.x = vCharDefaultDir.x;

	if( fabs(vDir.z-vCharDefaultDir.z) < 0.0001f )
		vDir.z = vCharDefaultDir.z;

	float fDot = EtVec3Dot( &vDir, &vCharDefaultDir );
	float fRot = EtAcos( fDot );

	if( vDir.x > 0.0f )
		fRot = -fRot;

	float fDegree = fRot * (180.0f/ET_PI);
	pActorInfo->fRotYDegree = fDegree;
	//EtMatrixRotationY( &pActorInfo->matRotY, fRot );
}

void CDnCutScenePlayer::_BeginParticleEvent( S_SEQUENCE* pParticleEvent )
{
	// Create particle
	S_PARTICLE_EVENT_SEQUENCE* pParticleSequence = static_cast<S_PARTICLE_EVENT_SEQUENCE*>(pParticleEvent);

	assert( -1 != pParticleSequence->pParticleEventInfo->iParticleDataIndex && "No particle data!" );
	if( -1 != pParticleSequence->pParticleEventInfo->iParticleDataIndex )
	{
		pParticleSequence->crossWorld.SetPosition( pParticleSequence->pParticleEventInfo->vPos );
		pParticleSequence->hParticle = EternityEngine::CreateBillboardEffect( pParticleSequence->pParticleEventInfo->iParticleDataIndex, 
			pParticleSequence->crossWorld );
		pParticleSequence->hParticle->SetCullDist( 1000000.0f );
		//pParticleSequence->hParticle->SetWorldMat(  );
		//pParticleSequence->hParticle->EnableIterate(  );
		pParticleSequence->hParticle->Show( true );

		m_vlhParticles.push_back( pParticleSequence->hParticle );
	}
}

void CDnCutScenePlayer::_BeginSoundEvent( S_SEQUENCE* pSoundEvent )
{
	S_SOUND_EVENT_SEQUENCE* pSoundSequence = static_cast<S_SOUND_EVENT_SEQUENCE*>(pSoundEvent);

	assert( -1 != pSoundSequence->pSoundEventInfo->iSoundDataIndex && "No sound data!" );
	if( -1 != pSoundSequence->pSoundEventInfo->iSoundDataIndex )
	{
		pSoundSequence->hSound = CEtSoundEngine::GetInstance().PlaySound( "NULL", pSoundSequence->pSoundEventInfo->iSoundDataIndex, false, true );
		//m_hCurChannel->SetVolume( pSound->fVolume );
		//m_hCurChannel->SetRollOff( 3, 0.0f, 1.0f, pSound->fRange * pSound->fRollOff, 1.0f, pSound->fRange, 0.0f );
		//m_hCurChannel->SetPosition( Cross.m_vPosition );
		if( !pSoundSequence->hSound )
			return;

		pSoundSequence->hSound->SetPriority(0);
		pSoundSequence->hSound->SetVolume( pSoundSequence->pSoundEventInfo->fVolume );
		pSoundSequence->hSound->Resume();

		m_vlhPlayingChannel.push_back( pSoundSequence->hSound );
	}
}

void CDnCutScenePlayer::_BeginFadeEvent( S_SEQUENCE* pFadeEvent )
{
	S_FADE_EVENT_SEQUENCE* pFadeSequence = static_cast<S_FADE_EVENT_SEQUENCE*>(pFadeEvent);

	assert( -1 != pFadeSequence->pFadeEventInfo->iFadeKind && "Invalid fade-in/out information!" );
	if( -1 != pFadeSequence->pFadeEventInfo->iFadeKind )
	{
		DWORD dwColor = D3DCOLOR_ARGB( 0, pFadeSequence->pFadeEventInfo->iColorRed, 
									      pFadeSequence->pFadeEventInfo->iColorGreen, pFadeSequence->pFadeEventInfo->iColorBlue );

		switch( pFadeSequence->pFadeEventInfo->iFadeKind )
		{
			case FadeEventInfo::FADE_IN:
				m_pActorProcessor->FadeIn( 1.0f / pFadeSequence->pFadeEventInfo->fTimeLength, dwColor );
				//CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.0f, 0.3f, false );
				break;

			case FadeEventInfo::FADE_OUT:
				m_pActorProcessor->FadeOut( 1.0f / pFadeSequence->pFadeEventInfo->fTimeLength, dwColor );
				//CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.0f, 0.3f, false );
				break;
		}

	}
}


void CDnCutScenePlayer::_BeginSubtitleEvent( S_SEQUENCE* pEvent )
{
	S_SUBTITLE_EVENT_SEQUENCE* pSubtitleEvent = static_cast<S_SUBTITLE_EVENT_SEQUENCE*>(pEvent);
	const wchar_t* pSubtitle = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSubtitleEvent->pSubtitleEventInfo->iUIStringID );
	m_pActorProcessor->ShowSubtitle( pSubtitle );
	m_pLiveSubtitleSequence = pSubtitleEvent;
}

// PRE_ADD_FILTEREVENT
//void CDnCutScenePlayer::_BeginColorFilterEvent( S_SEQUENCE* pEvent )
//{
//	S_COLORFILTER_EVENT_SEQUENCE* pColorFilterEvent = static_cast<S_COLORFILTER_EVENT_SEQUENCE*>(pEvent);
//	//const wchar_t* pSubtitle = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pColorFilterEvent->pEventInfo->iUIStringID );
//	//m_pActorProcessor->ShowSubtitle( pSubtitle );
//	m_pLiveSubtitleSequence = pColorFilterEvent;
//}


void CDnCutScenePlayer::_ProcessPropActionEvent( S_SEQUENCE* pPropActionEvent, LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime )
{
	S_PROP_EVENT_SEQUENCE* pPropSequence = static_cast<S_PROP_EVENT_SEQUENCE*>(pPropActionEvent);

	// Calculate and set the current frame..
	LOCAL_TIME LiveElapsedTime = ElapsedTime - pPropSequence->dwStartTime;
	float fLiveElapsedTime = (float)LiveElapsedTime * 0.001f;

	float fFrame = fLiveElapsedTime * pPropSequence->pPropEventInfo->fSpeed;

	CEtWorldSector* pSector = m_pWorld->GetSector( 0.0f, 0.0f );
	CEtWorldProp* pProp = pSector->GetPropFromCreateUniqueID( pPropSequence->pPropEventInfo->iPropID );

	if( pProp ) 
	{
		if (false == pPropSequence->pPropEventInfo->strActionName.empty()) 
		{
			m_pActorProcessor->SetPropAction( pProp, pPropSequence->pPropEventInfo->strActionName.c_str(), AbsoluteTime, fFrame );
		}

		m_pActorProcessor->ShowProp( pProp, pPropSequence->pPropEventInfo->bShow );
		//pProp->GetObjectHandle()->ShowObject( pPropSequence->pPropEventInfo->bShow );
	}
}

int CDnCutScenePlayer::_SearchFOVOffsetIndex( vector<SCameraFOVKey>& vlCamFOV, int iFrame )
{
	int iNumFOV = (int)vlCamFOV.size();

	if( 0 == iFrame || 1 == iNumFOV )
		return 0;

	int iResult = 0;
	int iStartIndex = 0;
	int iEndIndex = iNumFOV-1;
	int iCenterIndex = 0;
	do 
	{
		iCenterIndex = iStartIndex + (iEndIndex-iStartIndex) / 2;
		if( iFrame < vlCamFOV.at( iCenterIndex ).nTime )		// Left section
		{
			//iResult = 
			iEndIndex = iCenterIndex;
		}
		else														// Right section
		{
			//iResult = 
			iStartIndex = iCenterIndex;
		}

		if( iCenterIndex == iNumFOV-2 )
			break;

	} while ( !(vlCamFOV.at(iCenterIndex).nTime <= iFrame &&
		iFrame < vlCamFOV.at(iCenterIndex+1).nTime) );

	iResult = iCenterIndex;

	return iResult;
}

int CDnCutScenePlayer::_SearchRotOffsetIndex( vector<SCameraRotationKey>& vlCamRotation, int iFrame )
{
	int iNumRots = (int)vlCamRotation.size();

	if( 0 == iFrame || 1 == iNumRots )
		return 0;

	int iResult = 0;
	int iStartIndex = 0;
	int iEndIndex = iNumRots-1;
	int iCenterIndex = 0;
	do 
	{
		iCenterIndex = iStartIndex + (iEndIndex-iStartIndex) / 2;
		if( iFrame < vlCamRotation.at( iCenterIndex ).nTime )		// Left section
		{
			//iResult = 
			iEndIndex = iCenterIndex;
		}
		else														// Right section
		{
			//iResult = 
			iStartIndex = iCenterIndex;
		}

		if( iCenterIndex == iNumRots-2 )
			break;

	} while ( !(vlCamRotation.at(iCenterIndex).nTime <= iFrame &&
				iFrame < vlCamRotation.at(iCenterIndex+1).nTime) );

	iResult = iCenterIndex;

	return iResult;
}

int CDnCutScenePlayer::_SearchPosOffsetIndex( vector<SCameraPositionKey>& vlCamPosition, int iFrame )
{
	int iNumPos = (int)vlCamPosition.size();

	if( 0 == iFrame || 1 == iNumPos )
		return 0;

	int iResult = 0;
	int iStartIndex = 0;
	int iEndIndex = iNumPos-1;
	int iCenterIndex = 0;
	do 
	{
		iCenterIndex = iStartIndex + (iEndIndex-iStartIndex) / 2;
		if( iFrame < vlCamPosition.at( iCenterIndex ).nTime )		// Left section
		{
			//iResult = 
			iEndIndex = iCenterIndex;
		}
		else														// Right section
		{
			//iResult = 
			iStartIndex = iCenterIndex;
		}

		if( iCenterIndex == iNumPos-2 )
			break;

	} while ( !(vlCamPosition.at(iCenterIndex).nTime <= iFrame &&
				iFrame < vlCamPosition.at(iCenterIndex+1).nTime) );

	iResult = iCenterIndex;

	return iResult;
}


void CDnCutScenePlayer::_ProcessActors( LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime )
{
	// TODO: Among actors whose action has finished, those with continuous animations must keep updating.
	// We'll have to see whether to make this the default animation setting or not.
	float fDelta = float(AbsoluteTime - m_PrevTime) * 0.001f;
	//int iNumActors = (int)m_vlpActors.size();
	int iNumActors = m_pActorProcessor->GetNumActors();
	for( int iActor = 0; iActor < iNumActors; ++iActor )
	{
		S_CS_ACTOR_INFO* pActorInfo = m_pActorProcessor->GetActorInfoByIndex( iActor );//m_vlpActors.at( iActor );

		//assert( pActorInfo->iNowAniIndex != -1 && "CDnCutScenePlayer::Process() -> Invalid animation index." );

		int iAniIndex = m_pActorProcessor->GetActionIndex( iActor, pActorInfo->strNowActionName.c_str() );
		if( -1 == iAniIndex )
		{
			// Returns -1 if no action exists. If action name is empty, fall back to index 0.
			// Otherwise it hits continue here and this actor's position is not updated.
			if( pActorInfo->strNowActionName.empty() )
				iAniIndex = 0;
			else
				continue;
		}
		if( iAniIndex >= (int)pActorInfo->vlAniIndex.size() || pActorInfo->vlAniIndex[ iAniIndex ] == -1 )
			continue;

		float fAniLength = (float)pActorInfo->hObject->GetLastFrame( pActorInfo->vlAniIndex[ iAniIndex ] );
		//pActorInfo->fFrame += fDelta*pActorInfo->fAnimationSpeed;		

		switch( pActorInfo->iAnimationType )
		{
			case ActionInfo::AT_ONCE:
				{
					if( pActorInfo->fFrame > fAniLength )
					{
						pActorInfo->fFrame = fAniLength;
						pActorInfo->bElapsed = true;
					}
					else
					if( pActorInfo->fFrame < 0.0f )
						pActorInfo->fFrame = 0.0f;
				}
				break;

			case ActionInfo::AT_REPEAT:
				{
					float fOverElapse = pActorInfo->fFrame / fAniLength;
					if( pActorInfo->fFrame > fAniLength )
					{
						//pActorInfo->fFrame = 0.0f;
						pActorInfo->fFrame = pActorInfo->fFrame - int(fOverElapse)*fAniLength;
						pActorInfo->bElapsed = true;
					}
					else
					if( pActorInfo->fFrame < 0.0f )
					{
						pActorInfo->fFrame = pActorInfo->fFrame - int(fOverElapse)*fAniLength;
						pActorInfo->fFrame += fAniLength;
						//pActorInfo->fFrame = fAniLength;
					}
				}
				break;

			case ActionInfo::AT_CONTINUE:
				{
					LOCAL_TIME LiveElapsedTime = ElapsedTime - pActorInfo->dwSequenceStartTime;
					pActorInfo->fSequenceLiveElapsedSec = float(LiveElapsedTime) * 0.001f;
					pActorInfo->fFrame = pActorInfo->fSequenceLiveElapsedSec * pActorInfo->fAnimationSpeed;

					float fOverElapse = pActorInfo->fFrame / fAniLength;
					if( pActorInfo->fFrame > fAniLength )
					{
						pActorInfo->fFrame = pActorInfo->fFrame - int(fOverElapse)*fAniLength;
						pActorInfo->bElapsed = true;
					}
					else
					if( pActorInfo->fFrame < 0.0f )
					{
						pActorInfo->fFrame = pActorInfo->fFrame - int(fOverElapse)*fAniLength;
						pActorInfo->fFrame += fAniLength;
					}
				}
				break;
		}

		// TODO: Handle section loops identically to how they are handled in the action file
		if( NULL != m_pLastProcessedActionSeq )
		{
			if( 0 < m_pLastProcessedActionSeq->pActionInfo->iNextActionFrame )
			{
				if( pActorInfo->bElapsed )
				{
					if( (int)pActorInfo->fFrame < m_pLastProcessedActionSeq->pActionInfo->iNextActionFrame )
						pActorInfo->fFrame += (float)m_pLastProcessedActionSeq->pActionInfo->iNextActionFrame;
				}
			}
		}

		float fFinalRotYDegree = pActorInfo->fRotYDegree + pActorInfo->fNowRotProceed;
		//float fFinalRotYDegree = 180.0f + ( pActorInfo->fNowRotProceed - pActorInfo->fRotYDegree );
		EtMatrixRotationY( &pActorInfo->matRotY, -(fFinalRotYDegree*ET_PI / 180.0f) );

		float fScale = m_pActorProcessor->GetActorScale( iActor );
		if( fScale != 1.0f )
		{
			// Compensate for the difference in bip_01 between scaled and unscaled states, because even when scaled, ani distance moves only by the original scale's Y value.
			//pActorInfo->hObject->GetAniHandle()->GetBone( 0 )->GetAniKey( pActorInfo->vlAniIndex[ iAniIndex ] )->CalcPosition( vDefaultPos, 0.0f );
			pActorInfo->hObject->SetCalcPositionFlag( CALC_POSITION_Y );
		}

		if( pActorInfo->bUseAniDistance )
		{
			//pActorInfo->hObject->CalcAniDistance( pActorInfo->vlAniIndex.at(pActorInfo->iNowAniIndex),
			//									  pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );
			m_pActorProcessor->CalcAniDistance( pActorInfo, pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

			if( fScale != 1.0f )
				pActorInfo->vAniDistanceOffset.y = 0.0f;

			EtVec3TransformCoord( &pActorInfo->vAniDistanceOffset, &pActorInfo->vAniDistanceOffset, &pActorInfo->matRotY );

			if( pActorInfo->bFitAniDistanceYToMap )
			{
				EtVector3 vFinalPos = pActorInfo->vPos + pActorInfo->vAniDistanceOffset;
				pActorInfo->vAniDistanceOffset.y = m_pWorld->GetHeight( vFinalPos.x, vFinalPos.z ) + pActorInfo->vAniDistanceOffset.y - pActorInfo->vPos.y;
			}
			//else
			//	pActorInfo->vAniDistanceOffset.y = 0.0f;
		}
		else
		{
			// If YPosToMap is enabled on Key, calculate and assign only the Y value for accurate height. 
			// Must also account for map height and the y value height of AniDistanct. 
			if( pActorInfo->bKeyUseYPosToMap )
			{
				m_pActorProcessor->CalcAniDistance( pActorInfo, pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

				if( fScale != 1.0f )
					pActorInfo->vAniDistanceOffset.y = 0.0f;

				pActorInfo->vAniDistanceOffset.y = m_pWorld->GetHeight( pActorInfo->vPos.x, pActorInfo->vPos.z ) + pActorInfo->vAniDistanceOffset.y - pActorInfo->vPos.y;
				pActorInfo->vAniDistanceOffset.x = pActorInfo->vAniDistanceOffset.z = 0.0f;
			}
			else
				pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );
		}

		EtMatrix matScale;
		EtMatrixScaling( &matScale, fScale, fScale, fScale );

		pActorInfo->matExFinal.Identity();
		pActorInfo->matExFinal.RotateYaw( fFinalRotYDegree/*pActorInfo->fRotYDegree*/ );
		pActorInfo->matExFinal.SetPosition( pActorInfo->vPos + pActorInfo->vAniDistanceOffset );

		EtMatrix matWorld( pActorInfo->matExFinal );
		matWorld = matScale * matWorld;
		pActorInfo->matExFinal = matWorld;

		set<string>::iterator iter = m_setHideActor.find( pActorInfo->strActorName );
		if( iter != m_setHideActor.end() )
			pActorInfo->hObject->ShowObject( false );
	}

	m_pActorProcessor->Process( ElapsedTime, fDelta );
}

void CDnCutScenePlayer::_UpdateSequence( LOCAL_TIME AbsoluteTime, LOCAL_TIME ElapsedTime )
{
	if( m_vlpSequenceDB.empty() )
		return;

	// Immediately build the screen matching the current time based on absolute time.
	// Actor needs to be calculated separately considering Ani Distance.
	// For actors, update in order: rotation -> key -> action.
	S_SEQUENCE* apProcessingSequence[ SEQUENCE_COUNT ];
	ZeroMemory( apProcessingSequence, sizeof(apProcessingSequence) );

	int iNumActor = m_pActorProcessor->GetNumActors();
	for( int i = 0; i < iNumActor; ++i )
	{
		S_CS_ACTOR_INFO* pActorInfo = m_pActorProcessor->GetActorInfoByIndex( i );
	
		pActorInfo->vPos = pActorInfo->vDefaultPos;
		pActorInfo->fRotYDegree = pActorInfo->fDefaultYRotation;
		pActorInfo->vPrevPos = EtVector3( 0.0f, 0.0f, 0.0f );
		pActorInfo->fNowRotProceed = 0.0f;
	}

	//deque<S_SEQUENCE*> dOpenSequences;

	S_SEQUENCE* pProcessSequence = m_vlpSequenceDB.at( 0 );
	int iSequence = 0;
	//int iCount = 0;
	while( pProcessSequence )
	{
		DWORD dwEndTime = pProcessSequence->dwStartTime + pProcessSequence->dwTimeLength;

		if( pProcessSequence->dwStartTime < ElapsedTime )
		{
			// Actor-controlling sequence related.. apply result to the actor based on whichever sequence finishes now.
			switch( pProcessSequence->iType )
			{
				case ACTION_SEQUENCE:
					{
						S_ACTION_SEQUENCE* pActionSequence = static_cast<S_ACTION_SEQUENCE*>(pProcessSequence);

						S_CS_ACTOR_INFO* pActorInfo = pActionSequence->pAniObjectInfoToPlay;

						//if( 0 == iCount )
						//	OutputDebug( "[Pos] (%2.2f %2.2f, %2.2f)\n", pActorInfo->vPos.x, pActorInfo->vPos.y, pActorInfo->vPos.z );

						//++iCount;

						m_pLastProcessedActionSeq = pActionSequence;

						pActorInfo->dwSequenceStartTime = pActionSequence->dwStartTime;
						pActorInfo->iNowAniIndex = pActionSequence->pActionInfo->iAnimationIndex;

						bool bActionChanged = (pActorInfo->strNowActionName != pActionSequence->pActionInfo->strAnimationName);

						pActorInfo->strNowActionName = pActionSequence->pActionInfo->strAnimationName;

						pActorInfo->iAnimationType = pActionSequence->pActionInfo->iActionType;
						pActorInfo->fAnimationSpeed = pActionSequence->pActionInfo->fAnimationSpeed;

						if( pActionSequence->pActionInfo->bUseStartRotation )
							pActorInfo->fRotYDegree = pActionSequence->pActionInfo->fStartRotation;

						if( pActionSequence->pActionInfo->bUseStartPosition )
							pActorInfo->vPos = pActionSequence->pActionInfo->vStartPos;

						pActorInfo->bUseAniDistance = pActionSequence->pActionInfo->bUseAniDistance;
						if( pActorInfo->bUseAniDistance )
							pActorInfo->bFitAniDistanceYToMap = pActionSequence->pActionInfo->bFitAniDistanceYToMap;
						else
							pActorInfo->bFitAniDistanceYToMap = false;

						bool bEndedSequence = dwEndTime <= ElapsedTime;

						if( bEndedSequence )			// For finished sequences, ensure AniDistance is processed as well.
						{
							pActorInfo->fFrame = ((float)pActionSequence->dwTimeLength * 0.001f) * pActorInfo->fAnimationSpeed;

							// Also apply Y rotation value if present... though if rotations overlap for long, it won't render properly..
							float fFinalRotYDegree = pActorInfo->fRotYDegree + pActorInfo->fNowRotProceed;
							EtMatrixRotationY( &pActorInfo->matRotY, -(fFinalRotYDegree*ET_PI / 180.0f) );
							if( pActorInfo->bUseAniDistance )
							{
								//pActorInfo->hObject->CalcAniDistance( pActorInfo->vlAniIndex.at(pActorInfo->iNowAniIndex),
								//									  pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

								m_pActorProcessor->CalcAniDistance( pActorInfo, pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

								float fScale = m_pActorProcessor->GetActorScale( pActorInfo->iActorIndexInProcessorVector );
								if( fScale != 1.0f )
									pActorInfo->vAniDistanceOffset.y = 0.0f;

								EtVec3TransformCoord( &pActorInfo->vAniDistanceOffset, &pActorInfo->vAniDistanceOffset, &pActorInfo->matRotY );

								if( pActorInfo->bFitAniDistanceYToMap )
								{
									EtVector3 vFinalPos = pActorInfo->vPos + pActorInfo->vAniDistanceOffset;
									pActorInfo->vAniDistanceOffset.y = m_pWorld->GetHeight( vFinalPos.x, vFinalPos.z ) + pActorInfo->vAniDistanceOffset.y - pActorInfo->vPos.y;
								}
								//===
								//	pActorInfo->vAniDistanceOffset.y = 0.0f;
							}
							else
								pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

							pActorInfo->vPos += pActorInfo->vAniDistanceOffset;
							pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

							// If it is a continue action type, keep it as is.
							// Or similarly when there is no following action..
							//if( ActionInfo::AT_CONTINUE != pActorInfo->iAnimationType &&
							//	pActionSequence->pNextSequence != NULL )
							//{;
								pActorInfo->bUseAniDistance = false;
							//}

							pActorInfo->pNowActionSequence = NULL;
						}
						else
						{
							LOCAL_TIME LiveElapsedTime = ElapsedTime - pActionSequence->dwStartTime;
							pActorInfo->fSequenceLiveElapsedSec = (float)LiveElapsedTime * 0.001f;
							pActorInfo->fFrame = pActorInfo->fSequenceLiveElapsedSec * pActionSequence->pActionInfo->fAnimationSpeed;

							//if( bActionChanged )
							//	m_pActorProcessor->OnChangeAction( pActorInfo );

							pActorInfo->pNowActionSequence = pProcessSequence;

							apProcessingSequence[ ACTION_SEQUENCE ] = pProcessSequence;
						}
							
					}
					break;

				case KEY_SEQUENCE:
					{
						S_KEY_SEQUENCE* pKeySequence = static_cast<S_KEY_SEQUENCE*>(pProcessSequence);
						
						LOCAL_TIME LiveElapsed = ElapsedTime - pKeySequence->dwStartTime;
						float fNowProceed = 1.0f;

						bool bEndedSequence = dwEndTime <= ElapsedTime;
						if( bEndedSequence )
						{
							fNowProceed = 1.0f;
						}
						else
						{
							fNowProceed = float(LiveElapsed) / float(pKeySequence->dwTimeLength);
							apProcessingSequence[ KEY_SEQUENCE ] = pProcessSequence;
						}

						const KeyInfo* pKeyInfo = pKeySequence->pKeyInfo;

						switch( pKeySequence->pKeyInfo->iKeyType )
						{
							case KeyInfo::MOVE:
								{
									EtVector3 vDir = pKeyInfo->vDestPos - pKeyInfo->vStartPos;
									EtVector3 vNewPos = pKeyInfo->vStartPos + vDir*fNowProceed;
									EtVector3 vPrevPos = pKeySequence->pAniObjectInfoToPlay->vPrevPos;		// Copy previous position info to use here,

									// Update previous position info to save now.
									//pKeyObject->pAniObjectInfoToPlay->vPrevPos = pKeyObject->pAniObjectInfoToPlay->crossPos.GetPosition();
									pKeySequence->pAniObjectInfoToPlay->vPrevPos = pKeyInfo->vStartPos + vDir*(fNowProceed*0.99f);

									if( pKeyInfo->bRotateFront )
									{
										if( vNewPos != vPrevPos )
										{
											EtVector3 vTangent = vNewPos - vPrevPos;
											_CalcRotateFront( pKeySequence->pAniObjectInfoToPlay, vTangent );
										}
									}

									// Calculated in _ProcessActors().
									//if( pKeyInfo->bFitYPosToMap )
									//{
									//	vNewPos.y = m_pWorld->GetHeight( vNewPos.x, vNewPos.z );
									//}

									pKeySequence->pAniObjectInfoToPlay->vPos = vNewPos;
									pKeySequence->pAniObjectInfoToPlay->bKeyUseYPosToMap = pKeyInfo->bFitYPosToMap;
									//pKeyObject->pAniObjectInfoToPlay->matExFinalPos.SetPosition( vNewPos );

									if( bEndedSequence )
									{
										S_CS_ACTOR_INFO* pActorInfo = pKeySequence->pAniObjectInfoToPlay;
										pActorInfo->vPos = pKeyInfo->vDestPos;

										// If the flag to match the y value is on, calculate anidistance separately and adjust.
										if( pActorInfo->bKeyUseYPosToMap )
										{
											// Also apply Y rotation value if present... though if rotations overlap for long, it won't render properly..
											float fFinalRotYDegree = pActorInfo->fRotYDegree + pActorInfo->fNowRotProceed;

											float fScale = m_pActorProcessor->GetActorScale( pActorInfo->iActorIndexInProcessorVector );
											if( fScale != 1.0f )
												pActorInfo->vAniDistanceOffset.y = 0.0f;

											m_pActorProcessor->CalcAniDistance( pActorInfo, pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

											pActorInfo->vAniDistanceOffset.y = m_pWorld->GetHeight( pActorInfo->vPos.x, pActorInfo->vPos.z ) + pActorInfo->vAniDistanceOffset.y - pActorInfo->vPos.y;
		
										}
										else
											pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

										pActorInfo->vPos.y += pActorInfo->vAniDistanceOffset.y;
										pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

										pActorInfo->bKeyUseYPosToMap = false;
									}
								}
								break;

							case KeyInfo::MULTI_MOVE:
								{
									// First find the subkey based on LiveElapsedTime.
									float fNowStartTime = 0.0f;
									int iNumSubKey = (int)pKeySequence->pKeyInfo->vlMoveKeys.size();
									const SubKey* pNowSubKey = NULL;
									float fNowSubKeyProceed = 0.0f;
									int iNowStartSubKey = -1;
									for( int iKey = 0; iKey < iNumSubKey; ++iKey ) 
									{
										const SubKey& Key = pKeyInfo->vlMoveKeys.at( iKey );

										float fLiveElapsed = (float)LiveElapsed * 0.001f;
										if( fNowStartTime < fLiveElapsed && fLiveElapsed < fNowStartTime+Key.fTimeLength )
										{
											pNowSubKey = &Key;
											fNowSubKeyProceed = (fLiveElapsed - fNowStartTime) / Key.fTimeLength;
											iNowStartSubKey = iKey;
											break;
										}

										fNowStartTime += Key.fTimeLength;
									}

									if( -1 != iNowStartSubKey )
									{
										EtVector3 vNewPos;
										EtVector3 vArgs[ 4 ];
										int iLeaveCount = (int)pKeyInfo->vlMoveKeys.size() - iNowStartSubKey - 1;

										// Initial start index
										if( 0 == iNowStartSubKey )
										{
											vArgs[ 0 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 1 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 2 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+1 ).vPos;
											vArgs[ 3 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+1 ).vPos;
										}
										else
										if( 0 == iLeaveCount )		// Last index
										{
											vArgs[ 0 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 1 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 2 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 3 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
										}
										else
										if( 1 == iLeaveCount )
										{
											vArgs[ 0 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 1 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 2 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+1 ).vPos;
											vArgs[ 3 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+1 ).vPos;
										}
										else
											//if( 2 < iLeaveCount )
										{
											vArgs[ 0 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey-1 ).vPos;
											vArgs[ 1 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey ).vPos;
											vArgs[ 2 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+1 ).vPos;
											vArgs[ 3 ] = pKeyInfo->vlMoveKeys.at( iNowStartSubKey+2 ).vPos;
										}

										if( pKeyInfo->bFitYPosToMap )
										{
											for( int i = 0; i < 4; ++i )
												vArgs[ i ].y = 0.0f;
										}

										EtVec3CatmullRom( &vNewPos, &vArgs[ 0 ], &vArgs[ 1 ], &vArgs[ 2 ], &vArgs[ 3 ], fNowSubKeyProceed );

										EtVector3 vPrevPos = pKeySequence->pAniObjectInfoToPlay->vPrevPos;		// Copy previous position info to use here,

										if( pKeyInfo->bFitYPosToMap )
										{
											vNewPos.y = 0.0f;
											vPrevPos.y = 0.0f;
										}

										EtVector3 vDir = vNewPos - vPrevPos;
										pKeySequence->pAniObjectInfoToPlay->vPrevPos = vNewPos;

										if( pKeyInfo->bRotateFront )
										{
											if( vNewPos != vPrevPos )
											{
												EtVector3 vTangent = vNewPos - vPrevPos;
												_CalcRotateFront( pKeySequence->pAniObjectInfoToPlay, vTangent );
											}
										}


										//if( pKeyInfo->bFitYPosToMap )
										//	vNewPos.y = m_pWorld->GetHeight( vNewPos.x, vNewPos.z );

										pKeySequence->pAniObjectInfoToPlay->vPos = vNewPos;
										pKeySequence->pAniObjectInfoToPlay->bKeyUseYPosToMap = pKeyInfo->bFitYPosToMap;
										//pKeyObject->pAniObjectInfoToPlay->matExFinalPos.SetPosition( vNewPos );

										if( bEndedSequence )
										{
											if( pKeyInfo->bRotateFront )
											{
												EtVector3 vDirInner = pKeyInfo->vlMoveKeys.back().vPos - pKeyInfo->vlMoveKeys.at(pKeyInfo->vlMoveKeys.size()-2).vPos;
												_CalcRotateFront( pKeySequence->pAniObjectInfoToPlay, vDirInner );
											}

											EtVector3 vNewPosInner = pKeySequence->pKeyInfo->vlMoveKeys.back().vPos;
											//if( pKeyInfo->bFitYPosToMap )
											//	vNewPosInner.y = m_pWorld->GetHeight( vNewPosInner.x, vNewPosInner.z );

											S_CS_ACTOR_INFO* pActorInfo = pKeySequence->pAniObjectInfoToPlay;
											pKeySequence->pAniObjectInfoToPlay->vPos = vNewPosInner;

											// If the flag to match the y value is on, calculate anidistance separately and adjust.
											if( pActorInfo->bKeyUseYPosToMap )
											{
												// Also apply Y rotation value if present... though if rotations overlap for long, it won't render properly..
												float fFinalRotYDegree = pActorInfo->fRotYDegree + pActorInfo->fNowRotProceed;

												float fScale = m_pActorProcessor->GetActorScale( pActorInfo->iActorIndexInProcessorVector );
												if( fScale != 1.0f )
													pActorInfo->vAniDistanceOffset.y = 0.0f;

												m_pActorProcessor->CalcAniDistance( pActorInfo, pActorInfo->fFrame, 0.0f, pActorInfo->vAniDistanceOffset );

												pActorInfo->vAniDistanceOffset.y = m_pWorld->GetHeight( pActorInfo->vPos.x, pActorInfo->vPos.z ) + pActorInfo->vAniDistanceOffset.y - pActorInfo->vPos.y;

											}
											else
												pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

											pActorInfo->vPos.y += pActorInfo->vAniDistanceOffset.y;
											pActorInfo->vAniDistanceOffset = EtVector3( 0.0f, 0.0f, 0.0f );

											pActorInfo->bKeyUseYPosToMap = false;
										}
									}
								}
								break;

							case KeyInfo::ROTATION:
								{
									float fRotDeltaToDest = pKeyInfo->fRotDegree - pKeySequence->pAniObjectInfoToPlay->fRotYDegree;
									pKeySequence->pAniObjectInfoToPlay->fNowRotProceed = (fRotDeltaToDest * fNowProceed);

									if( bEndedSequence )
									{
										pKeySequence->pAniObjectInfoToPlay->fRotYDegree += pKeySequence->pAniObjectInfoToPlay->fNowRotProceed;
										pKeySequence->pAniObjectInfoToPlay->fNowRotProceed = 0.0f;
									}
								}
								break;

							case KeyInfo::HIDE:
								pKeySequence->pAniObjectInfoToPlay->hObject->ShowObject( false );
								break;

							case KeyInfo::SHOW:
								pKeySequence->pAniObjectInfoToPlay->hObject->ShowObject( true );
								break;

							default:
								break;
						}

					if( bEndedSequence )
						pKeySequence->pAniObjectInfoToPlay->pNowKeySequence = NULL;
					}

					//_UpdateKeySequence( pProcessSequence );
					break;

				case CAM_EVENT_SEQUENCE:
					{
						S_CAM_EVENT_SEQUENCE* pCamEventSequence = static_cast<S_CAM_EVENT_SEQUENCE*>(pProcessSequence);
						CCameraData* pCamData = pCamEventSequence->pCamEventInfo->pCameraData;
						EtVector3 vCamPos( 0.0f, 0.0f, 0.0f );
						EtVector3 vUp( 0.0f, 1.0f, 0.0f );
						EtVector3 vLookAt( 0.0f, 0.0f, 0.0f );

						// Calculate the current frame.
						LOCAL_TIME LiveElapsedTime = ElapsedTime - pCamEventSequence->dwStartTime;
						float fLiveElapsedTime = (float)LiveElapsedTime * 0.001f;

						float fFrame = fLiveElapsedTime * pCamEventSequence->pCamEventInfo->fSpeed;

						EtMatrix matCamera;

						// Handle FOV first
						if( pCamData->m_Header.nFOVKeyCount > 0 )
						{
							pCamEventSequence->fFOVKeyFrame = fFrame;
							if( (int)pCamEventSequence->fFOVKeyFrame > pCamData->m_vecFOV.back().nTime-1 )
								pCamEventSequence->fFOVKeyFrame = (float)pCamData->m_vecFOV.back().nTime-1;

							int iFOVOffsetIndex = _SearchFOVOffsetIndex( pCamData->m_vecFOV, (int)pCamEventSequence->fFOVKeyFrame );

							const SCameraFOVKey& StartCamFOVKey = pCamData->m_vecFOV.at( iFOVOffsetIndex );
							const SCameraFOVKey& EndCamFOVKey = pCamData->m_vecFOV.at( iFOVOffsetIndex+1 );
							float fNowCamFOVProceed = float(pCamEventSequence->fFOVKeyFrame-(float)StartCamFOVKey.nTime) / float(EndCamFOVKey.nTime - StartCamFOVKey.nTime);

							float fNowFOV = StartCamFOVKey.fFOV + ((EndCamFOVKey.fFOV-StartCamFOVKey.fFOV)*fNowCamFOVProceed);
							EtMatrix matNewProj;
							EtMatrixPerspectiveFovLH( &matNewProj, fNowFOV, m_hCamera->GetAspectRatio(),
													  m_hCamera->GetCameraNear(), m_hCamera->GetCameraFar() );
							m_hCamera->SetProjMat( matNewProj );
						}

						// Handle camera position movement keys
						if( pCamData->m_Header.nPositionKeyCount > 0 )
						{
							pCamEventSequence->fPosKeyFrame = fFrame;
							if( (int)pCamEventSequence->fRotKeyFrame > pCamData->m_vecPosition.back().nTime-1 )
								pCamEventSequence->fRotKeyFrame = float(pCamData->m_vecPosition.back().nTime-1);

							int iPosOffsetIndex = _SearchPosOffsetIndex( pCamData->m_vecPosition, (int)pCamEventSequence->fPosKeyFrame );

							const SCameraPositionKey& StartCamPosKey = pCamData->m_vecPosition.at( iPosOffsetIndex );
							const SCameraPositionKey& EndCamPosKey = pCamData->m_vecPosition.at( iPosOffsetIndex+1 );
							float fNowCamPosProceed = float(pCamEventSequence->fPosKeyFrame-(float)StartCamPosKey.nTime) / float(EndCamPosKey.nTime - StartCamPosKey.nTime);

							EtVec3Lerp( &vCamPos, &(StartCamPosKey.vPosition), &(EndCamPosKey.vPosition), fNowCamPosProceed );

							// Camera start offset value
							vCamPos += pCamEventSequence->pCamEventInfo->vCamStartOffset;
							vLookAt += pCamEventSequence->pCamEventInfo->vCamStartOffset;
						}

						EtQuat qCamRot;
						EtQuaternionIdentity( &qCamRot );
						if( pCamData->m_Header.nRotationKeyCount > 0 )
						{
							pCamEventSequence->fRotKeyFrame = fFrame;
							if( (int)pCamEventSequence->fRotKeyFrame > pCamData->m_vecRotation.back().nTime-1 )
								pCamEventSequence->fRotKeyFrame = float(pCamData->m_vecRotation.back().nTime-1);

							int iRotOffsetIndex = _SearchRotOffsetIndex( pCamData->m_vecRotation, (int)pCamEventSequence->fRotKeyFrame );

							// Rotation key
							const SCameraRotationKey& StartRotKey = pCamData->m_vecRotation.at( iRotOffsetIndex );
							const SCameraRotationKey& EndRotKey = pCamData->m_vecRotation.at( iRotOffsetIndex+1 );
							float fRotKeyProceed = float(pCamEventSequence->fRotKeyFrame-(float)StartRotKey.nTime) / float(EndRotKey.nTime - StartRotKey.nTime);
							EtQuaternionSlerp( &qCamRot, &StartRotKey.qRotation, &EndRotKey.qRotation, fRotKeyProceed );
						}

						EtMatrixTransformation( &matCamera, NULL, NULL, NULL, NULL, &qCamRot, &vCamPos );
						m_hCamera->Update( &matCamera );

						// Handle show/hide for visible and non-visible actors
						if( false == pCamEventSequence->pCamEventInfo->mapActorsShowHide.empty() )
						{
							map<string, bool>& mapActorsShowHide = pCamEventSequence->pCamEventInfo->mapActorsShowHide;
							map<string, bool>::iterator iter = mapActorsShowHide.begin();
							for( iter; iter != mapActorsShowHide.end(); ++iter )
							{
								S_CS_ACTOR_INFO* pActorInfo = m_pActorProcessor->GetActorInfoByName( iter->first.c_str() );
								//if( pActorInfo ) pActorInfo->hObject->ShowObject( iter->second );

								// #54681 - Hide handling for player character.
								if( pActorInfo )
								{
									if( pActorInfo->bPlayer )	// Player.
									{
										if( pActorInfo->pActor )
											m_pActorProcessor->ShowEffect( pActorInfo->pActor, iter->second );
									}
									else	// Other.
									{
										if( pActorInfo->hObject )
											pActorInfo->hObject->ShowObject( iter->second );
									}
								}
							}
						}
					}
					//_UpdateCamEventSequence( pProcessSequence );
					break;

				case DOF_EVENT_SEQUENCE:
					{
						if( !m_pDOFFilter )
						{
							break;
						}
						S_DOF_EVENT_SEQUENCE* pDofEventSequence = static_cast<S_DOF_EVENT_SEQUENCE*>(pProcessSequence);
						DofEventInfo* pDofEventInfo = pDofEventSequence->pDofEventInfo;

						// Calculate the current frame
						LOCAL_TIME WholeTime = pDofEventSequence->dwTimeLength;
						LOCAL_TIME LiveElapsedTime = ElapsedTime - pDofEventSequence->dwStartTime;
						//float fLiveElapsedTime = (float)LiveElapsedTime * 0.001f;
						float fProceed = 0.0f;
						if( WholeTime > LiveElapsedTime )
							fProceed = (float)LiveElapsedTime / (float)WholeTime;
						else
							fProceed = 1.0f;

						float fNearStart = pDofEventInfo->fNearStartFrom + (pDofEventInfo->fNearStartDest-pDofEventInfo->fNearStartFrom)*fProceed;
						float fNearEnd = pDofEventInfo->fNearEndFrom + (pDofEventInfo->fNearEndDest-pDofEventInfo->fNearEndFrom)*fProceed;
						float fFarStart = pDofEventInfo->fFarStartFrom + (pDofEventInfo->fFarStartDest-pDofEventInfo->fFarStartFrom)*fProceed;
						float fFarEnd = pDofEventInfo->fFarEndFrom + (pDofEventInfo->fFarEndDest-pDofEventInfo->fFarEndFrom)*fProceed;
						float fFocusDist = pDofEventInfo->fFocusDistFrom + (pDofEventInfo->fFocusDistDest-pDofEventInfo->fFocusDistFrom)*fProceed;
						float fNearBlurSize = pDofEventInfo->fNearBlurSizeFrom + (pDofEventInfo->fNearBlurSizeDest-pDofEventInfo->fNearBlurSizeFrom)*fProceed;
						float fFarBlurSize = pDofEventInfo->fFarBlurSizeFrom + (pDofEventInfo->fFarBlurSizeDest-pDofEventInfo->fFarBlurSizeFrom)*fProceed;

						m_pDOFFilter->SetNearDOFStart( fNearStart );
						m_pDOFFilter->SetNearDOFEnd( fNearEnd );
						m_pDOFFilter->SetFarDOFStart( fFarStart );
						m_pDOFFilter->SetFarDOFEnd( fFarEnd );
						m_pDOFFilter->SetFocusDistance( fFocusDist );
						m_pDOFFilter->SetNearBlurSize( fNearBlurSize );
						m_pDOFFilter->SetFarBlurSize( fFarBlurSize );
					}
					break;

				case PARTICLE_EVENT_SEQUENCE:
					{
						set<S_SEQUENCE*>::iterator iter = m_setProcessedTrigger.find( pProcessSequence );
						if( m_setProcessedTrigger.end() == iter )
						{
							_BeginParticleEvent( pProcessSequence );
							m_setProcessedTrigger.insert( pProcessSequence );
							apProcessingSequence[ PARTICLE_EVENT_SEQUENCE ] = pProcessSequence;
						}
					}
					//_UpdateParticleSequence( pProcessSequence );
					break;

				//case SOUND_EVENT_SEQUENCE:
				//	{
				//		set<S_SEQUENCE*>::iterator iter = m_setProcessedTrigger.find( pProcessSequence );
				//		if( m_setProcessedTrigger.end() == iter )
				//		{
				//			_BeginSoundEvent( pProcessSequence );
				//			m_setProcessedTrigger.insert( pProcessSequence );
				//			apProcessingSequence[ SOUND_EVENT_SEQUENCE ] = pProcessSequence;
				//		}
				//	}
				//	//_UpdateSoundEventSequence( pProcessSequence );
				//	break;

				case FADE_EVENT_SEQUENCE:
					{
						set<S_SEQUENCE*>::iterator iter = m_setProcessedTrigger.find( pProcessSequence );
						if( m_setProcessedTrigger.end() == iter )
						{
							_BeginFadeEvent( pProcessSequence );
							m_setProcessedTrigger.insert( pProcessSequence );
							apProcessingSequence[ FADE_EVENT_SEQUENCE ] = pProcessSequence;
						}
					}
					//_UpdateFadeEventSequence( pProcessSequence );
					break;

				case PROP_EVENT_SEQUENCE:
					{
						set<S_SEQUENCE*>::iterator iter = m_setProcessedTrigger.find( pProcessSequence );
						if( m_setProcessedTrigger.end() == iter )
						{
							_ProcessPropActionEvent( pProcessSequence, AbsoluteTime, ElapsedTime );
							m_setProcessedTrigger.insert( pProcessSequence );
							apProcessingSequence[ PROP_EVENT_SEQUENCE ] = pProcessSequence;
						}
					}
					break;

				case SOUND_EVENT_1_SEQUENCE:
				case SOUND_EVENT_2_SEQUENCE:
				case SOUND_EVENT_3_SEQUENCE:
				case SOUND_EVENT_4_SEQUENCE:
				case SOUND_EVENT_5_SEQUENCE:
				case SOUND_EVENT_6_SEQUENCE:
				case SOUND_EVENT_7_SEQUENCE:
				case SOUND_EVENT_8_SEQUENCE:
					{
						set<S_SEQUENCE*>::iterator iter = m_setProcessedTrigger.find( pProcessSequence );
						if( m_setProcessedTrigger.end() == iter )
						{
							_BeginSoundEvent( pProcessSequence );
							m_setProcessedTrigger.insert( pProcessSequence );

							int iSequenceIndex = SOUND_EVENT_1_SEQUENCE + (pProcessSequence->iType-SOUND_EVENT_1_SEQUENCE);
							apProcessingSequence[ iSequenceIndex ] = pProcessSequence;
						}
					}
					break;

				case SUBTITLE_EVENT_SEQUENCE:
					{
						if( m_pLiveSubtitleSequence == NULL ||
							(m_pLiveSubtitleSequence->pSubtitleEventInfo->iUIStringID != 
							 static_cast<S_SUBTITLE_EVENT_SEQUENCE*>(pProcessSequence)->pSubtitleEventInfo->iUIStringID) )
						{
							m_pActorProcessor->HideSubtitle();
							_BeginSubtitleEvent( pProcessSequence );
							apProcessingSequence[ SUBTITLE_EVENT_SEQUENCE ] = pProcessSequence;
						}
					}
					break;


//#ifdef PRE_ADD_FILTEREVENT
				case COLORFILTER_EVENT_SEQUENCE:
					{
						S_COLORFILTER_EVENT_SEQUENCE * pColorFilterEventSequence = static_cast<S_COLORFILTER_EVENT_SEQUENCE*>(pProcessSequence);
						ColorFilterEventInfo * pCFEventInfo = pColorFilterEventSequence->pEventInfo;
						//DofEventInfo* pDofEventInfo = pDofEventSequence->pDofEventInfo;

						// Calculate the current frame
						LOCAL_TIME WholeTime = pColorFilterEventSequence->dwTimeLength;
						LOCAL_TIME LiveElapsedTime = ElapsedTime - pColorFilterEventSequence->dwStartTime;
						//float fLiveElapsedTime = (float)LiveElapsedTime * 0.001f;
						float fProceed = 0.0f;
						if( WholeTime > LiveElapsedTime )
							fProceed = (float)LiveElapsedTime / (float)WholeTime;
						else
							fProceed = 1.0f;
						
						//static_cast<CDnCutSceneWorld *>(m_pWorld)->SetSceneAbsoluteColor( pCFEventInfo->vColor, pCFEventInfo->fVolume );
						m_pWorld->SetSceneAbsoluteColor( pCFEventInfo->bMonochrome, pCFEventInfo->vColor, pCFEventInfo->fVolume );
					}
					break;
//#endif PRE_ADD_FILTEREVENT
			}
		}

		// Skip if the start time has passed ElapsedTime
		++iSequence;

		bool bDBEnd = false;
		bDBEnd = (iSequence >= (int)m_vlpSequenceDB.size());
		if( bDBEnd )
			break;

		pProcessSequence = m_vlpSequenceDB.at( iSequence );
	}
}

void CDnCutScenePlayer::_ProcessSubtitle( LOCAL_TIME ElapsedTime )
{
	if( m_pLiveSubtitleSequence )
	{
		// Hide subtitles when time runs out
		if( ElapsedTime - m_pLiveSubtitleSequence->dwStartTime > m_pLiveSubtitleSequence->dwTimeLength )
		{
			m_pActorProcessor->HideSubtitle();
		}
	}
}

void CDnCutScenePlayer::Process( LOCAL_TIME AbsoluteTime )
{
	LOCAL_TIME ElapsedTime = AbsoluteTime - m_StartTime;

	if( false == IsEndScene() )
	{
		//_FigureOutEndSeuqnces(ElapsedTime);
		//_FigureOutLiveSequences(ElapsedTime);
		_UpdateSequence( AbsoluteTime, ElapsedTime );
		//_ProcessSequences(ElapsedTime);
		_ProcessActors(AbsoluteTime, ElapsedTime);
		_ProcessSubtitle( ElapsedTime );

		if( m_WholeSceneLength < ElapsedTime )
		{
			StopPlay();
		}
	}

	m_PrevTime = AbsoluteTime;
}


void CDnCutScenePlayer::ResetPlayingSoundChannels( void )
{
	// Stop playing sound.. Currently no function exists, so lower volume to 0.
	int iNumPlayingChannel = (int)m_vlhPlayingChannel.size();
	for( int i = 0; i < iNumPlayingChannel; ++i )
	{
		EtSoundChannelHandle hChannel = m_vlhPlayingChannel.at(i);
		if( hChannel )
			CEtSoundEngine::GetInstance().RemoveChannel( hChannel );
	}
	m_vlhPlayingChannel.clear();
}

void CDnCutScenePlayer::StopPlay()
{
	if( m_bSceneEnd == true ) return;
	m_bSceneEnd = true;
	// Restore the projection matrix.
	m_hCamera->SetProjMat( m_matOriProj );

	ResetPlayingSoundChannels();

	m_pActorProcessor->OnEndPlayScene();

//#ifdef PRE_ADD_FILTEREVENT
	m_pWorld->SetSceneAbsoluteColor( false, EtVector3(1.0f,1.0f,1.0f), 1.0f );
//#endif // PRE_ADD_FILTEREVENT
}