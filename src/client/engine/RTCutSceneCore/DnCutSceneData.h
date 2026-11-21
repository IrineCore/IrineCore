#pragma once

#include "IDnCutSceneDataReader.h"
#include "IDnCutSceneDataModifier.h"

struct lua_State;

// Holds all info needed to run a cutscene and also handles file I/O.
class CDnCutSceneData : public IDnCutSceneDataReader,
						public IDnCutSceneDataModifier
{
public:
	enum
	{
		RT_RES_MAP,
		RT_RES_ACTOR,
		//RT_RES_MAX_CAMERA,
		RT_RES_SOUND,
		RT_COUNT,
	};

	enum
	{
		AT_ACTION,
		AT_MOVE,
		AT_ROTATION,
		AT_COUNT,
	};


private:
	struct S_USE_RESOURCE
	{
		string		strResName;
		int			iResourceKind;

		virtual ~S_USE_RESOURCE() {};
	};

	struct S_ACTOR_RESOURCE : public S_USE_RESOURCE
	{
		EtVector3	vPos;
		//string		strBaseAnimation;
		float		fRotation;
		vector<ActionEleInfo*>	vlpActionElements;
		bool		bFitYPosToMap;
		bool		bInfluenceLightmap;
		int			iMonsterTableID;		// If it is a boss mob actor, set the table info containing boss monster data from the tool.
		bool		bScaleLock;

		S_ACTOR_RESOURCE( void ) : vPos( 0.0f, 0.0f, 0.0f ), 
								   fRotation( 0.0f ),
								   bFitYPosToMap( false ),
								   bInfluenceLightmap( true ),
								   iMonsterTableID( 0 ),
								   bScaleLock( true )
		{

		}

		~S_ACTOR_RESOURCE( void ) 
		{
			for_each( vlpActionElements.begin(), vlpActionElements.end(), 
					  DeleteData<ActionEleInfo*>() );
		}
	};

	struct FindActionByID : public unary_function<const ActionInfo*, bool>
	{
		int m_iIDToFind;

		FindActionByID( int iID ) : m_iIDToFind( iID ) {};

		bool operator () ( const ActionInfo* pLhs )
		{
			return pLhs->iID == m_iIDToFind;
		}
	};

	SceneInfo							m_SceneInfo;

	map<string, S_USE_RESOURCE*>		m_mapUseRes;
	vector<S_USE_RESOURCE*>				m_vlUseRes;
	vector<S_USE_RESOURCE*>				m_vlActors;
	
	map< string, vector<ActionInfo*> >	m_mapActionSequenceDB;
	map<int, ActionInfo*>				m_mapActionIDTable;
	
	map<string, vector<KeyInfo*> >		m_mapKeySequenceDB;
	map<int, KeyInfo*>					m_mapKeyIDTable;

	//map<int, vector<EventInfo*> >		m_mapEventDB;
	vector<EventInfo*>					m_avlEventSequenceDB[ EventInfo::TYPE_COUNT ];
	map<int, EventInfo*>				m_mapEventIDTable;

	DWORD							    m_dwNowActionIDOffset;
	//vector<ActionInfo*>				m_vlActionSequence;


public:
	CDnCutSceneData(void);
	virtual ~CDnCutSceneData(void);


private:
	void _UpdateOrderActionSequence( const char* pActorName );
	void _UpdateOrderKeySequence( const char* pActorName );
	void _UpdateOrderEventSequence( int iEventType );

	EventInfo* _CreateEventInfo( int iEventType );

	void _UpdateID( DWORD dwID );
	void _SaveToLuaTable( lua_State* pLua );
	void _LoadFromLuaTable( lua_State* pLua, int iDataTableIndex );

public:
	// from IDnCutSceneDataReader
	int GetNumRegResource( void ) const;
	int GetRegResKindByIndex( int iIndex ) const;
	const char* GetRegResNameByIndex( int iIndex ) const;
	bool IsRegResource( const char* pResName );
	int GetNumActors( void ) const;
	const char* GetActorByIndex( int iActorIndex );

	// Related to new overall attributes
	const SceneInfo* GetSceneInfo( void ) { return &m_SceneInfo; };
	bool ModifySceneInfo( SceneInfo* pSceneInfo ) { m_SceneInfo = *pSceneInfo; return true; };

	const ActionInfo* GetActionInfoByID( int iID );
	int GetThisActorsActionNum( const char* pActorName );
	const ActionInfo* GetThisActorsActionInfoByIndex( const char* pActorName, int iActionIndex );

	const KeyInfo* GetKeyInfoByID( int iID );
	int GetThisActorsKeyNum( const char* pActorName );
	const KeyInfo* GetThisActorsKeyInfoByIndex( const char* pActorName, int iKeyIndex );
	
	int GetThisActorsAnimationNum( const char* pActorName );
	const ActionEleInfo* GetThisActorsAnimation( const char* pActorName, int iAnimationIndex );
	bool GetThisActorsFitYPosToMap( const char* pActorName );
	bool GetThisActorsInfluenceLightmap( const char* pActorName );
	bool GetThisActorsScaleLock( const char* pActorName );
	const EtVector3& GetRegResPos( const char* pResName );
	float GetRegResRot( const char* pResName );

	// Event related
	const EventInfo* GetEventInfoByID( int iID );
	int GetThisTypesEventNum( int iEventType );
	const EventInfo* GetEventInfoByIndex( int iEventType, int iIndex );

	bool LoadFromFile( const char* pFileName );
	bool LoadFromStringBuffer( const char* pStringBuffer );
	//

	// from IDnCutSceneDataModifier
	bool RegisterResource( /*int iTableID, */const char* pResName, int iResourceKind );
	bool UnRegisterResource( const char* pResName, int iResourceKind );
	bool SetMonsterTableID( const char* pActorName, int iMonsterTableID );
	int GetMonsterTableID( const char* pActorName );
	bool ChangeRegResName( const char* pOriResName, const char* pNewResName );
	// pResName and pActorName are the same concept, except adding animations is only possible for actors, so it is a function specialized for actors.
	bool AddActorsAnimation( const char* pActorName, ActionEleInfo* pAnimationElement );
	bool SetActorsFitYPosToMap( const char* pActorName, bool bFitYPosToMap );
	bool SetActorsInfluenceLightmap( const char* pActorName, bool bInfluenceLightmap );
	bool SetActorsScaleLock( const char* pActorName, bool bScaleLock );
	bool ClearActorsAnimations( const char* pActorName );
	bool SetRegResPos( const char* pResName, EtVector3& vPos );
	bool SetRegResRot( const char* pResName, float fRot );

	DWORD GenerateID( void );

	bool InsertAction( ActionInfo* pActionInfo );
	bool ModifyActionInfo( ActionInfo& Action );
	bool RemoveAction( int iID );

	bool InsertKey( KeyInfo* pKeyInfo );
	bool ModifyKeyInfo( KeyInfo& Key );
	bool RemoveKey( int iID );

	// Event related
	bool InsertEvent( const EventInfo* pEventInfo );
	bool ModifyEventInfo( const EventInfo* pEventInfo );
	bool RemoveEvent( int iID );

	bool SaveToFile( const char* pFileName );

	void Clear( void );
	//

};
