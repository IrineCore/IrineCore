#pragma once
#include "DnObservable.h"

namespace BubbleSystem
{

//------------------------------------------------------------------------------
/**
Specific skill use message
*/
//------------------------------------------------------------------------------
class CDnUseSkillMessage : public ::IDnObserverNotifyEvent
{
private:
	// Skill ID that has this block state effect
	int m_iSkillID;
	int m_iSkillLevel;

public:
	CDnUseSkillMessage( void ) : m_iSkillID( 0 ), m_iSkillLevel( 0 )
	{ 
		SetEventType( EVENT_BUBBLE_ON_USE_SKILL );
	};

	virtual ~CDnUseSkillMessage( void ) {};

	virtual void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };
	int GetSkillID( void ) { return m_iSkillID; };
	void SetSkillLevel( int iSkillLevel );
	int GetSkillLevel( void ) { return m_iSkillID; };
};

//------------------------------------------------------------------------------
/**
Block success message from block state effect
*/
//------------------------------------------------------------------------------
class CDnBlockSuccessMessage : public ::IDnObserverNotifyEvent
{
private:
	// Skill ID that has this block state effect
	int m_iSkillID;

public:
	CDnBlockSuccessMessage( void ) : m_iSkillID( 0 )
	{ 
		SetEventType( EVENT_BUBBLE_BLOCK_SUCCESS );
	};

	virtual ~CDnBlockSuccessMessage( void ) {};

	virtual void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };
	int GetSkillID( void ) { return m_iSkillID; };
};

//------------------------------------------------------------------------------
/**
Count of a specific bubble type was updated.
*/
//------------------------------------------------------------------------------
class CDnBubbleCountUpdatedMessage : public ::IDnObserverNotifyEvent
{
private:
	int m_iBubbleTypeID;

public:
	CDnBubbleCountUpdatedMessage( void ) : m_iBubbleTypeID( 0 )
	{ 
		SetEventType( EVENT_BUBBLE_COUNT_UPDATED );
	};

	virtual ~CDnBubbleCountUpdatedMessage( void ) {};

	virtual void SetBubbleTypeID( int iBubbleTypeID ) { m_iBubbleTypeID = iBubbleTypeID; };
	int GetBubbleTypeID( void ) { return m_iBubbleTypeID; };
};

//------------------------------------------------------------------------------
/**
Parry success message from parry state effect
*/
//------------------------------------------------------------------------------
class CDnParringSuccessMessage : public ::IDnObserverNotifyEvent
{
private:
	// Skill ID that has this parry state effect
	int m_iSkillID;

public:
	CDnParringSuccessMessage( void ) : m_iSkillID( 0 )
	{ 
		SetEventType( EVENT_BUBBLE_PARRING_SUCCESS );
	};

	virtual ~CDnParringSuccessMessage( void ) {};

	virtual void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };
	int GetSkillID( void ) { return m_iSkillID; };
};

//------------------------------------------------------------------------------
/**
Cooldown parry success message from cooldown parry state effect
*/
//------------------------------------------------------------------------------
class CDnCooltimeParringSuccessMessage : public ::IDnObserverNotifyEvent
{
private:
	// Skill ID that has this cooldown parry state effect
	int m_iSkillID;

public:
	CDnCooltimeParringSuccessMessage( void ) : m_iSkillID( 0 )
	{ 
		SetEventType( EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS );
	};

	virtual ~CDnCooltimeParringSuccessMessage( void ) {};

	virtual void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };
	int GetSkillID( void ) { return m_iSkillID; };
};

//------------------------------------------------------------------------------
/**
Message when performing a normal attack.
*/
//------------------------------------------------------------------------------
class CDnOnChangeActionMessage : public ::IDnObserverNotifyEvent
{
private:
	//const char* m_pActionNameToChange;

public:
	CDnOnChangeActionMessage( void )
	{ 
		SetEventType( EVENT_ONCHANGEACTION );
	};

	virtual ~CDnOnChangeActionMessage( void ) {};
};

//------------------------------------------------------------------------------
/**
When a player kills another target.
*/
//------------------------------------------------------------------------------
class CDnPlayerKillTargetMessage : public ::IDnObserverNotifyEvent
{
private:

public:
	CDnPlayerKillTargetMessage( void )
	{ 
		SetEventType( EVENT_PLAYER_KILL_TARGET );
	};

	virtual ~CDnPlayerKillTargetMessage( void ) {};
};

class CDnOnCriticalHitMessage : public ::IDnObserverNotifyEvent
{
private:
	int m_iSkillID;

public:
	CDnOnCriticalHitMessage(void)
	{
		SetEventType(EVENT_ONCRITICALHIT);
	};

	virtual ~CDnOnCriticalHitMessage(void) {};

	virtual void SetSkillID(int iSkillID) { m_iSkillID = iSkillID; };
	int GetSkillID(void) { return m_iSkillID; };
};

}