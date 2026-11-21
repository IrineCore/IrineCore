#pragma once


// Forward declaration of event class.
class IDnObserverNotifyEvent;

namespace BubbleSystem
{

class CDnBubbleSystem;

//------------------------------------------------------------------------------
/**
Detailed event handling conditions externalized to data
Order must match the "ConditionType" defined in the bubble table
The corresponding class must be defined.
*/
//------------------------------------------------------------------------------
enum BUBBLE_CONDITION
{
	BUBBLE_CONDITION_NONE = 0,
	ON_USE_SKILL,					// When a specific skill is used.
	BLOCK_SUCCESS,					// When block state effect succeeds.
	BUBBLE_COUNT_GREATER,			// When the count of a specific bubble is at least a certain amount.
	BUBBLE_COUNT_UPDATED,			// When the count of a specific bubble is updated.
	OWN_THIS_SKILL,					// Whether the actor currently possesses a specific skill.
	PARRING_SUCCESS,				// Parry succeeded.
	COOLTIME_PARRING_SUCCESS,		// Cooldown parry succeeded.
	DO_NORMAL_ATTACK,				// Execute normal attack. (Even if it misses)
	PLAYER_KILL_TARGET_ON_GHOUL_MODE, // When a player kills another target (monster/player).
	ON_USE_SKILL_WITH_SPECIFIC_SKILLLEVEL,		// When a skill is used at a specific level.
	PROBABILITY,					// Acquire bubble with a certain probability.
	ONCRITICALHIT = 17,				//Mastery 1 Dark Avenger incrase bubble on critical damage !
};


//------------------------------------------------------------------------------
/** 
Interface for the condition check object defined in the bubble table and exposed externally as data.
*/
//------------------------------------------------------------------------------
class IDnConditionChecker
{
protected:
	int m_iType;

	IDnConditionChecker( void ) : m_iType( 0 ) {};

public:
	virtual ~IDnConditionChecker( void ) {};

	static IDnConditionChecker* Create( int iConditionType, const char* pArgument );

	void SetType( int iType ) { m_iType = iType; };

	virtual void Initialize( const char* pArgument ) = 0;
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent ) = 0;
};


//------------------------------------------------------------------------------
/**
Object that checks whether a specific skill was used.
*/
//------------------------------------------------------------------------------
class CDnUseSkill : public IDnConditionChecker
{
private:
	int m_iSkillID;

public:
	CDnUseSkill( void ) : m_iSkillID( 0 )
	{
		SetType( ON_USE_SKILL );
	}
	virtual ~CDnUseSkill( void ) {};
	
	void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};


//------------------------------------------------------------------------------
/**
Object that checks for a successful block against a specific skill.
*/
//------------------------------------------------------------------------------
class CDnBlockSuccess : public IDnConditionChecker
{
private:
	// Skill ID that has this block state effect
	int m_iSkillID;

public:
	CDnBlockSuccess( void ) : m_iSkillID( 0 )
	{ 
		SetType( BLOCK_SUCCESS );
	};

	virtual ~CDnBlockSuccess( void ) {};

	void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
When the count of bubbles of a specific type is at or above a certain threshold.
*/
//------------------------------------------------------------------------------
class CDnBubbleCountGreater : public IDnConditionChecker
{
private:
	// Bubble type ID to check count for
	int m_iBubbleTypeID;
	
	// Must be at least this count.
	int m_iBasisCount;

public:
	CDnBubbleCountGreater( void ) : m_iBubbleTypeID( 0 ), m_iBasisCount( 0 )
	{ 
		SetType( BUBBLE_COUNT_GREATER );
	};

	virtual ~CDnBubbleCountGreater( void ) {};
	
	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
When the count of bubbles of a specific type is updated.
Since this condition is called directly from the bubble system, skip all checks and
Unconditionally treat as true if only the bubble type matches.
*/
//------------------------------------------------------------------------------
class CDnBubbleCountUpdated : public IDnConditionChecker
{
private:
	int m_iBubbleTypeID;

public:
	CDnBubbleCountUpdated( void ) : m_iBubbleTypeID( 0 )
	{ 
		SetType( BUBBLE_COUNT_UPDATED );
	};

	virtual ~CDnBubbleCountUpdated( void ) {};
	
	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Whether the actor has a specific skill.
*/
//------------------------------------------------------------------------------
class CDnOwnThisSkill : public IDnConditionChecker
{
private:
	int m_iSkillID;

public:
	CDnOwnThisSkill( void ) : m_iSkillID( 0 )
	{ 
		SetType( OWN_THIS_SKILL );
	};

	virtual ~CDnOwnThisSkill( void ) {};
	
	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Object that checks for a successful parry against a specific skill.
*/
//------------------------------------------------------------------------------
class CDnParringSuccess : public IDnConditionChecker
{
private:
	// Skill ID that has this block state effect
	int m_iSkillID;

public:
	CDnParringSuccess( void ) : m_iSkillID( 0 )
	{ 
		SetType( PARRING_SUCCESS );
	};

	virtual ~CDnParringSuccess( void ) {};

	void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Object that checks for a successful cooldown parry against a specific skill.
*/
//------------------------------------------------------------------------------
class CDnCooltimeParringSuccess : public IDnConditionChecker
{
private:
	// Skill ID that has this block state effect
	int m_iSkillID;

public:
	CDnCooltimeParringSuccess( void ) : m_iSkillID( 0 )
	{ 
		SetType( COOLTIME_PARRING_SUCCESS );
	};

	virtual ~CDnCooltimeParringSuccess( void ) {};

	void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Whether a normal attack was executed.
*/
//------------------------------------------------------------------------------
class CDnDoNormalAttack : public IDnConditionChecker
{
private:

public:
	CDnDoNormalAttack( void )
	{
		SetType( DO_NORMAL_ATTACK );
	}

	virtual ~CDnDoNormalAttack( void ) {};

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
When a player kills another target.
*/
//------------------------------------------------------------------------------
class CDnPlayerKillTargetOnGhoulMode : public IDnConditionChecker
{
private:

public:
	CDnPlayerKillTargetOnGhoulMode( void )
	{
		SetType( PLAYER_KILL_TARGET_ON_GHOUL_MODE );
	}

	virtual ~CDnPlayerKillTargetOnGhoulMode( void ) {};

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Object that checks whether a specific skill was used at a specific level.
*/
//------------------------------------------------------------------------------
class CDnUseSkillWithSpecificSkillLevel : public IDnConditionChecker
{
private:
	int m_iSkillID;
	int m_iSkillLevel;

public:
	CDnUseSkillWithSpecificSkillLevel( void ) : m_iSkillID( 0 ), m_iSkillLevel( 0 )
	{
		SetType( ON_USE_SKILL_WITH_SPECIFIC_SKILLLEVEL );
	}
	virtual ~CDnUseSkillWithSpecificSkillLevel( void ) {};
	
	void SetSkillID( int iSkillID ) { m_iSkillID = iSkillID; };
	void SetSkillLevel( int iSkillLevel ) { m_iSkillLevel = iSkillLevel; };

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//------------------------------------------------------------------------------
/**
Check probability.
*/
//------------------------------------------------------------------------------
class CDnProbability : public IDnConditionChecker
{
private:
	float m_fProbability;

public:
	CDnProbability( void ) : m_fProbability( 0.0f )
	{
		SetType( PROBABILITY );
	}
	virtual ~CDnProbability( void ) {};

	virtual void Initialize( const char* pArgument );
	virtual bool IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent );
};

//mastery 1 dark avenger on critical hit!
class CDnOnCriticalHit : public IDnConditionChecker
{
private:
	std::vector<int> m_vecCheckSkills;

public:
	CDnOnCriticalHit(void)
	{
		SetType(ONCRITICALHIT);
	}
	virtual ~CDnOnCriticalHit(void) {};
	void SetVecList(std::vector<int> lista) { m_vecCheckSkills = lista; }
	std::vector<int> GetVecList(void) { return m_vecCheckSkills; }
	virtual void Initialize(const char* pArgument);
	virtual bool IsSatisfy(CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent);
};

}