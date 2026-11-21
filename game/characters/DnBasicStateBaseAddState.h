#pragma once
#include "DnBlow.h"
#include "DnBasicBlow.h"

class CDnBasicStateBaseAddState : public CDnBlow, public TBoostMemoryPool< CDnBasicStateBaseAddState >
{
protected:
public:
	CDnBasicStateBaseAddState( DnActorHandle hActor, const char* szValue );
	virtual ~CDnBasicStateBaseAddState(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:
	std::vector< CDnBasicBlow * > m_vecBasicBlow;

	void AddState( int nStateType, const float fStateValue, LOCAL_TIME LocalTime, float fDelta );
	void AddBasicBlow( const int nStateType, char * buff, LOCAL_TIME LocalTime, float fDelta );
};