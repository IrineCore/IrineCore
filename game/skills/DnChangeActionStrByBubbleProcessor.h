#pragma once
#include "IDnSkillProcessor.h"


// Change the skill action configured in the skill table based on bubble count and consume the configured number of bubbles.
// Because player skills are used on the client and sent to the server, the client must also
// Check, change action, remove bubble, and the server receives the packet and processes identically. (Handled separately on client/server)
// Restore when the skill ends.
class CDnChangeActionStrByBubbleProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnChangeActionStrByBubbleProcessor >
{
protected:

	struct S_ACTION_BY_BUBBLE
	{
		int iBubbleID;
		int iBubbleCount;
		string strActionName;

		S_ACTION_BY_BUBBLE( void ) : iBubbleID( 0 ), iBubbleCount( 0 ) {};
	};
	vector<S_ACTION_BY_BUBBLE> m_vlActionByBubble;

public:

	CDnChangeActionStrByBubbleProcessor() { m_iType = CHANGE_ACTIONSTR_BY_BUBBLE; }; // Constructor created for temp skill backup. Initialize essential variables.
	CDnChangeActionStrByBubbleProcessor( DnActorHandle hActor, const char* pArg );
	virtual ~CDnChangeActionStrByBubbleProcessor( void );

	// from IDnSkillProcessor.h
	virtual void SetHasActor( DnActorHandle hActor );

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	bool IsFinished( void );
	int GetNumArgument( void ) { return 1; };

	const char *GetChangeActionNameAndRemoveNeedBubble( bool* pBChanged = NULL );

	virtual void CopyFrom( IDnSkillProcessor* pProcessor );
	vector<S_ACTION_BY_BUBBLE> *GetBubbleActionList(){ return &m_vlActionByBubble; }

};
