#pragma once
#include "DnPropState.h"

class CDnPropStateDoAction : public CDnPropState, public TBoostMemoryPool< CDnPropStateDoAction >
{
private:
	vector<string> m_vlAvailActionList;
	bool m_bUseCmdAction;

public:
	CDnPropStateDoAction( DnPropHandle hEntity );
	virtual ~CDnPropStateDoAction(void);

	// Randomize if 1 or more. Seed value is received from the prop, because it must match the seed sent in the packet to the client.
	void AddActionName( const char* pActionName ) { m_vlAvailActionList.push_back( string(pActionName) ); };
	void UseCmdAction( bool bUseCmdAction ) { m_bUseCmdAction = bUseCmdAction; };

	virtual bool OnEntry( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnLeave( LOCAL_TIME LocalTime, float fDelta );
};
