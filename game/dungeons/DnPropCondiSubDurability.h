#pragma once
#include "DnPropCondition.h"

class CDnWorldActProp;

// Transition by comparing prop sub-durability; used only in MultiDurabilityProp.
class CDnPropCondiSubDurability : public CDnPropCondition, 
								  public TBoostMemoryPool<CDnPropCondiSubDurability>
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
	int m_iDurabilityIndex;
	int m_iOperation;

	CDnWorldActProp* m_pProp;

public:
	CDnPropCondiSubDurability( DnPropHandle hEntity );
	virtual ~CDnPropCondiSubDurability( void );

	void Initialize( int iStandardDurability, int iDurabilityIndex, int iOperation );
	virtual bool IsSatisfy( void );
};
