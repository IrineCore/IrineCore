#pragma once
#include "IDnSkillProcessor.h"

class CDnPlayAniProcess : public IDnSkillProcessor, public TBoostMemoryPool< CDnPlayAniProcess >
{
protected:
	string	m_strActionName;
	string	m_strChainActionName;
	string	m_strLandingActionName;		// If the action transitions directly into a landing action, wait until the landing action also finishes before considering the skill ended.
	string	m_strOriginalActionName;	// While this is set, transitions to another action only once.

#ifdef PRE_ADD_ADDITIOANL_SKILL_ACTION
	std::vector<std::string> m_vecAdditionalAction;
#endif

protected:
	void CheckLandingAction( const char* pActionName, DnActorHandle hActor );

public:
	CDnPlayAniProcess( void ) { m_iType = PLAY_ANI; m_fTimeLength = 0.0f; }; // Constructor created for temp skill backup. Initialize essential variables.
	CDnPlayAniProcess( DnActorHandle hActor, const char* pActionName );

	virtual ~CDnPlayAniProcess(void);

	const char* GetActionName( void ) { return m_strActionName.c_str(); };
	const char* GetChainActionName() { return m_strChainActionName.c_str(); }
	const char* GetLandingActionName() { return m_strLandingActionName.c_str(); }

	// from IDnSkillProcessor
	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetNumArgument( void ) { return 1; };
	void CopyFrom( IDnSkillProcessor* pProcessor );

	// Is processing finished?
	virtual bool IsFinished( void );

	// Extend the action end time when a chain action request is received.
	void OnChainInput( const char* pActionName );

	void ChangeActionNameOnce( const char* pActionName );
	void RestoreActionNameOnce( void );


public:
	bool CheckAnimation();
};
