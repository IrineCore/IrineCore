#pragma once
#include "DnWorldActProp.h"

// This one does nothing on the server. 
// For client cosmetic use only.
class CDnWorldEnvironmentProp : public CDnWorldActProp, public TBoostMemoryPool< CDnWorldEnvironmentProp >
{
public:
	CDnWorldEnvironmentProp( CMultiRoom* pRoom );
	virtual ~CDnWorldEnvironmentProp(void);

	void ReleasePostCustomParam( void ) 
	{ 
		if( m_pData && m_bPostCreateCustomParam )
		{
			EnvironmentPropStruct* pEnvironmentPropStruct = static_cast<EnvironmentPropStruct*>( m_pData );
			SAFE_DELETE( pEnvironmentPropStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};
};

