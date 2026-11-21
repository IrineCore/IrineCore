#pragma once

#include "EtMatrixEx.h"
#include "EtWorld.h"

// Collection of animation object info
struct S_CS_ACTOR_INFO
{
	int iActorIndexInProcessorVector;				// Index of the vector stored in ActorProcessor.

	void*					pActor;
	EtAniObjectHandle		hObject;				// Handle must be passed externally when calling AddObjectToPlay().
	// For players, call EnableCull separately since they hold a combined aniobject handle.
	// Animation is attached as a child of the default member aniobject, so it doesn't matter.
	EtAniObjectHandle		hMergedObject;
	string					strActorName;
	MatrixEx			matExFinal;			// Final position taking Ani Distance into account
	EtVector3				vPrevPos;
	bool					bUseAniDistance;
	bool					bKeyUseYPosToMap;		// When YPosToMap flag is enabled in key sequence.
	bool					bFitAniDistanceYToMap;	// When using Ani Distance.. automatically adjusts Y height.
	bool					bInfluenceLightmap;		// Whether affected by lightmap or not.
	EtVector3				vAniDistanceOffset;		// When using Ani Distance.. value is output to this offset position.
	EtVector3				vPos;					// Original position before applying Ani Distance
	float					fRotYDegree;			// Rotation value
	float					fNowRotProceed;	
	EtVector3				vDefaultPos;			// Initial position value
	float					fDefaultYRotation;		// Initial Y rotation value
	EtMatrix				matRotY;				// Transformation matrix corresponding to Rotation
	vector<int>				vlAniIndex;				// Vector list mapping action index to actual animation index. Used only in tool, probably..
	int						iAnimationType;
	int						iNowAniIndex;
	string					strNowActionName;
	float					fSequenceLiveElapsedSec;
	DWORD					dwSequenceStartTime;
	float					fAnimationSpeed;
	float					fFrame;
	bool					bElapsed;				// At least 1 animation loop has occurred.
	bool					bHided;					// Actor that does not appear in the cutscene

	void*					pNowActionSequence;
	void*					pNowKeySequence;

	bool				    bPlayer; // Whether it is a player.
	bool					bClone;  // #60295 Kali not visible in Kali opening cutscene. - Flag to remove extra player actors when multiple were created.

	S_CS_ACTOR_INFO( void ) : iActorIndexInProcessorVector( 0 ),
							  pActor( NULL ), fFrame( 0.0f ), iNowAniIndex( -1 ), iAnimationType( 0 ), 
							  bUseAniDistance( false ), bKeyUseYPosToMap( false ), bFitAniDistanceYToMap( false ), vAniDistanceOffset( 0.0f, 0.0f, 0.0f ),
							  vPos( 0.0f, 0.0f, 0.0f ),
							  fRotYDegree( 0.0f ), fNowRotProceed( 0.0f ), bElapsed( false ),
							  vDefaultPos( 0.0f, 0.0f, 0.0f ), fDefaultYRotation( 0.0f ),
							  pNowActionSequence( NULL ),
							  pNowKeySequence( NULL ),
							  bHided( false ),
							  bInfluenceLightmap( false ),
							  dwSequenceStartTime(0),
							  fAnimationSpeed(0.0f),
							  fSequenceLiveElapsedSec(0.0f),
							  bPlayer(false),
							  bClone(false)
	{
		EtMatrixIdentity( &matRotY );
	};	
};


struct S_CS_PROP_INFO
{
	int iPropID;
	string strPropActioName;
};


// This is a custom actor processor!
class ICustomActorProcessor
{
protected:
	vector<S_CS_ACTOR_INFO*>		m_vlpActors;			// Share animation object info and control among objects
	map<string, S_CS_ACTOR_INFO*>	m_mapActors;

	CEtWorld*						m_pWorld;
	vector<S_CS_PROP_INFO*>			m_vlpProps;


protected:
	ICustomActorProcessor(void);							// Cannot be instantiated standalone.

public:
	virtual ~ICustomActorProcessor(void);

	virtual void AddObjectToPlay( S_CS_ACTOR_INFO* pNewObject );
	virtual void AddPropToPlay( S_CS_PROP_INFO* pPropInfo );
	virtual void CalcAniDistance(  S_CS_ACTOR_INFO* pActorInfo, float fCurFrame, float fPrevFrame, EtVector3& DistVec ) = 0;
	virtual void SetWorld( CEtWorld* pWorld ) { m_pWorld = pWorld; };
	virtual void SetPropAction( CEtWorldProp* pProp, const char* pActionName, LOCAL_TIME AbsoluteTime, float fFrame ) = 0;
	virtual void OnChangeAction( S_CS_ACTOR_INFO* pActionChangeActor ) {};
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) = 0;
	virtual void OnEndPlayScene( void ) {};
	virtual void ShowSubtitle( const wchar_t* pSubtitle ) = 0;
	virtual void HideSubtitle( void ) = 0;
	virtual int GetActionIndex( int iActorIndex, const char* pActionName ) = 0;		// Main characters may change, so retrieve the index reliably.
	virtual float GetActorScale( int iActorIndex ) = 0;
	virtual void ShowProp( CEtWorldProp* pProp, bool bShow ) = 0;

	int GetNumActors( void ) { return (int)m_vlpActors.size(); };
	S_CS_ACTOR_INFO* GetActorInfoByIndex( int iIndex ) { return m_vlpActors.at( iIndex ); };
	S_CS_ACTOR_INFO* GetActorInfoByName( const char* pActorName );

	// Unrelated to actor, but tool and client use different fade-in/out -_-
	virtual void FadeIn( float fSpeed, DWORD dwColor ) {};
	virtual void FadeOut( float fSpeed, DWORD dwColor ) {};


	virtual void ShowEffect( void * pActor, bool bShow ){} // #54681 - Hide handling for player character.
};
