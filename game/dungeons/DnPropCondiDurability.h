#pragma once
#include "DnPropCondition.h"


// Condition comparison object for prop durability; currently applies only to Broken Prop.
// Format is prop durability [operation] Standard value.
class CDnPropCondiDurability : public CDnPropCondition, public TBoostMemoryPool< CDnPropCondiDurability >
{
public:
	enum
	{
		EQUAL,				// ==
		GREATER,			// >
		LESS,				// <
		GREATER_EQUAL,		// >=
		LESS_EQUAL,			// <=
		OPERATION_COUNT,
	};

private:
	int m_iStandardDurability;
	int m_iOperation;

public:
	CDnPropCondiDurability( DnPropHandle hEntity );
	virtual ~CDnPropCondiDurability(void);

	void Initialize( int iStandardDurability, int iOperation );

	virtual bool IsSatisfy( void );
};
