#pragma once
#include "DnObserver.h"

//------------------------------------------------------------------------------
/**
Declared per event used.
*/
//------------------------------------------------------------------------------
enum
{
	NONE_BUBBLE_EVENT_MESSAGE = 0,
	
	// Messages sent to the bubble system. //////////////////////////////////////
	EVENT_BUBBLE_ON_USE_SKILL,					// When a specific skill is used.
	EVENT_BUBBLE_BLOCK_SUCCESS,					// When the specified block state effect succeeds for a specific skill.
	EVENT_BUBBLE_COUNT_UPDATED,					// Count of a specific bubble was updated. 
	EVENT_BUBBLE_PARRING_SUCCESS,				// When the specified parry state effect succeeds for a specific skill.
	EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS,		// When the specified cooldown parry state effect succeeds for a specific skill.
	//EVENT_BUBBLE_HIT_BY_SKILL, // Hit by a specific skill.
	EVENT_ONCHANGEACTION,						// Action changed.
	EVENT_PLAYER_KILL_TARGET,					// When killing someone
	//////////////////////////////////////////////////////////////////////////

	EVENT_ONCRITICALHIT = 50,
};

//------------------------------------------------------------------------------
/**
Event type interface
*/
//------------------------------------------------------------------------------
class IDnObserverNotifyEvent
{
private:
	int m_iEventType;

protected:
	IDnObserverNotifyEvent( void ) 
	{ 
		m_iEventType = 0; 
	};
	
public:
	virtual ~IDnObserverNotifyEvent( void ) {};

	static IDnObserverNotifyEvent* Create( int iType );

	virtual void SetSkillID( int iSkillID ) {};
	virtual void SetBubbleTypeID( int iBubbleTypeID ) {};

	void SetEventType( int iEventType ) { m_iEventType = iEventType; };
	int GetEventType( void ) { return m_iEventType; }
};


//------------------------------------------------------------------------------
/**
Observer registration/removal and event notification
*/
//------------------------------------------------------------------------------
class CDnObservable
{
private:
	vector<CDnObserver*> m_vlpObservers;

public:
	CDnObservable( void );
	virtual ~CDnObservable( void );

	bool RegisterObserver( CDnObserver* pObserver );
	bool RemoveObserver( CDnObserver* pObserver );
	void Notify( boost::shared_ptr<IDnObserverNotifyEvent> pEvent );
};
