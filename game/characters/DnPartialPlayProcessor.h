#pragma once
#include "IDnSkillProcessor.h"

// Skills that play animations per segment.. for example, like "Whirlwind".
class CDnPartialPlayProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnPartialPlayProcessor >
{
protected:
	enum
	{
		START_ACTION,
		LOOP_ACTION,
		END_ACTION,
	};

	string					m_strStartActionName;
	string					m_strLoopActionName;
	string					m_strEndActionName;

	float					m_fMoveSpeed;
	int						m_iTime;
	int						m_iNowActionState;
	//bool					m_bStartedLastAction;
	float					m_fStartActionLength;
	float					m_fLoopActionLength;
	float					m_fElapsedTime;
	float					m_fLoopActionElapsedTime;

	// Finished playing up through the end action.
	bool					m_bPlayEnd;

#if defined( PRE_FIX_71333 )
	bool					m_bLoopActionEnd;	// For loop actions, check if the end action was properly queued
#endif

protected:
	void _UpdatePlayTime( void );

public:
	CDnPartialPlayProcessor( void );
	CDnPartialPlayProcessor( DnActorHandle hActor, const char* pStartActionName, 
							 const char* pLoopActionName, const char* pEndActionName, int iTime, float fMoveSpeed );
	virtual ~CDnPartialPlayProcessor(void);

	// from IDnSkillProcessor
	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	int GetNumArgument( void ) { return 5; };
	void CopyFrom( IDnSkillProcessor* pProcessor );

	const char* GetStartActionName( void ) { return m_strStartActionName.c_str(); }
	const char* GetLoopActionName( void ) { return m_strLoopActionName.c_str(); };
	const char* GetEndActionName( void ) { return m_strEndActionName.c_str(); };


#if defined(PRE_FIX_68898)
protected:
	bool m_isSkipEndAction;
public:
	void SetSkipEndAction(bool isSkipEndAction) { m_isSkipEndAction = isSkipEndAction; }
#endif // PRE_FIX_68898

};
