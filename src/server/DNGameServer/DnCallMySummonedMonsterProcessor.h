#pragma once
#include "IDnSkillProcessor.h"


// Processor responsible for overall aura skill handling. Used only on GameServer.
class CDnCallMySummonedMonsterProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnCallMySummonedMonsterProcessor >
{
private:
	int m_iSummonedMonsterIDToCall;
	bool m_bCalled;

public:
	CDnCallMySummonedMonsterProcessor( DnActorHandle hActor, int iSummonedMosnterIDToCall );
	virtual ~CDnCallMySummonedMonsterProcessor(void);

	// from IDnSkillProcessor
	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	bool IsFinished( void ) { return m_bCalled; };
	
	int GetNumArgument( void ) { return 1; };
};
