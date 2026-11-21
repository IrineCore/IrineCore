#pragma once

#include "IDnCutSceneDataReader.h"


// Interface definition for modifying cutscene data
class IDnCutSceneDataModifier
{
private:

public:
	IDnCutSceneDataModifier(void);
	virtual ~IDnCutSceneDataModifier(void);

	virtual bool RegisterResource( const char* pResName, int iResourceKind ) = 0;
	virtual bool UnRegisterResource( const char* pResName, int iResourceKind ) = 0;
	virtual bool SetMonsterTableID( const char* pActorName, int iMonsterTableID ) = 0;
	virtual DWORD GenerateID( void ) = 0;

	// Related to new overall attribute info
	virtual bool ModifySceneInfo( SceneInfo* pSceneInfo ) = 0;
	
	// Action related..
	virtual bool InsertAction( ActionInfo* pActionInfo ) = 0;
	virtual bool ModifyActionInfo( ActionInfo& Action ) = 0;
	virtual bool RemoveAction( int iID ) = 0;

	virtual bool InsertKey( KeyInfo* pKeyInfo ) = 0;
	virtual bool ModifyKeyInfo( KeyInfo& Key ) = 0;
	virtual bool RemoveKey( int iID ) = 0;

	// Event related..
	virtual bool InsertEvent( const EventInfo* pEventInfo ) = 0;
	virtual bool ModifyEventInfo( const EventInfo* pEventInfo ) = 0;
	virtual bool RemoveEvent( int iID ) = 0; 

	virtual bool ChangeRegResName( const char* pOriResName, const char* pNewResName ) = 0;
	virtual bool AddActorsAnimation( const char* pActorName, ActionEleInfo* pAnimationElement ) = 0;
	virtual bool SetActorsFitYPosToMap( const char* pActorName, bool bFitYPosToMap ) = 0;
	virtual bool SetActorsInfluenceLightmap( const char* pActorName, bool bInfluenceLightmap ) = 0;
	virtual bool SetActorsScaleLock( const char* pActorName, bool bScaleLock ) = 0;
	virtual bool ClearActorsAnimations( const char* pActorName ) = 0;
	virtual bool SetRegResPos( const char* pResName, EtVector3& vPos ) = 0;
	virtual bool SetRegResRot( const char* pResName, float fRot ) = 0;

	// Save data
	virtual bool SaveToFile( const char* pFileName ) = 0;

	// Clear data
	virtual void Clear( void ) = 0;
};
