#pragma once
#include "DnPropCondition.h"


// Transition by checking the prop's current action
class CDnPropActionCondition : public CDnPropCondition, 
							   public TBoostMemoryPool<CDnPropActionCondition>
{
private:
	string m_strActionName;

public:
	CDnPropActionCondition( DnPropHandle hEntity );
	virtual ~CDnPropActionCondition(void);

	void SetActionName( const char* pActionName ) 
	{  
		if( pActionName )
			m_strActionName.assign( pActionName );
	};

	virtual bool IsSatisfy( void );
};
