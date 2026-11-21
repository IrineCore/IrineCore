#pragma once


// Interface to check whether skill can be used
class IDnSkillUsableChecker
{
public:
	enum
	{
		OP_GREATER,
		OP_EQUAL,
		OP_NOT_EQUAL,
		OP_LESS,
		OP_GREATER_EQUAL,
		OP_LESS_EQUAL,
		
		OP_AND,
		OP_NOT_AND,			// !( A & B )
		OP_COUNT,
	};

	enum
	{
		STATE_CHECKER,
		HP_CHECKER,
		PROB_CHECKER,				
		TARGET_STATE_CHECKER,
		MOVABLE_CHECKER,
		JUMPABLE_CHECKER,
		HIT_CHECKER,
		TOGGLE_CHECKER,				// Whether a specific toggle skill is toggled on.
		ENERGY_CHARGE_CHECKER,		// Whether Cleric is in Energy Charge state. Some skills can only be used when Energy Charged.
		GROUNDMOVABLE_CHECKER,		// Check if ground movement is possible
		RANGE_CHECKER,				// Check if an enemy is within weapon + skill range. Skill can only be used if an enemy is present
		BUBBLE_COUNT_CHECKER,		// Skill usable only when bubble count is at or above threshold.
		SUMMON_CHECKER,             // Skill can be used when a summon exists and is within a certain distance
		TARGET_STATEEFECT_CHECKER,	// Check if target has the state effect
		TYPE_COUNT,
	};

protected:
	int						m_iType;
	DnActorHandle			m_hHasActor;

	IDnSkillUsableChecker( void );
	IDnSkillUsableChecker( DnActorHandle hActor ) : m_hHasActor(hActor), m_iType( TYPE_COUNT ) {};

public:
	virtual ~IDnSkillUsableChecker(void);

	void SetHasActor( DnActorHandle hActor ) { m_hHasActor = hActor; };

	virtual bool CanUse( void ) = 0;

	// Number of arguments used by Checker. Validation of data passed from the table.
	virtual int GetNumArgument( void ) = 0;

	int GetType( void ) { return m_iType; };

	// factory
	static IDnSkillUsableChecker* Create( DnActorHandle hActor, int iCheckerIndex, int iSkillLevelTableID, /*IN OUT*/ int* pIParamOffset );
	static int MapOperator( char* pOperator );

	virtual IDnSkillUsableChecker* Clone() = 0;
};
