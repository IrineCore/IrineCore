#pragma once
#include "DnObserver.h"
#include "SmartPtrDef.h"
#include "DNTableFile.h"

class CPacketCompressStream;

namespace BubbleSystem
{
class CDnBubble;
class IDnConditionChecker;
class IDnBubbleEventHandler;

//------------------------------------------------------------------------------
/**
Class implementing the bubble system.

This class is held one per CDnPlayerActor on both client and game server, and
Handle player bubble acquisition, retention, and consumption as defined in data.
Contents may vary depending on character class.

If monsters themselves accumulate bubbles in the future, in CDnMonsterActor as well
Must keep as a member and process it.
*/
//------------------------------------------------------------------------------
class CDnBubbleSystem : public CDnObserver
{
public:
	struct S_CREATE_BUBBLE 
	{
		int iBubbleTypeID;
		int iIconIndex;
		float fDurationTime;
		int iServerBubbleCount;		// Used only on client. Used to correct when count differs from server.

		S_CREATE_BUBBLE( void ) : iBubbleTypeID( 0 ), iIconIndex( 0 ), fDurationTime( 0.0f ), iServerBubbleCount( 0 ) {};
	};

	// Bubble info queryable externally.
	struct S_BUBBLE_INFO
	{
		int iBubbleTypeID;
		int iCount;
		int iIconIndex;
		float fDurationTime;
		float fRemainTime;
		S_BUBBLE_INFO( void ) : iBubbleTypeID( 0 ), iCount( 0 ), iIconIndex( 0 ), fDurationTime( 0.0f ), fRemainTime( 0.0f ) {};
	};

private:
	// Actor that owns this bubble system object
	DnActorHandle m_hActor;

	// For now, bubbles added earlier are removed later in that order.
	//vector<CDnBubble*> m_vlpBubbles;
	typedef map<int, deque<CDnBubble*> > BubblesByTypeID;
	typedef BubblesByTypeID::iterator BubblesByTypeID_iter;
	BubblesByTypeID m_mapBubblesByTypeID;

	// Struct collecting event information defined in the bubble table
	struct S_DEFINED_BUBBLE_EVENT
	{
		int iTableID;		// Bubble table ID
		vector<IDnConditionChecker*> vlpConditions;
		vector<IDnBubbleEventHandler*> vlpEventHandlers;

		S_DEFINED_BUBBLE_EVENT( void ) : iTableID( 0 ) {};
		~S_DEFINED_BUBBLE_EVENT( void );
	};

	// Event info organized by table ID
	//map<int, S_DEFINED_BUBBLE_EVENT*> m_mapDefinedBubbleEvent;
	vector<S_DEFINED_BUBBLE_EVENT*> m_vlpDefinedBubbleEvent;

	// Event definition info grouped by each event type
	// Since condition info used in events is not defined in external data 
	// Hardcode the relationship in code like this.
	typedef multimap<int, S_DEFINED_BUBBLE_EVENT*> DefinedByEventMMap;
	typedef DefinedByEventMMap::iterator DefinedByEventMMap_iter;
	DefinedByEventMMap m_mmapDefinedByEvent;

	// Collection of event handlers processed on consumption per bubble type.
	// Bubble consumption events are not defined in a separate table; conditions are placed directly in code.
	//map<int, IDnBubbleEventHandler*> m_mapBubbleRemoveEventHandlers;

	// State effects added by bubbles.
	map<int, vector<int> > m_mapBubbleStateBlowsByBubbleType;

	// Consumption method per bubble type.

	// Effect generated when maintaining bubbles by type.


protected:
	void _CreateBubble( const S_CREATE_BUBBLE& Info );

	CDnBubble* _CreateNewBubble( const S_CREATE_BUBBLE &Info );

	void _OnCreatedBubble( CDnBubble* pCreatedBubble );
	//void _OnRemoveBubble( CDnBubble* pBubbleToRemove );
	void _OnRemovedBubbles( int iBubbleTypeID, int iCount );

	int _GetRelatedEventMessageType( int iConditionType );

	void _OnCreateEventHandler( DNTableFileFormat* pBubbleTable, int iBubbleTableID, IDnBubbleEventHandler* pEventHandler, const char* pArgument );

public:
	CDnBubbleSystem( void );
	virtual ~CDnBubbleSystem( void );

	// TODO: Receive required info from outside as a struct (job history, skill list, etc.).
	void Initialize( DnActorHandle hActor );

	// TODO: When a character changes class, etc., all data must be reset and re-initialized.
	void Clear( void );
	
	// Returns count corresponding to the passed bubble type ID.
	int GetBubbleCountByTypeID( int iBubbleTypeID );

	// Number of bubble types currently added
	void GetAllAppliedBubbles( /*OUT*/ vector<S_BUBBLE_INFO>& vlBubbleInfos );
	
	// Add bubble
	void AddBubble( const S_CREATE_BUBBLE& Info );

	// Used only on client
	void AddBubbleAndCountRevision( const S_CREATE_BUBBLE& Info );

	void RevisionBubbleCount( const S_CREATE_BUBBLE& Info );

	// Remove specified count of bubbles matching the passed bubble type ID.
	void RemoveBubbleByTypeID( int iBubbleTypeID, int iRemoveCount );

	// Remove all bubbles matching the passed bubble type ID.
	void RemoveAllBubbleByTypeID( int iBubbleTypeID );

	// Delete all bubbles.
	void RemoveAllBubbles( bool bHandleRemoveEvent = true );

	// Get the bubble object corresponding to the bubble type.
	CDnBubble* GetBubble( int iBubbleTypeID );

	// Sets the duration of a specific bubble type to the time given as an argument.
	void SetDurationTime( int iBubbleTypeID, float fDurationTime );
	
	// Process
	void Process( LOCAL_TIME LocalTime, float fDelta );

	// from CDnObserver
	virtual void OnEvent( boost::shared_ptr<::IDnObserverNotifyEvent> pEvent );

	// Get the handle of the actor that owns this bubble system
	DnActorHandle GetHasActor( void ) { return m_hActor; };

#ifdef _GAMESERVER
	// Called only from CDnAddStateEffectHandler.
	void AddBubbleStateBlow( int iBubbleTypeID, int iBlowID );

	// Called only from CDnRemoveStateEffectHandler.
	void RemoveBubbleStateBlow( int iBubbleTypeID );
#else
	void CreateBubbleFromPacketStream( ::CPacketCompressStream* pStream );
	void RefreshBubbleDurationTimeFromPacketStream( ::CPacketCompressStream* pStream );
#endif // #ifdef _GAMESERVER
};

} // namespace BubbleSystem