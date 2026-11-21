#pragma once
#include "TDnFSMTransitCondition.h"

//#define ENABLE_PROP_CONDITION_LOG


class CDnPropCondition : public TDnFSMTransitCondition<DnPropHandle>
{
public:
	enum
	{
		NULL_CONDITION, // No condition.
		COMPARE_DURABILITY,
		IS_HIT,
		OPERATED,
		NEED_ITEM,		// Whether the user has a specific item..
		ACTION_CHECK,	// Whether a specific action is being performed.
		COMPARE_SUB_DURABILITY,		// Check sub durability.
		COUNT,
	};

protected:
	int m_iType;


public:
	CDnPropCondition( DnPropHandle hEntity );
	virtual ~CDnPropCondition(void);

	static CDnPropCondition* Create( DnPropHandle hEntity, int iType );

	int GetType( void ) { return m_iType; };
};
