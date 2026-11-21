#pragma once
#include "SmartPtrDef.h"

class IDnObserverNotifyEvent;
class CPacketCompressStream;

namespace BubbleSystem
{

class CDnBubbleSystem;


enum BUBBLE_HANDLER
{
	BUBBLE_HANDLER_NONE = 0,
	GETTING_BUBBLE,
	REMOVE_BUBBLE,
	UPDATE_BUBBLE_STATE_EFFECT,
	ENCHANT_SKILL_ONCE,
};

//------------------------------------------------------------------------------
/**
Interface declaration for the bubble event handler.
*/
//------------------------------------------------------------------------------
class IDnBubbleEventHandler
{
public:
	static IDnBubbleEventHandler* Create( int iEventHandlerType, DnActorHandle hActor, const char* pArgument );

	virtual ~IDnBubbleEventHandler() {}
	virtual void Initialize( DnActorHandle hActor, const char* pArgument ) = 0;
	virtual void ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent ) = 0;
	
	virtual int GetType( void ) = 0;
};


//------------------------------------------------------------------------------
/**
Process bubble acquisition.
*/
//------------------------------------------------------------------------------
class CDnGettingBubbleHandler : public IDnBubbleEventHandler
{
private:
	DnActorHandle m_hActor;
	int m_iBubbleTypeID;
	int m_iAllowedMaxBubbleCount;
	int m_iIconIndex;
	float m_fDurationTime;

public: 
	CDnGettingBubbleHandler( void ) : m_iBubbleTypeID( 0 ), m_fDurationTime( 0.0f ), m_iIconIndex( 0 ), m_iAllowedMaxBubbleCount( 0 ) {};
	virtual ~CDnGettingBubbleHandler() {}

	virtual void Initialize( DnActorHandle hActor, const char* pArgument );
	virtual void ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );

	virtual int GetType( void ) { return BUBBLE_HANDLER::GETTING_BUBBLE; }

	void SetIconIndex( int iIconIndex ) { m_iIconIndex = iIconIndex; };
};


//------------------------------------------------------------------------------
/**
Processes the input count of bubbles of a specific type.
*/
//------------------------------------------------------------------------------
class CDnRemoveBubbleHandler : public IDnBubbleEventHandler
{
private:
	DnActorHandle m_hActor;
	int m_iBubbleTypeID;
	int m_iRemoveCount;

	boost::shared_ptr<CPacketCompressStream> m_pPacketStream;
	char m_PacketBuffer[ 32 ];

public:
	CDnRemoveBubbleHandler( void ) : m_iBubbleTypeID( 0 ), m_iRemoveCount( 0 ) 
	{
		memset (m_PacketBuffer, 0x00, sizeof(m_PacketBuffer));
	};
	virtual ~CDnRemoveBubbleHandler() { m_pPacketStream.reset(); }

	virtual void Initialize( DnActorHandle hActor, const char* pArgument );
	virtual void ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );

	virtual int GetType( void ) { return BUBBLE_HANDLER::REMOVE_BUBBLE; }
};


//------------------------------------------------------------------------------
/**
State effects belonging to the passed skill ID by count of specific bubble type
Update to the matching state effect accordingly.
*/
//------------------------------------------------------------------------------
class CDnUpdateStateEffectHandler : public IDnBubbleEventHandler
{
private:
	DnActorHandle m_hActor;
	int m_iBubbleTypeID;
	int m_iSkillID;
	DnSkillHandle m_hSkill;

public:
	CDnUpdateStateEffectHandler( void ) : m_iBubbleTypeID( 0 ), m_iSkillID( 0 ) {};

	virtual void Initialize( DnActorHandle hActor, const char* pArgument );
	virtual void ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );

	virtual int GetType( void ) { return BUBBLE_HANDLER::UPDATE_BUBBLE_STATE_EFFECT; }

	int GetBubbleTypeID( void ) { return m_iBubbleTypeID; };
};

//------------------------------------------------------------------------------
/**
Handler that applies an enhancement skill to a specific skill.
Applied when used and removed when the skill ends.
*/
//------------------------------------------------------------------------------
class CDnEnchantSkillOnce : public IDnBubbleEventHandler
{
private:
	DnActorHandle m_hActor;
	int m_iTargetSkillID;
	int m_iEnchantSkillID;

public:
	CDnEnchantSkillOnce( void ) : m_iTargetSkillID( 0 ), m_iEnchantSkillID( 0 ) {};

	virtual void Initialize( DnActorHandle hActor, const char* pArgument );
	virtual void ProcessEvent( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );

	virtual int GetType( void ) { return BUBBLE_HANDLER::ENCHANT_SKILL_ONCE; }
};

}