#pragma once

#include "DnWorldActProp.h"


// Take action on the event only on the client.
class CDnWorldEnvironmentProp : public CDnWorldActProp
{
private:
	float m_fRange;
	string m_strOnRangeInAction;
	string m_strOnRangeOutAction;

	bool m_bActorIsInRange;

protected:
	void ReleasePostCustomParam( void ) 
	{
		if( m_pData && m_bPostCreateCustomParam )
		{
			EnvironmentPropStruct* pEnvironmentPropStruct = static_cast<EnvironmentPropStruct*>( m_pData );
			SAFE_DELETE( pEnvironmentPropStruct );
			m_pData = NULL;		// Must be set to NULL so it will not be deleted again in the destructor of a derived class.
		}
	};

public:
	CDnWorldEnvironmentProp( void );
	virtual ~CDnWorldEnvironmentProp( void );

	virtual bool InitializeTable( int nTableID );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnFinishAction( const char* szPrevAction, const char *szNextAction, LOCAL_TIME time );
};