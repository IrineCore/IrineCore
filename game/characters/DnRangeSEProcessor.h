#pragma once
#include "IDnSkillProcessor.h"

// Related to #8209.
// Applies target state effect from Fire Shield / Ice Shield skills within a certain range with a certain probability
// Processor that adds to the target.
// Created and runs only on the server.
class CDnRangeSEProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnRangeSEProcessor >
{
private:
	float m_fRange;
	float m_fProbability;
	int m_iTargetEffectOutputID;
	int m_iDurationTime;

	bool m_bFinished;
	set<DWORD> m_setAffectedActor;

public:
	CDnRangeSEProcessor( DnActorHandle hActor, float fRange, float fProbability, int iTargetEffectOutputID, int iDurationTime );
	virtual ~CDnRangeSEProcessor(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetNumArgument( void ) { return 4; };

	//bool IsFinished( void ) { return m_bFinished; }
};
