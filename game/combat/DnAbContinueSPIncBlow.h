#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

// Continuous SP increase flat value - #12
class CDnAbContinueSPIncBlow : public CDnBlow, public TBoostMemoryPool< CDnAbContinueSPIncBlow >
{
private:
	CDnActorStatIntervalManipulator m_SPManipulator;

public:
	CDnAbContinueSPIncBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAbContinueSPIncBlow(void);

	virtual void SetServerStartTime( LOCAL_TIME ServerStartTime ) { m_SPManipulator.SetServerStartTime( ServerStartTime ); };
	virtual void SetServerEndTime( LOCAL_TIME ServerEndTime ) { m_SPManipulator.SetServerEndTime( ServerEndTime ); };
	virtual float GetStatDelta( void ) { return m_fValue; };

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
