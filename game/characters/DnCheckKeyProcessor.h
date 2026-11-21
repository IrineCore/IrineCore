#pragma once
#include "IDnSkillProcessor.h"

class CDnCheckKeyProcess : public IDnSkillProcessor, public TBoostMemoryPool< CDnCheckKeyProcess >
{
protected:
	string				m_strActionName;
	string				m_strOriginalActionName;	// While this is set, transitions to another action only once.

	vector<string>		strTokens;

public:
	CDnCheckKeyProcess(void) { m_iType = CHECK_KEY; m_fTimeLength = 0.0f; }; // Constructor created for temp skill backup. Initialize essential variables.
	CDnCheckKeyProcess(DnActorHandle hActor, const char* pActionName);

	virtual ~CDnCheckKeyProcess(void);

	const char* GetActionName(void) { return m_strActionName.c_str(); };

	// from IDnSkillProcessor
	void OnBegin(LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill);
	void Process(LOCAL_TIME LocalTime, float fDelta);
	void OnEnd(LOCAL_TIME LocalTime, float fDelta);
	int GetNumArgument(void) { return 1; };
	void CopyFrom(IDnSkillProcessor* pProcessor);

	// Is processing finished?
	virtual bool IsFinished(void);

};