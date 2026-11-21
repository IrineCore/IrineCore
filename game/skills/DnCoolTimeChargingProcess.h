#pragma once
#include "IDnSkillProcessor.h"

// Skill processor index 14 (1-based table 15): CoolTime Charging stack processor
class CDnCoolTimeChargingProcess : public IDnSkillProcessor, public TBoostMemoryPool< CDnCoolTimeChargingProcess >
{
protected:
	int m_nMaxChargeCount;

public:
	CDnCoolTimeChargingProcess( void );
	CDnCoolTimeChargingProcess( DnActorHandle hActor, const char* szMaxChargeCount );
	virtual ~CDnCoolTimeChargingProcess( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	virtual bool IsFinished( void ) { return true; }
	virtual int GetNumArgument( void ) { return 1; }
	virtual void CopyFrom( IDnSkillProcessor* pProcessor );

	int GetMaxChargeCount( void ) const { return m_nMaxChargeCount; }
};
